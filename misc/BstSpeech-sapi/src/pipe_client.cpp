#include "pipe_client.h"
#include <shlwapi.h>
#include <cstring>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "advapi32.lib")

namespace {

constexpr wchar_t BESTSPEECH_SERVER_MUTEX[] = L"Global\\BestspeechTTSServerMutex";
constexpr wchar_t BESTSPEECH_LAUNCH_MUTEX[] = L"Global\\BestspeechTTSLaunchMutex";

HMODULE GetCurrentModule() {
    HMODULE hModule = nullptr;
    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&GetCurrentModule),
        &hModule
    );
    return hModule;
}
}

PipeClient::PipeClient()
    : pipe_(INVALID_HANDLE_VALUE)
    , serverProcess_(nullptr)
{
    InitializeCriticalSection(&cs_);

    HKEY hKey;
    if (RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\BestspeechSAPI",
            0, KEY_READ | KEY_WOW64_32KEY, &hKey) == ERROR_SUCCESS) {
        wchar_t path[MAX_PATH];
        DWORD size = sizeof(path);
        if (RegQueryValueExW(hKey, L"InstallLocation", nullptr, nullptr,
                             reinterpret_cast<LPBYTE>(path), &size) == ERROR_SUCCESS) {
            serverPath_ = path;
            if (!serverPath_.empty() && serverPath_.back() != L'\\') {
                serverPath_ += L'\\';
            }
            serverPath_ += L"BestspeechServer.exe";
        }
        RegCloseKey(hKey);
    }

    if (serverPath_.empty() || GetFileAttributesW(serverPath_.c_str()) == INVALID_FILE_ATTRIBUTES) {
        wchar_t dllPath[MAX_PATH];
        if (HMODULE hModule = GetCurrentModule()) {
            GetModuleFileNameW(hModule, dllPath, MAX_PATH);
            PathRemoveFileSpecW(dllPath);

            serverPath_ = std::wstring(dllPath) + L"\\BestspeechServer.exe";

            if (GetFileAttributesW(serverPath_.c_str()) == INVALID_FILE_ATTRIBUTES) {
                PathRemoveFileSpecW(dllPath);
                serverPath_ = std::wstring(dllPath) + L"\\BestspeechServer.exe";
            }
        }
    }
}

PipeClient::~PipeClient() {
    disconnect();
    DeleteCriticalSection(&cs_);
}

bool PipeClient::isServerRunning() {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, BESTSPEECH_SERVER_MUTEX);
    if (hMutex) {
        CloseHandle(hMutex);
        return true;
    }
    return false;
}

bool PipeClient::launchServer() {
    if (isServerRunning()) {
        return true;
    }

    if (GetFileAttributesW(serverPath_.c_str()) == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    HANDLE launchMutex = CreateMutexW(nullptr, FALSE, BESTSPEECH_LAUNCH_MUTEX);
    if (!launchMutex) {
        return false;
    }

    DWORD waitResult = WaitForSingleObject(launchMutex, 5000);
    if (waitResult != WAIT_OBJECT_0 && waitResult != WAIT_ABANDONED) {
        CloseHandle(launchMutex);
        return false;
    }

    if (isServerRunning()) {
        ReleaseMutex(launchMutex);
        CloseHandle(launchMutex);
        return true;
    }

    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi = {};

    bool launched = CreateProcessW(serverPath_.c_str(), nullptr, nullptr, nullptr, FALSE,
                                   CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);

    if (launched) {
        serverProcess_ = pi.hProcess;
        CloseHandle(pi.hThread);

        for (int i = 0; i < 20 && !isServerRunning(); ++i) {
            Sleep(100);
        }
    }

    ReleaseMutex(launchMutex);
    CloseHandle(launchMutex);
    return launched;
}

void PipeClient::disconnect() {
    if (pipe_ != INVALID_HANDLE_VALUE) {
        CloseHandle(pipe_);
        pipe_ = INVALID_HANDLE_VALUE;
    }
}

bool PipeClient::connect() {
    CriticalLock lock(&cs_);
    return ensureConnected();
}

bool PipeClient::ensureConnected() {
    if (pipe_ != INVALID_HANDLE_VALUE) {
        return true;
    }

    for (int attempt = 0; attempt < 5; ++attempt) {
        pipe_ = CreateFileW(
            BESTSPEECH_PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING, 0, nullptr
        );

        if (pipe_ != INVALID_HANDLE_VALUE) {
            DWORD mode = PIPE_READMODE_BYTE;
            SetNamedPipeHandleState(pipe_, &mode, nullptr, nullptr);
            return true;
        }

        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND) {
            if (attempt == 0) {
                launchServer();
            }
            Sleep(300);
        } else if (error == ERROR_PIPE_BUSY) {
            WaitNamedPipeW(BESTSPEECH_PIPE_NAME, 1000);
        } else {
            break;
        }
    }
    return false;
}

