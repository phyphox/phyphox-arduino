# User input

Elements the user operates on the phone. Each writes into an **input channel** (1…16) on the
board, chosen with `setChannel(k)` or the constructor's last argument. The board reads a
channel with `PhyphoxBLE::read(...)`, `PhyphoxBLE::readChannel(k)` or `PhyphoxBLE::changed(k)`,
or attaches a callback: `element.onChange(fn)`, `button.onPress(fn)`,
`PhyphoxBLE::onChange(k, fn)`. Callbacks are plain functions (a captureless lambda works):

```cpp
float blinkInterval = 100;
bool ledOn = true;

PhyphoxBleExperiment::Slider interval("Interval", 50, 1000, 50, /*channel*/ 1);
interval.setUnit("ms"); interval.setDefault(100);
interval.onChange([](float v) { blinkInterval = v; });

PhyphoxBleExperiment::Toggle led("LED", 2);
led.setDefault(1);
led.onChange([](float v) { ledOn = v > 0.5f; });

PhyphoxBleExperiment::Dropdown mode("Mode", 3);
mode.addOption("Always on", 1); mode.addOption("Blink", 2); mode.addOption("Flash", 3);

PhyphoxBleExperiment::Button zero("Set zero", 4);
zero.onPress([]() { offset = lastReading; });
```

Two elements may share a channel — an edit field and a slider for the same value, say. A
button owns its channel.

## Only changes arrive

The phone sends a channel when its value changes, not continuously. Before the first change
`read()` returns the element's default. A button press always arrives, even if the value is
the same as last time. Values flow only while the experiment is running (▶ in the app) — an
edit made while paused reaches the board when the user starts again.
`experiment.setResendUnchanged(true)` restores the 1.x behaviour, where the phone writes every
channel on every analysis cycle (about a hundred times per second); useful as a heartbeat from
the phone, and for a sketch written against 1.x that relied on it.

## Elements

| element | constructor | setters |
|---|---|---|
| `Edit` | `Edit(label, channel)` | `setDefault(v)`, `setUnit`, `setSigned(b)`, `setDecimal(b)`, `setMin(v)`, `setMax(v)`, `setFactor(f)`, `submitWith(button)` |
| `Slider` | `Slider(label, min, max, step, channel)` | `setMinValue`, `setMaxValue`, `setStepSize`, `setDefault`, `setPrecision(n)`, `setShowValue(b)`, `setColor` |
| `Dropdown` | `Dropdown(label, channel)` | `addOption(text, value)` (up to 8) or `setOptions(n, labels, values)` (any number), `setDefault(value)`, `setColor` |
| `Toggle` | `Toggle(label, channel)` | `setDefault(0 \| 1)` |
| `Button` | `Button(label, channel)` | `setValue(v)` — what a press writes, default 1 — and `onPress(fn)` |

All have `setLabel`, `setChannel`, `onChange` (not meaningful for a button), `setVisibility`
and `setXMLAttribute`.

## A form with a submit button

`edit.submitWith(button)` delivers the field only when that button is pressed:

```cpp
PhyphoxBleExperiment::Button apply("Apply", 5);
PhyphoxBleExperiment::Edit setpoint("Setpoint", 6);
setpoint.submitWith(apply);
setpoint.onChange([](float v) { target = v; });   // fires once per press, with the field's value
```

Set the button's channel before calling `submitWith`.

## From the remote interface

phyphox's remote access (the web interface and its API) can write these channels too: an edit
field corresponds to `cmd=set`, a button to `cmd=trigger`. The board sees no difference.
