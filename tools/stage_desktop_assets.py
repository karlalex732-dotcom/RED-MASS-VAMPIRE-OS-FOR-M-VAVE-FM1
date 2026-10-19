#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Karsei
"""Stage the native Windows application and original trap-kit listening assets.
Run build_desktop.py and tests/trap_kits_test.c first. Python is build-only.
"""
from pathlib import Path
import json, shutil, subprocess, html
root=Path(__file__).resolve().parents[1];bundle=root.parent
exe=root/'build/desktop/RED-MASS-VAMPIRE-OS.exe'
assert exe.read_bytes()[:2]==b'MZ'
shutil.copy2(exe,bundle/exe.name)
audio=bundle/'audio/trap-kits';audio.mkdir(parents=True,exist_ok=True)
for kit in ('NIGHT-TRAP','RAGE-TRAP'):
    src=root/'build/groove-kit-wav'/kit
    assert len(list(src.glob('*.wav')))==16
    shutil.copytree(src,audio/kit,dirs_exist_ok=True)
    demo=root/f'build/trap-kits/{kit}-demo.wav'
    assert demo.stat().st_size>44100
    shutil.copy2(demo,audio/demo.name)
    subprocess.run(['ffmpeg','-v','error','-y','-i',str(demo),'-codec:a','libmp3lame','-b:a','192k',str(audio/f'{kit}-demo.mp3')],check=True)
records=json.loads((root/'build/groove-kit-wav/kits.json').read_text())
assert len(records)==32
(audio/'kits.json').write_text(json.dumps(records,indent=2)+'\n')
cards=[]
for kit,desc in [('NIGHT TRAP','Deep sustained 808, tight kick, dry snare, crisp hats. A dark halftime trap kit.'),('RAGE TRAP','A driven 808, harder kick, different layered clap, metallic hats and sharp percussion. A more aggressive rage kit.')]:
    slug=kit.replace(' ','-')
    rows=''.join(f'<tr><td>{r["pad"]}</td><td>{r["note"]}</td><td>{html.escape(r["name"])}</td><td><audio controls preload="none" src="{slug}/{r["pad"]:02}-{r["name"]}.wav"></audio></td></tr>' for r in records if r['kit']==kit)
    cards.append(f'<section><h2>{kit}</h2><p>{desc}</p><p>Native firmware beat demo · 140 BPM · T6</p><audio controls preload="metadata" src="{slug}-demo.mp3"></audio><p><a href="{slug}-demo.wav" download>Download native demo WAV</a></p><table><thead><tr><th>Pad</th><th>MIDI note</th><th>Sound</th><th>Original one-shot WAV</th></tr></thead><tbody>{rows}</tbody></table></section>')
(audio/'index.html').write_text('''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>RED MASS · Trap kits</title><style>body{margin:0;background:#09090c;color:#fff;font:15px/1.5 system-ui}header{background:#ff2449;color:#09090c;padding:28px}main{max-width:1050px;margin:auto;padding:24px}h1{font-size:40px;margin:0}h2,a{color:#ff76b9}section{padding:24px;background:#242428;margin:24px 0}p{max-width:76ch;color:#d4d4d9}table{border-collapse:collapse;width:100%}th,td{padding:10px;text-align:left;border-bottom:1px solid #45454c}audio{max-width:100%}@media(max-width:650px){section{padding:12px}th,td{padding:5px;font-size:12px}audio{width:160px}}</style><header><h1>NIGHT TRAP + RAGE TRAP</h1><b>RED MASS VAMPIRE OS · Original sounds by Karsei</b></header><main><p>Two original 16-pad kits. The beat demos were rendered by the actual C firmware DSP on the host. Individual WAVs are the uncompressed generator output; firmware playback uses IMA ADPCM and the native mixer. No artist recordings are included.</p><p><b>On FM-1:</b> select track 6, choose SMPKIT, then load NIGHT TRAP or RAGE TRAP. You can also turn the KIT parameter: NIGHT / USR1 / USR2 / USR3 / RAGE. White key 9 is the 808; the generic pad display still calls it LT. USER KIT slots are preserved.</p>'''+''.join(cards)+'</main></html>')
(audio/'README-EN.txt').write_text('RED MASS VAMPIRE OS · Original NIGHT TRAP and RAGE TRAP kits\nDeveloper: Karsei\n\nOpen index.html or the application Trap kits tab to listen.\n32 original generator WAVs plus two native C DSP beat demos (WAV/MP3).\nFirmware playback uses IMA ADPCM; source WAVs are provided for listening and reuse.\nNo artist recordings or artist sample packs are included.\nThe code and distribution follow the accompanying GPL-3.0 licence.\n\nSelect T6 > SMPKIT > NIGHT TRAP or RAGE TRAP. USER KIT 1–3 remain unchanged.\nThe KIT parameter retains USER slot indices 1–3; RAGE is appended at index 4.\n808 samples are sustained one-shots, not a dedicated chromatic or slide-bass engine.\n')
print('Staged native Windows EXE, 32 original WAVs, two native beat demos and the trap-kit listening page.')
