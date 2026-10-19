#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Export the exact native 256-preset library and eight optional 32-slot banks."""
from pathlib import Path
import copy, csv, json

root = Path(__file__).resolve().parents[1]
old = json.loads((root/'bank/RED-MASS-Trap-Vampire-128.json').read_text())
spec = json.loads((root/'bank/RED-MASS-Club-Vampire-128.spec.json').read_text())['presets']
native = json.loads((root/'build/club128-audio/native-presets.json').read_text())
assert len(old['patches']) == len(spec) == len(native) == 128
patches = copy.deepcopy(old['patches'])
for p, n in zip(spec, native):
    assert p['name'] == n['name'] and p['engine_id'] == n['engine'] and p['factory_index'] == n['factory']
    assert len(n['params']) == 91 and n['params'][83:] == p['engine_values']
    for k, v in p['sound_controls'].items():
        assert n['params'][int(k)] == v
    patch = dict(name=p['name'], engine=n['engine'], engineName=p['engine'], params=n['params'],
                 pattern=None, grid=None, tags=['Vampire','Club',p['category'],'POLY'],
                 number=p['number'], factoryPreset=n['factory'],
                 created='2026-10-09T00:00:00.000Z', modified='2026-10-09T00:00:00.000Z')
    if n.get('fm6'):
        assert len(n['fm6']) == 128 and n['fm6'] == p['fm6']
        patch['fm6'] = n['fm6']
    patches.append(patch)
assert len({p['name'] for p in patches}) == 256
out = dict(old, format='red-mass-library', firmware='RED MASS VAMPIRE OS v0.5.1-alpha', exported='2026-10-09T00:00:00.000Z', patches=patches)
bank = root/'bank'
(bank/'RED-MASS-VAMPIRE-256.json').write_text(json.dumps(out,indent=2)+'\n')
(bank/'RED-MASS-Club-Vampire-128.json').write_text(json.dumps(dict(out,patches=patches[128:]),indent=2)+'\n')
for i, letter in enumerate('ABCDEFGH'):
    subset = dict(out, kind='bank', patches=[dict(p,slot=j) for j,p in enumerate(patches[i*32:(i+1)*32])])
    (bank/f'RED-MASS-VAMPIRE-256-{letter}-32.json').write_text(json.dumps(subset,indent=2)+'\n')
all_spec = json.loads((bank/'RED-MASS-Trap-Vampire-128.spec.json').read_text())['presets'] + spec
with (bank/'RED-MASS-VAMPIRE-256.csv').open('w',newline='') as f:
    w = csv.writer(f)
    w.writerow(['number','name','category','role','engine','factory_index','track_voice_cap','new_in_v0.4'])
    for p in all_spec:
        w.writerow([p['number'],p['name'],p['category'],p['role'],p['engine'],p['factory_index'],p['polyphony'],int(p['number']>128)])
print('256 exact native records; old 128 retained; 128 new; eight optional 32-slot bank files')
