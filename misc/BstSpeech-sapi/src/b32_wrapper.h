#pragma once

#include <windows.h>
#include <memory>

namespace b32 {

struct State;

struct StateDeleter {
    void operator()(State* s) const noexcept;
};

using StatePtr = std::unique_ptr<State, StateDeleter>;

using AsyncCallback = bool(*)(const char* data, long size, void* user);

[[nodiscard]] const char* const* get_voices(int* count = nullptr) noexcept;

[[nodiscard]] StatePtr init(const char* module_path = "b32_tts.dll") noexcept;
[[nodiscard]] StatePtr init(const wchar_t* module_path = L"b32_tts.dll") noexcept;

void speak_async(
    State* state,
    AsyncCallback callback,
    void* user,
    const char* text,
    int voice = 0,
    int native_rate = 0,        // Native b32_tts rate (-100 to 100)
    float sonic_multiplier = 1.0f, // Sonic speed multiplier (only for extreme speeds)
    int gain = 0,
    int frequency = -1  // -1 means use voice default
) noexcept;
void set_hinstance(HINSTANCE h) noexcept;
}
