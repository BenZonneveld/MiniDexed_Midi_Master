#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/adc.h>
#include "Pots.h"
#include "gpio_pins.h"

std::array<void(*)(uint8_t), 3>cPots::potCallback;

void cPots::init()
{
	adc_gpio_init(ADC0);
	adc_gpio_init(ADC1);
	adc_gpio_init(ADC2);
	adc_init();
	adc_set_clkdiv(0);
};

void cPots::capture(uint8_t channel)
{
	updated[channel] = false;
	uint32_t current = 0;
	adc_select_input(channel);


	for (uint16_t i = 0; i < CAPTURE_DEPTH; ++i) {
		current += adc_read();
	}
	current = current / (CAPTURE_DEPTH);
	if (current != pot[channel])
	{
		uint16_t deviation = 0;
		if (current > pot[channel])
		{
			deviation = current - pot[channel];
		}
		else {
			deviation = pot[channel] - current;
		}

		if (deviation < MIN_DEVIATION)
			return;
		pot[channel] = (uint16_t)current;
		updated[channel] = true;
		if (potCallback[channel] != nullptr) potCallback[channel](channel);
	}
	else {
		updated[channel] = false;
	}
}

void cPots::readAll()
{
	capture(0);
	capture(1);
	capture(2);
}

uint16_t cPots::getPot(uint8_t id)
{
	return pot[id];
}

void cPots::setPotCallback(uint8_t pot, void(*callback)(uint8_t pot))
{
	potCallback[pot] = callback;
}
