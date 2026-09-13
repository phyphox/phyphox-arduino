// The static copy of the experiment that outlives the sketch's builder objects.
//
// PhyphoxBLE::addExperiment(exp) walks the builder's pointers (views → elements → subgraphs,
// export sets → entries, sensors) and copies every plain-data struct into `data`. From then on
// the builder objects may die; the serializer and the channel store read only this copy.
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
    /// Deep-copies the builder into `data`, assigns characteristics, validates channels and
    /// capacities. Returns false if an ERR_06/ERR_07 was recorded (the experiment is still
    /// served, with the error shown in the app).
    bool copyFrom(const PhyphoxBleExperiment& exp);
    /// The experiment served when a sketch calls start() without addExperiment(): one view with
    /// a graph of channel 1 over time and a value of channel 1 — the same on every board (§3.7).
    void buildDefault();
    void clear();

    const ExperimentData& data() const { return data_; }
    ExperimentData& data() { return data_; }
    const InputChannelInfo& input(uint8_t channel) const { return inputs_[channel]; }
    bool hasErrors() const;
    /// Highest input channel in use (so the serializer emits only what exists).
    uint8_t inputChannelsUsed() const { return inputChannelsUsed_; }

private:
    ExperimentData data_;
    InputChannelInfo inputs_[PHYPHOX_BLE_INPUT_CHANNELS + 1];   ///< 1-based
    uint8_t inputChannelsUsed_ = 0;
};

} // namespace phyphox

#endif
