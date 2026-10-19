#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Stage/finalize the English firmware pack. Finalization requires passing tests."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import argparse,csv,hashlib,html,json,re,shutil,subprocess,wave,zipfile
from PIL import Image

root=Path(__file__).resolve().parents[1]
ap=argparse.ArgumentParser()
ap.add_argument('--previous',required=True,type=Path)
ap.add_argument('--out',required=True,type=Path)
ap.add_argument('--final',action='store_true')
args=ap.parse_args()
bundle=args.out/'RED-MASS-VAMPIRE-OS-v0.4.0-alpha-English-Pack'
for f in ['firmware','bank','docs/screens','preview/audio','audio/pianos','validation']:
 (bundle/f).mkdir(parents=True,exist_ok=True)
spec=json.loads((root/'bank/RED-MASS-Trap-Vampire-128.spec.json').read_text())['presets']
spec+=json.loads((root/'bank/RED-MASS-Club-Vampire-128.spec.json').read_text())['presets']

def inline(s):
 s=html.escape(s)
 s=re.sub(chr(96)+'([^'+chr(96)+']+)'+chr(96),r'<code>\1</code>',s)
 return re.sub(r'\*\*([^*]+)\*\*',r'<strong>\1</strong>',s)

def markdown(s):
 ls=s.splitlines();out=[];i=0
 while i<len(ls):
  l=ls[i];i+=1
  if not l.strip():continue
  if l.startswith(chr(96)*3):
   a=[]
   while i<len(ls) and not ls[i].startswith(chr(96)*3):a.append(ls[i]);i+=1
   i+=1;out.append('<pre>'+html.escape('\n'.join(a))+'</pre>');continue
  if l.startswith('#'):
   n=len(l)-len(l.lstrip('#'));out.append(f'<h{n}>'+inline(l[n:].strip())+f'</h{n}>');continue
  if l.startswith('|'):
   rows=[l]
   while i<len(ls) and ls[i].startswith('|'):rows.append(ls[i]);i+=1
   a=[]
   for j,row in enumerate(rows):
    if j==1 and re.match(r'^\|[\s:|\-]+$',row):continue
    tag='th' if j==0 else 'td'
    a.append('<tr>'+''.join(f'<{tag}>'+inline(x.strip())+f'</{tag}>' for x in row.strip('|').split('|'))+'</tr>')
   out.append('<div class="table"><table>'+''.join(a)+'</table></div>');continue
  if re.match(r'^(?:- |\d+\. )',l):
   tag='ol' if re.match(r'^\d+\.',l) else 'ul';a=[re.sub(r'^(?:- |\d+\. )','',l)]
   while i<len(ls) and re.match(r'^(?:- |\d+\. )',ls[i]):a.append(re.sub(r'^(?:- |\d+\. )','',ls[i]));i+=1
   out.append(f'<{tag}>'+''.join('<li>'+inline(x)+'</li>' for x in a)+f'</{tag}>');continue
  if l.startswith('> '):out.append('<aside>'+inline(l[2:])+'</aside>');continue
  a=[l]
  while i<len(ls) and ls[i] and not re.match(r'^(?:#|\||>|- |\d+\. )',ls[i]) and not ls[i].startswith(chr(96)*3):a.append(ls[i]);i+=1
  out.append('<p>'+inline(' '.join(a))+'</p>')
 return '\n'.join(out)

def encode(p):
 n=p['number'];slug=p['name'][4:];name=f'{n:03d}-{slug}.mp3';dest=bundle/'preview/audio'/name
 if n<=128:shutil.copy2(args.previous/'preview/audio'/name,dest)
 else:
  wav=root/f'build/club128-audio/{n:03d}.wav'
  if not dest.exists():subprocess.run(['ffmpeg','-v','error','-nostdin','-y','-i',str(wav),'-c:a','libmp3lame','-b:a','128k',str(dest)],check=True)
  if n>=249:
   lossless=bundle/'audio/pianos'/f'{n:03d}-{slug}.flac'
   if not lossless.exists():subprocess.run(['ffmpeg','-v','error','-nostdin','-y','-i',str(wav),'-c:a','flac','-sample_fmt','s16',str(lossless)],check=True)
   decoded=subprocess.check_output(['ffmpeg','-v','error','-nostdin','-i',str(lossless),'-f','s16le','-c:a','pcm_s16le','-'])
   with wave.open(str(wav),'rb') as w:assert decoded==w.readframes(w.getnframes())
 return dict(number=n,name=p['name'],category=p['category'],engine=p['engine'],voices=min(4,p['polyphony']),audio='audio/'+name)

