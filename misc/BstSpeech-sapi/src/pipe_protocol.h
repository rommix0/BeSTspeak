#pragma once

#include <stdint.h>

#define BESTSPEECH_PIPE_NAME L"\\\\.\\pipe\\BestspeechTTS"

enum PipeCommand : uint32_t {
    CMD_PING = 0,
    CMD_GET_VOICES = 1,
    CMD_SPEAK = 2,
    CMD_STOP = 3,
    CMD_SHUTDOWN = 4,
};

enum PipeResponse : uint32_t {
    RESP_OK = 0,
    RESP_ERROR = 1,
    RESP_AUDIO_DATA = 2,
    RESP_AUDIO_END = 3,
    RESP_VOICES = 4,
    RESP_PONG = 5,
};

#pragma pack(push, 1)
struct PipeMessageHeader {
    uint32_t type;
    uint32_t size;
};

struct SpeakCommand {
    int32_t voice_index;
    int32_t native_rate;
    float sonic_multiplier;
    int32_t gain;
    int32_t frequency;
    uint32_t text_length;
};

struct VoiceInfo {
    char name[32];
    uint8_t is_female;
};

struct VoicesResponse {
    uint32_t count;
};

struct AudioDataChunk {
    uint32_t size;
};
#pragma pack(pop)

#define BESTSPEECH_SAMPLE_RATE 11025
#define BESTSPEECH_BITS_PER_SAMPLE 16
#define BESTSPEECH_CHANNELS 1
