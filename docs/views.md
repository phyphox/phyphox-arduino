# Views and graphs

An experiment has one or more **views**, shown as tabs in the app. A view holds its elements
in the order they are added. Every class lives inside `PhyphoxBleExperiment`; the examples
write `PhyphoxBleExperiment::Graph` in full.

```cpp
PhyphoxBleExperiment exp("Voltmeter", "Arduino Experiments", "Plots the voltage on A0.");
PhyphoxBleExperiment::View  view("Data");
PhyphoxBleExperiment::Graph graph("Voltage");
graph.setChannel(0, 1);                  // x: time (channel 0), y: data channel 1
graph.setLabelX("Time");   graph.setUnitX("s");
graph.setLabelY("Voltage"); graph.setUnitY("V");
PhyphoxBleExperiment::Value value("Now", 1);
value.setUnit("V"); value.setPrecision(2);
view.addElement(graph).addElement(value);
exp.addView(view);
PhyphoxBLE::start("Voltmeter", exp);
```

All objects can be locals in `setup()`: `PhyphoxBLE::start()` and `addExperiment()` copy the
description. Strings must be literals or static buffers ([Concepts](concepts.md#strings)).
Every setter that receives something the format cannot express records an error that the app
shows ([Concepts](concepts.md#errors)).

## View

| method | effect |
|---|---|
| `View()`, `View(label)` | a tab; `setLabel` names it |
| `addElement(element)` | appends an element; returns the view, so calls chain |
| `setXMLAttribute(text)` | raw attribute text on the `<view>` tag |

## Graph

Plots one data channel against another. Channel 0 is the time at which the phone received a
notification; 1…5 are the values of `PhyphoxBLE::write(...)`.

| method | effect |
|---|---|
| `Graph()`, `Graph(label)`, `setLabel` | the title above the graph |
| `setChannel(x, y)` | the first curve: `setChannel(0, 1)` is channel 1 over time |
| `setLabelX`, `setLabelY`, `setUnitX`, `setUnitY`, `setUnitYperX` | axis labels and units |
| `setXPrecision(n)`, `setYPrecision(n)` | digits on the axes |
| `setStyle(STYLE_LINES \| STYLE_DOTS \| STYLE_VBARS \| STYLE_HBARS \| STYLE_MAP)` | how the first curve is drawn |
| `setColor("ff7e22")`, `setLineWidth(w)` (1.x spelling `setLinewidth`) | colour (six hex digits; `COLOR_*` constants exist) and width of the first curve |
| `setMinX(v, layout)`, `setMaxX`, `setMinY`, `setMaxY` | axis range; layout is `LAYOUT_AUTO`, `LAYOUT_EXTEND` (grows past v) or `LAYOUT_FIXED` |
| `setLogX(b)`, `setLogY(b)` | logarithmic axes |
| `setFollowX(b)` | the visible x range follows the newest data |
| `setTimeOnX(b)`, `setTimeOnY(b)`, `setSystemTime(b)`, `setLinearTime(b)` | show the axis as a time axis, wall-clock time, or a linear time base |
| `setAspectRatio(r)` | width over height of the plot area |
| `setPartialUpdate(b)`, `setHideTimeMarkers(b)`, `setSuppressScientificNotation(b)`, `setShowColorScale(b)` | further attributes of the phyphox `graph` element |
| `addSubgraph(sub)` | a further curve (below); `addChannel(x, y, color)` is the 1.x one-call form for the first curve |
| `setVisibility(channel)`, `setXMLAttribute(text)` | as for every element |

### Several curves

```cpp
PhyphoxBleExperiment::Graph graph("sin and cos");
graph.setChannel(0, 1); graph.setColor("ff7e22");
PhyphoxBleExperiment::Graph::Subgraph cosine;
cosine.setChannel(0, 2); cosine.setStyle(STYLE_DOTS); cosine.setColor("39a2ff");
graph.addSubgraph(cosine);
```

A `Subgraph` has `setChannel`, `setStyle`, `setColor` and `setLineWidth`. The graph keeps a
pointer to it, so declare it where it lives until `start()` / `addExperiment()`.

## Value

Shows the current value of a data channel as a number.

| method | effect |
|---|---|
| `Value()`, `Value(label, channel)`, `setLabel`, `setChannel(1…5)` | which channel, under which caption |
| `setUnit`, `setPrecision(n)`, `setScientific(b)`, `setFactor(f)` | unit, digits, notation, a multiplier applied before display |
| `setColor`, `setSize(s)` | colour and relative font size |
| `addMap(min, max, "text")`, `addMapBelow(max, "text")`, `addMapAbove(min, "text")` | show a text instead of the number while the value is in the range; the first match wins. Up to 8 this way; `setMaps(n, mins, maxs, texts)` takes arrays of any length |

```cpp
PhyphoxBleExperiment::Value state("State", 1);
state.addMapBelow(20, "cold");
state.addMap(20, 30, "fine");
state.addMapAbove(30, "hot");
```

## InfoField

A line of text. `InfoField(text)` or `setInfo(text)`; `setColor`, `setSize(s)`,
`setAlign("left" | "center" | "right")`, `setBold(b)`, `setItalic(b)`.

## Separator

A horizontal rule between elements: `setHeight(h)` (relative, default 0.1), `setColor`.

## Visibility

Every element has `setVisibility(inputChannel)`: it is shown only while the last value in that
input channel is greater than zero. Pair it with a `Toggle` on the same channel to let the
user show and hide parts of the view.

## The escape hatch

`setXMLAttribute("logY=\"true\"")` appends raw attribute text to the element's tag. The library
does not check it; the phyphox file-format documentation says what an element accepts. Useful
for the rare attribute the API does not model — and a way to produce a document the app
refuses, so `PhyphoxBLE::printXML(&Serial)` is the first thing to look at when that happens.