with ThreadPoolExecutor(max_workers=4) as ex:cards=list(ex.map(encode,spec))
assert len(cards)==256
for p in (root/'bank').glob('RED-MASS-VAMPIRE-256*'):shutil.copy2(p,bundle/'bank'/p.name)
for name in ['RED-MASS-Club-Vampire-128.json','RED-MASS-Club-Vampire-128.spec.json','RED-MASS-Trap-Vampire-128.spec.json']:
 shutil.copy2(root/'bank'/name,bundle/'bank'/name)
for p in (root/'docs').glob('*'):
 if p.is_file():shutil.copy2(p,bundle/'docs'/p.name)
names=['COPY','CTRL ALL','LOCKS','MACROS','MACRO SET','MELODY','MUTATE','BANK 256','MUTE GROUP','DRUM CHANCE','MICRO','HISTORY','SONG ROW','SONG SET','SCENES']
gallery=[]
for i,name in enumerate(names,1):
 file=f'tool-{i:02d}.png';Image.open(root/f'build/upgrade-ui/tool-{i:02d}.ppm').save(bundle/'docs/screens'/file)
 gallery.append(f'<figure><img src="screens/{file}" alt="{name}"><figcaption>{name}</figcaption></figure>')
css="""body{margin:0;background:#faf7f8;color:#201a21;font:16px/1.6 system-ui}main{max-width:1000px;margin:auto;padding:32px 26px 80px}header{background:#180e16;color:white;padding:40px 26px}header div{max-width:1000px;margin:auto}header b{color:#ff689a;font-size:14px;letter-spacing:2px}h1{font-size:38px;line-height:1.15}h2{margin-top:44px;color:#8e2450}h3{margin-top:28px}p,li{max-width:85ch}li{margin:8px 0}a{color:#ad2157}code{font-size:.9em;background:#ece6e9;padding:2px 4px}pre{overflow:auto;background:#211823;color:#f8e9ef;padding:20px;border-radius:10px}aside{border-left:4px solid #cc3773;background:#f4e7ed;padding:14px 20px}.table{overflow:auto}table{border-collapse:collapse;width:100%;font-size:14px}th,td{text-align:left;vertical-align:top;padding:10px;border-bottom:1px solid #ddcbd3}th{background:#efdee5}nav{display:flex;gap:20px;flex-wrap:wrap}.screens{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:18px}.screens img{width:100%;max-width:240px;border-radius:6px}.screens figure{margin:0}figcaption{font-size:14px}@media print{header{background:white;color:black;padding:0}main{padding:0}h2{break-after:avoid}table,pre,figure{break-inside:avoid}}"""
guide='<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>RED_MASS v0.4 — English Field Guide</title><style>'+css+'</style><header><div><b>FM1 · VAMPIRE OS · ALPHA</b><h1>Your complete v0.4 field guide</h1><p>256 sounds. A full-song workflow. Twelve new tools.</p></div></header><main><nav><a href="../preview/index.html">Listen to all 256 sounds</a><a href="Quick-Reference-EN.txt">Quick reference</a><a href="../validation/TESTING-EN.txt">Validation report</a></nav>'+markdown((root/'docs/RED-MASS-English-Guide.md').read_text())+'<h2>Actual firmware UI renders</h2><p>Captured from the C firmware on a simulated display; these are not physical FM1 photographs.</p><div class="screens">'+''.join(gallery)+'</div></main></html>'
(bundle/'docs/RED-MASS-English-Guide.html').write_text(guide)
player="""<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>RED_MASS — 256 sounds</title><style>body{margin:0;background:#100b10;color:#f7eaf0;font:16px system-ui}main{max-width:1120px;margin:auto;padding:40px 24px}h1{font-size:clamp(36px,7vw,72px);line-height:1;max-width:850px}strong{color:#ff568f}a{color:#ff8caf}p{line-height:1.6;max-width:820px}input,select{font:inherit;border:1px solid #654251;border-radius:7px;background:#22131d;color:white;padding:10px;margin:5px 8px 5px 0}#list{display:grid;grid-template-columns:repeat(auto-fit,minmax(270px,1fr));gap:14px;margin-top:24px}article{border:1px solid #3d2633;padding:18px;border-radius:12px;background:#1b1118}article h2{font-size:19px;margin:0 0 8px}article small{color:#cbb5c1}audio{width:100%;margin-top:16px}.note{padding:16px;border-left:3px solid #ff568f;background:#21121b}#count{color:#ff8caf}</style><main><strong>RED_MASS VAMPIRE OS · v0.4</strong><h1>256 sounds for the dark hours.</h1><p>Keep the original 128. Explore 40 new plucks, 40 pads, 40 leads and eight sampled grand piano variations. Every example passes through the native synthesis engine, modulation and RED_MASS effects.</p><p><a href="../docs/RED-MASS-English-Guide.html">Read the English guide</a> · <a href="../bank/RED-MASS-VAMPIRE-256.json" download>Download all 256 presets</a></p><p class="note">Four audio tracks · Eight shared voices · 32 editable user slots. Eight pianos share recorded CC0 grand piano multisamples with different native processing. This is an alpha; physical FM1 testing is pending.</p><input id="search" placeholder="Search name or engine" aria-label="Search sounds"><select id="group" aria-label="Preset group"><option value="all">All 256</option><option value="old">Existing 128</option><option value="new">New 128</option><option value="piano">Eight pianos</option></select><select id="category" aria-label="Category"><option value="">All categories</option></select><p id="count"></p><div id="list"></div><p>12-second stereo examples. C minor chord, three seconds held, then release/effect tail. Bass examples use the lower register. PHYS examples have three voices; other examples have four. MP3 encoding is the only post-render processing.</p></main><script>const sounds=CARDS;const search=document.getElementById('search'),group=document.getElementById('group'),category=document.getElementById('category'),list=document.getElementById('list'),count=document.getElementById('count');for(const c of [...new Set(sounds.map(x=>x.category))]){let o=document.createElement('option');o.value=c;o.textContent=c;category.append(o)}function render(){const q=search.value.toLowerCase(),g=group.value,c=category.value;list.replaceChildren();const shown=sounds.filter(x=>(g==='all'||g==='old'&&x.number<=128||g==='new'&&x.number>128||g==='piano'&&x.number>=249)&&(!c||x.category===c)&&(x.name+' '+x.engine).toLowerCase().includes(q));count.textContent=shown.length+' sounds';for(const x of shown){const a=document.createElement('article'),h=document.createElement('h2'),s=document.createElement('small'),p=document.createElement('audio');h.textContent=x.name;s.textContent=x.category+' · '+x.engine+' · '+x.voices+' voices';p.controls=true;p.preload='none';p.src=x.audio;p.addEventListener('play',()=>document.querySelectorAll('audio').forEach(y=>{if(y!==p)y.pause()}));a.append(h,s,p);list.append(a)}}for(const el of [search,group,category])el.addEventListener('input',render);render();</script></html>"""
(bundle/'preview/index.html').write_text(player.replace('CARDS',json.dumps(cards,separators=(',',':'))))
readme="""RED MASS VAMPIRE OS v0.4.0-alpha — M-VAVE FM1
Package identity FM-1_904

START HERE: docs/RED-MASS-English-Guide.html
Fast controls: docs/Quick-Reference-EN.txt
New functions: docs/Upgrade-v0.4-EN.md
Listen: preview/index.html (all 256 presets)

All twelve selected tools, expanded 32-row Song Mode, existing 128 + 128 new presets, including eight sampled piano variations. Four tracks/eight shared voices. 256 built-in ROM presets and a separate 32-slot editable user bank. Four saved projects A–D are reused as arrangement sources.

INSTALL
Extract the complete ZIP. Back up current music and keep a pre-upgrade backup.
Windows:
py -3.12 -m pip install mido python-rtmidi
py -3.12 source/tools/fm1_install.py --info
py -3.12 source/tools/fm1_install.py firmware/RED-MASS-VAMPIRE-OS-v0.4.0-alpha-FM1.fwsc
macOS/Linux: replace py -3.12 with python3.
Expected identity after install: FM-1_904. Follow installer model checks.

LOCAL BROWSER TOOLS
python3 -m http.server 8765 --directory web-tools
Installer: http://localhost:8765/webapp/installer/
Editor: http://localhost:8765/webapp/editor/
Use Chrome/Edge, a USB data cable and MIDI/SysEx permission.
The browser supports extended Song Mode and complete feature backups; the new performance tools are operated on the FM1 panel.

SOUNDS AND PROJECTS
bank/RED-MASS-VAMPIRE-256.json is the full computer library. Optional A–H files contain 32 sounds for the same 32 user slots, not eight onboard banks. Built-in presets need no import.
The eight pianos share existing recorded CC0 multisamples with different native processing. Lossless examples are in audio/pianos.
FUN10 saves include linked feature data. Use complete v0.4 backups for scenes, macros, mute groups, pad chances, microtiming and the full arranger. Older projects/backups migrate; older firmware cannot read FUN10 feature extensions. Missing feature data is rejected before replacing current music.

STATUS
See validation/TESTING-EN.txt for checks and limits. Physical FM1 qualification is pending. This independent alpha is based on Felucca; corresponding GPL-3.0-only source and notices are included. This pack does not contain an updated Reels video.
Verify integrity using SHA256SUMS.txt.
"""
(bundle/'README-EN.txt').write_text(readme)
if not args.final:
 print('English docs, 256 previews, eight bit-exact FLAC pianos and bank files staged:',bundle)
 raise SystemExit(0)
