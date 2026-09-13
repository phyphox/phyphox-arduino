#include "ExperimentStore.h"
#include "ChannelStore.h"
#include "../phyphoxBleExperiment.h"
#include <stdlib.h>
#include <string.h>

namespace phyphox {

// ---------------------------------------------------------------- measuring
// A bump allocator over the one block; the same walk measures (dry run) and places.
namespace {

struct Bump {
    uint8_t* base;
    size_t used;
    bool dry;
    void* take(size_t bytes, size_t align = 8) {
        used = (used + align - 1) & ~(align - 1);
        void* p = dry ? nullptr : base + used;
        used += bytes;
        return p;
    }
    template <class T> T* array(size_t n) { return (T*)take(sizeof(T) * n, alignof(T) > 4 ? 8 : 4); }
};

typedef PhyphoxBleExperiment B;

struct Counts {
    size_t views = 0, elements = 0, subgraphs = 0, entries = 0, exportSets = 0, sensors = 0, exportData = 0;
    uint8_t maxChannel = 0;
};

void count(const B& exp, Counts& c) {
    for (const B::View* v = exp.views; v; v = v->next) {
        c.views++;
        for (const B::Element* e = v->elements; e; e = e->next) {
            c.elements++;
            const ElementData& d = e->data();
            if (d.type == EL_GRAPH) {
                c.subgraphs++;   // the first curve
                for (const B::Graph::Subgraph* s = static_cast<const B::Graph*>(e)->subgraphs; s; s = s->next) c.subgraphs++;
            } else if (d.type == EL_VALUE) {
                c.entries += static_cast<const B::Value*>(e)->mapCount;
            } else if (d.type == EL_DROPDOWN) {
                c.entries += static_cast<const B::Dropdown*>(e)->optionCount;
            }
            const InputCommon* in = d.input();
            if (in && in->channel > c.maxChannel) c.maxChannel = in->channel;
            if (d.visibilityChannel > c.maxChannel) c.maxChannel = d.visibilityChannel;
        }
    }
    for (const B::ExportSet* s = exp.exportSets; s; s = s->next) {
        c.exportSets++;
        for (const B::ExportData* d = s->entries; d; d = d->nextEntry) c.exportData++;
    }
    for (const B::Sensor* s = exp.sensors; s; s = s->next) {
        c.sensors++;
        for (uint8_t i = 0; i < s->data.componentCount; ++i)
            if (s->data.channels[i] > c.maxChannel) c.maxChannel = s->data.channels[i];
    }
}

size_t place(Bump& b, const Counts& c) {
    b.array<ElementData>(c.elements);
    b.array<SubgraphData>(c.subgraphs);
    b.array<MapEntry>(c.entries);
    b.array<ViewData>(c.views);
    b.array<ExportSetData>(c.exportSets);
    b.array<ExportDataData>(c.exportData);
    b.array<SensorData>(c.sensors);
    b.array<InputChannelInfo>(c.maxChannel + 1);
    b.take(ChannelStore::bytesNeeded(c.maxChannel));
    b.array<uint16_t>(c.views + 3);
    return b.used;
}

} // namespace

size_t ExperimentStore::bytesNeeded(const PhyphoxBleExperiment& exp) {
    Counts c; count(exp, c);
    Bump b{nullptr, 0, true};
    return place(b, c);
}

void ExperimentStore::clear() {
    if (block_) free(block_);
    block_ = nullptr; blockSize_ = 0; inputs_ = nullptr; channelStorage_ = nullptr; partTable_ = nullptr;
    inputChannelsUsed_ = 0;
    data_ = ExperimentData();
}

const InputChannelInfo& ExperimentStore::input(uint8_t channel) const {
    static const InputChannelInfo none;
    if (!inputs_ || channel == 0 || channel > inputChannelsUsed_) return none;
    return inputs_[channel];
}

bool ExperimentStore::hasErrors() const {
    if (data_.error.set()) return true;
    for (uint8_t v = 0; v < data_.viewCount; ++v)
        for (uint8_t i = 0; i < data_.views[v].elementCount; ++i)
            if (data_.views[v].elements[i].error.set()) return true;
    for (uint8_t s = 0; s < data_.sensorCount; ++s) if (data_.sensors[s].error.set()) return true;
    return false;
}

// ---------------------------------------------------------------- copying

bool ExperimentStore::copyFrom(const PhyphoxBleExperiment& exp) {
    clear();
    Counts c; count(exp, c);
    Bump dry{nullptr, 0, true};
    size_t bytes = place(dry, c);
    block_ = (uint8_t*)malloc(bytes);
    if (!block_) { data_.error.record(ERR_06_CAPACITY, "addExperiment"); return false; }
    memset(block_, 0, bytes);
    blockSize_ = bytes;

    Bump b{block_, 0, false};
    ElementData* elements = b.array<ElementData>(c.elements);
    SubgraphData* subgraphs = b.array<SubgraphData>(c.subgraphs);
    MapEntry* entries = b.array<MapEntry>(c.entries);
    ViewData* views = b.array<ViewData>(c.views);
    ExportSetData* sets = b.array<ExportSetData>(c.exportSets);
    ExportDataData* exportData = b.array<ExportDataData>(c.exportData);
    SensorData* sensors = b.array<SensorData>(c.sensors);
    inputs_ = b.array<InputChannelInfo>(c.maxChannel + 1);
    channelStorage_ = b.take(ChannelStore::bytesNeeded(c.maxChannel));
    partTable_ = b.array<uint16_t>(c.views + 3);
    for (uint8_t i = 0; i <= c.maxChannel; ++i) inputs_[i] = InputChannelInfo();
    inputChannelsUsed_ = c.maxChannel;

    // experiment-level fields
    data_ = exp.data;
    data_.views = views; data_.viewCount = 0;
    data_.exportSets = sets; data_.exportSetCount = 0;
    data_.sensors = sensors; data_.sensorCount = 0;

    bool ok = true;
    size_t ei = 0, si = 0, mi = 0;
    for (const B::View* v = exp.views; v; v = v->next) {
        ViewData& vd = views[data_.viewCount++];
        vd = v->data;
        vd.elements = elements + ei;
        vd.elementCount = 0;
        for (const B::Element* e = v->elements; e; e = e->next) {
            ElementData& d = elements[ei++];
            memcpy(&d, &e->data(), sizeof(ElementData));
            vd.elementCount++;
            if (d.type == EL_GRAPH) {
                const B::Graph* g = static_cast<const B::Graph*>(e);
                d.graph.subgraphs = subgraphs + si;
                d.graph.subgraphCount = 0;
                subgraphs[si++] = g->first.data;
                subgraphs[si - 1].active = true;
                d.graph.subgraphCount++;
                for (const B::Graph::Subgraph* s = g->subgraphs; s; s = s->next) {
                    subgraphs[si++] = s->data;
                    d.graph.subgraphCount++;
                    if (s->error.set() && !d.error.set()) d.error = s->error;
                }
            } else if (d.type == EL_VALUE) {
                const B::Value* val = static_cast<const B::Value*>(e);
                d.value.maps = entries + mi;
                d.value.mapCount = val->mapCount;
                for (uint8_t i = 0; i < val->mapCount; ++i) {
                    MapEntry& m = entries[mi++];
                    if (val->mapsTexts) {
                        m.text = val->mapsTexts[i];
                        m.a = val->mapsMins ? val->mapsMins[i] : 0;
                        m.b = val->mapsMaxs ? val->mapsMaxs[i] : 0;
                        m.flags = (val->mapsMins ? MapEntry::HAS_MIN : 0) | (val->mapsMaxs ? MapEntry::HAS_MAX : 0);
                    } else {
                        m = val->inlineMaps[i];
                    }
                }
            } else if (d.type == EL_DROPDOWN) {
                const B::Dropdown* dd = static_cast<const B::Dropdown*>(e);
                d.dropdown.options = entries + mi;
                d.dropdown.optionCount = dd->optionCount;
                for (uint8_t i = 0; i < dd->optionCount; ++i) {
                    MapEntry& m = entries[mi++];
                    if (dd->optionLabels) { m.text = dd->optionLabels[i]; m.a = dd->optionValues[i]; m.b = 0; m.flags = 0; }
                    else m = dd->inlineOptions[i];
                }
            }
            // input channel bookkeeping
            InputCommon* in = d.input();
            if (in) {
                if (in->channel == 0) { d.error.record(ERR_02_ABOVE_LIMIT, "setChannel"); ok = false; continue; }
                InputChannelInfo& ci = inputs_[in->channel];
                bool isButton = d.type == EL_BUTTON;
                if (ci.used && (isButton || ci.isButton || ci.fromSensor)) {
                    d.error.record(ERR_07_CHANNEL_CONFLICT, "setChannel"); ok = false;
                }
                ci.used = true;
                ci.isButton = ci.isButton || isButton;
                if (in->hasDefault) { ci.defaultValue = in->defaultValue; ci.hasDefault = true; }
                if (in->submitWithChannel) ci.submitWithChannel = in->submitWithChannel;
            }
        }
    }
    for (const B::ExportSet* s = exp.exportSets; s; s = s->next) {
        ExportSetData& sd = sets[data_.exportSetCount++];
        sd = s->data;
        sd.entries = exportData + data_.exportDataCount_;
        sd.entryCount = 0;
        for (const B::ExportData* d = s->entries; d; d = d->nextEntry) {
            exportData[data_.exportDataCount_++] = d->exportData;
            sd.entryCount++;
        }
    }
    for (const B::Sensor* s = exp.sensors; s; s = s->next) {
        uint8_t sensorIndex = data_.sensorCount;
        SensorData& sd = sensors[data_.sensorCount++];
        sd = s->data;
        for (uint8_t i = 0; i < sd.componentCount; ++i) {
            InputChannelInfo& ci = inputs_[sd.channels[i]];
            if (ci.used) { sd.error.record(ERR_07_CHANNEL_CONFLICT, "mapChannel"); ok = false; }
            ci.used = true; ci.fromSensor = true; ci.sensorIndex = sensorIndex; ci.componentIndex = i;
        }
    }
    return ok;
}

void ExperimentStore::buildDefault() {
    B exp;
    B::View view("Data");
    B::Graph graph("Graph");
    graph.setChannel(0, 1);
    graph.setLabelX("time"); graph.setUnitX("s"); graph.setLabelY("value");
    B::Value value("value", 1);
    view.addElement(graph).addElement(value);
    exp.addView(view);
    copyFrom(exp);
}

} // namespace phyphox
