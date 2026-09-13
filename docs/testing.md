# Testing

T0 exists and runs in CI (`.github/workflows/t0.yml`); T1 and T2 are built in phases 4 and 5 of
the rewrite plan.

```
cmake -S tests/host -B tests/host/build && cmake --build tests/host/build -j
tests/host/build/phyphox_tests            # the core unit tests (doctest)
tests/conformance/run.py                  # generated documents vs. the phyphox file-format spec
tools/check_surface.py                    # api/surface.yml vs. the spec
arduino-cli compile --fqbn esp32:esp32:esp32 --library . examples/randomNumbers   # one row of the compile matrix
```

`.github/workflows/compile.yml` compiles every example for nine boards with a size report per
pull request (arduino/compile-sketches).

## The bench (T1)

```
tools/bench/bench.py --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 --label esp32 --name bench_esp32
tools/bench/bench.py --fqbn arduino:samd:nano_33_iot --port /dev/ttyACM0 --label nano33iot --name bench_nano33iot
```

## The phones (T2)

```
tools/bench/phones.py --board-port /dev/ttyUSB0 --board-name bench_esp32 --label esp32 \
    --android <adb serial> --ios <udid>
```

Drives a real phyphox app on an Android phone and on an iPhone against a board running the
bench sketch, from this Linux host. Android needs the app's debug and androidTest APKs
installed (`./gradlew installRegularDebug installRegularDebugAndroidTest` in phyphox-android);
the connect step is the app's `BleCompatConnectTest` seam. iOS needs a development build of
the app on the phone, developer mode, and `sudo pymobiledevice3 remote tunneld` running; the
app is launched with its `-phyphoxBleConnect` seam through the tunnel. Everything after the
connect goes through the remote-access API: start and stop, buffer writes, button triggers,
readings — with the board's serial echo as the other half of each assertion. It checks
change-only delivery of every input element, buttons, submit-with-button, the phone's
accelerometer streaming to the board, start/pause/clear events, and whether an input set while
paused reaches the board (it does not, on either app).

### The release sweep

```
tools/bench/examples.py --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 --label esp32 --android <serial> --ios <udid>
```

Flashes every shipped example as it is and lets both real apps load and start it, checking
what the example is about: the board's data arriving in the app, or the board's own serial
output for the examples that only receive. Run it per board before a release.

### Transfer robustness

```
tools/bench/phones.py ... --transfer-loop 20
```

Loads the experiment from the board twenty times per phone over fresh connections and reports
the board's transfer statistics (refused and retried packets, abandoned transfers). Run it with
the phone far from the board or with other boards advertising to see the flow control at work.
Set `PHYPHOX_BENCH_SERIAL_LOG=<file>` to keep everything the board prints; on the ESP32, build
the bench sketch with `-DPHYPHOX_BLE_ESP32_TRACE` in the extra flags to see the Bluetooth
stack's refused confirmations as well (`[phyphoxBLE] conf ...` lines).

### The user-XML mode

```
tools/bench/bench.py --fqbn ... --port ... --label ... --custom-xml
```

Flashes `tools/bench/benchCustomXml`, a sketch that serves a hand-written document, and checks
that it is served verbatim with the 1.x characteristic layout (the five-float config on
`cddf1003`).

## The bench sketch

Flashes `tools/bench/benchSketch` (a rich experiment that echoes every callback on serial), then
plays the phone from this machine's Bluetooth adapter with `bleak`: discovery, the expected
characteristics, the transfer on subscription and on the control write, byte-for-byte equality
with the document the board prints on serial, CRC, data rate, one callback per input write, a
button press per write, a sensor write fanning out to its channels, an event write, repeated
transfers over fresh connections, and the board's own statistics (no aborted transfers). The
served document is validated against the phyphox spec. Give each board on the desk its own
`--name`. Results go to `tools/bench/results/<label>.json`. The script takes the same
`.bench-lock-<hostname>` in the working root as the phyphox-docs lab.

The conformance step needs `phyphox-docs` checked out beside this repository and `lxml`; it
uses that repository's own validators (RELAX NG, Schematron, `validate_experiments.py`), with
the Schematron's version-gate warnings counted as failures. `tests/host/golden/` holds the
documents the example configurations must produce byte for byte; regenerate them with
`tests/host/build/gen_examples tests/host/golden` and review the diff before committing.

| tier | what | runs where |
|---|---|---|
| T0 | host unit tests of the core (`tests/host`), conformance of the generated documents against the phyphox file-format spec (`tests/conformance`, needs `phyphox-docs` checked out beside this repo), compile matrix of every example on every supported board with a size report | every push, GitHub Actions |
| T1 | the bench: every supported board flashed in turn and driven by a host BLE client (`tools/bench`) that plays the phone's part — transfer, data, inputs, buttons, events, MTU, disconnects | the lab machine, before a release |
| T2 | the phones: one Android and one iOS phone against every board, for what only the real app can show | the lab, release gate |

Run the surface check now: `tools/check_surface.py` compares `api/surface.yml` with the spec.
