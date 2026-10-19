#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Karsei
"""Two original, deterministic, mix-ready 16-pad trap kits. No sampled artist recordings."""
import math, random, sys, wave, struct, json
from pathlib import Path
from sampleio import ima_encode
NOTES=[36,38,39,42,46,45,37,56,41,43,47,49,51,54,70,75]
NAMES=['Kick','Snare','Clap','ClosedHat','OpenHat','Tom','Rim','Cowbell','808','Perc','HighTom','Crash','Ride','Tambourine','Shaker','Triangle']
KITS=['NIGHT TRAP','RAGE TRAP']
DURATIONS=[[.16,.13,.14,.042,.18,.105,.055,.09,.66,.10,.085,.25,.14,.075,.065,.09],
           [.14,.115,.13,.035,.16,.095,.05,.08,.58,.09,.075,.22,.12,.07,.055,.08]]
GAINS=[.94,.74,.65,.43,.46,.66,.61,.54,.93,.58,.59,.44,.40,.42,.38,.43]

def sound(kit,pad):
    rate=16000 if pad in (3,4,11,12,13,14,15) else 12000
    duration=DURATIONS[kit][pad];rng=random.Random(0x524d5600+kit*37+pad)
    out=[];phase=lp=0.0
    for i in range(round(rate*duration)):
        t=i/rate;n=rng.uniform(-1,1);lp+=(.17 if kit==0 else .11)*(n-lp);hp=n-lp
        if pad==0:
            f=(47 if not kit else 53)+155*math.exp(-t*90);phase+=2*math.pi*f/rate
            s=math.tanh((1.8 if not kit else 2.5)*math.sin(phase))*math.exp(-t*29)+hp*.22*math.exp(-t*260)
        elif pad==8:
            f=(55.0 if not kit else 51.913)+9*math.exp(-t*70);phase+=2*math.pi*f/rate
            core=math.sin(phase)+(.11 if not kit else .27)*math.sin(2*phase)+(.035 if not kit else .10)*math.sin(3*phase)
            s=math.tanh((1.25 if not kit else 2.1)*core)*math.exp(-t*(4.7 if not kit else 5.2))
            s+=hp*.025*math.exp(-t*230)
        elif pad==1:
            body=math.sin(2*math.pi*(188 if not kit else 210)*t)*math.exp(-t*36)
            s=.40*body+hp*.78*math.exp(-t*(43 if not kit else 52))
        elif pad==2:
            starts=(0,.007,.016) if not kit else (0,.005,.012,.020)
            bursts=sum(math.exp(-(t-start)*210) for start in starts if t>=start)
            s=hp*(.34*bursts+.31*math.exp(-t*37))
        elif pad in (3,4,11,12,13,14):
            freqs=(2573,3691,5233,6927) if not kit else (2939,4087,5741,7489)
            metal=sum(math.sin(2*math.pi*f*t) for f in freqs)*.12
            decay={3:145,4:29,11:19,12:34,13:65,14:84}[pad]*(1.1 if kit else 1)
            s=(.59*hp+metal)*math.exp(-t*decay)
            if pad in (13,14):s*=.55+.45*math.sin(2*math.pi*(177+kit*39)*t)**2
        elif pad in (5,9,10):
            f={5:135,9:330,10:210}[pad]*(1.0+kit*.21)*(1+.42*math.exp(-t*66));phase+=2*math.pi*f/rate
            s=(math.sin(phase)+.14*math.sin(phase*1.83))*math.exp(-t*42)+hp*.07*math.exp(-t*100)
        elif pad==6:
            s=(.5*math.sin(2*math.pi*(920+kit*210)*t)+.26*math.sin(2*math.pi*2340*t)+.18*hp)*math.exp(-t*110)
        elif pad==7:
            s=(math.sin(2*math.pi*(562+kit*65)*t)+.65*math.sin(2*math.pi*(847+kit*99)*t))*.54*math.exp(-t*47)
        else:
            s=(math.sin(2*math.pi*(1720+kit*340)*t)+.29*math.sin(2*math.pi*4279*t))*math.exp(-t*60)
        attack=.00035 if pad in (3,4,6,13,14,15) else .0012
        fade=min(1,t/attack)*min(1,max(0,(duration-t)/.006))
        out.append(s*fade)
    # Remove DC; preserve fades, balance kit levels, and leave mixer headroom.
    mean=sum(out)/len(out);out=[v-mean*min(1,i/16)*min(1,(len(out)-1-i)/64) for i,v in enumerate(out)]
    peak=max(abs(v) for v in out) or 1
    pcm=[round(v/peak*GAINS[pad]*30000) for v in out];pcm[0]=pcm[-1]=0
    return rate,pcm

def main():
    dest=Path(sys.argv[1]);dest.mkdir(parents=True,exist_ok=True)
    wavdir=dest.parent/'groove-kit-wav';wavdir.mkdir(exist_ok=True)
    data=bytearray();zones=[];manifest=[]
    for kit,name in enumerate(KITS):
        sub=wavdir/name.replace(' ','-');sub.mkdir(exist_ok=True)
        for pad,note in enumerate(NOTES):
            rate,pcm=sound(kit,pad);adp,_=ima_encode(pcm,0)
            zones.append((len(data),len(pcm),0,len(pcm)-1,round(rate/44100*65536),note*16,0,0,note,note,0));data+=adp
            path=sub/f'{pad+1:02}-{NAMES[pad]}.wav'
            with wave.open(str(path),'wb') as w:
                w.setparams((1,2,rate,len(pcm),'NONE','not compressed'));w.writeframes(struct.pack(f'<{len(pcm)}h',*pcm))
            manifest.append(dict(kit=name,pad=pad+1,note=note,name=NAMES[pad],rate=rate,frames=len(pcm),peak=max(abs(v) for v in pcm),rms=(sum(v*v for v in pcm)/len(pcm))**.5))
    lines=['/* SPDX-License-Identifier: GPL-3.0-only; Copyright (C) 2026 Karsei. Original trap kits. */','static const smp_zone_t GKIT_ZONES[32] = {']
    lines+=['    {'+', '.join(map(str,z))+'},' for z in zones]
    lines+=['};','static const uint8_t GKIT_DATA[] = {']
    lines+=['    '+','.join(map(str,data[i:i+32]))+',' for i in range(0,len(data),32)];lines+=['};','']
    (dest/'groove_kit.h').write_text('\n'.join(lines));(wavdir/'kits.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print(f'RED MASS: NIGHT TRAP + RAGE TRAP, 32 original pads, {len(data)} B IMA ADPCM')
if __name__=='__main__':main()
