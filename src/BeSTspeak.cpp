// BeSTspeak.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "windows.h"
#include "bst.h"

#define ID_EDITCHILD  100
#define MAX_LOADSTRING 20
#define WM_STARTUP (WM_USER+0)

// Global Variables:
HINSTANCE hInst;								                           // current instance
TCHAR     szTitle[MAX_LOADSTRING];								           // The title bar text
TCHAR     szWindowClass[MAX_LOADSTRING];								   // The title bar text
HWND      hWndEdit;                                                        // hWnd for text box

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

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

// Set up prefix string with chosen voice parameter
void SetVoice(int voice_id) {
	free(prefix);
	free(prefix_init);
	switch (voice_id) {
		case FRED:
			prefix = (char *)malloc(strlen(V_FRED) + 1);
			strcpy(prefix, V_FRED);
			prefix_init = (char *)malloc(strlen(V_FRED_X) + 1);
			strcpy(prefix_init, V_FRED_X);
			break;
		case SARAH:
			prefix = (char *)malloc(strlen(V_SARAH) + 1);
			strcpy(prefix, V_SARAH);
			prefix_init = (char *)malloc(strlen(V_SARAH_X) + 1);
			strcpy(prefix_init, V_SARAH_X);
			break;
		case HARRY:
			prefix = (char *)malloc(strlen(V_HARRY) + 1);
			strcpy(prefix, V_HARRY);
			prefix_init = (char *)malloc(strlen(V_HARRY_X) + 1);
			strcpy(prefix_init, V_HARRY_X);
			break;
		case WENDY:
			prefix = (char *)malloc(strlen(V_WENDY) + 1);
			strcpy(prefix, V_WENDY);
			prefix_init = (char *)malloc(strlen(V_WENDY_X) + 1);
			strcpy(prefix_init, V_WENDY_X);
			break;
		case DEXTER:
			prefix = (char *)malloc(strlen(V_DEXTER) + 1);
			strcpy(prefix, V_DEXTER);
			prefix_init = (char *)malloc(strlen(V_DEXTER_X) + 1);
			strcpy(prefix_init, V_DEXTER_X);
			break;
		case ALIEN:
			prefix = (char *)malloc(strlen(V_ALIEN) + 1);
			strcpy(prefix, V_ALIEN);
			prefix_init = (char *)malloc(strlen(V_ALIEN_X) + 1);
			strcpy(prefix_init, V_ALIEN_X);
			break;
		case KIT:
			prefix = (char *)malloc(strlen(V_KIT) + 1);
			strcpy(prefix, V_KIT);
			prefix_init = (char *)malloc(strlen(V_KIT_X) + 1);
			strcpy(prefix_init, V_KIT_X);
			break;
		case BRUNO:
			prefix = (char *)malloc(strlen(V_BRUNO) + 1);
			strcpy(prefix, V_BRUNO);
			prefix_init = (char *)malloc(strlen(V_BRUNO_X) + 1);
			strcpy(prefix_init, V_BRUNO_X);
			break;
		case GHOST:
			prefix = (char *)malloc(strlen(V_GHOST) + 1);
			strcpy(prefix, V_GHOST);
			prefix_init = (char *)malloc(strlen(V_GHOST_X) + 1);
			strcpy(prefix_init, V_GHOST_X);
			break;
		case PEEPER:
			prefix = (char *)malloc(strlen(V_PEEPER) + 1);
			strcpy(prefix, V_PEEPER);
			prefix_init = (char *)malloc(strlen(V_PEEPER_X) + 1);
			strcpy(prefix_init, V_PEEPER_X);
			break;
		case DRACULA:
			prefix = (char *)malloc(strlen(V_DRACULA) + 1);
			strcpy(prefix, V_DRACULA);
			prefix_init = (char *)malloc(strlen(V_DRACULA_X) + 1);
			strcpy(prefix_init, V_DRACULA_X);
			break;
		case GRANNY:
			prefix = (char *)malloc(strlen(V_GRANNY) + 1);
			strcpy(prefix, V_GRANNY);
			prefix_init = (char *)malloc(strlen(V_GRANNY_X) + 1);
			strcpy(prefix_init, V_GRANNY_X);
			break;
		case MARTHA:
			prefix = (char *)malloc(strlen(V_MARTHA) + 1);
			strcpy(prefix, V_MARTHA);
			prefix_init = (char *)malloc(strlen(V_MARTHA_X) + 1);
			strcpy(prefix_init, V_MARTHA_X);
			break;
		case TIM:
			prefix = (char *)malloc(strlen(V_TIM) + 1);
			strcpy(prefix, V_TIM);
			prefix_init = (char *)malloc(strlen(V_TIM_X) + 1);
			strcpy(prefix_init, V_TIM_X);
			break;
		default:
			prefix = (char *)malloc(strlen(V_FRED) + 1);
			strcpy(prefix, V_FRED);
			prefix_init = (char *)malloc(strlen(V_FRED_X) + 1);
			strcpy(prefix_init, V_FRED_X);
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
	if (global_gain > -5) {
		global_gain = -5;
	}
	_bstSetParams(tts_handle, GAIN_SETTING, global_gain);
}

// Decrease voice gain
void DecreaseGain() {
	int global_gain = GetGlobalGain() - GAIN_ADJUST;
	if (global_gain < -55) {
		global_gain = -55;
	}
	_bstSetParams(tts_handle, GAIN_SETTING, global_gain);
}

// Function for initializing/reset speech synthesizer
int InitSpeech() {
	int current_gain  = -5;
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
		char *text_to_speak;
		const char *pprefix = prefix;
		const char *fprefix = prefix_init;
		if (init) {
			_bstSetParams(tts_handle, RATE_SETTING, 0);
			text_to_speak = (char *)malloc(strlen(pprefix) + strlen(fprefix) + strlen(text) + 1);
			strcpy(text_to_speak, pprefix);
			strcat(text_to_speak, fprefix);
			strcat(text_to_speak, text);
		}
		else {
			text_to_speak = (char *)malloc(strlen(pprefix) + strlen(text) + 1);
			strcpy(text_to_speak, pprefix);
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
 	// Load the typical variables for window handling
	MSG msg;
	HACCEL hAccelTable;

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
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)SPEAKACCEL);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
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

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndEdit; // static keyword is important for textbox fill to work.
	HWND hwnd_text;
	int wmId, wmEvent;
	char *read_buf;
	int len;

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
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case ID_HELP_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_EXIT:
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
		case WM_SIZE:
			MoveWindow(hwndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
			break;
		case WM_DESTROY:
			CloseSpeech();
			free(prefix);
			free(prefix_init);
			FreeLibrary(bstLib);
			PostQuitMessage(0);
			break;
		case WM_STARTUP:
			SpeakText(hWnd, "Best speak version one point zero is running.");
			break;
		case IS_STILL_TALKING:
			_bstRelBuf(tts_handle);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}
