#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""RED_MASS 129..256: 40 plucks, 40 pads, 40 leads, eight sampled-piano voicings.
Ten deliberately chosen synthesis recipes per synth role, each with four
musically voiced articulations. Existing 001..128 and factory indices stay.
"""
from pathlib import Path
import json,sys
from gen_fm6_patches import op,voice,pack
root=Path(__file__).resolve().parents[1]
out=Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
old=json.loads((root/'bank/RED-MASS-Trap-Vampire-128.spec.json').read_text())['presets']
eng={0:('ANALOG',12,8),2:('PHASE',6,8),3:('LOFI',5,8),4:('SAMPLE',5,8),5:('VOICE',4,4),6:('TRIO',5,8),7:('WHEEL',5,8),9:('PHYS',9,3),11:('NOISE',4,8),12:('FM6',8,6),14:('VAMPIRE',64,4)}
off={k:v[1] for k,v in eng.items()}
for p in old:off[p['engine_id']]=max(off[p['engine_id']],p['factory_index']+1)
recipes={
'pluck':[(9,[1,24,102,81,26,89,0,9]),(9,[0,95,96,63,35,109,0,7]),(9,[3,68,88,84,46,96,0,8]),
(0,[0,4,41,0,39,51,19,88]),(2,[4,0,89,103,0,0,0,0]),(2,[6,7,106,92,7,1,0,0]),
(12,[0,0,-17,0,-16,2,0,8]),(12,[0,1,9,0,-25,3,0,8]),(6,[13,7,19,8,1,86,25,68]),(14,[5,92,23,87,4,0,31,0])],
'pad':[(0,[0,39,69,0,53,15,9,56]),(0,[4,21,57,3,69,11,5,81]),(2,[0,4,52,27,16,0,14,0]),
(2,[6,8,94,8,27,1,0,0]),(6,[4,0,7,22,0,66,9,64]),(6,[13,12,19,17,3,81,14,71]),
(5,[116,82,9,0,58,22,59,11]),(14,[0,85,11,61,21,8,69,0]),(14,[6,73,13,104,24,2,71,0]),(12,[0,-1,-25,0,29,0,47,8])],
'lead':[(0,[1,13,70,0,107,26,21,91]),(0,[0,0,0,0,58,97,29,113]),(2,[5,6,108,99,3,1,0,0]),
(2,[2,1,95,54,11,0,31,0]),(6,[9,0,12,7,0,108,23,64]),(6,[3,7,12,23,2,76,17,88]),
(3,[1,4,61,25,0,6,0,99]),(5,[39,112,61,0,112,3,87,3]),(14,[1,109,42,80,6,4,39,0]),(12,[0,1,19,0,-8,3,4,8])],
}
names={
'pluck':'BLOODWIRE NIGHTWIRE ASHWIRE FANGWIRE IRONTOLL REDTOLL CRYPTTOLL DUSKTOLL BONEHARP HEXHARP SILKHARP GRAVEHARP BATSTAB COFFSTAB VEINSTAB NITESTAB GHOSTPIN THORNPIN SILVERPIN RAVEPIN RUBYGLAS BLACKGLAS FROSTGLAS GRAVEGLAS HEXCRYST NITECRYST FANGCRYST ASHCRYST TOMBWOOD DARKWOOD BONESNAP CRYPTSNAP MOONZITH REDZITH NIGHTZITH FANGZITH BATMALLET HEXMALLET ASHMALLET IRONMAL'.split(),
'pad':'DARKSWELL NIGHTWASH BLOODSEA ABYSSFLOW MOONVELV REDVELV ASHVELV BLACKVELV VEILDRIFT HEXDRIFT BATDRIFT DUSKDRIFT FROSTAURA GLASSAURA REDAURA DEADAURA COFFINSEA WOLFHALO RAVEHALO IRONHALO NOIRSHIM ASHSHIM GHOSTSHIM VEINSHIM CRYPTOOH HUSHOOH LOSTOOH SHADOWOOH NIGHTMASS LILITHSEA CHOIRVEIL BATCATHED BLOODBOW SILKBOW ASHBOW DEADVIOLA ECLIPSE VOIDSHIM NIGHTICE CRIMHALO'.split(),
'lead':'RAZORTECH BATLASER FANGRAVE REDSIGNAL HEXACID CRYPTACID BLOODACID VILEACID IRONPHAS FROSTPHAS DARKPHAS NITEPHAS GRAVEPLS RAVEPULSE FANGPULSE REDPULSE WOLFSYNC BLOODSYNC CRYPTSYNC BATSYNC NOIRDUAL SILVERJET NIGHTJET FANGJET CHROME8 CRYPTBYTE BLOODBIT RAVEBIT FANGCHANT HEXCHANT ASHCHANT WOLFCHANT BANSHEE3 REDHOWL CRYPTHOWL BATSCREAM IRONFANG DARKFM VEINFM CRIMFM'.split()
}
ids=[5,6,7,9,10,11,12,13,14,15,16,37,38,49,50,51,52,53,54,55,56,57,58,59,60]
limits={0:[(0,4)]+[(0,127)]*7,2:[(0,7),(0,8),(0,127),(0,127),(0,127),(0,1),(0,127),(0,0)],3:[(0,4),(0,4)]+[(0,127)]*4+[(0,3),(0,127)],5:[(0,127)]*3+[(-12,12)]+[(0,127)]*4,6:[(0,15),(-24,24),(-24,24),(0,50),(0,3)]+[(0,127)]*3,9:[(0,3)]+[(0,127)]*7,12:[(0,32),(-7,7),(-64,63),(-16,16),(-64,63),(-7,7),(0,127),(0,8)],14:[(0,7)]+[(0,127)]*6+[(-12,12)]}
bank=[]
for role in ('pluck','pad','lead'):
 assert len(names[role])==40
 for j,(ei,base) in enumerate(recipes[role]):
  for v in range(4):
   n=len(bank)+129;short=names[role][j*4+v];e=base.copy()
   # DARK / SHARP / WIDE / SOFT articulations: continuous engine controls only.
   edits={0:[(4,[-8,13,3,-3][v]),(1,[0,0,11,5][v]),(6,[0,9,3,-4][v])],
    2:[(2,[-12,7,14,-5][v]),(3,[0,11,-7,5][v]),(4,[0,3,9,1][v])],
    3:[(2,[-8,11,24,-17][v]),(3,[0,7,3,-9][v])],
    5:[(0,[-7,7,15,-3][v]),(2,[0,7,15,3][v]),(6,[-5,7,1,-9][v])],
    6:[(5,[-9,12,4,-2][v]),(3,[0,3,7,1][v]),(6,[0,7,2,-5][v])],
    9:[(1,[-8,11,19,-2][v]),(2,[-9,9,17,-3][v]),(3,[-8,2,7,-2][v]),(4,[0,5,13,23][v])],
    12:[(2,[-6,7,2,-2][v]),(4,[0,-5,7,13][v])],
    14:[(1,[-11,13,3,-5][v]),(2,[-4,7,13,-2][v]),(3,[0,5,-11,9][v]),(6,[-7,9,15,-1][v])]}[ei]
   for k,d in edits:e[k]=min(limits[ei][k][1],max(limits[ei][k][0],e[k]+d))
   if role=='pluck':
    env=[v//3, [67,55,88,76][v]+j%3*3, [0,0,8,0][v], [37,24,59,48][v]]
    fx=[[3,21,8,0][v],[7,2,34,13][v],[27,15,44,8][v],[51,22,73,39][v]]
    filt=[43,57,21,32][v];rate=29+j*4;depth=0;fade=0;amp=0
   elif role=='pad':
    env=[[87,65,103,75][v]+j%2*3, [96,106,84,113][v], [107,121,96,114][v], [107,97,117,102][v]]
    fx=[[2,8,5,1][v],[49,28,73,37][v],[19,7,31,11][v],[84,57,95,68][v]]
    filt=[-9,11,19,7][v];rate=12+j*3+v*5;depth=3 if j==4 else 0;fade=73;amp=0
   else:
    env=[[0,1,7,3][v], [79,67,99,88][v], [103,89,119,97][v], [35,27,51,43][v]]
    fx=[[17,29,9,21][v],[11,3,24,7][v],[37,19,49,27][v],[29,13,47,21][v]]
    filt=[23,41,11,32][v];rate=53+j*4;depth=[2,0,3,1][v];fade=39;amp=0
   matrix=[[3,4,18+v*3],[1,2,5+v*2 if role=='pad' else 3],[6,2,24+v*5],[7,9,19+v*6]]
   vals=[filt,0,[0,9,13,-5][v],rate,0,(j*9+v*19)%128,fade,depth,[0,5,7,3][v],0,amp,0,0,*[a for row in matrix for a in row]]
   p=dict(number=n,name=f'{n:03d} {short}',role=role,category={'pluck':'17 Club plucks','pad':'18 Vampire pads','lead':'19 Feral club leads'}[role],engine=eng[ei][0],engine_id=ei,factory_index=off[ei],polyphony=eng[ei][2],voice_mode='POLY',engine_values=e,envelope=env,effects=fx,filter_env=filt,sound_controls=dict(zip(ids,vals)),recommended_notes=[48,55,60,63],recipe=j+1,articulation=('Dark','Sharp','Wide','Soft')[v])
   if ei==12:
    rates={'pluck':(99,61+v*5,42,61),'pad':(41+v*3,66,43,47),'lead':(95,63,43,73)}[role]
    levels={'pluck':(99,48+v*7,0,0),'pad':(99,91,87,0),'lead':(99,89,79,0)}[role]
    ops=[]
    for pair in range(3):
     c=(1,1 if role=='pad' else 2,2 if role=='pad' else 3)[pair]
     ops.append(op(r=rates,l=levels,ol=95-pair*9,fc=c,ff=pair if role=='pad' else 0,det=6+(v+pair)%3,kvs=2))
     mr=(99 if role!='pad' else 37+pair*5,min(99,rates[1]+7+pair*5),rates[2],rates[3])
     ml=(99,71+pair*5,0 if role=='pluck' else 71+v*3,0)
     fc=(2,3,7)[pair] if j==6 else (1,4,9)[pair] if role=='pluck' else (1,2,3)[pair] if role=='pad' else (3,5,7)[pair]
     ops.append(op(r=mr,l=ml,ol=55+v*7+pair*5,fc=fc,ff=(13+v*7)*pair if role=='pluck' else 0,kvs=5))
    p['fm6']=pack(voice(short[:10],5,ops,fb=v%3,lfs=23,lfd=61 if role=='pad' else 31,lpmd=0,lamd=0,lpms=0))
   off[ei]+=1;bank.append(p)
# Eight timbres of the existing CC0 Salamander piano multisamples. Original
# recorded attacks stay clean; tone, sustain, saturation and space set character.
pianos=[('DREADGRND',117,3,[0,108,31,71],[3,7,13,51]),('BLACKIVRY',88,7,[0,99,21,55],[7,4,8,39]),
 ('IRONHAMMR',126,15,[0,88,7,39],[17,2,7,24]),('CRYPTROOM',103,0,[1,119,43,89],[0,9,11,91]),
 ('BLOODGLAS',121,4,[0,114,37,79],[5,17,39,67]),('VELVETKEY',73,1,[3,121,39,85],[2,13,17,61]),
 ('FANGSTACC',112,11,[0,64,0,21],[11,3,19,27]),('GRAVECONC',109,2,[0,126,49,101],[1,7,23,79])]
for short,cut,drive,env,fx in pianos:
 n=len(bank)+129;e=[0,0,0,1,cut,0,drive,0];v=n-249
 vals=[0,0,0,31+v*3,0,0,0,0,0,0,0,0,0,*[a for row in [[3,4,22],[6,9,17],[7,2,13],[0,0,0]] for a in row]]
 bank.append(dict(number=n,name=f'{n:03d} {short}',role='piano',category='20 Sinister grand pianos',engine='SAMPLE',engine_id=4,factory_index=off[4],polyphony=8,voice_mode='POLY',engine_values=e,envelope=env,effects=fx,filter_env=0,sound_controls=dict(zip(ids,vals)),recommended_notes=[48,55,60,63],sample_source='Salamander piano, existing CC0 multisamples',articulation=short));off[4]+=1
assert len(bank)==128 and all(len(p['name'])<=13 for p in bank)
dp=root/'build/host/desc.json'
if dp.exists():
 d=json.loads(dp.read_text())
 for p in bank:
  for k,(val,pr) in enumerate(zip(p['engine_values'],d['ENG'][p['engine_id']]['edit'])):assert pr['min']<=val<=pr['max'],(p['name'],k,val)
  for k,val in p['sound_controls'].items():assert d['TP'][int(k)]['min']<=val<=d['TP'][int(k)]['max'],(p['name'],k,val)
for ei,(name,_,_) in eng.items():
 lines=['/* RED_MASS Club 128: append-only. GPL-3.0-only. */']
 for p in bank:
  if p['engine_id']==ei:lines.append('    {"'+p['name']+'", {'+','.join(map(str,p['engine_values']))+'}, {'+','.join(map(str,p['envelope']))+'}, '+str(p['filter_env'])+', 0, FX('+','.join(map(str,p['effects']))+')},')
 (out/f'club_{name.lower()}_presets.inc').write_text('\n'.join(lines)+'\n')
lines=['/* RED_MASS Club128: native controls. GPL-3.0-only. */','#define CLUB_COUNT 128u',
 'static const uint8_t CLUB_ENGINE[CLUB_COUNT] = {'+','.join(str(p['engine_id']) for p in bank)+'};',
 'static const uint8_t CLUB_FACTORY[CLUB_COUNT] = {'+','.join(str(p['factory_index']) for p in bank)+'};',
 'static const int8_t CLUB_PARAMS[CLUB_COUNT][25] = {']
lines+=[' {'+','.join(str(p['sound_controls'][k]) for k in ids)+'},' for p in bank]
lines+=['};','static void club_preset_controls(track_t *t,uint32_t pi) {',' for(uint32_t n=0;n<CLUB_COUNT;n++)if(t->eng_req==CLUB_ENGINE[n] && pi==CLUB_FACTORY[n]) {','  for(uint32_t k=0;k<NELEM(TRAP_IDS);k++)t->p[TRAP_IDS[k]]=CLUB_PARAMS[n][k];return;',' }','}']
fm=[p for p in bank if p['engine_id']==12]
lines+=['static const uint8_t CLUB_FM6_INDEX[] = {'+','.join(str(p['factory_index']) for p in fm)+'};',f'static const uint8_t CLUB_FM6[{len(fm)}][128] = {{']
lines+=[' {'+','.join(map(str,p['fm6']))+'},' for p in fm]
lines+=['};','static void club_patch_load(track_t *t,uint32_t pi) {',' if(t->eng_req!=ENGI_FM6)return;',' for(uint32_t k=0;k<NELEM(CLUB_FM6_INDEX);k++)if(pi==CLUB_FM6_INDEX[k]){','  uint8_t v[FP_SIZE+1u];uint32_t tr=(uint32_t)(t-trk);fm6_unpack(CLUB_FM6[k],v);fm6_set_patch(tr,v);fm6_adopt(tr);return;',' }','}']
(out/'club_bank.h').write_text('\n'.join(lines)+'\n')
cat={'sub':8,'bass':8,'reese':8,'talkbass':8,'pluck':4,'pad':5,'lead':6,'piano':7,'keys':9,'bell':10,'choir':11,'string':12,'organ':13,'perc':14,'fx':15,'pulse':5,'stab':4}
tags=[[0]*128 for _ in range(16)]
for p in old+bank:
 e,i=p['engine_id'],p['factory_index'];assert i<128
 assert tags[e][i]==0;tags[e][i]=(16 if p['number']<=128 else 32)|cat[p['role']]
(out/'vampire256_browser.h').write_text('/* Curated 256 sound map. GPL-3.0-only. */\nstatic const uint8_t BANK_CATEGORY[16][128] = {\n'+'\n'.join(' {'+','.join(map(str,t))+'},' for t in tags)+'\n};\n')
(root/'bank/RED-MASS-Club-Vampire-128.spec.json').write_text(json.dumps(dict(format='red_mass-club-vampire-bank',version=1,firmware='RED MASS VAMPIRE OS v0.4.0-alpha',presets=bank),indent=2)+'\n')
print('Club128: 40 plucks + 40 pads + 40 leads + 8 sampled pianos; 256 curated total')
