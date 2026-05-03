#include "MicroBit.h"
// #include "samples/Tests.h"

MicroBit uBit;

static void onLight(MicroBitEvent)
{
    uBit.serial.printf("BRIGHT LIGHT!");
}

static void onDark(MicroBitEvent)
{
    uBit.serial.printf("Hey who turned out the lights?");
}

int main()
{
    uBit.init();

    uBit.display.setDisplayMode(DisplayMode::DISPLAY_MODE_BLACK_AND_WHITE_LIGHT_SENSE);

    uBit.messageBus.listen(MICROBIT_ID_DISPLAY, MICROBIT_DISPLAY_EVT_LIGHT_LEVEL_HIGH, onLight);
    uBit.messageBus.listen(MICROBIT_ID_DISPLAY, MICROBIT_DISPLAY_EVT_LIGHT_LEVEL_DARK, onDark);

    while(1)
    {
        uBit.sleep(1000);
    }

    microbit_panic( 999 );
}

