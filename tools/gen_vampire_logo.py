#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""RED MASS wordmarks and bounded scanline runs, from the OFL Inter Tight face."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import sys
root=Path(__file__).resolve().parents[1]
def raster(text,size,width=None,height=None):
 f=ImageFont.truetype(str(root/'assets/fonts/InterTight[wght].ttf'),size);f.set_variation_by_axes([700])
 b=f.getbbox(text);im=Image.new('L',(b[2]-b[0],b[3]-b[1]));ImageDraw.Draw(im).text((-b[0],-b[1]),text,font=f,fill=255)
 if width:im=im.resize((width,height),Image.Resampling.LANCZOS)
 return im
def runs(im):
 out=[]
 for y in range(im.height):
  row=[];x=0
  while x<im.width:
   if im.getpixel((x,y))<110:x+=1;continue
   start=x
   while x<im.width and im.getpixel((x,y))>=110:x+=1
   row.extend([start,x-start])
  out.extend([len(row)//2,*row])
 return out
lines=['/* Generated from Inter Tight (SIL OFL 1.1). */','#pragma once']
for name,im in [('VAMP_LOGO',raster('RED MASS',64,206,45)),('RM_WORD',raster('RED MASS',22,74,13))]+[(f'RM_DIGIT_{i}',raster(str(i),65))for i in range(7)]:
 r=runs(im);lines.extend([f'#define {name}_W {im.width}u',f'#define {name}_H {im.height}u',f'static const uint8_t {name}[] = {{'+','.join(map(str,r))+'};'])
lines.append('static const uint8_t *const RM_DIGIT[7] = {'+','.join(f'RM_DIGIT_{i}'for i in range(7))+'};')
lines.append('static const uint8_t RM_DIGIT_H[7] = {'+','.join(f'RM_DIGIT_{i}_H' for i in range(7))+'};')
Path(sys.argv[1]).write_text('\n'.join(lines)+'\n')
print('RED MASS wordmark and seven digits generated')
