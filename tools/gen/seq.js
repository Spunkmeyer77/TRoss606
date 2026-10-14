// Super 606 pattern memory: 8 slots x 9 rows x 16 steps.
// Grid clicks edit the playing slot. Picking a slot while Live plays queues it
// and it starts when the current pattern reaches step 1 again; while stopped it
// switches at once. Every row is saved in two hidden Live parameters (0..255 each)
// so all 8 patterns are stored with the Live set.
inlets = 1;
outlets = 1;

var ROWS = ["ac", "bd", "sd", "lt", "ht", "cp", "ch", "oh", "cy"];
var NS = ROWS.length, NSLOT = 8, NSTEP = 16;
var masks = [];
var cur = 0, pending = -1, lastStep = -100000, ready = false;
var curStep = 0, stepMs = 125, seqLen = 16, recOn = 1;  // always records while Live plays
// keyboard note -> sequencer row (same map as the device's MIDI input)
var NOTE_ROW = { 36: 1, 35: 1, 38: 2, 40: 2, 45: 3, 41: 3, 43: 3, 50: 4, 47: 4, 48: 4,
                 39: 5, 42: 6, 44: 6, 46: 7, 49: 8, 57: 8, 51: 8, 52: 8, 55: 8 };
var me = this;
for (var k = 0; k < NSLOT; k++) { masks.push([]); for (var r = 0; r < NS; r++) masks[k].push(0); }

function obj(name) {
    var o = me.patcher.getnamed(name);
    if (!o) post("super606: missing object " + name + "\n");
    return o;
}
function num(v) { if (v instanceof Array) v = v[0]; v = Number(v); return isNaN(v) ? 0 : Math.round(v); }
function now() { return Date.now(); }
function playing() { return now() - lastStep < 1500; }

function persist(slot, row) {
    var m = masks[slot][row], k = slot * NS + row;
    var lo = obj("pat_" + k + "_lo"), hi = obj("pat_" + k + "_hi");
    if (lo) lo.message("int", m & 255);
    if (hi) hi.message("int", (m >> 8) & 255);
}
function sendRow(row) { outlet(0, "m_" + ROWS[row], masks[cur][row]); }
function sendAll() { for (var r = 0; r < NS; r++) sendRow(r); }
function showGrid() {
    for (var r = 0; r < NS; r++)
        for (var s = 0; s < NSTEP; s++) {
            var t = obj("st_" + r + "_" + s);
            if (t) t.message("set", (masks[cur][r] >> s) & 1);
        }
}
function showLeds() {
    for (var i = 0; i < NSLOT; i++) { var l = obj("sl_" + i); if (l) l.message("int", i === cur ? 1 : 0); }
}
function switchTo(k) { cur = k; showGrid(); sendAll(); showLeds(); }

// device loaded: read the 8 stored patterns back from the hidden parameters
function init() {
    for (var k = 0; k < NSLOT * NS; k++) {
        var lo = obj("pat_" + k + "_lo"), hi = obj("pat_" + k + "_hi");
        masks[Math.floor(k / NS)][k % NS] = (lo ? num(lo.getvalueof()) : 0) + 256 * (hi ? num(hi.getvalueof()) : 0);
    }
    var tab = obj("pattern_tab");
    cur = tab ? Math.max(0, Math.min(NSLOT - 1, num(tab.getvalueof()))) : 0;
    pending = -1;
    ready = true;
    switchTo(cur);
}
function slot(k) {
    if (!ready) return;
    k = Math.max(0, Math.min(NSLOT - 1, num(k)));
    if (playing()) { pending = (k === cur) ? -1 : k; }
    else { pending = -1; switchTo(k); }
}
function step(s) {
    var t = now(), gap = t - lastStep;
    if (gap > 20 && gap < 1500) stepMs = 0.7 * stepMs + 0.3 * gap;  // follow the tempo
    lastStep = t;
    curStep = num(s);
    if (num(s) === 0 && pending >= 0) { var p = pending; pending = -1; switchTo(p); }
}
function cell(r, s, v) {
    if (!ready) return;
    r = num(r); s = num(s);
    var m = masks[cur][r];
    m = num(v) ? (m | (1 << s)) : (m & ~(1 << s));
    masks[cur][r] = m & 65535;
    persist(cur, r);
    sendRow(r);
}
function clear() {
    if (!ready) return;
    for (var r = 0; r < NS; r++) { masks[cur][r] = 0; persist(cur, r); }
    showGrid();
    sendAll();
}

function len(n) { seqLen = Math.max(1, Math.min(NSTEP, num(n))); }
function rec(v) { recOn = num(v) ? 1 : 0; }

// REC on + Live playing: a keyboard hit writes its drum into the nearest step
// of the playing slot (velocity above 100 also sets the accent on that step).
function note(n, v) {
    if (!ready || !recOn || !playing()) return;
    var r = NOTE_ROW[num(n)];
    if (r === undefined) return;
    var s = curStep;
    if (now() - lastStep > stepMs / 2) s = (curStep + 1) % seqLen;
    setStep(r, s);
    if (num(v) > 100) setStep(0, s);
}
function setStep(r, s) {
    if ((masks[cur][r] >> s) & 1) return;
    cell(r, s, 1);
    var t = obj("st_" + r + "_" + s);
    if (t) t.message("set", 1);
}
