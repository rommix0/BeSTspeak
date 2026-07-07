'''

Window messages for BST:
------------------------------------------------

955 (3bb)		MM_WOM_OPEN
	wParam -> (WPARAM) hOutputDev				# audio output device handle that was opened
	lParam -> 0									# must be zero

956 (3bc)		MM_WOM_CLOSE
	wParam -> (WPARAM) hOutputDev				# audio output device handle that was closed
	lParam -> 0									# must be zero

957 (3bd)		MM_WOM_DONE (TTS_SYNC)
	wParam -> (WPARAM) hOutputDev				# audio output device handle that played buffer
	lParam -> (LONG) lpwvhdr					# pointer to WAVEHDR structure identifying buffer

typedef struct WAVEHDR {
  LPSTR              lpData;					# pointer to the waveform buffer
  DWORD              dwBufferLength;			# length of buffer in bytes
  DWORD              dwBytesRecorded;			# how much data in buffer when recording
  DWORD_PTR          dwUser;					# user data
  DWORD              dwFlags;					# bitwise flags
  DWORD              dwLoops;					# number of times to play loop
  struct WAVEHDR     *lpNext;					# pointer to next buffer
  DWORD_PTR          reserved;					# reserved
}

Flags:
	WHDR_DONE		1							# buffer is done and completed
	WHDR_PREPARED	2							# buffer is prepared to be played
	WHDR_BEGINLOOP	4							# buffer is first buffer in a loop
	WHDR_ENDLOOP	8							# buffer is last buffer in a loop
	WHDR_INQUEUE	16							# buffer is queued for playback

'''
from ctypes.wintypes import *
import pywintypes, ctypes as ct
import win32gui, win32con, win32api

class WAVEHDR(ct.Structure):
    _fields_ = [("lpData", LPSTR),
                ("dwBufferLength", DWORD),
                ("dwBytesRecorded", DWORD),
                ("dwUser", LPWORD),
                ("dwFlags", DWORD),
                ("dwLoops", DWORD),
                ("lpNext", LPWORD),
                ("reserved", LPWORD)]
wavheader = WAVEHDR()
#whdr_pointer = ct.pointer(wavheader)
isTalking = False
lib = ct.CDLL('BST.DLL')

# This wndProc function closely matches the callback function used by bstCreateTts
def wndProc(hwnd, msg, wParam, lParam):
    if msg == 2:
        if isTalking: lib.bstShutup(tts_handle)
        lib.bstDestroy(tts_handle)
        return win32gui.DefWindowProc(hwnd, msg, wParam, lParam)
    elif msg != 957:
        return win32gui.DefWindowProc(hwnd, msg, wParam, lParam)
    
    #structPointer = WAVEHDR(lParam)
    #print(ct.string_at(structPointer.lpData, structPointer.dwBufferLength))
    
    lib.bstRelBuf(tts_handle)
    return 0

# Create window class
classname = "TTS_CLASS"
wndClass = win32gui.WNDCLASS()
wndClass.lpfnWndProc = wndProc
wndClass.hInstance = win32api.GetModuleHandle(None)
wndClass.lpszClassName = classname
win32gui.RegisterClass(wndClass)

# Create a window and return the HWND for BeSTspeech to use
# The window is never shown. It's a dummy window
hwnd = win32gui.CreateWindow(
    classname,
    "mainwindow",
    win32con.WS_OVERLAPPEDWINDOW,
    win32con.CW_USEDEFAULT,
    win32con.CW_USEDEFAULT,
    win32con.CW_USEDEFAULT,
    win32con.CW_USEDEFAULT,
    None,
    None,
    win32gui.GetModuleHandle(None),
    None
)

# Initialize TTS synthesizer
tts_handle = ct.c_long(0)
tts_handle_ptr = ct.pointer(tts_handle)
stat = lib.bstCreate(tts_handle_ptr)
if stat != 0:
    raise Exception('Can\'t load TTS system!')

# Initialize phoneme buffer stuff
phoneme_buffer = ct.c_char_p()
phbuf_pointer = ct.pointer(phoneme_buffer)

# Main functions for BST (to be updated)
def speak_text(text):
    isTalking = True
    _ = lib.TtsWav(tts_handle, hwnd, ct.c_char_p(text.encode('ascii')))
    isTalking = False
def print_phoneme_buffer(utt):
    utt_to_parse = ct.c_char_p(utt.encode('ascii'))
    lib.GetPhBuf(tts_handle, hwnd, utt_to_parse, len(utt), phbuf_pointer, len(utt) * 64)
    print(ct.string_at(phbuf_pointer).decode('ascii'))
    print()

# begin terminal prompt
while True:
    try:
        prompt = input('> ')
        speak_text(prompt)
        print_phoneme_buffer(prompt)
    except KeyboardInterrupt:
        break
