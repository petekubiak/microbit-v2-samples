#include "MicroBit.h"
#include "MicroBitCompat.h"
#include "SoundEmojiSynthesizer.h"
#include "Synthesizer.h"
#include "SoundSynthesizerEffects.h"

MicroBit uBit;

static void onLight(MicroBitEvent)
{
    uBit.serial.printf("BRIGHT LIGHT!\r\n");
    uBit.audio.soundExpressions.playAsync("002373041050001000392300001023010802050005000000000000000000000000000000");
}

static void onDark(MicroBitEvent)
{
    uBit.serial.printf("Hey who turned out the lights?\r\n");
    uBit.audio.soundExpressions.stop();
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

    fx->duration = 1000;
    fx->tone.tonePrint = Synthesizer::SquareWaveTone;
    fx->frequency = 130.81f;
    fx->volume = 1.0f;

    while(1)
    {
        synth->play(b);
        uBit.sleep(1000);
    }

    microbit_panic( 999 );
}

