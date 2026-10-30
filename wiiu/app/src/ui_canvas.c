/*
 * Wii U native drawing primitives. Colors/layout originate in the upstream
 * Theme.cs and Banner.cs; vector shapes and sea scenery are drawn locally.
 * font8x8_basic.h: Daniel Hepper, public domain; replacement with Nunito
 * (already licensed and bundled upstream) is a later visual parity task.
 */
#include <string.h>
#include "ui_canvas.h"
#include "font8x8_basic.h"

static int max_int(int a, int b) { return a > b ? a : b; }
static int min_int(int a, int b) { return a < b ? a : b; }

void ui_canvas_init(UiCanvas *ctx, OSScreenID screen) {
    ctx->screen = screen;
    ctx->numerator = screen == SCREEN_TV ? 3 : 1;
    ctx->denominator = screen == SCREEN_TV ? 2 : 1;
}

static void point(UiCanvas *ctx, int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= 854 || y >= 480) return;
    int x0 = x * ctx->numerator / ctx->denominator;
    int y0 = y * ctx->numerator / ctx->denominator;
    int x1 = (x + 1) * ctx->numerator / ctx->denominator;
    int y1 = (y + 1) * ctx->numerator / ctx->denominator;
    for (int iy = y0; iy < y1; ++iy)
        for (int ix = x0; ix < x1; ++ix)
            OSScreenPutPixelEx(ctx->screen, (uint32_t)ix, (uint32_t)iy, color);
}

void ui_fill(UiCanvas *ctx, int x, int y, int w, int h, uint32_t color) {
    if (w <= 0 || h <= 0) return;
    int x0 = max_int(0, x), y0 = max_int(0, y);
    int x1 = min_int(854, x + w), y1 = min_int(480, y + h);
    if (x1 <= x0 || y1 <= y0) return;
    int dx0 = x0 * ctx->numerator / ctx->denominator;
    int dy0 = y0 * ctx->numerator / ctx->denominator;
    int dx1 = x1 * ctx->numerator / ctx->denominator;
    int dy1 = y1 * ctx->numerator / ctx->denominator;
    for (int iy = dy0; iy < dy1; ++iy)
        for (int ix = dx0; ix < dx1; ++ix)
            OSScreenPutPixelEx(ctx->screen, (uint32_t)ix, (uint32_t)iy, color);
}

void ui_round(UiCanvas *ctx, int x, int y, int w, int h, int r, uint32_t color) {
    if (w <= 0 || h <= 0) return;
    if (r < 1) { ui_fill(ctx, x, y, w, h, color); return; }
    r = min_int(r, min_int(w, h) / 2);
    for (int yy = max_int(0, y); yy < min_int(480, y + h); ++yy) {
        int top = yy < y + r;
        int bottom = yy >= y + h - r;
        for (int xx = max_int(0, x); xx < min_int(854, x + w); ++xx) {
            int left = xx < x + r;
            int right = xx >= x + w - r;
            if ((top || bottom) && (left || right)) {
                int cx = left ? x + r : x + w - r - 1;
                int cy = top ? y + r : y + h - r - 1;
                int dx = xx - cx, dy = yy - cy;
                if (dx * dx + dy * dy > r * r) continue;
            }
            point(ctx, xx, yy, color);
        }
    }
}

void ui_border(UiCanvas *ctx, int x, int y, int w, int h, int thickness, uint32_t color) {
    ui_fill(ctx, x, y, w, thickness, color);
    ui_fill(ctx, x, y + h - thickness, w, thickness, color);
    ui_fill(ctx, x, y, thickness, h, color);
    ui_fill(ctx, x + w - thickness, y, thickness, h, color);
}

void ui_circle(UiCanvas *ctx, int cx, int cy, int r, uint32_t color) {
    for (int yy = max_int(0, cy - r); yy <= min_int(479, cy + r); ++yy)
        for (int xx = max_int(0, cx - r); xx <= min_int(853, cx + r); ++xx) {
            int dx = xx - cx, dy = yy - cy;
            if (dx * dx + dy * dy <= r * r) point(ctx, xx, yy, color);
        }
}

