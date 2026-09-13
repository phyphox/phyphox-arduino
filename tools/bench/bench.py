#!/usr/bin/env python3
"""The bench (tier T1): flash tools/bench/benchSketch to a board and drive it from this host's
Bluetooth adapter, playing the phone's part. Asserts the transfer (header, CRC, bytes equal
to what the board prints on serial), data notifications and their rate, input and sensor
writes reaching the callbacks, button presses, events, a re-triggered transfer, and repeated
transfers. Writes tools/bench/results/<label>.json and exits 1 on any failure.

    tools/bench/bench.py --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 --label esp32 [--no-flash] [--transfers 5]

Needs arduino-cli, pyserial and bleak (uses ../phyphox-docs/.venv when present), a powered
BlueZ adapter, and nobody else on the bench (it takes .bench-lock-<hostname> in the working
root, like the phyphox-docs lab).
"""
import argparse, asyncio, json, os, socket, struct, subprocess, sys, threading, time, zlib, queue

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
WORKROOT = os.path.normpath(os.path.join(ROOT, ".."))
SUFFIX = "-30f7-4671-8b43-5e40ba53514a"
EXP_SERVICE, EXP_CHAR, CTRL_CHAR, EVENT_CHAR = ("cddf0001" + SUFFIX, "cddf0002" + SUFFIX, "cddf0003" + SUFFIX, "cddf0004" + SUFFIX)
DATA_CHAR = "cddf1002" + SUFFIX
def input_char(k): return "cddf20%02x%s" % (k, SUFFIX)
def sensor_char(s): return "cddf30%02x%s" % (s, SUFFIX)

try:
    import serial, bleak
except ImportError:
    venv = os.path.join(WORKROOT, "phyphox-docs", ".venv", "bin", "python")
    if os.path.exists(venv) and os.path.abspath(sys.executable) != os.path.abspath(venv):
        os.execv(venv, [venv] + sys.argv)
    sys.exit("needs pyserial and bleak")
from bleak import BleakClient, BleakScanner

class Report:
    def __init__(self): self.checks = []; self.failed = 0
    def check(self, name, ok, detail=""):
        self.checks.append({"name": name, "ok": bool(ok), "detail": detail})
        print(("  ok   " if ok else "  FAIL ") + name + (("  " + detail) if detail else ""))
        if not ok: self.failed += 1

class SerialLog:
    """Background reader; lines land in a queue and in the full log."""
    def __init__(self, port, baud=115200):
        self.ser = serial.Serial(port, baud, timeout=0.1)
        self.q = queue.Queue(); self.lines = []; self.stop = False
        self.t = threading.Thread(target=self.run, daemon=True); self.t.start()
    def run(self):
        buf = b""
        while not self.stop:
            try: chunk = self.ser.read(256)
            except Exception: break
            if not chunk: continue
            buf += chunk
            while b"\n" in buf:
                line, buf = buf.split(b"\n", 1)
                s = line.decode("utf-8", "replace").rstrip("\r")
                self.lines.append(s); self.q.put(s)
    def send(self, text): self.ser.write(text.encode()); self.ser.flush()
    def wait_for(self, predicate, timeout):
        end = time.time() + timeout
        while time.time() < end:
            try: line = self.q.get(timeout=0.1)
            except queue.Empty: continue
            if predicate(line): return line
        return None
    def drain(self):
        while not self.q.empty(): self.q.get_nowait()
    def close(self): self.stop = True; self.ser.close()

def flash(fqbn, port, extra):
    cmd = ["arduino-cli", "compile", "--upload", "-p", port, "--fqbn", fqbn, "--library", ROOT]
    if extra: cmd += ["--build-property", "compiler.cpp.extra_flags=" + extra]
    cmd.append(os.path.join(HERE, "benchSketch"))
    print("flashing:", " ".join(cmd))
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        print(r.stdout[-3000:]); print(r.stderr[-3000:]); sys.exit("flash failed")
    for line in r.stdout.splitlines():
        if "Sketch" in line or "Global" in line or "verwendet" in line: print("  " + line.strip())

def wait_port(port, timeout=30):
    end = time.time() + timeout
    while time.time() < end:
        if os.path.exists(port): return True
        time.sleep(0.5)
    return False

def get_xml(log, rep):
    """Ask the sketch for its document; returns the bytes between the markers."""
    for attempt in range(3):
        log.drain(); log.send("x")
        if not log.wait_for(lambda l: l == "XML-BEGIN", 5): continue
        lines = []
        while True:
            line = log.wait_for(lambda l: True, 5)
            if line is None: break
            if line == "XML-END": return ("\n".join(lines) + "\n").encode()
            lines.append(line)
    rep.check("serial: XML dump", False, "no XML-BEGIN/END within 3 attempts"); return None

async def find_device(name, timeout=15):
    end = time.time() + timeout
    while time.time() < end:
        devices = await BleakScanner.discover(timeout=3.0, return_adv=True)
        for d, adv in devices.values():
            if (adv.local_name or d.name or "") == name or EXP_SERVICE in [u.lower() for u in (adv.service_uuids or [])]:
                if (adv.local_name or d.name or "") == name: return d
    return None

