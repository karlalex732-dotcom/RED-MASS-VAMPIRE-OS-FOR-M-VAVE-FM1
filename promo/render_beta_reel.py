#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Render the English beta launch reel from the firmware's own DSP and UI.

python3 promo/render_beta_reel.py OUTPUT_DIRECTORY
Requires a generated build/gen, cc, ffmpeg, numpy and Pillow.
"""
from pathlib import Path
from functools import lru_cache
import json, math, subprocess, sys, wave
import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / 'build/beta-promo'
W, H, FPS = 1080, 1920, 30
INTRO, SEGMENT, END = 2.6, 4.0, 6.0
PICKS = [(249, 'DREADGRND', 'SINISTER GRAND'), (41, 'BONEHARP', 'BONE PLUCK'),
         (65, 'NOSFERA2', 'VAMPIRE CHOIR'), (33, 'DEATHBELL', 'RITUAL BELL'),
         (197, 'NIGHTMASS', 'HAUNTED PAD'), (225, 'WOLFSYNC', 'FERAL LEAD')]
TOTAL = INTRO + SEGMENT * len(PICKS) + END
ACCENT = (255, 34, 80)
WHITE = (246, 239, 237)
PINK = (246, 160, 179)
MUTED = (150, 135, 142)

@lru_cache(maxsize=96)
def font(size, gothic=False, weight=750):
    path = ROOT / 'assets/fonts' / ('PirataOne-Regular.ttf' if gothic else 'InterTight[wght].ttf')
    f = ImageFont.truetype(str(path), size)
    if not gothic:
        f.set_variation_by_axes([weight])
    return f

def text(draw, value, xy, size, color=WHITE, gothic=False, center=False, weight=750, max_width=900):
    f = font(size, gothic, weight)
    while draw.textbbox((0,0), value, font=f)[2] > max_width:
        size -= 1
        f = font(size, gothic, weight)
    if center:
        xy = (xy[0] - draw.textlength(value, font=f)/2, xy[1])
    draw.text(xy, value, font=f, fill=color, stroke_width=0)

def read_wav(path):
    with wave.open(str(path)) as w:
        assert (w.getnchannels(), w.getsampwidth(), w.getframerate()) == (2,2,44100)
        return np.frombuffer(w.readframes(w.getnframes()), dtype='<i2').reshape(-1,2).astype(np.float32)/32768

def fade(x, start=.018, end=.065):
    x=x.copy(); a=min(len(x), int(start*44100)); b=min(len(x), int(end*44100))
    x[:a]*=np.linspace(0,1,a)[:,None]; x[-b:]*=np.linspace(1,0,b)[:,None]
    return x

def make_audio(out):
    n=int(round(TOTAL*44100)); result=np.zeros((n,2),np.float32)
    storm=read_wav(ROOT/'build/dark-isolated/audio/121.wav')
    intro=fade(storm[:int(INTRO*44100)]*.6, .12, .08)
    result[:len(intro)]+=intro
    manifest=[]
    for i,(number,name,category) in enumerate(PICKS):
        x=read_wav(BUILD/f'preset-{number}.wav')[:int(SEGMENT*44100)]
        peak=max(.001,float(np.max(np.abs(x))))
        gain=min(2.8,.62/peak)
        x=fade(x*gain)
        start=int(round((INTRO+i*SEGMENT)*44100)); result[start:start+len(x)]+=x
        manifest.append(dict(start=INTRO+i*SEGMENT,end=INTRO+(i+1)*SEGMENT,preset=number,name=name,category=category))
    tail=read_wav(ROOT/'build/dark-isolated/audio/033.wav')[int(2*44100):int(8*44100)]
    start=int(round((INTRO+len(PICKS)*SEGMENT)*44100)); tail=fade(tail*.95,.08,1.4)
    result[start:start+len(tail)]+=tail
    assert np.max(np.abs(result))<1
    wav=out/'reel-edit.wav'
    with wave.open(str(wav),'wb') as w:
        w.setnchannels(2); w.setsampwidth(2); w.setframerate(44100)
        w.writeframes(np.rint(result*32767).astype('<i2').tobytes())
    subprocess.run(['ffmpeg','-v','error','-y','-i',str(wav),'-af','loudnorm=I=-15:TP=-1.2:LRA=9','-ar','48000','-c:a','pcm_s16le',str(out/'reel-audio.wav')],check=True)
    wav.unlink()
    (out/'SOUND-CREDITS.json').write_text(json.dumps(dict(firmware='RED MASS VAMPIRE OS v0.5.2-beta',source='Native firmware DSP and native UI capture; not a physical-device recording.',editing='Cuts, short fades, segment level matching and final loudness normalization. No external music, added synthesis, EQ or purchased stock audio.',presets=manifest),indent=2)+'\n')
    return result.mean(axis=1)

def main():
    out=Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    # Existing validated stems may be reused; each native capture preserves factory parameters.
    if any(not (BUILD/f'preset-{n}.wav').exists() for n,_,_ in PICKS):
        subprocess.run(['cc','-O2','-w','-Ibuild/gen','-Ifirmware/src','-o',str(BUILD/'capture'),'promo/capture.c','-lm'],cwd=ROOT,check=True)
        for n,_,_ in PICKS:
            subprocess.run([str(BUILD/'capture'),str(n),str(BUILD/f'preset-{n}.wav'),str(BUILD)],cwd=ROOT,check=True)
    for n,_,_ in PICKS:
        Image.open(BUILD/f'preset-{n:03d}.ppm').save(BUILD/f'preset-{n:03d}.png')
    audio=make_audio(out)
    # Static grain and a restrained burgundy light field; motion is driven by actual audio.
    yy,xx=np.mgrid[:H,:W];rng=np.random.default_rng(907)
    glow=np.exp(-(((xx-W*.55)/(W*.72))**2+((yy-H*.51)/(H*.5))**2)*2.4)
    a=np.empty((H,W,3),np.float32);noise=rng.normal(0,.65,(H,W))
    for c,(base,amp) in enumerate([(10,31),(8,4),(13,11)]):a[:,:,c]=base+amp*glow+noise
    bg=Image.fromarray(np.clip(a,0,255).astype('uint8'),'RGB')
    thumbs={n:Image.open(BUILD/f'preset-{n:03d}.png').convert('RGB').resize((600,600),Image.Resampling.NEAREST) for n,_,_ in PICKS}
    ring=Image.new('RGBA',(W,H),(0,0,0,0));rd=ImageDraw.Draw(ring)
    for diameter in [710,850,1040,1330]:
        box=(W/2-diameter/2,H*.51-diameter/2,W/2+diameter/2,H*.51+diameter/2)
        rd.ellipse(box,outline=(132,34,58,45),width=1)
    bg=Image.alpha_composite(bg.convert('RGBA'),ring).convert('RGB')
    frames=int(round(TOTAL*FPS)); video=out/'RED-MASS-v0.5.2-beta-Reel-EN.mp4'
    cmd=['ffmpeg','-v','error','-y','-f','rawvideo','-vcodec','rawvideo','-pix_fmt','rgb24','-s',f'{W}x{H}','-r',str(FPS),'-i','-','-i',str(out/'reel-audio.wav'),'-map','0:v:0','-map','1:a:0','-c:v','libx264','-preset','fast','-crf','20','-pix_fmt','yuv420p','-c:a','aac','-b:a','192k','-movflags','+faststart','-t',str(TOTAL),str(video)]
    encoder=subprocess.Popen(cmd,stdin=subprocess.PIPE)
    for frame in range(frames):
        t=frame/FPS;im=bg.copy();d=ImageDraw.Draw(im)
        sample=int(t*44100);chunk=audio[sample:sample+2048]
        if len(chunk)<2048:chunk=np.pad(chunk,(0,2048-len(chunk)))
        rms=float(np.sqrt(np.mean(chunk*chunk)));pulse=min(1,rms*9)
        # Abstract vampire crest, drawn as vectors.
        d.line([(90,122),(118,150),(146,122)],fill=ACCENT,width=4)
        d.line([(118,150),(118,171)],fill=ACCENT,width=4)
        text(d,'KARSei'.upper(),(165,117),31,weight=650)
        text(d,'M-VAVE FM1',(990,121),28,MUTED,center=False,max_width=200)
        # Right-aligned compact hardware identifier.
        d.rectangle((847,110,1020,165),fill=(12,9,14))
        text(d,'M-VAVE FM1',(1015,120),25,MUTED,center=False,max_width=190)
        # Repaint with precise right alignment.
        f=font(25,False,650);s='M-VAVE FM1';d.rectangle((840,110,1080,166),fill=(12,9,14));d.text((990-d.textlength(s,font=f),120),s,font=f,fill=MUTED)
        d.line((90,201,990,201),fill=(95,38,55),width=1)
        if t<INTRO:
            text(d,'RED',(W/2,365),267,gothic=True,center=True)
            text(d,'MASS',(W/2,630),290,gothic=True,center=True)
            d.rounded_rectangle((220,998,860,1094),radius=5,fill=ACCENT)
            text(d,'v0.5.2  /  BETA',(W/2,1008),58,(12,9,14),center=True,weight=850)
            text(d,'THE BETA',(W/2,1210),80,center=True)
            text(d,'HAS AWAKENED',(W/2,1310),80,center=True)
            text(d,'SOUND ON. STEP INSIDE.',(W/2,1528),35,PINK,center=True,weight=650)
        elif t<INTRO+len(PICKS)*SEGMENT:
            i=min(5,int((t-INTRO)/SEGMENT));local=t-INTRO-i*SEGMENT;n,name,category=PICKS[i]
            text(d,'Red Mass',(90,260),118,gothic=True)
            d.rounded_rectangle((756,291,990,352),radius=3,outline=ACCENT,width=2)
            text(d,'0.5.2 BETA',(873,300),31,PINK,center=True,weight=750)
            text(d,category,(90,450),38,PINK,weight=650)
            text(d,name,(90,507),113,weight=850)
            d.line((90,650,990,650),fill=(108,42,60),width=1)
            text(d,f'{i+1:02d} / 06',(90,687),34,MUTED,weight=650)
            text(d,f'PRESET {n:03d}',(760,687),34,MUTED,weight=650,max_width=230)
            # This is the actual 240px firmware screen; no invented device UI.
            border=3+int(3*pulse);d.rectangle((235,773,845,1383),fill=(22,12,18),outline=ACCENT,width=border)
            im.paste(thumbs[n],(240,778))
            d=ImageDraw.Draw(im)
            text(d,'NATIVE FM1 DSP',(W/2,1660),28,MUTED,center=True,weight=650)
            for j in range(6):
                x=90+j*152;fill=ACCENT if j<i else (64,34,45)
                d.rectangle((x,1764,x+137,1768),fill=fill)
                if j==i:d.rectangle((x,1764,x+int(137*local/SEGMENT),1768),fill=WHITE)
        else:
            text(d,'RED MASS',(W/2,302),186,gothic=True,center=True)
            text(d,'VAMPIRE OS',(W/2,536),42,PINK,center=True,weight=650)
            d.rounded_rectangle((90,667,990,891),radius=5,fill=ACCENT)
            text(d,'BETA IS OUT',(W/2,694),130,(12,9,14),center=True,weight=900)
            text(d,'216 DARKER VOICES',(W/2,1006),78,center=True,weight=850)
            text(d,'256 PRESETS IN TOTAL',(W/2,1114),40,MUTED,center=True,weight=650)
            text(d,'808 / BASS / DRUMS',(W/2,1270),48,PINK,center=True,weight=750)
            text(d,'PRESERVED',(W/2,1337),42,PINK,center=True,weight=650)
            text(d,'GET v0.5.2-beta ON GITHUB',(W/2,1550),47,center=True,weight=750)
            text(d,'LINK IN CAPTION',(W/2,1630),32,MUTED,center=True,weight=650)
        # Actual sound-reactive spectrum and waveform.
        cy=1540 if INTRO<=t<INTRO+24 else 1760
        spectrum=np.abs(np.fft.rfft(chunk*np.hanning(2048)))
        boundaries=np.geomspace(2,700,45).astype(int)
        for j in range(44):
            v=float(np.max(spectrum[boundaries[j]:max(boundaries[j]+1,boundaries[j+1])]))
            height=min(65,4+math.log1p(v)*13)
            x=90+j*20.65
            d.rectangle((x,cy-height,x+8,cy+height),fill=ACCENT if j%3 else PINK)
        text(d,'RED MASS VAMPIRE OS',(90,1836),24,MUTED,weight=650)
        f=font(24,False,650);v='v0.5.2-beta';d.text((990-d.textlength(v,font=f),1836),v,font=f,fill=MUTED)
        if t<.25:im=Image.blend(Image.new('RGB',(W,H),(5,4,7)),im,t/.25)
        if t>TOTAL-.5:im=Image.blend(Image.new('RGB',(W,H),(5,4,7)),im,(TOTAL-t)/.5)
        if frame==34:im.save(out/'RED-MASS-v0.5.2-beta-Reel-Cover.png')
        if frame in [30,150,360,570,840,930]:im.resize((540,960)).save(out/f'qa-{frame:04d}.jpg',quality=90)
        encoder.stdin.write(im.tobytes())
        if frame%150==0:print(f'Reel {frame}/{frames}',flush=True)
    encoder.stdin.close();assert encoder.wait()==0
    (out/'CAPTION-EN.txt').write_text('RED MASS VAMPIRE OS v0.5.2 BETA IS OUT.\n\n216 darker voicings. 256 factory presets. Your 808s, basses and drums stay intact.\n\nHear six sounds from the new bank: DREADGRND, BONEHARP, NOSFERA2, DEATHBELL, NIGHTMASS and WOLFSYNC.\n\nMade for the M-VAVE FM1. Get the beta, installer and GPL-3.0 source on GitHub:\nhttps://github.com/karlalex732-dotcom/RED-MASS-VAMPIRE-OS-FOR-M-VAVE-FM1/releases/tag/v0.5.2-beta\n\nAudio and screens rendered from the firmware source. Physical-device qualification is still pending. Back up your music before updating.\n\n#RedMass #VampireOS #Karsei #MVaveFM1 #Synthesizer #SoundDesign #DarkAmbient #Trap\n')
    print(video,video.stat().st_size,'bytes',flush=True)

if __name__=='__main__':main()
