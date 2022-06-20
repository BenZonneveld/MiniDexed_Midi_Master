#include "pico/time.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <inttypes.h>
#include <vector>
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "sph0645.pio.h"
#include <algorithm>
#include "hardware/dma.h"

#include "gpio_pins.h"
#include "i2s_input.h"
#include "midicore.h"


