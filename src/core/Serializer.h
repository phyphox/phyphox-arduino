// The streaming XML serializer.
//
// The document is a sequence of PARTS, each generated deterministically from the store:
//   0            prologue: <phyphox>, title, category, description, data-containers, the
//                bluetooth input (data channels, phone sensors), the bluetooth output (one
//                <input> per input channel / sensor characteristic), the analysis block with
//                the change detector of every input channel
//   1 … V        one part per view (its opening tag, its elements, its closing tag)
//   V+1          epilogue: the error view if any, the export block, </phyphox>
//
// build() runs every part once through a CountingSink, recording each part's length; the
// cumulative offsets form the PART TABLE. After that, any byte range of the document is served
// by regenerating only the one or two parts it touches through a WindowSink. There is one full
// pass per addExperiment(), never one per packet.
//
// Determinism is the invariant everything rests on: the same store must yield identical bytes
// every time. The host tests assert it (part-table offsets ≡ a full serialization; every
// window ≡ the slice of the full document).
#ifndef PHYPHOX_BLE_CORE_SERIALIZER_H
#define PHYPHOX_BLE_CORE_SERIALIZER_H

#include "ExperimentStore.h"
#include "Sink.h"

namespace phyphox {

class Serializer {
public:
    /// Bind to a store and build the part table. `deviceName` goes into the bluetooth
    /// name attributes; `mtu` > 20 emits the mtu attribute.
    void build(const ExperimentStore& store, const char* deviceName, uint16_t mtu);

    uint32_t size() const { return totalSize_; }   ///< bytes of the whole document
    uint32_t crc() const { return crc_; }          ///< CRC-32 of the whole document
    uint16_t partCount() const { return partCount_; }

    /// Copy document bytes [offset, offset + n) into `out`; returns bytes produced (fewer at
    /// the end). Regenerates only the parts covering the range.
    uint32_t read(uint32_t offset, uint8_t* out, uint32_t n) const;
    /// Stream the entire document (printXML, host tests).
    void writeAll(Sink& sink) const;

    /// Serialize one part into a sink (public for the tests).
    void emitPart(uint16_t part, Sink& sink) const;

private:
    const ExperimentStore* store_ = nullptr;
    const char* deviceName_ = nullptr;
    uint16_t mtu_ = PHYPHOX_BLE_DEFAULT_MTU;
    uint32_t totalSize_ = 0;
    uint32_t crc_ = 0;
    uint16_t partCount_ = 0;
    /// Cumulative start offset of each part; partOffset_[partCount_] == totalSize_. Lives in
    /// the store's block (viewCount + 3 entries of uint16_t — documents stay far below 64 KB).
    uint16_t* partOffset_ = nullptr;

    void emitPrologue(Sink&) const;
    void emitView(uint8_t view, Sink&) const;
    void emitElement(const ElementData&, Sink&) const;
    void emitEpilogue(Sink&) const;
};

/// Text helpers shared by the emitters (public so the host tests can pin them):
/// XML-escape `text` into the sink; format a float the way the phyphox format expects
/// (shortest round-trip-ish decimal, no exponent for the ranges sketches use, "." decimal point).
void writeEscaped(Sink&, const char* text);
void writeFloat(Sink&, float v);
void writeInt(Sink&, int32_t v);
/// The UUID of the k-th input-channel / s-th sensor characteristic (docs/protocol.md).
void writeInputCharUuid(Sink&, uint8_t channel);
void writeSensorCharUuid(Sink&, uint8_t sensor);

} // namespace phyphox

#endif
