#include <windows.h>
#include <string>
#include <vector>
#include "pipe_protocol.h"
#include "b32_wrapper.h"

namespace {
constexpr wchar_t BESTSPEECH_SERVER_MUTEX[] = L"Global\\BestspeechTTSServerMutex";

b32::StatePtr g_bst;
HANDLE g_pipe = INVALID_HANDLE_VALUE;
HANDLE g_mutex = nullptr;
bool g_stop_speaking = false;
bool g_shutdown_requested = false;

bool audio_callback(const char* data, long size, void* user) {
    auto pipe = static_cast<HANDLE>(user);

    if (g_stop_speaking) {
        return false;
    }

    PipeMessageHeader header;
    header.type = RESP_AUDIO_DATA;
    header.size = static_cast<uint32_t>(sizeof(uint32_t) + size);

    DWORD written;
    if (!WriteFile(pipe, &header, sizeof(header), &written, nullptr)) {
        return false;
    }

    auto chunk_size = static_cast<uint32_t>(size);
    if (!WriteFile(pipe, &chunk_size, sizeof(chunk_size), &written, nullptr)) {
        return false;
    }

    if (!WriteFile(pipe, data, size, &written, nullptr)) {
        return false;
    }

    return true;
}

void send_response(HANDLE pipe, PipeResponse resp, const void* data = nullptr, uint32_t size = 0) {
    PipeMessageHeader header;
    header.type = resp;
    header.size = size;

    DWORD written;
    WriteFile(pipe, &header, sizeof(header), &written, nullptr);
    if (data && size > 0) {
        WriteFile(pipe, data, size, &written, nullptr);
    }
}

void handle_ping(HANDLE pipe) {
    send_response(pipe, RESP_PONG);
}

void handle_get_voices(HANDLE pipe) {
    int count = 0;
    const char* const* voices = b32::get_voices(&count);

    std::vector<char> buffer;
    buffer.resize(sizeof(VoicesResponse) + static_cast<size_t>(count) * sizeof(VoiceInfo));

    auto* resp = reinterpret_cast<VoicesResponse*>(buffer.data());
    resp->count = static_cast<uint32_t>(count);

    auto* voice_infos = reinterpret_cast<VoiceInfo*>(buffer.data() + sizeof(VoicesResponse));

    constexpr int female_voices[] = {1, 3, 6, 11, 12};

    for (int i = 0; i < count; ++i) {
        memset(voice_infos[i].name, 0, sizeof(voice_infos[i].name));
        strncpy(voice_infos[i].name, voices[i], sizeof(voice_infos[i].name) - 1);

        voice_infos[i].is_female = 0;
        for (int j = 0; j < 5; ++j) {
            if (i == female_voices[j]) {
                voice_infos[i].is_female = 1;
                break;
            }
        }
    }

    send_response(pipe, RESP_VOICES, buffer.data(), static_cast<uint32_t>(buffer.size()));
}

void handle_speak(HANDLE pipe, const char* payload, uint32_t payload_size) {
    if (payload_size < sizeof(SpeakCommand)) {
        send_response(pipe, RESP_ERROR);
        return;
    }

    const auto* cmd = reinterpret_cast<const SpeakCommand*>(payload);

    if (payload_size < sizeof(SpeakCommand) + cmd->text_length) {
        send_response(pipe, RESP_ERROR);
        return;
    }

    std::string text(payload + sizeof(SpeakCommand), cmd->text_length);

    g_stop_speaking = false;

    b32::speak_async(
        g_bst.get(),
        audio_callback,
        pipe,
        text.c_str(),
        cmd->voice_index,
        cmd->native_rate,
        cmd->sonic_multiplier,
        cmd->gain,
        cmd->frequency
    );

    send_response(pipe, RESP_AUDIO_END);
}

void handle_stop(HANDLE pipe) {
    g_stop_speaking = true;
    send_response(pipe, RESP_OK);
}

void handle_client(HANDLE pipe) {
    while (true) {
        PipeMessageHeader header;
        DWORD bytesRead;

        if (!ReadFile(pipe, &header, sizeof(header), &bytesRead, nullptr) || bytesRead != sizeof(header)) {
            break;
        }

        std::vector<char> payload;
        if (header.size > 0) {
            payload.resize(header.size);
            if (!ReadFile(pipe, payload.data(), header.size, &bytesRead, nullptr) || bytesRead != header.size) {
                break;
            }
        }

        switch (static_cast<PipeCommand>(header.type)) {
            case CMD_PING:
                handle_ping(pipe);
                break;
            case CMD_GET_VOICES:
                handle_get_voices(pipe);
                break;
            case CMD_SPEAK:
                handle_speak(pipe, payload.data(), header.size);
                break;
            case CMD_STOP:
                handle_stop(pipe);
                break;
            case CMD_SHUTDOWN:
                send_response(pipe, RESP_OK);
                g_shutdown_requested = true;
                return;
            default:
                send_response(pipe, RESP_ERROR);
                break;
        }
    }
}

}

int main(int /*argc*/, char* /*argv*/[]) {
    g_mutex = CreateMutexW(nullptr, TRUE, BESTSPEECH_SERVER_MUTEX);
    if (!g_mutex) {
        return 1;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(g_mutex);
        return 0;
    }

    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(nullptr, exe_path, MAX_PATH);
    wchar_t* last_slash = wcsrchr(exe_path, L'\\');
    if (last_slash) {
        wcscpy_s(last_slash + 1, MAX_PATH - (last_slash - exe_path + 1), L"b32_tts.dll");
    }

    g_bst = b32::init(exe_path);
    if (!g_bst) {
        g_bst = b32::init(L"b32_tts.dll");
    }

    if (!g_bst) {
        ReleaseMutex(g_mutex);
        CloseHandle(g_mutex);
        return 1;
    }

    while (!g_shutdown_requested) {
        g_pipe = CreateNamedPipeW(
            BESTSPEECH_PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            65536,
            65536,
            0,
            nullptr
        );

        if (g_pipe == INVALID_HANDLE_VALUE) {
            Sleep(1000);
            continue;
        }

        if (ConnectNamedPipe(g_pipe, nullptr) || GetLastError() == ERROR_PIPE_CONNECTED) {
            handle_client(g_pipe);
        }

        DisconnectNamedPipe(g_pipe);
        CloseHandle(g_pipe);
        g_pipe = INVALID_HANDLE_VALUE;
    }

    ReleaseMutex(g_mutex);
    CloseHandle(g_mutex);
    return 0;
}
