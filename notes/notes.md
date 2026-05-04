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

The NRF52LEDMatrix class already has the ability to do light sensing, so the light sensing itself does not need to be implemented, just the event hooks to allow "on_light" and "on_dark".

Light sensing appears to be performed as part of the NRF52LedMatrix's `render()` function.
It seems like each time `render()` is called, one row is "rendered".
Once all rows have been rendered, light sensing is performed.

Events are the method by which different parts of the system notify one another.
By default, the event constructor fires the event immediately on whatever the default EventModel is.
On the Micro:bit, this is the MessageBus.

It is noted in the Event docstrings that the default "CREATE_AND_FIRE" launch mode is not suitable for interrupts (likely due to mutex locking).

I'm thinking that my approach should be to add a check when the NRF52LedMatrix's `lightLevel` field is updated, to check its value against a threshold.
If the threshold has been crossed, the relevant event should fire, which can then be hooked to a callback.
I'll need to define the events (looks like these are defined in codal-core) so that they can be referenced in the LedMatrix's code.
I have a funny feeling that the `lightLevel` update happens in an IRQ - need to check this.
Might be a good idea to add a debounce to the thresholding to avoid noise near the threshold from causing rapid-fire events.
This is a stretch goal though.

There's also the issue of the threshold.
This is quite arbitrary, as the raw value scale used is simply 0 - 255.
Maybe use a default of 128, but allow the user to set a custom threshold?

`MicroBitCompat.h` seems to mainly be to map Micro:bit specific codes to CODAL codes.
There is reference to a `DISPLAY_EVT_LIGHT_SENSE` in here, however I can't find any definition of this in `codal-core`.
There is however a `LED_MATRIX_EVT_LIGHT_SENSE`, which is fired by `LEDMatrix::renderWithLightSense()`.
This however doesn't show up anywhere else, so maybe is unused?
NRF52LEDMatrix doesn't inherit from LEDMatrix (although LEDMatrix.h is included), so that can probably be ignored.

I have been contemplating where to define the events, in order to fit into the predefined structure of the codebase.
I think that defining my new events in NRF52LEDMatrix.h, including this in `MicroBitCompat.h` and providing Micro:bit versions most closely follows the prior.

As suspected, the `render()` function is called by `display_irq()`.
This made me think that I should investigate how `Event`s are sent elsewhere.
First I searched for `Events` created with the `CREATE_ONLY` flag, however I couldn't find any examples in `codal_core` or `codal_microbit_v2`.
Looking through the `send()` logic of the `MessageBus`, it appears that this may actually be a non-issue - the comments say that event processing is "simply queued" until scheduled in a normal thread context.
This is specifically done to avoid executing event handler code in an IRQ.

It looks like unless the listener has been flagged as `URGENT`, events are simply added to a linked list inside a critical section.
As far as I can tell, this means that firing an event from within `render()` shouldn't be able to cause a deadlock, unless ain `URGENT` listener was assigned to it which caused some sort of blocking behaviour.
I may have read this wrong, and it's something to bring up in the discussion.

It would potentially be possible to mitigate the possibility of a user attaching an `URGENT` listener to events generated by the IRQ by using the currently unused `LIGHT_SENSE` event to notify that a new light reading has been taken, internally attaching a listener to that with a lower priority, then having that listener do the threshold comparison and fire the "on_light" and "on_dark" events for the user to attach listeners to.
This may be overkill, but again would be interesting to discuss.

I've set up a switch statement in my new light sensing listener to determine which event to fire when the result of the light level evaluation changes.
The way the logic is written, the `levelEvaluation` variable should only ever be able to be in the `LIGHT` or `DARK` state.
I have added a case for `UNKNOWN` and also a `default`, however these should be unreachable.
I've written a comment to this effect in the code, however sometimes in the past I have used a mechanism whereby I can flag to myself or another developer that this state has been reached (ideally without affecting the user experience).
I'd be interested to know if such a mechanism exists in the CODAL codebase, however investigating this is a stretch goal.

While looking through some of the examples I came across the `LevelDetector` and `LevelDetectorSPL` (which judging by the context is likely Sound Pressure Level).
It looks like the LevelDetector is designed for audio levels, however I imagine this could be generalised and used for light levels too.
I will stick with my implementation, however it would be a point of interest to discuss this alternative route which could be pursued.
It would seem preferable to have a single implementation of the thresholding logic, maybe with some more specific types inheriting from the generalised verison.

Running an initial little test I hit a problem which kept me busy for a few minutes.
I had assigned my new events the IDs 1 and 2, which turned out to already be being used in the display's "event space". This led to my callbacks being called constantly.
There didn't seem to be a central location for mapping IDs, which means that clashes like these are easy to cause.
I eventually picked 4 and 5 arbitrarily, which seemed to work, but of course this could cause bugs in future if a new feature used these without realising they were already taken.

