/*
 * Wii U Achievements — native frontend bootstrap.
 *
 * This intentionally simple navigation shell establishes a real WUHB entry
 * point, proper Wii U lifecycle and GamePad input. It does NOT pretend to be
 * the final ported WinForms UI, a live game monitor or an achievement reader.
 *
 * The Windows source and root XML remain canonical. See wiiu/docs/COMPATIBILITY.md.
 */
#include <stdbool.h>
#include <stdint.h>

#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <vpad/input.h>
#include <whb/log.h>
#include <whb/log_console.h>
#include <whb/proc.h>

typedef enum {
    PAGE_HOME = 0,
    PAGE_WIND_WAKER,
    PAGE_INFO
} Page;

static void draw_page(Page page)
{
    /* WHBLogConsoleClear is the wut logging-console reset function. */
    WHBLogConsoleClear();

    WHBLogPrintf("========================================");
    WHBLogPrintf("           Wii U Achievements");
    WHBLogPrintf("========================================");
    WHBLogPrintf(" Port nativo del progetto WiiU-Achievements");
    WHBLogPrintf(" Branch: wiiu-native");
    WHBLogPrintf("");

    switch (page) {
    case PAGE_HOME:
        WHBLogPrintf("CATALOGO GIOCHI");
        WHBLogPrintf("");
        WHBLogPrintf(" > The Legend of Zelda: The Wind Waker HD");
        WHBLogPrintf("   Primo gioco previsto (non ancora collegato)");
        WHBLogPrintf("");
        WHBLogPrintf(" A: apri scheda     X: informazioni");
        WHBLogPrintf(" B: esci");
        break;
    case PAGE_WIND_WAKER:
        WHBLogPrintf("THE LEGEND OF ZELDA: THE WIND WAKER HD");
        WHBLogPrintf("");
        WHBLogPrintf(" La lista e le regole arriveranno dai file");
        WHBLogPrintf(" canonici del progetto Windows.");
        WHBLogPrintf("");
        WHBLogPrintf(" Stato: prototipo frontend, nessun");
        WHBLogPrintf(" achievement monitorato o sbloccabile.");
        WHBLogPrintf("");
        WHBLogPrintf(" B: torna al catalogo");
        break;
    case PAGE_INFO:
        WHBLogPrintf("INFORMAZIONI");
        WHBLogPrintf("");
        WHBLogPrintf(" App nativa wut / Aroma.");
        WHBLogPrintf(" Interfaccia definitiva e core in porting.");
        WHBLogPrintf(" Nessuna lettura o scrittura in memoria");
        WHBLogPrintf(" dei giochi.");
        WHBLogPrintf("");
        WHBLogPrintf(" B: torna al catalogo");
        break;
    }
    WHBLogConsoleDraw();
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    WHBProcInit();
    WHBLogConsoleInit();

    Page page = PAGE_HOME;
    bool needs_redraw = true;
    bool quit_requested = false;

    while (WHBProcIsRunning() && !quit_requested) {
        VPADStatus status = {0};
        VPADRead(VPAD_CHAN_0, &status, 1, NULL);

        if (status.trigger & VPAD_BUTTON_B) {
            if (page == PAGE_HOME) {
                quit_requested = true;
            } else {
                page = PAGE_HOME;
                needs_redraw = true;
            }
        } else if (page == PAGE_HOME && (status.trigger & VPAD_BUTTON_A)) {
            page = PAGE_WIND_WAKER;
            needs_redraw = true;
        } else if (page == PAGE_HOME && (status.trigger & VPAD_BUTTON_X)) {
            page = PAGE_INFO;
            needs_redraw = true;
        }

        if (needs_redraw) {
            draw_page(page);
            needs_redraw = false;
        } else {
            /* The WHB console is double-buffered: keep presenting it. */
            WHBLogConsoleDraw();
        }
        OSSleepTicks(OSMillisecondsToTicks(33));
    }

    WHBLogConsoleFree();
    WHBProcShutdown();
    return 0;
}
