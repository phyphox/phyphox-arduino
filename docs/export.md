# Export

*Skeleton — filled in during phase 3.*

The app's export button saves the recorded channels as CSV/Excel. By default the library
exports every data channel; to name and select them:

```cpp
PhyphoxBleExperiment::ExportSet set("Measurement");
PhyphoxBleExperiment::ExportData t("Time (s)", 0);
PhyphoxBleExperiment::ExportData v("Voltage (V)", 1);
set.addElement(t).addElement(v);
exp.addExportSet(set);
```
