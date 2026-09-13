#!/usr/bin/env python3
"""The phones (tier T2): a real phyphox app on an Android phone and on an iPhone against a
board running tools/bench/benchSketch, driven from this Linux host.

The connect step uses the apps' development seams (Android: the BleCompatConnectTest
instrumentation with bleDevice/holdForHost, remote access via debug.phyphox.remote; iOS: the
-phyphoxBleConnect / -phyphoxRemote / -phyphoxAutoConfirm launch arguments, launched through
pymobiledevice3 over the USB tunnel). Everything after it goes through the remote-access API
(phyphox-docs/docs/remote-interface/openapi.yaml), with the board's serial echo as the other
half of every assertion. Results: tools/bench/results/phones-<label>.json.

    tools/bench/phones.py --board-port /dev/ttyUSB0 --board-name bench_esp32 --label esp32 \
        [--android SERIAL] [--ios UDID]

Needs: adb, the debug + androidTest APKs installed on the Android phone; the development
build on the iPhone, developer mode, and `sudo pymobiledevice3 remote tunneld` running.
"""
import argparse, json, os, shlex, socket, subprocess, sys, time, urllib.request, urllib.error, urllib.parse

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from bench import SerialLog, Report, ROOT, WORKROOT   # noqa: E402

ANDROID_PACKAGE = "de.rwth_aachen.phyphox"
IOS_BUNDLE = "de.rwth-aachen.physics.phyphox"
PY = sys.executable

def sh(cmd, timeout=60, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, **kw)

def api(base, path, timeout=5):
    try:
        with urllib.request.urlopen(base + path, timeout=timeout) as r:
            return r.status, r.read()
    except urllib.error.HTTPError as e:
        return e.code, e.read()
    except Exception as e:
        return None, str(e).encode()

def api_json(base, path, timeout=5):
    status, body = api(base, path, timeout)
    try: return status, json.loads(body)
    except Exception: return status, None

def control(base, cmd, **params):
    q = "&".join([f"cmd={cmd}"] + [f"{k}={urllib.parse.quote(str(v))}" for k, v in params.items()])
    status, data = api_json(base, "/control?" + q)
    return bool(data and data.get("result"))

def wait_api(base, seconds):
    end = time.time() + seconds
    while time.time() < end:
        status, _ = api(base, "/config", timeout=3)
        if status == 200: return True
        time.sleep(1)
    return False

# ---------------------------------------------------------------- the phones

