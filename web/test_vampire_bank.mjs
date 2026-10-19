// SPDX-License-Identifier: GPL-3.0-only
// The 64 exported patches through the actual companion editor importer.
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
const html=readFileSync(new URL('./editor.html',import.meta.url),'utf8');
const proto=html.split('/*PROTO-BEGIN*/')[1].split('/*PROTO-END*/')[0];
const E=vm.runInNewContext(proto+';({readLibraryFile,paramKeys,makeMockDevice})',{setTimeout,clearTimeout,setInterval,clearInterval,console});
const m=E.makeMockDevice({auto:false,red_mass:true}),T=m.tables;
const ctx={keys:E.paramKeys(T.TP,T.P_E0,T.P_COUNT),engines:T.ENG.map(e=>e.name),pe0:T.P_E0};
const file=JSON.parse(readFileSync(new URL('../bank/RED-MASS-VAMPIRE-64.json',import.meta.url),'utf8'));
const r=E.readLibraryFile(file,ctx);m.stop();
if(r.skipped||r.patches.length!==64||new Set(r.patches.map(p=>p.name)).size!==64)throw Error('Wrong exported bank count or duplicate names');
for(const [i,p] of r.patches.entries()){
 if(p.engine!==14||p.engineName!=='VAMPIRE'||p.p.length!==91||p.p[37]!==0||p.p.slice(33,37).some(x=>x<=0))throw Error('Wrong engine/polyphony/FX: '+p.name);
 if(p.p.some((x,k)=>x!==file.patches[i].params[k]))throw Error('Parameter changed during import: '+p.name);
}
console.log('RED MASS bank: all 64 patches import unchanged; VAMPIRE / POLY / four FX sends verified');
