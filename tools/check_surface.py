#!/usr/bin/env python3
"""Check api/surface.yml against the phyphox file-format spec in ../phyphox-docs/spec.

Every XML element and attribute the library claims to emit must exist in the spec, every
enumerated value the library accepts must be one the spec allows, and nothing may need a
format version newer than surface.yml's format_version. Exit 1 on any finding.

Usage: tools/check_surface.py [--docs PATH]   (default: ../phyphox-docs)
"""
import argparse, glob, os, sys
try:
    import yaml
except ImportError:
    sys.exit("needs pyyaml (pip install pyyaml)")

def load_spec(docs):
    """Return {path: element} where path is block/parent/.../name as the spec's parent chain."""
    elements = {}
    by_name = {}
    for f in sorted(glob.glob(os.path.join(docs, "spec", "*.yml"))):
        y = yaml.safe_load(open(f))
        if not isinstance(y, dict) or "elements" not in y:
            continue
        for el in y["elements"]:
            by_name.setdefault(el.get("name"), []).append(el)
    # resolve full paths by walking parents; a name may repeat under different parents
    def path_of(el, seen=0):
        parent = el.get("parent")
        if not parent or seen > 10:
            return el["name"]
        # pick the parent that lists this element as a child, else the first
        candidates = by_name.get(parent, [])
        chosen = next((p for p in candidates if el["name"] in (p.get("children") or [])), candidates[0] if candidates else None)
        return (path_of(chosen, seen + 1) + "/" if chosen else parent + "/") + el["name"]
    for name, els in by_name.items():
        for el in els:
            elements[path_of(el)] = el
    # views.yml: attributes common to every view element (label, visibility, …) live in
    # `common`, not on each element — merge them in.
    views = yaml.safe_load(open(os.path.join(docs, "spec", "views.yml")))
    common = (views.get("common") or {}).get("view_element_attributes") or []
    for path, el in elements.items():
        # paths carry the root: phyphox/views/view/<element>
        if "views/view/" in path and path.split("views/view/")[1].count("/") == 0:
            if not el.get("attributes"):
                el["attributes"] = []
            names = {a.get("name") for a in el["attributes"]}
            el["attributes"] += [a for a in common if a.get("name") not in names]
    # slot children: an element with `outputs: {attribute, components}` (the sensor) models its
    # <output component=…> children as slots, not as a separate element — synthesise one.
    for path, el in list(elements.items()):
        slots = el.get("outputs")
        if isinstance(slots, dict) and slots.get("components"):
            elements[path + "/output"] = {
                "name": "output", "since": el.get("since"),
                "attributes": [{"name": slots.get("attribute", "component"),
                                "values": [c.get("name") for c in slots["components"]]}],
            }
    return elements

def version_tuple(v):
    return tuple(int(x) for x in str(v).split("."))

def attr(el, name):
    for a in el.get("attributes") or []:
        if a.get("name") == name:
            return a
    return None

def allowed_values(a):
    vals = a.get("values") or []
    out = []
    for v in vals:
        out.append(v.get("value") if isinstance(v, dict) else v)
    return out

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--docs", default=os.path.join(os.path.dirname(__file__), "..", "..", "phyphox-docs"))
    ap.add_argument("--surface", default=os.path.join(os.path.dirname(__file__), "..", "api", "surface.yml"))
    args = ap.parse_args()
    if not os.path.isdir(os.path.join(args.docs, "spec")):
        sys.exit(f"phyphox-docs not found at {args.docs} (need its spec/ directory)")
    spec = load_spec(args.docs)
    surface = yaml.safe_load(open(args.surface))
    fmt = version_tuple(surface["format_version"])
    findings = []

    def resolve(path):
        # spec paths start with the block name (root block elements are just "phyphox/…")
        for cand in (path, "phyphox/" + path):
            if cand in spec:
                return spec[cand]
        return None

    def check_attr(path, el, name, values=None, where=""):
        if name == "*":
            return
        a = attr(el, name)
        if a is None:
            findings.append(f"{where}: attribute '{name}' does not exist on <{path}>")
            return
        since = a.get("since") or el.get("since")
        if since and version_tuple(since) > fmt:
            findings.append(f"{where}: '{name}' on <{path}> needs format {since} > {surface['format_version']}")
        if values:
            allowed = allowed_values(a)
            if allowed:
                bad = [v for v in values if v not in allowed]
                if bad:
                    findings.append(f"{where}: values {bad} for '{name}' on <{path}> not in spec {allowed}")

    def check_element(path, where):
        el = resolve(path)
        if el is None:
            findings.append(f"{where}: element <{path}> does not exist in the spec")
            return None
        since = el.get("since")
        if since and version_tuple(since) > fmt:
            findings.append(f"{where}: <{path}> needs format {since} > {surface['format_version']}")
        return el

    # document-level emits
    for e in surface.get("document", {}).get("emits", []):
        path = e["element"]
        el = check_element(path, "document")
        if el:
            for a in e.get("attributes", []):
                check_attr(path, el, a, where=f"document <{path}>")

    for cls, spec_entry in surface.get("elements", {}).items():
        base = spec_entry["xml"]
        el = check_element(base, cls)
        if el is None:
            continue
        for setter, s in (spec_entry.get("setters") or {}).items():
            where = f"{cls}::{setter}"
            path, target = base, el
            if "xml" in s:
                path = s["xml"]; target = check_element(path, where)
                if target is None: continue
            if "child" in s:
                path = f"{path}/{s['child']}"; target = check_element(path, where)
                if target is None: continue
            names = s.get("attributes") or ([s["attribute"]] if "attribute" in s else [])
            for n in names:
                vals = s.get("values") if (s.get("check") == "subset" and (s.get("of") in (None, n))) else None
                check_attr(path, target, n, vals, where)
        for e in spec_entry.get("emits", []):
            path = base + ("/" + e["child"] if "child" in e else "")
            check_element(path, f"{cls} emits")

    if findings:
        print("\n".join(findings))
        print(f"\n{len(findings)} finding(s)")
        sys.exit(1)
    print(f"surface.yml: {len(surface.get('elements', {}))} classes checked against the spec, no findings (format {surface['format_version']})")

if __name__ == "__main__":
    main()
