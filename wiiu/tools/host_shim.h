#ifndef WIIU_UI_HOST_SHIM_H
#define WIIU_UI_HOST_SHIM_H
#include <stdint.h>
typedef enum { SCREEN_TV = 0, SCREEN_DRC = 1 } OSScreenID;
void OSScreenClearBufferEx(OSScreenID screen, uint32_t color);
void OSScreenPutPixelEx(OSScreenID screen, uint32_t x, uint32_t y, uint32_t color);
int host_save_png_inputs(const char *file, OSScreenID screen);
#endif
