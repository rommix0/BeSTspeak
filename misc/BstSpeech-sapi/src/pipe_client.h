#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include "pipe_protocol.h"

class CriticalLock {
public:
    explicit CriticalLock(CRITICAL_SECTION* cs) noexcept : cs_(cs) {
        EnterCriticalSection(cs_);
    }

    ~CriticalLock() {
        LeaveCriticalSection(cs_);
    }

    CriticalLock(const CriticalLock&) = delete;
    CriticalLock& operator=(const CriticalLock&) = delete;

private:
    CRITICAL_SECTION* cs_;
};

using PipeAudioCallback = bool(*)(const char* data, uint32_t size, void* user);

class PipeClient {
public:
    PipeClient();
    ~PipeClient();

    PipeClient(const PipeClient&) = delete;
    PipeClient& operator=(const PipeClient&) = delete;

    bool connect();

    bool getVoices(std::vector<VoiceInfo>& voices);

    bool speak(const char* text, int voiceIndex, int nativeRate, float sonicMultiplier,
               int gain, int frequency, PipeAudioCallback callback, void* user);

    void shutdownServer();

private:
    bool ensureConnected();
    void disconnect();
    bool isServerRunning();
    bool launchServer();
    bool sendCommand(PipeCommand cmd, const void* data = nullptr, uint32_t size = 0);
    bool readResponse(PipeResponse& resp, std::vector<char>& data);

    HANDLE pipe_;
    HANDLE serverProcess_;
    std::wstring serverPath_;
    CRITICAL_SECTION cs_;
};
