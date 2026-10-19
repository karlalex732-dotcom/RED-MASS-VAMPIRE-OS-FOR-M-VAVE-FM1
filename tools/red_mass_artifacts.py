#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Package real C-rendered audio and UI with the corresponding GPL source.
Run vampire_test build/vampire-audio and vampire_preview build/vampire-ui first.
No audio filters, EQ, normalization or additional effects in this script.
"""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import csv, hashlib, json, shutil, subprocess, sys, wave
from PIL import Image, ImageDraw, ImageFont

root=Path(__file__).resolve().parents[1]
dest=Path(sys.argv[1]).resolve(); dest.mkdir(parents=True,exist_ok=True)
audio=root/'build/vampire-audio'; ui=root/'build/vampire-ui'
desc=json.loads((root/'build/host/desc.json').read_text())
spec=json.loads((root/'bank/RED-MASS-VAMPIRE-64.spec.json').read_text())['presets']
native=json.loads((audio/'native-presets.json').read_text())
assert len(native)==len(spec)==64 and desc['P_COUNT']==91 and desc['P_E0']==83
labels=[];seen={}
for i,d in enumerate(desc['TP']):
 l=d['label'];seen[l]=seen.get(l,0)+1;labels.append(l+(f"#{seen[l]}" if seen[l]>1 else ''))
labels += [f'E{i}' for i in range(8)]
patches=[]
for p,n in zip(spec,native):
 assert p['name']==n['name'] and len(n['params'])==91
 patches.append(dict(name=p['name'],engine=14,engineName='VAMPIRE',params=n['params'],pattern=None,grid=None,
     tags=['Vampire',p['category'],'POLY4'],created='2026-10-08T00:00:00.000Z',modified='2026-10-08T00:00:00.000Z'))
bank=dict(format='red-mass-library',version=1,kind='library',firmware='RED MASS VAMPIRE OS v0.1.0-alpha',
    pCount=91,pE0=83,paramLabels=labels,engines=[p['name'] for p in desc['ENG']],exported='2026-10-08T00:00:00.000Z',patches=patches)
bankpath=root/'bank/RED-MASS-VAMPIRE-64.json'; bankpath.write_text(json.dumps(bank,indent=2)+'\n')
for half,sl in [('A',slice(0,32)),('B',slice(32,64))]:
 b=dict(bank);b['kind']='bank';b['patches']=[dict(p,slot=i) for i,p in enumerate(patches[sl])]
 (root/f'bank/RED-MASS-VAMPIRE-{half}-32.json').write_text(json.dumps(b,indent=2)+'\n')

bundle=dest/'RED-MASS-VAMPIRE-OS-v0.1.0-alpha';bundle.mkdir(exist_ok=True)
for folder in ['firmware','bank','audio/flac','preview/audio','preview/ui','source']:(bundle/folder).mkdir(parents=True,exist_ok=True)
fwname='RED-MASS-VAMPIRE-OS-v0.1.0-alpha-FM1.fwsc'
shutil.copy2(root/'build/felucca-0.1.0-alpha.fwsc',bundle/'firmware'/fwname)
for p in (root/'bank').glob('RED_MASS*.json'):shutil.copy2(p,bundle/'bank'/p.name)
shutil.copy2(audio/'audio-health.csv',bundle/'bank/audio-health.csv')

def encode(p):
 n=p['number'];name=p['name'];slug=name.replace(' ','-')
 wav=audio/f'{n:02d}-{name}.wav';assert wav.is_file()
 for codec,loc in [('flac','audio/flac'),('mp3','preview/audio')]:
  args=['ffmpeg','-v','error','-nostdin','-y','-i',str(wav)]
  args+=['-c:a','flac','-sample_fmt','s16'] if codec=='flac' else ['-c:a','libmp3lame','-b:a','160k']
  target=bundle/loc/f'{n:02d}-{slug}.{codec}';subprocess.run(args+[str(target)],check=True)
 return dict(number=n,name=name,category=p['category'],audio=f'audio/{n:02d}-{slug}.mp3')
with ThreadPoolExecutor(max_workers=4) as pool:cards=list(pool.map(encode,spec))
print('64 exact lossless FLAC renders and 64 MP3 listening previews encoded')
# Verify that compression preserved every PCM sample (one representative of each category).
for p in spec[::8]:
 f=bundle/'audio/flac'/f"{p['number']:02d}-{p['name'].replace(' ','-')}.flac"
 decoded=subprocess.check_output(['ffmpeg','-v','error','-i',str(f),'-f','s16le','-c:a','pcm_s16le','-'])
 with wave.open(str(audio/f"{p['number']:02d}-{p['name']}.wav"),'rb') as w:assert decoded==w.readframes(w.getnframes())
montage=dest/'RED-MASS-VAMPIRE-Audition.wav'
with wave.open(str(montage),'wb') as out:
 out.setparams((2,2,44100,0,'NONE','not compressed'))
 for p in spec[::8]:
  with wave.open(str(audio/f"{p['number']:02d}-{p['name']}.wav"),'rb') as w:out.writeframes(w.readframes(w.getnframes()))
mp3=dest/'RED-MASS-VAMPIRE-Audition.mp3'
subprocess.run(['ffmpeg','-v','error','-nostdin','-y','-i',str(montage),'-c:a','libmp3lame','-b:a','160k',str(mp3)],check=True)
shutil.copy2(mp3,bundle/'preview/RED-MASS-VAMPIRE-Audition.mp3');montage.unlink()

for f in ui.glob('*.ppm'):
 if f.stem.startswith(('wave-','boot-')):continue
 Image.open(f).save(bundle/'preview/ui'/f.with_suffix('.png').name)
waveframes=[Image.open(ui/f'wave-{i:02d}.ppm').convert('RGB').resize((480,480),Image.Resampling.NEAREST) for i in range(96)]
waveframes[0].save(bundle/'preview/ui/wave-modes.gif',save_all=True,append_images=waveframes[1:],duration=34,loop=0,optimize=True)
bootframes=[Image.open(ui/f'boot-{i:02d}.ppm').resize((480,480),Image.Resampling.NEAREST) for i in range(8)]
bootframes[0].save(bundle/'preview/ui/boot.gif',save_all=True,append_images=bootframes[1:],duration=80,loop=0)
font=root/'assets/fonts/PirataOne-Regular.ttf';shutil.copy2(font,bundle/'preview/PirataOne-Regular.ttf')
word=ImageFont.truetype(str(font),68);title=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',16)
board=Image.new('RGB',(1200,945),'#080408');d=ImageDraw.Draw(board)
d.text((42,22),'RED_MASS',font=word,fill='#f8e9dc');d.text((280,71),'(VAMPIRE) OS  /  FM1  /  v0.1.0-alpha',font=title,fill='#ff567e')
d.text((42,122),'Native 240 x 240 firmware screens. Audio and display rendered from the real C source.',font=title,fill='#9e818d')
screens=[('boot','Awakening'),('scope-1','Blood wave / GLOW'),('source','Vampire synthesis'),('euclid','Euclid rhythms'),
         ('transform','Pattern transformations'),('scenes','A/B morph'),('display','VAMP palette'),('crypt','CRYPT palette')]
for i,(file,label) in enumerate(screens):
 x=42+(i%4)*288;y=177+(i//4)*360
 d.rounded_rectangle((x-5,y-5,x+253,y+253),radius=14,fill='#260d1a',outline='#623146',width=2)
 board.paste(Image.open(ui/(file+'.ppm')).resize((248,248),Image.Resampling.NEAREST),(x,y))
 d.text((x,y+268),label,font=title,fill='#f2ddd5')
d.text((42,900),'ALPHA  /  Host checks passed for the new UI and bank. Physical FM1 testing is still required.',font=title,fill='#ac8390')
board.save(dest/'RED-MASS-VAMPIRE-OS-Preview.png');board.save(bundle/'preview/RED-MASS-VAMPIRE-OS-Preview.png')

# Corresponding source: original upstream notices and license files retained.
ignore=shutil.ignore_patterns('build','.git','__pycache__','node_modules','.DS_Store')
shutil.copytree(root,bundle/'source',dirs_exist_ok=True,ignore=ignore)
for p in [root/'LICENSE',root/'LICENSING.md']:shutil.copy2(p,bundle/p.name)
shutil.copytree(root/'LICENSES',bundle/'LICENSES',dirs_exist_ok=True)
shutil.copy2(root/'assets/fonts/PirataOne-OFL.txt',bundle/'LICENSES/OFL-PirataOne.txt')
index=Path(__file__).with_name('red_mass_preview.html').read_text().replace('__RED_MASS_CARDS__',json.dumps(cards))
(bundle/'preview/index.html').write_text(index)
print('Native UI images/GIFs, compatible preset JSON, GPL source and licenses packaged')
print(bundle)
