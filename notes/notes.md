# Summary
- Goal is to create API for light sensing feature on micro:bit
- Create example to demonstrate new feature (music box)

# Part 1
- Micro:bit uses event-based model for user interaction
- Examples: button presses, accelerometer gestures
- Ask is to create events for sensing light and dark in CODAL
- API should conform to other examples, such that it could be added to the MakeCode and MicroPython APIs which sit on top.
- Use MicroBitDisplay for light sensing: use LEDs in reverse-bias configuration for sensing light level.
**Immediate thought is that light will generate E-H pairs in LED, reducing bandgap and allowing reverse current flow - need to check if we're reading digital output or are pins ADC?**
- Build a "music box" - when we detect light, the box is open, so music should play.
- Mentions to pay attention to how a music box "works and sounds". **Is the micro:bit capable of replicating instrument sounds? Or does this more mean the way it plays a simple melody on repeat?**
- **Video doesn't show it, but music boxes wind down an slow down - maybe a stretch goal to replicate this?**
- Don't need to worry about wind-up mechanism - **would be interesting to do, but leave for now.**

# Part 2 - Describe audio pipeline
- Block diagram of CODAL audio pipeline
- Instantiated as MicroBitAudio in the MicroBit object
- Question: What's the meaning of push/pull
- Question: What is hooked up initially to the mixer?

# Notes
## Links
[Samples fork](https://github.com/petekubiak/microbit-v2-samples)
[CODAL fork](https://github.com/petekubiak/codal-microbit-v2)

## Part 1
- [samples folder](https://github.com/petekubiak/microbit-v2-samples/tree/master/source/samples) contains a number of example programs and tests.
- [ButtonTest.cpp](https://github.com/petekubiak/microbit-v2-samples/blob/master/source/samples/ButtonTest.cpp) shows implementation of button events
- [AccelerometerTest.cpp](https://github.com/petekubiak/microbit-v2-samples/blob/master/source/samples/AccelerometerTest.cpp) shows implementation of accelerometer "shake" event
- `uBit.messageBus.listen()` is the API for registering a callback with a particular event.
- Takes an event ID (Need to track down where this is defined), what seems to be an event *type*, and then a callback function pointer.
**Would be nice to pin down the `MessageBus` dependency, not in the CODAL repo**

There is also a [LightLevelTestRaw](https://github.com/petekubiak/microbit-v2-samples/blob/master/source/samples/LightLevelTestRaw.cpp) sample which looks like it gives the logic for reading the light level.
It looks like it maybe makes all row pins inputs, and all column pins outputs (driving high presumably for reverse bias).
It then sets the row1 pin as an output and pulls low to "drain" it, then counts the rise time in loop cycles (max 1000000)

This functionality is inherently tied to the display, so the display could not be used in its "normal" way if we want to do this kind of sensing (at least not easily).
One option would be to extend the functionality of the MicroBitDisplay to allow initialisation in this mode.
Alternatively one could create a new class for light sensing.
One the one hand, giving the display class responsibilities which are not related to being a display seems maybe a little counter-intuitive, however it is using the LED display in order to perform this feature.

Display does already have an ID for sending events on the MessageBus.

Need to implement light sensing feature, add a new event for "light change", and...
