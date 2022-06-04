/*
 */
#include <stdio.h>
#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"
//#include "hardware/uart.h"
#include "pico/util/queue.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"

#include "MidiCore.h"
#include "gpio_pins.h"

#include "Pots/Pots.h"
#include "Buttons/Buttons.h"
#include "Menu/MainMenu.h"
#include "Dexed/TG.h"
//#include "hagl.h"
//#include "hagl_hal.h"
#include "MDMA.h"
 //--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

//#define IGNORE_MIDI_CC

cPots Pots = cPots();
cButtons buttons = cButtons();
cTG dexed[8];

queue_t tg_fifo;
queue_t midi_fifo;

/*------------- MAIN -------------*/
int main(void)
{
    dexed_t tgdata;

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

//    tgdata.channel = 0;
    while (1)
    {
        Pots.readAll();
        buttons.handleButtons();
//        queue_add_blocking(&tg_fifo, &tgdata);
//        printf("added queue data %i\n", tgdata.channel);
//        tgdata.channel++;
    }

    return 0;
}