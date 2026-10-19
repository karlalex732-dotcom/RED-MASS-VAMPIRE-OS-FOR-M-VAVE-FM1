# SPDX-License-Identifier: GPL-3.0-only
"""The kit CLI's WAV import through the native firmware, without a USB device."""
import math, os, struct, subprocess, sys, tempfile, wave
from pathlib import Path
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as folder:
    paths=[]
    for pad in range(16):
        path=Path(folder)/f'pad-{pad+1}.wav'
        with wave.open(str(path),'wb') as out:
            out.setparams((1,2,22050,0,'NONE','not compressed'))
            audio=[int(16000*math.sin(2*math.pi*(110+pad*30)*i/22050)*math.sin(math.pi*i/1764)) for i in range(1764)]
            out.writeframes(struct.pack('<'+str(len(audio))+'h',*audio))
        paths.append(str(path))
    prefix=str(Path(folder)/'sixteen')
    subprocess.run([sys.executable,str(root/'tools/fm1_kit.py'),'build','SIXTEEN',prefix,*paths],check=True)
    env=dict(os.environ,GROOVE_USERKIT=prefix+'.fsmp')
    subprocess.run([str(root/'build/host/groove_test')],env=env,check=True)
    for bad in ([],paths+[paths[0]]):
        result=subprocess.run([sys.executable,str(root/'tools/fm1_kit.py'),'build','BAD',prefix,*bad],capture_output=True)
        assert result.returncode != 0
print('kit CLI: 16 zones rendered by native firmware; invalid pad counts refused')
