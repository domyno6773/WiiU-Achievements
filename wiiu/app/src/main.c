/*
 * Wii U Achievements — native Aroma catalog prototype.
 * Upstream source of truth: src/Core.cs, Theme.cs, Banner.cs and four XML files.
 * Read-only viewer: NO memory reads, unlock engine, cloud sync or game patches.
 * The visual renderer uses original palette and code-generated artwork; text
 * font is a temporary public-domain bitmap until Nunito is ported.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <coreinit/cache.h>
#include <coreinit/screen.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <vpad/input.h>
#include <whb/proc.h>
#include "catalog.h"
#include "ui_canvas.h"

typedef enum { PAGE_GAMES, PAGE_ACHIEVEMENTS, PAGE_DETAILS, PAGE_INFO } Page;
typedef enum { FILTER_ALL, FILTER_PC_RULES, FILTER_PENDING, FILTER_COUNT } Filter;

static const char *const categories[] = {
    "Tutte", "Storia", "Equipaggiamento", "Esplorazione",
    "Collezioni", "Secondarie", "Crescita"
};
#define CATEGORY_COUNT (sizeof(categories) / sizeof(categories[0]))
#define CARDS_PER_PAGE 4

static Page page = PAGE_GAMES;
static Filter filter = FILTER_ALL;
static unsigned category = 0;
static unsigned chapter = 0;
static unsigned selected = 0;
static const AchievementEntry *detail = NULL;

static unsigned active_count(void) {
    unsigned result = 0;
    for (unsigned i = 0; i < g_catalog_count; ++i)
        if (g_catalog[i].executable) ++result;
    return result;
}

static const char *chapter_at(unsigned index) {
    if (index == 0) return "Tutte le tappe";
    unsigned found = 0;
    for (unsigned i = 0; i < g_catalog_count; ++i) {
        const char *name = g_catalog[i].chapter;
        bool seen = false;
        for (unsigned j = 0; j < i; ++j) {
            if (strcmp(name, g_catalog[j].chapter) == 0) { seen = true; break; }
        }
        if (!seen && ++found == index) return name;
    }
    return "Tutte le tappe";
}

static unsigned chapter_count(void) {
    unsigned count = 1;
    while (chapter_at(count) != NULL &&
           strcmp(chapter_at(count), "Tutte le tappe") != 0) ++count;
    return count;
}

static bool matches(const AchievementEntry *a) {
    if (filter == FILTER_PC_RULES && !a->executable) return false;
    if (filter == FILTER_PENDING && a->executable) return false;
    if (category > 0 && strcmp(a->category, categories[category]) != 0) return false;
    if (chapter > 0 && strcmp(a->chapter, chapter_at(chapter)) != 0) return false;
    return true;
}

static unsigned filtered_count(void) {
    unsigned result = 0;
    for (unsigned i = 0; i < g_catalog_count; ++i)
        if (matches(&g_catalog[i])) ++result;
    return result;
}

static const AchievementEntry *filtered_at(unsigned index) {
    for (unsigned i = 0; i < g_catalog_count; ++i) {
        const AchievementEntry *a = &g_catalog[i];
        if (!matches(a)) continue;
        if (index-- == 0) return a;
    }
    return NULL;
}

static void draw_header(UiCanvas *c) {
    ui_round(c, 12, 5, 830, 43, 13, UI_WHITE);
    ui_fill(c, 24, 43, 805, 2, UI_ACCENT_SOFT);
    ui_circle(c, 36, 26, 11, UI_ACCENT);
    ui_text(c, 31, 22, "*", 1, UI_WHITE, 1);
    ui_text(c, 58, 16, "Wii U Achievements", 2, UI_INK, 22);
    ui_round(c, 666, 14, 160, 27, 12, UI_ACCENT_SOFT);
    ui_text(c, 684, 24, "VERSIONE WII U", 1, UI_ACCENT_DARK, 15);
}

static void draw_footer(UiCanvas *c, const char *left, const char *right) {
    ui_fill(c, 0, 446, 854, 34, UI_WHITE);
    ui_fill(c, 0, 445, 854, 1, UI_LINE);
    ui_text(c, 21, 459, left, 1, UI_MUTED, 55);
    if (right) ui_text(c, 600, 459, right, 1, UI_ACCENT_DARK, 28);
}

static void draw_banner(UiCanvas *c, int compact) {
    int h = compact ? 111 : 135;
    ui_ocean_banner(c, 21, 57, 812, h);
    ui_text(c, 45, 74, "THE LEGEND OF ZELDA", 1, UI_ACCENT_DARK, 30);
    ui_text(c, 45, 91, "THE WIND WAKER HD", 2, UI_INK, 22);
    ui_round(c, 45, 120, 142, 24, 12, UI_WHITE);
    ui_text(c, 56, 129, "PLAYTHROUGH TEST", 1, UI_ACCENT_DARK, 16);
    if (!compact) {
        ui_text(c, 45, 164, "CATALOGO ORIGINALE", 1, UI_WHITE, 28);
        ui_text(c, 252, 164, "PROGRESSI NON DISPONIBILI", 1, UI_WHITE, 32);
    }
}

static void draw_games(UiCanvas *c) {
    draw_banner(c, 0);
    ui_text(c, 25, 209, "Giochi", 2, UI_INK, 10);
    ui_text(c, 145, 216, "Un solo titolo supportato dal catalogo originale", 1, UI_MUTED, 58);
    ui_round(c, 25, 245, 804, 155, 16, UI_LINE);
    ui_round(c, 25, 241, 804, 155, 16, UI_WHITE);
    ui_border(c, 24, 240, 806, 157, 3, UI_ACCENT);
    ui_medal(c, 103, 309, "gold", 0);
    ui_text(c, 156, 274, "The Wind Waker HD", 2, UI_INK, 24);
    char buffer[100];
    snprintf(buffer, sizeof buffer, "%u obiettivi  |  %u regole PC  |  %u da verificare",
             g_catalog_count, active_count(), g_catalog_count - active_count());
    ui_text(c, 157, 317, buffer, 1, UI_MUTED, 76);
    ui_round(c, 157, 348, 175, 25, 12, UI_ACCENT);
    ui_text(c, 173, 357, "A  APRI OBIETTIVI", 1, UI_WHITE, 18);
    draw_footer(c, "A: apri catalogo    X: informazioni    B: esci", "SOLO LETTURA");
}

static void draw_filter_pill(UiCanvas *c, int x, int y, int w, const char *label, bool active) {
    ui_round(c, x, y, w, 24, 12, active ? UI_ACCENT : UI_WHITE);
    ui_text(c, x + 12, y + 8, label, 1, active ? UI_WHITE : UI_INK, 28);
}

static void draw_achievement_card(UiCanvas *c, int x, int y,
                                  const AchievementEntry *a, bool focus) {
    ui_round(c, x, y + 3, 393, 85, 12, UI_LINE);
    ui_round(c, x, y, 393, 85, 12, UI_WHITE);
    if (focus) ui_border(c, x, y, 393, 85, 3, UI_ACCENT);
    ui_medal(c, x + 39, y + 42, a->rarity, !a->executable);
    ui_text(c, x + 75, y + 12, a->ascii_title, 1, UI_INK, 36);
    ui_text(c, x + 75, y + 30, a->ascii_description, 1, UI_MUTED, 37);
    if (a->executable) {
        ui_round(c, x + 75, y + 52, a->experimental ? 130 : 105, 21, 10,
                 a->experimental ? 0xFFF2DE00u : UI_ACCENT_SOFT);
        ui_text(c, x + 84, y + 59, a->experimental ? "SPERIMENTALE" : "REGOLA PC",
                1, a->experimental ? UI_WARNING : UI_ACCENT_DARK, 15);
        char b[24];
        snprintf(b, sizeof b, "%d PT", a->points);
        ui_text(c, x + 326, y + 59, b, 1, UI_INK, 7);
    } else {
        ui_round(c, x + 75, y + 52, 114, 21, 10, 0xFFF2DE00u);
        ui_text(c, x + 84, y + 59, "NON ATTIVO", 1, UI_WARNING, 12);
    }
}

static void draw_catalog(UiCanvas *c) {
    draw_banner(c, 1);
    draw_filter_pill(c, 22, 176, 88, "Tutti", filter == FILTER_ALL);
    draw_filter_pill(c, 117, 176, 128, "Regole PC", filter == FILTER_PC_RULES);
    draw_filter_pill(c, 252, 176, 132, "Non attivi", filter == FILTER_PENDING);
    char buf[96];
    unsigned total = filtered_count();
    snprintf(buf, sizeof buf, "%u / %u", total, g_catalog_count);
    ui_text(c, 764, 185, buf, 1, UI_MUTED, 9);
    ui_text(c, 23, 211, "Categoria:", 1, UI_MUTED, 12);
    ui_text(c, 117, 211, categories[category], 1, UI_ACCENT_DARK, 25);
    ui_text(c, 379, 211, "Tappa:", 1, UI_MUTED, 8);
    ui_text(c, 448, 211, chapter_at(chapter), 1, UI_ACCENT_DARK, 40);
    if (!total) {
        ui_round(c, 25, 235, 804, 192, 14, UI_WHITE);
        ui_text(c, 90, 316, "Nessun obiettivo con questi filtri.", 2, UI_MUTED, 38);
    } else {
        unsigned start = selected / CARDS_PER_PAGE * CARDS_PER_PAGE;
        for (unsigned slot = 0; slot < CARDS_PER_PAGE && start + slot < total; ++slot) {
            const AchievementEntry *a = filtered_at(start + slot);
            int x = 25 + (int)(slot % 2) * 408;
            int y = 238 + (int)(slot / 2) * 95;
            draw_achievement_card(c, x, y, a, start + slot == selected);
        }
    }
    draw_footer(c, "Frecce: naviga  A: dettagli  B: indietro",
                "X: stato  Y: categoria  L/R: tappa");
}

static void draw_details(UiCanvas *c) {
    draw_banner(c, 1);
    if (!detail) return;
    ui_round(c, 24, 177, 805, 255, 16, UI_LINE);
    ui_round(c, 24, 174, 805, 255, 16, UI_WHITE);
    ui_medal(c, 100, 228, detail->rarity, !detail->executable);
    ui_text(c, 154, 197, detail->ascii_title, 2, UI_INK, 40);
    char b[100];
    snprintf(b, sizeof b, "Rarita: %s   |   %s", detail->rarity,
             detail->executable ? "Regola presente nella versione PC" :
                                  "Non ancora implementato");
    ui_text(c, 154, 238, b, 1, UI_MUTED, 70);
    snprintf(b, sizeof b, "Categoria: %s", detail->category);
    ui_text(c, 154, 256, b, 1, UI_MUTED, 60);
    ui_fill(c, 46, 284, 760, 1, UI_LINE);
    ui_text(c, 52, 301, "DESCRIZIONE", 1, UI_ACCENT_DARK, 25);
    int next = ui_wrap(c, 52, 321, detail->ascii_description, 82, 3, UI_INK);
    if (!detail->executable && next < 396) {
        ui_text(c, 52, next + 5, "DA VERIFICARE", 1, UI_WARNING, 23);
        ui_wrap(c, 52, next + 20, detail->ascii_requirement, 82, 2, UI_MUTED);
    } else if (detail->executable) {
        ui_text(c, 52, 390, "Questa build non registra ancora gli sblocchi.", 1, UI_WARNING, 65);
    }
    draw_footer(c, "B: torna alla lista degli obiettivi", NULL);
}

static void draw_info(UiCanvas *c) {
    ui_round(c, 24, 73, 805, 348, 16, UI_WHITE);
    ui_text(c, 50, 96, "Informazioni sul porting", 2, UI_INK, 38);
    ui_fill(c, 50, 136, 750, 2, UI_ACCENT_SOFT);
    ui_text(c, 54, 162, "Wii U Achievements - versione nativa Aroma", 1, UI_INK, 69);
    char b[100];
    snprintf(b, sizeof b, "Catalogo: %u obiettivi / revisione %s",
             g_catalog_count, g_catalog_revision);
    ui_text(c, 54, 185, b, 1, UI_INK, 80);
    snprintf(b, sizeof b, "%u regole Windows, %u non attive",
             active_count(), g_catalog_count - active_count());
    ui_text(c, 54, 207, b, 1, UI_INK, 60);
    ui_text(c, 54, 251, "Non e' ancora presente il motore dei trofei Wii U.", 1, UI_WARNING, 78);
    ui_text(c, 54, 275, "Non vengono letti giochi, salvataggi o progressi.", 1, UI_MUTED, 78);
    ui_text(c, 54, 297, "Nessun dato personale viene scritto dalla app.", 1, UI_MUTED, 78);
    ui_text(c, 54, 340, "Grafica provvisoria ispirata alla versione PC.", 1, UI_MUTED, 78);
    draw_footer(c, "B: torna al catalogo giochi", "BRANCH: WIIU-NATIVE");
}

static void render(OSScreenID screen) {
    UiCanvas canvas;
    ui_canvas_init(&canvas, screen);
    ui_backdrop(&canvas);
    draw_header(&canvas);
    if (page == PAGE_GAMES) draw_games(&canvas);
    else if (page == PAGE_ACHIEVEMENTS) draw_catalog(&canvas);
    else if (page == PAGE_DETAILS) draw_details(&canvas);
    else draw_info(&canvas);
}

static void repaint(void *tv, uint32_t tv_size, void *drc, uint32_t drc_size) {
    /* Both halves of each double buffer are updated on a view change. */
    for (int n = 0; n < 2; ++n) {
        render(SCREEN_TV);
        render(SCREEN_DRC);
        DCFlushRange(tv, tv_size);
        DCFlushRange(drc, drc_size);
        OSScreenFlipBuffersEx(SCREEN_TV);
        OSScreenFlipBuffersEx(SCREEN_DRC);
    }
}

