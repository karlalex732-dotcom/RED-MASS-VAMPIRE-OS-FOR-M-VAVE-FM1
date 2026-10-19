#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Describe the actual processed PCM, without changing the audio.
Byte uniqueness is not a perceptual diversity measure. This adds spectrum,
envelope, stereo width and attack/decay features; never claims listener proof.
"""
from pathlib import Path
import csv, json, wave
import numpy as np
root=Path(__file__).resolve().parents[1]
src=root/'build/trap128-audio'
spec=json.loads((root/'bank/RED-MASS-Trap-Vampire-128.spec.json').read_text())['presets']
rows=[];vectors=[];spectra=[]
edges=np.geomspace(24,21000,33)
for p in spec:
 with wave.open(str(src/f"{p['number']:03d}.wav"),'rb') as w:
  sr=w.getframerate(); x=np.frombuffer(w.readframes(w.getnframes()),dtype='<i2').astype(float).reshape(-1,2)/32768
 m=x.mean(axis=1); n=4096; win=np.hanning(n)
 frames=np.lib.stride_tricks.sliding_window_view(m[:sr*3],n)[::1024]
 power=(np.abs(np.fft.rfft(frames*win,axis=1))**2).mean(axis=0)
 freq=np.fft.rfftfreq(n,1/sr)
 bands=np.array([power[(freq>=lo)&(freq<hi)].sum() for lo,hi in zip(edges,edges[1:])])
 bands/=max(bands.sum(),1e-20)
 rms=np.sqrt((m[:(len(m)//1024)*1024].reshape(-1,1024)**2).mean(axis=1))
 attack=int(np.argmax(rms[:int(sr*3/1024)]))*1024/sr
 ev=np.array([np.sqrt(np.mean(m[int(lo*sr):int(hi*sr)]**2)) for lo,hi in zip(np.linspace(0,12,25)[:-1],np.linspace(0,12,25)[1:])])
 ev/=max(ev.max(),1e-12)
 side=(x[:,0]-x[:,1])/2
 width=float(np.sqrt(np.mean(side**2))/max(np.sqrt(np.mean(m**2)),1e-10))
 centroid=float((freq*power).sum()/max(power.sum(),1e-20))
 rolloff=float(freq[np.searchsorted(np.cumsum(power)/power.sum(),.95)])
 flat=float(np.exp(np.mean(np.log(power[1:]+1e-16)))/(np.mean(power[1:])+1e-16))
 peak=float(np.abs(x).max())
 row=dict(number=p['number'],name=p['name'],category=p['category'],engine=p['engine'],
   peak_dbfs=round(20*np.log10(max(peak,1e-12)),2),rms_dbfs=round(20*np.log10(max(np.sqrt(np.mean(x**2)),1e-12)),2),
   centroid_hz=round(centroid,2),rolloff95_hz=round(rolloff,2),spectral_flatness=round(flat,6),
   peak_envelope_time_s=round(attack,3),stereo_side_mid=round(width,4))
 rows.append(row);spectra.append(bands)
 vectors.append(np.r_[np.sqrt(bands),ev*.28,np.tanh(width)*.25])
v=np.array(vectors);v/=np.linalg.norm(v,axis=1)[:,None]
similar=v@v.T;np.fill_diagonal(similar,-1)
pairs=[]
for a,b in sorted(zip(*np.triu_indices(128,1)),key=lambda ab:similar[ab],reverse=True)[:12]:
 pairs.append(dict(a=rows[a]['name'],b=rows[b]['name'],profile_cosine=round(float(similar[a,b]),6)))
report=dict(count=128,engines=sorted({p['engine'] for p in spec}),categories=16,
 audio_path='12-second native processed chord renders; bass register one octave lower',
 method='32 log frequency power bands + 24 relative envelope bins + side/mid width; weighted cosine',
 limitation='Descriptive signal comparison, not a guarantee of perceived uniqueness. Four bass roles use a lower register; compare within roles as well.',
 centroid_range_hz=[min(p['centroid_hz'] for p in rows),max(p['centroid_hz'] for p in rows)],
 attack_peak_range_s=[min(p['peak_envelope_time_s'] for p in rows),max(p['peak_envelope_time_s'] for p in rows)],
 closest_profiles=pairs,rows=rows)
(src/'diversity-audit.json').write_text(json.dumps(report,indent=2)+'\n')
with (src/'sound-features.csv').open('w',newline='') as f:
 w=csv.DictWriter(f,fieldnames=list(rows[0]));w.writeheader();w.writerows(rows)
print(json.dumps({k:report[k] for k in ['count','engines','centroid_range_hz','attack_peak_range_s','closest_profiles']},indent=2))