class TransferSink:
    """Collects the packets of one transfer on an existing subscription."""
    def __init__(self): self.reset()
    def reset(self):
        self.packets = []; self.size = None; self.crc = None; self.done = asyncio.Event()
    def __call__(self, _, data):
        data = bytes(data)
        self.packets.append(data)
        if len(self.packets) == 1 and len(data) >= 15 and data[:7] == b"phyphox":
            self.size = struct.unpack(">I", data[7:11])[0]; self.crc = struct.unpack(">I", data[11:15])[0]
        elif self.size is not None and sum(len(p) for p in self.packets[1:]) >= self.size:
            self.done.set()

async def receive_transfer(client, rep, label, expected, sink=None, subscribe=True, timeout=30):
    """Wait for one transfer: subscribing (which triggers it) or on an existing subscription
    after the caller triggered it another way (the control write)."""
    t0 = time.time()
    if sink is None: sink = TransferSink()
    if subscribe: await client.start_notify(EXP_CHAR, sink)
    try: await asyncio.wait_for(sink.done.wait(), timeout)
    except asyncio.TimeoutError: pass
    dt = time.time() - t0
    packets = sink.packets; state = {"size": sink.size, "crc": sink.crc}
    body = b"".join(packets[1:])
    hdr_ok = bool(packets) and packets[0][:7] == b"phyphox"
    rep.check(f"{label}: header", hdr_ok, f"{len(packets)} packets in {dt:.2f} s")
    if not hdr_ok: return None
    size, crc = state["size"], state.get("crc")
    body = body[:size]
    rep.check(f"{label}: size {size} received", len(body) == size, f"got {len(body)} of {size} in {len(packets)-1} body packets, {dt:.2f} s")
    rep.check(f"{label}: CRC", zlib.crc32(body) & 0xFFFFFFFF == crc, f"crc {crc:08x} vs {zlib.crc32(body) & 0xFFFFFFFF:08x}")
    if expected is not None:
        rep.check(f"{label}: bytes equal the serial dump", body == expected,
                  "" if body == expected else f"first difference at {next((i for i in range(min(len(body), len(expected))) if body[i] != expected[i]), min(len(body), len(expected)))}")
    return body

