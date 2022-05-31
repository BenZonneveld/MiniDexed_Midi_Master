/*
 */
#include <stdio.h>
#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"
//#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"

#include "MidiCore.h"
#include "gpio_pins.h"

#include "Pots/Pots.h"
#include "Buttons/Buttons.h"
#include "Menu/MainMenu.h"
#include "Dexed/TG.h"
#include "hagl.h"
#include "hagl_hal.h"

 //--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

//#define IGNORE_MIDI_CC

cPots Pots = cPots();
cButtons buttons = cButtons();
cTG dexed[8] = { cTG() };

/*------------- MAIN -------------*/
int main(void)
{
    cMenu menu;
    board_init();
    set_sys_clock_khz(133000, true);

    stdio_init_all();
    printf("MDMA Booting\r\n");
    
    multicore_launch_core1(midicore);
 
    Pots.init();

    menu.Init();
    menu.ShowMenu();
    menu.ShowMenu();

    while (1)
    {
        Pots.readAll();
        buttons.handleButtons();
        hagl_flush();
    }

    return 0;
}