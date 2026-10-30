/* Host-only implementation of Wii U screen writes for native renderer QA.
 * Same ui_canvas.c and screen layouts as the compiled WUHB are used.
 * Output is PPM, converted by Pillow in GitHub Actions to PNG.
 */
#include <stdio.h>
#include <stdint.h>
#include "host_shim.h"
static uint32_t tv[1280 * 720], drc[854 * 480];
void OSScreenClearBufferEx(OSScreenID screen, uint32_t color) {
    uint32_t *data=screen==SCREEN_TV?tv:drc;
    unsigned len=screen==SCREEN_TV?1280u*720u:854u*480u;
    for(unsigned i=0;i<len;i++)data[i]=color;
}
void OSScreenPutPixelEx(OSScreenID screen,uint32_t x,uint32_t y,uint32_t color){
    unsigned w=screen==SCREEN_TV?1280:854,h=screen==SCREEN_TV?720:480;
    if(x<w&&y<h)(screen==SCREEN_TV?tv:drc)[y*w+x]=color;
}
int host_save_png_inputs(const char *path,OSScreenID screen){
    unsigned w=screen==SCREEN_TV?1280:854,h=screen==SCREEN_TV?720:480;
    uint32_t *data=screen==SCREEN_TV?tv:drc;
    FILE *out=fopen(path,"wb");
    if(!out)return 1;
    fprintf(out,"P6\n%u %u\n255\n",w,h);
    for(unsigned i=0;i<w*h;i++){
        unsigned char rgb[3]={
            (unsigned char)((data[i]>>24)&255),
            (unsigned char)((data[i]>>16)&255),
            (unsigned char)((data[i]>>8)&255)
        };
        if(fwrite(rgb,1,3,out)!=3){fclose(out);return 2;}
    }
    fclose(out);
    return 0;
}
