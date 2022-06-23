#include "Pots/Pots.h"
#include "Buttons/Buttons.h"
#include "Dexed/tg.h"
#include "pico/util/queue.h"
#include "Menu/MainMenu.h"
#define FIFOLENGTH 16

extern queue_t tg_fifo;
extern queue_t midi_fifo;

extern cPots Pots;
extern cButtons buttons;
extern cTG dexed[8];
extern cMenu menu;