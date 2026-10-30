/*
 * Wii U Achievements — native Aroma frontend bootstrap.
 *
 * First milestone: Wii U app lifecycle, TV/GamePad rendering, input/navigation.
 * NOT the final WinForms visual port and NOT a functioning achievement monitor.
 * Original source/XML in this repository remain the canonical implementation.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>

#include <coreinit/cache.h>
#include <coreinit/screen.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <vpad/input.h>
#include <whb/proc.h>

typedef enum {
    PAGE_HOME = 0,
    PAGE_WIND_WAKER,
    PAGE_INFO
} Page;

static void put_line(OSScreenID screen, uint32_t row, const char *text)
{
    OSScreenPutFontEx(screen, 2, row, text);
}

static void render_page(OSScreenID screen, Page page)
{
    OSScreenClearBufferEx(screen, 0x102A4600);

    put_line(screen, 2,  "======================================");
    put_line(screen, 3,  "          Wii U Achievements");
    put_line(screen, 4,  "======================================");
    put_line(screen, 6,  "Port nativo del progetto originale");
    put_line(screen, 7,  "Branch: wiiu-native");

    switch (page) {
    case PAGE_HOME:
        put_line(screen, 10, "CATALOGO GIOCHI");
        put_line(screen, 12, "> The Legend of Zelda: Wind Waker HD");
        put_line(screen, 13, "  Primo gioco previsto - non collegato");
        put_line(screen, 18, "A: Apri scheda  X: Informazioni");
        put_line(screen, 19, "B: Esci");
        break;
    case PAGE_WIND_WAKER:
        put_line(screen, 10, "THE LEGEND OF ZELDA: WIND WAKER HD");
        put_line(screen, 12, "Catalogo e regole saranno importati");
        put_line(screen, 13, "dal progetto Windows originale.");
        put_line(screen, 15, "Prototipo: nessun obiettivo attivo");
        put_line(screen, 18, "B: Torna al catalogo");
        break;
    case PAGE_INFO:
        put_line(screen, 10, "INFORMAZIONI");
        put_line(screen, 12, "App nativa wut / Aroma.");
        put_line(screen, 13, "Interfaccia originale ancora in porting.");
        put_line(screen, 15, "Nessuna lettura della memoria dei giochi.");
        put_line(screen, 18, "B: Torna al catalogo");
        break;
    }
}

static void redraw(Page page, void *tv, uint32_t tv_size,
                   void *drc, uint32_t drc_size)
{
    /* Both frames of each double buffer must show the newly selected page. */
    for (unsigned frame = 0; frame < 2; ++frame) {
        render_page(SCREEN_TV, page);
        render_page(SCREEN_DRC, page);
        DCFlushRange(tv, tv_size);
        DCFlushRange(drc, drc_size);
        OSScreenFlipBuffersEx(SCREEN_TV);
        OSScreenFlipBuffersEx(SCREEN_DRC);
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    WHBProcInit();
    OSScreenInit();

    const uint32_t tv_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    const uint32_t drc_size = OSScreenGetBufferSizeEx(SCREEN_DRC);
    void *tv = memalign(0x100, tv_size);
    void *drc = memalign(0x100, drc_size);

    if (tv == NULL || drc == NULL) {
        free(drc);
        free(tv);
        OSScreenShutdown();
        WHBProcShutdown();
        return 1;
    }

    OSScreenSetBufferEx(SCREEN_TV, tv);
    OSScreenSetBufferEx(SCREEN_DRC, drc);
    OSScreenEnableEx(SCREEN_TV, true);
    OSScreenEnableEx(SCREEN_DRC, true);

    Page page = PAGE_HOME;
    bool quit_requested = false;
    redraw(page, tv, tv_size, drc, drc_size);

    while (WHBProcIsRunning() && !quit_requested) {
        VPADStatus status = {0};
        VPADReadError error;
        VPADRead(VPAD_CHAN_0, &status, 1, &error);

        if (error == VPAD_READ_SUCCESS) {
            const Page previous = page;
            if (status.trigger & VPAD_BUTTON_B) {
                if (page == PAGE_HOME) {
                    quit_requested = true;
                } else {
                    page = PAGE_HOME;
                }
            } else if (page == PAGE_HOME && (status.trigger & VPAD_BUTTON_A)) {
                page = PAGE_WIND_WAKER;
            } else if (page == PAGE_HOME && (status.trigger & VPAD_BUTTON_X)) {
                page = PAGE_INFO;
            }

            if (page != previous) {
                redraw(page, tv, tv_size, drc, drc_size);
            }
        }
        OSSleepTicks(OSMillisecondsToTicks(33));
    }

    OSScreenEnableEx(SCREEN_TV, false);
    OSScreenEnableEx(SCREEN_DRC, false);
    OSScreenShutdown();
    free(drc);
    free(tv);
    WHBProcShutdown();
    return 0;
}
