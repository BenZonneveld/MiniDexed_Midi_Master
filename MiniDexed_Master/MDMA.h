#include "Pots/Pots.h"
#include "Buttons/Buttons.h"
#include "Dexed/tg.h"
#include "GFX-lib/TFT.h"

extern cPots Pots;
extern cButtons buttons;
extern cTG dexed[8];
extern TFT tft;
static bool led_usb_state;
static bool led_uart_state;
