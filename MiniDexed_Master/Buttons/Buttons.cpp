#include <stdio.h>
#include <pico/stdlib.h>

#include <hardware/spi.h>
#include "hardware/gpio.h"

#include "Buttons.h"
#include "gpio_pins.h"
#include "spi_helper.h"

cButtons::cButtons()
{
	cs = KEY_CS;
	gpio_init(cs);
	gpio_set_dir(cs, GPIO_OUT);
	gpio_put(cs, 1);
}

uint8_t cButtons::getState()
{
	return state;
}

void cButtons::getButtons()
{
	uint8_t buf = 0;
	spi_cs(csKEY);
	uint8_t bytesread = spi_read_blocking(spi1, 0, &buf, 1);
	state = buf & 0xF0 | (buf & 0x8) >> 3| (buf&0x4) >> 1| (buf&0x2)<<1 | (buf&0x1)<< 3;
//	printf("%02X", buf);
	spiAllHigh();
}