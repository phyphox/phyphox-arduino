#!/usr/bin/env python3
"""Release sweep (tier T2): every shipped example, flashed as it is, loaded and started by the
real Android and iOS apps. Asserts what the example is about: the board's data arriving in
the app, or — for the examples that only receive — the board's own serial output.

    tools/bench/examples.py --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 --label esp32 \
        --android SERIAL --ios UDID [--only randomNumbers,multigraph]

Same requirements as phones.py. Results: tools/bench/results/examples-<label>.json.
"""
import argparse, json, os, socket, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from bench import SerialLog, Report, ROOT, WORKROOT, flash as flash_sketch, wait_port   # noqa: E402
from phones import Android, IOS, api_json, control, since, sh, ANDROID_PACKAGE   # noqa: E402
import asyncio
from bleak import BleakScanner
CAPTURE_DIR = None; CAPTURE_TAG = ""
import subprocess

# example -> (advertised name, expectation)
#   data            the board streams: CH1 fills within 15 s of the start
#   serial:<text>   the board prints <text> within 15 s of the start (receive-only examples)
#   set:<buf>=<v>:<text>  after the start, the phone writes <v> into <buf>; the board prints <text>
EXAMPLES = {
    "randomNumbers":               ("phyphox device",  "data"),
    "CreateExperiment":            ("create experiment", "data"),
    "multigraph":                  ("phyphox-Arduino", "data"),
    "connectionParameter":         ("phyphox-Arduino", "data"),
    "readoutADC":                  ("Voltmeter",       "data"),
    "buttonsAndCallbacks":         ("phyphox buttons", "data"),
    "getSystemAndEventTime":       ("phyphox-Arduino", "serial:New experiment event received"),
    "getSensorDataFromSmartphone": ("phyphox-Arduino", "serial:x: "),
    "getDataFromSmartphone":       ("phyphox-Arduino", "set:in1=300:Mode: "),
}

def flash_example(fqbn, port, example, extra):
    cmd = ["arduino-cli", "compile", "--upload", "-p", port, "--fqbn", fqbn, "--library", ROOT]
    if extra: cmd += ["--build-property", "compiler.cpp.extra_flags=" + extra]
    cmd.append(os.path.join(ROOT, "examples", example))
    r = subprocess.run(cmd, capture_output=True, text=True)
    return r.returncode == 0, (r.stdout + r.stderr)[-800:]

def run_example(example, name, expect, phones, port, rep):
    log = SerialLog(port); time.sleep(2)
    try:
        for phone in phones:
            p = phone.name(); tag = f"{example} / {p}"
            mark = len(log.lines)
            try:
                if not phone.connect(name, rep, timeout=120):
                    continue
                if CAPTURE_DIR and p == "android":
                    # the document exactly as the phone received it (Android keeps the transfer
                    # at files/temp_bt/bt.phyphox; the debug build lets run-as read it) — the
                    # way the phyphox-docs corpus freezes library-generated XML
                    r = sh(phone.adb + ["shell", "run-as", ANDROID_PACKAGE, "cat", "files/temp_bt/bt.phyphox"], timeout=30)
                    body = (r.stdout or "") if r.returncode == 0 else ""
                    ok = body.startswith("<phyphox")
                    if ok:
                        os.makedirs(CAPTURE_DIR, exist_ok=True)
                        open(os.path.join(CAPTURE_DIR, f"{example}-{CAPTURE_TAG}.phyphox"), "w").write(body)
                    rep.check(f"{tag}: received document captured", ok, f"{len(body)} bytes")
                time.sleep(4)
                started = False
                for _ in range(30):                 # iOS can take >15 s to reconnect for the experiment
                    control(phone.base, "start"); time.sleep(1)
                    st, d = api_json(phone.base, "/get?CH1")
                    if d and d.get("status", {}).get("measuring"): started = True; break
                rep.check(f"{tag}: started", started)
                if not started: continue
                if expect == "data":
                    n = 0
                    for _ in range(15):
                        st, d = api_json(phone.base, "/get?CH1=full", timeout=10)
                        n = len(((d or {}).get("buffer", {}).get("CH1", {}) or {}).get("buffer", []) or [])
                        if n >= 10: break
                        time.sleep(1)
                    rep.check(f"{tag}: data arrives", n >= 10, f"{n} values in CH1")
                elif expect.startswith("serial:"):
                    text = expect[7:]
                    rep.check(f"{tag}: board printed '{text.strip()}'", since(log, mark, lambda l: text in l, 15) is not None)
                elif expect.startswith("set:"):
                    _, assignment, text = expect.split(":", 2)
                    buf, val = assignment.split("=")
                    mark2 = len(log.lines)
                    control(phone.base, "set", buffer=buf, value=val)
                    rep.check(f"{tag}: board printed '{text.strip()}' after {assignment}", since(log, mark2, lambda l: text in l, 10) is not None)
                control(phone.base, "stop")
            finally:
                phone.release(rep)
                time.sleep(3)
    finally:
        log.close()

