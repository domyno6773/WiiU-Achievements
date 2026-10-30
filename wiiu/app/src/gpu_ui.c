/*
 * Wii U Achievements — faithful Wii U-native frontend.
 * UI: source Theme.cs/MainForm.Ui.cs/Banner.cs, data: original XML catalog.
 * GX2-accelerated SDL2 video, Nunito via SDL_ttf, original vector-derived sprites.
 * 1280x720 TV + 854x480 GamePad, coherent controller-only navigation.
 *
 * IMPORTANT: Read-only catalog viewer. No simulated progress, unlock awards,
 * game hooks, system-title patches, cloud or save access.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <vpad/input.h>
#include "catalog.h"

#define DECL_ASSET(n) extern const unsigned char asset_##n##_start[],asset_##n##_end[]
DECL_ASSET(shell_tv); DECL_ASSET(shell_drc);
DECL_ASSET(card_tv); DECL_ASSET(card_tv_focus);
DECL_ASSET(card_drc); DECL_ASSET(card_drc_focus);
DECL_ASSET(medal_bronze_rupee); DECL_ASSET(medal_bronze_heart);
DECL_ASSET(medal_silver_rupee); DECL_ASSET(medal_silver_heart);
DECL_ASSET(medal_gold_rupee); DECL_ASSET(medal_gold_heart);
DECL_ASSET(nunito_regular); DECL_ASSET(nunito_bold); DECL_ASSET(nunito_extrabold);

typedef enum { PAGE_CATALOG=0, PAGE_GAMES, PAGE_DETAILS, PAGE_INFO } Page;
typedef enum { FILT_ALL=0, FILT_UNLOCKED, FILT_AVAILABLE, FILT_PENDING } Filter;
typedef struct { Page page; Filter filter; unsigned category,chapter,selected; const AchievementEntry *detail; } ViewState;
typedef struct {
 SDL_Window *window;
 SDL_Renderer *renderer;
 SDL_Texture *shell,*card,*card_focus,*medals[3][2],*static_page,*previous;
 TTF_Font *regular,*bold,*heavy,*small,*subhead,*heading;
 int width,height,cols,rows,cardW,cardH;
 int start; /* top-left card index shown on this output */
 Uint32 change_ms;
} Display;
static Display screens[2];
static ViewState state={PAGE_CATALOG,FILT_ALL,0,0,0,NULL};
static const char *cats[]={"Tutte","Storia","Equipaggiamento","Esplorazione",
                           "Collezioni","Secondarie","Crescita"};
static SDL_AudioDeviceID audio_device=0;
static uint32_t last_repeat=0;

