#include "Pots/Pots.h"
#include "Buttons/Buttons.h"
#include "Screen/Screen.h"
#include "Dexed/tg.h"

extern cPots Pots;
extern cButtons buttons;
extern cScreen screen;
extern cTG dexed[8];
static bool led_usb_state;
static bool led_uart_state;
