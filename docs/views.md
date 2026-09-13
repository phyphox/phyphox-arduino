# Views and graphs

*Skeleton — filled in during phase 3 with the ported examples.*

An experiment has one or more **views** (tabs). A view holds elements in the order they are
added:

```cpp
PhyphoxBleExperiment exp("Voltmeter", "Arduino Experiments", "Plots the voltage on A0.");
PhyphoxBleExperiment::View  view("Data");
PhyphoxBleExperiment::Graph graph("Voltage");
graph.setChannel(0, 1);                  // x: time, y: channel 1
graph.setLabelX("Time"); graph.setUnitX("s");
graph.setLabelY("Voltage"); graph.setUnitY("V");
PhyphoxBleExperiment::Value value("Now", 1);
value.setUnit("V"); value.setPrecision(2);
view.addElement(graph).addElement(value);
exp.addView(view);
PhyphoxBLE::start("Voltmeter", exp);
```

## Graph

Setters: `setLabel`, `setLabelX/Y`, `setUnitX/Y`, `setUnitYperX`, `setXPrecision/YPrecision`,
`setStyle(STYLE_LINES | STYLE_DOTS | STYLE_VBARS | STYLE_HBARS | STYLE_MAP)`, `setColor("ff7e22")`,
`setLineWidth`, `setMinX/MaxX/MinY/MaxY(value, LAYOUT_AUTO | LAYOUT_EXTEND | LAYOUT_FIXED)`,
`setLogX/LogY`, `setFollowX`, `setTimeOnX/TimeOnY`, `setSystemTime`, `setLinearTime`,
`setAspectRatio`, `setShowColorScale`, `setPartialUpdate`, `setHideTimeMarkers`,
`setSuppressScientificNotation`.

Several curves: `addSubgraph(sub)` with a `Graph::Subgraph` configured through `setChannel`,
`setColor`, `setStyle`, `setLineWidth`; or `addChannel(x, y, color)` in one call.

## Value

`setChannel(1…5)`, `setUnit`, `setPrecision`, `setColor`, `setSize`, `setFactor`,
`setScientific`; `addMap(min, max, "text")` shows a text instead of the number while the value
is in the range (status displays: "OK" / "HOT").

## InfoField and Separator

`InfoField("text")` with `setColor`, `setSize`, `setAlign("left"|"center"|"right")`, `setBold`,
`setItalic`. `Separator` with `setHeight`, `setColor`.

## Visibility

Every element has `setVisibility(inputChannel)`: it is shown only while the last value in that
input channel is greater than zero — pair it with a `Toggle` on the same channel.

## Escape hatch

`setXMLAttribute("logY=\"true\"")` appends raw attribute text to the element. The library does
not check it; the file-format documentation says what is allowed.
