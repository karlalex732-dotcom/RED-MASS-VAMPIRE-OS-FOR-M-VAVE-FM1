// SPDX-License-Identifier: GPL-3.0-only
// The real companion protocol against both its demo and a native firmware reply.
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import vm from "node:vm";

const html = readFileSync(new URL("editor.html", import.meta.url), "utf8");
const proto = html.slice(html.indexOf("/*PROTO-BEGIN*/"), html.indexOf("/*PROTO-END*/"));
const E = vm.runInNewContext(proto + ";({frame,unframe,req,parse,CMD,Link,makeMockDevice,parseHits,hitsText,CONDITIONS})",
  {setTimeout,clearTimeout,setInterval,clearInterval,console});
const m = E.makeMockDevice({auto:false,red_mass:true});
const input = [...m.access.inputs.values()][0], output = [...m.access.outputs.values()][0];
const link = new E.Link(d => output.send(d), {timeout:300});
input.onmidimessage = e => link.receive(e.data);
const rq = r => link.request(r);
try {
  const info = E.parse[E.CMD.INFO](await rq(E.req.info()));
  assert.ok(info.engines.includes("SMPKIT"));assert.equal(info.ntrk,6);
  const rows=[{slot:0,repeat:1,length:16,mute:32,bpm:140},{slot:1,repeat:2,length:64,mute:63,bpm:0}];
  const song=E.parse[E.CMD.SONG](await rq(E.req.song(1,rows,{})));
  assert.equal(song.rows[0].mute,32);assert.equal(song.rows[1].mute,63);
  await rq(E.req.track(5));
  await rq(E.req.motion(5,3,{step:63,param:39,value:21}));
  const motion=E.parse[E.CMD.MOTION](await rq(E.req.motion(5)));
  assert.equal(motion.events[0].step,63);assert.equal(motion.events[0].value,21);
  const step = {n:0,notes:[0,0,0,0],time:0,flags:0,vel:100,hit:0x8101,acc:0,ghost:0x8001,condition:13,chance:75,ratchet:4};
  assert.deepEqual(Array.from(E.req.stepSet(2, step)[1]),
    [2,0,0,0,0,0,0,0,100,1,0,0,75,4,13,1,0,1,1,0,2]);
  for (let condition=0;condition<14;condition++) {
    const s = E.parse[E.CMD.STEP_SET](await rq(E.req.stepSet(2,{...step,condition})));
    assert.equal(s.condition,condition); assert.equal(s.hit,step.hit); assert.equal(s.ghost,step.ghost);
    assert.equal(s.chance,75); assert.equal(s.ratchet,4);
  }
  const s = E.parse[E.CMD.STEP_SET](await rq(E.req.stepSet(2,{...step,hit:0,acc:0,ghost:0,condition:0})));
  assert.equal(s.hit,0); assert.equal(s.ghost,0); assert.equal(s.condition,0);
  const defaults = E.req.stepSet(0,{n:0,notes:[],hit:0x8000,condition:8})[1];
  assert.equal(defaults.length,21); assert.equal(defaults[12],100); assert.equal(defaults[13],1);
  assert.throws(() => E.req.stepSet(0,{...step,condition:14}), /condition/);
  const pads = E.parseHits("KICK LOWTOM >CUICA");
  assert.equal(pads.hit,0x8101); assert.equal(pads.acc,0x8000);
  assert.equal(E.hitsText(pads.hit,pads.acc),"KICK LOWTOM >CUICA");

  // Produced by tests/editor_test.c, not by the JS mock or its encoder.
  const wire = E.unframe(readFileSync(new URL("../build/host/groove-step.syx", import.meta.url)));
  assert.equal(wire.cmd,E.CMD.STEP_GET);
  const native = E.parse[wire.cmd](wire.a);
  assert.equal(native.index,2); assert.equal(native.hit,0x8100); assert.equal(native.acc,0);
  assert.equal(native.ghost,0x8000); assert.equal(native.condition,13);
  assert.equal(native.chance,75); assert.equal(native.ratchet,4);
  const bad = wire.a.slice(); bad[14]=14;
  assert.throws(() => E.parse[wire.cmd](bad), /condition/);
  console.log("groove editor: 16 pads, all 14 conditions, ghosts, clear and native C reply passed");
} finally {m.stop();}
