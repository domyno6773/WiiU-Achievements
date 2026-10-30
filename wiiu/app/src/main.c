/* Wii U Achievements: native catalog viewer, source-linked to Windows XML.
 * Temporary OSScreen renderer: NOT final visual parity or active monitoring.
 * This app only reads its compiled catalog. It cannot unlock achievements.
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

enum { HOME, LIST, DETAIL, INFO };
enum { ALL, ACTIVE, PLANNED, FILTERS };
static const char *const cats[] = {
    "Tutte", "Storia", "Equipaggiamento", "Esplorazione",
    "Collezioni", "Secondarie", "Crescita"
};
#define CAT_COUNT (sizeof(cats) / sizeof(cats[0]))
#define ROWS 11
static unsigned page = HOME, filter = ALL, category = 0, selected = 0;
static const AchievementEntry *detail = NULL;

static int matches(const AchievementEntry *a) {
    return (filter == ALL || (filter == ACTIVE) == (a->executable != 0))
        && (category == 0 || strcmp(a->category, cats[category]) == 0);
}
static unsigned visible_count(void) {
    unsigned count = 0;
    for (unsigned i = 0; i < g_catalog_count; ++i) count += matches(&g_catalog[i]);
    return count;
}
static const AchievementEntry *at(unsigned n) {
    for (unsigned i = 0; i < g_catalog_count; ++i) {
        if (matches(&g_catalog[i])) {
            if (n == 0) return &g_catalog[i];
            --n;
        }
    }
    return NULL;
}
static void line(OSScreenID s, unsigned row, const char *str) {
    OSScreenPutFontEx(s, 2, row, str);
}
static unsigned wrapped(OSScreenID s, unsigned row, const char *str, unsigned lines) {
    for (unsigned z = 0; z < lines && *str; ++z) {
        char b[39];
        unsigned len = 0;
        while (*str == ' ') ++str;
        while (len < sizeof(b) - 1 && *str && *str != '\n') b[len++] = *str++;
        b[len] = '\0';
        if (len == sizeof(b) - 1 && *str && *str != ' ') {
            char *space = strrchr(b, ' ');
            if (space != NULL && space - b > 12) {
                str -= len - (unsigned)(space - b);
                len = (unsigned)(space - b);
            }
        }
        b[len] = 0;
        if (*str == '\n') ++str;
        line(s, row++, b);
    }
    return row;
}
static void draw(OSScreenID s) {
    char b[128];
    OSScreenClearBufferEx(s, 0x102A4600);
    line(s, 1, "======================================");
    line(s, 2, "           Wii U Achievements");
    line(s, 3, "======================================");
    if (page == HOME) {
        line(s, 6, "GIOCHI");
        line(s, 9, "> The Legend of Zelda: Wind Waker HD");
        snprintf(b, sizeof b, "  %u obiettivi dal progetto originale", g_catalog_count);
        line(s, 11, b);
        line(s, 14, "Versione tecnica per Aroma.");
        line(s, 16, "Monitoraggio non ancora disponibile.");
        line(s, 24, "A: Catalogo  X: Info  B: Esci");
    } else if (page == LIST) {
        const char *const filters[] = {"Tutti", "Eseguibili", "Non attivi"};
        const unsigned count = visible_count();
        snprintf(b, sizeof b, "WIND WAKER HD - %u obiettivi", g_catalog_count);
        line(s, 5, b);
        snprintf(b, sizeof b, "Stato: %s", filters[filter]); line(s, 7, b);
        snprintf(b, sizeof b, "Categoria: %s", cats[category]); line(s, 8, b);
        snprintf(b, sizeof b, "Selezione: %u / %u", count ? selected + 1 : 0, count);
        line(s, 9, b);
        if (!count) line(s, 12, "Nessun obiettivo con questi filtri.");
        const unsigned start = selected / ROWS * ROWS;
        for (unsigned i = 0; i < ROWS && start + i < count; ++i) {
            const AchievementEntry *a = at(start + i);
            snprintf(b, sizeof b, "%c%c %-32.32s",
                selected == start + i ? '>' : ' ',
                !a->executable ? '-' : (a->experimental ? '*' : '+'),
                a->ascii_title);
            line(s, 11 + i, b);
        }
        line(s, 24, "SU/GIU: Scorri  A: Dettagli  B: Indietro");
        line(s, 25, "X: Stato  Y: Categoria   *: Test");
    } else if (page == DETAIL && detail != NULL) {
        line(s, 5, "DETTAGLI OBIETTIVO");
        unsigned row = wrapped(s, 7, detail->ascii_title, 3);
        snprintf(b, sizeof b, "Rarita: %s | Punti: %d", detail->rarity, detail->points);
        line(s, row + 1, b);
        snprintf(b, sizeof b, "Stato: %s", !detail->executable ? "Non attivo" :
            (detail->experimental ? "Sperimentale" : "Regola disponibile"));
        line(s, row + 2, b);
        snprintf(b, sizeof b, "Categoria: %s", detail->category);
        line(s, row + 3, b);
        line(s, row + 5, "Descrizione:");
        row = wrapped(s, row + 6, detail->ascii_description, 4);
        if (!detail->executable && row < 23) {
            line(s, row, "Da verificare:");
            wrapped(s, row + 1, detail->ascii_requirement, 3);
        }
        line(s, 25, "B: Torna alla lista");
    } else {
        line(s, 6, "INFORMAZIONI");
        line(s, 9, "Catalogo generato dagli XML originali");
        snprintf(b, sizeof b, "Revisione: %s", g_catalog_revision);
        line(s, 11, b);
        unsigned n_active = 0, n_planned = 0;
        for (unsigned i = 0; i < g_catalog_count; ++i) {
            if (g_catalog[i].executable) ++n_active;
            else ++n_planned;
        }
        snprintf(b, sizeof b, "%u eseguibili, %u non attivi", n_active, n_planned);
        line(s, 12, b);
        line(s, 14, "Nessun progresso viene ancora letto.");
        line(s, 16, "Nessuna memoria dei giochi viene letta.");
        line(s, 25, "B: Torna al catalogo giochi");
    }
}
static void repaint(void *tv, uint32_t ts, void *drc, uint32_t ds) {
    for (int n = 0; n < 2; ++n) {
        draw(SCREEN_TV);
        draw(SCREEN_DRC);
        DCFlushRange(tv, ts);
        DCFlushRange(drc, ds);
        OSScreenFlipBuffersEx(SCREEN_TV);
        OSScreenFlipBuffersEx(SCREEN_DRC);
    }
}
static int input(uint32_t key, bool *quit) {
    const unsigned old_page = page, old_filter = filter;
    const unsigned old_cat = category, old_selected = selected;
    if (key & VPAD_BUTTON_B) {
        if (page == HOME) *quit = true;
        else if (page == DETAIL) page = LIST;
        else page = HOME;
    } else if (page == HOME) {
        if (key & VPAD_BUTTON_A) page = LIST;
        else if (key & VPAD_BUTTON_X) page = INFO;
    } else if (page == LIST) {
        if (key & VPAD_BUTTON_DOWN) {
            if (selected + 1 < visible_count()) ++selected;
        } else if (key & VPAD_BUTTON_UP) {
            if (selected > 0) --selected;
        } else if (key & VPAD_BUTTON_X) {
            filter = (filter + 1) % FILTERS;
            selected = 0;
        } else if (key & VPAD_BUTTON_Y) {
            category = (category + 1) % CAT_COUNT;
            selected = 0;
        } else if (key & VPAD_BUTTON_A) {
            detail = at(selected);
            if (detail) page = DETAIL;
        }
    }
    return page != old_page || filter != old_filter ||
           category != old_cat || selected != old_selected;
}
int main(int argc, char **argv) {
    (void)argc; (void)argv;
    WHBProcInit();
    OSScreenInit();
    const uint32_t ts = OSScreenGetBufferSizeEx(SCREEN_TV);
    const uint32_t ds = OSScreenGetBufferSizeEx(SCREEN_DRC);
    void *tv = memalign(0x100, ts), *drc = memalign(0x100, ds);
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
    repaint(tv, ts, drc, ds);
    bool quit = false;
    while (WHBProcIsRunning() && !quit) {
        VPADStatus st = {0};
        VPADReadError err;
        VPADRead(VPAD_CHAN_0, &st, 1, &err);
        if (err == VPAD_READ_SUCCESS && input(st.trigger, &quit))
            repaint(tv, ts, drc, ds);
        OSSleepTicks(OSMillisecondsToTicks(33));
    }
    OSScreenEnableEx(SCREEN_TV, false);
    OSScreenEnableEx(SCREEN_DRC, false);
    OSScreenShutdown();
    free(drc); free(tv);
    WHBProcShutdown();
    return 0;
}
