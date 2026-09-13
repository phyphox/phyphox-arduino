#!/usr/bin/env python3
"""Every public method of the two public headers must be named somewhere in docs/*.md
(test-matrix row api-doc-parity). Exit 1 with the missing names otherwise."""
import os, re, sys
ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), ".."))
headers = [os.path.join(ROOT, "src", "phyphoxBle.h"), os.path.join(ROOT, "src", "phyphoxBleExperiment.h")]
docs = ""
for fn in os.listdir(os.path.join(ROOT, "docs")):
    if fn.endswith(".md"): docs += open(os.path.join(ROOT, "docs", fn)).read()
docs += open(os.path.join(ROOT, "README.md")).read()
names = set()
for h in headers:
    text = open(h).read()
    for m in re.finditer(r"^\s*(?:static\s+)?(?:virtual\s+)?[\w:<>&*\s]+?\b(\w+)\s*\([^;{]*\)\s*(?:const)?\s*(?:override)?\s*[;{]", text, re.M):
        name = m.group(1)
        if name[0].isupper() or name in ("if", "for", "while", "switch", "return", "sizeof", "defined"): continue
        names.add(name)
internal = {"data", "input", "isExportData", "instance", "server", "next", "lastEvent", "stats", "version", "transportName", "begin"}
missing = sorted(n for n in names if n not in internal and not re.search(r"\b" + re.escape(n) + r"\b", docs))
if missing:
    print("public methods not mentioned in docs/ or README.md:\n  " + "\n  ".join(missing))
    sys.exit(1)
print(f"api docs: {len(names)} public methods, all documented")
