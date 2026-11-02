// SPDX-License-Identifier: GPL-3.0-only
// Regression for beta connecting through the real firmware's bounded SysEx replies.
// Consume tests/editor_connect_capture.c output; no Web MIDI or device is simulated by the editor mock.
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';

const html = readFileSync(new URL('./editor.html', import.meta.url), 'utf8');
const proto = html.split('/*PROTO-BEGIN*/')[1].split('/*PROTO-END*/')[0];
const E = vm.runInNewContext(proto + ';({ Link, req, CMD, unframe, parse, connectNames, RED_MASS_MOCK })',
  { setTimeout, clearTimeout, setInterval, clearInterval, console });
const capture = JSON.parse(readFileSync(process.argv[2] || new URL('../build/host/editor-connect-beta.json', import.meta.url), 'utf8'));
const nativeInfo = E.parse[E.CMD.INFO](E.unframe(capture.info).a);
assert.equal(nativeInfo.version, 'RED MASS VAMPIRE OS v0.5.2-beta');
assert.equal(nativeInfo.nengines, capture.names.length);
assert.equal(nativeInfo.ntrk, 6);

const link = new E.Link(data => {
  const f = E.unframe(data);
  const wire = f.cmd === E.CMD.INFO ? capture.info : f.cmd === E.CMD.NAMES ? capture.names[f.a[0]] : null;
  assert(wire, 'Unexpected request in native preset-list connection phase');
  queueMicrotask(() => link.receive(Uint8Array.from(wire)));
});
let recovered = 0;
try {
  const info = E.parse[E.CMD.INFO](await link.request(E.req.info()));
  for (let engine = 0; engine < info.nengines; engine++) {
    const reply = await link.request(E.req.names(engine));
    const names = E.connectNames(reply, engine, info);
    // ED_NAMES transmits at most 12 characters of each native preset name.
    assert.deepEqual(Array.from(names.names), Array.from(E.RED_MASS_MOCK.engines[engine].presets, p => names.recovered ? p.name : p.name.slice(0, 12)));
    recovered += !!names.recovered;
  }
} finally { link.close(); }
assert.equal(recovered, 1, 'Only the overlong VAMPIRE list needs recovery');

const engine = nativeInfo.engines.indexOf('VAMPIRE');
const wire = capture.names[engine], cut = E.unframe(wire).a;
assert.equal(wire.length, 600);
assert.equal(cut[1], 99);
assert.throws(() => E.parse[E.CMD.NAMES](cut), /short reply/);
for (const version of ['v0.5.1-alpha', 'v0.5.2-alpha', 'v0.5.2-beta']) {
  const result = E.connectNames(cut, engine, { ...nativeInfo, version: 'RED MASS VAMPIRE OS ' + version });
  assert.equal(result.names.length, 99);
  assert.equal(result.recovered, true);
}
for (const version of ['some other firmware', 'RED MASS VAMPIRE OS v0.5.3-beta'])
  assert.throws(() => E.connectNames(cut, engine, { ...nativeInfo, version }), /short reply/);
for (const at of [0, 1, 2, cut.length - 1]) {
  const damaged = cut.slice(); damaged[at] ^= 1;
  assert.throws(() => E.connectNames(damaged, engine, nativeInfo), /short reply|differs/);
}
assert.throws(() => E.connectNames(cut.slice(0, -1), engine, nativeInfo), /short reply|differs/);
const engines = Array.from(nativeInfo.engines); engines[engine] = 'OTHER';
assert.throws(() => E.connectNames(cut, engine, { ...nativeInfo, engines }), /short reply|differs/);

console.log(`Native beta connection: INFO + ${capture.names.length} engine lists PASS; all 99 VAMPIRE names restored.`);
console.log('Known alpha/beta versions PASS; unknown firmware, wrong counts/engine, damaged and short frames rejected.');
