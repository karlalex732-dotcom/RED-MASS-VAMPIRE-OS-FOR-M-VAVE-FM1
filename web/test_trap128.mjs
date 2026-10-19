// SPDX-License-Identifier: GPL-3.0-only
// Native parameters and six-operator snapshots survive the real importer.
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
const html=readFileSync(new URL('./editor.html',import.meta.url),'utf8');
const proto=html.split('/*PROTO-BEGIN*/')[1].split('/*PROTO-END*/')[0];
const E=vm.runInNewContext(proto+';({readLibraryFile,paramKeys,makeMockDevice})',{setTimeout,clearTimeout,setInterval,clearInterval,console});
const m=E.makeMockDevice({auto:false,red_mass:true}),T=m.tables;
const ctx={keys:E.paramKeys(T.TP,T.P_E0,T.P_COUNT),engines:T.ENG.map(e=>e.name),pe0:T.P_E0};
for(const [file,count] of [['RED-MASS-Trap-Vampire-128.json',128],...Array.from('ABCD',x=>[`RED-MASS-Trap-Vampire-${x}-32.json`,32]),['RED-MASS-VAMPIRE-256.json',256],['RED-MASS-Club-Vampire-128.json',128],...Array.from('ABCDEFGH',x=>[`RED-MASS-VAMPIRE-256-${x}-32.json`,32])]){
 const bank=JSON.parse(readFileSync(new URL('../bank/'+file,import.meta.url),'utf8'));
 const r=E.readLibraryFile(bank,ctx);
 const legacy=E.readLibraryFile({...bank,format:'karsei-library'},ctx);
 if(JSON.stringify(legacy)!==JSON.stringify(r))throw Error('Previous product format changed on import: '+file);
 if(r.skipped||r.patches.length!==count||new Set(r.patches.map(p=>p.name)).size!==count)throw Error('Wrong count: '+file);
 for(const [i,p] of r.patches.entries()){
  const source=bank.patches[i];
  if(p.engine!==source.engine||p.p[37]!==0||p.p.length!==91||p.p.slice(33,37).some(x=>x<0||x>127))throw Error('Wrong poly/FX/engine: '+p.name);
  if(p.p.some((x,k)=>x!==source.params[k]))throw Error('Parameter changed: '+p.name);
  if(p.engine===12&&JSON.stringify(p.fm6)!==JSON.stringify(source.fm6))throw Error('FM6 patch changed: '+p.name);
  if(T.ENG[p.engine].presets[source.factoryPreset].name!==p.name)throw Error('Factory mismatch: '+p.name);
 }
 console.log(file+': '+count+' patches import exactly, POLY and native FX values verified');
}
m.stop();