void ui_text(UiCanvas *ctx, int x, int y, const char *str, int size,
             uint32_t color, int max_chars) {
    if (!str || size < 1 || max_chars <= 0) return;
    int i = 0;
    for (; str[i] && str[i] != '\n' && i < max_chars; ++i) {
        unsigned char code = (unsigned char)str[i];
        if (code > 127 || code < 32) code = '?';
        int gx = x + i * 8 * size;
        if (gx + size * 8 > 854) break;
        for (int gy = 0; gy < 8; ++gy)
            for (int px = 0; px < 8; ++px)
                if (((unsigned char)font8x8_basic[code][gy] >> px) & 1)
                    ui_fill(ctx, gx + px * size, y + gy * size, size, size, color);
    }
    if (str[i] && str[i] != '\n' && max_chars >= 3 && i >= 3) {
        int tx = x + (i - 3) * size * 8;
        for (int k = 0; k < 3; ++k)
            for (int yy = 0; yy < 8; ++yy)
                for (int xx = 0; xx < 8; ++xx)
                    if (((unsigned char)font8x8_basic[(unsigned char)'.'][yy] >> xx) & 1)
                        ui_fill(ctx, tx + (k * 8 + xx) * size,
                                y + yy * size, size, size, color);
    }
}

int ui_wrap(UiCanvas *ctx, int x, int y, const char *str,
            int max_chars, int max_lines, uint32_t color) {
    if (!str || max_chars < 1 || max_chars > 100) return y;
    for (int n = 0; n < max_lines && *str; ++n) {
        char buf[128];
        int len = 0;
        while (*str == ' ') ++str;
        while (str[len] && str[len] != '\n' && len < max_chars) ++len;
        int cut = len;
        if (str[len] && str[len] != '\n' && str[len] != ' ') {
            for (int k = len; k > 9; --k) {
                if (str[k - 1] == ' ') { cut = k - 1; break; }
            }
        }
        if (!cut) { ++str; continue; }
        memcpy(buf, str, (size_t)cut);
        buf[cut] = '\0';
        ui_text(ctx, x, y, buf, 1, color, cut);
        str += cut;
        if (*str == '\n') ++str;
        y += 14;
    }
    return y;
}

void ui_backdrop(UiCanvas *ctx) {
    OSScreenClearBufferEx(ctx->screen, UI_BG);
    for (int y = 48; y < 450; y += 18)
        ui_fill(ctx, 0, y, 854, 1, UI_BG_STRIPE);
}

void ui_ocean_banner(UiCanvas *ctx, int x, int y, int w, int h) {
    /* Static cel-shaded ocean inspired by the source OceanBanner, no copied art. */
    ui_round(ctx, x, y + 3, w, h, 15, UI_LINE);
    ui_round(ctx, x, y, w, h, 15, UI_SKY);
    ui_fill(ctx, x + 2, y + 61, w - 4, h - 63, UI_SEA);
    ui_fill(ctx, x + 2, y + 80, w - 4, h - 82, 0x146A9D00u);
    ui_fill(ctx, x + 2, y + 99, w - 4, h - 101, 0x0C507E00u);
    /* Three banks of original foam highlights. */
    for (int i = 0; i < 30; ++i) {
        int xx = x + 12 + i * (w - 24) / 30;
        int offset = ((i * 7 + i * i) % 17);
        ui_fill(ctx, xx, y + 65 + offset, 9, 2, 0xBDEDFC00u);
        if (i % 3 == 0) ui_fill(ctx, xx + 5, y + 94 + offset / 2, 17, 2, 0x4AB7DD00u);
    }
    ui_round(ctx, x + w - 175, y + 13, 45, 12, 6, UI_CLOUD);
    ui_round(ctx, x + w - 150, y + 8, 45, 17, 8, UI_CLOUD);
    ui_round(ctx, x + w - 110, y + 31, 52, 11, 5, UI_CLOUD);
    /* Island silhouette */
    ui_circle(ctx, x + w - 112, y + 64, 22, 0x86B16A00u);
    ui_fill(ctx, x + w - 143, y + 63, 73, 8, 0x5B985E00u);
}

void ui_medal(UiCanvas *ctx, int cx, int cy, const char *rarity, int pending) {
    uint32_t tier = UI_BRONZE;
    if (rarity && strcmp(rarity, "silver") == 0) tier = UI_SILVER;
    else if (rarity && strcmp(rarity, "gold") == 0) tier = UI_GOLD;
    else if (rarity && strcmp(rarity, "platinum") == 0) tier = 0xB7C7F600u;
    ui_circle(ctx, cx, cy + 2, 25, UI_LINE);
    ui_circle(ctx, cx, cy, 23, tier);
    ui_circle(ctx, cx, cy, 18, UI_WHITE);
    ui_circle(ctx, cx, cy, 15, tier);
    ui_text(ctx, cx - 8, cy - 8, pending ? "?" : "*", 2,
            pending ? UI_INK : UI_WHITE, 1);
}