async def run(args, rep, log):
    dev = await find_device(args.name, 20)
    rep.check("advertising as '%s' with the phyphox service" % args.name, dev is not None)
    if dev is None: return
    xml = get_xml(log, rep)
    async with BleakClient(dev, timeout=20) as client:
        rep.check("connected", client.is_connected)
        uuids = {c.uuid.lower() for s in client.services for c in s.characteristics}
        for u, what in [(EXP_CHAR, "experiment"), (CTRL_CHAR, "control"), (EVENT_CHAR, "event"), (DATA_CHAR, "data"),
                        (input_char(1), "input 1"), (input_char(5), "input 5"), (sensor_char(1), "sensor 1")]:
            rep.check(f"characteristic {what}", u in uuids)
        rep.check("no input characteristic for sensor-fed channel 6", input_char(6) not in uuids)
        mtu = getattr(client, "mtu_size", 23)
        print(f"  (negotiated MTU {mtu})")

        # 1. transfer on subscription
        sink = TransferSink()
        body = await receive_transfer(client, rep, "transfer 1 (subscription)", xml, sink)
        # 2. a second transfer on the same subscription, triggered by the control write (the
        # app subscribes first and then writes 1 if the control characteristic exists)
        sink.reset()
        await client.write_gatt_char(CTRL_CHAR, b"\x01", response=True)
        await receive_transfer(client, rep, "transfer 2 (control write)", xml, sink, subscribe=False)
        await client.stop_notify(EXP_CHAR)

        # 3. data notifications and their rate
        count = [0]; first = [None]; last = [None]; values = []
        def dcb(_, data):
            count[0] += 1; now = time.time()
            if first[0] is None: first[0] = now
            last[0] = now
            if len(data) >= 8: values.append(struct.unpack("<ff", bytes(data[:8])))
        await client.start_notify(DATA_CHAR, dcb)
        await asyncio.sleep(3.0)
        await client.stop_notify(DATA_CHAR)
        rate = (count[0] - 1) / (last[0] - first[0]) if count[0] > 1 else 0
        rep.check("data notifications at ~20 Hz", 15 <= rate <= 25, f"{count[0]} packets, {rate:.1f} Hz")
        if len(values) > 2:
            counters = [v[0] for v in values]
            rep.check("counter channel increases by 1", all(b - a == 1 for a, b in zip(counters, counters[1:])), f"{counters[0]:.0f} … {counters[-1]:.0f}")
            rep.check("second channel in range", all(10 <= v[1] <= 90 for v in values))

        # 4. inputs: a float per channel characteristic, echoed by the callback
        for ch, val, expect in [(1, 250.0, "CHANGE 1 250.000"), (2, 0.0, "CHANGE 2 0.000"), (3, 2.0, "CHANGE 3 2.000"), (5, -7.5, "CHANGE 5 -7.500")]:
            log.drain()
            await client.write_gatt_char(input_char(ch), struct.pack("<f", val), response=True)
            got = log.wait_for(lambda l, e=expect: l == e, 3)
            rep.check(f"input channel {ch} → callback", got is not None, expect)
        log.drain()
        await client.write_gatt_char(input_char(4), struct.pack("<f", 1.0), response=True)
        rep.check("button press → onPress", log.wait_for(lambda l: l == "PRESS 4", 3) is not None)
        log.drain()
        await client.write_gatt_char(input_char(4), struct.pack("<f", 1.0), response=False)
        rep.check("second press (write without response) → onPress again", log.wait_for(lambda l: l == "PRESS 4", 3) is not None)
        # sensor: three components in one write
        log.drain()
        await client.write_gatt_char(sensor_char(1), struct.pack("<fff", 1.5, -2.5, 9.81), response=True)
        seen = set()
        for _ in range(3):
            l = log.wait_for(lambda l: l.startswith("CHANGE "), 3)
            if l: seen.add(l)
        rep.check("sensor write → three channel callbacks", {"CHANGE 6 1.500", "CHANGE 7 -2.500", "CHANGE 8 9.810"} <= seen, str(sorted(seen)))
        rep.check("configHandler fired", any(l == "CONFIG" for l in log.lines[-40:]))

        # 5. events
        log.drain()
        ev = bytes([0x01]) + struct.pack(">q", 1000) + struct.pack(">q", 1700000000000)
        await client.write_gatt_char(EVENT_CHAR, ev, response=True)
        rep.check("event write → handler", log.wait_for(lambda l: l == "EVENT 1 1000 1700000000", 3) is not None)

    # the document the board served, validated against the file-format spec like the golden ones
    if body:
        served = os.path.join(HERE, "results", args.label + "-served")
        os.makedirs(served, exist_ok=True)
        open(os.path.join(served, args.label + ".phyphox"), "wb").write(body)
        r = subprocess.run([sys.executable, os.path.join(ROOT, "tests", "conformance", "run.py"), "--validate-only", served],
                           capture_output=True, text=True)
        rep.check("served document validates against the phyphox spec", r.returncode == 0, r.stdout.strip().splitlines()[-1] if r.stdout.strip() else r.stderr[-200:])

    # 6. repeated transfers over fresh connections
    for i in range(args.transfers):
        dev = await find_device(args.name, 20)
        if dev is None: rep.check(f"transfer {i+3}: device advertising again", False); break
        async with BleakClient(dev, timeout=20) as client:
            await receive_transfer(client, rep, f"transfer {i+3} (fresh connection)", xml)
    log.drain(); log.send("s")
    stats = log.wait_for(lambda l: l.startswith("STATS"), 3)
    rep.check("stats reported", stats is not None, stats or "")
    if stats:
        kv = dict(p.split("=") for p in stats.split()[1:])
        rep.check("no aborted transfers", kv.get("aborted") == "0", stats)
        print(f"  (packets refused by the stack and retried: {kv.get('refused')})")

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--fqbn", required=True); ap.add_argument("--port", required=True); ap.add_argument("--label", required=True)
    ap.add_argument("--name", default="phyphox bench"); ap.add_argument("--no-flash", action="store_true")
    ap.add_argument("--extra", default=""); ap.add_argument("--transfers", type=int, default=3)
    args = ap.parse_args()
    if not args.extra: args.extra = "-DBENCH_NAME=" + args.name.replace(" ", "_").replace("-", "_")
    args.name = args.name.replace(" ", "_").replace("-", "_")
    lock = os.path.join(WORKROOT, ".bench-lock-" + socket.gethostname())
    if os.path.exists(lock):
        sys.exit(f"bench in use: {open(lock).read().strip()} (delete {lock} if stale)")
    open(lock, "w").write(f"phyphox-arduino bench {args.label} pid {os.getpid()} since {time.strftime('%Y-%m-%d %H:%M')}\n")
    rep = Report()
    try:
        if not args.no_flash: flash(args.fqbn, args.port, args.extra)
        if not wait_port(args.port): sys.exit("port did not come back after flashing")
        time.sleep(2.0)
        log = SerialLog(args.port)
        try:
            ready = log.wait_for(lambda l: l.startswith("READY"), 8)
            if ready is None:
                log.send("x"); ready = log.wait_for(lambda l: l.startswith("READY") or l == "XML-BEGIN", 5)
            rep.check("sketch running", ready is not None, ready or "")
            asyncio.run(run(args, rep, log))
        finally:
            log.close()
    finally:
        os.remove(lock)
    out = os.path.join(HERE, "results", args.label + ".json")
    json.dump({"label": args.label, "fqbn": args.fqbn, "checks": rep.checks, "failed": rep.failed, "when": time.strftime("%Y-%m-%d %H:%M")}, open(out, "w"), indent=1)
    print(f"\n{len(rep.checks) - rep.failed} of {len(rep.checks)} checks passed → {out}")
    sys.exit(1 if rep.failed else 0)

if __name__ == "__main__":
    main()
