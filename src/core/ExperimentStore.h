// The copy of the experiment that outlives the sketch's builder objects.
//
// PhyphoxBLE::addExperiment(exp) walks the builder's lists (views → elements → subgraphs,
// export sets → entries, sensors) twice: once to measure, then — after ONE malloc of exactly
// that size — to copy every plain-data struct into the block. From then on the builder objects
// may die; the serializer and the channel store read only this copy. The block is never freed
// (a second addExperiment() frees and reallocates it, which is the one case where the heap is
// touched twice). No capacity limits: RAM is what the experiment needs.
#ifndef PHYPHOX_BLE_CORE_EXPERIMENTSTORE_H
#define PHYPHOX_BLE_CORE_EXPERIMENTSTORE_H

#include "ElementData.h"

class PhyphoxBleExperiment;

namespace phyphox {

/// What the store knows about one input channel after the copy: which characteristic carries
/// it, how the app is told to deliver it, and which element owns it.
struct InputChannelInfo {
    bool used = false;
    bool isButton = false;      ///< a button owns this channel (no change detector, no default)
    bool fromSensor = false;    ///< fed by a phone sensor component (periodic by nature)
    uint8_t sensorIndex = 0;    ///< if fromSensor: which sensor, and
    uint8_t componentIndex = 0; ///< which component (offset = componentIndex * 4)
    float defaultValue = 0;
    bool hasDefault = false;
    uint8_t submitWithChannel = 0;
};

class ExperimentStore {
public:
    /// Measures, allocates and deep-copies the builder into the block; assigns characteristics;
    /// validates channels. Returns false if the allocation failed (ERR_06) or a channel
    /// conflict was recorded (ERR_07) — the experiment is still served with the error shown.
    bool copyFrom(const PhyphoxBleExperiment& exp);
    /// Bytes the copy of `exp` needs (what copyFrom() would allocate) — for the size report.
    static size_t bytesNeeded(const PhyphoxBleExperiment& exp);
    /// The experiment served when a sketch calls start() without addExperiment(): one view with
    /// a graph of channel 1 over time and a value of channel 1 — the same on every board (§3.7).
    void buildDefault();
    void clear();

    const ExperimentData& data() const { return data_; }
    ExperimentData& data() { return data_; }
    const InputChannelInfo& input(uint8_t channel) const;   ///< 1 … inputChannelsUsed()
    bool hasErrors() const;
    /// Highest input channel in use (so the serializer emits only what exists).
    uint8_t inputChannelsUsed() const { return inputChannelsUsed_; }
    size_t blockSize() const { return blockSize_; }         ///< bytes allocated
    /// Storage inside the block for the ChannelStore (ChannelStore::bytesNeeded(inputChannelsUsed())).
    void* channelStorage() const { return channelStorage_; }
    /// Storage inside the block for the serializer's part table (viewCount + 3 entries).
    uint16_t* partTable() const { return partTable_; }
    ~ExperimentStore() { clear(); }

private:
    ExperimentData data_;
    uint8_t* block_ = nullptr;         ///< the one allocation; holds every counted array
    size_t blockSize_ = 0;
    InputChannelInfo* inputs_ = nullptr; ///< in the block, 1-based, inputChannelsUsed_ + 1 entries
    void* channelStorage_ = nullptr;
    uint16_t* partTable_ = nullptr;
    uint8_t inputChannelsUsed_ = 0;
};

} // namespace phyphox

#endif