log=(root/'build/upgrade-suite.log').read_text(errors='replace')
assert log.rstrip().endswith('ALL HOST TESTS PASSED'),'Final suite has not passed'
target=(root/'build/upgrade-target.log').read_text()
assert '  ok    image ' in target and 'identity FM-1_904' in target
fw=root/'build/felucca-0.4.0-alpha.fwsc';fwname='RED-MASS-VAMPIRE-OS-v0.4.0-alpha-FM1.fwsc'
shutil.copy2(fw,bundle/'firmware'/fwname)
subprocess.run(['python3','web/make_site.py',str(fw),'0.4.0-alpha',str(bundle/'web-tools')],cwd=root,check=True)
installer=bundle/'web-tools/webapp/installer/index.html'
installer.write_text(installer.read_text().replace('<title>Felucca','<title>RED_MASS').replace('<h1>Felucca</h1>','<h1>RED MASS VAMPIRE OS</h1>'))
shutil.copytree(root,bundle/'source',dirs_exist_ok=True,ignore=shutil.ignore_patterns('build','.git','__pycache__','node_modules','.DS_Store'))
for name in ['LICENSE','LICENSING.md']:shutil.copy2(root/name,bundle/name)
shutil.copytree(root/'LICENSES',bundle/'LICENSES',dirs_exist_ok=True)
shutil.copy2(root/'assets/fonts/PirataOne-OFL.txt',bundle/'LICENSES/OFL-PirataOne.txt')
for name in ['upgrade-suite.log','upgrade-target.log','upgrade-check.log','upgrade-persistence-check.log']:
 shutil.copy2(root/'build'/name,bundle/'validation'/name)
