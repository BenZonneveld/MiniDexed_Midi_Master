#include "Pots/Pots.h"
#include "Buttons/Buttons.h"
#include "Dexed/tg.h"
#include "pico/util/queue.h"

#define FIFOLENGTH 8

extern queue_t tg_fifo;
extern queue_t midi_fifo;

#define FIFOLENGTH 16

extern cPots Pots;
extern cButtons buttons;
extern cTG dexed[8];
