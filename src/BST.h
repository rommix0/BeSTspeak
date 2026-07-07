// Define global variables for BST synthesizer
// @rommix0 (11/28/2024)

// Voice IDs
#define FRED    0
#define SARAH   1
#define HARRY   2
#define WENDY   3
#define DEXTER  4
#define ALIEN   5
#define KIT     6
#define BRUNO   7
#define GHOST   8
#define PEEPER  9
#define DRACULA 10
#define GRANNY  11
#define MARTHA  12
#define TIM     13

// Voices parameters
//-----------------------------------------------------------
// FRED
const char *V_FRED      = "~v0]~e3]~h0]~u0]~f80]";
const char *V_FRED_X    = "~r0]";

// SARAH
const char *V_SARAH     = "~v2]~e3]~h-20]~u0]~f175]";
const char *V_SARAH_X   = "~r0]";

// HARRY
const char *V_HARRY     = "~v3]~e3]~h10]~u0]~f65]";
const char *V_HARRY_X   = "~r5]";

// WENDY
const char *V_WENDY     = "~v2]~e1]~h50]~u0]~f150]";
const char *V_WENDY_X   = "~r-5]";

// DEXTER
const char *V_DEXTER    = "~v6]~e6]~h0]~u-25]~f90]";
const char *V_DEXTER_X  = "~r7]";

// ALIEN
const char *V_ALIEN     = "~v4]~e6]~h-50]~u-20]~f115]";
const char *V_ALIEN_X   = "~r-20]";

// KIT
const char *V_KIT       = "~v5]~e3]~h40]~u0]~f230]";
const char *V_KIT_X     = "~r-10]";

// BRUNO
const char *V_BRUNO     = "~v3]~e3]~h50]~u0]~f60]";
const char *V_BRUNO_X   = "~r8]";

// GHOST
const char *V_GHOST     = "~v3]~e2]~h50]~u0]~f60]";
const char *V_GHOST_X   = "~r8]";

// PEEPER
const char *V_PEEPER    = "~v2]~e2]~h0]~u5]~f80]";
const char *V_PEEPER_X  = "~r0]";

// DRACULA
const char *V_DRACULA   = "~v3]~e3]~h45]~u-5]~f47]";
const char *V_DRACULA_X = "~r10]";

// GRANNY
const char *V_GRANNY    = "~v4]~e3]~h-60]~u0]~f350]";
const char *V_GRANNY_X  = "~r20]";

// MARTHA
const char *V_MARTHA    = "~v6]~e4]~h100]~u-5]~f300]";
const char *V_MARTHA_X  = "~r-10]";

// TIM
const char *V_TIM       = "~v3]~e4]~h-10]~u0]~f60]";
const char *V_TIM_X     = "~r-10]";
//-----------------------------------------------------------

// Used in WndProc callback when synthesizer is talking
#define IS_STILL_TALKING 957

// Settings for BST parameters
#define RATE_SETTING  257
#define GAIN_SETTING  258
#define PITCH_SETTING 260

// Settings adjust amounts
#define RATE_ADJUST  25
#define GAIN_ADJUST  10
#define PITCH_ADJUST 5

// Setup function types for BST functions
typedef int  (__cdecl *bstCreateFunc)(long*&);
typedef int  (__cdecl *TtsWavFunc)(long*, HWND, char*);
typedef void (__cdecl *bstRelBufFunc)(long*);
typedef void (__cdecl *bstShutupFunc)(long*);
typedef void (__cdecl *bstCloseFunc)(long*);
typedef void (__cdecl *bstDestroyFunc)();
typedef void (__cdecl *bstSetParamsFunc)(long*, int, int);
typedef void (__cdecl *bstGetParamsFunc)(long*, int, int*&);
TtsWavFunc       _TtsWav;
bstCreateFunc    _bstCreate;
bstRelBufFunc    _bstRelBuf;
bstShutupFunc    _bstShutup;
bstDestroyFunc   _bstDestroy;
bstCloseFunc     _bstClose;
bstSetParamsFunc _bstSetParams;
bstGetParamsFunc _bstGetParams;

// Load library and setup uninitialized variables (except tts_handle which is initialized to 0)
HINSTANCE  bstLib = LoadLibrary("BST.DLL"); // The speech synthesizer itself.
char      *prefix;                          // Prefix string for the voice parameters to reside.
char      *prefix_init;                     // Prefix string for pitch and rate (concat at end of main prefix)
long      *tts_handle  = 0;                 // The handle where the synthesizer will reside.
