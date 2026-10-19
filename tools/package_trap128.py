#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Package 128 native DSP renders with exact presets and corresponding source.
FLAC is bit-exact, MP3 is for preview; no post-render EQ/normalization/FX.
"""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import csv,hashlib,json,shutil,subprocess,sys,wave,zipfile
from PIL import Image
root=Path(__file__).resolve().parents[1]
dest=Path(sys.argv[1]);dest.mkdir(parents=True,exist_ok=True)
bundle=dest/'RED-MASS-VAMPIRE-OS-v0.2.0-alpha';bundle.mkdir(exist_ok=True)
audio=root/'build/trap128-audio'
spec=json.loads((root/'bank/RED-MASS-Trap-Vampire-128.spec.json').read_text())['presets']
for f in ('firmware','bank','audio/flac','preview/audio','preview/ui','validation'):(bundle/f).mkdir(parents=True,exist_ok=True)
fw='RED-MASS-VAMPIRE-OS-v0.2.0-alpha-FM1.fwsc'
shutil.copy2(root/'build/felucca-0.2.0-alpha.fwsc',bundle/'firmware'/fw)
shutil.copy2(bundle/'firmware'/fw,dest/fw)
for p in (root/'bank').glob('RED-MASS-Trap-Vampire-*'):shutil.copy2(p,bundle/'bank'/p.name)
shutil.copy2(root/'bank/RED-MASS-Trap-Vampire-128.json',dest/'RED-MASS-Trap-Vampire-128.json')
for name in ('audio-health.csv','sound-features.csv','diversity-audit.json','native-presets.json'):
 shutil.copy2(audio/name,bundle/'validation'/name)
def encode(p):
 n=p['number'];slug=p['name'][4:];wav=audio/f'{n:03d}.wav'
 flac=bundle/'audio/flac'/f'{n:03d}-{slug}.flac'
 mp3=bundle/'preview/audio'/f'{n:03d}-{slug}.mp3'
 subprocess.run(['ffmpeg','-v','error','-nostdin','-y','-i',str(wav),'-c:a','flac','-sample_fmt','s16',str(flac)],check=True)
 decoded=subprocess.check_output(['ffmpeg','-v','error','-nostdin','-i',str(flac),'-f','s16le','-c:a','pcm_s16le','-'])
 with wave.open(str(wav),'rb') as w:assert decoded==w.readframes(w.getnframes())
 subprocess.run(['ffmpeg','-v','error','-nostdin','-y','-i',str(wav),'-c:a','libmp3lame','-b:a','160k',str(mp3)],check=True)
 return dict(number=n,name=p['name'],category=p['category'],engine=p['engine'],voices=min(4,p['polyphony']),audio=f'audio/{mp3.name}')
with ThreadPoolExecutor(max_workers=6) as pool:cards=list(pool.map(encode,spec))
print('128 FLAC files verified bit-for-bit; 128 MP3 preview files encoded',flush=True)
montage=dest/'RED-MASS-Trap-Vampire-128-Audition.wav'
with wave.open(str(montage),'wb') as w:
 w.setparams((2,2,44100,0,'NONE','not compressed'))
 for p in spec[::8]:
  with wave.open(str(audio/f"{p['number']:03d}.wav"),'rb') as src:w.writeframes(src.readframes(44100*8))
mp3=dest/'RED-MASS-Trap-Vampire-128-Audition.mp3'
subprocess.run(['ffmpeg','-v','error','-nostdin','-y','-i',str(montage),'-c:a','libmp3lame','-b:a','160k',str(mp3)],check=True)
shutil.copy2(mp3,bundle/'preview'/mp3.name);montage.unlink()
ui=root/'build/vampire-ui'
for p in ui.glob('*.ppm'):
 if p.stem.startswith(('wave-','boot-')):continue
 Image.open(p).save(bundle/'preview/ui'/p.with_suffix('.png').name)
frames=[Image.open(ui/f'wave-{i:02d}.ppm').resize((480,480),Image.Resampling.NEAREST) for i in range(96)]
frames[0].save(bundle/'preview/ui/wave-modes.gif',save_all=True,append_images=frames[1:],duration=34,loop=0,optimize=True)
shutil.copy2(root/'assets/fonts/PirataOne-Regular.ttf',bundle/'preview/PirataOne-Regular.ttf')
page=Path(__file__).with_name('red_mass_preview.html').read_text()
page=page.replace('64 sounds','128 trap sounds').replace('v0.1.0-alpha','v0.2.0-alpha').replace('64 POLYPHONIC SOUNDS','128 TRAP / VAMPIRE SOUNDS')
page=page.replace('Все 64 звука','Все 128 звуков').replace('RED-MASS-VAMPIRE-64.json','RED-MASS-Trap-Vampire-128.json').replace('64 пресета JSON','128 пресетов JSON')
page=page.replace('RED-MASS-VAMPIRE-Audition.mp3','RED-MASS-Trap-Vampire-128-Audition.mp3').replace('Демо: 8 категорий','Демо: 16 категорий')
page=page.replace('Тёмные хоры и крики, дождь и грозы, колокола, струны и органы. Каждый пресет звучит четырёхнотным аккордом через настоящий движок VAMPIRE, фильтр, модуляцию и эффекты OS.',
 '128 новых трэп-тембров: 808, сабы, Reese, говорящие басы, колокола, плэки, клавиши, лиды, хоры, пэды, струны, органы, вокальные стабы, пульсации, перкуссия и кошмарные текстуры. Десять нативных движков; каждый звук прошёл их собственный синтез, модуляцию и эффекты RED_MASS OS. FM6-звуки содержат отдельные шестиоператорные патчи.')
page=page.replace('12 секунд · аккорд C minor: 48 / 55 / 60 / 63 · 3 секунды удержания, затем хвост эффектов.',
 '12 секунд · C minor: 48 / 55 / 60 / 63; басовые категории — на октаву ниже. 3 секунды удержания и хвост. PHYS ограничен тремя голосами, остальные примеры — четырьмя.')
page=page.replace(" / 64 · POLY 4`"," / 128 · POLY ${c.voices} · ${c.engine}`").replace("+' / POLY 4'","+' / '+c.engine+' / POLY '+c.voices")
page=page.replace('__RED_MASS_CARDS__',json.dumps(cards))
(bundle/'preview/index.html').write_text(page)
ignore=shutil.ignore_patterns('build','.git','__pycache__','node_modules','.DS_Store')
shutil.copytree(root,bundle/'source',dirs_exist_ok=True,ignore=ignore)
for f in ('LICENSE','LICENSING.md'):shutil.copy2(root/f,bundle/f)
shutil.copytree(root/'LICENSES',bundle/'LICENSES',dirs_exist_ok=True)
shutil.copy2(root/'assets/fonts/PirataOne-OFL.txt',bundle/'LICENSES/OFL-PirataOne.txt')
shutil.copy2(root/'RED-MASS-TRAP128-README.txt',bundle/'README-RU.txt')
print('Source, firmware, four slot banks, native wave animation and searchable player packaged',flush=True)
print(bundle,flush=True)