static SDL_Texture *load_texture(Display *d,const uint8_t *start,const uint8_t *end){
 SDL_RWops *io=SDL_RWFromConstMem(start,(int)(end-start));
 if(!io)return NULL;
 SDL_Surface *surface=IMG_Load_RW(io,1);
 if(!surface)return NULL;
 SDL_Texture *tex=SDL_CreateTextureFromSurface(d->renderer,surface);
 SDL_FreeSurface(surface);
 if(tex){SDL_SetTextureBlendMode(tex,SDL_BLENDMODE_BLEND);SDL_SetTextureScaleMode(tex,SDL_ScaleModeLinear);}
 return tex;
}
static TTF_Font *load_font(const unsigned char *start,const unsigned char *end,int px){
 SDL_RWops *io=SDL_RWFromConstMem(start,(int)(end-start));
 return io?TTF_OpenFontRW(io,1,px):NULL;
}
static void rect(Display *d,int x,int y,int w,int h,SDL_Color color){
 SDL_SetRenderDrawColor(d->renderer,color.r,color.g,color.b,color.a);
 SDL_Rect r={x,y,w,h};SDL_RenderFillRect(d->renderer,&r);
}
static void line(Display *d,int x,int y,int xx,int yy,SDL_Color color){
 SDL_SetRenderDrawColor(d->renderer,color.r,color.g,color.b,color.a);
 SDL_RenderDrawLine(d->renderer,x,y,xx,yy);
}
static SDL_Color ink={52,58,64,255},muted={124,135,144,255},cyan={0,168,226,255};
static SDL_Color white={255,255,255,255},warn={235,146,20,255},green={82,180,46,255};
static SDL_Color gold={212,151,39,255};
static void draw_text(Display *d,TTF_Font *f,const char *s,int x,int y,int max_w,SDL_Color color){
 if(!f||!s||!*s||max_w<=0)return;
 SDL_Surface *surface=TTF_RenderUTF8_Blended(f,s,color);
 if(!surface)return;
 SDL_Texture *tx=SDL_CreateTextureFromSurface(d->renderer,surface);
 if(tx){
   SDL_Rect src={0,0,surface->w,surface->h};
   SDL_Rect dst={x,y,surface->w,surface->h};
   if(dst.w>max_w){src.w=max_w;dst.w=max_w;}
   if(dst.x+dst.w>d->width){src.w=d->width-dst.x;dst.w=src.w;}
   if(dst.w>0&&dst.y<d->height)SDL_RenderCopy(d->renderer,tx,&src,&dst);
   SDL_DestroyTexture(tx);
 }
 SDL_FreeSurface(surface);
}
static void draw_wrapped(Display *d,TTF_Font *font,const char *s,int x,int y,int max_w,int max_lines,int step,SDL_Color color){
 if(!s||!*s||!font)return;
 const char *p=s;
 for(int n=0;n<max_lines&&*p;n++){
   while(*p==' ')p++;
   if(!*p)break;
   char buffer[512];int best=0,used=0;
   const char *z=p;
   while(*z&&*z!='\n'&&used<450){
     const char *next=z;
     while(*next&&*next!=' '&&*next!='\n')next++;
     int nextlen=(int)(next-p);
     if(nextlen>=500)nextlen=499;
     memcpy(buffer,p,(size_t)nextlen);buffer[nextlen]=0;
     int w=0,h=0;TTF_SizeUTF8(font,buffer,&w,&h);
     if(w>max_w&&best>0)break;
     if(w>max_w&&best==0){
       for(int j=1;j<=nextlen;j++){
         buffer[j]=0;TTF_SizeUTF8(font,buffer,&w,&h);
         if(w>max_w){best=j-1;break;}
         buffer[j]=p[j];
       }
       if(!best)best=nextlen;
       break;
     }
     best=nextlen;
     z=next;
     while(*z==' ')z++;
     if(*z=='\n'||!*z)break;
     used=(int)(z-p);
   }
   if(best<=0)break;
   memcpy(buffer,p,(size_t)best);buffer[best]=0;
   draw_text(d,font,buffer,x,y,max_w,color);
   p+=best;
   while(*p==' ')p++;
   if(*p=='\n')p++;
   y+=step;
 }
}
static void copy_tex(Display *d,SDL_Texture *t,int x,int y,int w,int h){
 if(!t)return;
 SDL_Rect dst={x,y,w,h};
 SDL_RenderCopy(d->renderer,t,NULL,&dst);
}
static void draw_shell(Display *d){
 SDL_RenderCopy(d->renderer,d->shell,NULL,NULL);
}
static int available_count(void){int n=0;for(unsigned i=0;i<g_catalog_count;i++)n+=(g_catalog[i].executable!=0);return n;}
static const char *chapter_at(unsigned index){
 if(!index)return "Tutte le tappe";
 unsigned seen=0;
 for(unsigned i=0;i<g_catalog_count;i++){
   int duplicate=0;
   for(unsigned j=0;j<i;j++)
     if(strcmp(g_catalog[j].chapter,g_catalog[i].chapter)==0){duplicate=1;break;}
   if(!duplicate&&++seen==index)return g_catalog[i].chapter;
 }
 return NULL;
}
static int chapters_total(void){int n=1;while(chapter_at((unsigned)n))n++;return n;}
static bool matches(const AchievementEntry *e){
 if(state.filter==FILT_UNLOCKED)return false; /* Until real unlock store */
 if(state.filter==FILT_AVAILABLE&&!e->executable)return false;
 if(state.filter==FILT_PENDING&&e->executable)return false;
 if(state.category&&strcmp(e->category,cats[state.category]))return false;
 const char *ch=chapter_at(state.chapter);
 if(state.chapter&&ch&&strcmp(e->chapter,ch))return false;
 return true;
}
static int visible_count(void){int n=0;for(unsigned i=0;i<g_catalog_count;i++)n+=matches(&g_catalog[i]);return n;}
static const AchievementEntry *visible_at(int nth){
 for(unsigned i=0;i<g_catalog_count;i++)if(matches(&g_catalog[i])&&nth--==0)return &g_catalog[i];
 return NULL;
}
static void medal(Display *d,const AchievementEntry *e,int x,int y,int size){
 int tier=!strcmp(e->rarity,"gold")?2:!strcmp(e->rarity,"silver")?1:0;
 int kind=!strcmp(e->field,"max_health")?1:0;
 copy_tex(d,d->medals[tier][kind],x,y,size,size);
}
static void rounded_button(Display *d,const char *label,int x,int y,int w,SDL_Color fg,bool active){
 SDL_Color fill=active?(SDL_Color){255,255,255,255}:(SDL_Color){233,239,244,255};
 rect(d,x,y,w,d->height>500?29:23,fill);
 draw_text(d,d->small,label,x+8,y+2,w-14,fg);
}
static void page_header(Display *d){
 bool tv=d->width>1000;
 draw_text(d,d->bold,"Profilo locale",tv?74:48,tv?12:7,tv?180:125,ink);
 if(tv)draw_text(d,d->small,"Non collegato",74,34,150,muted);
 draw_text(d,d->heavy,"Wii U Achievements",tv?484:286,tv?8:6,tv?440:345,ink);
 draw_text(d,d->small,tv?"MONITOR NON ATTIVO":"NON ATTIVO",tv?967:652,tv?22:14,tv?153:104,warn);
 /* console coherent status, never the Windows Cemu button */
}
static void banner_text(Display *d){
 bool tv=d->width>1000;
 int x=tv?49:34,y=tv?89:60;
 draw_text(d,d->heavy,"The Wind Waker HD",x,y,tv?790:570,white);
 draw_text(d,d->small,"EUR v0  ·  Console Wii U",x,y+(tv?43:32),tv?480:360,white);
 draw_text(d,d->bold,"Rilevamento non ancora disponibile",x,y+(tv?69:56),tv?550:430,white);
 /* Real count instead of fake demo percentage */
 char buf[64];
 snprintf(buf,sizeof buf,"%u",g_catalog_count);
 draw_text(d,d->heavy,buf,tv?1112:752,tv?146:86,tv?115:95,ink);
 draw_text(d,d->small,"OBIETTIVI",tv?1091:729,tv?187:115,tv?170:125,cyan);
 char summary[64];
 snprintf(summary,sizeof buf,"%d regole Windows",available_count());
 draw_text(d,d->small,summary,tv?62:37,tv?226:145,tv?205:170,ink);
 snprintf(summary,sizeof buf,"%u da verificare",g_catalog_count-available_count());
 draw_text(d,d->small,summary,tv?269:208,tv?226:145,tv?180:154,ink);
}
static void tab(Display *d,const char *name,int count,int x,int y,int w,bool active,bool disabled){
 SDL_Color c=disabled?muted:(active?cyan:muted);
 int h=d->width>1000?31:24;
 if(active){
   SDL_SetRenderDrawColor(d->renderer,255,255,255,255);
   SDL_Rect b={x,y,w,h};SDL_RenderFillRect(d->renderer,&b);
 }
 char b[90];
 if(disabled)snprintf(b,sizeof b,"%s  --",name);
 else snprintf(b,sizeof b,"%s  %d",name,count);
 draw_text(d,active?d->bold:d->small,b,x+10,y+2,w-10,c);
}
static void catalog_tabs(Display *d){
 bool tv=d->width>1000;
 int y=tv?288:181;
 tab(d,"Tutti",(int)g_catalog_count,tv?22:15,y,tv?128:88,state.filter==FILT_ALL,false);
 tab(d,"Sbloccati",0,tv?157:106,y,tv?155:117,state.filter==FILT_UNLOCKED,true);
 tab(d,"Da sbloccare",available_count(),tv?318:229,y,tv?178:140,state.filter==FILT_AVAILABLE,false);
 tab(d,"Da verificare",(int)g_catalog_count-available_count(),tv?502:375,y,tv?190:146,state.filter==FILT_PENDING,false);
 char buf[180];
 snprintf(buf,sizeof buf,"Categoria: %s    ·    Tappa: %s",
           cats[state.category],chapter_at(state.chapter));
 draw_text(d,d->small,buf,tv?30:18,tv?327:215,tv?870:740,muted);
}
static void render_card(Display *d,const AchievementEntry *e,int x,int y,bool selected){
 int w=d->cardW,h=d->cardH;
 copy_tex(d,selected?d->card_focus:d->card,x-9,y-7,w+18,h+18);
 int medal_size=d->width>1000?81:68;
 medal(d,e,x+7,y+(h-medal_size)/2,medal_size);
 int tx=x+(d->width>1000?97:80),max=w-(tx-x)-14;
 draw_text(d,d->bold,e->title,tx,y+8,max-49,ink);
 draw_wrapped(d,d->small,e->description,tx,y+(d->width>1000?34:30),
              max,2,d->width>1000?17:15,muted);
 int ft=y+h-(d->width>1000?28:25);
 draw_text(d,d->small,!e->executable?"Non attivo":
           (e->experimental?"Regola sperimentale":"Da sbloccare · non monitorato"),
           tx,ft,max,!e->executable?warn:muted);
 if(e->executable){
   char points[24];snprintf(points,sizeof points,"+%d",e->points);
   draw_text(d,d->small,points,x+w-41,y+9,39,gold);
 }
}

