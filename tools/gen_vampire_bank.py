#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""64 original polyphonic presets and four deterministic, original PCM textures.
No downloaded recordings or human performances. Sources: synthetic choir/scream,
procedural rain/thunder. The firmware applies the final synthesis, filter, modulation and FX.
"""
from pathlib import Path
import json, math, random, struct, sys, wave

root = Path(__file__).resolve().parents[1]
out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
groups = [
 ('Choirs', ['NOSFERATU','BLOOD MASS','VESPER','LILITH','DARK CHOIR','SANGUINE','VEIL VOICES','MIDNIGHT AA']),
 ('Screams', ['BANSHEE','FANG CRY','NIGHT WAIL','FERAL','LAST BREATH','BLOOD HOWL','CRYPT SCREAM','RED WHISPER']),
 ('Rain', ['CRYPT RAIN','BLACK RAIN','GRAVE DROPS','IRON RAIN','WET STONE','RAIN VEIL','NIGHT DRIZZL','RAIN MIRROR']),
 ('Storms', ['THUNDER','RED TEMPEST','STORM CELL','GRAVE RUMBLE','DARK SKY','BLOOD STORM','BLACK CLOUD','STORM CHOIR']),
 ('Textures', ['COFFIN MIST','MOURNING','ASH VEIL','GHOST DUST','TOMB AIR','SILVER FOG','SPORE','ETERNAL FOG']),
 ('Bells', ['TOLL','BLOOD BELL','CRYPT CHIME','GLASS FANG','FUNERAL','IRON CROSS','SILVER TOLL','MOON CHIME']),
 ('Strings', ['BLOOD SILK','RED VELVET','NIGHT STRNG','DARK THREAD','COFFIN BOW','GOTHIC SILK','MOON STRNG','SHADOW VEIL']),
 ('Organs', ['CATHEDRAL','BLACK ALTAR','VESPER PIPE','DARK ORGAN','VAMPIRE MASS','IRON PIPE','REQUIEM','ETERNAL']),
]
bank=[]
for mode,(category,names) in enumerate(groups):
 for i,name in enumerate(names):
  e=[mode, 34+((i*13+mode*11)%76), 8+((i*17+mode*9)%75),
     30+((i*19+mode*7)%80), 6+i*9, 6+((i*11+mode*4)%42), 24+i*11, (-12,0,0,7,12,-5,0,12)[i]]
  if mode==1: e[2]=48+i*7; e[5]=12+i*5
  if mode==2: e[3]=10+i*6; e[5]=30+i*3
  if mode==3: e[1]=22+i*5; e[3]=12+i*8
  if mode==5: e[3]=74+i*6
  if mode==7: e[3]=60+i*8; e[5]=i*2
  env=[50+i*4,70+(i*3)%35,104+(i*2)%22,75+i*6]
  if mode in (1,3): env=[(4+i*5 if mode==1 else i*3),80+i*4,45+i*9,80+i*5]
  if mode==5: env=[0,88+i*4,0,84+i*5]
  if mode==7: env=[4+i*3,80,118,68+i*7]
  fx=[6+(i*7+mode*3)%36,24+(i*9+mode*5)%60,12+(i*7+mode*3)%46,52+(i*7+mode*5)%62]
  bank.append(dict(number=len(bank)+1,name=name,category=category,engine='VAMPIRE',engine_id=14,
                   mode=mode,polyphony=4,voice_mode='POLY',engine_values=e,envelope=env,effects=fx,
                   filter_env=(-12+i*3),lfo_rate=20+i*7,lfo_depth=5+i*2,
                   matrix_amount=6+i*3,level=104))
assert len(bank)==64 and len({p['name'] for p in bank})==64
lines=['/* Generated original RED_MASS Vampire bank. Code GPL-3.0-only. */',
       'static const preset_t VAMPIRE_PRESETS[] = {']
for p in bank:
 lines.append('    {"'+p['name']+'", {'+','.join(map(str,p['engine_values']))+'}, {'+
              ','.join(map(str,p['envelope']))+'}, '+str(p['filter_env'])+', 0, FX('+
              ','.join(map(str,p['effects']))+')},')
lines+=['#include "trap_vampire_presets.inc"','#include "club_vampire_presets.inc"','};','static const uint8_t VAMPIRE_RATE[64] = {'+','.join(str(p['lfo_rate']) for p in bank)+'};',
        'static const uint8_t VAMPIRE_DEPTH[64] = {'+','.join(str(p['lfo_depth']) for p in bank)+'};',
        'static const uint8_t VAMPIRE_MATRIX[64] = {'+','.join(str(p['matrix_amount']) for p in bank)+'};']
(out/'vampire_presets.h').write_text('\n'.join(lines)+'\n')
bankdir=root/'bank'; bankdir.mkdir(exist_ok=True)
(bankdir/'RED-MASS-VAMPIRE-64.spec.json').write_text(json.dumps(dict(format='red_mass-vampire-bank',version=1,
    firmware='RED_MASS Vampire v0.1.0-alpha',status='host-rendered, hardware not tested',presets=bank),indent=2)+'\n')
sr,n=11025,8192
textures=[]
rawdir=root/'assets/vampire-sources'; rawdir.mkdir(exist_ok=True)
for mode,name in enumerate(('Synthetic_Choir','Synthetic_Scream','Procedural_Rain','Procedural_Thunder')):
 rng=random.Random(7331+mode*7919); data=[]; pole=0.; pole2=0.; burst=0.
 for i in range(n):
  tm=i/sr; noise=rng.uniform(-1.,1.); pole+=0.028*(noise-pole); pole2+=0.10*(noise-pole2)
  phase=2*math.pi*194*i/n
  if mode==0:
   v=sum(math.sin(phase*h)*math.exp(-((h*261-700)/400)**2)/h for h in range(1,19))
   v+=.20*math.sin(phase*3)+.10*math.sin(phase*8)+.02*noise
  elif mode==1:
   bent=phase+.35*math.sin(2*math.pi*3*i/n)
   v=.27*math.sin(bent)+.20*math.sin(bent*5)+.10*math.sin(bent*9)+.12*noise
   v=math.tanh(2*v)*(.65+.25*math.sin(2*math.pi*i/n)**2)
  elif mode==2:
   if rng.random()<.011: burst=rng.uniform(-1,1)
   burst*=.87
   v=.27*(noise-pole2)+.50*burst
  else:
   v=4.8*pole+.35*pole2+.12*math.sin(2*math.pi*8*i/n)
   v=math.tanh(v)*(.50+.40*math.sin(math.pi*i/n)**2)
  data.append(v)
 # Circular edge smoothing and a DC removal: playback loops have no jump.
 xf=256
 for i in range(xf):
  a=i/xf
  common=data[-xf+i]*(1-a)+data[i]*a
  data[-xf+i]=common
 data[-1]=data[0]
 dc=sum(data)/n; data=[v-dc for v in data]; peak=max(abs(v) for v in data)
 pcm=[round(v/peak*22000) for v in data]
 textures.append(pcm)
 with wave.open(str(rawdir/(name+'.wav')),'wb') as w:
  w.setparams((1,2,sr,n,'NONE','not compressed'));w.writeframes(struct.pack('<'+str(n)+'h',*pcm))
header=['/* Original synthetic/procedural textures; CC0-1.0 (see assets/vampire-sources/README.txt). */',
        '#define VAMP_PCM_N 8192u','#define VAMP_PCM_RATE 11025u',
        'static const int16_t VAMP_PCM[4][VAMP_PCM_N] = {']
for pcm in textures:
 header.append(' {')
 header.extend('  '+','.join(map(str,pcm[i:i+24]))+',' for i in range(0,n,24))
 header.append(' },')
header.append('};'); (out/'vampire_pcm.h').write_text('\n'.join(header)+'\n')
(rawdir/'README.txt').write_text('RED MASS VAMPIRE OS original generated sources.\n'
 'CC0-1.0: these four generated PCM assets are dedicated to the public domain.\n'
 'No human screams or field recordings were used. Choir and scream are synthetic; rain and thunder are procedural.\n'
 'These are sources, not the final presets: final sounds are processed by the VAMPIRE engine and OS effects.\n'
 'Sources: 8192 mono 16-bit samples at 11025 Hz each, DC removed, loop edges smoothed.\n')
print('Vampire bank: 64 POLY presets; original PCM: 65536 bytes')
