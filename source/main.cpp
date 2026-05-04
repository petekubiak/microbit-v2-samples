#include "MicroBit.h"
#include "MicroBitCompat.h"
#include "SoundEmojiSynthesizer.h"
#include "Synthesizer.h"
#include "SoundSynthesizerEffects.h"

#define SIL 0.0f
#define FS4 369.99f
#define G4  392.0f
#define A4  440.0f
#define AS4 466.16f
#define B4  493.88f
#define CS5 554.37f
#define D5  587.33f
#define E5  659.25f
#define FS5 739.99f
#define B6  1975.53f
#define D7  2349.32f
#define FS7 2959.96f
#define G7  3135.96f
#define A7  3520.0f
#define AS7 3729.31f
#define B7  3951.07f

const int beat = 200;

const float melody[] = {
    FS7, G7, FS7, D7, FS7, SIL, SIL, SIL, SIL, SIL,
    FS7, G7, FS7, D7, FS7, SIL, SIL, SIL, SIL, SIL,
    FS7, G7, FS7, D7, FS7, SIL, SIL, SIL, SIL, SIL,
    FS7, G7, FS7, D7, B6, SIL, SIL, SIL, SIL, SIL,
    FS7, G7, FS7, D7, FS7, SIL, SIL, SIL, SIL, SIL,
    FS7, G7, A7, G7, FS7, SIL, SIL, SIL, SIL, SIL,
    FS7, G7, FS7, D7, FS7, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, AS7, SIL, B7, 
    SIL
};

const float accompaniment[] = {
    SIL, SIL, SIL, SIL, B4, D5, FS5, D5, B4, SIL,
    SIL, SIL, SIL, SIL, AS4, D5, FS5, D5, AS4, SIL,
    SIL, SIL, SIL, SIL, A4, D5, FS4, D5, A4, SIL,
    SIL, SIL, SIL, SIL, G4, B4, E5, B4, G4, SIL,
    SIL, SIL, SIL, SIL, B4, D5, FS5, D5, B4, SIL,
    SIL, SIL, SIL, SIL, FS4, AS4, CS5, AS4, FS4, SIL,
    SIL, SIL, SIL, SIL, B4, D5, FS5, D5, B4, SIL, FS5, SIL,
    FS4, AS4, CS5, AS4, FS4, SIL, AS4, SIL, B4,
    SIL
};

const int durations[] = {
    beat, beat, beat, beat, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2,
    beat, beat, beat, beat, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2,
    beat, beat, beat, beat, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2,
    beat, beat, beat, beat, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2,
    beat, beat, beat, beat, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2,
    beat, beat, beat, beat, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2,
    beat, beat, beat, beat, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2,
    beat * 2, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2, beat * 2,
    beat * 12
};

MicroBit uBit;

static bool musicBoxOpen = false;

static void onLight(MicroBitEvent)
{
    uBit.serial.printf("BRIGHT LIGHT!\r\n");
    musicBoxOpen = true;
}

static void onDark(MicroBitEvent)
{
    uBit.serial.printf("Hey who turned out the lights?\r\n");
    musicBoxOpen = false;
}

int main()
{
    uBit.init();

    uBit.display.setDisplayMode(DisplayMode::DISPLAY_MODE_BLACK_AND_WHITE_LIGHT_SENSE);

    uBit.messageBus.listen(MICROBIT_ID_DISPLAY, MICROBIT_DISPLAY_EVT_LIGHT_LEVEL_HIGH, onLight);
    uBit.messageBus.listen(MICROBIT_ID_DISPLAY, MICROBIT_DISPLAY_EVT_LIGHT_LEVEL_LOW, onDark);

    SoundEmojiSynthesizer * melodySynth = new SoundEmojiSynthesizer(5);
    uBit.audio.mixer.addChannel(*melodySynth);

    ManagedBuffer melodyBuffer(sizeof(SoundEffect));
    SoundEffect *melodyFx = (SoundEffect *)&melodyBuffer[0];

    melodyFx->tone.tonePrint = Synthesizer::SquareWaveTone;
    melodyFx->volume = 1.0f;

    SoundEmojiSynthesizer * accSynth = new SoundEmojiSynthesizer(6);
    uBit.audio.mixer.addChannel(*accSynth);

    ManagedBuffer accBuffer(sizeof(SoundEffect));
    SoundEffect *accFx = (SoundEffect *)&accBuffer[0];

    accFx->tone.tonePrint = Synthesizer::SquareWaveTone;
    accFx->volume = 1.0f;

    int counter = 0;
    int counterMax = sizeof(melody) / sizeof(melody[0]);

    while(1)
    {
        if (musicBoxOpen)
        {
            // Avoid "playing" the silences as it interferes with the other part
            if (melody[counter] != SIL)
            {
                melodyFx->duration = durations[counter];
                melodyFx->frequency = melody[counter];
                melodySynth->play(melodyBuffer);
            }

            if (accompaniment[counter] != SIL)
            {
                accFx->duration = durations[counter];
                accFx->frequency = accompaniment[counter];
                accSynth->play(accBuffer);
            }

            uBit.sleep(durations[counter]);
            counter = (counter + 1) % counterMax;
        }
        else
        {
            uBit.sleep(100);
        }
    }

    microbit_panic( 999 );
}