for name in ['report.txt','align.txt']:shutil.copy2(root/'build/ui_new'/name,bundle/'validation'/('ui-'+name))
for name in ['audio-health.csv','native-presets.json']:shutil.copy2(root/'build/club128-audio'/name,bundle/'validation'/('new128-'+name))
health=bundle/'validation/new128-audio-health.csv';rows=list(csv.DictReader(health.open()))
with health.open('w',newline='') as f:
 w=csv.DictWriter(f,fieldnames=rows[0].keys());w.writeheader()
 for r in rows:r['number']=str(int(r['number'])+128);w.writerow(r)
mem=re.search(r'image (\d+) B; RAM .data\+\.bss (\d+) B of (\d+); pool (\d+) B of (\d+)',target)
values=list(map(int,mem.groups()))
manifest=dict(product='RED MASS VAMPIRE OS',version='0.4.0-alpha',identity='FM-1_904',firmware=fwname,bytes=fw.stat().st_size,
 sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),audio_tracks=4,shared_voices=8,curated_rom_presets=256,
 existing_presets=128,new_presets=128,new_piano_variations=8,user_slots=32,song_rows=32,song_project_sources=4,max_row_repeats=64,
 host_suite_passed=True,target_build_passed=True,physical_device_tested=False,host_cpu_cycle_counter_available=False,
 browser_emulator_tested=False,app_bytes=values[0],app_limit_bytes=0x8DFBC,bram_bytes=values[1],bram_limit_bytes=values[2],pool_bytes=values[3],pool_limit_bytes=values[4])
