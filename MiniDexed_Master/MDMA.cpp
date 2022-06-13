#include <stdio.h>
#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"
#include "pico/multicore.h"

#include "MidiCore.h"
#include "Menu/MainMenu.h"
#include "MDMA.h"
 //--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

//#define IGNORE_MIDI_CC

cPots Pots = cPots();
cButtons buttons = cButtons();
cTG dexed[8];
cMenu menu;

/*------------- MAIN -------------*/
int main(void)
{
    queue_init(&tg_fifo, sizeof(dexed_t), FIFOLENGTH);
    queue_init(&midi_fifo, sizeof(dexed_t), FIFOLENGTH);
    queue_init(&sysex_fifo, sizeof(sysex_t), FIFOLENGTH);
    board_init();
    set_sys_clock_khz(133000, true);

    stdio_init_all();
    printf("MDMA Booting\r\n");
    
    Pots.init();
    menu.Init();

    multicore_launch_core1(midicore);
 
//    while (midi_ready == false)
//    {
//        tight_loop_contents();
//    }
 //   tft.fillScreen(BLACK);
    menu.Show();

    sysex_t raw_sysex;
    while (1)
    {
        Pots.readAll();
        buttons.handleButtons();
        while (queue_try_remove(&sysex_fifo, &raw_sysex))
        {
            handleMidi(raw_sysex);
        }
    }

    return 0;
}