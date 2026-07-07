from ctypes import *
lib = CDLL('BST.DLL')
@CFUNCTYPE(None)
def _callback():
    print('the TTS system made a callback!')
def print_version_info():
    v = c_char_p()
    v_pt = pointer(v)
    res = lib.TtsVersion(0x1001, 0, v_pt)
    version = 'BeSTspeech TTS ' + string_at(v_pt).decode('ascii')
    print(f'\n{version}\nCopyright 1986-1995 Berkeley Speech Technologies')
    print('Python code by @rommix (built on 11/2/2024)\n')

instance = c_long(0)
inst_pnt = pointer(instance)
stat = lib.bstCreate(inst_pnt)
if stat != 0: raise Exception('Couldn\'t initialize TTS system...')

print_version_info()

test = '~f80]~h100]this A P I thing is frickin working!'
#test = 'testing 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20'
pass_str = c_char_p(test.encode('ascii'))
stat = lib.TtsWav(instance, _callback(), pass_str)

lib.bstShutup(instance)
lib.bstClose(instance)
lib.bstDestroy(instance)
