# API reference

An overview of the public API. The headers carry the full description of every method:
`src/phyphoxBle.h` (the simple API) and `src/phyphoxBleExperiment.h` (describing the
experiment), browsable at <https://phyphox.github.io/phyphox-arduino/> under *Classes*.

## PhyphoxBLE (static)

| method | purpose |
|---|---|
| `start(name = "phyphox-Arduino")`, `start(name, experiment)`, `start(name, xml, len)` | advertise; serve the default, a described, or a hand-written experiment |
| `addExperiment(experiment)` | serve this description (copies it) |
| `poll()` | every `loop()`; gives the stack CPU time, drives the transfer |
| `write(v1 … v5)`, `write(float*, n)`, `write(uint8_t*, n)` | send data channels |
| `read(v1 … v5)`, `readChannel(k)`, `changed(k)`, `onChange(k, cb)`, `onPress(k, cb)` | receive input channels |
| `configHandler`, `experimentEventHandler`, `eventType`, `experimentTime`, `systemTime` | callbacks for any value from the phone and for the app's start/pause/clear/sync events; the last event's fields |
| `setMTU(payload)`, `minConInterval`, `maxConInterval`, `slaveLatency`, `timeout` | radio parameters, before `start()` |
| `printXML(&Serial)`, `printErrors(&Serial)`, `stats()`, `currentConnections`, `isSubscribed`, `version()`, `transportName()` | introspection |

## PhyphoxBleExperiment

Experiment: `setTitle`, `setCategory`, `setDescription`, `setColor`, `setRepeating`,
`setSubscribeOnStart`, `setResendUnchanged`, `addView`, `addSensor`, `addExportSet`.

Elements: see [Views and graphs](views.md), [User input](inputs.md), [Phone sensors](sensors.md),
[Export](export.md).

## 1.x names

Names kept so that 1.x sketches compile; new sketches can ignore them.

| 1.x | use |
|---|---|
| `setLinewidth(w)` (graph, subgraph) | `setLineWidth(w)` |
| `graph.addChannel(x, y, color)` | `graph.setChannel(x, y)` and `graph.setColor(color)` |
| `setDatachannel(n)` (export) | `setDataChannel(n)` |
| `sensor.setComponent(c)` | `mapChannel(c, channel)`; `setComponent` maps to the next free channel |
| `SENSOR_ACCELEROMETER_WITHOUT_G`, `SENSOR_MAGNETOMETER` | see [Phone sensors](sensors.md#1x-names) |

## Errors

| code | meaning |
|---|---|
| ERR_01 | a string is longer than `PHYPHOX_BLE_MAX_STRING_LENGTH` (250) |
| ERR_02 | a number exceeds its limit (channel, rate, count) |
| ERR_03 | a colour is not six hexadecimal digits |
| ERR_04 | a style, component, sensor type or option the format does not have |
| ERR_05 | a layout that is not `auto`, `extend` or `fixed` |
| ERR_06 | out of memory for the experiment block, or more than `PHYPHOX_BLE_INLINE_OPTIONS` (8) entries added with `addMap`/`addOption` — use `setMaps`/`setOptions` |
| ERR_07 | a button shares its input channel with another element |
| ERR_08 | a null pointer where a string is required |
