#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Export the native factory loader's full parameters, including FM6 patches."""
from pathlib import Path
import csv,json
root=Path(__file__).resolve().parents[1]
desc=json.loads((root/'build/host/desc.json').read_text())
spec=json.loads((root/'bank/RED-MASS-Trap-Vampire-128.spec.json').read_text())['presets']
native=json.loads((root/'build/trap128-audio/native-presets.json').read_text())
assert len(native)==len(spec)==128 and desc['P_COUNT']==91
seen={};labels=[]
for d in desc['TP'][:83]:
 l=d['label'];seen[l]=seen.get(l,0)+1;labels.append(l+(f"#{seen[l]}" if seen[l]>1 else ''))
labels += [f'E{i}' for i in range(8)]
patches=[]
for p,n in zip(spec,native):
 assert p['name']==n['name'] and p['engine_id']==n['engine'] and p['factory_index']==n['factory'] and len(n['params'])==91
 assert n['params'][37]==0
 assert n['params'][83:]==p['engine_values']
 for k,v in p['sound_controls'].items():assert n['params'][int(k)]==v
 pt=dict(name=p['name'],engine=n['engine'],engineName=p['engine'],params=n['params'],pattern=None,grid=None,
  tags=['Vampire','Trap',p['category'],'POLY'],number=p['number'],factoryPreset=n['factory'],
  created='2026-10-08T02:08:48.000Z',modified='2026-10-08T02:08:48.000Z')
 if n.get('fm6'):
  assert len(n['fm6'])==128 and n['fm6']==p['fm6'];pt['fm6']=n['fm6']
 patches.append(pt)
b=dict(format='red-mass-library',version=1,kind='library',firmware='RED MASS VAMPIRE OS v0.2.0-alpha',
 pCount=91,pE0=83,paramLabels=labels,engines=[e['name'] for e in desc['ENG']],exported='2026-10-08T02:08:48.000Z',patches=patches)
bd=root/'bank'
(bd/'RED-MASS-Trap-Vampire-128.json').write_text(json.dumps(b,indent=2)+'\n')
for i,letter in enumerate('ABCD'):
 half=dict(b,kind='bank',patches=[dict(p,slot=j) for j,p in enumerate(patches[i*32:(i+1)*32])])
 (bd/f'RED-MASS-Trap-Vampire-{letter}-32.json').write_text(json.dumps(half,indent=2)+'\n')
with (bd/'RED-MASS-Trap-Vampire-128.csv').open('w',newline='') as f:
 w=csv.writer(f);w.writerow(['number','name','category','engine','factory_preset','native_voice_cap','verified_chord_notes','recommended_register'])
 for p in spec:w.writerow([p['number'],p['name'],p['category'],p['engine'],p['factory_index'],p['polyphony'],min(4,p['polyphony']),'C2' if p['number']<=32 else 'C3'])
print('128 exact native parameter records, FM6 snapshots and four optional 32-slot files exported')
