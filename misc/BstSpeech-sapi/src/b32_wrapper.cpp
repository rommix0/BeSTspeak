#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <windows.h>
#include "b32_wrapper.h"
#include "MinHook.h"
#include "debug_log.h"

extern "C" {
#include "sonic.h"
}

namespace b32 {

namespace constants {
    constexpr int RATE_SETTING = 257;
    constexpr int GAIN_SETTING = 258;
    constexpr int BIT_DEPTH_SETTING = 4097;
    constexpr int SAMPLE_RATE = 11025;
    constexpr int CHANNELS = 1;
    constexpr UINT WM_REL_BUF = WM_USER + 1;
}

static const char* const voice_data[] = {
    "Fred", "~v0]~e3]~h0]~u0]~f80]", "~r0]",
    "Sara", "~v2]~e3]~h-20]~u0]~f175]", "~r0]",
    "Hary", "~v3]~e3]~h10]~u0]~f65]", "~r5]",
    "Wendy", "~v2]~e1]~h50]~u0]~f150]", "~r-5]",
    "Dexter", "~v6]~e6]~h0]~u-25]~f90]", "~r7]",
    "Alien", "~v4]~e6]~h-50]~u-20]~f115]", "~r-20]",
    "Kit", "~v5]~e3]~h40]~u0]~f230]", "~r-10]",
    "Bruno", "~v3]~e3]~h50]~u0]~f60]", "~r8]",
    "Ghost", "~v3]~e2]~h50]~u0]~f60]", "~r8]",
    "Peeper", "~v2]~e2]~h0]~u5]~f80]", "~r0]",
    "Dracula", "~v3]~e3]~h45]~u-5]~f47]", "~r10]",
    "Granny", "~v4]~e3]~h-60]~u0]~f350]", "~r20]",
    "Martha", "~v6]~e4]~h100]~u-5]~f300]", "~r-10]",
    "Tim", "~v3]~e4]~h-10]~u0]~f60]", "~r-10]",
    nullptr, nullptr, nullptr
};

static int voice_count = 0;
static std::vector<const char*> voices_buffer;

using BstCreateFunc = int(__cdecl*)(long*&);
using TtsWavFunc = int(__cdecl*)(long*, void*, const char*);
using BstRelBufFunc = void(__cdecl*)(long*);
using BstCloseFunc = void(__cdecl*)(long*);
using BstDestroyFunc = void(__cdecl*)();
using BstSetParamsFunc = void(__cdecl*)(long*, int, int);

using WaveOutOpenFunc = MMRESULT(WINAPI*)(LPHWAVEOUT, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD);
using WaveOutHeaderFunc = MMRESULT(WINAPI*)(HWAVEOUT, WAVEHDR*, UINT);
using WaveOutResetFunc = MMRESULT(WINAPI*)(HWAVEOUT);

static WaveOutOpenFunc original_waveOutOpen;
static WaveOutHeaderFunc original_waveOutPrepareHeader;
static WaveOutHeaderFunc original_waveOutWrite;
static WaveOutHeaderFunc original_waveOutUnprepareHeader;
static WaveOutResetFunc original_waveOutReset;
static WaveOutResetFunc original_waveOutClose;

struct State {
    HMODULE dll = nullptr;
    long* tts = nullptr;
    BstCreateFunc bstCreate = nullptr;
    TtsWavFunc TtsWav = nullptr;
    BstRelBufFunc bstRelBuf = nullptr;
    BstCloseFunc bstClose = nullptr;
    BstDestroyFunc bstDestroy = nullptr;
    BstSetParamsFunc bstSetParams = nullptr;
    AsyncCallback async_callback = nullptr;
    void* async_callback_user = nullptr;
    bool async_stop_speaking = false;
    HWND message_window = nullptr;
    sonicStream sonic = nullptr;
    float rate_multiplier = 1.0f;
};

static bool winmm_hooked = false;
static thread_local State* winmm_hooked_state = nullptr;
static HINSTANCE g_hinstance = nullptr;

static LRESULT CALLBACK on_rel_buf(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == constants::WM_REL_BUF) {
        if (winmm_hooked_state && winmm_hooked_state->bstRelBuf) {
            winmm_hooked_state->bstRelBuf(winmm_hooked_state->tts);
        }
    } else if (message == WM_DESTROY) {
        PostQuitMessage(0);
    } else {
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

static HWND create_message_window() {
    WNDCLASSW wc = {};
    if (!GetClassInfoW(g_hinstance, L"b32tts_wrapper_class", &wc)) {
        wc.lpfnWndProc = on_rel_buf;
        wc.hInstance = g_hinstance;
        wc.lpszClassName = L"b32tts_wrapper_class";
        if (!RegisterClassW(&wc)) {
            return nullptr;
        }
    }
    return CreateWindowExW(0, L"b32tts_wrapper_class", L"b32tts_wrapper_window",
                           0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, g_hinstance, nullptr);
}

void set_hinstance(HINSTANCE h) noexcept {
    g_hinstance = h;
}

static MMRESULT WINAPI hook_waveOutOpen(LPHWAVEOUT outptr, UINT device,
                                        LPCWAVEFORMATEX format, DWORD_PTR callback,
                                        DWORD_PTR instance, DWORD flags) {
    if (!winmm_hooked_state || reinterpret_cast<State*>(callback) != winmm_hooked_state) {
        return original_waveOutOpen(outptr, device, format, callback, instance, flags);
    }
    *outptr = reinterpret_cast<HWAVEOUT>(callback);
    return MMSYSERR_NOERROR;
}

static MMRESULT WINAPI hook_waveOutPrepareHeader(HWAVEOUT ptr, WAVEHDR* header, UINT size) {
    if (!winmm_hooked_state || reinterpret_cast<State*>(ptr) != winmm_hooked_state) {
        return original_waveOutPrepareHeader(ptr, header, size);
    }
    return MMSYSERR_NOERROR;
}

static MMRESULT WINAPI hook_waveOutWrite(HWAVEOUT ptr, WAVEHDR* header, UINT size) {
    if (!winmm_hooked_state || reinterpret_cast<State*>(ptr) != winmm_hooked_state) {
        return original_waveOutWrite(ptr, header, size);
    }

    DEBUG_LOG("--- Audio Callback: Received %lu bytes", header->dwBufferLength);

    PostMessage(winmm_hooked_state->message_window, constants::WM_REL_BUF, 0, 0);

    if (winmm_hooked_state->async_stop_speaking) {
        DEBUG_LOG("  Audio Callback: STOPPING (async_stop_speaking = true)");
        return MMSYSERR_NOERROR;
    }

    if (winmm_hooked_state->async_callback) {
        sonicStream stream = winmm_hooked_state->sonic;
        if (stream && winmm_hooked_state->rate_multiplier != 1.0f) {
            const int numSamples = static_cast<int>(header->dwBufferLength / 2);
            DEBUG_LOG("  Audio Callback: Processing through SONIC (%d samples)", numSamples);
            sonicWriteShortToStream(stream, reinterpret_cast<short*>(header->lpData), numSamples);

            int totalRead = 0;
            int available;
            while ((available = sonicSamplesAvailable(stream)) > 0) {
                std::vector<short> outBuf(static_cast<size_t>(available));
                const int read = sonicReadShortFromStream(stream, outBuf.data(), available);
                if (read > 0) {
                    totalRead += read;
                    if (!winmm_hooked_state->async_callback(
                            reinterpret_cast<const char*>(outBuf.data()),
                            read * 2,
                            winmm_hooked_state->async_callback_user)) {
                        DEBUG_LOG("  Audio Callback: CALLBACK RETURNED FALSE (stopping)");
                        winmm_hooked_state->async_stop_speaking = true;
                        break;
                    }
                }
            }
            DEBUG_LOG("  Audio Callback: Sonic processed %d samples total", totalRead);
        } else {
            DEBUG_LOG("  Audio Callback: Direct passthrough (%lu bytes)", header->dwBufferLength);
            if (!winmm_hooked_state->async_callback(
                    header->lpData,
                    static_cast<long>(header->dwBufferLength),
                    winmm_hooked_state->async_callback_user)) {
                DEBUG_LOG("  Audio Callback: CALLBACK RETURNED FALSE (stopping)");
                winmm_hooked_state->async_stop_speaking = true;
            }
        }
    }
    DEBUG_LOG("  Audio Callback: Completed");
    return MMSYSERR_NOERROR;
}

static MMRESULT WINAPI hook_waveOutUnprepareHeader(HWAVEOUT ptr, WAVEHDR* header, UINT size) {
    if (!winmm_hooked_state || reinterpret_cast<State*>(ptr) != winmm_hooked_state) {
        return original_waveOutUnprepareHeader(ptr, header, size);
    }
    return MMSYSERR_NOERROR;
}

static MMRESULT WINAPI hook_waveOutReset(HWAVEOUT ptr) {
    if (!winmm_hooked_state || reinterpret_cast<State*>(ptr) != winmm_hooked_state) {
        return original_waveOutReset(ptr);
    }
    return MMSYSERR_NOERROR;
}

static MMRESULT WINAPI hook_waveOutClose(HWAVEOUT ptr) {
    if (!winmm_hooked_state || reinterpret_cast<State*>(ptr) != winmm_hooked_state) {
        return original_waveOutClose(ptr);
    }
    return MMSYSERR_NOERROR;
}

static void install_winmm_hooks() {
    if (winmm_hooked) return;

    MH_Initialize();
    MH_CreateHook(reinterpret_cast<LPVOID>(waveOutOpen),
                  reinterpret_cast<LPVOID>(hook_waveOutOpen),
                  reinterpret_cast<LPVOID*>(&original_waveOutOpen));
    MH_CreateHook(reinterpret_cast<LPVOID>(waveOutPrepareHeader),
                  reinterpret_cast<LPVOID>(hook_waveOutPrepareHeader),
                  reinterpret_cast<LPVOID*>(&original_waveOutPrepareHeader));
    MH_CreateHook(reinterpret_cast<LPVOID>(waveOutWrite),
                  reinterpret_cast<LPVOID>(hook_waveOutWrite),
                  reinterpret_cast<LPVOID*>(&original_waveOutWrite));
    MH_CreateHook(reinterpret_cast<LPVOID>(waveOutUnprepareHeader),
                  reinterpret_cast<LPVOID>(hook_waveOutUnprepareHeader),
                  reinterpret_cast<LPVOID*>(&original_waveOutUnprepareHeader));
    MH_CreateHook(reinterpret_cast<LPVOID>(waveOutReset),
                  reinterpret_cast<LPVOID>(hook_waveOutReset),
                  reinterpret_cast<LPVOID*>(&original_waveOutReset));
    MH_CreateHook(reinterpret_cast<LPVOID>(waveOutClose),
                  reinterpret_cast<LPVOID>(hook_waveOutClose),
                  reinterpret_cast<LPVOID*>(&original_waveOutClose));
    MH_EnableHook(MH_ALL_HOOKS);

    winmm_hooked = true;
}

static void count_voices() {
    if (voice_count == 0) {
        for (int i = 0; voice_data[i] != nullptr; i += 3) {
            voice_count++;
        }
    }
}

const char* const* get_voices(int* count) noexcept {
    count_voices();

    if (voices_buffer.empty()) {
        voices_buffer.reserve(static_cast<size_t>(voice_count) + 1);
        for (int i = 0; i < voice_count; i++) {
            voices_buffer.push_back(voice_data[i * 3]);
        }
        voices_buffer.push_back(nullptr);
    }

    if (count) {
        *count = voice_count;
    }
    return voices_buffer.data();
}

static StatePtr init_from_hmodule(HMODULE hmod) {
    if (!hmod) return nullptr;

    count_voices();

    auto state = std::make_unique<State>();
    state->dll = hmod;
    state->bstCreate = reinterpret_cast<BstCreateFunc>(GetProcAddress(hmod, "bstCreate"));
    state->TtsWav = reinterpret_cast<TtsWavFunc>(GetProcAddress(hmod, "TtsWav"));
    state->bstRelBuf = reinterpret_cast<BstRelBufFunc>(GetProcAddress(hmod, "bstRelBuf"));
    state->bstDestroy = reinterpret_cast<BstDestroyFunc>(GetProcAddress(hmod, "bstDestroy"));
    state->bstClose = reinterpret_cast<BstCloseFunc>(GetProcAddress(hmod, "bstClose"));
    state->bstSetParams = reinterpret_cast<BstSetParamsFunc>(GetProcAddress(hmod, "bstSetParams"));

    if (!state->bstCreate || !state->TtsWav || state->bstCreate(state->tts) != 0) {
        FreeLibrary(hmod);
        return nullptr;
    }

    state->bstSetParams(state->tts, constants::BIT_DEPTH_SETTING, 16);
    return StatePtr(state.release());
}

StatePtr init(const char* module_path) noexcept {
    return init_from_hmodule(LoadLibraryA(module_path));
}

StatePtr init(const wchar_t* module_path) noexcept {
    return init_from_hmodule(LoadLibraryW(module_path));
}

void StateDeleter::operator()(State* s) const noexcept {
    if (!s) return;

    if (s->bstClose && s->tts) {
        s->bstClose(s->tts);
    }
    if (s->bstDestroy) {
        s->bstDestroy();
    }
    if (s->dll) {
        FreeLibrary(s->dll);
    }
    if (s->message_window) {
        DestroyWindow(s->message_window);
    }

    delete s;
}

void speak_async(
    State* s,
    AsyncCallback callback,
    void* user,
    const char* text,
    int voice,
    int native_rate,
    float sonic_multiplier,
    int gain,
    int frequency
) noexcept {
    if (!s || !text || !callback) return;

    if (!s->message_window) {
        s->message_window = create_message_window();
    }

    DEBUG_LOG("--- Text Sanitization ---");
    DEBUG_LOG("  Original text length: %zu bytes", strlen(text));
    DEBUG_LOG("  First 100 chars: %.100s", text);

    std::string sanitized_text(text);

    size_t pos = 0;
    int replacements = 0;
    while ((pos = sanitized_text.find("\xE2\x80\xA6", pos)) != std::string::npos) {
        sanitized_text.replace(pos, 3, "...");
        pos += 3;
        replacements++;
    }
    if (replacements > 0) DEBUG_LOG("  Replaced %d ellipsis characters", replacements);

    pos = 0;
    while ((pos = sanitized_text.find("\xE2\x80\x93", pos)) != std::string::npos) {
        sanitized_text.replace(pos, 3, "-");
        pos += 1;
    }
    pos = 0;
    while ((pos = sanitized_text.find("\xE2\x80\x94", pos)) != std::string::npos) {
        sanitized_text.replace(pos, 3, "-");
        pos += 1;
    }
    pos = 0;
    while ((pos = sanitized_text.find("\xE2\x80\x98", pos)) != std::string::npos) {
        sanitized_text.replace(pos, 3, "'");
        pos += 1;
    }
    pos = 0;
    while ((pos = sanitized_text.find("\xE2\x80\x99", pos)) != std::string::npos) {
        sanitized_text.replace(pos, 3, "'");
        pos += 1;
    }
    pos = 0;
    while ((pos = sanitized_text.find("\xE2\x80\x9C", pos)) != std::string::npos) {
        sanitized_text.replace(pos, 3, "\"");
        pos += 1;
    }
    pos = 0;
    while ((pos = sanitized_text.find("\xE2\x80\x9D", pos)) != std::string::npos) {
        sanitized_text.replace(pos, 3, "\"");
        pos += 1;
    }

    for (size_t i = 0; i < sanitized_text.length(); ++i) {
        if (sanitized_text[i] == '\r' || sanitized_text[i] == '\n') {
            sanitized_text[i] = ' ';
        }
    }

    sanitized_text.erase(
        std::remove_if(sanitized_text.begin(), sanitized_text.end(),
            [](char c) { return (c < 32 || c > 126) && c != ' '; }),
        sanitized_text.end()
    );

    pos = 0;
    while ((pos = sanitized_text.find("  ", pos)) != std::string::npos) {
        sanitized_text.replace(pos, 2, " ");
    }

    DEBUG_LOG("  Sanitized text length: %zu bytes", sanitized_text.length());
    DEBUG_LOG("  First 100 chars after sanitization: %.100s", sanitized_text.c_str());

    std::string actual_text;
    if (voice >= 0 && voice < voice_count) {
        std::string voice_params = voice_data[voice * 3 + 1];

        if (frequency >= 0) {
            size_t freq_pos = voice_params.find("~f");
            if (freq_pos != std::string::npos) {
                size_t end_pos = voice_params.find("]", freq_pos);
                if (end_pos != std::string::npos) {
                    std::string original_freq_str = voice_params.substr(freq_pos + 2, end_pos - freq_pos - 2);
                    int original_freq = std::atoi(original_freq_str.c_str());

                    const int final_freq = std::clamp(frequency, 45, 600);

                    DEBUG_LOG("  Frequency Override: %d Hz -> %d Hz (clamped from %d)",
                              original_freq, final_freq, frequency);

                    voice_params.replace(freq_pos, end_pos - freq_pos + 1,
                                       "~f" + std::to_string(final_freq) + "]");
                }
            }
        }

        actual_text = voice_params;
        actual_text += sanitized_text;
    } else {
        actual_text = sanitized_text;
    }

    DEBUG_LOG("--- b32_wrapper: speak_async ---");
    DEBUG_LOG("  Voice: %d, Native Rate: %d, Sonic Multiplier: %.2fx", voice, native_rate, sonic_multiplier);
    DEBUG_LOG("  Gain: %d, Frequency: %d Hz", gain, frequency);
    DEBUG_LOG("  Final text length (with voice params): %zu bytes", actual_text.length());
    DEBUG_LOG("  Final text to TTS engine: %.100s", actual_text.c_str());

    s->bstSetParams(s->tts, constants::GAIN_SETTING, gain);
    s->bstSetParams(s->tts, constants::RATE_SETTING, native_rate);
    DEBUG_LOG("  Set bstSetParams: GAIN=%d, RATE=%d", gain, native_rate);

    if (s->sonic) {
        sonicDestroyStream(s->sonic);
        s->sonic = nullptr;
    }

    s->rate_multiplier = sonic_multiplier;
    if (sonic_multiplier != 1.0f) {
        s->sonic = sonicCreateStream(constants::SAMPLE_RATE, constants::CHANNELS);
        if (s->sonic) {
            sonicSetSpeed(s->sonic, sonic_multiplier);
            sonicSetQuality(s->sonic, 1);
            DEBUG_LOG("  Sonic ENABLED - Speed: %.2fx, Quality: High", sonic_multiplier);
        }
    } else {
        DEBUG_LOG("  Sonic DISABLED - Using native rate only");
    }

    s->async_callback = callback;
    s->async_callback_user = user;
    s->async_stop_speaking = false;

    install_winmm_hooks();
    winmm_hooked_state = s;

    DEBUG_LOG("--- Calling TTS Engine (TtsWav) ---");
    s->TtsWav(s->tts, s, actual_text.c_str());
    DEBUG_LOG("--- TTS Engine Returned ---");

    if (s->sonic) {
        DEBUG_LOG("--- Flushing Sonic Stream ---");
        sonicFlushStream(s->sonic);
        int available;
        int totalFlushed = 0;
        while ((available = sonicSamplesAvailable(s->sonic)) > 0) {
            std::vector<short> outBuf(static_cast<size_t>(available));
            const int read = sonicReadShortFromStream(s->sonic, outBuf.data(), available);
            if (read > 0 && !s->async_stop_speaking) {
                totalFlushed += read;
                s->async_callback(
                    reinterpret_cast<const char*>(outBuf.data()),
                    read * 2,
                    s->async_callback_user
                );
            }
        }
        DEBUG_LOG("  Sonic flush: %d samples flushed", totalFlushed);
        sonicDestroyStream(s->sonic);
        s->sonic = nullptr;
    }

    winmm_hooked_state = nullptr;
    DEBUG_LOG("=== Speech Complete ===\n");
}
}
