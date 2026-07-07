#include <windows.h>
extern "C"
__declspec(dllimport) int TtsWav(int instance, HWND window_handle, char* text);
__declspec(dllimport) int bstCreate(int instance);
__declspec(dllimport) int bstShutup(int instance);
__declspec(dllimport) int bstClose(int instance);
__declspec(dllimport) int bstDestroy(int instance);
