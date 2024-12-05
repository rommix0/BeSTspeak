// *************************************************************
// *                                                           *
// *                    BeSTspeak v1.01                        *
// *                                                           *
// *             Programmed by Anthony C. Bartman              *
// *                                                           *
// *************************************************************

#include "stdafx.h"
#include "resource.h"
#include "windows.h"
#include "bst.h"

#define ID_EDITCHILD    100
#define MAX_LOADSTRING  20
#define WM_STARTUP      (WM_USER+0)

#define ID_CONTROLS_VOICECHANGEFORWARD  500
#define ID_CONTROLS_VOICECHANGEBACKWARD 501
#define MOD_NOREPEAT                    0x4000

// Global Variables:
HINSTANCE hInst;							// current instance
TCHAR     szTitle[MAX_LOADSTRING];			// The title bar text
TCHAR     szWindowClass[MAX_LOADSTRING];	// The title bar text
HWND      hWndEdit;                         // hWnd for text box
BOOL      hotkeysEnable = TRUE;             // Need for when the window is in focus

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

// Get current gain
int GetGlobalGain() {
	int *get_gain;
	_bstGetParams(tts_handle, GAIN_SETTING, get_gain);
	return (int)get_gain;
}

// Get current rate
int GetGlobalRate() {
	int *get_rate;
	_bstGetParams(tts_handle, RATE_SETTING, get_rate);
	return (int)get_rate;
}

// Assign text to string using malloc
void AssignToString(char *&string_to_assign, const char *text) {
	string_to_assign = (char *)malloc(strlen(text) + 1);
	strcpy(string_to_assign, text);
}

// Convert int to char string (for pitch adjustment)
void IntToStr(int number) {
	memset(freq_str, 0, freq_str_sz * sizeof(char));

	int n = freq_str_sz;
    freq_str += n;
    *freq_str = '\0';

    while (n--)
    {
        *--freq_str = (number % 10) + '0';
        number /= 10;
    }
}

// Set frequency prefix string
void SetFreqPrefix(char *&prefix_to_assign, int pitch) {
	global_pitch = pitch;
	IntToStr(pitch);

	const char *freq_to_add = freq_str;
	prefix_to_assign = (char *)malloc((freq_str_sz + 3) * sizeof(char));
	strcpy(prefix_to_assign, "~f");
	strcat(prefix_to_assign, freq_to_add);
	strcat(prefix_to_assign, "]");
}

// Set up prefix string with chosen voice parameter
void SetVoice(int voice_id) {
	voice_select = voice_id;
	switch (voice_id) {
		case FRED:
			AssignToString(prefix, V_FRED);
			AssignToString(prefix_rate, V_FRED_R);
			SetFreqPrefix(prefix_freq, V_FRED_F);
			break;
		case SARAH:
			AssignToString(prefix, V_SARAH);
			AssignToString(prefix_rate, V_SARAH_R);
			SetFreqPrefix(prefix_freq, V_SARAH_F);
			break;
		case HARRY:
			AssignToString(prefix, V_HARRY);
			AssignToString(prefix_rate, V_HARRY_R);
			SetFreqPrefix(prefix_freq, V_HARRY_F);
			break;
		case WENDY:
			AssignToString(prefix, V_WENDY);
			AssignToString(prefix_rate, V_WENDY_R);
			SetFreqPrefix(prefix_freq, V_WENDY_F);
			break;
		case DEXTER:
			AssignToString(prefix, V_DEXTER);
			AssignToString(prefix_rate, V_DEXTER_R);
			SetFreqPrefix(prefix_freq, V_DEXTER_F);
			break;
		case ALIEN:
			AssignToString(prefix, V_ALIEN);
			AssignToString(prefix_rate, V_ALIEN_R);
			SetFreqPrefix(prefix_freq, V_ALIEN_F);
			break;
		case KIT:
			AssignToString(prefix, V_KIT);
			AssignToString(prefix_rate, V_KIT_R);
			SetFreqPrefix(prefix_freq, V_KIT_F);
			break;
		case BRUNO:
			AssignToString(prefix, V_BRUNO);
			AssignToString(prefix_rate, V_BRUNO_R);
			SetFreqPrefix(prefix_freq, V_BRUNO_F);
			break;
		case GHOST:
			AssignToString(prefix, V_GHOST);
			AssignToString(prefix_rate, V_GHOST_R);
			SetFreqPrefix(prefix_rate, V_GHOST_F);
			break;
		case PEEPER:
			AssignToString(prefix, V_PEEPER);
			AssignToString(prefix_rate, V_PEEPER_R);
			SetFreqPrefix(prefix_freq, V_PEEPER_F);
			break;
		case DRACULA:
			AssignToString(prefix, V_DRACULA);
			AssignToString(prefix_rate, V_DRACULA_R);
			SetFreqPrefix(prefix_freq, V_DRACULA_F);
			break;
		case GRANNY:
			AssignToString(prefix, V_GRANNY);
			AssignToString(prefix_rate, V_GRANNY_R);
			SetFreqPrefix(prefix_freq, V_GRANNY_F);
			break;
		case MARTHA:
			AssignToString(prefix, V_MARTHA);
			AssignToString(prefix_rate, V_MARTHA_R);
			SetFreqPrefix(prefix_freq, V_MARTHA_F);
			break;
		case TIM:
			AssignToString(prefix, V_TIM);
			AssignToString(prefix_rate, V_TIM_R);
			SetFreqPrefix(prefix_freq, V_TIM_F);
			break;
		default:
			AssignToString(prefix, V_FRED);
			AssignToString(prefix_rate, V_FRED_R);
			SetFreqPrefix(prefix_freq, V_FRED_F);
			voice_select = FRED;
	}
	_bstSetParams(tts_handle, GAIN_SETTING, -5);
}

