#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Run one fresh native process per sound in old/new trees, then compare PCM."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json,os,subprocess,sys

root=Path(__file__).resolve().parents[1]
baseline=Path(sys.argv[1]).resolve()
jobs=[]
for tree in [baseline,root]:
    (tree/'build/host').mkdir(parents=True,exist_ok=True)
    # The include is resolved alongside the fixture; supply a local fixture in
    # each tree so both compilations use their corresponding factory tables.
    if tree==baseline:
        local=baseline/'tests/dark_one_test.c';local.write_bytes((root/'tests/dark_one_test.c').read_bytes())
    subprocess.run(['cc','-O2','-w','-Ibuild/gen','-Ifirmware/src','-o','build/host/dark_one_test','tests/dark_one_test.c','-lm'],cwd=tree,check=True)
    for n in range(1,257):
        d=tree/'build/dark-isolated/audio';d.mkdir(parents=True,exist_ok=True)
        jobs.append((tree,n,d/f'{n:03d}.wav'))
def one(job):
    tree,n,path=job
    r=subprocess.run([str(tree/'build/host/dark_one_test'),str(n),str(path)],cwd=tree,capture_output=True,text=True)
    if r.returncode:raise RuntimeError(f'{tree.name} #{n}: {r.stdout} {r.stderr}')
    d=json.loads(r.stdout.strip().splitlines()[-1]);assert d['pass'];return tree,d
results={str(baseline):[],str(root):[]}
with ThreadPoolExecutor(max_workers=4) as pool:
    for tree,d in pool.map(one,jobs):results[str(tree)].append(d)
for tree in [baseline,root]:
    rows=results[str(tree)];assert len(rows)==256
    (tree/'build/isolated-health.json').write_text(json.dumps(rows,indent=2)+'\n')
    print(tree.name,': 256 isolated native renders PASS, max peak',max(r['peak'] for r in rows),flush=True)
env=dict(os.environ,OPENBLAS_NUM_THREADS='1',OMP_NUM_THREADS='1')
subprocess.run([sys.executable,str(root/'tools/audit_dark_bank.py'),str(baseline)],cwd=root,env=env,check=True)
