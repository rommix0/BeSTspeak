## This program requires Python 3.9 (32-bit) to work
##
## BeSTspeak v1.01
## Coded by @rommix0 (11/10/2024)
##

import pywintypes, ctypes as ct
import win32gui, win32con, win32api
from argparse import RawTextHelpFormatter
import argparse
lib = ct.CDLL('BST.DLL')
def get_version_info():
    v = ct.c_char_p()
    v_pt = ct.pointer(v)
    res = lib.TtsVersion(0x1001, 0, v_pt)
    version = 'BeSTspeech TTS ' + ct.string_at(v_pt).decode('ascii')
    return f'{version}\nCopyright © 1985, 1991 Berkeley Speech Technologies, Inc.\nPython code by @rommix0 (built on 11/10/2024)\n'


## Process the command line arguments first

parser = argparse.ArgumentParser(
    prog = 'BeSTspeak',
    epilog = 'Synthesizes text to speech using the BeSTspeech (Keynote GOLD) TTS system',
    description = get_version_info(),
    formatter_class=RawTextHelpFormatter
)
parser.add_argument('--text', '-t', required=True, help='text to feed to the synthesizer (required)')
parser.add_argument('--prefix', '-p', default='', help='prefix text to add (like for speech parameters)')
args = parser.parse_args()


## This section is for initializing the invisible Win32 window for BeSTspeech to use.
## This is a requirement for the TTS synthesizer to function and properly make callbacks.

def wndProc(hwnd, msg, wParam, lParam):
    if msg == win32con.WM_DESTROY:
        win32gui.PostQuitMessage(0)
        return 0
    else:
        return win32gui.DefWindowProc(hwnd, msg, wParam, lParam)

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


## Now onto the main section where the TTS action takes place

stat = lib.bstCreateTts(hwnd)
if stat == 0: raise Exception('Can\'t load TTS system!')
def speak_text(text):
    lib.SayBstText(0, ct.c_char_p(text.encode('ascii')))
#speak_text(args.prefix + args.text)

'''
int __cdecl SetParams(__int16 option, __int16 setting)

257 - speed
258 - volume
259 - unvoiced_gain
260 - pitch_freq
'''

speak_text(args.prefix + args.text)