// Increase voice speed
void IncreaseRate() {
	int global_rate = GetGlobalRate() - RATE_ADJUST;
	if (global_rate < -100) {
		global_rate = -100;
	}
	_bstSetParams(tts_handle, RATE_SETTING, global_rate);
}

// Decrease voice speed
void DecreaseRate() {
	int global_rate = GetGlobalRate() + RATE_ADJUST;
	_bstSetParams(tts_handle, RATE_SETTING, global_rate);
}

// Increase voice gain
void IncreaseGain() {
	int global_gain = GetGlobalGain() + GAIN_ADJUST;
	if (global_gain > 0) {
		global_gain = 0;
	}
	_bstSetParams(tts_handle, GAIN_SETTING, global_gain);
}

// Decrease voice gain
void DecreaseGain() {
	int global_gain = GetGlobalGain() - GAIN_ADJUST;
	if (global_gain < -35) {
		global_gain = -35;
	}
	_bstSetParams(tts_handle, GAIN_SETTING, global_gain);
}

// Increase pitch
void IncreasePitch() {
	global_pitch += PITCH_ADJUST;
	if (global_pitch > 600) {
		global_pitch = 600;
	}
	SetFreqPrefix(prefix_freq, global_pitch);
}

// Decrease pitch
void DecreasePitch() {
	global_pitch -= PITCH_ADJUST;
	if (global_pitch < 45) {
		global_pitch = 45;
	}
	SetFreqPrefix(prefix_freq, global_pitch);
}

// Function for initializing/reset speech synthesizer
int InitSpeech() {
	int current_gain  = 0;
	int current_rate  = 0;
	if (tts_handle) {
		current_gain  = GetGlobalGain();
		current_rate  = GetGlobalRate();
		_bstShutup(tts_handle);
		_bstClose(tts_handle);
		_bstDestroy();
		tts_handle = 0;
	}
	if (!tts_handle) {
		int stat = _bstCreate(tts_handle);
		if (stat != 0) {
			MessageBox(NULL, "TTS cannot be initialized!", "Init Error!", MB_ICONERROR);
			return 0;
		}
		_bstSetParams(tts_handle, GAIN_SETTING, current_gain);
		_bstSetParams(tts_handle, RATE_SETTING, current_rate);
		_bstSetParams(tts_handle, BIT_DEPTH_SETTING, 16);      // always set this for 16-bit audio
	}
	return 1;
}

// Close speech engine if present (when closing program)
void CloseSpeech() {
	if (tts_handle) {
		_bstShutup(tts_handle);
		_bstClose(tts_handle);
		_bstDestroy();
		tts_handle = 0;
	}
}

