/*----------------------------------------------
|                                              |
|  bst_spk 0.1               anthony c bartman |
|                                              |
----------------------------------------------*/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <stdio.h>
#include <cwchar>
#include "argparse.h"

// Setup function types for BST functions
typedef int (__cdecl *initTTSFunc)();
typedef int (__cdecl *deInitTTSFunc)();
typedef int (__cdecl *sayTTSFunc)(wchar_t*);
initTTSFunc   Init_TTS;
deInitTTSFunc DeInit_TTS;
sayTTSFunc    Say_TTS;

int main(int argc, const char* argv[]) {
	// parse arguments using argparse.c
	const char* language = "";
	const char* const usages[] = {"bst_spk [options] \"text\"", nullptr};
	argparse_option options[] = {
		OPT_HELP(),
		OPT_STRING('l', "language", &language, "language to speak with", nullptr, 0, 0),
		OPT_END()
	};
	argparse argparse;
	argparse_init(&argparse, options, usages, 0);
	argparse_describe(&argparse, "\nSpeak using BeSTspeech.", "");
	if (argc > 1) {
		argc = argparse_parse(&argparse, argc, argv);
	} else {
		argparse_usage(&argparse);
		return 1;
	}

	// get text as final argument
	// and convert to the compatible wchar_t text
	const char *text = argv[argc-1];
	size_t length = strlen(text);
	wchar_t text_wchar[length+1] = {0};
	for (int i = 0; i < length; i++) {
		text_wchar[i] = btowc(*text++);
	}

	// select the correct DLL for given language
	// FIX THIS
	HINSTANCE bstLib;
	if (strcmp(language, "arabic") == 0) {
		bstLib = LoadLibrary("dll_ara.dll");
	} else if (strcmp(language, "dutch") == 0) {
		bstLib = LoadLibrary("dll_dut.dll");
	} else if (strcmp(language, "french") == 0) {
		bstLib = LoadLibrary("dll_fre.dll");
	} else if (strcmp(language, "german") == 0) {
		bstLib = LoadLibrary("dll_ger.dll");
	} else if (strcmp(language, "greek") == 0) {
		bstLib = LoadLibrary("dll_gre.dll");
	} else if (strcmp(language, "hebrew") == 0) {
		bstLib = LoadLibrary("dll_heb.dll");
	} else if (strcmp(language, "italian") == 0) {
		bstLib = LoadLibrary("dll_ita.dll");
	} else if (strcmp(language, "japanese") == 0) {
		bstLib = LoadLibrary("dll_jpn.dll");
	} else if (strcmp(language, "polish") == 0) {
		bstLib = LoadLibrary("dll_pol.dll");
	} else if (strcmp(language, "portuguese") == 0) {
		bstLib = LoadLibrary("dll_por.dll");
	} else if (strcmp(language, "russian") == 0) {
		bstLib = LoadLibrary("dll_rus.dll");
	} else if (strcmp(language, "spanish") == 0) {
		bstLib = LoadLibrary("dll_spa.dll");
	} else {
		bstLib = LoadLibrary("dll_eng.dll");
	}
	if (!bstLib) {
		printf("TTS not found!\n");
		return 1;
	}

	// Load up functions
	Init_TTS   = (initTTSFunc)GetProcAddress(bstLib, "Init_TTS");
	DeInit_TTS = (deInitTTSFunc)GetProcAddress(bstLib, "DeInit_TTS");
	Say_TTS    = (sayTTSFunc)GetProcAddress(bstLib, "Say_TTS");

	// Create new TTS instance (no handle variable needed)
	Init_TTS();

	// Do synthesis
	Say_TTS(text_wchar);

	// Close TTS handle
	DeInit_TTS();

	return 0;
}
