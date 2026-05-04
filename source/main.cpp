#include "MicroBit.h"
#include "MicroBitCompat.h"
#include "SoundEmojiSynthesizer.h"
#include "Synthesizer.h"
#include "SoundSynthesizerEffects.h"

#define SIL 0.0f
#define B5  987.77f
#define D6  1174.66f
#define FS6 1479.98f
#define G6  1567.98f
#define A6  1760.0f

const int beat = 200;

const float melody[] = {
    FS6, G6, FS6, D6, FS6, SIL,
    FS6, G6, FS6, D6, FS6, SIL,
    FS6, G6, FS6, D6, FS6, SIL,
    FS6, G6, FS6, D6, B5, SIL,
    FS6, G6, FS6, D6, FS6, SIL,
    FS6, G6, A6, G6, FS6, SIL,
    FS6, G6, FS6, D6, FS6, SIL,
    SIL
};

const int durations[] = {
    beat, beat, beat, beat, beat * 4, beat * 8,
    beat, beat, beat, beat, beat * 4, beat * 8,
    beat, beat, beat, beat, beat * 4, beat * 8,
    beat, beat, beat, beat, beat * 4, beat * 8,
    beat, beat, beat, beat, beat * 4, beat * 8,
    beat, beat, beat, beat, beat * 4, beat * 8,
    beat, beat, beat, beat, beat * 4, beat * 8,
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

    SoundEmojiSynthesizer * synth = new SoundEmojiSynthesizer(5);
    uBit.audio.mixer.addChannel(*synth);

    ManagedBuffer b(sizeof(SoundEffect));
    SoundEffect *fx = (SoundEffect *)&b[0];

    fx->tone.tonePrint = Synthesizer::SquareWaveTone;
    fx->volume = 1.0f;

    int counter = 0;
    int counterMax = sizeof(melody) / sizeof(melody[0]);

    while(1)
    {
        if (musicBoxOpen)
        {
            fx->duration = durations[counter];
            fx->frequency = melody[counter];
            synth->play(b);
            counter = (counter + 1) % counterMax;
        }
        else
        {
            uBit.sleep(100);
        }
    }

    microbit_panic( 999 );
}

