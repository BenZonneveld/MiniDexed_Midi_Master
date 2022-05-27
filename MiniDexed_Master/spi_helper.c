#include <stdio.h>
#include <stdlib.h>

#include <hardware/gpio.h>
#include "gpio_pins.h"

void spiAllHigh()
{
    gpio_put(LCDCS, 1);
    gpio_put(SD_CS, 1);
    gpio_put(KEY_CS, 0);
}

void spi_cs(uint8_t port)
{
    spiAllHigh();
    switch (port)
    {
    case 0:
        gpio_put(KEY_CS, 1);
        break;
    case 1:
        gpio_put(SD_CS, 0);
        break;
    case 2:
        gpio_put(LCDCS, 0);
        break;
    default:
        break;
    }
}
