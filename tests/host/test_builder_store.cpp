#include "doctest.h"
#include "helpers.h"
#include "examples_config.h"
#include "core/ExperimentStore.h"

using namespace phyphox;

TEST_CASE("Setters validate and record the first error") {
    E::Graph g;
    g.setColor("12345");  CHECK(g.data().error.code == ERR_03_NOT_HEX_COLOR); CHECK(std::string(g.data().error.origin) == "setColor");
    g.setStyle("bogus");  CHECK(g.data().error.code == ERR_03_NOT_HEX_COLOR);   // first error wins
    E::Graph g2; g2.setStyle("bogus"); CHECK(g2.data().error.code == ERR_04_INVALID_VALUE);
    E::Graph g3; g3.setMinX(0, "sideways"); CHECK(g3.data().error.code == ERR_05_INVALID_LAYOUT);
    E::Graph g4; g4.setChannel(0, 6); CHECK(g4.data().error.code == ERR_02_ABOVE_LIMIT);
    E::Value v; v.setLabel(nullptr); CHECK(v.data().error.code == ERR_08_NULL_STRING);
    std::string longStr(300, 'x'); E::Value v2; v2.setUnit(longStr.c_str()); CHECK(v2.data().error.code == ERR_01_TOO_LONG);
    E::Slider s; s.setChannel(17); CHECK(s.data().error.code == ERR_02_ABOVE_LIMIT);
    E::Sensor sen; sen.setType("magnetometer"); CHECK(sen.data.error.code == ERR_04_INVALID_VALUE);
    E::Sensor sen2; sen2.setType("magnetic_field"); CHECK(!sen2.data.error.set()); sen2.mapChannel("w", 1); CHECK(sen2.data.error.code == ERR_04_INVALID_VALUE);
    E::InfoField i; i.setAlign("middle"); CHECK(i.data().error.code == ERR_04_INVALID_VALUE);
    E::Dropdown d; for (int k = 0; k < 9; ++k) d.addOption("o", k); CHECK(d.data().error.code == ERR_06_CAPACITY);
}

TEST_CASE("Lists link in order and the store copies them") {
    E exp("T", "C", "D");
    E::View v1("one"), v2("two");
    E::Graph g("g"); E::Value val("v", 2); E::InfoField info("i");
    v1.addElement(g).addElement(val); v2.addElement(info);
    exp.addView(v1).addView(v2);
    CHECK(v1.elements == &g); CHECK(g.next == &val); CHECK(val.next == nullptr);
    ExperimentStore store;
    CHECK(store.copyFrom(exp));
    const ExperimentData& d = store.data();
    CHECK(d.viewCount == 2);
    CHECK(d.views[0].elementCount == 2); CHECK(d.views[1].elementCount == 1);
    CHECK(d.views[0].elements[0].type == EL_GRAPH); CHECK(d.views[0].elements[1].type == EL_VALUE);
    CHECK(std::string(d.views[1].elements[0].label) == "i");
    CHECK(d.views[0].elements[1].value.channel == 2);
    CHECK(store.blockSize() == ExperimentStore::bytesNeeded(exp));
    CHECK(store.inputChannelsUsed() == 0);
}

TEST_CASE("Graph curves, value maps and dropdown options land in the block") {
    E exp; E::View v("v");
    E::Graph g("g"); g.setChannel(0, 1); g.setColor("ff0000");
    E::Graph::Subgraph a, b; a.setChannel(0, 2); b.setChannel(1, 3); b.setStyle(STYLE_DOTS);
    g.addSubgraph(a); g.addSubgraph(b);
    E::Value val("val", 1); val.addMap(0, 1, "low"); val.addMapAbove(1, "high");
    E::Dropdown dd("dd", 1); dd.addOption("x", 1); dd.addOption("y", 2);
    v.addElement(g).addElement(val).addElement(dd); exp.addView(v);
    ExperimentStore store; CHECK(store.copyFrom(exp));
    const ElementData* els = store.data().views[0].elements;
    REQUIRE(els[0].graph.subgraphCount == 3);
    CHECK(els[0].graph.subgraphs[0].channelY == 1); CHECK(std::string(els[0].graph.subgraphs[0].color) == "ff0000");
    CHECK(els[0].graph.subgraphs[2].channelX == 1); CHECK(els[0].graph.subgraphs[2].style == ST_DOTS);
    REQUIRE(els[1].value.mapCount == 2);
    CHECK(std::string(els[1].value.maps[1].text) == "high"); CHECK(els[1].value.maps[1].flags == MapEntry::HAS_MIN);
    REQUIRE(els[2].dropdown.optionCount == 2);
    CHECK(els[2].dropdown.options[1].a == 2); CHECK(std::string(els[2].dropdown.options[1].text) == "y");
}

