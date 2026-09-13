# Testing

*Skeleton — the suites are built in phases 2, 4 and 5 of the rewrite plan.*

| tier | what | runs where |
|---|---|---|
| T0 | host unit tests of the core (`tests/host`), conformance of the generated documents against the phyphox file-format spec (`tests/conformance`, needs `phyphox-docs` checked out beside this repo), compile matrix of every example on every supported board with a size report | every push, GitHub Actions |
| T1 | the bench: every supported board flashed in turn and driven by a host BLE client (`tools/bench`) that plays the phone's part — transfer, data, inputs, buttons, events, MTU, disconnects | the lab machine, before a release |
| T2 | the phones: one Android and one iOS phone against every board, for what only the real app can show | the lab, release gate |

Run the surface check now: `tools/check_surface.py` compares `api/surface.yml` with the spec.
