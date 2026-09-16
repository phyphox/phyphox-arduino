// The experiment configurations of the shipped examples, as functions, so the same
// descriptions feed the golden-file tests and the conformance generator. Objects must outlive
// the call that copies them, so each function takes the server and calls addExperiment itself.
#ifndef PHYPHOX_EXAMPLES_CONFIG_H
#define PHYPHOX_EXAMPLES_CONFIG_H

#include "phyphoxBleExperiment.h"
#include "core/Server.h"

typedef PhyphoxBleExperiment E;

inline void exampleCreateExperiment(phyphox::Server& server) {
    E exp("Create Experiment", "Arduino Experiments", "Random numbers are generated on Arduino and visualized with phyphox afterwards");
    E::View firstView("First view");
    E::Graph firstGraph("Random numbers");
    firstGraph.setUnitX("s"); firstGraph.setUnitY("");
    firstGraph.setLabelX("time"); firstGraph.setLabelY("random number");
    firstGraph.setXPrecision(1); firstGraph.setYPrecision(1);
    firstGraph.setStyle(STYLE_LINES); firstGraph.setColor("2E728E");
    firstGraph.setChannel(0, 1);
    E::Graph secondGraph("Squared numbers");
    secondGraph.setStyle(STYLE_DOTS); secondGraph.setLinewidth(2);
    secondGraph.setChannel(1, 2);
    E::View secondView("Second view");
    E::InfoField info("Values can also be shown as numbers & text");
    E::Separator sep; sep.setHeight(0.3f); sep.setColor("404040");
    E::Value value("Random value", 1); value.setPrecision(2); value.setUnit("Zufallszahl");
    E::Edit edit("Edit", 1); edit.setUnit("unit"); edit.setSigned(false); edit.setDecimal(false); edit.setDefault(5);
    firstView.addElement(firstGraph).addElement(secondGraph);
    secondView.addElement(info).addElement(sep).addElement(value).addElement(edit);
    E::ExportSet set("mySet");
    E::ExportData d1("time", 0); E::ExportData d2("random", 1); E::ExportData d3("square", 2);
    set.addElement(d1).addElement(d2).addElement(d3);
    exp.addView(firstView).addView(secondView).addExportSet(set);
    server.addExperiment(exp);
}

inline void exampleMultigraph(phyphox::Server& server) {
    E exp("Multi Graph Example", "Arduino Experiments", "ArduinoBLE Example");
    E::View view("Trigonometric functions");
    E::Graph graph("Trigonometric functions");
    graph.setUnitX("s"); graph.setLabelX("time"); graph.setLabelY("sin(t), cos(t)");
    graph.setChannel(0, 1); graph.setColor("eb46f4"); graph.setStyle(STYLE_LINES);
    E::Graph::Subgraph cosine; cosine.setChannel(0, 2); cosine.setStyle(STYLE_DOTS); cosine.setColor("ffffff");
    graph.addSubgraph(cosine);
    view.addElement(graph);
    exp.addView(view);
    server.addExperiment(exp);
}

inline void exampleGetDataFromSmartphone(phyphox::Server& server, int* holder = nullptr) {
    (void)holder;
    E exp("Get Data From Smartphone", "Arduino Experiments", "Control the blink interval, the LED and the mode from phyphox");
    E::View view("Controls");
    E::Edit intervalEdit("Interval", 1); intervalEdit.setUnit("ms"); intervalEdit.setDefault(100); intervalEdit.setSigned(false); intervalEdit.setDecimal(false);
    E::Slider intervalSlider("Interval", 50, 1000, 50, 1); intervalSlider.setDefault(100); intervalSlider.setPrecision(0);
    E::Toggle ledToggle("LED", 2); ledToggle.setDefault(1);
    E::Dropdown mode("Mode", 3);
    const char* const labels[] = {"Always On", "Blink", "Flash"};
    const float values[] = {1.0f, 2.0f, 3.0f};
    mode.setOptions(3, labels, values);
    E::Button reset("Reset counter", 4);
    E::Edit submitted("Submitted value", 5); submitted.submitWith(reset);
    view.addElement(intervalEdit).addElement(intervalSlider).addElement(ledToggle).addElement(mode).addElement(reset).addElement(submitted);
    exp.addView(view);
    server.addExperiment(exp);
}

inline void exampleGetSensorDataFromSmartphone(phyphox::Server& server) {
    E exp("Get Accelerometer Data", "Arduino Experiments", "Send smartphone accelerometer data to an arduino/esp32");
    E::View view("FirstView");
    E::InfoField info("Accelerometer data is sent to the board");
    E::Sensor acc("accelerometer"); acc.setAverage(true); acc.setRate(80);
    acc.mapChannel("x", 1); acc.mapChannel("y", 2); acc.mapChannel("z", 3);
    view.addElement(info);
    exp.addView(view).addSensor(acc);
    server.addExperiment(exp);
}

inline void exampleRangefinder(phyphox::Server& server) {
    E exp("Rangefinder", "Arduino Experiments", "Plot the distance from a time-of-flight sensor over time.");
    E::View view("Distance");
    E::Graph g("Distance"); g.setUnitX("s"); g.setUnitY("mm"); g.setLabelX("time"); g.setLabelY("distance");
    g.setChannel(0, 1); g.setMinY(0, LAYOUT_FIXED); g.setMaxY(2000, LAYOUT_EXTEND); g.setLogY(false); g.setFollowX(true);
    E::Value v("Distance", 1); v.setUnit("mm"); v.setPrecision(0);
    v.addMapBelow(50, "too close"); v.addMap(50, 1500, "in range"); v.addMapAbove(1500, "far");
    view.addElement(g).addElement(v);
    exp.addView(view);
    server.addExperiment(exp);
}

