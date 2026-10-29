# phyphox BLE — documentation

Start with the repository [README](../README.md) if you have never used phyphox. Then:

| page | what it answers |
|---|---|
| [Getting started](getting-started.md) | install, first sketch, per-board notes |
| [Concepts](concepts.md) | what the phone receives, channels, polling, memory, strings, errors |
| [Views and graphs](views.md) | `View`, `Graph`, `Value`, `InfoField`, `Separator` |
| [User input](inputs.md) | `Edit`, `Slider`, `Dropdown`, `Toggle`, `Button` and the board-side callbacks |
| [Phone sensors](sensors.md) | reading the phone's sensors on the board |
| [Export](export.md) | what the app's export button produces |
| [API reference](api.md) | every class and method on one page |
| [Protocol](protocol.md) | the BLE services and characteristics, the experiment transfer |

For existing projects: [Migrating from 1.x](migrating-from-1x.md) and the
[Changelog](../CHANGELOG.md). For contributors: [Testing](testing.md).

These pages are also published as a website, together with the reference of every class and
method generated from the headers: <https://phyphox.github.io/phyphox-arduino/>.

The phyphox file format itself — what the generated experiment *means* — is documented in the
[phyphox documentation](https://phyphox.org/docs) (the "file format" section), which is the canonical
reference; this library only generates a small subset of it. Version 2.0 declares format `1.20`.