def names_in_the_air(names, seconds=6):
    """Which of `names` some device is advertising right now. Every board on the desk has just
    been flashed quiet, so a hit is a foreign device — and the phone would connect to it."""
    async def scan():
        found = await BleakScanner.discover(timeout=seconds)
        return sorted({d.name for d in found if d.name in names})
    return asyncio.run(scan())

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--fqbn", required=True); ap.add_argument("--port", required=True); ap.add_argument("--label", required=True)
    ap.add_argument("--android"); ap.add_argument("--ios"); ap.add_argument("--only")
    ap.add_argument("--android-port", type=int, default=8091); ap.add_argument("--ios-port", type=int, default=8081)
    ap.add_argument("--capture", metavar="DIR", help="save the document as the Android phone received it, <example>-<label>.phyphox")
    ap.add_argument("--silence", action="append", default=[], metavar="PORT=FQBN",
                    help="another board on the desk: flash it quiet first, so it does not advertise an example's name")
    args = ap.parse_args()
    global CAPTURE_DIR, CAPTURE_TAG
    CAPTURE_DIR, CAPTURE_TAG = args.capture, args.label
    lock = os.path.join(WORKROOT, ".bench-lock-" + socket.gethostname())
    if os.path.exists(lock): sys.exit(f"bench in use: {open(lock).read().strip()}")
    open(lock, "w").write(f"phyphox-arduino examples {args.label} pid {os.getpid()} since {time.strftime('%Y-%m-%d %H:%M')}\n")
    rep = Report()
    try:
        phones = []
        if args.android: phones.append(Android(args.android, args.android_port))
        if args.ios: phones.append(IOS(args.ios, args.ios_port))
        names = [e.strip() for e in args.only.split(",")] if args.only else list(EXAMPLES)
        # Nobody else may advertise an example's name: the board under test and every board
        # named in --silence go quiet, then a scan must find none of the names in the air.
        for spec in args.silence:
            sport, sfqbn = spec.split("=", 1)
            print(f"== silencing {sport} ({sfqbn})")
            flash_sketch(sfqbn, sport, "", sketch="quiet")
        print(f"== silencing the board under test ({args.port})")
        flash_sketch(args.fqbn, args.port, "", sketch="quiet")
        time.sleep(3)
        advertised = {EXAMPLES[e][0] for e in names}
        foreign = names_in_the_air(advertised)
        rep.check("no other device advertises an example's name", not foreign, ", ".join(foreign))
        if foreign:
            sys.exit(f"another device advertises {foreign}: silence it (--silence PORT=FQBN, or unplug it)")
        for example in names:
            name, expect = EXAMPLES[example]
            extra = "-DLED_BUILTIN=2" if (args.fqbn.startswith("esp32:esp32:esp32") and example == "getDataFromSmartphone") else ""
            print(f"== {example} ({name})")
            ok, out = flash_example(args.fqbn, args.port, example, extra)
            rep.check(f"{example}: compiles and flashes", ok, "" if ok else out[-300:])
            if not ok: continue
            wait_port(args.port); time.sleep(3)
            run_example(example, name, expect, phones, args.port, rep)
    finally:
        os.remove(lock)
    out = os.path.join(HERE, "results", "examples-" + args.label + ".json")
    json.dump({"label": args.label, "fqbn": args.fqbn, "checks": rep.checks, "failed": rep.failed, "when": time.strftime("%Y-%m-%d %H:%M")}, open(out, "w"), indent=1)
    print(f"\n{len(rep.checks) - rep.failed} of {len(rep.checks)} checks passed → {out}")
    sys.exit(1 if rep.failed else 0)

if __name__ == "__main__":
    main()
