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
| `cddf1003` | **user-XML mode only**: the 1.x config characteristic, five `float32` at offsets 0…16, so hand-written documents keep working |

## The transfer

1. The app subscribes to `cddf0002`, or writes 1 to `cddf0003` — whichever comes first starts
   the transfer; a second trigger during a transfer is ignored.
2. The board sends one header packet: `phyphox` (7 ASCII bytes), the document size as a
   big-endian `uint32`, its CRC-32 as a big-endian `uint32`.
3. Then the document, in notifications of MTU − 3 bytes (20 by default; on the ArduinoBLE
   boards always 20, because that library does not expose the negotiated MTU), back to back as
   fast as the local Bluetooth stack accepts them. A packet the stack refuses is re-sent after a few
   milliseconds; after 50 consecutive refusals the transfer is abandoned. There is no fixed
   delay between packets.
4. A disconnect aborts the transfer; the app then offers to retry.

Why no chunk re-requests: the BLE link layer acknowledges and retransmits every packet, so a
weak link makes the transfer slow, not lossy. The 1.x losses were the board discarding packets
its own stack had refused. Version 2.0 fixes that; see the rewrite plan for the measurements
and the decision.

## Connection parameters

The board asks for a connection interval of 15–30 ms with no latency and a 5 s supervision
timeout — values iOS accepts (Apple's accessory guidelines require a minimum of at least
15 ms). `PhyphoxBLE::minConInterval` etc. change them, in BLE units.

## Events

See the phyphox documentation for the 17-byte layout: type byte, experiment time (int64 ms,
big-endian), Unix time (int64 ms, big-endian). The library decodes it into
`PhyphoxBLE::eventType`, `experimentTime`, `systemTime`.
