# Protocol

The canonical specification of the phyphox BLE contract is the "Bluetooth Low Energy" page of
the phyphox documentation ("Phyphox service" section). This page says how this library uses it.

## Services and characteristics

Fixed by the phyphox app (contract):

| UUID (`…-30f7-4671-8b43-5e40ba53514a`) | role |
|---|---|
| `cddf0001` | experiment service — advertised, so the app lists the device as compatible |
| `cddf0002` | experiment characteristic (notify): the document transfer |
| `cddf0003` | experiment control (write): the app writes 1 to request the transfer, 0 before disconnecting |
| `cddf0004` | event characteristic (write): 17-byte start/pause/clear/sync events |

Conventions of this library (named in the generated document, so the app needs no knowledge
of them):

| UUID | role |
|---|---|
| `cddf1001` | data service |
| `cddf1002` | data characteristic (notify): five `float32` little-endian, channels 1…5 at offsets 0, 4, 8, 12, 16; with `setRepeating(n)`, n samples per notification |
| `cddf20kk` | input channel k (write): one `float32` little-endian, written by the phone when the channel changes |
| `cddf30ss` | phone sensor s (write): its mapped components as consecutive `float32` little-endian |
| `cddf1003` | **hand-written documents only** (`start(name, xml, len)`): the config characteristic, five `float32` little-endian at offsets 0…16, read with `PhyphoxBLE::read(...)` |

## The transfer

1. The app subscribes to `cddf0002` or writes 1 to `cddf0003`; whichever comes first starts
   the transfer, and a second trigger during a transfer is ignored.
2. The board sends a header packet: `phyphox` (7 ASCII bytes), the document size as a
   big-endian `uint32`, its CRC-32 as a big-endian `uint32`.
3. After 150 ms, the board sends the document in notifications of MTU − 3 bytes (20 by default;
   always 20 on the ArduinoBLE boards; at most 512 on the ESP32), as fast as the local
   Bluetooth stack accepts them (on the Nano 33 BLE and Nicla Sense ME one packet every
   30 ms). A packet the stack refuses is sent again a little later.
   After 3 s without progress the board gives up; the next trigger starts afresh.
4. A disconnect aborts the transfer; the app then offers to retry.

The BLE link layer acknowledges and retransmits every packet, so a weak link makes the transfer
slower, not incomplete.

## Connection parameters

The board asks for a connection interval of 15–30 ms with no latency and a 5 s supervision
timeout, values both Android and iOS accept. `PhyphoxBLE::minConInterval`, `maxConInterval`,
`slaveLatency` and `timeout` change them before `start()`, in BLE units (1.25 ms for the
intervals, 10 ms for the timeout); iOS ignores a request below 15 ms.

## Events

See the phyphox documentation for the 17-byte layout: type byte, experiment time (int64 ms,
big-endian), Unix time (int64 ms, big-endian). The library decodes it into
`PhyphoxBLE::eventType`, `experimentTime`, `systemTime`.
