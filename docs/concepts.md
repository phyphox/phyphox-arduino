# Concepts

## What the phone receives

phyphox does not have a built-in "Arduino mode". Everything it shows for a device is described
by an experiment configuration — an XML document in the phyphox file format — that lists data
sources, how to draw them and what the user can control. This library **generates that
document on the board** from your `PhyphoxBleExperiment` and sends it over Bluetooth when the
phone connects. You never see the XML, but `PhyphoxBLE::printXML(&Serial)` shows it if you are
curious or something does not load.

## Channels

- **Data channels 1…5** are what the board sends: `PhyphoxBLE::write(a, b, c)` fills channels
  1, 2, 3 in one Bluetooth notification. **Channel 0** is the time at which the phone received
  the notification. A graph plots one channel against another: `graph.setChannel(0, 1)` is
  channel 1 over time.
- **Input channels 1…8** are what the phone sends: every `Edit`, `Slider`, `Dropdown`, `Toggle`
  or `Button` writes into the input channel you give it with `setChannel(k)`. The board reads
  them with `PhyphoxBLE::read(...)`, `PhyphoxBLE::readChannel(k)`, or through a callback
  (`element.onChange(...)`, `button.onPress(...)`). Two elements may share a channel (an edit
  field and a slider for the same value, say); a button owns its channel.
- **Phone sensors** land in input channels too: `sensor.mapChannel("x", 1)`.

## Only changes are sent

The phone sends an input channel's value when it changes, not continuously. That is a change
from 1.x, where every value was written on every analysis cycle — see
[Migrating](migrating-from-1x.md). `experiment.setResendUnchanged(true)` restores the old
behaviour if a sketch wants a periodic heartbeat from the phone. The board's `read()` returns
each element's default before the first change, and callbacks fire once per delivered value.

Values flow only while the experiment is running (▶) in the app, exactly as in 1.x.

## Bandwidth

One notification carries five floats (20 bytes). For more data per second use a larger MTU
(`PhyphoxBLE::setMTU(176)` before `start()`) together with the array write
`PhyphoxBLE::write(floatArray, n)` and `experiment.setRepeating(samplesPerPacket)`, so the phone
unpacks several samples per notification. iOS always negotiates the largest MTU; Android honours
the request.

## The transfer

The experiment document is sent in packets on the phyphox experiment characteristic as soon as
the phone subscribes to it (or asks for it on the control characteristic). The board sends as
fast as its Bluetooth stack accepts and retries a packet the stack refuses — no fixed pacing.
On a slow or noisy link the transfer takes longer but arrives complete; only a disconnect
aborts it, and then the app offers to retry. Details in [Protocol](protocol.md).

## Events

The app tells the board when the experiment starts, pauses, is cleared, and sends a time
sync when it connects. `PhyphoxBLE::experimentEventHandler` is called with
`PhyphoxBLE::eventType`, `experimentTime` (ms since start) and `systemTime` (Unix ms) set —
see `examples/getSystemAndEventTime`.

## Memory

There are no capacity limits: as many views, elements, curves, options and sensors as the
board's RAM holds. `addExperiment()` measures the description, allocates **one** block of
exactly that size and copies everything into it; the block lives until the sketch ends (a
second `addExperiment()` replaces it). Nothing else touches the heap. The XML document is
**never** assembled in memory: it is streamed packet by packet from the description, so a large
experiment costs flash, not RAM. Rough budget on a 32-bit board: about 170 bytes per view
element, so a typical experiment with a few graphs and inputs needs one to two kilobytes.
The only bounded things are bounded by nature: input channels up to 16 (each is a Bluetooth
characteristic), six components per phone sensor, strings up to 250 characters.

## Strings

Labels, units, titles and option texts are **stored by pointer, not copied**. Pass string
literals (`"Voltage"`) or static/global buffers. A temporary — `String(...).c_str()`, a local
`char buffer[]` in `setup()` — is gone by the time the phone connects and the phone would see
garbage. (1.x copied strings onto the heap; 2.0 does not, so the same sketch uses less RAM.)

## Errors

A setter that gets something the format cannot express (a colour that is not six hex digits, a
channel out of range, a style that does not exist) records an error. The experiment still
loads, with a red **ERROR FOUND: ERR_0x, in setFoo()** line in an extra view, and
`PhyphoxBLE::printErrors(&Serial)` lists the same. Codes: see [API reference](api.md#errors).
