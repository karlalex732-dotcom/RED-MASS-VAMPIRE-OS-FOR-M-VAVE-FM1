#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Build or upload a RED_MASS 16-pad sample drum kit from WAV files in pad order.
  python3 tools/fm1_kit.py build NAME OUT_PREFIX kick.wav snare.wav ...
  python3 tools/fm1_kit.py load SLOT NAME kick.wav snare.wav ...
Pads: KD SD CL CH OH TM RM CB LT MT HT CR RD TA SH CU. 1..16 files.
Uses the existing three 80-KiB user sample slots. Load needs mido/python-rtmidi.
"""
import json
import sys
from pathlib import Path
import fm1_sample_upload as upload
import sampleio as sio

NOTES = [36,38,39,42,46,45,37,56,41,43,47,49,51,54,70,75]
PADS = ['KD','SD','CL','CH','OH','TM','RM','CB','LT','MT','HT','CR','RD','TA','SH','CU']

def build(name, paths):
    if not 1 <= len(paths) <= 16: raise ValueError('supply 1..16 WAV files in pad order')
    zones=[]
    for path,note in zip(paths,NOTES):
        rate,audio=sio.read_any_wav(Path(path))
        audio=sio.resample(audio,rate,sio.SLOT_RATE)
        if not audio: raise ValueError(f'{path}: no audio')
        zones.append((sio.to_int16(audio),note,note,note))
    return sio.user_slot(name,zones)

def main():
    if len(sys.argv)<5 or sys.argv[1] not in ('build','load'): raise SystemExit(__doc__)
    cmd=sys.argv[1]; name=sys.argv[2] if cmd=='build' else sys.argv[3]
    paths=sys.argv[4:]; hdr,data=build(name,paths)
    if cmd=='build':
        out=Path(sys.argv[3]); out.parent.mkdir(parents=True,exist_ok=True)
        Path(str(out)+'.hdr').write_bytes(hdr);Path(str(out)+'.bin').write_bytes(data)
        Path(str(out)+'.fsmp').write_bytes(hdr.ljust(512,b'\0')+data)
        Path(str(out)+'.json').write_text(json.dumps({'name':name,'pads':[{'pad':i+1,'label':PADS[i],'note':NOTES[i],'wav':str(p)} for i,p in enumerate(paths)]},indent=2)+'\n')
        print(f'{name}: {len(paths)} pads, {len(data)} B ADPCM; {out}.fsmp')
        return
    slot=int(sys.argv[2])-1
    if slot not in range(3):raise ValueError('slot must be 1, 2 or 3')
    link=upload.Link()
    if link.req(11,[slot])[1]:raise RuntimeError('sample begin failed')
    for off in range(0,len(data),256):
        a=512+off
        reply=link.req(12,[slot,a&127,(a>>7)&127,(a>>14)&127]+upload.pack7(data[off:off+256]))
        if reply[4]:raise RuntimeError(f'write at {off} failed ({reply[4]})')
    reply=link.req(13,[slot]+upload.pack7(hdr),5)
    if reply[1]:raise RuntimeError(f'sample end failed ({reply[1]})')
    print(f'USR{slot+1}: {name}, {len(paths)} pads uploaded')

if __name__=='__main__':
    try:main()
    except (OSError,ValueError,RuntimeError,TimeoutError) as exc:raise SystemExit(str(exc))
