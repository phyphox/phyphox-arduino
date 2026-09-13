# Phone sensors

*Skeleton — filled in during phase 3.*

The board can ask the phone for its own sensors:

```cpp
PhyphoxBleExperiment::Sensor acc(SENSOR_ACCELEROMETER);
acc.setRate(50);            // Hz; 0 = as fast as the phone offers
acc.setAverage(false);
acc.mapChannel("x", 1);     // components: x, y, z, abs, t, accuracy
acc.mapChannel("y", 2);
acc.mapChannel("z", 3);
exp.addSensor(acc);
```

The values arrive in input channels 1…3 like user input (`PhyphoxBLE::read(x, y, z)` or
`onChange`), continuously while the experiment runs.

Sensor types (phyphox file-format names): `SENSOR_ACCELEROMETER`, `SENSOR_LINEAR_ACCELERATION`
(`SENSOR_ACCELEROMETER_WITHOUT_G`), `SENSOR_GRAVITY`, `SENSOR_GYROSCOPE`, `SENSOR_MAGNETIC_FIELD`,
`SENSOR_PRESSURE`, `SENSOR_LIGHT`, `SENSOR_PROXIMITY`, `SENSOR_TEMPERATURE`, `SENSOR_HUMIDITY`,
`SENSOR_ATTITUDE`. Not every phone has every sensor; the app shows which are missing.

`SENSOR_MAGNETOMETER` is a deprecated alias of `SENSOR_MAGNETIC_FIELD` — in 1.x it emitted a
name the format does not know, so it never worked.