static void draw_footer_buttons(Display *d,const char *a,const char *b,const char *x,const char *y){
 bool tv=d->width>1000;
 int py=d->height-(tv?51:40)+(tv?8:6);
 const char *actions[]={a,b,x,y};
 const char *keys[]={"A","B","X","Y"};
 const int xs[]={tv?20:16,tv?202:160,tv?364:294,tv?519:432};
 const int ws[]={tv?170:137,tv?149:126,tv?143:131,tv?191:155};
 for(int i=0;i<4;i++){
   if(!actions[i])continue;
   draw_text(d,d->small,keys[i],xs[i]+(tv?14:12),py+2,22,i?muted:cyan);
   draw_text(d,d->small,actions[i],xs[i]+(tv?40:34),py+2,ws[i]-(tv?46:40),i?ink:cyan);
 }
 if(d->width>1000)
    draw_text(d,d->small,"L/R  Tappa",898,py+2,300,muted);
 else draw_text(d,d->small,"L/R",660,py+2,150,muted);
}
static void draw_catalog(Display *d){
 banner_text(d);
 catalog_tabs(d);
 int count=visible_count();
 int start=(int)(state.selected/(unsigned)(d->cols*d->rows))*(d->cols*d->rows);
 int margin=d->width>1000?22:17;
 int gap=d->width>1000?13:11;
 int top=d->width>1000?353:237;
 int rowgap=d->width>1000?9:10;
 for(int i=0;i<d->cols*d->rows&&start+i<count;i++){
   const AchievementEntry *e=visible_at(start+i);
   if(!e)break;
   int x=margin+(i%d->cols)*(d->cardW+gap);
   int y=top+(i/d->cols)*(d->cardH+rowgap);
   render_card(d,e,x,y,(int)state.selected==start+i);
 }
 if(!count)draw_text(d,d->bold,"Nessun obiettivo in questa categoria.",
                      margin+35,top+30,d->width-2*margin-60,muted);
 draw_footer_buttons(d,"Dettagli","Giochi","Filtri","Categoria");
}
static void draw_games(Display *d){
 banner_text(d);
 int tv=d->width>1000;
 int x=tv?24:18,y=tv?311:218,w=d->width-x*2,h=tv?254:189;
 copy_tex(d,d->card_focus,x-9,y-7,w+18,h+18);
 draw_text(d,d->heavy,"The Wind Waker HD",x+(tv?180:128),y+27,w-270,ink);
 char info[130];snprintf(info,sizeof info,"%u obiettivi  ·  %d regole Windows  ·  %u da verificare",
        g_catalog_count,available_count(),g_catalog_count-available_count());
 draw_wrapped(d,d->regular,info,x+(tv?180:128),y+83,w-(tv?235:160),
             2,tv?24:19,muted);
 const AchievementEntry *ref=visible_at(0);
 if(ref)medal(d,ref,x+28,y+34,tv?112:84);
 draw_text(d,d->bold,"A  Apri obiettivi",x+(tv?180:128),y+h-66,290,cyan);
 draw_footer_buttons(d,"Apri","Esci","Info",NULL);
}
static void draw_details(Display *d){
 banner_text(d);
 if(!state.detail)return;
 const AchievementEntry *e=state.detail;
 int tv=d->width>1000,x=tv?88:24,y=tv?300:205,w=d->width-2*x;
 int h=tv?330:232;
 SDL_SetRenderDrawBlendMode(d->renderer,SDL_BLENDMODE_BLEND);
 rect(d,0,tv?279:180,d->width,d->height-(tv?279:180),(SDL_Color){34,50,65,80});
 copy_tex(d,d->card_focus,x-9,y-7,w+18,h+18);
 medal(d,e,x+17,y+13,tv?99:76);
 int tx=x+(tv?145:110);
 draw_text(d,d->heavy,e->title,tx,y+13,w-(tx-x)-28,ink);
 draw_text(d,d->regular,e->category,tx,y+(tv?69:52),w-165,muted);
 char meta[120];snprintf(meta,sizeof meta,"Medaglia %s  ·  %d punti  ·  %s",
           e->rarity,e->points,e->experimental?"Sperimentale":(e->executable?"Regola Windows":"Non attivo"));
 draw_text(d,d->small,meta,tx,y+(tv?102:78),w-(tx-x)-25,cyan);
 draw_text(d,d->bold,"Descrizione",x+35,y+(tv?160:123),w-50,ink);
 draw_wrapped(d,d->regular,e->description,x+35,y+(tv?194:149),
              w-65,tv?3:2,tv?25:20,muted);
 draw_footer_buttons(d,NULL,"Indietro",NULL,NULL);
}
static void draw_info(Display *d){
 draw_text(d,d->heavy,"Wii U Achievements",42,d->width>1000?315:218,d->width-70,ink);
 draw_text(d,d->regular,"Porting fedele dell'interfaccia Windows / Cemu.",42,d->width>1000?371:270,d->width-80,muted);
 draw_text(d,d->regular,"Catalogo originale: 110 obiettivi. Nessun dato di gioco letto.",
           42,d->width>1000?402:300,d->width-80,muted);
 draw_text(d,d->regular,"Il rilevamento e gli sblocchi in gioco non sono ancora disponibili.",
           42,d->width>1000?433:332,d->width-80,warn);
 draw_footer_buttons(d,NULL,"Indietro",NULL,NULL);
}
static void static_render(Display *d){
 SDL_SetRenderTarget(d->renderer,d->static_page);
 SDL_SetRenderDrawColor(d->renderer,250,251,252,255);SDL_RenderClear(d->renderer);
 draw_shell(d);
 page_header(d);
 switch(state.page){
   case PAGE_CATALOG:draw_catalog(d);break;
   case PAGE_GAMES:draw_games(d);break;
   case PAGE_DETAILS:draw_details(d);break;
   case PAGE_INFO:draw_info(d);break;
 }
 SDL_SetRenderTarget(d->renderer,NULL);
 d->change_ms=SDL_GetTicks();
}
static void frame(Display *d,Uint32 ticks){
 SDL_SetRenderTarget(d->renderer,NULL);
 SDL_SetRenderDrawColor(d->renderer,248,251,253,255);SDL_RenderClear(d->renderer);
 /* The scene is an accelerated render target, rather than millions of
    OSScreenPutPixelEx CPU writes. A short fade animates page changes. */
 SDL_RenderCopy(d->renderer,d->static_page,NULL,NULL);
 if(d->previous&&ticks-d->change_ms<200){
   float t=(ticks-d->change_ms)/200.0f;
   if(t<0)t=0;if(t>1)t=1;
   SDL_SetTextureBlendMode(d->previous,SDL_BLENDMODE_BLEND);
   SDL_SetTextureAlphaMod(d->previous,(Uint8)(255*(1-t)));
   SDL_RenderCopy(d->renderer,d->previous,NULL,NULL);
 }
 /* Tiny cyan waves animate in the original sea, without disturbing text. */
 SDL_SetRenderDrawBlendMode(d->renderer,SDL_BLENDMODE_BLEND);
 int horizon=d->width>1000?204:125;
 int x0=d->width>1000?22:16,xend=d->width-(d->width>1000?23:17);
 int limit=d->width>1000?252:166;
 for(int row=0;row<3;row++){
   for(int x=x0+5;x<xend-8;x+=5){
     int y=horizon+row*(d->width>1000?16:9)+
       (int)(2.0f*sin((float)x*.028f+ticks*.0017f+row*1.6f));
     if(y<limit)line(d,x,y,x+5,y,(SDL_Color){196,238,255,(Uint8)(70-row*13)});
   }
 }
 SDL_RenderPresent(d->renderer);
}
static void sound(int success){
 if(!audio_device)return;
 int count=success?2400:1250;
 static Sint16 buf[4800];
 for(int i=0;i<count;i++){
   float t=(float)i/48000.0f;
   float env=(1.0f-(float)i/count);
   float f=success?1046.5f:783.9f;
   float tone=sinf(t*6.2831853f*f)+.22f*sinf(t*6.2831853f*f*2.0f);
   Sint16 v=(Sint16)(env*1300*tone);
   buf[i*2]=v;buf[i*2+1]=v;
 }
 SDL_ClearQueuedAudio(audio_device);
 SDL_QueueAudio(audio_device,buf,(Uint32)(count*2*sizeof(Sint16)));
 SDL_PauseAudioDevice(audio_device,0);
}
static void move(int direction){
 int n=visible_count();if(n<=0){state.selected=0;return;}
 int cols=2; /* GamePad primary focus: two-column spatial grid. */
 int i=(int)state.selected;
 if(direction==-1&&i%cols>0)i--;
 if(direction==1&&i%cols<cols-1&&i+1<n)i++;
 if(direction==-2&&i>=cols)i-=cols;
 if(direction==2&&i+cols<n)i+=cols;
 state.selected=(unsigned)i;
}
static bool input(uint32_t p,bool *quit){
 ViewState before=state;
 if(p&VPAD_BUTTON_B){
   if(state.page==PAGE_CATALOG)state.page=PAGE_GAMES;
   else if(state.page==PAGE_GAMES)*quit=true;
   else if(state.page==PAGE_DETAILS)state.page=PAGE_CATALOG;
   else state.page=PAGE_GAMES;
 }else if(state.page==PAGE_GAMES){
   if(p&VPAD_BUTTON_A)state.page=PAGE_CATALOG;
   else if(p&VPAD_BUTTON_X)state.page=PAGE_INFO;
 }else if(state.page==PAGE_CATALOG){
   if(p&VPAD_BUTTON_LEFT)move(-1);
   else if(p&VPAD_BUTTON_RIGHT)move(1);
   else if(p&VPAD_BUTTON_UP)move(-2);
   else if(p&VPAD_BUTTON_DOWN)move(2);
   else if(p&VPAD_BUTTON_X){
     state.filter=(Filter)((state.filter+1)%4);
     if(state.filter==FILT_UNLOCKED)state.filter=FILT_AVAILABLE; /* Not yet available */
     state.selected=0;
   }else if(p&VPAD_BUTTON_Y){state.category=(state.category+1)%7;state.selected=0;}
   else if(p&VPAD_BUTTON_L){
     int n=chapters_total();state.chapter=(state.chapter+n-1)%n;state.selected=0;
   }else if(p&VPAD_BUTTON_R){
     state.chapter=(state.chapter+1)%chapters_total();state.selected=0;
   }else if(p&VPAD_BUTTON_A){
     state.detail=visible_at((int)state.selected);
     if(state.detail)state.page=PAGE_DETAILS;
   }
 }
 return before.page!=state.page||before.filter!=state.filter||
        before.category!=state.category||before.chapter!=state.chapter||
        before.selected!=state.selected||before.detail!=state.detail;
}
static int load_display(Display *d,int index){
 bool tv=index==0;
 memset(d,0,sizeof *d);
 d->width=tv?1280:854;d->height=tv?720:480;
 d->cols=tv?3:2;d->rows=tv?3:2;
 d->cardW=tv?400:399;d->cardH=tv?98:91;
 d->window=SDL_CreateWindow("Wii U Achievements",
    SDL_WINDOWPOS_CENTERED_DISPLAY(index),SDL_WINDOWPOS_CENTERED_DISPLAY(index),
    d->width,d->height,SDL_WINDOW_FULLSCREEN);
 if(!d->window)return -1;
 d->renderer=SDL_CreateRenderer(d->window,-1,
     SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_TARGETTEXTURE);
 if(!d->renderer)return -2;
 SDL_SetRenderDrawBlendMode(d->renderer,SDL_BLENDMODE_BLEND);
 d->shell=load_texture(d,tv?asset_shell_tv_start:asset_shell_drc_start,
                          tv?asset_shell_tv_end:asset_shell_drc_end);
 d->card=load_texture(d,tv?asset_card_tv_start:asset_card_drc_start,
                         tv?asset_card_tv_end:asset_card_drc_end);
 d->card_focus=load_texture(d,tv?asset_card_tv_focus_start:asset_card_drc_focus_start,
                               tv?asset_card_tv_focus_end:asset_card_drc_focus_end);
#define LOAD_M(tier,kind) d->medals[tier][kind]=load_texture(d, \
  asset_medal_##tier##_##kind##_start,asset_medal_##tier##_##kind##_end)
 /* literal arrays required for static symbols */
 d->medals[0][0]=load_texture(d,asset_medal_bronze_rupee_start,asset_medal_bronze_rupee_end);
 d->medals[0][1]=load_texture(d,asset_medal_bronze_heart_start,asset_medal_bronze_heart_end);
 d->medals[1][0]=load_texture(d,asset_medal_silver_rupee_start,asset_medal_silver_rupee_end);
 d->medals[1][1]=load_texture(d,asset_medal_silver_heart_start,asset_medal_silver_heart_end);
 d->medals[2][0]=load_texture(d,asset_medal_gold_rupee_start,asset_medal_gold_rupee_end);
 d->medals[2][1]=load_texture(d,asset_medal_gold_heart_start,asset_medal_gold_heart_end);
 d->regular=load_font(asset_nunito_regular_start,asset_nunito_regular_end,tv?16:13);
 d->small=load_font(asset_nunito_regular_start,asset_nunito_regular_end,tv?13:11);
 d->bold=load_font(asset_nunito_bold_start,asset_nunito_bold_end,tv?18:14);
 d->heavy=load_font(asset_nunito_extrabold_start,asset_nunito_extrabold_end,tv?32:22);
 d->subhead=load_font(asset_nunito_bold_start,asset_nunito_bold_end,tv?21:16);
 d->heading=load_font(asset_nunito_extrabold_start,asset_nunito_extrabold_end,tv?34:25);
 if(!d->shell||!d->card||!d->card_focus||!d->regular||!d->small||!d->bold||!d->heavy)return -3;
 d->static_page=SDL_CreateTexture(d->renderer,SDL_PIXELFORMAT_RGBA8888,
                                  SDL_TEXTUREACCESS_TARGET,d->width,d->height);
 d->previous=SDL_CreateTexture(d->renderer,SDL_PIXELFORMAT_RGBA8888,
                                  SDL_TEXTUREACCESS_TARGET,d->width,d->height);
 if(!d->static_page||!d->previous)return -4;
 static_render(d);
 d->change_ms=SDL_GetTicks()-200;
 return 0;
}
static void stop_display(Display *d){
 if(d->regular)TTF_CloseFont(d->regular);
 if(d->small)TTF_CloseFont(d->small);
 if(d->bold)TTF_CloseFont(d->bold);
 if(d->heavy)TTF_CloseFont(d->heavy);
 if(d->subhead)TTF_CloseFont(d->subhead);
 if(d->heading)TTF_CloseFont(d->heading);
 for(int tier=0;tier<3;tier++)for(int k=0;k<2;k++)if(d->medals[tier][k])SDL_DestroyTexture(d->medals[tier][k]);
 if(d->shell)SDL_DestroyTexture(d->shell);
 if(d->card)SDL_DestroyTexture(d->card);
 if(d->card_focus)SDL_DestroyTexture(d->card_focus);
 if(d->static_page)SDL_DestroyTexture(d->static_page);
 if(d->previous)SDL_DestroyTexture(d->previous);
 if(d->renderer)SDL_DestroyRenderer(d->renderer);
 if(d->window)SDL_DestroyWindow(d->window);
}
static void change_pages(void){
 for(int i=0;i<2;i++){
   Display *d=&screens[i];
   SDL_SetRenderTarget(d->renderer,d->previous);
   SDL_RenderCopy(d->renderer,d->static_page,NULL,NULL);
   SDL_SetRenderTarget(d->renderer,NULL);
   static_render(d);
 }
}
int main(int argc,char **argv){
 (void)argc;(void)argv;
 if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER)<0)return 1;
 if(!(IMG_Init(IMG_INIT_PNG)&IMG_INIT_PNG)){SDL_Quit();return 2;}
 if(TTF_Init()<0){IMG_Quit();SDL_Quit();return 3;}
 int count=SDL_GetNumVideoDisplays();
 if(count<2){TTF_Quit();IMG_Quit();SDL_Quit();return 4;}
 int ready=0;
 for(int i=0;i<2;i++){
   if(load_display(&screens[i],i)!=0)goto cleanup;
   ready++;
 }
 {
   SDL_AudioSpec desired={0},obtained={0};
   desired.freq=48000;desired.format=AUDIO_S16SYS;desired.channels=2;desired.samples=1024;
   audio_device=SDL_OpenAudioDevice(NULL,0,&desired,&obtained,0);
 }
 bool quit=false;
 while(!quit){
   SDL_Event event;
   while(SDL_PollEvent(&event))if(event.type==SDL_QUIT)quit=true;
   VPADStatus v={0};VPADReadError err;
   VPADRead(VPAD_CHAN_0,&v,1,&err);
   if(err==VPAD_READ_SUCCESS){
     uint32_t p=v.trigger;
     Uint32 now=SDL_GetTicks();
     if(!p&&(v.hold&(VPAD_BUTTON_UP|VPAD_BUTTON_DOWN|VPAD_BUTTON_LEFT|VPAD_BUTTON_RIGHT))
         &&now-last_repeat>165){
       p=v.hold&(VPAD_BUTTON_UP|VPAD_BUTTON_DOWN|VPAD_BUTTON_LEFT|VPAD_BUTTON_RIGHT);
       last_repeat=now;
     } else if(p)last_repeat=now;
     if(p&&input(p,&quit)){change_pages();sound(1);}
   }
   Uint32 tick=SDL_GetTicks();
   for(int i=0;i<ready;i++)frame(&screens[i],tick);
   SDL_Delay(16);
 }
cleanup:
 if(audio_device)SDL_CloseAudioDevice(audio_device);
 for(int i=0;i<2;i++)stop_display(&screens[i]);
 TTF_Quit();IMG_Quit();SDL_Quit();
 return ready==2?0:5;
}
