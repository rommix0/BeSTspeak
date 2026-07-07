BeSTspeak speak window
@rommix0

This is a very straight forward text to speech speak window program.

It is meant to be easy for people with visual impairments to use and otherwise.

The parameters are derived from both the Keynote GOLD developers manual and from partial reverse engineering of B32_TTS.DLL.

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


Mode switching
------------------------------------------------------------------------------------------------------------------------------

~t]					Normal reading mode (text mode)

~p]					Following text read according to phoneme rules (phoneme mode)

~x]					Begin dictionary entering mode (dictionary mode).

						This mode is used to connect any desired phonemic pronunciation
						with a designated English letter sequence (normally a word) in
						the RAM resident User Dictionary (UD). After a dictionary entry
						has been created, the specified phonemic pronunciation is used
						every time the word appears in the input text. Using the UD
						entries can be pronounced completely differently from the way
						they appear in the text, providing a method for automatic
						expansion of special abbreviations.

						Entries are added to the User Dictionary by associating an
						English spelling with its phonemic transcription. To add an
						entry, type both the word and the phonemic transcription of how
						you want it pronounced in the following form:

							~x]	places BeSTspeech in Dictionary entering mode.

							ssss	is the English spelling, typed without spaces.

							p p p p	is its pronunciation in phonemes, with each
						symbol			preceded by a space.

							~t]	returns to Text Reading Mode.

						Each word that you enter must be preceded by a separate "~x]". A
						return to text reading mode is only required after the last
						entry has been entered, but "~t]" and a carriage return may be
						placed at the end of each entry.


Boundaries and Silence
------------------------------------------------------------------------------------------------------------------------------

$W					Word boundary
$C					Primary stress
$P					Secondary stress

sl					silence (about 80 milliseconds long)


Consonant phonemes:
------------------------------------------------------------------------------------------------------------------------------

w					wet
y					yacht
h					hot
m					sum
n					sun
ng					sung
l					lots
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
p					pats
d					door
t					time
g					got
k					cot


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


Additional notes for phonemes:
------------------------------------------------------------------------------------------------------------------------------

Any of the vowel phonemes listed above for stressed syllables
can appear in unstressed syllables as well.  For example, the
final syllable of "lucky" has the same vowel phoneme ("i") as
"keep".  Those forms starred (*) contain vowels which are
conventionally considered to be lax due to the following "r".
There is an additional vowel phoneme ("=") that only appears in
unstressed syllables.

	=	---	canal (1st syllable)	support (1st syllable)

			action (2nd syllable)	tickle (2nd syllable)

Stress symbols in words are placed after the vowel of the
stressed syllable.  Primary stress is a single quote ';
secondary stress is double quotes ".  Boundaries between words
in multiword transcriptions are marked by the symbol $W.

Stress marks and phoneme symbols must always be preceded by a
space.  If a phoneme symbol is made up of two characters ("sh")
they must be kept together.  For example:

	quiche			k i ' sh

	pizza			p i ' t s =

	fettuchine 		f E " t = ch i ' n i

	three bedroom	~x]3BR th r i " $W b E ' d r u m ~t]


Precisely Specifying Stress and Pitch (taken directly from Keynote GOLD developers manual. It may be a little off for B32_TTS)
------------------------------------------------------------------------------------------------------------------------------

It is possible to fine-tune the intonation contours of a
phonemically transcribed passage by the use of stress and pitch
markers.  The best way to learn to use these is by experimenting
and listening carefully to the result.

Stress is indicated by a dollar sign ($).  Pitch is indicated by
a pound sign (œ).  These signs are followed by a digit that
indicates the level of stress or pitch.  Higher numbers indicate
higher levels.

As noted earlier, primary stress in words can be be marked by
placing the symbol ' after the vowel of the most-stressed
syllable in a word.  Using the ' symbol has the same effect as
using the stress level indicator $6.

The stress and pitch levels for secondary word stress " are: $5
œ4.  The default stress level is $2 (for unstressed syllables).

When transcribing a full text, stress and pitch markers may be
used to specify utterance-level intonation, not just word-level
stress.  A full range of stress markers (from $8 to $1) is
available in phoneme-reading mode, giving you the abilitv to
transcribe a wide variety of stress patterns:

	$8	highest stress level

	$6	equivalent to primary word stress

	$5	equivalent to secondary word stress

	$2	default (unstressed) level

	$1	lowest stress level

Stress markers mainly affect the duration and amplitude of
syllables.  The marker must immediately follow the vowel of the
syllable it marks.  Unmarked vowel phonemes are assigned default
stress.

Pitch markers are also used to specify the intonation contour of
an utterance.  So that many different kinds of contours can be
specified, a wide range of pitch targets is made available, from
œ10 to œ3:

	œ10	highest

	œ6	pitch target for primary stressed syllables

	œ-3	lowest

Pitch targets are associated with:

(1)	Syllables, and

(2)	"$C" and "$P" prosodic boundary markers.

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