static bool handle_input(uint32_t key, bool *quit) {
    Page old_page = page;
    Filter old_filter = filter;
    unsigned old_category = category, old_chapter = chapter, old_selected = selected;
    if (key & VPAD_BUTTON_B) {
        if (page == PAGE_GAMES) *quit = true;
        else if (page == PAGE_DETAILS) page = PAGE_ACHIEVEMENTS;
        else page = PAGE_GAMES;
    } else if (page == PAGE_GAMES) {
        if (key & VPAD_BUTTON_A) page = PAGE_ACHIEVEMENTS;
        if (key & VPAD_BUTTON_X) page = PAGE_INFO;
    } else if (page == PAGE_ACHIEVEMENTS) {
        unsigned total = filtered_count();
        if (key & VPAD_BUTTON_UP) {
            if (selected >= 2) selected -= 2;
        } else if (key & VPAD_BUTTON_DOWN) {
            if (selected + 2 < total) selected += 2;
        } else if (key & VPAD_BUTTON_LEFT) {
            if (selected > 0) --selected;
        } else if (key & VPAD_BUTTON_RIGHT) {
            if (selected + 1 < total) ++selected;
        } else if (key & VPAD_BUTTON_X) {
            filter = (Filter)((filter + 1) % FILTER_COUNT);
            selected = 0;
        } else if (key & VPAD_BUTTON_Y) {
            category = (category + 1) % CATEGORY_COUNT;
            selected = 0;
        } else if (key & VPAD_BUTTON_L) {
            unsigned count = chapter_count();
            chapter = (chapter + count - 1) % count;
            selected = 0;
        } else if (key & VPAD_BUTTON_R) {
            chapter = (chapter + 1) % chapter_count();
            selected = 0;
        } else if (key & VPAD_BUTTON_A) {
            if (total) {
                detail = filtered_at(selected);
                if (detail) page = PAGE_DETAILS;
            }
        }
    }
    return old_page != page || old_filter != filter || old_category != category ||
           old_chapter != chapter || old_selected != selected;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    WHBProcInit();
    OSScreenInit();
    const uint32_t tv_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    const uint32_t drc_size = OSScreenGetBufferSizeEx(SCREEN_DRC);
    void *tv = memalign(0x100, tv_size);
    void *drc = memalign(0x100, drc_size);
    if (!tv || !drc) {
        free(drc); free(tv);
        OSScreenShutdown();
        WHBProcShutdown();
        return 1;
    }
    OSScreenSetBufferEx(SCREEN_TV, tv);
    OSScreenSetBufferEx(SCREEN_DRC, drc);
    OSScreenEnableEx(SCREEN_TV, true);
    OSScreenEnableEx(SCREEN_DRC, true);
    repaint(tv, tv_size, drc, drc_size);

    bool quit = false;
    while (WHBProcIsRunning() && !quit) {
        VPADStatus status = {0};
        VPADReadError error;
        VPADRead(VPAD_CHAN_0, &status, 1, &error);
        if (error == VPAD_READ_SUCCESS && handle_input(status.trigger, &quit))
            repaint(tv, tv_size, drc, drc_size);
        OSSleepTicks(OSMillisecondsToTicks(33));
    }
    OSScreenEnableEx(SCREEN_TV, false);
    OSScreenEnableEx(SCREEN_DRC, false);
    OSScreenShutdown();
    free(drc); free(tv);
    WHBProcShutdown();
    return 0;
}
