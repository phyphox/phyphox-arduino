// A sketch that does nothing — flashed to a board that must stay off the air while another
// board on the same desk is tested (tools/bench/examples.py --silence). A board still running
// an example advertises that example's name, and two advertisers with the same name make the
// phone connect to whichever it sees first (found 2026-09-13: the Nano sweep talked to the ESP32).
void setup() {}
void loop() { delay(1000); }
