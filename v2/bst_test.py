# this version of BeSTspeech (for 2008 versions of the Lingvosoft Talking Dictionary programs)
# require the text to be wide char utf-8 text.
#
# requires 32-bit version of Python to run this script

import ctypes as ct
def test_speaklib(dllName, text):
    print(f'Testing \"{dllName}\"...')
    lib = ct.CDLL(dllName)
    lib.Init_TTS()
    lib.Say_TTS(ct.c_wchar_p(text))
    lib.DeInit_TTS()

test_speaklib('dll_eng.dll', 'hello world. this is best speech.')
test_speaklib('dll_spa.dll', 'Hola mundo. Este es el mejor discurso.')
test_speaklib('dll_fre.dll', 'Bonjour le monde. C''est le meilleur discours.')
test_speaklib('dll_ger.dll', 'Hallo Welt. Das ist die beste Rede.')
test_speaklib('dll_ita.dll', 'Ciao mondo. Questo è il miglior discorso.')

#test_speaklib('dll_ara.dll', 'مرحبا يا عالم. هذا أفضل خطاب.')   # not working

test_speaklib('dll_dut.dll', 'Hallo wereld. Dit is de beste toespraak.')
test_speaklib('dll_gre.dll', 'Γεια σου κόσμε. Αυτή είναι η καλύτερη ομιλία.')
test_speaklib('dll_heb.dll', 'שלום עולם. זו הנאום הכי טוב.')
test_speaklib('dll_jpn.dll', 'こんにちは、世界。これが最高のスピーチだ。')
test_speaklib('dll_pol.dll', 'Witaj świecie. To jest najlepsza przemowa.')
test_speaklib('dll_por.dll', 'Olá, mundo. Esse é o melhor discurso.')
test_speaklib('dll_rus.dll', 'Привет, мир. Это лучшая речь.')
