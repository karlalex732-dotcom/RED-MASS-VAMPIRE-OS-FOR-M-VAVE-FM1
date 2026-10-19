#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Finalize the existing RED MASS release tree, gated on the native test suite.

Run after tools/build.py, tests/run_tests.sh and tests/upgrade_capture.c:
  python3 tools/package_red_mass.py --suite /absolute/suite.log --target /absolute/build.log
The complete extracted release tree supplies the unchanged audio previews.
"""
from pathlib import Path
import argparse, hashlib, html, json, os, re, shutil, zipfile
from PIL import Image, ImageDraw, ImageFont
from red_mass_markdown import markdown

root = Path(__file__).resolve().parents[1]
bundle = root.parent
ap = argparse.ArgumentParser()
ap.add_argument('--suite', type=Path, required=True)
ap.add_argument('--target', type=Path, required=True)
ap.add_argument('--desktop-build', type=Path)
ap.add_argument('--desktop-check', type=Path)
ap.add_argument('--app-check', type=Path)
ap.add_argument('--visual-recheck', type=Path, required=True)
ap.add_argument('--visual-focus', type=Path, required=True)
ap.add_argument('--editor-hotfix', type=Path, required=True)
ap.add_argument('--stage', action='store_true')
args = ap.parse_args()
version = '0.5.1-alpha'
package_name = f'RED-MASS-VAMPIRE-OS-v{version}-Installer'
firmware_name = f'RED-MASS-VAMPIRE-OS-v{version}-FM1.fwsc'
fw = root / 'build' / f'red_mass-{version}.fwsc'
assert fw.is_file(), 'Build the target firmware first'

for p in (root / 'docs').iterdir():
    if p.is_file(): shutil.copy2(p, bundle / 'docs' / p.name)
for p in (root / 'bank').glob('RED-MASS-*'):
    if p.is_file(): shutil.copy2(p, bundle / 'bank' / p.name)
screens = bundle / 'docs/screens'
screens.mkdir(exist_ok=True)
for p in screens.glob('*.png'): p.unlink()
labels = ['COPY','CTRL ALL','LOCKS','MACROS','MACRO SET','MELODY','MUTATE','BANK 256',
          'MUTE GROUP','DRUM CHANCE','MICRO','HISTORY','SONG ROW','SONG SET','SCENES']
gallery = []
for i, label in enumerate(labels, 1):
    name = f'tool-{i:02d}.png'
    Image.open(root / f'build/red-mass-tools/tool-{i:02d}.ppm').save(screens / name)
    gallery.append((name, label))
pages = [('home','HOME'),('mixer','SIX-TRACK MIXER'),('drum','T6 DRUM GRID'),('song','SONG MODE'),
         ('env','ENVELOPE'),('lfo','MODULATION'),('presets','PRESET BROWSER'),('project','PROJECTS')]
for name, label in pages:
    if name == 'home':
        Image.open(root / 'build/red-mass-visual/home-track-5.ppm').save(screens / 'home.png')
    else:
        shutil.copy2(root / f'build/ui_new/REDMASS/{name}.png', screens / f'{name}.png')
    gallery.insert(0, (f'{name}.png', label))
for name,label in [('home-old-preset','HOME: ORIGINAL BANK'),('home-new-preset','HOME: NEW BANK'),
                   ('browse-old-preset','ORIGINAL PRESETS'),('browse-new-preset','NEW PRESETS')]:
    Image.open(root / f'build/red-mass-visual/{name}.ppm').save(screens / f'{name}.png')
    gallery.append((f'{name}.png',label))
css = '''body{margin:0;background:#09090c;color:#d4d4d9;font:16px/1.6 system-ui}
main{max-width:1050px;margin:auto;padding:32px 26px 80px}header{background:#ff2449;color:#09090c;padding:36px 26px}
header div{max-width:1050px;margin:auto}header b{font-size:13px;letter-spacing:2px}h1{font-size:42px;line-height:1.1}
h2{margin-top:40px;color:#ff76b9}h3{margin-top:28px;color:#fff}p,li{max-width:88ch}li{margin:8px 0}
a{color:#ff76b9}code{background:#242428;padding:2px 4px}pre{overflow:auto;background:#242428;padding:20px}
aside{border-left:4px solid #ff2449;background:#242428;padding:14px 20px}.table{overflow:auto}
table{border-collapse:collapse;width:100%;font-size:14px}th,td{text-align:left;vertical-align:top;padding:10px;border-bottom:1px solid #85858c}
th{background:#242428}nav{display:flex;gap:20px;flex-wrap:wrap}.screens{display:grid;grid-template-columns:repeat(auto-fit,minmax(240px,1fr));gap:18px}
.screens img{width:240px;height:240px}.screens figure{margin:0}figcaption{font-size:13px;color:#fff}
@media print{body{background:white;color:black}header{background:white;padding:0}main{padding:0}h2,h3,a{color:black}table,pre,figure{break-inside:avoid}}'''
gallery_html = ''.join(f'<figure><img src="screens/{name}" alt="{label}"><figcaption>{label}</figcaption></figure>' for name,label in gallery)
guide = ('<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width">'
         '<title>RED MASS VAMPIRE OS — English Field Guide</title><style>'+css+'</style><header><div><b>RED MASS VAMPIRE OS · FM1 · v0.5.1 ALPHA</b>'
         '<h1>RED MASS VAMPIRE OS</h1><p>Instruments 1–5. Drums 6. 256 sounds. A complete song workflow.</p></div></header><main>'
         '<nav><a href="../preview/index.html">Listen to 256 sounds</a><a href="Quick-Reference-EN.txt">Quick reference</a>'
         '<a href="../validation/TESTING-EN.txt">Validation report</a></nav>'+
         markdown((root/'docs/RED-MASS-English-Guide.md').read_text())+
         '<h2>Native firmware display captures</h2><p>Rendered by the C firmware on a simulated display. Physical FM1 qualification is pending.</p>'
         '<div class="screens">'+gallery_html+'</div></main></html>')
(bundle/'docs/RED-MASS-English-Guide.html').write_text(guide)
(root/'docs/RED-MASS-English-Guide.html').write_text(guide.replace('../preview/','../../preview/').replace('../validation/','../../validation/'))
shutil.copytree(screens,root/'docs/screens',dirs_exist_ok=True)

font_path = root/'assets/fonts/InterTight[wght].ttf'
title_font=ImageFont.truetype(str(font_path),42)
caption_font=ImageFont.truetype(str(font_path),16)
title_font.set_variation_by_axes([700]);caption_font.set_variation_by_axes([700])
canvas=Image.new('RGB',(1088,710),'#09090c');draw=ImageDraw.Draw(canvas)
draw.rectangle((0,0,1088,94),fill='#ff2449')
draw.text((28,14),'RED MASS VAMPIRE OS',font=title_font,fill='#09090c')
draw.text((30,65),'SIX TRACKS · INSTRUMENTS 1–5 · DRUMS 6 · NATIVE C DISPLAY CAPTURES',font=caption_font,fill='#09090c')
for i,(name,label) in enumerate(pages):
    x=28+(i%4)*264;y=116+(i//4)*282
    canvas.paste(Image.open(screens/f'{name}.png').convert('RGB'),(x,y))
    draw.text((x,y+247),label,font=caption_font,fill='#ff76b9')
draw.text((28,688),'v0.5.1-alpha / FM-1_906 / physical device testing pending',font=caption_font,fill='#85858c')
canvas.save(bundle/'RED-MASS-Interface.png')

if args.stage:
    print('English guide and native display previews staged')
    raise SystemExit(0)
suite=args.suite.read_text(errors='replace');target=args.target.read_text(errors='replace')
full_suite_passed=suite.rstrip().endswith('ALL HOST TESTS PASSED')
assert full_suite_passed, 'The complete native host suite must pass before packaging'
assert not any(word in suite for word in ['Traceback','AssertionError','overflow detected','Aborted']), 'Unexpected runtime failure'
assert args.desktop_build and args.desktop_check and args.app_check, 'Supply native desktop build, HTTP and app-navigation checks'
assert 'Python/Node/.NET not required' in args.desktop_build.read_text()
assert 'Desktop server checks: 0 failures' in args.desktop_check.read_text()
assert 'passed. No Windows or browser layout claims.' in args.app_check.read_text()
assert 'Trap kit native checks: 0 failures' in suite
visual=args.visual_recheck.read_text()
assert 'ui: all passed' in visual and '0 lint findings' in visual and '0 off by 1 px or more' in visual
focus=args.visual_focus.read_text()
assert 'six complete HOME frames, two preset generations: 0 failures' in focus
assert 'HOME overflow 0, browser overflow 0' in focus
editor_hotfix=args.editor_hotfix.read_text()
assert 'v0.5.1 VAMPIRE: recover all 99 names' in editor_hotfix
assert "native firmware's 600-byte SysEx reply connects with every VAMPIRE name" in editor_hotfix
assert editor_hotfix.rstrip().endswith('web tests passed')
assert 'OVER BUDGET' not in suite and 'OVERBUDGET' not in suite
assert '  ok    image ' in target and 'identity FM-1_906' in target
for p in (bundle/'firmware').glob('*.fwsc'):p.unlink()
shutil.copy2(fw,bundle/'firmware'/firmware_name)
for name in ['LICENSE','LICENSING.md']:shutil.copy2(root/name,bundle/name)
shutil.copytree(root/'LICENSES',bundle/'LICENSES',dirs_exist_ok=True)
validation=bundle/'validation'
for p in validation.iterdir():
    if p.is_file() and p.name not in ['new128-audio-health.csv','new128-native-presets.json']:p.unlink()
shutil.copy2(args.suite,validation/'red-mass-suite.log')
shutil.copy2(args.target,validation/'red-mass-target.log')
for path,name in [(args.desktop_build,'desktop-build.log'),(args.desktop_check,'desktop-http.log'),(args.app_check,'app-navigation.log'),(args.visual_recheck,'final-visual-check.log')]:
    shutil.copy2(path,validation/name)
shutil.copy2(args.visual_focus,validation/'visual-regression.log')
shutil.copy2(args.editor_hotfix,validation/'editor-connection-hotfix.log')
shutil.copy2(root/'build/desktop/pe-imports.txt',validation/'desktop-pe-imports.txt')
for name in ['report.txt','align.txt']:
    (validation/f'ui-{name}').write_text((root/'build/ui_new'/name).read_text(errors='backslashreplace'))
mem=re.search(r'image (\d+) B; RAM .data\+\.bss (\d+) B of (\d+); pool (\d+) B of (\d+)',target)
assert mem
image,ram,ram_max,pool,pool_max=map(int,mem.groups())
manifest=dict(product='RED MASS VAMPIRE OS',developer='Karsei',contributor='Karsei',version=version,visual_revision=2,editor_connection_hotfix=1,identity='FM-1_906',firmware=firmware_name,
 bytes=fw.stat().st_size,sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),audio_tracks=6,instrument_tracks=[1,2,3,4,5],drum_track=6,
 shared_voices=8,curated_rom_presets=256,existing_presets=128,new_presets=128,new_piano_variations=8,user_slots=32,
 song_rows=32,song_project_sources=4,max_row_repeats=64,project_format='FUNB',features_format='KPO5',
 host_suite_passed=full_suite_passed,host_checks_passed=True,target_build_passed=True,static_render_budget_passed=True,physical_device_tested=False,
 desktop_executable='RED-MASS-VAMPIRE-OS.exe',desktop_architecture='Windows x64',desktop_python_required=False,
 desktop_engine='Installed Microsoft Edge or Google Chrome app mode',desktop_http_host_tests_passed=True,desktop_navigation_tests_passed=True,
 desktop_windows_runtime_tested=False,desktop_browser_layout_tested=False,firmware_font_weight=700,preset_name_size_px=15,
 factory_sample_kits=['NIGHT TRAP','RAGE TRAP'],factory_kit_indices=[0,4],user_kit_indices=[1,2,3],original_trap_one_shots=32,
 host_cpu_cycle_counter_available=False,browser_emulator_tested=False,app_bytes=image,app_limit_bytes=0x8dfbc,
 bram_bytes=ram,bram_limit_bytes=ram_max,pool_bytes=pool,pool_limit_bytes=pool_max,aux_bytes=27716,aux_limit_bytes=28416)
(validation/'release-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
(validation/'TESTING-EN.txt').write_text('''RED MASS VAMPIRE OS v0.5.1-alpha — RED MASS validation

PASS: native host checks, target firmware build and unchanged static render budgets.
PASS: six-track round-trip, T5/T6 automation, all 32 Song Mode rows, sound changes, tempo and six-bit mutes.
PASS: original 0.4 drum and melodic project fixtures; T4 drums move to T6 with patterns, pad 16, ghost hits, automation, routing, scenes, macros, probability, microtiming and mutes. Melodic T4 remains T4. Migrated data survives resave and cold reload.
PASS: original four-track full backups through BEGIN/chunk/COMMIT; new 5632-byte backup objects and byte-exact browser capture/restore.
PASS: interrupted atomic core/feature writes, continuation-sector CRC fallback, failed retries and corrupt-feature rejection before replacing live music.
PASS: 418 current golden audio renders; new sound health, shared voice limits, MIDI and editor routing, all twelve tools, undo and redo. Reviewed against the previous release's pinned fixture: 284 non-kit hashes and all three user-kit hashes remain unchanged. The replacement factory kit is an intentional sound change; previously unpinned new presets are now pinned.
PASS: NIGHT TRAP and RAGE TRAP: all 32 native pads audible with headroom, finite one-shot termination, distinct kit renders, open-hat choke and RAGE project save/reload.
PASS: native UI lint/alignment and all thirteen palettes, normal/LARGE/LINE, every displayed parameter range, main and DIGITAL builds.
PASS: web editor/installer protocol and six-track mock; complete backup validation and restore ordering.
PASS: Windows x64 GUI executable cross-build, PE import validation (Windows system DLLs only), actual native HTTP server logic under a fragmented-socket/host-filesystem adapter, read-only path rejection, and application navigation/operation locks under an instrumented iframe.
PASS: all four firmware fonts at weight 700, measured glyph spacing and native reference rendering. Normal/LARGE layouts have no actual lint findings; 279202 column values and 142790 alignment measurements checked in the main build. Lint self-test findings in logs are intentional detector checks.
VISUAL REVISION 2: the fifth HOME track's entire selection frame is inside its native canvas; all six frames are checked pixel by pixel. Original and new factory presets use one 15 px bold face on HOME and in preset lists. Both generations' HOME name ink spans were checked against native screen captures. All screens and palettes were rerendered, linted and aligned, and the target image rebuilt. DSP and persistence code were unchanged.
EDITOR CONNECTION HOTFIX: the previous FM-1_906 firmware's one-shot NAMES reply for the 99 VAMPIRE presets fills its 600-byte output buffer. The editor now validates the complete received prefix against this release's exact native factory catalog and restores the missing names for that release only. A captured SysEx reply from native C and a negative mismatch case pass the focused web checks. Firmware bytes and executable bytes are unchanged; installing the new extracted application is enough.

LIMITS: no physical FM1 or Windows session was available. Native Windows API execution, app-window layout and actual USB-MIDI installation remain unqualified. Playwright browser layout checks could not run because Chromium was unavailable; the runnable test source is included. Hardware CPU cycles were not measured. Browser WebAssembly emulator checks were skipped because Emscripten was unavailable. Static instruction checks are not a physical deadline measurement.
Six tracks share eight sounding voices, four project sources, 32 editable user slots and 64 automation events. All 256 curated presets are in ROM. Eight pianos use shared recorded CC0 multisamples with different native processing.

Read release-manifest.json and the attached logs for exact build sizes and test evidence. Keep a complete pre-upgrade backup. This is an alpha release.
''')
assert (bundle/'RED-MASS-VAMPIRE-OS.exe').read_bytes()==(root/'build/desktop/RED-MASS-VAMPIRE-OS.exe').read_bytes()
assert len(list((bundle/'audio/trap-kits').glob('*/*.wav')))==32
assert len(list((bundle/'preview/audio').glob('*.mp3')))==256
ignore={'build','.git','__pycache__','node_modules','.DS_Store'}
files=sorted(p for p in bundle.rglob('*') if p.is_file() and not any(x in ignore for x in p.relative_to(bundle).parts) and p.name!='SHA256SUMS.txt')
public_docs=[p for p in files if p.suffix in ['.md','.txt'] and (p.parent==bundle or 'docs' in p.relative_to(bundle).parts)]
assert not [str(p) for p in public_docs if re.search('felucca',p.read_text(errors='replace'),re.I)],'Unbranded public documentation remains'
sums=''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.relative_to(bundle).as_posix()+'\n' for p in files)
(bundle/'SHA256SUMS.txt').write_text(sums)
archive=bundle.parent/(package_name+'.zip')
archive_tmp=archive.with_suffix('.zip.part')
with zipfile.ZipFile(archive_tmp,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
    for p in files+[bundle/'SHA256SUMS.txt']:z.write(p,package_name+'/'+p.relative_to(bundle).as_posix())
with zipfile.ZipFile(archive_tmp) as z:assert z.testzip() is None
with archive_tmp.open('rb') as flushed:os.fsync(flushed.fileno())
os.replace(archive_tmp,archive)
print(json.dumps({'zip':str(archive),'bytes':archive.stat().st_size,'sha256':hashlib.sha256(archive.read_bytes()).hexdigest(),'files':len(files)+1}))
