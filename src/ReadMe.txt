BeSTspeak speak window
@rommix0

This is a very straight forward text to speech speak window program.

It is meant to be easy for people with visual impairments to use and otherwise.

The parameters are derived from both the Keynote GOLD developers manual and from reverse engineering of B32_TTS.DLL.

Here are the hot keys:
------------------------------------------------------------------------------------------------------------------------------

ESC					Exit program
F1					Help
F2					Program credits
F3					Reset synthesizer
F4					Stop speaking
F5					Speak text
Ctrl+F6				Decrease volume
F6					Increase volume
F7					Decrease speaking rate
F8					Increase speaking rate
F9					Decrease pitch
F10					Increase pitch
Ctrl+F11			Change voice backwards
F11					Change voice forwards


Speech Parameters
------------------------------------------------------------------------------------------------------------------------------

~|					Commence speaking previous text (only really needed for phoneme mode).

~~2,1]				Read immediately without waiting for punctuation.

~c{dec}]			Change the lead-in character (dec is ascii number of character)
						By default, BeSTspeech for this version uses ~ (126) as the lead-in character.
						In most Keynote GOLD hardware systems (including old BrailleNotes), [ (91) is the lead-in character.

~r{percentage}]		Percentage of normal speed, range -100 to infinity.
						lower is faster, higher is slower

~e{num}]			Voice excitation setting from 1 to 6.
						breathy = 1, whispery = 2, normal = 3

~g{num}]			Voice gain setting from -70 to 20.

~u{num}]			Unvoiced gain setting from -70 to 20.
						turning it down makes the speaker sound like their nose is plugged up.

~f{freq}]			baseline frequency {freq} in Hz, range 43 to 600

~h{num}]			Alters pitch range of voice. {num} in range -300 to 100
						lower is more monotone. higher is more expressive.

~v{num}]			Alters head size of speaker from 0 to 6.
						head sizes are predetermined in no particular order.


Text Parser Parameters   "x" is 0 (off) or 1 (on)
------------------------------------------------------------------------------------------------------------------------------

~					Homograph mark. Place immediately ahead of word to select alternate pronunciation
						(e.g. minute  ~minute, read  ~read)

~n1,x]				Pronounce names of letters (spelling mode)
						default is off

~n2,x]				Pronounce digits individually.
						default is off

~n3,x]				Pronounce commas, periods, etc.
						default is off

~n4,x]				Pronounce space, carriage return, tab.
						default is off

~n5,x]				Read mathematical texts.
						default is off

~n6,x]				Inhibit grouping of digits, read full numbers.
						default is off

~n7,x]				Pronounce uppercase letter groups as words.
						default is off

~n8,x]				Pronounce control characters.
						default is off

~n9,x]				Pronounce times of day. (e.g. 8:00 = eight o'clock)
						default is on

~n10,x]				abbreviation expansion
						default is on

~z1,x]				Field reset pronunciation for personal name
						(e.g., DR becomes doctor)

~z3,x]				Field reset pronunciation for organization name

~z4,x]				Field reset pronunciation for street address (room number, floor, apartment etc.)
						(e.g., NE becomes north east, and DR becomes drive)

~z5,x]				Field reset pronunciation for city name

~z6,x]				Field reset pronunciation for state or province, zip code
						(e.g., NE becomes Nebraska)

~z7,x]				Field reset pronunciation for nation

~z]					restores all fields to default.


Prosody
------------------------------------------------------------------------------------------------------------------------------

]					comma-like pause
}					period-like pause
~+					Stress the following word
~-					Remove stress from the following word
~!					Emphasize the following word (de-stress the rest of the sentence)
~? 					Produce rising intonation at the end of the sentence like asking a question.

~I{num},{some_index}]	inflection marker

	possibly used for sentence inflection.

Stress markers from lowest to highest($):
	$1	$2	`	$4	"	'	$7	$8
	
	$2 is default for unstressed
	' is primary stress
	" is secondary stress
	" #4 is default for secondary stress


Pitch contour markers from lowest to highest (#):
	#-4		to		#11
	
	#6 is default contour

	used for pitch targeting syllables and prosodic boundaries.

	On syllables, pitch markers immediately follow the stress
	marker.  If a syllable has no stress marker, the pitch target
	immediately follows the vowel phoneme for the syllable.  Pitch
	targets on boundaries immediately follow the boundary marker.

	Unmarked prosodic boundaries receive a default pitch target.
	However, unlike stress, there is no default pitch marking for
	syllables.  The actual pitch levels for unmarked syllables are
	interpolated from surrounding pitch targets.

	Boundaries can be marked with as many as two pitch targets.  Two
	pitch targets should appear on boundaries that have words both
	to the right and to the left.  The first target will be the
	final pitch level for the words that precede the boundary and
	the second target will be the pitch onset for the words that
	follow the boundary.  The initial and final "$C" of a text
	should each have only one pitch target.

	Syllables can also have up to two pitch targets.  However,
	usually a single target is sufficient.  Two pitch targets are
	permitted on stressed syllables only to allow for very rapid
	rises and falls in pitch.


Mode switching
------------------------------------------------------------------------------------------------------------------------------

~t]					Text mode    (reads text)
					
					this is default behavior.

~p]					Phoneme mode (reads phoneme codes)

~x]					Dictionary mode

					Used for entering custom pronunciations of a word into the
					synthesizer's user dictionary (UD) residing in memory.
					
					Example of how to use this mode (always use ~t] at the end):
						~x] XD ~p] $C ~I4,19460] l ae ' f I $2 ng $W E $2 m o ' jh i $2 sl $C ; ~t]


Boundaries and Silence
------------------------------------------------------------------------------------------------------------------------------

$W					Word boundary
$C					Major prosodic boundary
$P					Minor prosodic boundary
$S					some unknown prosodic boundary (TBD)
$A					some unknown prosodic boundary (TBD)
$D					some unknown prosodic boundary (TBD)

sl					silence (about 80 milliseconds long)

<>					padding (generated by letter-to-sound engine)
@					skip word utterance (only works when phonemes are enclosed with $W tags)
					use is not recommended.
;					end of utterance (comes after a prosodic boundary)


Consonant phonemes:
------------------------------------------------------------------------------------------------------------------------------

w					wet
y					yacht
h					hot
m					sum
n					sun
ng					sung
l	&	=l			lots (=l is unstressed l)
r					rots
f					fat
v					vat
th					thesis
dh					other
s					sue
z					zoo
ch					chin
jh					judge
sh					shin
zh					measure
b					bats
p	&	p0			pats (p0 is unstressed p)
d					door
t	&	t0			time (t0 is unstressed t)
g					got
k	&	k0			cot  (k0 is unstressed k)


Vowel phonemes:
------------------------------------------------------------------------------------------------------------------------------

i					beet
I					bit
e					bait
E					bet
ae					bat
u					boot
U					put
o					boat
O					bought
a					pot
^					but
R					lurk
ay					bite
Oy					boy
aw					cow
=					unstressed vowel
