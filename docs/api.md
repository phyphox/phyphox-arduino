# API reference

*Skeleton — kept in step with the headers by a test from phase 2 on; the generated Doxygen
reference will be linked here.* Until then the headers are the reference: `src/phyphoxBle.h`
(the simple API), `src/phyphoxBleExperiment.h` (the description API).

## PhyphoxBLE (static)

| method | purpose |
|---|---|
| `start(name = "phyphox-Arduino")`, `start(name, experiment)`, `start(name, xml, len)` | advertise; serve the default, a described, or a hand-written experiment |
| `addExperiment(experiment)` | serve this description (copies it) |
| `poll()` | every `loop()`; gives the stack CPU time, drives the transfer |
| `write(v1 … v5)`, `write(float*, n)`, `write(uint8_t*, n)` | send data channels |
| `read(v1 … v5)`, `readChannel(k)`, `changed(k)`, `onChange(k, cb)`, `onPress(k, cb)` | receive input channels |
| `configHandler`, `experimentEventHandler`, `eventType`, `experimentTime`, `systemTime` | 1.x hooks and event fields |
| `setMTU(payload)`, `minConInterval`, `maxConInterval`, `slaveLatency`, `timeout` | radio parameters, before `start()` |
| `printXML(&Serial)`, `printErrors(&Serial)`, `stats()`, `currentConnections`, `isSubscribed`, `version()`, `transportName()` | introspection |

## PhyphoxBleExperiment

Experiment: `setTitle`, `setCategory`, `setDescription`, `setColor`, `setRepeating`,
`setSubscribeOnStart`, `setResendUnchanged`, `addView`, `addSensor`, `addExportSet`.

Elements: see [Views and graphs](views.md), [User input](inputs.md), [Phone sensors](sensors.md),
[Export](export.md).

## Errors

| code | meaning |
|---|---|
| ERR_01 | a string is longer than `PHYPHOX_BLE_MAX_STRING_LENGTH` (250) |
| ERR_02 | a number exceeds its limit (channel, rate, count) |
| ERR_03 | a colour is not six hexadecimal digits |
| ERR_04 | a style, component, sensor type or option the format does not have |
| ERR_05 | a layout that is not `auto`, `extend` or `fixed` |
| ERR_06 | a pool is full — raise the `PHYPHOX_BLE_MAX_*` constant |
| ERR_07 | a button shares its input channel with another element |
| ERR_08 | a null pointer where a string is required |
