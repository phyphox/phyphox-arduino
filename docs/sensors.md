# Phone sensors

*Skeleton — the example is ported in phase 3.*

The board can ask the phone for its own sensors:

```cpp
PhyphoxBleExperiment::Sensor acc(SENSOR_ACCELEROMETER);
acc.setRate(50);            // Hz; unset = as fast as the phone offers
acc.setAverage(false);
acc.mapChannel("x", 1);     // components: x, y, z, abs, t, accuracy
acc.mapChannel("y", 2);
acc.mapChannel("z", 3);
exp.addSensor(acc);
```

The values arrive in input channels 1…3 like user input (`PhyphoxBLE::read(x, y, z)` or
`onChange`), continuously while the experiment runs. All components of one sensor arrive in
one Bluetooth write, so the callbacks of its channels fire together.

## Sensor types

Every sensor the phyphox file format knows, under the format's own name:

| constant | reading | unit |
|---|---|---|
| `SENSOR_ACCELEROMETER` | acceleration including gravity | m/s² |
| `SENSOR_LINEAR_ACCELERATION` (`SENSOR_ACCELEROMETER_WITHOUT_G`) | acceleration without gravity | m/s² |
| `SENSOR_GRAVITY` | gravity in the device frame | m/s² |
| `SENSOR_GYROSCOPE` | angular velocity | rad/s |
| `SENSOR_MAGNETIC_FIELD` | magnetic field | µT |
| `SENSOR_PRESSURE` | air pressure | hPa |
| `SENSOR_LIGHT` | illuminance | lx |
| `SENSOR_PROXIMITY` | proximity (most phones: 0 or 5) | cm |
| `SENSOR_TEMPERATURE` | ambient temperature, where a phone has it | °C |
| `SENSOR_HUMIDITY` | relative humidity | % |
| `SENSOR_ATTITUDE` | orientation as a quaternion: `x`, `y`, `z` are its vector part, `abs` is `w` | — |
| `SENSOR_CUSTOM` | any sensor the phone exposes, selected with `setNameFilter` / `setTypeFilter` (Android) | sensor-specific |

Not every phone has every sensor; the app tells the user which are missing.

## Components

`mapChannel(component, channel)` takes `x`, `y`, `z`, `abs` (the magnitude, or `w` for the
attitude), `t` (the phone's timestamp of the reading, in seconds) and `accuracy` (the sensor's
own accuracy flag).

## Rate

`setRate(hz)` asks for a rate; `setRateStrategy("auto" | "request" | "generate" | "limit")`
says how the phone reconciles it with what the sensor offers, and `setStride(n)` keeps only
every n-th reading. `setAverage(true)` averages readings between deliveries instead of
dropping them.

## 1.x note

`SENSOR_MAGNETOMETER` is a deprecated alias of `SENSOR_MAGNETIC_FIELD` — in 1.x it emitted a
name the format does not know, so it never worked.
