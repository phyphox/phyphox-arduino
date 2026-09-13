# Export

The app's share button exports the recorded channels as CSV or Excel. Without an export set the
library exports every data channel under generic names (`myData0` … `myData5`). To name and
select them:

```cpp
PhyphoxBleExperiment::ExportSet set("Measurement");
PhyphoxBleExperiment::ExportData t("Time (s)", 0);        // channel 0: the phone's time axis
PhyphoxBleExperiment::ExportData v("Voltage (V)", 1);     // data channel 1
set.addElement(t).addElement(v);
exp.addExportSet(set);
```

Several sets become several sheets or files. `ExportData(label, channel)` takes the same
channels as a graph: 0 is the phone's arrival time, 1…5 the values of `PhyphoxBLE::write`.
The 1.x spelling `setDatachannel(n)`, its alias `setDataChannel(n)`, and `setLabel` remain.
