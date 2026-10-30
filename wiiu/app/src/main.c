/*
 * Wii U Achievements — native visual port of the original application.
 * Dynamic Wii U TV (1280x720) and GamePad (854x480) layouts.
 * Palette/typography from Theme.cs; ocean from Banner.cs; original XML catalog.
 *
 * This is a READ-ONLY browser. No unlock state is fabricated or persisted;
 * no other game process is accessed; WUPS is intentionally not installed.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#ifndef UI_HOST_PREVIEW
#include <coreinit/cache.h>
#include <coreinit/screen.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <vpad/input.h>
#include <whb/proc.h>
#endif
#include "catalog.h"
#include "ui_canvas.h"

enum { PAGE_GAMES, PAGE_ACHIEVEMENTS, PAGE_DETAILS, PAGE_INFO };
enum { FILTER_ALL, FILTER_PC_RULES, FILTER_PENDING, FILTER_COUNT };
static const char *const cats[] = {
    "Tutte", "Storia", "Equipaggiamento", "Esplorazione",
    "Collezioni", "Secondarie", "Crescita"
};
#define CAT_COUNT (sizeof(cats)/sizeof(cats[0]))

static unsigned page = PAGE_ACHIEVEMENTS, filter = FILTER_ALL;
static unsigned category = 0, chapter = 0, selected = 0;
static const AchievementEntry *detail = NULL;

static bool is_tv(const UiCanvas *c) { return c->screen == SCREEN_TV; }
static int font_small(const UiCanvas *c) { return is_tv(c) ? NUNITO_REGULAR_16 : NUNITO_REGULAR_13; }
static int font_bold(const UiCanvas *c) { return is_tv(c) ? NUNITO_BOLD_21 : NUNITO_BOLD_16; }
static int font_title(const UiCanvas *c) { return is_tv(c) ? NUNITO_EXTRABOLD_33 : NUNITO_EXTRABOLD_25; }
static int col_count(const UiCanvas *c) { return is_tv(c) ? 3 : 2; }
static int row_count(const UiCanvas *c) { return is_tv(c) ? 3 : 2; }
static int cards_per_page(const UiCanvas *c) { return col_count(c) * row_count(c); }

static unsigned active_count(void){
    unsigned n=0;for(unsigned i=0;i<g_catalog_count;++i)n+=(unsigned)(g_catalog[i].executable!=0);
    return n;
}
static const char *chapter_name(unsigned index){
    if(!index)return "Tutte le tappe";
    unsigned count=0;
    for(unsigned i=0;i<g_catalog_count;i++){
        bool seen=false;
        for(unsigned j=0;j<i;j++)
            if(strcmp(g_catalog[i].chapter,g_catalog[j].chapter)==0){seen=true;break;}
        if(!seen&&++count==index)return g_catalog[i].chapter;
    }
    return NULL;
}
static unsigned chapter_count(void){
    unsigned n=1;
    while(chapter_name(n)!=NULL)n++;
    return n;
}
static bool matches(const AchievementEntry *e){
    return !(filter==FILTER_PC_RULES&&!e->executable)
        && !(filter==FILTER_PENDING&&e->executable)
        && !(category&&strcmp(e->category,cats[category])!=0)
        && !(chapter&&strcmp(e->chapter,chapter_name(chapter))!=0);
}
static unsigned filtered_count(void){
    unsigned n=0;
    for(unsigned i=0;i<g_catalog_count;i++)if(matches(&g_catalog[i]))n++;
    return n;
}
static const AchievementEntry *filtered_at(unsigned idx){
    for(unsigned i=0;i<g_catalog_count;i++)
        if(matches(&g_catalog[i])&&idx--==0)return &g_catalog[i];
    return NULL;
}
static void text(UiCanvas *c,int x,int y,const char *s,int font,
                 uint32_t color,uint32_t bg,int width){
    ui_text(c,x,y,s,font,color,bg,width);
}
static void pill(UiCanvas *c,int x,int y,int w,int h,
                 const char *label,int active,uint32_t bg){
    uint32_t fill=active?UI_ACCENT:UI_WHITE;
    ui_round(c,x,y+2,w,h,12,active?0x87CDE600u:UI_LINE);
    ui_round(c,x,y,w,h,12,fill);
    text(c,x+12,y+(h-(is_tv(c)?16:13))/2,label,font_small(c),
         active?UI_WHITE:UI_MUTED,fill,w-20);
    (void)bg;
}
static void header(UiCanvas *c){
    int h=is_tv(c)?62:43,w=c->width;
    ui_fill(c,0,0,w,h,UI_WHITE);
    ui_fill(c,0,h-2,w,2,UI_LINE);
    ui_circle(c,is_tv(c)?34:25,h/2, is_tv(c)?19:15,UI_ACCENT_SOFT);
    ui_circle(c,is_tv(c)?34:25,h/2, is_tv(c)?14:11,UI_ACCENT);
    text(c,is_tv(c)?27:19,h/2-(is_tv(c)?8:7),"★",font_small(c),
         UI_WHITE,UI_ACCENT,26);
    text(c,is_tv(c)?58:46,h/2-(is_tv(c)?15:11),"Profilo locale",
         font_bold(c),UI_INK,UI_WHITE,is_tv(c)?160:110);
    if(is_tv(c))text(c,58,h/2+6,"Non collegato",font_small(c),UI_MUTED,UI_WHITE,180);

    const char *name="Wii U Achievements";
    int font=font_title(c),tw=ui_text_width(name,font);
    text(c,(w-tw)/2,h/2-(is_tv(c)?19:14),name,font,UI_INK,UI_WHITE,tw+4);
    const char *state=is_tv(c)?"MONITOR NON ATTIVO":"SOLO CATALOGO";
    int statew=is_tv(c)?197:126;
    ui_round(c,w-statew-18,h/2-(is_tv(c)?17:13),statew,is_tv(c)?33:26,
             14,0xF7EDE000u);
    text(c,w-statew-8,h/2-(is_tv(c)?7:6),state,font_small(c),
         0x9C6E3500u,0xF7EDE000u,statew-16);
}
static void footer(UiCanvas *c,const char *left){
    int y=c->height-(is_tv(c)?56:37);
    ui_fill(c,0,y,c->width,c->height-y,UI_WHITE);
    ui_fill(c,0,y,c->width,2,UI_LINE);
    ui_circle(c,is_tv(c)?32:22,y+(is_tv(c)?25:18),is_tv(c)?13:10,UI_ACCENT_SOFT);
    text(c,is_tv(c)?26:17,y+(is_tv(c)?16:11),"A",font_small(c),UI_ACCENT_DARK,UI_ACCENT_SOFT,20);
    text(c,is_tv(c)?52:39,y+(is_tv(c)?17:12),left,font_small(c),
         UI_MUTED,UI_WHITE,c->width-(is_tv(c)?100:80));
}
static void banner(UiCanvas *c,int compact){
    bool tv=is_tv(c);
    int x=tv?22:16,y=tv?77:55,w=c->width-2*x,h=tv?(compact?159:188):(compact?109:123);
    ui_ocean_banner(c,x,y,w,h);
    int bx=x+(tv?30:20),ty=y+(tv?21:13);
    text(c,bx,ty,"The Wind Waker HD",font_title(c),UI_WHITE,0x63C5F200u,w-(tv?285:155));
    text(c,bx,ty+(tv?37:27),"EUR v0  ·  Wii U nativa",font_small(c),
         UI_WHITE,0x63C5F200u,300);
    int status_y=y+h*56/100-(tv?7:6);
    int status_w=tv?339:256,status_h=tv?28:21;
    uint32_t status_bg=0x10658F00u;
    ui_round(c,bx,status_y,status_w,status_h,12,status_bg);
    ui_circle(c,bx+(tv?12:10),status_y+status_h/2,tv?5:4,UI_WARNING);
    text(c,bx+(tv?23:19),status_y+(tv?4:3),
         "Monitoraggio non disponibile",font_small(c),
         UI_WHITE,status_bg,status_w-27);

    int ringr=tv?74:46,ringx=x+w-(tv?115:76),ringy=y+h/2-(tv?2:0);
    ui_circle(c,ringx,ringy+4,ringr+5,0x2379A700u);
    ui_circle(c,ringx,ringy,ringr+3,UI_WHITE);
    ui_circle(c,ringx,ringy,ringr-6,0xDCE6EB00u);
    ui_circle(c,ringx,ringy,ringr-12,UI_WHITE);
    char num[18];snprintf(num,sizeof num,"%u",g_catalog_count);
    int nf=font_title(c),numw=ui_text_width(num,nf);
    text(c,ringx-numw/2,ringy-(tv?26:21),num,nf,UI_INK,UI_WHITE,numw+3);
    const char *label="OBIETTIVI";
    int labelw=ui_text_width(label,font_small(c));
    text(c,ringx-labelw/2,ringy+(tv?18:14),label,font_small(c),UI_ACCENT_DARK,UI_WHITE,labelw+4);
    if(!compact) {
        int cy=y+h-(tv?39:30),px=bx;
        char first[40];snprintf(first,sizeof first,"%u regole PC",active_count());
        pill(c,px,cy,tv?153:127,tv?26:21,first,0,UI_WHITE);
        px+=tv?166:136;
        char second[45];snprintf(second,sizeof second,"%u da verificare",
              g_catalog_count-active_count());
        pill(c,px,cy,tv?173:143,tv?26:21,second,0,UI_WHITE);
        px+=tv?185:150;
        if(tv)pill(c,px,cy,170,26,"Dati non disponibili",0,UI_WHITE);
    }
}
static void game_card(UiCanvas *c){
    bool tv=is_tv(c);int x=tv?22:16,w=c->width-2*x;
    int y=tv?295:202,h=tv?300:222;
    text(c,x+8,y-26,"Giochi",font_bold(c),UI_INK,UI_BG,140);
    ui_round(c,x,y+6,w,h,18,0xC9D6DE00u);
    ui_round(c,x,y,w,h,18,UI_ACCENT);
    ui_round(c,x+3,y+3,w-6,h-6,16,UI_WHITE);
    ui_medal(c,x+(tv?117:80),y+h/2,tv?67:47,"gold","rupees",0);
    int tx=x+(tv?227:151);
    text(c,tx,y+(tv?45:35),"The Legend of Zelda:",font_bold(c),UI_INK,UI_WHITE,w-(tx-x)-25);
    text(c,tx,y+(tv?79:66),"The Wind Waker HD",font_title(c),UI_INK,UI_WHITE,w-(tx-x)-25);
    char stats[110];snprintf(stats,sizeof stats,
        "%u obiettivi originali  ·  %u regole PC  ·  %u da verificare",
        g_catalog_count,active_count(),g_catalog_count-active_count());
    ui_wrap(c,tx,y+(tv?140:110),stats,font_small(c),w-(tx-x)-40,2,tv?22:18,UI_MUTED,UI_WHITE);
    ui_round(c,tx,y+h-(tv?65:58),tv?290:206,tv?40:32,16,UI_ACCENT);
    text(c,tx+17,y+h-(tv?55:49),"A  Apri obiettivi",font_bold(c),UI_WHITE,UI_ACCENT,tv?254:182);
    footer(c,"Apri il catalogo   ·   X informazioni   ·   B esci");
}
static void filters(UiCanvas *c){
    int y=is_tv(c)?248:186,x=is_tv(c)?22:17;
    unsigned all=filtered_count();
    (void)all;
    pill(c,x,y,is_tv(c)?127:92,is_tv(c)?35:29,"Tutti",filter==FILTER_ALL,UI_WHITE);
    pill(c,x+(is_tv(c)?138:100),y,is_tv(c)?164:132,is_tv(c)?35:29,
         "Regole PC",filter==FILTER_PC_RULES,UI_WHITE);
    pill(c,x+(is_tv(c)?312:239),y,is_tv(c)?180:139,is_tv(c)?35:29,
         "Da verificare",filter==FILTER_PENDING,UI_WHITE);
    char summary[80];
    snprintf(summary,sizeof summary,"%u obiettivi in questa vista",filtered_count());
    if(is_tv(c))text(c,c->width-280,y+9,summary,font_small(c),UI_MUTED,UI_BG,250);
    else {
        char compact[24];snprintf(compact,sizeof compact,"%u / %u",filtered_count(),g_catalog_count);
        text(c,c->width-86,y+9,compact,font_small(c),UI_MUTED,UI_BG,75);
    }
    int fy=is_tv(c)?286:222;
    char section[100];
    snprintf(section,sizeof section,"Categoria: %s    |    Tappa: %s",
        cats[category],chapter_name(chapter));
    text(c,x+5,fy,section,font_small(c),UI_ACCENT_DARK,UI_BG,c->width-2*x-8);
}
static void card(UiCanvas *c,int x,int y,int w,int h,const AchievementEntry *a,bool focus){
    bool tv=is_tv(c);
    ui_round(c,x,y+5,w,h,13,0xC8D5DE00u);
    ui_round(c,x,y,w,h,13,focus?UI_ACCENT:UI_LINE);
    ui_round(c,x+(focus?3:1),y+(focus?3:1),w-(focus?6:2),h-(focus?6:2),11,UI_WHITE);
    int radius=tv?38:30;
    int mx=x+(tv?55:42),my=y+h/2-1;
    ui_medal(c,mx,my,radius,a->rarity,a->field,!a->executable);
    int tx=x+(tv?107:83),textmax=w-(tx-x)-12;
    text(c,tx,y+(tv?10:9),a->title,font_bold(c),UI_INK,UI_WHITE,textmax-(tv?62:49));
    int description_y=y+(tv?41:33);
    ui_wrap(c,tx,description_y,a->description,NUNITO_REGULAR_13,textmax,2,
            tv?18:17,UI_MUTED,UI_WHITE);
    int bottom=y+h-(tv?27:21);
    if(!a->executable) {
        ui_round(c,tx,bottom-3,tv?118:96,tv?22:18,9,0xFFF0DE00u);
        text(c,tx+7,bottom+1,"NON ATTIVO",font_small(c),UI_WARNING,0xFFF0DE00u,tv?103:85);
    }else {
        text(c,tx,bottom,"Stato non disponibile",
             NUNITO_REGULAR_13,UI_MUTED,UI_WHITE,textmax-4);
    }
    if(a->executable){
        char points[30];snprintf(points,sizeof points,"+%d",a->points);
        int pwidth=tv?53:39;
        ui_round(c,x+w-pwidth-11,y+11,pwidth,tv?24:20,12,
                 strcmp(a->rarity,"gold")==0?0xF4DC9C00u:
                 strcmp(a->rarity,"silver")==0?0xE0E6EB00u:0xE7C7AB00u);
        text(c,x+w-pwidth-2,y+(tv?16:14),points,font_small(c),UI_INK,
             strcmp(a->rarity,"gold")==0?0xF4DC9C00u:
             strcmp(a->rarity,"silver")==0?0xE0E6EB00u:0xE7C7AB00u,pwidth-5);
    }
}
static void catalog(UiCanvas *c){
    bool tv=is_tv(c);
    banner(c,1);
    filters(c);
    unsigned count=filtered_count();
    int cols=col_count(c),rows=row_count(c),page_size=cards_per_page(c);
    int x0=tv?22:17,gap=tv?13:11,top=tv?311:243;
    int w=(c->width-x0*2-(cols-1)*gap)/cols;
    int h=tv?109:91,vgap=tv?8:10;
    int start=(int)(selected/page_size*page_size);
    if(count==0){
        ui_round(c,x0,top,c->width-2*x0,tv?277:189,18,UI_WHITE);
        text(c,x0+25,top+32,"Nessun obiettivo con questi filtri.",
             font_bold(c),UI_MUTED,UI_WHITE,c->width-2*x0-50);
    }
    for(int i=0;i<cols*rows&&start+i<(int)count;i++){
        int xx=x0+i%cols*(w+gap),yy=top+i/cols*(h+vgap);
        const AchievementEntry *e=filtered_at((unsigned)(start+i));
        if(e)card(c,xx,yy,w,h,e,selected==(unsigned)(start+i));
    }
    footer(c,tv?"Frecce: seleziona (GamePad)  ·  A dettagli  ·  B giochi  ·  X stato  ·  Y categoria  ·  L/R tappa":
             "Frecce: naviga   A: dettagli   B: giochi   X/Y: filtri   L/R: tappa");
}
static void details(UiCanvas *c){
    banner(c,1);
    if(!detail)return;
    bool tv=is_tv(c);int x=tv?52:20,y=tv?278:182,w=c->width-2*x,h=tv?367:254;
    ui_round(c,x,y+5,w,h,18,0xCAD8E200u);
    ui_round(c,x,y,w,h,18,UI_WHITE);
    ui_medal(c,x+(tv?96:65),y+(tv?85:63),tv?58:39,
             detail->rarity,detail->field,!detail->executable);
    int tx=x+(tv?180:123),avail=w-(tx-x)-30;
    text(c,tx,y+(tv?29:21),detail->title,font_title(c),UI_INK,UI_WHITE,avail);
    char sub[150];
    snprintf(sub,sizeof sub,"%s  ·  %s",
             detail->category,detail->executable?
             (detail->experimental?"Regola sperimentale Windows":"Regola presente su Windows"):"Non attivo");
    text(c,tx,y+(tv?86:64),sub,font_small(c),UI_MUTED,UI_WHITE,avail);
    char points[50];snprintf(points,sizeof points,"Medaglia %s  ·  %d punti",
                             detail->rarity,detail->points);
    text(c,tx,y+(tv?118:87),points,font_small(c),UI_ACCENT_DARK,UI_WHITE,avail);
    ui_fill(c,x+28,y+(tv?176:120),w-56,2,UI_LINE);
    text(c,x+32,y+(tv?194:134),"Descrizione",font_bold(c),UI_INK,UI_WHITE,w-64);
    int next=ui_wrap(c,x+32,y+(tv?229:157),detail->description,font_small(c),
                  w-66,tv?3:2,tv?23:18,UI_MUTED,UI_WHITE);
    if(!detail->executable&&next+40<y+h){
        text(c,x+32,next+8,"Da verificare:",font_bold(c),UI_WARNING,UI_WHITE,w-68);
        ui_wrap(c,x+32,next+(tv?35:29),detail->requirement,font_small(c),
                w-66,2,tv?22:18,UI_MUTED,UI_WHITE);
    }else if(detail->executable){
        text(c,x+32,y+h-(tv?43:28),"Sblocchi su Wii U non ancora attivi.",
             font_small(c),UI_WARNING,UI_WHITE,w-66);
    }
    footer(c,"B: torna al catalogo degli obiettivi");
}
static void info(UiCanvas *c){
    bool tv=is_tv(c),x=tv?45:23,y=tv?93:64;
    int w=c->width-2*x,h=c->height-y-(tv?86:55);
    ui_round(c,x,y,w,h,18,UI_WHITE);
    text(c,x+35,y+30,"Informazioni",font_title(c),UI_INK,UI_WHITE,w-70);
    ui_fill(c,x+30,y+(tv?89:65),w-60,2,UI_ACCENT_SOFT);
    char line[128];
    snprintf(line,sizeof line,"The Wind Waker HD  ·  %u obiettivi",g_catalog_count);
    text(c,x+35,y+(tv?112:87),line,font_bold(c),UI_INK,UI_WHITE,w-70);
    snprintf(line,sizeof line,"%u regole PC  ·  %u da verificare",
             active_count(),g_catalog_count-active_count());
    text(c,x+35,y+(tv?154:116),line,font_small(c),UI_MUTED,UI_WHITE,w-70);
    text(c,x+35,y+(tv?202:150),"Catalogo generato dagli XML originali.",
         font_small(c),UI_MUTED,UI_WHITE,w-70);
    text(c,x+35,y+(tv?243:178),"Nessun dato di gioco o salvataggio viene letto.",
         font_small(c),UI_WARNING,UI_WHITE,w-70);
    text(c,x+35,y+(tv?284:207),"Notifiche e progressi ancora in sviluppo.",
         font_small(c),UI_MUTED,UI_WHITE,w-70);
    footer(c,"B: torna ai giochi");
}
static void render(OSScreenID screen){
    UiCanvas c;ui_canvas_init(&c,screen);ui_background(&c);header(&c);
    if(page==PAGE_GAMES){banner(&c,0);game_card(&c);}
    else if(page==PAGE_ACHIEVEMENTS)catalog(&c);
    else if(page==PAGE_DETAILS)details(&c);
    else info(&c);
}
#ifdef UI_HOST_PREVIEW
int main(int argc,char **argv){
    if(argc!=4){fprintf(stderr,"usage: preview <TV|DRC> <games|catalog|details|info> <out.ppm>\n");return 1;}
    OSScreenID screen=strcmp(argv[1],"TV")==0?SCREEN_TV:SCREEN_DRC;
    if(strcmp(argv[2],"catalog")==0)page=PAGE_ACHIEVEMENTS;
    else if(strcmp(argv[2],"details")==0){page=PAGE_DETAILS;detail=filtered_at(0);}
    else if(strcmp(argv[2],"info")==0)page=PAGE_INFO;
    else if(strcmp(argv[2],"games")!=0)return 1;
    render(screen);
    return host_save_png_inputs(argv[3],screen);
}
#else
static void repaint(void *tv,uint32_t ts,void *drc,uint32_t ds){
    for(int i=0;i<2;i++){
        render(SCREEN_TV);render(SCREEN_DRC);
        DCFlushRange(tv,ts);DCFlushRange(drc,ds);
        OSScreenFlipBuffersEx(SCREEN_TV);OSScreenFlipBuffersEx(SCREEN_DRC);
    }
}
static bool input(uint32_t pressed,bool *quit){
    unsigned old_page=page,old_filter=filter,old_category=category;
    unsigned old_chapter=chapter,old_sel=selected;
    if(pressed&VPAD_BUTTON_B){
        if(page==PAGE_GAMES)*quit=true;
        else if(page==PAGE_DETAILS)page=PAGE_ACHIEVEMENTS;
        else page=PAGE_GAMES;
    }else if(page==PAGE_GAMES){
        if(pressed&VPAD_BUTTON_A)page=PAGE_ACHIEVEMENTS;
        else if(pressed&VPAD_BUTTON_X)page=PAGE_INFO;
    }else if(page==PAGE_ACHIEVEMENTS){
        unsigned count=filtered_count();
        /* Primary navigation is the GamePad 2-column grid. */
        if(pressed&VPAD_BUTTON_LEFT){if(selected%2==1)selected--;}
        else if(pressed&VPAD_BUTTON_RIGHT){if(selected%2==0&&selected+1<count)selected++;}
        else if(pressed&VPAD_BUTTON_UP){if(selected>=2)selected-=2;}
        else if(pressed&VPAD_BUTTON_DOWN){if(selected+2<count)selected+=2;}
        else if(pressed&VPAD_BUTTON_X){filter=(filter+1)%FILTER_COUNT;selected=0;}
        else if(pressed&VPAD_BUTTON_Y){category=(category+1)%CAT_COUNT;selected=0;}
        else if(pressed&VPAD_BUTTON_L){
            unsigned n=chapter_count();chapter=(chapter+n-1)%n;selected=0;
        }else if(pressed&VPAD_BUTTON_R){
            chapter=(chapter+1)%chapter_count();selected=0;
        }else if(pressed&VPAD_BUTTON_A){
            if(count&&selected<count){
                detail=filtered_at(selected);
                if(detail)page=PAGE_DETAILS;
            }
        }
    }
    return page!=old_page||filter!=old_filter||category!=old_category||
           chapter!=old_chapter||selected!=old_sel;
}
int main(int argc,char **argv){
    (void)argc;(void)argv;
    WHBProcInit();OSScreenInit();
    uint32_t ts=OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t ds=OSScreenGetBufferSizeEx(SCREEN_DRC);
    void *tv=memalign(0x100,ts),*drc=memalign(0x100,ds);
    if(!tv||!drc){
        free(drc);free(tv);OSScreenShutdown();WHBProcShutdown();return 1;
    }
    OSScreenSetBufferEx(SCREEN_TV,tv);
    OSScreenSetBufferEx(SCREEN_DRC,drc);
    OSScreenEnableEx(SCREEN_TV,true);
    OSScreenEnableEx(SCREEN_DRC,true);
    repaint(tv,ts,drc,ds);
    bool quit=false;
    while(WHBProcIsRunning()&&!quit){
        VPADStatus status={0};VPADReadError error;
        VPADRead(VPAD_CHAN_0,&status,1,&error);
        if(error==VPAD_READ_SUCCESS&&input(status.trigger,&quit))
            repaint(tv,ts,drc,ds);
        OSSleepTicks(OSMillisecondsToTicks(33));
    }
    OSScreenEnableEx(SCREEN_TV,false);
    OSScreenEnableEx(SCREEN_DRC,false);
    OSScreenShutdown();free(drc);free(tv);WHBProcShutdown();
    return 0;
}

#endif /* UI_HOST_PREVIEW */
