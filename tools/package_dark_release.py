#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Package validated 0.5.2 firmware, native library, player and Windows launcher.

package_dark_release.py OUTDIR PREVIOUS_INSTALLER BASELINE_SOURCE
Run the build, exports, isolated comparison and release checks first.
"""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import csv,hashlib,json,shutil,subprocess,sys,wave,zipfile
import numpy as np
from red_mass_markdown import markdown

ROOT=Path(__file__).resolve().parents[1]
KEEP={'sub','bass','reese','talkbass','perc'}
def copy(a,b):b.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(a,b)
def main():
    if len(sys.argv)!=4:raise SystemExit(__doc__)
    out,previous,baseline=map(lambda p:Path(p).resolve(),sys.argv[1:]);out.mkdir(parents=True,exist_ok=True)
    build=ROOT/'build'
    specs=sum([json.loads((ROOT/'bank'/n).read_text())['presets'] for n in ['RED-MASS-Trap-Vampire-128.spec.json','RED-MASS-Club-Vampire-128.spec.json']],[])
    audit=json.loads((build/'dark-audit/audit.json').read_text())['summary']
    assert audit['preserved_exactly']==40 and audit['revoiced']==216 and audit['unique_new_pcm_hashes']==256
    with zipfile.ZipFile(previous) as z:
        prefix=z.namelist()[0].split('/')[0]+'/'
        for name in z.namelist():
            if not name.startswith(prefix) or name.endswith('/'):continue
            rel=name[len(prefix):]
            if rel=='RED-MASS-VAMPIRE-OS.exe' or rel.startswith(('docs/','audio/trap-kits/')):
                p=out/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(z.read(name))
    for folder in ['bank','preview/audio','audio','firmware','rollback','validation','docs']:(out/folder).mkdir(parents=True,exist_ok=True)
    for p in (ROOT/'bank').glob('RED-MASS-VAMPIRE-256*'):copy(p,out/'bank'/p.name)
    for n in ['RED-MASS-Trap-Vampire-128.spec.json','RED-MASS-Club-Vampire-128.spec.json']:copy(ROOT/'bank'/n,out/'bank'/n)
    fw=out/'firmware/RED-MASS-VAMPIRE-OS-v0.5.2-beta-FM1.fwsc';copy(build/'red_mass-0.5.2-beta.fwsc',fw)
    copy(previous.parent/'RED-MASS-VAMPIRE-OS-v0.5.1-alpha-FM1.fwsc',out/'rollback/RED-MASS-VAMPIRE-OS-v0.5.1-alpha-FM1.fwsc')
    copy(ROOT/'docs/DARK-BANK-0.5.2-RU.md',out/'README-RU.md')
    doc=(ROOT/'docs/DARK-BANK-0.5.2-RU.md').read_text()
    css='body{font:16px/1.65 system-ui;background:#fff9fb;color:#24151c;margin:0}main{max-width:980px;margin:auto;padding:35px 25px 80px}h1{line-height:1.15;letter-spacing:-.03em;color:#8b173d}h2{margin-top:38px}pre{overflow:auto;background:#24151c;color:#fff;padding:20px}code{font-size:.9em}a{color:#a01f4f}li{margin:7px 0}'
    (out/'README-RU.html').write_text('<!doctype html><html lang="ru"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>RED MASS 0.5.2</title><style>'+css+'</style><main>'+markdown(doc)+'</main></html>')
    copy(ROOT/'docs/DARK-BANK-0.5.2-RU.md',out/'docs/DARK-BANK-0.5.2-RU.md')
    # Preserve the existing complete usage guide and add the current sound-bank note.
    guide=out/'docs/RED-MASS-English-Guide.html'
    s=guide.read_text().replace('v0.5.1','v0.5.2').replace('v0.5.2 ALPHA','v0.5.2 BETA').replace('v0.5.2-alpha','v0.5.2-beta')
    s=s.replace('<main>','<main><aside>0.5.2 sound update: 216 revoiced presets; basses, 808s and percussion preserved. <a href="../README-RU.html">Current update guide</a>.</aside>',1)
    guide.write_text(s)
    for n in ['LICENSE','LICENSING.md']:copy(ROOT/n,out/n)
    shutil.copytree(ROOT/'LICENSES',out/'LICENSES',dirs_exist_ok=True)
    copy(ROOT/'assets/fonts/PirataOne-OFL.txt',out/'LICENSES/OFL-PirataOne.txt')
    shutil.copytree(ROOT,out/'source',ignore=shutil.ignore_patterns('build','__pycache__','*.pyc','.git'),dirs_exist_ok=True)
    shutil.copytree(ROOT.parent/'sdk-assets',out/'sdk-assets',dirs_exist_ok=True)
    def encode(p):
        n=p['number'];src=build/f'dark-isolated/audio/{n:03d}.wav';name=f"{n:03d}-{p['name'][4:]}.mp3";dest=out/'preview/audio'/name
        if not dest.exists() or dest.stat().st_mtime<src.stat().st_mtime:
            subprocess.run(['ffmpeg','-v','error','-nostdin','-y','-i',str(src),'-map_metadata','-1','-c:a','libmp3lame','-q:a','4',str(dest)],check=True)
        return dict(number=n,name=p['name'],category=p['category'],engine=p['engine'],voices=min(p['polyphony'],4),kept=p['role'] in KEEP,audio='audio/'+name)
    with ThreadPoolExecutor(max_workers=4) as pool:cards=list(pool.map(encode,specs))
    # Same-note A/B montage. Only tiny boundary fades and silent separators;
    # the individual 12-second preview files keep the unprocessed native render.
    selection=[33,41,49,57,65,74,81,89,97,121,197,249];pairs=[];timeline=[]
    abwav=out/'audio/RED-MASS-Darker-AB.wav';duration=8.;gap=.25;frames=int(duration*44100);fade=220
    with wave.open(str(abwav),'wb') as w:
        w.setnchannels(2);w.setsampwidth(2);w.setframerate(44100)
        for j,n in enumerate(selection):
            pairs.append(dict(number=n,name=specs[n-1]['name'],start=j*2*(duration+gap)))
            for which,tree in [('BEFORE 0.5.1',baseline),('AFTER 0.5.2',ROOT)]:
                with wave.open(str(tree/f'build/dark-isolated/audio/{n:03d}.wav'),'rb') as r:x=np.frombuffer(r.readframes(frames),dtype='<i2').reshape(-1,2).astype(np.float64)
                ramp=np.linspace(0,1,fade)[:,None];x[:fade]*=ramp;x[-fade:]*=ramp[::-1]
                w.writeframes(np.rint(x).astype('<i2').tobytes());w.writeframes(b'\0'*(int(gap*44100)*4))
                timeline.append([len(timeline)*(duration+gap),n,specs[n-1]['name'],which,duration])
    ab=out/'audio/RED-MASS-Darker-AB.mp3'
    subprocess.run(['ffmpeg','-v','error','-nostdin','-y','-i',str(abwav),'-c:a','libmp3lame','-b:a','160k',str(ab)],check=True);abwav.unlink()
    with (out/'audio/AB-order.csv').open('w',newline='') as f:
        w=csv.writer(f);w.writerow(['start_seconds','number','name','version','duration_seconds']);w.writerows(timeline)
    page=(ROOT/'tools/dark_preview.html').read_text().replace('/*SOUNDS*/',json.dumps(cards,separators=(',',':'))).replace('/*PAIRS*/',json.dumps(pairs,separators=(',',':')))
    (out/'preview/index.html').write_text(page)
    subprocess.run([sys.executable,str(ROOT/'web/make_site.py'),str(fw),'0.5.2-beta',str(out/'web-tools')],check=True)
    for n in ['build-0.5.2.log','build-0.5.2-beta.log','beta-version-check.log','beta-web-tests.log','tests-0.5.2.log','noise-final.log','noise-tuning.log','regression-final.log','target-budget-final.log','release-web-tests.log','release-native-tests.log','isolated-health.json','isolated-render.log']:
        p=build/n
        if p.exists():copy(p,out/'validation'/n)
    for n in ['audit.json','comparison.csv']:copy(build/'dark-audit'/n,out/'validation'/n)
    for n in ['ui_new/report.txt','ui_fm4/report.txt']:
        p=build/n
        if p.exists():copy(p,out/'validation'/('ui-'+p.parent.name+'.txt'))
    report=ROOT/'docs/TESTING-0.5.2-RU.md'
    if report.exists():copy(report,out/'validation/TESTING-RU.md')
    (out/'validation/TESTING-EN.txt').write_text('0.5.2: preset-only update; 216 replacements, 40 protected records and isolated PCM files identical.\nSee TESTING-RU.md, audit.json and the accompanying native/web/build logs.\nPhysical FM1 and Windows runtime qualification were not performed.\n')
    sums=[]
    for p in sorted(out.rglob('*')):
        if p.is_file() and p.name!='SHA256SUMS.txt':sums.append(f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.relative_to(out).as_posix()}')
    (out/'SHA256SUMS.txt').write_text('\n'.join(sums)+'\n')
    archive=Path(str(out)+'.zip')
    with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
        for p in sorted(out.rglob('*')):
            if p.is_file():z.write(p,p.relative_to(out.parent).as_posix())
    with zipfile.ZipFile(archive) as z:assert z.testzip() is None
    print(json.dumps({'archive':str(archive),'bytes':archive.stat().st_size,'firmware_sha256':hashlib.sha256(fw.read_bytes()).hexdigest(),'sounds':len(cards),'pairs':len(pairs)}))

if __name__=='__main__':main()
