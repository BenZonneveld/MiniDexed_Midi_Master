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

/*------------- MAIN -------------*/
int main(void)
{
    queue_init(&tg_fifo, sizeof(dexed_t), FIFOLENGTH);
    queue_init(&midi_fifo, sizeof(dexed_t), FIFOLENGTH);

    cMenu menu;
    board_init();
    set_sys_clock_khz(133000, true);

    stdio_init_all();
    printf("MDMA Booting\r\n");
    
    multicore_launch_core1(midicore);
 
    Pots.init();

    menu.Init();
    menu.Show();

    while (1)
    {
        Pots.readAll();
        buttons.handleButtons();
    }

    return 0;
}