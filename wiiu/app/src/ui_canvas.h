#ifndef WIIU_ACHIEVEMENTS_UI_CANVAS_H
#define WIIU_ACHIEVEMENTS_UI_CANVAS_H
#include <stdint.h>
#include <coreinit/screen.h>

/* Logical layout: 854x480. TV is 1280x720, GamePad 854x480.
 * All artwork is generated from simple geometry, as in the Windows version.
 * The temporary public-domain bitmap font is not the final Nunito renderer. */
typedef struct {
    OSScreenID screen;
    int numerator;
    int denominator;
} UiCanvas;

enum {
    UI_WHITE = 0xFFFFFF00u,
    UI_BG = 0xF8FAFC00u,
    UI_BG_STRIPE = 0xE8EEF300u,
    UI_INK = 0x343A4000u,
    UI_MUTED = 0x7C879000u,
    UI_LINE = 0xD4DBE100u,
    UI_ACCENT = 0x00A8E200u,
    UI_ACCENT_DARK = 0x007EC000u,
    UI_ACCENT_SOFT = 0xD6F1FC00u,
    UI_SUCCESS = 0x52B42E00u,
    UI_WARNING = 0xEC921400u,
    UI_CLOUD = 0xEDF9FF00u,
    UI_SKY = 0xBDEDFC00u,
    UI_SEA = 0x258DBD00u,
    UI_BRONZE = 0xC58C6000u,
    UI_SILVER = 0xBECBD500u,
    UI_GOLD = 0xEBC25D00u
};

void ui_canvas_init(UiCanvas *ctx, OSScreenID screen);
void ui_fill(UiCanvas *ctx, int x, int y, int w, int h, uint32_t color);
void ui_round(UiCanvas *ctx, int x, int y, int w, int h, int radius, uint32_t color);
void ui_border(UiCanvas *ctx, int x, int y, int w, int h, int thickness, uint32_t color);
void ui_circle(UiCanvas *ctx, int x, int y, int radius, uint32_t color);
void ui_text(UiCanvas *ctx, int x, int y, const char *ascii, int size, uint32_t color, int max_chars);
int ui_wrap(UiCanvas *ctx, int x, int y, const char *ascii,
            int max_chars, int max_lines, uint32_t color);
void ui_backdrop(UiCanvas *ctx);
void ui_ocean_banner(UiCanvas *ctx, int x, int y, int w, int h);
void ui_medal(UiCanvas *ctx, int cx, int cy, const char *rarity, int pending);
#endif