// Function for speaking text
void SpeakText(HWND hWnd, const char *text, bool init = FALSE) {
	int _ret = InitSpeech();
	if (_ret) {
		// Add prefix strings to text using malloc and string operations
		char       *text_to_speak;
		const char *p_prefix = prefix;
		const char *r_prefix = prefix_rate;
		const char *f_prefix = prefix_freq;
		if (init) {
			_bstSetParams(tts_handle, RATE_SETTING, 0);
			text_to_speak = (char *)malloc(strlen(p_prefix) + strlen(r_prefix) + strlen(f_prefix) + strlen(text) + 1);
			strcpy(text_to_speak, p_prefix);
			strcat(text_to_speak, f_prefix);
			strcat(text_to_speak, text);
		}
		else {
			text_to_speak = (char *)malloc(strlen(p_prefix) + strlen(f_prefix) + strlen(text) + 1);
			strcpy(text_to_speak, p_prefix);
			strcat(text_to_speak, f_prefix);
			strcat(text_to_speak, text);
		}


		// Speak the text
		_TtsWav(tts_handle, hWnd, text_to_speak);
		free(text_to_speak);
	}
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// Check if BST.DLL is loaded into memory
	if (!bstLib) {
		MessageBox(NULL, "TTS system not found!", "TTS Engine Error!", MB_ICONERROR);
		return 1;
	}

	// Load the important BST functions
	_bstCreate    = (bstCreateFunc)GetProcAddress(bstLib, "bstCreate");
	_TtsWav       = (TtsWavFunc)GetProcAddress(bstLib, "TtsWav");
	_bstRelBuf    = (bstRelBufFunc)GetProcAddress(bstLib, "bstRelBuf");
	_bstShutup    = (bstShutupFunc)GetProcAddress(bstLib, "bstShutup");
	_bstDestroy   = (bstDestroyFunc)GetProcAddress(bstLib, "bstDestroy");
	_bstClose     = (bstCloseFunc)GetProcAddress(bstLib, "bstClose");
	_bstSetParams = (bstSetParamsFunc)GetProcAddress(bstLib, "bstSetParams");
	_bstGetParams = (bstGetParamsFunc)GetProcAddress(bstLib, "bstGetParams");

	// Initialize speech synthesizer
	int stat = InitSpeech();
	if (!stat) {
		return 1;
	}
	SetVoice(FRED);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_BESTSPEAK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	HWND hWnd;
	MSG msg;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		                CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Register hotkeys for responsive keyboard shortcuts
	RegisterHotKey(hWnd, IDM_EXIT,                        MOD_NOREPEAT,               VK_ESCAPE);
	RegisterHotKey(hWnd, IDM_HELP,                        MOD_NOREPEAT,               VK_F1);
	RegisterHotKey(hWnd, IDM_ABOUT,                       MOD_NOREPEAT,               VK_F2);
	RegisterHotKey(hWnd, ID_CONTROLS_RESET,               MOD_NOREPEAT,               VK_F3);
	RegisterHotKey(hWnd, ID_CONTROLS_SHUTUP,              MOD_NOREPEAT,               VK_F4);
	RegisterHotKey(hWnd, ID_CONTROLS_SPEAKTEXT,           MOD_NOREPEAT,               VK_F5);

	RegisterHotKey(hWnd, ID_CONTROLS_DECREASEVOLUME,      MOD_CONTROL | MOD_NOREPEAT, VK_F6);
	RegisterHotKey(hWnd, ID_CONTROLS_INCREASEVOLUME,      MOD_NOREPEAT,               VK_F6);

	RegisterHotKey(hWnd, ID_CONTROLS_SLOWERSPEED,         MOD_NOREPEAT,               VK_F7);
	RegisterHotKey(hWnd, ID_CONTROLS_FASTERSPEED,         MOD_NOREPEAT,               VK_F8);
	RegisterHotKey(hWnd, ID_CONTROLS_PITCHDOWN,           MOD_NOREPEAT,               VK_F9);
	RegisterHotKey(hWnd, ID_CONTROLS_PITCHUP,             MOD_NOREPEAT,               VK_F10);

	RegisterHotKey(hWnd, ID_CONTROLS_VOICECHANGEBACKWARD, MOD_CONTROL | MOD_NOREPEAT, VK_F11);
	RegisterHotKey(hWnd, ID_CONTROLS_VOICECHANGEFORWARD,  MOD_NOREPEAT,               VK_F11);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_BESTSPEAK);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_BESTSPEAK;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}


