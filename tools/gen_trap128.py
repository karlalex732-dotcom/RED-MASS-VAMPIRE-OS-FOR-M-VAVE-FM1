#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""128 deliberately voiced polyphonic trap sounds, ten existing native engines.
Append factory sounds without moving any old engine or preset number.
No random parameter scrambling. Every row defines a different synthesis recipe.
"""
from pathlib import Path
import json, sys
from gen_fm6_patches import op, voice, pack

root = Path(__file__).resolve().parents[1]
out = Path(sys.argv[1]); out.mkdir(parents=True, exist_ok=True)
# (native engine name, old factory count, maximum voices per part)
engines = {0:('ANALOG',12,8),2:('PHASE',6,8),3:('LOFI',5,8),
           5:('VOICE',4,4),6:('TRIO',5,8),7:('WHEEL',5,8),
           9:('PHYS',9,3),11:('NOISE',4,8),12:('FM6',8,6),14:('VAMPIRE',64,4)}
# Short names fit the hardware. Engine controls are explicit, not variants of
# one all-purpose patch. Envelopes and modulation are set by musical role below.
groups = [
 ('01 Sub and 808','sub',[
  ('CRYPT808',0,[3,0,0,0,57,0,8,70]),('BLOODSUB',0,[2,0,0,0,38,8,23,80]),
  ('SINEFANG',12,[0,0,-28,0,-10,2,0,2]),('TOMB808',6,[4,0,-12,0,0,36,6,64]),
  ('NOIRBOOM',2,[3,0,18,82,0,0,87,0]),('VELVET08',0,[3,3,32,0,83,3,44,64]),
  ('ABYSS808',6,[5,-12,0,2,0,49,18,64]),('NITEPULS',0,[4,0,0,0,32,19,17,45])]),
 ('02 Saturated bass','bass',[
  ('IRON808',0,[1,0,0,0,65,8,115,64]),('FANGCLIP',6,[6,0,-12,3,0,58,27,38]),
  ('HEXDRIVE',2,[2,3,110,94,1,1,88,0]),('BLACKBIT',3,[1,1,22,40,0,0,0,68]),
  ('RUSTSUB',12,[5,3,25,-1,-22,4,0,2]),('BLOODSAW',0,[0,2,35,0,46,60,89,100]),
  ('VILE808',6,[15,0,-12,4,0,78,10,93]),('GRAVEBIT',3,[2,4,8,12,12,0,0,83])]),
 ('03 Reese and growl','reese',[
  ('WOLFREES',6,[0,0,-12,23,0,67,30,64]),('COFFINRX',0,[0,61,71,0,84,14,58,72]),
  ('VAMPWARP',6,[11,0,7,14,0,82,50,40]),('REDGROWL',5,[86,27,44,-7,118,3,78,19]),
  ('CHROME',2,[5,7,95,54,31,1,23,0]),('RAVEBITE',6,[10,12,-12,8,1,72,66,99]),
  ('VIRUSLOW',12,[18,4,37,2,-8,2,16,2]),('PULSEHEX',0,[4,35,94,0,58,46,94,20])]),
 ('04 Talking bass','talkbass',[
  ('GHOULWOW',5,[100,19,77,-4,112,2,94,0]),('FANGTALK',5,[31,112,106,-7,99,4,71,12]),
  ('CRYBASS',14,[1,49,82,106,7,0,13,-12]),('MAWFORM',5,[65,8,29,0,126,0,110,31]),
  ('DARKVOW',5,[12,95,57,-12,65,13,67,7]),('MURMUR',14,[0,41,63,87,14,8,35,-12]),
  ('TONGUE',12,[12,6,29,4,11,5,4,3]),('ORCLOW',6,[12,7,-12,1,1,38,92,28])]),
 ('05 Ritual bells','bell',[
  ('DEATHBELL',12,[0,1,13,1,-9,2,0,1]),('GLASSHEX',2,[6,0,113,110,0,0,0,0]),
  ('IRONCHIM',9,[0,109,112,76,18,120,0,8]),('CRIMSON',14,[5,115,13,123,2,0,0,12]),
  ('BROKENTL',6,[13,7,19,5,1,105,42,64]),('GHOSTTOLL',12,[23,-1,-19,5,15,3,10,1]),
  ('FROSTBEL',9,[0,31,99,49,102,88,0,26]),('MOONRING',2,[7,7,61,72,4,1,0,0])]),
 ('06 Bone plucks','pluck',[
  ('BONEHARP',9,[1,29,118,87,27,109,0,11]),('COFFINPK',0,[0,0,0,0,31,67,14,96]),
  ('SILVERPK',12,[0,0,16,3,-31,4,0,7]),('GUTSTRING',9,[3,117,91,94,69,114,0,1]),
  ('NEEDLE',2,[4,1,94,126,0,0,0,0]),('BATPLUCK',3,[2,4,71,0,0,4,0,117]),
  ('FANGZITH',6,[14,12,7,0,1,91,15,46]),('DUSKPIN',0,[2,11,91,0,111,2,3,54])]),
 ('07 Dark keys','keys',[
  ('NOIREP',12,[0,0,-17,0,9,3,7,0]),('TOMBKEY',7,[2,0,-3,-4,3,81,12,0]),
  ('DUSTTINE',12,[31,2,21,2,-14,4,12,0]),('CHIPIVRY',3,[1,4,39,17,0,3,0,79]),
  ('CANDLE',2,[3,4,44,17,12,0,0,0]),('ASHPIANO',12,[7,-2,-5,6,-20,2,0,5]),
  ('MIDNITE',0,[4,17,73,0,102,7,16,64]),('GOTHKEY',7,[12,1,3,-2,1,24,39,1])]),
 ('08 Feral leads','lead',[
  ('RAZOR',6,[9,12,0,0,0,116,46,64]),('BATSIGNAL',0,[1,17,74,0,120,32,23,70]),
  ('VAMPACID',0,[0,0,0,0,49,112,42,94]),('REDLASER',2,[5,6,119,118,0,1,0,0]),
  ('CRYPTIC',3,[4,0,19,58,8,23,0,124]),('FANGVOX',5,[38,108,33,5,125,4,92,9]),
  ('IRONFM',12,[4,7,36,7,-19,4,2,3]),('SPECTER',6,[3,7,12,33,2,70,21,96])]),
 ('09 Vampire choirs','choir',[
  ('NOSFERA2',14,[0,94,9,39,3,11,70,0]),('LILITH2',5,[9,45,16,-2,32,34,78,37]),
  ('BLOODAAH',5,[0,0,0,0,98,0,53,0]),('BLACKOH',5,[88,84,0,-5,71,13,99,4]),
  ('VEILOOH',5,[118,39,11,4,30,55,62,19]),('CRYPTMASS',14,[0,39,83,105,92,2,4,7]),
  ('BANSHEE2',14,[1,121,23,12,11,68,127,0]),('ANGELASH',12,[32,-2,-24,1,18,0,59,4])]),
 ('10 Haunted pads','pad',[
  ('MOONVEIL',0,[4,22,61,2,63,5,9,33]),('DARKHALO',2,[0,4,32,27,19,0,14,0]),
  ('VELVET',6,[4,0,7,17,0,53,16,64]),('GHOSTLACE',14,[4,79,3,41,56,67,92,12]),
  ('NIGHTSEA',12,[0,-1,-23,0,38,1,64,4]),('BLOODGLS',2,[6,8,93,11,43,1,0,0]),
  ('STILLNESS',14,[2,33,7,98,8,17,16,0]),('SLOWFANG',0,[0,82,89,0,29,38,21,110])]),
 ('11 Cursed strings','string',[
  ('COFFINBW',9,[1,66,67,27,31,72,114,12]),('REDVIOLA',14,[6,84,17,120,27,1,5,0]),
  ('DARKCELLO',9,[3,98,83,35,59,63,99,16]),('GUTSAW',6,[0,0,12,36,0,77,13,64]),
  ('SILKHEX',2,[0,5,67,9,29,0,8,0]),('FROSTBOW',9,[1,113,45,19,94,57,83,3]),
  ('NITESTR',0,[4,13,97,3,107,6,11,82]),('FANGHARM',6,[13,12,19,13,3,88,22,81])]),
 ('12 Black organs','organ',[
  ('ALTAR',7,[12,4,2,-5,0,10,19,1]),('SACRIFCE',7,[15,-2,5,3,0,85,87,2]),
  ('VESPER2',14,[7,61,3,125,1,0,0,0]),('CHAPEL',2,[3,4,20,0,0,0,93,0]),
  ('GARGOYLE',12,[0,4,24,2,4,2,8,6]),('DUSKPIPE',7,[0,2,-4,6,4,33,3,0]),
  ('IRONMASS',7,[10,-4,1,8,6,94,43,2]),('TOMBREE D',14,[7,117,72,89,44,7,12,7])]),
 ('13 Vocal stabs','stab',[
  ('FANGCHOP',5,[27,115,110,0,123,10,109,9]),('BLOODHEY',14,[1,102,99,75,3,22,63,12]),
  ('WHISPRPK',5,[62,4,24,7,25,127,48,36]),('GHOULAH',5,[3,75,82,-4,84,7,126,23]),
  ('CRYPTCUT',14,[0,66,48,15,110,14,110,7]),('NIGHTERR',3,[0,4,95,73,41,0,0,99]),
  ('RITUAL',5,[121,44,112,-9,113,28,84,56]),('LAMENT',14,[1,18,37,122,64,1,1,-5])]),
 ('14 Pulse and motion','pulse',[
  ('BATGATE',0,[4,7,46,0,89,37,20,83]),('CRYPTSEQ',3,[2,0,87,12,0,0,3,108]),
  ('HEXSTEPS',3,[4,4,122,26,22,0,1,113]),('BLOODPMP',6,[6,0,7,19,0,82,53,20]),
  ('RAVEGATE',2,[2,2,107,81,6,1,24,0]),('GHOSTCLK',11,[2,9,101,35,127,88,0,84]),
  ('RAINROLL',14,[2,119,59,19,31,40,37,0]),('BATWINGS',14,[4,51,22,114,5,9,71,0])]),
 ('15 Tuned percussion','perc',[
  ('BONEHIT',9,[2,19,100,98,16,124,0,12]),('CHAINHIT',11,[3,2,107,20,120,45,0,0]),
  ('CRYPTWOOD',12,[19,4,49,-2,-42,7,0,5]),('FANGMETL',6,[12,19,7,0,1,114,26,64]),
  ('RIMHEX',9,[0,72,125,110,111,124,0,91]),('GHOSTHAT',11,[0,3,121,4,39,0,8,12]),
  ('TOMBTRI',2,[7,0,121,109,0,0,0,0]),('ASHCLAP',11,[1,17,98,21,72,104,29,37])]),
 ('16 Storm and nightmare','fx',[
  ('THUNDER2',14,[3,29,89,7,0,54,90,-12]),('CRYPTDUST',11,[1,112,43,95,111,37,81,0]),
  ('RAINSILK',14,[2,108,17,1,9,11,109,12]),('DREADFM',12,[15,7,53,11,36,7,19,1]),
  ('BLOODWND',11,[0,101,79,116,91,0,118,4]),('BATSWARM',6,[8,7,19,48,1,118,99,17]),
  ('GHOSTLFS',11,[2,44,79,29,99,75,33,99]),('ENDRUMBL',14,[3,76,22,100,47,2,6,-12])]),
]
# Each role has eight intentionally different shapes (ADSR values use native
# exponential time mapping). FM6 uses its own six operator envelopes.
envs={
 'sub':[(0,83,0,23),(1,106,0,37),(0,73,83,18),(2,98,0,30),(0,66,58,16),(4,114,0,51),(0,89,93,20),(1,57,30,12)],
 'bass':[(0,91,0,26),(2,66,84,22),(0,47,19,11),(3,107,0,34),(0,80,100,22),(1,76,57,18),(0,113,0,43),(2,59,68,27)],
 'reese':[(5,81,107,44),(0,121,79,27),(19,66,99,48),(0,48,69,19),(11,108,46,59),(1,76,92,36),(0,102,112,41),(8,61,72,21)],
 'talkbass':[(0,66,79,23),(1,79,41,19),(0,101,0,31),(11,57,103,40),(3,117,28,53),(0,85,76,15),(2,72,107,33),(6,43,24,9)],
 'bell':[(0,112,0,101),(1,82,0,57),(0,100,0,73),(0,126,0,116),(2,93,0,61),(0,118,0,92),(1,66,0,51),(0,104,0,84)],
 'pluck':[(0,73,0,47),(0,57,0,21),(1,96,0,67),(0,114,0,79),(2,43,0,28),(0,62,13,37),(0,84,0,52),(4,77,0,44)],
 'keys':[(2,110,23,64),(0,62,83,38),(1,87,37,82),(3,70,58,49),(6,99,14,71),(0,57,0,53),(4,74,0,62),(1,82,90,55)],
 'lead':[(0,72,112,36),(2,110,79,56),(0,59,51,17),(7,84,101,42),(1,51,91,25),(3,96,127,49),(0,112,69,28),(17,60,113,62)],
 'choir':[(93,111,116,112),(71,96,99,97),(35,72,127,78),(18,115,89,90),(106,90,111,117),(58,78,117,82),(9,79,42,62),(84,115,103,106)],
 'pad':[(101,112,111,120),(74,91,91,96),(47,124,127,105),(117,95,110,122),(96,101,89,118),(83,79,119,107),(26,96,58,90),(113,120,102,116)],
 'string':[(37,82,124,74),(74,101,109,96),(52,110,121,89),(67,94,103,77),(98,84,96,110),(22,113,118,63),(46,80,124,101),(12,77,91,52)],
 'organ':[(0,100,127,42),(0,119,127,24),(9,87,127,76),(2,71,127,38),(0,99,127,65),(19,103,113,57),(1,63,88,31),(4,118,116,93)],
 'stab':[(0,42,0,17),(1,64,0,32),(0,84,0,66),(4,55,0,31),(0,54,0,33),(0,72,0,45),(3,94,0,74),(11,47,17,39)],
 'pulse':[(0,106,112,40),(2,89,99,56),(0,78,127,28),(19,101,117,67),(4,94,111,58),(0,112,96,43),(14,82,79,75),(62,123,127,102)],
 'perc':[(0,46,0,22),(0,73,0,58),(0,65,0,35),(0,87,0,71),(0,32,0,16),(0,42,0,18),(0,104,0,82),(0,59,0,28)],
 'fx':[(1,114,0,120),(96,102,124,116),(35,117,108,110),(69,104,91,124),(118,98,121,116),(12,91,17,73),(0,81,67,67),(7,124,0,117)]}
# Deliberately different movements. No matrix modulates a discrete model selector.
# The last two sources provide expression from MOD WHEEL or AFTERTOUCH.
routes=[[(3,4,24),(2,2,13),(6,3,18),(5,1,1)],
        [(1,2,21),(3,4,16),(6,9,17),(5,3,7)],
        [(2,3,27),(3,4,31),(1,5,9),(7,2,25)],
        [(1,3,29),(3,4,19),(6,2,24),(5,1,2)],
        [(2,2,-17),(1,5,25),(3,4,34),(6,8,22)],
        [(1,2,-31),(3,3,16),(6,9,33),(5,1,1)],
        [(1,4,39),(3,2,18),(6,3,30),(7,5,12)],
        [(2,3,-24),(3,4,29),(1,5,-17),(6,2,41)]]
bank=[]; offsets={k:v[1] for k,v in engines.items()}
for category,role,rows in groups:
 for i,(short,engine,e) in enumerate(rows):
  number=len(bank)+1; name=f'{number:03d} {short.replace(" ", "")}'
  assert len(name)<=13
  envelope=envs[role][i]
  if role in ('sub','bass','talkbass'):
   effects=[(5,27,61,19,83,38,53,11)[i],(1,2,4,1,7,3,1,6)[i],(1,4,2,8,1,6,3,2)[i],(2,7,3,11,4,2,8,5)[i]]
  elif role in ('choir','pad','string','fx'):
   effects=[(2,17,4,39,1,23,8,31)[i],(57,12,84,31,100,19,64,42)[i],(28,65,11,48,7,72,35,19)[i],(106,62,81,117,93,49,73,111)[i]]
  else:
   effects=[(4,31,9,61,17,2,44,23)[i],(11,43,3,68,17,1,37,23)[i],(61,17,39,7,81,27,13,48)[i],(73,29,91,43,57,109,19,84)[i]]
  # LFO rates cover slow evolving pads, audio-ish snarls, rhythmic shapes and
  # delayed vibrato. Bass foundation avoids random pitch/stereo destabilisation.
  rate=(18,71,43,99,27,58,111,36)[i]
  wave=(0,1,2,3,4,0,3,1)[i]
  fade=(0,11,46,0,84,20,0,64)[i]
  depth=(0,8,2,17,6,11,4,23)[i]
  matrix=[list(x) for x in routes[i]]
  if role in ('sub','bass'):
   matrix=[[3,4,20+i*2],[3,2,9+i],[6,3,23],[7,2,19]]
   depth=0;rate=24+i*4
  if engine==12: # ENV is not an FM6 source: native operator EGs own amplitude.
   matrix=[([6,d,a] if s==2 else [s,d,a]) for s,d,a in matrix]
  amp=(72,13,97,43,108,56,24,88)[i] if role=='pulse' else (14 if role=='reese' and i==6 else 0)
  values=[0]*30
  ids=[5,6,7,9,10,11,12,13,14,15,16,37,38,49,50,51,52,53,54,55,56,57,58,59,60]
  vals=[(37,-14,52,9,-31,24,5,-8)[i],0,(0,12,-11,29,0,7,-23,16)[i],rate,wave,i*15,fade,
        (0,0,0,2,0,1,0,0)[i],depth,(0,5,16,0,21,3,10,0)[i],amp,0,0,
        *[v for row in matrix for v in row]]
  if role in ('sub','bass'):vals[0]=0; vals[1]=(9,2,14,5,0,3,7,1)[i]; vals[2]=0;vals[7]=0;vals[9]=0
  params=dict(zip(ids,vals))
  p=dict(number=number,name=name,category=category,role=role,engine=engines[engine][0],engine_id=engine,
         factory_index=offsets[engine],polyphony=engines[engine][2],voice_mode='POLY',engine_values=e,
         envelope=list(envelope),effects=effects,filter_env=params[5],sound_controls=params,
         recommended_notes=[36,43,48,51] if role in ('sub','bass','reese','talkbass') else [48,55,60,63])
  if engine==12:
   # Original independent six-operator patches, including individual EGs.
   family=role;rates=(99,43+i*3,31+i*2,62);levels=(99,80,0,0)
   if family in ('sub','bass'):rates=(99,42+i*4,30,77);levels=(99,83,43 if i%2 else 0,0)
   if family in ('lead','reese','talkbass','organ'):rates=(88,62,41,67);levels=(99,89,82,0)
   if family in ('pad','choir'):rates=(34+i*3,63,40,49);levels=(99,87,84,0)
   if family=='perc':rates=(99,88,71,88);levels=(99,24,0,0)
   if family=='bell':rates=(99,35+i*3,25,43);levels=(99,71,0,0)
   if family=='pluck':rates=(99,69,51,74);levels=(99,41,0,0)
   if family=='fx':rates=(51,48,37,47);levels=(99,79,13,0)
   ratios={'sub':[(0,0),(1,0),(2,0)],'bass':[(1,0),(2,0),(3,0)],
    'bell':[(1,0),(2,39),(5,71)],'pluck':[(1,0),(2,0),(4,11)],
    'perc':[(0,0),(2,73),(6,42)],'keys':[(1,0),(2,0),(4,17)],
    'pad':[(1,0),(1,1),(2,0)],'choir':[(1,0),(2,1),(3,0)],
    'organ':[(0,0),(1,0),(3,0)],'talkbass':[(1,0),(3,37),(5,79)],
    'reese':[(1,0),(1,7),(2,0)]}.get(family,[(1,0),(2,0),(3,49)])
   ops=[]
   for pair,(coarse,fine) in enumerate(ratios):
    ops.append(op(r=rates,l=levels,ol=96-pair*11,fc=coarse,ff=fine,det=7+((i+pair)%3)-1,kvs=2+pair%2))
    mf=(1,2,3,5,7,9,11,13)[(i+pair*2)%8]
    mr=(99,min(99,rates[1]+12+pair*7),min(99,rates[2]+9),min(99,rates[3]+8))
    ml=(99,65+pair*9,0 if family in ('bell','pluck','perc','keys','sub') else 63+pair*5,0)
    ops.append(op(r=mr,l=ml,ol=min(96,48+i*4+pair*9),fc=mf,ff=(i*17+pair*23)%100,kvs=4+pair))
   alg=5 if family in ('sub','bell','pluck','perc','keys','pad','choir','organ') else (3,5,7,12,18,24,27,31)[i]
   raw=voice(short.replace(' ','')[:10],alg,ops,fb=(i+2)%6,lfs=23+i*6,
             lfd=45 if family in ('pad','choir') else 0,lpmd=0,lamd=0,lpms=0)
   p['fm6']=pack(raw);p['engine_values'][0]=0;p['engine_values'][7]=8
  offsets[engine]+=1;bank.append(p)
assert len(bank)==128 and len({p['name'] for p in bank})==128
# An audit catches out-of-range values against the actual native descriptors.
descpath=root/'build/host/desc.json'
if descpath.exists():
 desc=json.loads(descpath.read_text())
 for p in bank:
  for val,d in zip(p['engine_values'],desc['ENG'][p['engine_id']]['edit']):
   assert d['min']<=val<=d['max'],(p['name'],d['label'],val)
  for k,val in p['sound_controls'].items():
   d=desc['TP'][int(k)];assert d['min']<=val<=d['max'],(p['name'],d['label'],val)
for ei,(ename,old,poly) in engines.items():
 lines=['/* RED_MASS Trap Vampire: append-only factory entries. GPL-3.0-only. */']
 for p in bank:
  if p['engine_id']!=ei:continue
  lines.append('    {"'+p['name']+'", {'+','.join(map(str,p['engine_values']))+'}, {'+
               ','.join(map(str,p['envelope']))+'}, '+str(p['filter_env'])+', 0, FX('+
               ','.join(map(str,p['effects']))+')},')
 (out/f'trap_{ename.lower()}_presets.inc').write_text('\n'.join(lines)+'\n')
ids=list(bank[0]['sound_controls'])
lines=['/* RED_MASS Trap Vampire factory map and expressive sound controls. */',
 '#define TRAP_COUNT 128u',
 'static const uint8_t TRAP_ENGINE[TRAP_COUNT] = {'+','.join(str(p['engine_id']) for p in bank)+'};',
 'static const uint8_t TRAP_FACTORY[TRAP_COUNT] = {'+','.join(str(p['factory_index']) for p in bank)+'};',
 'static const uint8_t TRAP_IDS[] = {'+','.join(map(str,ids))+'};',
 f'static const int8_t TRAP_PARAMS[TRAP_COUNT][{len(ids)}] = {{']
lines+=[' {'+','.join(str(p['sound_controls'][k]) for k in ids)+'},' for p in bank]
lines+=['};','static void trap_preset_controls(track_t *t, uint32_t pi) {',
 ' for (uint32_t n=0; n<TRAP_COUNT; n++) if (t->eng_req==TRAP_ENGINE[n] && pi==TRAP_FACTORY[n]) {',
 '  for (uint32_t k=0; k<NELEM(TRAP_IDS); k++) t->p[TRAP_IDS[k]]=TRAP_PARAMS[n][k];',
 '  return;', ' }', '}']
fm=[p for p in bank if p['engine_id']==12]
lines+=['static const uint8_t TRAP_FM6_INDEX[] = {'+','.join(str(p['factory_index']) for p in fm)+'};',
        f'static const uint8_t TRAP_FM6[{len(fm)}][128] = {{']
lines+=[' {'+','.join(map(str,p['fm6']))+'},' for p in fm]
lines+=['};','static void trap_patch_load(track_t *t, uint32_t pi) {',
 ' if(t->eng_req!=ENGI_FM6)return;',
 ' for(uint32_t k=0;k<NELEM(TRAP_FM6_INDEX);k++) if(pi==TRAP_FM6_INDEX[k]) {',
 '  uint8_t v[FP_SIZE+1u]; uint32_t tr=(uint32_t)(t-trk);',
 '  fm6_unpack(TRAP_FM6[k],v); fm6_set_patch(tr,v); fm6_adopt(tr);return;',
 ' }','}']
(out/'trap_bank.h').write_text('\n'.join(lines)+'\n')
bd=root/'bank'; bd.mkdir(exist_ok=True)
(bd/'RED-MASS-Trap-Vampire-128.spec.json').write_text(json.dumps(dict(format='red_mass-trap-vampire-bank',version=2,
 firmware='RED MASS VAMPIRE OS v0.2.0-alpha',status='native-rendered, physical FM1 not tested',
 presets=bank),indent=2)+'\n')
print('RED_MASS Trap Vampire: 128 POLY patches, 16 roles, 10 native engines; old factory indices retained')