/// One configuration that exercises every setter once (the "everything" document).
/// The pattern of the MNU article's sketch (2026): one Graph object added to two views, three
/// graphs sharing axis settings, umlauts in labels. 1.x copied the XML at addElement(); 2.0
/// links aliases. The first view must end up with one graph, the second with three.
inline void exampleSharedGraphViews(phyphox::Server& server) {
    E exp("Magnetische Feldstärke mit Abstandsmessung", "Externer Sensor",
          "In diesem Experiment wird die Magnetische Feldstärke gegen den Abstand aufgetragen.");
    E::View first("Messung X-Komponente"), second("Messung X-Y-Z-Komponenten");
    E::Graph gx, gy, gz;
    E::Graph* graphs[] = {&gx, &gy, &gz};
    const char* labels[] = {"X-Komponente", "Y-Komponente", "Z-Komponente"};
    for (int i = 0; i < 3; ++i) {
        E::Graph& g = *graphs[i];
        g.setMinY(-500, LAYOUT_FIXED); g.setMaxY(6500, i == 0 ? LAYOUT_EXTEND : LAYOUT_FIXED);
        g.setMinX(0, LAYOUT_FIXED); g.setMaxX(400, LAYOUT_FIXED);
        g.setLabel(labels[i]); g.setUnitX("mm"); g.setUnitY("uT"); g.setLabelX("Distanz"); g.setLabelY("Mag. Feldstärke");
        g.setXPrecision(1); g.setYPrecision(1); g.setStyle(STYLE_DOTS); g.setChannel(1, 2 + i);
    }
    first.addElement(gx);
    exp.addView(first);
    second.addElement(gx); second.addElement(gy); second.addElement(gz);
    exp.addView(second);
    server.addExperiment(exp);
}

inline void exampleEverything(phyphox::Server& server) {
    E exp("Everything & \"more\"", "Arduino <Experiments>", "Every setter, once.");
    exp.setColor("ff7e22"); exp.setRepeating(0); exp.setSubscribeOnStart(true);
    E::View v1("View 1"); v1.setXMLAttribute("");
    E::Graph g("G"); g.setLabelX("x"); g.setLabelY("y"); g.setUnitX("s"); g.setUnitY("V"); g.setUnitYperX("V/s");
    g.setXPrecision(2); g.setYPrecision(3); g.setMinX(-1, LAYOUT_AUTO); g.setMaxX(10.5f, LAYOUT_EXTEND);
    g.setMinY(0.001f, LAYOUT_FIXED); g.setMaxY(1e12f, LAYOUT_FIXED);
    g.setTimeOnX(true); g.setTimeOnY(false); g.setSystemTime(true); g.setLinearTime(true); g.setLogX(true); g.setLogY(true);
    g.setFollowX(true); g.setPartialUpdate(true); g.setHideTimeMarkers(true); g.setSuppressScientificNotation(true);
    g.setShowColorScale(false); g.setAspectRatio(1.5f); g.setChannel(1, 2); g.setStyle(STYLE_MAP); g.setColor("00ff00"); g.setLineWidth(2.5f);
    g.setVisibility(3);
    E::Graph::Subgraph s2; s2.setChannel(0, 3); s2.setStyle(STYLE_HBARS); s2.setLineWidth(0.5f); g.addSubgraph(s2);
    E::Value val("Val", 2); val.setPrecision(4); val.setUnit("°C"); val.setColor("ff0000"); val.setSize(2); val.setFactor(0.001f); val.setScientific(true);
    val.addMap(0, 10, "cold");
    E::Edit ed("Ed", 1); ed.setDefault(-2.5f); ed.setSigned(true); ed.setDecimal(true); ed.setMin(-10); ed.setMax(10); ed.setUnit("u"); ed.setFactor(2);
    E::Slider sl("Sl", 0, 1, 0.1f, 2); sl.setDefault(0.5f); sl.setPrecision(1); sl.setShowValue(false); sl.setColor("0000ff");
    E::Toggle tg("Tg", 3); tg.setDefault(0);
    E::Dropdown dd("Dd", 4); dd.addOption("a", 1); dd.addOption("b & c", 2.5f); dd.setDefault(1); dd.setColor("123456");
    E::Button bt("Bt", 5); bt.setValue(7);
    E::InfoField inf("Info"); inf.setColor("101010"); inf.setSize(1.2f); inf.setAlign("center"); inf.setBold(true); inf.setItalic(true);
    E::Separator sep; sep.setHeight(0.2f); sep.setColor("202020");
    v1.addElement(g).addElement(val).addElement(ed).addElement(sl).addElement(tg).addElement(dd).addElement(bt).addElement(inf).addElement(sep);
    E::View v2("View 2");
    E::Value v2v("Other", 5);
    v2.addElement(v2v);
    E::Sensor sen("attitude"); sen.setRate(10); sen.setRateStrategy("limit"); sen.setStride(2); sen.mapChannel("x", 6); sen.mapChannel("abs", 7);
    E::Sensor custom("custom"); custom.setNameFilter("BMI160"); custom.setTypeFilter(65600); custom.mapChannel("x", 8);
    E::ExportSet set("Set"); E::ExportData d0("t", 0); E::ExportData d1("v", 1); set.addElement(d0).addElement(d1);
    exp.addView(v1).addView(v2).addSensor(sen).addSensor(custom).addExportSet(set);
    server.addExperiment(exp);
}

#endif