(bundle/'validation/release-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
testing="""RED_MASS v0.4.0-alpha — VALIDATION

PASS: full host suite; native C panel/protocol tests; 32-row arranger, engine transitions, repeats/start/loop/length/BPM/mutes; all twelve tools; exact microtiming and compressed ratchets; sixteen-edit history; per-pad chance; four-track PHYS state claims/stealing; linked project save/load and feature backups; FUN9 migration retaining ghost/condition/FM6/DUST/DUCK fields.
PASS: interrupted NOR writes at 27 page-cut positions, including failed retries after an orphan extension. Reload retains a complete old or new project. Missing/corrupt feature data is rejected before changing current music.
PASS: 128 new processed DSP renders: peaks, DC, voice counts, release, distinct hashes and preset persistence. Existing 417 golden renders are unchanged. Appended renders also receive health checks.
PASS: every UI page/value/palette, normal/LARGE/LINE; no layout lint findings or alignment offsets of one pixel or more.
PASS: web editor/backup/installer against simulated devices; exact 256-library/eight 32-slot imports; eight FLAC piano examples decode bit-for-bit to the native WAVs.
PASS: pi32v2 target build, image/BRAM/pool bounds and static render-loop cost budget. Exact memory figures and firmware SHA-256 are in release-manifest.json.

LIMITS
No physical FM1 was available. Real boot, USB, flash-chip behavior, audio timing and sustained CPU load are not qualified.
The Linux host lacks the original macOS instruction-counter interface; runtime cycle checks are skipped. Static target budget checks passed.
The WebAssembly emulator was skipped because Emscripten was unavailable.
Eight piano programs share existing CC0 recorded multisamples; they are not eight distinct sample libraries.
Four tracks/eight voices/32 editable slots remain intentional limits. 32 arranger rows reuse four saved project sources.
Screenshots are native C UI renders on a simulated display, not hardware photographs. This is an alpha build.
"""
(bundle/'validation/TESTING-EN.txt').write_text(testing)
hashes=[]
for p in sorted(bundle.rglob('*')):
 if p.is_file() and p.name!='SHA256SUMS.txt':hashes.append(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+str(p.relative_to(bundle)))
(bundle/'SHA256SUMS.txt').write_text('\n'.join(hashes)+'\n')
archive=args.out/(bundle.name+'.zip')
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(bundle.rglob('*')):
  if p.is_file():z.write(p,str(p.relative_to(args.out)))
with zipfile.ZipFile(archive) as z:assert z.testzip() is None
print('FINAL:',archive,archive.stat().st_size,'bytes; firmware SHA-256',manifest['sha256'])

