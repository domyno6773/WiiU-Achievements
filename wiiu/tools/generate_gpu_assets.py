#!/usr/bin/env python3
"""Original Windows-UI-faithful Wii U visual assets, generated at build time.

The artwork follows src/Theme.cs, src/Banner.cs and docs/screenshots/main.png:
rounded white tiles, original palette, Wii U cyan focus, layered ocean, subtle
shadows, bronze/silver/gold medals and original lock/heart/rupee shapes.
No Nintendo textures, Mii assets, logos, proprietary sounds or ripped files.

The generator creates supersampled PNG sprites and an assembler .incbin manifest.
All assets are embedded in the *one* WUHB, so SD installation stays single-file.
"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter
import argparse
import math

SCALE = 2
PAL = dict(ink="#343A40", muted="#7C8790", line="#D4DBE1",
           accent="#00A8E2", bg="#E9EFF4", cyan="#D6F1FC",
           sky="#62C5F1", sea="#0878B8")
FILES = []

def rgb(v):
    v=v.lstrip("#")
    return tuple(int(v[i:i+2],16) for i in (0,2,4))

def sc(box):
    return tuple(round(x*SCALE) for x in box)

def rect(d, box, fill, radius=0, outline=None, width=1):
    d.rounded_rectangle(sc(box), radius=radius*SCALE,
                        fill=fill, outline=outline, width=width*SCALE)

def ellipse(d, box, fill, outline=None, width=1):
    d.ellipse(sc(box),fill=fill,outline=outline,width=width*SCALE)

def polygon(d, points, fill):
    d.polygon([(int(x*SCALE),int(y*SCALE)) for x,y in points], fill=fill)

def line(d, points, fill, width=1):
    d.line([(int(x*SCALE),int(y*SCALE)) for x,y in points],
           fill=fill,width=width*SCALE,joint="curve")

def shadow(image,box,radius,blur=6,opacity=40,offset=3):
    layer=Image.new("RGBA",image.size,(0,0,0,0))
    d=ImageDraw.Draw(layer)
    x0,y0,x1,y1=box
    rect(d,(x0,y0+offset,x1,y1+offset),(26,52,75,opacity),radius)
    layer=layer.filter(ImageFilter.GaussianBlur(blur*SCALE))
    image.alpha_composite(layer)

def gradient(image,box,start,end):
    x0,y0,x1,y1=sc(box)
    start,end=rgb(start),rgb(end)
    d=ImageDraw.Draw(image)
    length=max(1,y1-y0)
    for y in range(y0,y1):
        t=(y-y0)/length
        c=tuple(round(start[i]*(1-t)+end[i]*t) for i in range(3))
        d.line([(x0,y),(x1,y)],fill=c+(255,),width=1)

def save(img,dest,name):
    dest.mkdir(parents=True,exist_ok=True)
    path=dest/(name+".png")
    if SCALE!=1:
        img=img.resize((img.width//SCALE,img.height//SCALE),Image.Resampling.LANCZOS)
    img.save(path,optimize=True)
    FILES.append((name,path))

def ocean(image,box):
    x,y,w,h=box
    crop_rect=sc((x,y,x+w,y+h))
    background=image.crop(crop_rect)
    d=ImageDraw.Draw(image)
    shadow(image,(x,y,x+w,y+h),19,blur=7,opacity=60,offset=4)
    rect(d,(x,y,x+w,y+h),"#54BDF0",20)
    # atmospheric sky, lighter toward horizon
    gradient(image,(x+2,y+2,x+w-2,y+h*.72),"#4CB5ED","#C5EEF8")
    # tall translucent ocean sky streaks and pale distant land mass
    polygon(d,[(x,y+h*.62),(x+w*.2,y+h*.49),
      (x+w*.43,y+h*.57),(x+w*.63,y+h*.43),(x+w,y+h*.59),
      (x+w,y+h*.74),(x,y+h*.74)],(66,135,175,74))
    # pale distant islands
    polygon(d,[(x+w*.16,y+h*.63),(x+w*.28,y+h*.55),
      (x+w*.39,y+h*.63)],"#89BBD1")
    # sun and clouds like source OceanBanner
    sx,sy=x+w*.605,y+h*.31
    ellipse(d,(sx-h*.14,sy-h*.14,sx+h*.14,sy+h*.14),"#FFF1AF")
    ellipse(d,(sx-h*.1,sy-h*.1,sx+h*.1,sy+h*.1),"#FFFBE1")
    for cx,cy,s in [(x+w*.63,y+h*.38,1),(x+w*.71,y+h*.4,.8),
                     (x+w*.93,y+h*.28,.8)]:
        ellipse(d,(cx-23*s,cy-12*s,cx+23*s,cy+12*s),"#EFFAFF")
        ellipse(d,(cx-42*s,cy-4*s,cx+12*s,cy+11*s),"#E7F9FD")
        ellipse(d,(cx+9*s,cy-8*s,cx+42*s,cy+11*s),"#E7F9FD")
    # horizon, foamy waves and three ocean layers
    horizon=y+h*.59
    rect(d,(x+2,horizon,x+w-2,y+h-2),"#288FCB")
    for j,(color,start) in enumerate([("#147DBB",.67),("#0A6BAC",.81),("#0B5DA5",.92)]):
        top=y+h*start
        pts=[(x+2,top)]
        for px in range(int(x+2),int(x+w-2),3):
            theta=(px-x)/37+j*.55
            pts.append((px,top+2.6*math.sin(theta)+1.6*math.sin(theta*.37)))
        pts.extend([(x+w-2,y+h-2),(x+2,y+h-2)])
        polygon(d,pts,color)
    # multiple white/blue curved wave lines along entire banner
    for j in range(5):
        yy=y+h*(.61+j*.073)
        pts=[]
        for px in range(int(x+2),int(x+w-2),3):
            phase=(px-x)/21.4+j*1.43
            pts.append((px,yy+3.2*math.sin(phase)+1.2*math.sin(phase*.56)))
        line(d,pts,("#6ED0F5" if j<3 else "#8FD6F4"),1)
    # island palm tree
    ix,iy=x+w*.50,horizon-1
    ps=min(1.0,h/188.0)
    def palm_point(px,py):
        return (ix+px*ps,iy+py*ps)
    polygon(d,[palm_point(a,b) for a,b in
        [(-57,0),(-32,-10),(-10,-21),(20,-25),(50,-13),(69,0)]],"#72B254")
    polygon(d,[palm_point(a,b) for a,b in
        [(-18,0),(-6,-9),(44,-13),(58,0)]],"#399C49")
    line(d,[palm_point(a,b) for a,b in
       [(22,-18),(26,-48),(20,-61)]],"#796A39",max(1,round(3*ps)))
    for branch in [
        [(20,-61),(4,-69),(-2,-82)],
        [(20,-61),(30,-71),(35,-86)],
        [(20,-61),(39,-65),(51,-71)],
        [(20,-61),(15,-77),(14,-82)]]:
        line(d,[palm_point(a,b) for a,b in branch],"#238D53",max(2,round(5*ps)))
    # rounded border
    rect(d,(x,y,x+w,y+h),None,20,outline="#8BD7FC",width=1)
    # Clip all waves, gradients and scenery to the original rounded banner.
    artwork=image.crop(crop_rect)
    mask=Image.new("L",artwork.size,0)
    ImageDraw.Draw(mask).rounded_rectangle(
        (0,0,artwork.width-1,artwork.height-1),
        radius=20*SCALE,fill=255)
    background.paste(artwork,(0,0),mask)
    image.paste(background,(crop_rect[0],crop_rect[1]))

def shell(w,h,name,dest):
    tv=w>1000
    im=Image.new("RGBA",(w*SCALE,h*SCALE),"#F9FBFD")
    gradient(im,(0,0,w,h),"#FCFDFE","#E0E9F0")
    d=ImageDraw.Draw(im)
    # fine striped "Wii U system" backdrop
    for n in range(-h,w+h,8):
        line(d,[(n,60),(n-h*.64,h-43)],"#EDF3F7",1)
    head=60 if tv else 43
    rect(d,(0,0,w,head),"#FCFDFE")
    line(d,[(0,head),(w,head)],"#C8D5DF")
    # top avatar
    avx,avy=(37,30) if tv else (24,21)
    ellipse(d,(avx-21 if tv else avx-16,avy-21 if tv else avy-16,
               avx+21 if tv else avx+16,avy+21 if tv else avy+16),
               "#FFFFFF",outline="#C8DCEB",width=1)
    ellipse(d,(avx-17 if tv else avx-12,avy-17 if tv else avy-12,
               avx+17 if tv else avx+12,avy+17 if tv else avy+12),
               "#32B7E9")
    ellipse(d,(avx-7,avy-10,avx+7,avy+4),"#FFFFFF")
    ellipse(d,(avx-13,avy+5,avx+13,avy+16 if tv else avy+12),"#FFFFFF")
    # Original system-menu-like icons, drawn without proprietary assets.
    side=w-(58 if tv else 39)
    for cx in (side,side-(56 if tv else 42)):
        ellipse(d,(cx-17,head/2-17,cx+17,head/2+17),"#F9FBFD",outline="#D4E0E8")
    gear=side-(56 if tv else 42)
    ellipse(d,(gear-7,head/2-7,gear+7,head/2+7),"#8998A3")
    ellipse(d,(gear-3,head/2-3,gear+3,head/2+3),"#F9FBFD")
    for j in range(8):
        a=j*math.pi/4
        line(d,[(gear+9*math.cos(a),head/2+9*math.sin(a)),
                (gear+12*math.cos(a),head/2+12*math.sin(a))],"#8998A3",2)
    for dy in (-6,0,6):
        ellipse(d,(side-8,head/2+dy-1,side-6,head/2+dy+1),"#8D9BA5")
        line(d,[(side-3,head/2+dy),(side+9,head/2+dy)],"#8D9BA5",2)
    tx=454 if tv else 264; ty=head/2
    polygon(d,[(tx-10,ty-10),(tx+10,ty-10),(tx+7,ty+3),
               (tx+2,ty+7),(tx+2,ty+11),(tx+8,ty+11),
               (tx+8,ty+14),(tx-8,ty+14),(tx-8,ty+11),
               (tx-2,ty+11),(tx-2,ty+7),(tx-7,ty+3)],"#E6AC35")
    d.arc(sc((tx-16,ty-10,tx-2,ty+8)),90,250,fill="#DDA12A",width=2*SCALE)
    d.arc(sc((tx+2,ty-10,tx+16,ty+8)),290,90,fill="#DDA12A",width=2*SCALE)
    # banner, same proportions as original screenshot
    banner=(21,78,w-42,188) if tv else (15,53,w-30,121)
    ocean(im,banner)
    # Original Windows banner: pale circular objective dial, inset status pills.
    bx,by,bw,bh=banner
    rr=77 if tv else 49
    rx=bx+bw-(110 if tv else 79)
    ry=by+bh//2
    shadow(im,(rx-rr,ry-rr,rx+rr,ry+rr),rr,blur=6,opacity=48,offset=5)
    d=ImageDraw.Draw(im)
    ellipse(d,(rx-rr-2,ry-rr-2,rx+rr+2,ry+rr+2),"#E3ECF2")
    ellipse(d,(rx-rr+4,ry-rr+4,rx+rr-4,ry+rr-4),"#FFFFFF")
    ellipse(d,(rx-rr+14,ry-rr+14,rx+rr-14,ry+rr-14),
            "#E3ECF2",outline="#C6D5DF",width=2)
    ellipse(d,(rx-rr+25,ry-rr+25,rx+rr-25,ry+rr-25),"#F9FBFC")
    # These arcs represent decorative shine, not fabricated achievement progress.
    d.arc(sc((rx-rr+7,ry-rr+7,rx+rr-7,ry+rr-7)),
          start=218,end=281,fill="#C8EAF7",width=(5 if tv else 3)*SCALE)
    # Re-create the original inset white information pills.
    py=by+bh-(46 if tv else 34)
    p_h=30 if tv else 23
    specs=[(bx+(22 if tv else 11),196 if tv else 170),
           (bx+(224 if tv else 184),187 if tv else 161)]
    for px,pw in specs:
        shadow(im,(px,py,px+pw,py+p_h),p_h//2,blur=3,opacity=27,offset=2)
        rect(d,(px,py,px+pw,py+p_h),"#F4F8FB",p_h//2,outline="#DAE5EC")
    # Open badge in system header uses no Cemu/Windows terminology.
    sx=954 if tv else 630
    sw=172 if tv else 128
    sy=13 if tv else 8
    rect(d,(sx,sy,sx+sw,sy+(34 if tv else 27)),"#FFF8EF",17,
         outline="#DBE3E8")
    # Make room for the fourth status tab in one segmented strip.
    # GamePad button bar: beveled white, pill-sized Wii U controls.
    foot=h-(51 if tv else 40)
    gradient(im,(0,foot,w,h),"#F5F8FA","#E2E9EF")
    rect(d,(0,foot,w,foot+1),"#C6D4DE")
    buttons=([(20,170),(202,149),(364,143),(519,191)] if tv
             else [(16,137),(160,126),(294,131),(432,155)])
    for i,(px,pw) in enumerate(buttons):
        py=foot+(6 if tv else 5);bh=37 if tv else 30
        shadow(im,(px,py,px+pw,py+bh),bh//2,blur=2,opacity=38,offset=2)
        rect(d,(px,py,px+pw,py+bh),"#FFFFFF" if i else "#D2EEF9",bh//2,
             outline="#C5D3DC")
        ellipse(d,(px+8,py+7,px+(30 if tv else 25),py+(30 if tv else 24)),
               "#FFFFFF" if not i else "#EEF3F7")
    # filter tab container under ocean
    fy=(283 if tv else 180)
    rect(d,(22 if tv else 15,fy,713 if tv else 533,fy+(38 if tv else 29)),
         "#E3EAF0",19,outline="#C8D4DE")
    # Elevated white segmented selector matches the source status bar.
    if tv:
        shadow(im,(23,285,153,317),16,blur=2,opacity=22,offset=1)
        rect(d,(24,285,151,317),"#FFFFFF",16,outline="#CDD8E1")
    else:
        shadow(im,(15,181,104,209),14,blur=2,opacity=20,offset=1)
        rect(d,(16,182,103,208),"#FFFFFF",13,outline="#CDD8E1")
    save(im,dest,name)

def card_template(w,h,selected,dest,name):
    im=Image.new("RGBA",((w+18)*SCALE,(h+18)*SCALE),(0,0,0,0))
    shadow(im,(9,7,w+9,h+7),12,blur=5,opacity=65,offset=3)
    d=ImageDraw.Draw(im)
    rect(d,(9,7,w+9,h+7),"#D7E1E8" if not selected else "#00A8E2",12)
    rect(d,(11 if not selected else 12,9 if not selected else 10,
             w+7 if not selected else w+6,
             h+5 if not selected else h+4),"#FDFEFF",10)
    # polished vertical top gradients, very subtle
    gradient(im,(16,12,w+2,18),"#FFFFFF","#F7FAFC")
    if selected:
        rect(d,(9,7,w+9,h+7),None,12,outline="#00B7F4",width=2)
    save(im,dest,name)

def medal(tier,kind,dest,name):
    # High-detail original GDI+-inspired medal. Source art is generated shapes.
    size=100
    im=Image.new("RGBA",(size*SCALE,size*SCALE),(0,0,0,0))
    shadow(im,(12,9,90,87),40,blur=3,opacity=65,offset=3)
    d=ImageDraw.Draw(im)
    outer={"bronze":("#CA8E5A","#A97349"),
           "silver":("#CFD3D8","#999FA7"),
           "gold":("#F7CF60","#D8A23B")}[tier]
    ellipse(d,(10,7,88,85),outer[1],outline="#8B9198",width=1)
    ellipse(d,(13,9,85,81),outer[0],outline="#8D9094",width=1)
    ellipse(d,(19,15,79,75),"#DDE3E7")
    ellipse(d,(23,19,75,71),"#F9FAFB",outline="#C0C8CE",width=2)
    ellipse(d,(27,22,71,67),"#E8ECEF",outline="#D6DBE0",width=1)
    ellipse(d,(34,25,61,35),(255,255,255,128))
    if kind=="rupee":
        polygon(d,[(51,26),(63,35),(62,53),(51,64),(40,53),(39,35)],
                "#3C936F")
        polygon(d,[(51,26),(63,35),(51,39),(39,35)],"#8CF2BD")
        polygon(d,[(39,35),(51,39),(51,64),(40,53)],"#66D5A5")
        polygon(d,[(63,35),(62,53),(51,64),(51,39)],"#1E7954")
        polygon(d,[(51,33),(58,38),(51,43),(44,38)],"#C9FDE4")
    else:
        ellipse(d,(35,32,53,49),"#CBA2AC")
        ellipse(d,(49,32,67,49),"#CBA2AC")
        polygon(d,[(34,43),(68,43),(51,64)],"#B28691")
        ellipse(d,(39,35,49,44),"#F3DFE3")
    # visible lock badge matching Windows screenshot
    ellipse(d,(65,5,93,33),"#FFFFFF")
    ellipse(d,(69,8,90,30),"#919BA5")
    rect(d,(76,19,84,25),"#FFFFFF",2)
    d.arc(sc((75,12,85,22)),180,360,fill="#FFFFFF",width=2*SCALE)
    save(im,dest,name)

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--out",type=Path,required=True)
    p.add_argument("--root",type=Path,required=True)
    a=p.parse_args()
    dest=a.out
    shell(1280,720,"shell_tv",dest)
    shell(854,480,"shell_drc",dest)
    for screen,w,h in (("tv",392,98),("drc",398,91)):
        for focus in (False,True):
            card_template(w,h,focus,dest,"card_"+screen+("_focus" if focus else ""))
    for tier in ("bronze","silver","gold"):
        for kind in ("rupee","heart"):
            medal(tier,kind,dest,"medal_"+tier+"_"+kind)
    # Single-file payload. .incbin embeds compressed PNGs plus original
    # upstream Nunito fonts into the .wuhb; SDL2_image decodes them to GX2.
    for weight in ("Regular","Bold","ExtraBold"):
        FILES.append(("nunito_"+weight.lower(),a.root/"assets/fonts"/("Nunito-"+weight+".ttf")))
    assembly=["/* Generated Wii U high-fidelity assets: do not edit */",
              ".section .rodata", ".balign 4"]
    for name,path in FILES:
        absfile=("gpu-assets/"+path.name) if path.parent==dest else ("../../../assets/fonts/"+path.name)
        assembly+=[".global asset_"+name+"_start",
                   "asset_"+name+"_start:",
                   f'.incbin "{absfile}"',
                   ".global asset_"+name+"_end",
                   "asset_"+name+"_end:",
                   ".balign 4"]
    (dest/"visual_assets.S").write_text("\n".join(assembly)+"\n")
    print("Baked",len(FILES),"visual/font assets.")

if __name__=="__main__":
    main()