// Main callback function where the majority of functions happen
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndEdit; // static keyword is important for textbox fill to work.
	HWND   hwnd_text;
	char  *read_buf;
	int    wmId;
	int    len;

	switch (message) 
	{
		case WM_CREATE:
			hwndEdit = CreateWindow(
				"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
				0, 0, 0, 0,
				hWnd,
				(HMENU) ID_EDITCHILD,
				(HINSTANCE) GetWindowLong(hWnd, -6),
				NULL);
			PostMessage(hWnd, WM_STARTUP, wParam, lParam); // Send message to announce TTS version after window is open.
			break;
		case WM_HOTKEY:
			// Parse hotkeys for responsive actions
			if (hotkeysEnable) {
				wmId = LOWORD(wParam);
				switch (wmId)
				{
					case ID_CONTROLS_VOICECHANGEBACKWARD:
						voice_select -= 1;
						if (voice_select < FRED) {
							voice_select = DRACULA;
						}
						PostMessage(hWnd, WM_COMMAND, ID_VOICES_FRED + voice_select, lParam);
						break;
					case ID_CONTROLS_VOICECHANGEFORWARD:
						voice_select += 1;
						if (voice_select > DRACULA) {
							voice_select = FRED;
						}
						PostMessage(hWnd, WM_COMMAND, ID_VOICES_FRED + voice_select, lParam);
						break;
					default:
						PostMessage(hWnd, WM_COMMAND, wmId, lParam);
				}
			}
			break;
		case WM_COMMAND:
			// Parse the menu selections:
			switch (LOWORD(wParam))
			{
				case IDM_ABOUT:
					SpeakText(hWnd, about_text);
					break;
				case IDM_HELP:
					SpeakText(hWnd, help_text);
					SpeakText(hWnd, "End of help!");
					break;
				case IDM_EXIT:
					SpeakText(hWnd, "Exiting.");
					DestroyWindow(hWnd);
					break;
				case ID_CONTROLS_SPEAKTEXT:
					// Grab text from EDIT control and allocate memory for it for the synthesizer to speak it.
					hwnd_text = GetDlgItem(hWnd, ID_EDITCHILD);
					len = GetWindowTextLength(hwnd_text) + 1;
					read_buf = (char *)malloc(len);
					GetWindowText(hwnd_text, read_buf, len);
					
					// Speak the text
					SpeakText(hWnd, read_buf);
					free(read_buf);
					break;
				case ID_CONTROLS_SHUTUP:
					SpeakText(hWnd, "");
					break;
				case ID_CONTROLS_RESET:
					SetVoice(FRED);
					SpeakText(hWnd, "Reset.", TRUE);
					break;
				case ID_CONTROLS_INCREASEVOLUME:
					IncreaseGain();
					SpeakText(hWnd, "Louder.");
					break;
				case ID_CONTROLS_REDUCEVOLUME:
					DecreaseGain();
					SpeakText(hWnd, "Quieter.");
					break;
				case ID_CONTROLS_FASTERSPEED:
					IncreaseRate();
					SpeakText(hWnd, "Faster.");
					break;
				case ID_CONTROLS_SLOWERSPEED:
					DecreaseRate();
					SpeakText(hWnd, "Slower.");
					break;
				case ID_CONTROLS_PITCHUP:
					IncreasePitch();
					SpeakText(hWnd, "Higher.");
					break;
				case ID_CONTROLS_PITCHDOWN:
					DecreasePitch();
					SpeakText(hWnd, "Lower.");
					break;
				case ID_VOICES_FRED:
					SetVoice(FRED);
					SpeakText(hWnd, "Fred", TRUE);
					break;
				case ID_VOICES_SARAH:
					SetVoice(SARAH);
					SpeakText(hWnd, "Sarah", TRUE);
					break;
				case ID_VOICES_HARRY:
					SetVoice(HARRY);
					SpeakText(hWnd, "Harry", TRUE);
					break;
				case ID_VOICES_MARTHA:
					SetVoice(MARTHA);
					SpeakText(hWnd, "Martha", TRUE);
					break;
				case ID_VOICES_TIM:
					SetVoice(TIM);
					SpeakText(hWnd, "Tim", TRUE);
					break;
				case ID_VOICES_DEXTER:
					SetVoice(DEXTER);
					SpeakText(hWnd, "Dexter", TRUE);
					break;
				case ID_VOICES_ALIEN:
					SetVoice(ALIEN);
					SpeakText(hWnd, "Alien", TRUE);
					break;
				case ID_VOICES_KIT:
					SetVoice(KIT);
					SpeakText(hWnd, "Kit", TRUE);
					break;
				case ID_VOICES_WENDY:
					SetVoice(WENDY);
					SpeakText(hWnd, "Wendy", TRUE);
					break;
				case ID_VOICES_BRUNO:
					SetVoice(BRUNO);
					SpeakText(hWnd, "Bruno", TRUE);
					break;
				case ID_VOICES_GRANNY:
					SetVoice(GRANNY);
					SpeakText(hWnd, "Granny", TRUE);
					break;
				case ID_VOICES_GHOST:
					SetVoice(GHOST);
					SpeakText(hWnd, "Ghost", TRUE);
					break;
				case ID_VOICES_PEEPER:
					SetVoice(PEEPER);
					SpeakText(hWnd, "Peeper", TRUE);
					break;
				case ID_VOICES_DRACULA:
					SetVoice(DRACULA);
					SpeakText(hWnd, "Dracula", TRUE);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
			break;
		case WM_SETFOCUS:
			SetFocus(hwndEdit);
			break;
		case WM_ACTIVATE:
			// Turn hotkeys on or off based on window focus
			// This is needed to prevent hotkey conflicts when out of focus
			if (LOWORD(wParam) == WA_INACTIVE) {
				hotkeysEnable = FALSE;
			} else {
				hotkeysEnable = TRUE;
			}
			break;
		case WM_SIZE:
			MoveWindow(hwndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
			break;
		case WM_DESTROY:
			CloseSpeech();
			FreeLibrary(bstLib);
			PostQuitMessage(0);
			break;
		case WM_STARTUP:
			SpeakText(hWnd, version_text, TRUE);
			break;
		case TTS_BUFFER_FULL:
			_bstRelBuf(tts_handle);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}
