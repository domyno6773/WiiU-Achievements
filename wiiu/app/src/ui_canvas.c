/*
 * Wii U Achievements: original vector-look native renderer.
 * Adaptation of Theme.cs / Banner.cs design and original SIL OFL Nunito glyphs.
 * TV: 1280x720. GamePad: 854x480. This is not a bitmap of the PC screenshot.
 *
 * Colors are R,G,B,X as required by OSScreen.
 * Drawing only occurs on UI state transitions, never in the game process.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "ui_canvas.h"

static int mini(int a,int b) { return a < b ? a : b; }
static int maxi(int a,int b) { return a > b ? a : b; }

void ui_canvas_init(UiCanvas *c, OSScreenID screen) {
    c->screen = screen;
    c->width = screen == SCREEN_TV ? 1280 : 854;
    c->height = screen == SCREEN_TV ? 720 : 480;
}
static void pixel(UiCanvas *c, int x,int y, uint32_t color) {
    if ((unsigned)x < (unsigned)c->width && (unsigned)y < (unsigned)c->height)
        OSScreenPutPixelEx(c->screen,(uint32_t)x,(uint32_t)y,color);
}
void ui_fill(UiCanvas *c,int x,int y,int w,int h,uint32_t color) {
    if (w <= 0 || h <= 0) return;
    int left=maxi(0,x), right=mini(c->width,x+w);
    int top=maxi(0,y),bottom=mini(c->height,y+h);
    for(int py=top;py<bottom;++py)
        for(int px=left;px<right;++px)
            OSScreenPutPixelEx(c->screen,(uint32_t)px,(uint32_t)py,color);
}
void ui_hline(UiCanvas *c,int x,int y,int len,uint32_t color) {
    ui_fill(c,x,y,len,1,color);
}
void ui_round(UiCanvas *c,int x,int y,int w,int h,int radius,uint32_t color) {
    if(w<=0||h<=0)return;
    radius=mini(maxi(radius,0),mini(w,h)/2);
    if(radius==0){ui_fill(c,x,y,w,h,color);return;}
    int left=maxi(0,x),right=mini(c->width,x+w);
    int top=maxi(0,y),bottom=mini(c->height,y+h);
    for(int yy=top;yy<bottom;++yy) {
        int dy=yy<y+radius ? y+radius-1-yy :
               yy>=y+h-radius ? yy-(y+h-radius) : 0;
        for(int xx=left;xx<right;++xx) {
            int dx=xx<x+radius ? x+radius-1-xx :
                   xx>=x+w-radius ? xx-(x+w-radius) : 0;
            if(dx*dx+dy*dy<=radius*radius)
                OSScreenPutPixelEx(c->screen,(uint32_t)xx,(uint32_t)yy,color);
        }
    }
}
void ui_circle(UiCanvas *c,int x,int y,int r,uint32_t color) {
    if(r<=0)return;
    for(int yy=maxi(0,y-r);yy<mini(c->height,y+r+1);++yy)
        for(int xx=maxi(0,x-r);xx<mini(c->width,x+r+1);++xx) {
            int dx=xx-x,dy=yy-y;
            if(dx*dx+dy*dy<=r*r)
                OSScreenPutPixelEx(c->screen,(uint32_t)xx,(uint32_t)yy,color);
        }
}
static int edge(int ax,int ay,int bx,int by,int x,int y) {
    return (x-ax)*(by-ay)-(y-ay)*(bx-ax);
}
void ui_triangle(UiCanvas *c,int ax,int ay,int bx,int by,int cx,int cy,uint32_t color){
    int l=maxi(0,mini(ax,mini(bx,cx))),r=mini(c->width-1,maxi(ax,maxi(bx,cx)));
    int t=maxi(0,mini(ay,mini(by,cy))),b=mini(c->height-1,maxi(ay,maxi(by,cy)));
    for(int y=t;y<=b;y++)for(int x=l;x<=r;x++) {
        int a=edge(ax,ay,bx,by,x,y),d=edge(bx,by,cx,cy,x,y),e=edge(cx,cy,ax,ay,x,y);
        if((a>=0&&d>=0&&e>=0)||(a<=0&&d<=0&&e<=0))pixel(c,x,y,color);
    }
}
static uint32_t glyph_next(const char **input) {
    const unsigned char *s=(const unsigned char *)*input;
    if(!*s)return 0;
    if(s[0]<0x80){++*input;return s[0];}
    if((s[0]&0xe0)==0xc0&&(s[1]&0xc0)==0x80){
        *input+=2;return ((uint32_t)(s[0]&31)<<6)|(s[1]&63);
    }
    if((s[0]&0xf0)==0xe0&&(s[1]&0xc0)==0x80&&(s[2]&0xc0)==0x80){
        *input+=3;return ((uint32_t)(s[0]&15)<<12)|((uint32_t)(s[1]&63)<<6)|(s[2]&63);
    }
    if((s[0]&0xf8)==0xf0&&(s[1]&0xc0)==0x80&&(s[2]&0xc0)==0x80&&(s[3]&0xc0)==0x80){
        *input+=4;return ((uint32_t)(s[0]&7)<<18)|((uint32_t)(s[1]&63)<<12)|((uint32_t)(s[2]&63)<<6)|(s[3]&63);
    }
    ++*input;return '?';
}
static const NunitoGlyph *glyph(unsigned cp,int face) {
    const NunitoFace *f=&nunito_faces[face];
    unsigned l=0,r=f->count;
    while(l<r){unsigned m=(l+r)/2;
        if(f->glyphs[m].codepoint < cp)l=m+1;else r=m;
    }
    if(l<f->count&&f->glyphs[l].codepoint==cp)return &f->glyphs[l];
    if(cp=='?')return NULL;
    return glyph('?',face);
}
int ui_text_width(const char *text,int face) {
    if(!text||face<0||face>5)return 0;
    int w=0;
    while(*text&&*text!='\n'){
        uint32_t cp=glyph_next(&text);
        const NunitoGlyph *g=glyph(cp,face);
        if(g)w+=g->advance;
    }
    return w;
}
static uint32_t mix(uint32_t foreground,uint32_t background,unsigned a){
    if(a>=253)return foreground;
    if(a<8)return background;
    unsigned inv=255-a;
    uint32_t out=0;
    for(int shift=8;shift<=24;shift+=8){
        uint32_t f=(foreground>>shift)&255,b=(background>>shift)&255;
        out |= ((f*a+b*inv+127)/255)<<shift;
    }
    return out;
}
void ui_text(UiCanvas *c,int x,int y,const char *text,int face,
             uint32_t color,uint32_t bg,int max_width) {
    if(!text||face<0||face>5||max_width<=0)return;
    int cursor=x,right=x+max_width;
    while(*text&&*text!='\n'){
        uint32_t cp=glyph_next(&text);
        const NunitoGlyph *g=glyph(cp,face);
        if(!g)continue;
        if(cursor+g->advance>right)break;
        if(g->offset+(unsigned)g->width*(unsigned)g->height>nunito_pixels_size)break;
        const uint8_t *mask=nunito_pixels+g->offset;
        for(unsigned gy=0;gy<g->height;gy++)
            for(unsigned gx=0;gx<g->width;gx++) {
                unsigned a=mask[gy*g->width+gx];
                if(a>=12)pixel(c,cursor+g->left+(int)gx,y+g->top+(int)gy,mix(color,bg,a));
            }
        cursor+=g->advance;
    }
}
int ui_wrap(UiCanvas *c,int x,int y,const char *text,int face,
            int max_width,int max_lines,int line_height,uint32_t color,uint32_t bg) {
    if(!text||max_width<=0||max_lines<=0)return y;
    for(int n=0;n<max_lines&&*text;n++){
        char line[512];int bytes=0,w=0,last_space=-1;
        const char *read=text;
        while(*read&&*read!='\n'&&bytes<490){
            const char *before=read;
            unsigned cp=glyph_next(&read);
            const NunitoGlyph *g=glyph(cp,face);
            int advance=g?g->advance:0;
            if(w+advance>max_width)break;
            if(cp==' ')last_space=bytes;
            int count=(int)(read-before);
            memcpy(line+bytes,before,(size_t)count);
            bytes+=count;w+=advance;
        }
        if(bytes==0){glyph_next(&text);continue;}
        if(*read&&*read!='\n'&&last_space>4)bytes=last_space;
        line[bytes]='\0';
        ui_text(c,x,y,line,face,color,bg,max_width);
        text+=bytes;
        while(*text==' ')text++;
        if(*text=='\n')text++;
        y+=line_height;
    }
    return y;
}
void ui_background(UiCanvas *c) {
    OSScreenClearBufferEx(c->screen,UI_BG);
    /* Fine diagonal system-menu stripe motif, static and inexpensive. */
    for(int y=51;y<c->height-48;y+=10)
        for(int x=0;x<c->width;x+=24)
            ui_hline(c,x+((y/10)%3)*6,y,9,UI_BG_STRIPE);
}
void ui_ocean_banner(UiCanvas *c,int x,int y,int w,int h) {
    ui_round(c,x,y+5,w,h,20,0xD2DEE600u);
    ui_round(c,x,y,w,h,20,0x63C5F200u);
    /* Static sky layers, inspired by source OceanBanner. */
    for(int i=0;i<5;i++)ui_fill(c,x+3,y+3+i*(h/13),w-6,h/13,
       i%2?0x6BCBF000u:0x64C7F200u);
    int water_y=y+h*56/100;
    ui_fill(c,x+3,water_y,w-6,y+h-water_y-2,0x288DC800u);
    ui_fill(c,x+3,water_y+(h/6),w-6,h/6,0x187BB600u);
    ui_fill(c,x+3,water_y+(h/3),w-6,h/8,0x0E69A500u);
    for(int i=0;i<3;i++){
        int cy=water_y+12+i*(h/8);
        for(int xx=x+5;xx<x+w-8;xx++){
            int phase=((xx-x+i*37)%160)-80;
            int v=(phase*phase/640)-5;
            pixel(c,xx,cy+v,i==0?0x90D7FA00u:0x4EB7E500u);
            if(i==0)pixel(c,xx,cy+v+1,0x90D7FA00u);
        }
    }
    /* Original code-drawn island, sun, clouds: no extracted game assets. */
    int sunx=x+w*68/100,suny=y+h/3;
    ui_circle(c,sunx,suny,h/8,0xFFF2B300u);
    ui_circle(c,sunx,suny,h/10,0xFFF9DD00u);
    ui_circle(c,x+w*82/100,y+h*30/100,h/11,UI_CLOUD);
    ui_round(c,x+w*77/100,y+h*29/100,w/8,h/12,h/22,UI_CLOUD);
    ui_round(c,x+w*63/100,y+h*39/100,w/12,h/16,h/22,UI_CLOUD);
    int ix=x+w*60/100,iy=water_y-1;
    ui_circle(c,ix,iy,h/7,0x6EB15A00u);
    ui_fill(c,ix-h/5,iy-h/45,h/2,h/13,0x5B985E00u);
    /* Palm silhouette */
    ui_fill(c,ix+h/10,iy-h/3,3,h/4,0x7D6B3C00u);
    ui_triangle(c,ix+h/10,iy-h/3,ix-h/7,iy-h/2,ix+h/16,iy-h*32/100,0x2B9D6300u);
    ui_triangle(c,ix+h/10,iy-h/3,ix+h/4,iy-h/2,ix+h/16,iy-h*32/100,0x2B9D6300u);
}
void ui_medal(UiCanvas *c,int cx,int cy,int r,
              const char *rarity,const char *field,int pending){
    uint32_t ring=UI_BRONZE;
    if(rarity&&strcmp(rarity,"silver")==0)ring=UI_SILVER;
    if(rarity&&strcmp(rarity,"gold")==0)ring=UI_GOLD;
    ui_circle(c,cx,cy+3,r+3,0xCBD4DA00u);
    ui_circle(c,cx,cy,r+2,ring);
    ui_circle(c,cx,cy,r-4,0xF3F6F800u);
    ui_circle(c,cx,cy,r-8,0xD5DDE200u);
    ui_circle(c,cx,cy,r-12,0xF5F7F800u);
    if(pending){
        ui_text(c,cx-8,cy-13,"?",NUNITO_EXTRABOLD_25,UI_MUTED,0xF5F7F800u,28);
    } else if(field&&strcmp(field,"max_health")==0){
        int size=r/2;
        ui_circle(c,cx-size/2,cy-3,size,0xAF677900u);
        ui_circle(c,cx+size/2,cy-3,size,0xAF677900u);
        ui_triangle(c,cx-size*3/2,cy-2,cx+size*3/2,cy-2,cx,cy+size*2,0xAF677900u);
        ui_circle(c,cx-size/2,cy-7,size/3,0xD2A4B000u);
    }else{
        ui_triangle(c,cx-r/3,cy-r/3,cx+r/3,cy-r/3,cx,cy+r/2,0x249F7700u);
        ui_triangle(c,cx-r/3,cy-r/3,cx,cy-r*2/3,cx+r/3,cy-r/3,0x66DCA600u);
        ui_triangle(c,cx-r/3,cy-r/3,cx,cy+r/2,cx,cy-r/3,0x48CE9500u);
        ui_triangle(c,cx+r/3,cy-r/3,cx,cy+r/2,cx,cy-r/3,0x0A876800u);
    }
    if(!pending){
        /* Opaque lock badge signals unearned/unknown, never a fake unlock. */
        int bx=cx+r*3/4,by=cy-r*3/4;
        ui_circle(c,bx,by,r/3+2,UI_WHITE);
        ui_circle(c,bx,by,r/3,0x929EA700u);
        ui_round(c,bx-r/7,by-2,r*2/7,r/4,r/12,UI_WHITE);
        ui_round(c,bx-r/10,by-r/5,r/5,r/4,r/12,UI_WHITE);
    }
}
