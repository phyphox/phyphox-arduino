#!/usr/bin/env python3
"""Conformance of the generated documents against the phyphox file-format spec (tier T0).

Builds the host tools, writes every example configuration as a .phyphox document, then checks
each document the way the phyphox-docs build checks its corpus: the RELAX NG grammar, the
Schematron rules (its warnings — the version gates — count as failures here, because the
library declares the version it emits), and tools/validate_experiments.py. Finally runs
tools/check_surface.py. Exit 1 on any finding.

    tests/conformance/run.py [--docs ../phyphox-docs] [--skip-build] [--out DIR]

Needs phyphox-docs checked out beside this repository (its spec/, tools/ and validators), and
lxml + pyyaml (+ rnc2rng if the validators have to be regenerated). Uses phyphox-docs' .venv
python when present.
"""
import argparse, os, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--docs", default=os.path.join(ROOT, "..", "phyphox-docs"))
    ap.add_argument("--build-dir", default=os.path.join(ROOT, "tests", "host", "build"))
    ap.add_argument("--out", default=os.path.join(HERE, "out"))
    ap.add_argument("--skip-build", action="store_true")
    args = ap.parse_args()
    docs = os.path.abspath(args.docs)
    if not os.path.isdir(os.path.join(docs, "spec")):
        sys.exit(f"phyphox-docs not found at {docs}")

    # re-exec inside the docs venv if we are not already using a python that has lxml
    venv_py = os.path.join(docs, ".venv", "bin", "python")
    try:
        import lxml  # noqa: F401
    except ImportError:
        if os.path.exists(venv_py) and os.path.abspath(sys.executable) != os.path.abspath(venv_py):
            os.execv(venv_py, [venv_py] + sys.argv)
        sys.exit("needs lxml (pip install lxml pyyaml)")
    from lxml import etree
    from lxml.isoschematron import Schematron

    if not args.skip_build:
        subprocess.check_call(["cmake", "-S", os.path.join(ROOT, "tests", "host"), "-B", args.build_dir, "-DCMAKE_BUILD_TYPE=Release"],
                              stdout=subprocess.DEVNULL)
        subprocess.check_call(["cmake", "--build", args.build_dir, "-j"], stdout=subprocess.DEVNULL)
    os.makedirs(args.out, exist_ok=True)
    subprocess.check_call([os.path.join(args.build_dir, "gen_examples"), args.out])

    # validators: the docs build publishes them under docs/assets/validators; regenerate if absent
    vdir = os.path.join(docs, "docs", "assets", "validators")
    if not os.path.exists(os.path.join(vdir, "phyphox.rng")):
        sys.path.insert(0, os.path.join(docs, "tools"))
        import generate_validators as gv
        vdir, _ = gv.generate()
    rng = etree.RelaxNG(etree.parse(os.path.join(vdir, "phyphox.rng")))
    sch = Schematron(etree.parse(os.path.join(vdir, "phyphox.sch")), store_report=True)
    svrl = "{http://purl.oclc.org/dsdl/svrl}failed-assert"

    problems = []
    files = sorted(f for f in os.listdir(args.out) if f.endswith(".phyphox"))
    for fn in files:
        path = os.path.join(args.out, fn)
        tree = etree.parse(path)
        if not rng.validate(tree):
            for e in rng.error_log:
                problems.append(f"{fn}: RELAX NG: line {e.line}: {e.message}")
        sch.validate(tree)
        for fa in sch.validation_report.iter(svrl):
            text = (fa.findtext("{http://purl.oclc.org/dsdl/svrl}text") or "").strip()
            role = fa.get("role") or "error"
            problems.append(f"{fn}: Schematron {role}: {text}")

    # the spec-driven checker of phyphox-docs (a finding may be a spec bug — report either way)
    r = subprocess.run([sys.executable, os.path.join(docs, "tools", "validate_experiments.py"), "--detail", args.out],
                       capture_output=True, text=True)
    # it reports success by phrase, not by exit code
    if r.returncode != 0 or "Nothing to report" not in r.stdout:
        problems.append("validate_experiments.py:\n" + r.stdout.strip() + ("\n" + r.stderr.strip() if r.stderr.strip() else ""))

    r = subprocess.run([sys.executable, os.path.join(ROOT, "tools", "check_surface.py"), "--docs", docs], capture_output=True, text=True)
    if r.returncode != 0:
        problems.append("check_surface.py:\n" + r.stdout.strip())

    print(f"{len(files)} documents validated against {vdir}")
    if problems:
        print("\n".join(problems))
        print(f"\n{len(problems)} finding(s)")
        sys.exit(1)
    print("conformance: no findings")

if __name__ == "__main__":
    main()
