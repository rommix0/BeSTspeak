#include <new>
#include <string>
#include <cmath>
#include <algorithm>
#include "utils.hpp"
#include "ISpTTSEngineImpl.hpp"
#include "debug_log.h"

#ifdef BUILD_X64
#include "pipe_client.h"
#else
#include "b32_wrapper.h"
#endif

namespace Bestspeech {
namespace sapi {

namespace {

constexpr WORD AUDIO_CHANNELS = 1;
constexpr DWORD AUDIO_SAMPLE_RATE = 11025;
constexpr WORD AUDIO_BITS_PER_SAMPLE = 16;

constexpr int MIN_RATE = -10;
constexpr int MAX_RATE = 10;

constexpr int NATIVE_RATE_MIN = -100;
constexpr int NATIVE_RATE_MAX = 100;

constexpr int FREQ_MIN = 45;
constexpr int FREQ_MAX = 400;

constexpr int VOICE_DEFAULT_FREQS[] = {
    80,
    175,
    65,
    150,
    90,
    115,
    230,
    60,
    60,
    80,
    47,
    350,
    300,
    60
};
constexpr int VOICE_COUNT = sizeof(VOICE_DEFAULT_FREQS) / sizeof(VOICE_DEFAULT_FREQS[0]);

#ifdef BUILD_X64
PipeClient* g_pipeClient = nullptr;
#endif

struct SpeakContext {
    ISpTTSEngineSite* caller = nullptr;
    ULONGLONG bytes_written = 0;
    bool aborted = false;
};

#ifdef BUILD_X64
bool speak_callback(const char* data, uint32_t size, void* user) {
#else
bool speak_callback(const char* data, long size, void* user) {
#endif
    auto* ctx = static_cast<SpeakContext*>(user);
    if (!ctx || !ctx->caller) {
        DEBUG_LOG("SAPI Callback: ERROR - No context or caller");
        return false;
    }

    const DWORD actions = ctx->caller->GetActions();
    if (actions & SPVES_ABORT) {
        DEBUG_LOG("SAPI Callback: ABORT requested");
        ctx->aborted = true;
        return false;
    }
    if (actions & SPVES_SKIP) {
        DEBUG_LOG("SAPI Callback: SKIP requested");
        ctx->caller->CompleteSkip(0);
        ctx->aborted = true;
        return false;
    }

    DEBUG_LOG("SAPI Callback: Writing %ld bytes to SAPI", (long)size);

    auto ptr = reinterpret_cast<const BYTE*>(data);
    ULONG remaining = static_cast<ULONG>(size);

    while (remaining > 0) {
        const DWORD actions = ctx->caller->GetActions();
        if (actions & SPVES_ABORT) {
            DEBUG_LOG("SAPI Callback: ABORT during write");
            ctx->aborted = true;
            return false;
        }
        if (actions & SPVES_SKIP) {
            DEBUG_LOG("SAPI Callback: SKIP during write");
            ctx->caller->CompleteSkip(0);
            ctx->aborted = true;
            return false;
        }

        ULONG written = remaining;
        HRESULT hr = ctx->caller->Write(ptr, remaining, &written);
        if (FAILED(hr)) {
            DEBUG_LOG("SAPI Callback: Write FAILED with HRESULT 0x%08X", hr);
            return false;
        }
        if (written > remaining) {
            DEBUG_LOG("SAPI Callback: Write error - written (%lu) > remaining (%lu)", written, remaining);
            return false;
        }
        ctx->bytes_written += written;
        remaining -= written;
        ptr += written;
    }

    DEBUG_LOG("SAPI Callback: Successfully wrote %ld bytes", (long)size);
    return true;
}
}

#ifdef BUILD_X64
void InitPipeClient()
{
    if (!g_pipeClient) {
        g_pipeClient = new PipeClient();
    }
}

void CleanupPipeClient()
{
    delete g_pipeClient;
    g_pipeClient = nullptr;
}

void ShutdownPipeServer()
{
    if (g_pipeClient) {
        g_pipeClient->shutdownServer();
    }
}
#endif

ISpTTSEngineImpl::ISpTTSEngineImpl()
    : voice_index_(0)
{
}

ISpTTSEngineImpl::~ISpTTSEngineImpl() = default;

STDMETHODIMP ISpTTSEngineImpl::SetObjectToken(ISpObjectToken* pToken)
{
    DEBUG_LOG("=== SetObjectToken Called ===");

    if (!pToken) {
        DEBUG_LOG("SetObjectToken: ERROR - pToken is NULL");
        return E_INVALIDARG;
    }

    try {
        ISpDataKeyPtr attr;
        if (FAILED(pToken->OpenKey(L"Attributes", &attr))) {
            DEBUG_LOG("SetObjectToken: ERROR - Failed to open Attributes key");
            return E_INVALIDARG;
        }

        utils::out_ptr<wchar_t> name(CoTaskMemFree);
        if (FAILED(attr->GetStringValue(L"Name", name.address()))) {
            DEBUG_LOG("SetObjectToken: ERROR - Failed to get Name attribute");
            return E_INVALIDARG;
        }

        const std::string voice_name = utils::wstring_to_string(name.get());
        DEBUG_LOG("SetObjectToken: Voice name = %s", voice_name.c_str());
        voice_index_ = 0;

#ifdef BUILD_X64
        if (g_pipeClient) {
            std::vector<VoiceInfo> voices;
            if (g_pipeClient->getVoices(voices)) {
                for (size_t i = 0; i < voices.size(); ++i) {
                    if (_stricmp(voices[i].name, voice_name.c_str()) == 0) {
                        voice_index_ = static_cast<int>(i);
                        break;
                    }
                }
            }
        }
#else
        for (int i = 0; i < bst_voice_count; ++i) {
            if (_stricmp(bst_voices[i].name, voice_name.c_str()) == 0) {
                voice_index_ = i;
                break;
            }
        }
#endif

        token_ = pToken;
        DEBUG_LOG("SetObjectToken: SUCCESS - Voice index = %d", voice_index_);
        return S_OK;
    }
    catch (const std::bad_alloc&) {
        DEBUG_LOG("SetObjectToken: ERROR - Out of memory");
        return E_OUTOFMEMORY;
    }
    catch (...) {
        DEBUG_LOG("SetObjectToken: ERROR - Unexpected exception");
        return E_UNEXPECTED;
    }
}

STDMETHODIMP ISpTTSEngineImpl::GetObjectToken(ISpObjectToken** ppToken)
{
    DEBUG_LOG("=== GetObjectToken Called ===");

    if (!ppToken) {
        DEBUG_LOG("GetObjectToken: ERROR - ppToken is NULL");
        return E_POINTER;
    }
    *ppToken = nullptr;

    if (token_) {
        token_.AddRef();
        *ppToken = token_.GetInterfacePtr();
        DEBUG_LOG("GetObjectToken: SUCCESS - Returned token");
        return S_OK;
    }
    DEBUG_LOG("GetObjectToken: ERROR - No token set");
    return E_UNEXPECTED;
}

STDMETHODIMP ISpTTSEngineImpl::GetOutputFormat(
    const GUID* /*pTargetFmtId*/,
    const WAVEFORMATEX* /*pTargetWaveFormatEx*/,
    GUID* pOutputFormatId,
    WAVEFORMATEX** ppCoMemOutputWaveFormatEx)
{
    DEBUG_LOG("=== GetOutputFormat Called ===");

    if (!pOutputFormatId) {
        DEBUG_LOG("GetOutputFormat: ERROR - pOutputFormatId is NULL");
        return E_POINTER;
    }
    if (!ppCoMemOutputWaveFormatEx) {
        DEBUG_LOG("GetOutputFormat: ERROR - ppCoMemOutputWaveFormatEx is NULL");
        return E_POINTER;
    }

    *pOutputFormatId = SPDFID_WaveFormatEx;
    *ppCoMemOutputWaveFormatEx = nullptr;

    auto* pwfex = static_cast<WAVEFORMATEX*>(CoTaskMemAlloc(sizeof(WAVEFORMATEX)));
    if (!pwfex) {
        DEBUG_LOG("GetOutputFormat: ERROR - Out of memory");
        return E_OUTOFMEMORY;
    }

    pwfex->wFormatTag = WAVE_FORMAT_PCM;
    pwfex->nChannels = AUDIO_CHANNELS;
    pwfex->nSamplesPerSec = AUDIO_SAMPLE_RATE;
    pwfex->wBitsPerSample = AUDIO_BITS_PER_SAMPLE;
    pwfex->nBlockAlign = pwfex->nChannels * pwfex->wBitsPerSample / 8;
    pwfex->nAvgBytesPerSec = pwfex->nSamplesPerSec * pwfex->nBlockAlign;
    pwfex->cbSize = 0;

    *ppCoMemOutputWaveFormatEx = pwfex;
    DEBUG_LOG("GetOutputFormat: SUCCESS - Channels=%d, Rate=%lu, Bits=%d",
              pwfex->nChannels, pwfex->nSamplesPerSec, pwfex->wBitsPerSample);
    return S_OK;
}

STDMETHODIMP ISpTTSEngineImpl::Speak(
    DWORD dwSpeakFlags,
    REFGUID /*rguidFormatId*/,
    const WAVEFORMATEX* /*pWaveFormatEx*/,
    const SPVTEXTFRAG* pTextFragList,
    ISpTTSEngineSite* pOutputSite)
{
    DEBUG_LOG("=== Speak Called ===");
    DEBUG_LOG("Speak Flags: 0x%08X", dwSpeakFlags);

    if (!pTextFragList) {
        DEBUG_LOG("Speak: ERROR - pTextFragList is NULL");
        return E_INVALIDARG;
    }
    if (!pOutputSite) {
        DEBUG_LOG("Speak: ERROR - pOutputSite is NULL");
        return E_INVALIDARG;
    }

#ifdef BUILD_X64
    if (!g_pipeClient) {
        return E_FAIL;
    }
#endif

    try {
#ifndef BUILD_X64
        if (!bst_state_) {
            wchar_t dll_path[MAX_PATH];
            HMODULE hm = nullptr;

            if (GetModuleHandleExW(
                    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                    reinterpret_cast<LPCWSTR>(&speak_callback), &hm)) {
                GetModuleFileNameW(hm, dll_path, MAX_PATH);
                wchar_t* last_slash = wcsrchr(dll_path, L'\\');
                if (last_slash) {
                    wcscpy_s(last_slash + 1, MAX_PATH - (last_slash - dll_path + 1), L"b32_tts.dll");
                    bst_state_ = b32::init(dll_path);
                }
            }

            if (!bst_state_) {
                bst_state_ = b32::init(static_cast<const char*>(nullptr));
            }

            if (!bst_state_) {
                return E_FAIL;
            }
        }
#endif

        long sapi_rate = 0;
        pOutputSite->GetRate(&sapi_rate);

        unsigned short sapi_volume = 100;
        pOutputSite->GetVolume(&sapi_volume);
        int gain = static_cast<int>((sapi_volume - 100) * 0.5);

        DEBUG_LOG("=== New Speech Request ===");
        DEBUG_LOG("Voice Index: %d", voice_index_);
        DEBUG_LOG("SAPI Rate: %d, SAPI Volume: %u (Gain: %d)", (int)sapi_rate, sapi_volume, gain);

        ULONGLONG event_interest = 0;
        pOutputSite->GetEventInterest(&event_interest);
        const bool send_sentence_events = (event_interest & (1ULL << SPEI_SENTENCE_BOUNDARY)) != 0;
        const bool send_word_events = (event_interest & (1ULL << SPEI_WORD_BOUNDARY)) != 0;
        DEBUG_LOG("Event interest: 0x%llX (sentence: %d, word: %d)", event_interest, send_sentence_events, send_word_events);

        SpeakContext ctx;
        ctx.caller = pOutputSite;
        ctx.bytes_written = 0;
        ctx.aborted = false;

        int frag_count = 0;
        for (const SPVTEXTFRAG* f = pTextFragList; f; f = f->pNext) {
            frag_count++;
        }
        DEBUG_LOG("Fragment count: %d", frag_count);

        int frag_num = 0;
        for (const SPVTEXTFRAG* frag = pTextFragList; frag; frag = frag->pNext) {
            frag_num++;
            DEBUG_LOG("--- Processing Fragment %d/%d ---", frag_num, frag_count);
            const DWORD actions = pOutputSite->GetActions();

            if (actions & SPVES_ABORT) {
                break;
            }
            if (actions & SPVES_SKIP) {
                pOutputSite->CompleteSkip(0);
                break;
            }

            if (actions & SPVES_RATE) {
                pOutputSite->GetRate(&sapi_rate);
            }
            if (actions & SPVES_VOLUME) {
                pOutputSite->GetVolume(&sapi_volume);
                gain = static_cast<int>((sapi_volume - 100) * 0.5);
            }

            DEBUG_LOG("Fragment eAction: %d (SPVA_Speak=0, SPVA_Silence=1, SPVA_Pronounce=2, SPVA_Bookmark=3, SPVA_SpellOut=4)",
                      frag->State.eAction);
            DEBUG_LOG("Fragment ulTextSrcOffset: %lu, ulTextLen: %lu", frag->ulTextSrcOffset, frag->ulTextLen);

            if (frag->State.eAction == SPVA_Bookmark) {
                DEBUG_LOG("Fragment is a BOOKMARK");
                if (frag->ulTextLen > 0 && frag->pTextStart) {
                    std::wstring bookmark_text(frag->pTextStart, frag->ulTextLen);
                    DEBUG_LOG("Bookmark text: \"%S\"", bookmark_text.c_str());

                    long bookmark_id = 0;
                    try {
                        bookmark_id = std::stol(bookmark_text);
                    } catch (...) {
                    }

                    SPEVENT event = {};
                    event.eEventId = SPEI_TTS_BOOKMARK;
                    event.elParamType = SPET_LPARAM_IS_STRING;
                    event.ullAudioStreamOffset = ctx.bytes_written;
                    event.ulStreamNum = 0;
                    event.lParam = reinterpret_cast<LPARAM>(bookmark_text.c_str());
                    event.wParam = bookmark_id;
                    HRESULT hr = pOutputSite->AddEvents(&event, 1);
                    DEBUG_LOG("SAPI Event: Bookmark at byte offset %llu, id=%ld, text=\"%S\" - Result: 0x%08X",
                              ctx.bytes_written, bookmark_id, bookmark_text.c_str(), hr);
                } else {
                    DEBUG_LOG("Bookmark has no text, sending with id=0");
                    SPEVENT event = {};
                    event.eEventId = SPEI_TTS_BOOKMARK;
                    event.elParamType = SPET_LPARAM_IS_UNDEFINED;
                    event.ullAudioStreamOffset = ctx.bytes_written;
                    event.ulStreamNum = 0;
                    event.lParam = 0;
                    event.wParam = 0;
                    HRESULT hr = pOutputSite->AddEvents(&event, 1);
                    DEBUG_LOG("SAPI Event: Bookmark (empty) at byte offset %llu - Result: 0x%08X",
                              ctx.bytes_written, hr);
                }
                continue;
            }

            if (frag->State.eAction != SPVA_Speak && frag->State.eAction != SPVA_SpellOut) {
                DEBUG_LOG("Fragment skipped - not Speak or SpellOut action");
                continue;
            }

            if (frag->ulTextLen == 0 || !frag->pTextStart) {
                DEBUG_LOG("Fragment skipped - no text");
                continue;
            }

            const std::string text = utils::wstring_to_string(frag->pTextStart, frag->ulTextLen);
            DEBUG_LOG("Fragment text: \"%s\"", text.c_str());
            if (text.empty()) {
                DEBUG_LOG("Fragment skipped - empty after conversion");
                continue;
            }

            if (send_sentence_events) {
                SPEVENT event = {};
                event.eEventId = SPEI_SENTENCE_BOUNDARY;
                event.elParamType = SPET_LPARAM_IS_UNDEFINED;
                event.ullAudioStreamOffset = ctx.bytes_written;
                event.ulStreamNum = 0;
                event.lParam = frag->ulTextSrcOffset;
                event.wParam = frag->ulTextLen;
                HRESULT hr = pOutputSite->AddEvents(&event, 1);
                DEBUG_LOG("SAPI Event: Sentence boundary at byte offset %llu, position %lu, length %lu - Result: 0x%08X",
                          ctx.bytes_written, frag->ulTextSrcOffset, frag->ulTextLen, hr);
            }

            if (send_word_events) {
                const wchar_t* text_start = frag->pTextStart;
                ULONG text_len = frag->ulTextLen;

                bool in_word = false;
                ULONG word_start = 0;

                for (ULONG i = 0; i <= text_len; ++i) {
                    bool is_word_char = (i < text_len) &&
                                       (iswalnum(text_start[i]) || text_start[i] == L'\'' || text_start[i] == L'-');

                    if (is_word_char && !in_word) {
                        word_start = i;
                        in_word = true;
                    } else if (!is_word_char && in_word) {
                        ULONG word_len = i - word_start;
                        SPEVENT event = {};
                        event.eEventId = SPEI_WORD_BOUNDARY;
                        event.elParamType = SPET_LPARAM_IS_UNDEFINED;
                        event.ullAudioStreamOffset = ctx.bytes_written;
                        event.ulStreamNum = 0;
                        event.lParam = frag->ulTextSrcOffset + word_start;
                        event.wParam = word_len;
                        HRESULT hr = pOutputSite->AddEvents(&event, 1);

                        std::wstring word_text(text_start + word_start, word_len);
                        DEBUG_LOG("SAPI Event: Word boundary at byte offset %llu, position %lu, length %lu (\"%S\") - Result: 0x%08X",
                                  ctx.bytes_written, frag->ulTextSrcOffset + word_start, word_len,
                                  word_text.c_str(), hr);

                        in_word = false;
                    }
                }
            }

            int combined_rate = static_cast<int>(sapi_rate) + frag->State.RateAdj;
            combined_rate = std::clamp(combined_rate, MIN_RATE, MAX_RATE);

            DEBUG_LOG("--- Rate Calculation ---");
            DEBUG_LOG("  RateAdj: %d, Combined Rate: %d", frag->State.RateAdj, combined_rate);

            const float speed_multiplier = std::pow(2.0f, static_cast<float>(combined_rate) / 8.0f);

            DEBUG_LOG("  Speed Multiplier: %.2fx (2^(rate/8))", speed_multiplier);

            int native_rate = 0;
            float sonic_multiplier = 1.0f;

            float desired_native_rate = -100.0f * std::log2(speed_multiplier);

            if (desired_native_rate > NATIVE_RATE_MAX) {
                native_rate = NATIVE_RATE_MAX;
                sonic_multiplier = speed_multiplier / 0.5f;
                DEBUG_LOG("  Mode: VERY SLOW - Native rate maxed at +%d, using Sonic %.2fx", native_rate, sonic_multiplier);
            } else if (desired_native_rate < NATIVE_RATE_MIN) {
                native_rate = NATIVE_RATE_MIN;
                sonic_multiplier = speed_multiplier / 2.0f;
                DEBUG_LOG("  Mode: VERY FAST - Native rate maxed at %d, using Sonic %.2fx", native_rate, sonic_multiplier);
            } else {
                native_rate = static_cast<int>(std::round(desired_native_rate));
                sonic_multiplier = 1.0f;
                DEBUG_LOG("  Mode: NORMAL - Using Native Rate Only (No Sonic)");
                DEBUG_LOG("  Native Rate: %d", native_rate);
            }

            const int frag_gain = gain + static_cast<int>((frag->State.Volume - 100) * 0.5);

            int default_freq = 100;
            if (voice_index_ >= 0 && voice_index_ < VOICE_COUNT) {
                default_freq = VOICE_DEFAULT_FREQS[voice_index_];
            }

            DEBUG_LOG("--- Pitch Calculation ---");
            DEBUG_LOG("  Voice Default Frequency: %d Hz", default_freq);

            const int pitch_val = frag->State.PitchAdj.MiddleAdj;
            DEBUG_LOG("  SAPI Pitch Value: %d", pitch_val);

            int frequency;
            if (pitch_val < 0) {
                const double ratio = (pitch_val + 25) / 25.0;
                frequency = static_cast<int>(std::round(
                    FREQ_MIN * std::pow(static_cast<double>(default_freq) / FREQ_MIN, ratio)
                ));
                DEBUG_LOG("  Mode: LOWER PITCH - Logarithmic scaling from %d to %d Hz", FREQ_MIN, default_freq);
                DEBUG_LOG("  Ratio: %.2f", ratio);
            } else if (pitch_val > 0) {
                const double ratio = pitch_val / 25.0;
                const double curved_ratio = std::pow(ratio, 1.5);
                frequency = static_cast<int>(std::round(
                    default_freq * std::pow(static_cast<double>(FREQ_MAX) / default_freq, curved_ratio)
                ));
                DEBUG_LOG("  Mode: HIGHER PITCH - Logarithmic scaling from %d to %d Hz", default_freq, FREQ_MAX);
                DEBUG_LOG("  Ratio: %.2f, Curved: %.2f", ratio, curved_ratio);
            } else {
                frequency = default_freq;
                DEBUG_LOG("  Mode: NATURAL PITCH - Using voice default");
            }
            DEBUG_LOG("  Final Frequency: %d Hz", frequency);

            DEBUG_LOG("--- Final Values Summary ---");
            DEBUG_LOG("  Native Rate: %d, Sonic: %.2fx, Gain: %d, Frequency: %d Hz",
                      native_rate, sonic_multiplier, frag_gain, frequency);
            DEBUG_LOG("===========================\n");

#ifdef BUILD_X64
            g_pipeClient->speak(
                text.c_str(),
                voice_index_,
                native_rate,
                sonic_multiplier,
                frag_gain,
                frequency,
                speak_callback,
                &ctx
            );
#else
            b32::speak_async(
                bst_state_.get(),
                speak_callback,
                &ctx,
                text.c_str(),
                voice_index_,
                native_rate,
                sonic_multiplier,
                frag_gain,
                frequency
            );
#endif

            if (ctx.aborted) {
                break;
            }
        }

        DEBUG_LOG("=== Speak Completed Successfully ===");
        DEBUG_LOG("=== RETURNING from Speak() - Engine ready for next call ===\n");
        return S_OK;
    }
    catch (const std::bad_alloc&) {
        DEBUG_LOG("=== Speak Failed: Out of memory ===\n");
        return E_OUTOFMEMORY;
    }
    catch (...) {
        DEBUG_LOG("=== Speak Failed: Unexpected exception ===\n");
        return E_UNEXPECTED;
    }
}
}
}
