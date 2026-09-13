# User input

*Skeleton — filled in during phase 3 with the ported examples.*

Elements the user operates on the phone. Each writes into an **input channel** on the board.

```cpp
PhyphoxBleExperiment::Slider interval("Interval", 50, 1000, 50, /*channel*/ 1);
interval.setUnit("ms"); interval.setDefault(100);
interval.onChange([](float v) { blinkInterval = v; });

PhyphoxBleExperiment::Toggle led("LED", 2);
PhyphoxBleExperiment::Dropdown mode("Mode", 3);
mode.addOption("Always on", 1); mode.addOption("Blink", 2);

PhyphoxBleExperiment::Button zero("Set zero", 4);
zero.onPress([]() { offset = lastReading; });
```

Reading without callbacks: `PhyphoxBLE::read(a, b, c)` gives channels 1…3;
`PhyphoxBLE::readChannel(k)` any channel; `PhyphoxBLE::changed(k)` is true once per new value.
The 1.x `PhyphoxBLE::configHandler` still fires after any delivery.

| element | setters |
|---|---|
| `Edit` | `setDefault`, `setUnit`, `setSigned`, `setDecimal`, `setMin`, `setMax`, `setFactor`, `submitWith(button)` |
| `Slider` | `setMinValue`, `setMaxValue`, `setStepSize`, `setDefault`, `setPrecision`, `setShowValue`, `setColor` |
| `Dropdown` | `addOption(text, value)` or `setOptions(n, labels, values)`, `setDefault`, `setColor` |
| `Toggle` | `setDefault` (0 off, 1 on) |
| `Button` | `setValue` (what a press writes, default 1), `onPress` |

## Only changes arrive

The phone sends a channel when its value changes. A button press always arrives, even if the
value is the same as last time. `Edit::submitWith(button)` delivers the field only when that
button is pressed — a form with a submit button. `experiment.setResendUnchanged(true)` makes
the phone write every channel every cycle as 1.x did.

Values arrive only while the experiment runs (▶ in the app).