TEST_CASE("Input channels: bookkeeping, defaults, sharing and conflicts") {
    E exp; E::View v("v");
    E::Edit e("e", 1); e.setDefault(3);
    E::Slider s("s", 0, 1, 0.1f, 1);          // shares channel 1 with the edit: allowed
    E::Button b("b", 2);
    E::Toggle t("t", 3);
    E::Edit sub("sub", 4); sub.submitWith(b);
    v.addElement(e).addElement(s).addElement(b).addElement(t).addElement(sub); exp.addView(v);
    ExperimentStore store; CHECK(store.copyFrom(exp));
    CHECK(store.inputChannelsUsed() == 4);
    CHECK(store.input(1).used); CHECK(store.input(1).hasDefault); CHECK(store.input(1).defaultValue == 3); CHECK(!store.input(1).isButton);
    CHECK(store.input(2).isButton);
    CHECK(store.input(4).submitWithChannel == 2);
    CHECK(!store.hasErrors());

    E exp2; E::View v2("v"); E::Button b1("b", 1); E::Edit e1("e", 1);
    v2.addElement(b1).addElement(e1); exp2.addView(v2);
    ExperimentStore store2; CHECK(!store2.copyFrom(exp2));
    CHECK(store2.data().views[0].elements[1].error.code == ERR_07_CHANNEL_CONFLICT);

    E exp3; E::View v3("v"); E::Slider noChannel("s", 0, 1, 1);
    v3.addElement(noChannel); exp3.addView(v3);
    ExperimentStore store3; CHECK(!store3.copyFrom(exp3));
    CHECK(store3.data().views[0].elements[0].error.code == ERR_02_ABOVE_LIMIT);
}

TEST_CASE("Sensors map into input channels") {
    E exp; E::View v("v"); E::InfoField i("i"); v.addElement(i);
    E::Sensor acc("accelerometer"); acc.mapChannel("x", 1); acc.mapChannel("y", 2); acc.mapChannel("abs", 5);
    exp.addView(v).addSensor(acc);
    ExperimentStore store; CHECK(store.copyFrom(exp));
    CHECK(store.data().sensorCount == 1);
    CHECK(store.inputChannelsUsed() == 5);
    CHECK(store.input(5).fromSensor); CHECK(store.input(5).sensorIndex == 0); CHECK(store.input(5).componentIndex == 2);
    CHECK(!store.input(3).used);
}

TEST_CASE("The default experiment is graph plus value") {
    ExperimentStore store; store.buildDefault();
    REQUIRE(store.data().viewCount == 1);
    REQUIRE(store.data().views[0].elementCount == 2);
    CHECK(store.data().views[0].elements[0].type == EL_GRAPH);
    CHECK(store.data().views[0].elements[1].type == EL_VALUE);
    CHECK(store.data().views[0].elements[0].graph.subgraphs[0].channelX == 0);
    CHECK(store.data().views[0].elements[0].graph.subgraphs[0].channelY == 1);
}

TEST_CASE("Builder objects may die after the copy") {
    ExperimentStore store;
    {
        E exp("Scoped", "C", "D"); E::View v("v"); E::Graph g("g"); g.setChannel(0, 1); v.addElement(g); exp.addView(v);
        store.copyFrom(exp);
    }
    CHECK(std::string(store.data().title) == "Scoped");   // literals live on; the objects did not
    CHECK(store.data().views[0].elements[0].graph.subgraphCount == 1);
}