bool PipeClient::sendCommand(PipeCommand cmd, const void* data, uint32_t size) {
    if (pipe_ == INVALID_HANDLE_VALUE) {
        return false;
    }

    PipeMessageHeader header = { cmd, size };
    DWORD written;

    if (!WriteFile(pipe_, &header, sizeof(header), &written, nullptr)) {
        return false;
    }

    if (data && size > 0) {
        if (!WriteFile(pipe_, data, size, &written, nullptr)) {
            return false;
        }
    }

    return true;
}

bool PipeClient::readResponse(PipeResponse& resp, std::vector<char>& data) {
    if (pipe_ == INVALID_HANDLE_VALUE) {
        return false;
    }

    PipeMessageHeader header;
    DWORD bytesRead;

    if (!ReadFile(pipe_, &header, sizeof(header), &bytesRead, nullptr) ||
        bytesRead != sizeof(header)) {
        return false;
    }

    resp = static_cast<PipeResponse>(header.type);
    data.clear();

    if (header.size > 0) {
        data.resize(header.size);
        if (!ReadFile(pipe_, data.data(), header.size, &bytesRead, nullptr) ||
            bytesRead != header.size) {
            return false;
        }
    }
    return true;
}

bool PipeClient::getVoices(std::vector<VoiceInfo>& voices) {
    CriticalLock lock(&cs_);
    voices.clear();

    if (!ensureConnected() || !sendCommand(CMD_GET_VOICES)) {
        return false;
    }

    PipeResponse resp;
    std::vector<char> data;
    if (!readResponse(resp, data) || resp != RESP_VOICES ||
        data.size() < sizeof(VoicesResponse)) {
        return false;
    }

    const auto* vr = reinterpret_cast<const VoicesResponse*>(data.data());
    const auto* vi = reinterpret_cast<const VoiceInfo*>(data.data() + sizeof(VoicesResponse));

    for (uint32_t i = 0; i < vr->count; ++i) {
        voices.push_back(vi[i]);
    }

    return true;
}

bool PipeClient::speak(const char* text, int voiceIndex, int nativeRate, float sonicMultiplier,
                       int gain, int frequency, PipeAudioCallback callback, void* user) {
    CriticalLock lock(&cs_);

    auto textLen = static_cast<uint32_t>(strlen(text));
    std::vector<char> payload(sizeof(SpeakCommand) + textLen);
    auto* cmd = reinterpret_cast<SpeakCommand*>(payload.data());
    cmd->voice_index = voiceIndex;
    cmd->native_rate = nativeRate;
    cmd->sonic_multiplier = sonicMultiplier;
    cmd->gain = gain;
    cmd->frequency = frequency;
    cmd->text_length = textLen;
    memcpy(payload.data() + sizeof(SpeakCommand), text, textLen);

    if (!ensureConnected() || !sendCommand(CMD_SPEAK, payload.data(), static_cast<uint32_t>(payload.size()))) {
        disconnect();
        return false;
    }

    bool stopped = false;

    while (true) {
        PipeResponse resp;
        std::vector<char> data;

        if (!readResponse(resp, data)) {
            disconnect();
            return false;
        }

        if (resp == RESP_AUDIO_END) {
            break;
        }

        if (resp == RESP_ERROR) {
            return false;
        }

        if (resp == RESP_OK) {
            continue;
        }

        if (resp == RESP_AUDIO_DATA && data.size() >= sizeof(uint32_t) && !stopped) {
            uint32_t chunkSize = *reinterpret_cast<uint32_t*>(data.data());
            const char* audioData = data.data() + sizeof(uint32_t);

            if (callback && !callback(audioData, chunkSize, user)) {
                sendCommand(CMD_STOP);
                stopped = true;
            }
        }
    }

    return true;
}

void PipeClient::shutdownServer() {
    CriticalLock lock(&cs_);

    if (!isServerRunning()) {
        return;
    }

    if (ensureConnected()) {
        sendCommand(CMD_SHUTDOWN);
        PipeResponse resp;
        std::vector<char> data;
        readResponse(resp, data);
        disconnect();
    }

    for (int i = 0; i < 20 && isServerRunning(); ++i) {
        Sleep(100);
    }
}
