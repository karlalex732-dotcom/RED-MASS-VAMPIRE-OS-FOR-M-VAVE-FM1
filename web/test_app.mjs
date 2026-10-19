// SPDX-License-Identifier: GPL-3.0-only; Copyright (C) 2026 Karsei.
// Run the actual portal controller against an instrumented iframe, without MIDI.
import vm from 'node:vm';
import {readFileSync,existsSync} from 'node:fs';
import {fileURLToPath} from 'node:url';
import {resolve,dirname} from 'node:path';
import assert from 'node:assert/strict';
const here=dirname(fileURLToPath(import.meta.url)),bundle=resolve(here,'../..');
const html=readFileSync(resolve(here,'app/index.html'),'utf8'),listeners={};
const buttons=[...html.matchAll(/<button\b[^>]*data-view="([^"]+)"[^>]*>/g)].map(m=>({dataset:{view:m[1]},disabled:false,setAttribute(){},addEventListener(){}}));
let releases=0,tabClicks=0,finish;
const child={document:{querySelector:()=>({click(){tabClicks++;}})}};
const frame={hidden:true,contentWindow:child,src:''},home={hidden:false};
const document={getElementById:id=>id==='view'?frame:home,querySelectorAll:()=>buttons};
const ctx=vm.createContext({document,location:{origin:'http://localhost'},window:{addEventListener:(event,fn)=>listeners[event]=fn}});
vm.runInContext(html.match(/<script>([\s\S]*?)<\/script>/)[1]+'\nwindow.testShow=show;',ctx);
const show=ctx.window.testShow,source='http://localhost';
const busy=(value,origin=source)=>listeners.message({origin,source:child,data:{type:'redmass-operation',busy:value}});
await show('editor');assert.equal(frame.src,'../editor/');assert(home.hidden&&!frame.hidden);
child.redMassReleaseDevice=()=>{releases++;return new Promise(r=>finish=r);};
busy(true);assert(buttons.every(b=>b.disabled));await show('installer');assert.equal(releases,0);assert.equal(frame.src,'../editor/');
busy(false,'http://untrusted');assert(buttons.every(b=>b.disabled));busy(false);
const move=show('installer');assert.equal(releases,1);assert(buttons.every(b=>b.disabled));
busy(false);assert(buttons.every(b=>b.disabled),'port-release transition must remain locked even after editor reports idle');
await show('home');assert(home.hidden);finish();await move;
assert.equal(frame.src,'../installer/');assert(buttons.every(b=>!b.disabled));
await show('editor','projects');frame.onload();assert.equal(tabClicks,1);
await show('editor','projects');assert.equal(tabClicks,2);assert.equal(releases,1,'selecting an editor tab preserves connection');
const leave=show('home');finish();await leave;assert(!home.hidden&&frame.hidden);
for(const view of ['sounds','kits','guide']){await show(view);assert(existsSync(resolve(bundle,'web-tools/webapp/app',frame.src)));}
for(const page of ['editor.html','index_pkg.html']){
 const source=readFileSync(resolve(here,page),'utf8');
 for(const match of source.matchAll(/<script(?:\s[^>]*)?>([\s\S]*?)<\/script>/g)){
  const code=match[1].replace(/^import .*?;\n/gm,'').replace(/\/\*META\*\//g,'{}');
  new vm.Script(code,{filename:page});
 }
}
console.log('App navigation: editor port release, operation/transition locks, project shortcut, same-tab connection preservation, packaged guide/previews and editor/installer script syntax passed. No Windows or browser layout claims.');
