#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Update the companion editor's optional RED_MASS mock from native C descriptors.
Run after build/host/descdump > build/host/desc.json. Old Felucca mocks stay available
for protocol migration tests. The editor demo uses red_mass:true.
"""
from pathlib import Path
import json, re
root = Path(__file__).resolve().parents[1]
desc = json.loads((root/'build/host/desc.json').read_text())
bank = json.loads((root/'bank/RED-MASS-VAMPIRE-64.spec.json').read_text())['presets']
trap = json.loads((root/'bank/RED-MASS-Trap-Vampire-128.spec.json').read_text())['presets']
trap+=json.loads((root/'bank/RED-MASS-Club-Vampire-128.spec.json').read_text())['presets']
data = dict(common=desc['TP'][83:], engines=desc['ENG'], globals=desc['GP'], bank=bank, trap=trap)
page = root/'web/editor.html'
html = page.read_text()
block = '/* RED-MASS-MOCK-BEGIN */\nconst RED_MASS_MOCK = '+json.dumps(data,separators=(',',':'))+';\n/* RED-MASS-MOCK-END */'
if '/* RED-MASS-MOCK-BEGIN */' in html:
    html = re.sub(r'/\* RED-MASS-MOCK-BEGIN \*/.*?/\* RED-MASS-MOCK-END \*/', lambda _:block, html, flags=re.S)
else:
    html = html.replace('function makeMockDevice(opt = {}) {',block+'\nfunction makeMockDevice(opt = {}) {')
page.write_text(html)
print('RED_MASS editor mock: native descriptors and legacy 64 and 256 curated factory patches synchronized')
