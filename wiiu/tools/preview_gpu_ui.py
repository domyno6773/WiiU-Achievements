#!/usr/bin/env python3
"""Inspect GPU UI layout with the *same* baked sprites and original Nunito.

This is a host-side layout preview of the SDL2 frontend, not a Wii U screenshot.
It contains only genuine catalog entries, with no fabricated save progress.
"""
from PIL import Image,ImageDraw,ImageFont
from pathlib import Path
import argparse,sys
sys.path.insert(0,str(Path(__file__).resolve().parent))
from generate_catalog import load_catalog

def font(root,weight,size):
 return ImageFont.truetype(str(root/'assets/fonts'/('Nunito-'+weight+'.ttf')),size)
def text(draw,x,y,s,f,color,width=None):
 if width is not None:
  while draw.textbbox((x,y),s,font=f)[2]-x>width and s:
   s=s[:-1]
  if s and width and draw.textbbox((x,y),s,font=f)[2]-x>width-15:s=s[:-2]+'…'
 draw.text((x,y),s,font=f,fill=color,stroke_width=0)
def wrap(draw,x,y,s,f,color,maxwidth,lines,lineh):
 import textwrap
 words=s.split();line=''
 for word in words:
  test=(line+' '+word).strip()
  if draw.textbbox((0,0),test,font=f)[2]>maxwidth and line:
   text(draw,x,y,line,f,color);y+=lineh;lines-=1
   if not lines:return
   line=word
  else:line=test
 if line:text(draw,x,y,line,f,color)
def put(im,asset,x,y,w,h):
 im.alpha_composite(Image.open(asset).convert('RGBA').resize((w,h),Image.Resampling.LANCZOS),(x,y))
def main():
 p=argparse.ArgumentParser()
 p.add_argument('--root',type=Path,required=True)
 p.add_argument('--assets',type=Path,required=True)
 p.add_argument('--out',type=Path,required=True)
 opt=p.parse_args()
 _,_,entries=load_catalog(opt.root)
 opt.out.mkdir(parents=True,exist_ok=True)
 for w,h,cols,rows,asset in [(1280,720,3,3,'tv'),(854,480,2,2,'drc')]:
  tv=w>1000
  im=Image.open(opt.assets/('shell_'+asset+'.png')).convert('RGBA')
  d=ImageDraw.Draw(im)
  ink='#343A40';muted='#7C8790';accent='#00A8E2'
  small=font(opt.root,'Regular',13 if tv else 11)
  regular=font(opt.root,'Regular',16 if tv else 13)
  bold=font(opt.root,'Bold',18 if tv else 14)
  heavy=font(opt.root,'ExtraBold',32 if tv else 22)
  text(d,74 if tv else 48,12 if tv else 7,'Profilo locale',bold,ink)
  if tv:text(d,74,34,'Non collegato',small,muted)
  text(d,484 if tv else 286,8 if tv else 6,'Wii U Achievements',heavy,ink)
  text(d,967 if tv else 652,22 if tv else 14,'MONITOR NON ATTIVO' if tv else 'NON ATTIVO',small,'#D28917')
  text(d,49 if tv else 34,89 if tv else 60,'The Wind Waker HD',heavy,'#FFFFFF')
  text(d,49 if tv else 34,132 if tv else 92,'EUR v0 · Console Wii U',small,'#FFFFFF')
  text(d,49 if tv else 34,158 if tv else 116,'Rilevamento non ancora disponibile',bold,'#FFFFFF')
  text(d,1112 if tv else 752,146 if tv else 86,'110',heavy,ink)
  text(d,1091 if tv else 729,187 if tv else 115,'OBIETTIVI',small,accent)
  text(d,62 if tv else 37,226 if tv else 145,'70 regole Windows',small,ink)
  text(d,269 if tv else 208,226 if tv else 145,'40 da verificare',small,ink)
  tab_y=288 if tv else 181
  xx=[22,157,318,502] if tv else [15,106,229,375]
  tabs=['Tutti 110','Sbloccati --','Da sbloccare 70','Da verificare 40']
  for i,(x,label) in enumerate(zip(xx,tabs)):
   text(d,x+10,tab_y+2,label,bold if i==0 else small,accent if i==0 else muted)
  text(d,30 if tv else 18,327 if tv else 215,
    'Categoria: Tutte    ·    Tappa: Tutte le tappe',small,muted)
  x0=22 if tv else 17;gap=13 if tv else 11;top=353 if tv else 237
  cw=400 if tv else 399;ch=98 if tv else 91;rgap=9 if tv else 10
  for i,e in enumerate(entries[:cols*rows]):
   cx=x0+(i%cols)*(cw+gap);cy=top+(i//cols)*(ch+rgap)
   put(im,opt.assets/('card_'+asset+('_focus' if i==0 else '')+'.png'),cx-9,cy-7,cw+18,ch+18)
   tier=e['rarity']
   kind='heart' if e['field']=='max_health' else 'rupee'
   if e['field']=='rupees':
    rank=sum(1 for r in entries if r['executable'] and r['field']=='rupees' and r['threshold']<e['threshold'])
    kind=['rupee','rupee_blue','rupee_red','rupee_purple','rupee_gold','rupee_silver'][min(rank,5)]
   put(im,opt.assets/f'medal_{tier}_{kind}.png',cx+7,cy+(ch-(81 if tv else 68))//2,81 if tv else 68,81 if tv else 68)
   d=ImageDraw.Draw(im)
   tx=cx+(97 if tv else 80);maxw=cw-(tx-cx)-14
   text(d,tx,cy+8,e['title'],bold,ink,maxw-49)
   wrap(d,tx,cy+(34 if tv else 30),e['description'],small,muted,maxw,2,17 if tv else 15)
   ft=cy+ch-(28 if tv else 25)
   label='Non attivo' if not e['executable'] else ('Regola sperimentale' if e['experimental'] else 'Da sbloccare · non monitorato')
   text(d,tx,ft,label,small,'#EC9214' if not e['executable'] else muted,maxw)
   if e['executable']:
    d.rounded_rectangle((tx,cy+ch-9,tx+(145 if tv else 123),cy+ch-5),radius=2,fill='#E1E9EE')
    text(d,cx+cw-41,cy+9,'+'+str(e['points']),small,'#D49727',40)
  bx=[20,202,364,519] if tv else [16,160,294,432]
  keys=['A','B','X','Y']
  labels=['Dettagli','Giochi','Filtri','Categoria']
  top=h-(51 if tv else 40)+(10 if tv else 8)
  for xx,key,label in zip(bx,keys,labels):
   text(d,xx+(14 if tv else 12),top,key,small,accent)
   text(d,xx+(40 if tv else 34),top,label,small,ink)
  text(d,898 if tv else 660,top,'L/R Tappa' if tv else 'L/R',small,muted)
  im=im.convert('RGB')
  out=opt.out/('TV-Catalog.png' if tv else 'GamePad-Catalog.png')
  im.save(out,optimize=True)
  print(out)
if __name__=='__main__':main()
