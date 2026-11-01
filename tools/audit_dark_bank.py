#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Compare native before/after renders, including exact preservation checks.

Usage: audit_dark_bank.py BASELINE_SOURCE [OUTPUT_DIR]
Spectral metrics describe the audio; they are not a numerical mood score.
"""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import csv,hashlib,json,sys,wave
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
KEEP={'sub','bass','reese','talkbass','perc'}

def spec(root):
    return sum([json.loads((root/'bank'/n).read_text())['presets'] for n in
        ['RED-MASS-Trap-Vampire-128.spec.json','RED-MASS-Club-Vampire-128.spec.json']],[])

def wav_path(root,n):
    return root/'build/dark-isolated/audio'/f'{n:03d}.wav'

def measure(path):
    b=path.read_bytes()
    with wave.open(str(path),'rb') as f:
        assert f.getframerate()==44100 and f.getsampwidth()==2 and f.getnchannels()==2
        raw=f.readframes(f.getnframes());x=np.frombuffer(raw,dtype='<i2').reshape(-1,2).astype(np.float64)
    held=x[int(.15*44100):int(2.95*44100)]
    nfft=4096;starts=range(0,len(held)-nfft+1,2048);window=np.hanning(nfft)[:,None]
    power=np.zeros(nfft//2+1)
    for at in starts:
        z=np.fft.rfft(held[at:at+nfft]*window,axis=0)
        power+=np.sum(abs(z)**2,axis=1)
    hz=np.fft.rfftfreq(nfft,1/44100);total=float(power.sum())
    return {'sha256':hashlib.sha256(raw).hexdigest(),'peak':int(np.max(abs(x))),
       'rms':float(np.sqrt(np.mean(x*x))), 'dc':float(x.mean()),
       'centroid_hz':float(np.sum(hz*power)/max(total,1)),
       'above_4k_fraction':float(power[hz>=4000].sum()/max(total,1)),
       'below_1k_fraction':float(power[hz<1000].sum()/max(total,1))}

def main():
    baseline=Path(sys.argv[1]).resolve();out=Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/dark-audit'
    out.mkdir(parents=True,exist_ok=True);old,new=spec(baseline),spec(ROOT)
    assert len(old)==len(new)==256
    def one(pair):
        a,b=pair;n=b['number'];keep=b['role'] in KEEP
        before,after=measure(wav_path(baseline,n)),measure(wav_path(ROOT,n))
        assert (a['number'],a['engine_id'],a['factory_index'],a['name'],a['polyphony'])==(n,b['engine_id'],b['factory_index'],b['name'],b['polyphony'])
        if keep:assert a==b and before['sha256']==after['sha256'],b['name']
        else:assert a['engine_values']!=b['engine_values'] and before['sha256']!=after['sha256'],b['name']
        return dict(number=n,name=b['name'],role=b['role'],engine=b['engine'],preserved=keep,
           before=before,after=after,centroid_ratio=after['centroid_hz']/max(before['centroid_hz'],1))
    with ThreadPoolExecutor(max_workers=4) as pool:rows=list(pool.map(one,zip(old,new)))
    changed=[r for r in rows if not r['preserved']]
    summary={'preserved_exactly':sum(r['preserved'] for r in rows),'revoiced':len(changed),
       'unique_new_pcm_hashes':len({r['after']['sha256'] for r in rows}),
       'median_centroid_ratio':float(np.median([r['centroid_ratio'] for r in changed])),
       'lower_centroid_count':sum(r['centroid_ratio']<1 for r in changed),
       'peak_max':max(r['after']['peak'] for r in rows),
       'notes':'Held 0.15..2.95 s; stereo power. Timbre/mood is subjective. Raw PCM has no post-render processing.'}
    assert summary['preserved_exactly']==40 and summary['revoiced']==216
    assert summary['unique_new_pcm_hashes']==256 and summary['peak_max']<32768
    # The kit generators and all source sample data remain unchanged.
    generators=['gen_groove_kit.py','gen_vampire_bank.py','gen_samples.py']
    for n in generators:assert (ROOT/'tools'/n).read_bytes()==(baseline/'tools'/n).read_bytes(),n
    summary['unchanged_sample_generators']=generators
    (out/'audit.json').write_text(json.dumps({'summary':summary,'presets':rows},indent=2)+'\n')
    with (out/'comparison.csv').open('w',newline='') as f:
        w=csv.writer(f);w.writerow(['number','name','role','engine','preserved_exactly','before_centroid_hz','after_centroid_hz','centroid_ratio','before_pcm_sha256','after_pcm_sha256'])
        for r in rows:w.writerow([r['number'],r['name'],r['role'],r['engine'],r['preserved'],r['before']['centroid_hz'],r['after']['centroid_hz'],r['centroid_ratio'],r['before']['sha256'],r['after']['sha256']])
    print(json.dumps(summary,indent=2))

if __name__=='__main__':main()