class Android:
    def __init__(self, serial, port):
        self.serial, self.port, self.base = serial, port, f"http://127.0.0.1:{port}"
        self.adb = ["adb", "-s", serial]
        self.proc = None
    def name(self): return "android"
    def connect(self, board_name, rep, timeout=120):
        r = sh(["adb", "forward", "--list"])
        for line in r.stdout.splitlines():
            parts = line.split()
            if len(parts) >= 2 and parts[1] == f"tcp:{self.port}": sh(["adb", "-s", parts[0], "forward", "--remove", f"tcp:{self.port}"])
        sh(self.adb + ["forward", f"tcp:{self.port}", "tcp:8080"])
        sh(self.adb + ["shell", "am", "force-stop", ANDROID_PACKAGE])
        sh(self.adb + ["shell", "setprop", "debug.phyphox.remote", "1"])
        sh(self.adb + ["shell", "setprop", "debug.phyphox.autoConfirm", "1"])   # else the save-locally offer blocks the BLE connect
        sh(self.adb + ["shell", "setprop", "debug.phyphox.labRelease", "0"])
        sh(self.adb + ["logcat", "-c"])
        self.proc = subprocess.Popen(self.adb + ["shell", "am", "instrument", "-w",
                                                 "-e", "class", "de.rwth_aachen.phyphox.BleCompatConnectTest",
                                                 "-e", "bleDevice", shlex.quote(board_name),
                                                 "-e", "holdForHost", "true",
                                                 "de.rwth_aachen.phyphox.test/androidx.test.runner.AndroidJUnitRunner"],
                                     stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        end = time.time() + timeout
        while time.time() < end:
            r = sh(self.adb + ["logcat", "-d", "-s", "phyphoxBleCompat"], timeout=30)
            if "holding the app open" in (r.stdout or ""):
                rep.check("android: connect test loaded the board's experiment and is holding", True)
                return wait_api(self.base, 20)
            if self.proc.poll() is not None: break
            time.sleep(1)
        out = ""
        if self.proc.poll() is not None: out = (self.proc.communicate()[0] or "")[-600:]
        else: self.proc.kill()
        rep.check("android: connect test loaded the board's experiment and is holding", False, out.strip())
        return False
    def release(self, rep):
        if not self.proc: return
        sh(self.adb + ["shell", "setprop", "debug.phyphox.labRelease", "1"])
        try: out = self.proc.communicate(timeout=60)[0] or ""
        except subprocess.TimeoutExpired:
            self.proc.kill(); out = self.proc.communicate()[0] or ""
        sh(self.adb + ["shell", "setprop", "debug.phyphox.labRelease", "0"])
        sh(self.adb + ["shell", "setprop", "debug.phyphox.remote", "0"])
        sh(self.adb + ["shell", "setprop", "debug.phyphox.autoConfirm", "0"])
        rep.check("android: connect test finished without failures", self.proc.returncode == 0 and "FAILURES" not in out, out[-300:].strip())
        sh(self.adb + ["shell", "am", "force-stop", ANDROID_PACKAGE])

class IOS:
    def __init__(self, udid, port):
        self.udid, self.port, self.base = udid, port, f"http://127.0.0.1:{port}"
        self.forward = None
    def name(self): return "ios"
    def connect(self, board_name, rep, timeout=120):
        self.forward = subprocess.Popen([PY, "-m", "pymobiledevice3", "usbmux", "forward", str(self.port), "80", "--serial", self.udid],
                                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(1)
        # one string: this tool shlex-splits the whole command line itself
        line = " ".join(shlex.quote(a) for a in [IOS_BUNDLE, "-phyphoxBleConnect", board_name, "-phyphoxRemote",
                                                    "-phyphoxRemotePort", "80", "-phyphoxAutoConfirm"])
        r = sh([PY, "-m", "pymobiledevice3", "developer", "dvt", "launch", "--kill-existing", "--udid", self.udid, line], timeout=120)
        launched = r.returncode == 0
        rep.check("ios: app launched with the seams through the tunnel", launched, (r.stdout or r.stderr or "").strip()[-300:])
        if not launched: return False
        ok = wait_api(self.base, timeout)
        rep.check("ios: remote API answering (experiment loaded from the board)", ok)
        return ok
    def release(self, rep):
        sh([PY, "-m", "pymobiledevice3", "developer", "dvt", "pkill", "phyphox", "--udid", self.udid], timeout=60)
        if self.forward: self.forward.terminate()

# ---------------------------------------------------------------- the sequence

def expect_line(log, predicate, timeout):
    return log.wait_for(predicate, timeout)

def since(log, index, predicate, timeout=0):
    """A line matching `predicate` among those logged after `index` (polling up to `timeout`)."""
    end = time.time() + timeout
    while True:
        for l in log.lines[index:]:
            if predicate(l): return l
        if time.time() >= end: return None
        time.sleep(0.1)

def sequence(phone, base, log, rep):
    p = phone.name()
    start_index = len(log.lines)
    status, data = api_json(base, "/get?CH1")
    rep.check(f"{p}: loaded but not measuring", data is not None and data.get("status", {}).get("measuring") is False)
    # the app connects to the board for the experiment on its own a moment after loading;
    # the board reports that link on serial, and a start before it is refused
    link = expect_line(log, lambda l: l.startswith("LINK connections=1"), 30)
    rep.check(f"{p}: app connected to the board for the experiment", link is not None, link or "no LINK line within 30 s")
    time.sleep(2)
    log.drain()
    started = False
    for _ in range(10):
        if control(base, "start"): started = True; break
        time.sleep(1)
    rep.check(f"{p}: start accepted", started)
    if not started: return
    ev = expect_line(log, lambda l: l.startswith("EVENT 1 "), 5)
    rep.check(f"{p}: START event reached the board", ev is not None, ev or "")
    # data flowing: CH1 grows
    end = time.time() + 15; n = 0
    while time.time() < end:
        status, data = api_json(base, "/get?CH1=full", timeout=10)
        n = len(((data or {}).get("buffer", {}).get("CH1", {}) or {}).get("buffer", []) or [])
        if n >= 20: break
        time.sleep(1)
    rep.check(f"{p}: board data arrives in the app", n >= 20, f"{n} values in CH1")
    # the change detector must not have sent the defaults
    # channels 6-8 carry the phone's accelerometer and change all the time; 1-5 must be silent
    stray = [l for l in log.lines[start_index:] if l.startswith("CHANGE") and l.split()[1] in ("1", "2", "3", "4", "5")]
    rep.check(f"{p}: no CHANGE on input channels before any input (defaults are not sent)", not stray, str(stray[:3]))
    # edit → change → same → change again
    log.drain(); control(base, "set", buffer="in1", value=250)
    rep.check(f"{p}: set in1=250 → CHANGE 1", expect_line(log, lambda l: l == "CHANGE 1 250.000", 5) is not None)
    log.drain(); control(base, "set", buffer="in1", value=250)
    again = expect_line(log, lambda l: l.startswith("CHANGE 1"), 3)
    rep.check(f"{p}: setting the same value again sends nothing (change-only)", again is None, again or "")
    log.drain(); control(base, "set", buffer="in1", value=300)
    rep.check(f"{p}: set in1=300 → CHANGE 1", expect_line(log, lambda l: l == "CHANGE 1 300.000", 5) is not None)
    log.drain(); control(base, "set", buffer="in2", value=0)
    rep.check(f"{p}: toggle channel → CHANGE 2 0", expect_line(log, lambda l: l == "CHANGE 2 0.000", 5) is not None)
    log.drain(); control(base, "set", buffer="in3", value=2)
    rep.check(f"{p}: dropdown channel → CHANGE 3 2", expect_line(log, lambda l: l == "CHANGE 3 2.000", 5) is not None)
    # submit-with-button: in5 alone must not arrive; the button press delivers it
    log.drain(); control(base, "set", buffer="in5", value=-7.5)
    early = expect_line(log, lambda l: l.startswith("CHANGE 5"), 3)
    rep.check(f"{p}: in5 (submitWith button) not sent before the press", early is None, early or "")
    pressed_index = None
    mark = len(log.lines)
    for idx in (6, 7):
        if not control(base, "trigger", element=idx): continue
        if since(log, mark, lambda l: l == "PRESS 4", 4): pressed_index = idx; break
    rep.check(f"{p}: button trigger → PRESS 4", pressed_index is not None, f"element index {pressed_index}")
    rep.check(f"{p}: the press delivered in5", since(log, mark, lambda l: l == "CHANGE 5 -7.500", 4) is not None)
    mark = len(log.lines)
    if pressed_index: control(base, "trigger", element=pressed_index)
    rep.check(f"{p}: second press → PRESS 4 again", since(log, mark, lambda l: l == "PRESS 4", 4) is not None)
    # phone sensor → channels 6..8 continuously
    log.drain(); time.sleep(2)
    sens = [l for l in log.lines[-300:] if l.startswith("CHANGE 6") or l.startswith("CHANGE 7") or l.startswith("CHANGE 8")]
    rep.check(f"{p}: phone accelerometer streams into channels 6-8", len(sens) >= 6, f"{len(sens)} lines in 2 s")
    # pause: event, and does an input still reach the board?
    log.drain(); control(base, "stop")
    ev = expect_line(log, lambda l: l.startswith("EVENT 0 "), 5)
    rep.check(f"{p}: PAUSE event reached the board", ev is not None, ev or "")
    log.drain(); control(base, "set", buffer="in1", value=123)
    paused = expect_line(log, lambda l: l == "CHANGE 1 123.000", 4)
    rep.checks.append({"name": f"{p}: observation — input set while paused reaches the board", "ok": True, "detail": "yes" if paused else "no"})
    print(f"  note {p}: input set while paused reaches the board: {'yes' if paused else 'no'}")
    log.drain(); control(base, "start")
    expect_line(log, lambda l: l.startswith("EVENT 1 "), 5)
    log.drain(); control(base, "set", buffer="in1", value=124)
    rep.check(f"{p}: after restart set in1=124 → CHANGE 1", expect_line(log, lambda l: l == "CHANGE 1 124.000", 5) is not None)
    log.drain(); control(base, "clear")
    ev = expect_line(log, lambda l: l.startswith("EVENT 2 "), 5)
    rep.check(f"{p}: CLEAR event reached the board", ev is not None, ev or "")
    log.drain(); log.send("s")
    stats = expect_line(log, lambda l: l.startswith("STATS"), 3)
    rep.check(f"{p}: board stats", stats is not None, stats or "")
    if stats:
        kv = dict(x.split("=") for x in stats.split()[1:])
        rep.check(f"{p}: no aborted transfers", kv.get("aborted") == "0", stats)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--board-port", required=True); ap.add_argument("--board-name", required=True); ap.add_argument("--label", required=True)
    ap.add_argument("--android"); ap.add_argument("--ios")
    ap.add_argument("--android-port", type=int, default=8091); ap.add_argument("--ios-port", type=int, default=8081)
    args = ap.parse_args()
    if not args.android and not args.ios: sys.exit("give --android SERIAL and/or --ios UDID")
    lock = os.path.join(WORKROOT, ".bench-lock-" + socket.gethostname())
    if os.path.exists(lock): sys.exit(f"bench in use: {open(lock).read().strip()}")
    open(lock, "w").write(f"phyphox-arduino phones {args.label} pid {os.getpid()} since {time.strftime('%Y-%m-%d %H:%M')}\n")
    rep = Report()
    try:
        log = SerialLog(args.board_port)
        try:
            log.send("s")
            rep.check("board answers on serial", log.wait_for(lambda l: l.startswith("STATS"), 6) is not None)
            phones = []
            if args.android: phones.append(Android(args.android, args.android_port))
            if args.ios: phones.append(IOS(args.ios, args.ios_port))
            for phone in phones:
                print(f"== {phone.name()}")
                try:
                    if phone.connect(args.board_name, rep):
                        sequence(phone, phone.base, log, rep)
                finally:
                    phone.release(rep)
                time.sleep(3)   # the board advertises again
        finally:
            log.close()
    finally:
        os.remove(lock)
    out = os.path.join(HERE, "results", "phones-" + args.label + ".json")
    json.dump({"label": args.label, "checks": rep.checks, "failed": rep.failed, "when": time.strftime("%Y-%m-%d %H:%M")}, open(out, "w"), indent=1)
    print(f"\n{len(rep.checks) - rep.failed} of {len(rep.checks)} checks passed → {out}")
    sys.exit(1 if rep.failed else 0)

if __name__ == "__main__":
    main()
