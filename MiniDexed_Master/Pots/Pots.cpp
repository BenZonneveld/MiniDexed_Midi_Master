#include <stdio.h>
#include <pico/stdlib.h>

#include <hardware/adc.h>
#include <hardware/dma.h>

#include "Pots.h"
#include "gpio_pins.h"

std::array<void(*)(), 3>cPots::potCallback;

cPots::cPots()
{
	adc_gpio_init(ADC0);
	adc_gpio_init(ADC1);
	adc_gpio_init(ADC2);

	adc_init();
	adc_select_input(0);
	adc_fifo_setup(
		true,    // Write each completed conversion to the sample FIFO
		true,    // Enable DMA data request (DREQ)
		1,       // DREQ (and IRQ) asserted when at least 1 sample present
		false,   // We won't see the ERR bit because of 8 bit reads; disable.
		true     // Shift each sample to 8 bits when pushing to FIFO
	);

	adc_set_clkdiv(0);
};

void cPots::capture(uint8_t channel)
{
	updated[channel] = false;
	uint64_t current = 0;
//	printf("Starting capture\n");
	adc_select_input(channel);

	uint dma_chan = dma_claim_unused_channel(true);
	dma_channel_config cfg = dma_channel_get_default_config(dma_chan);

	// Reading from constant address, writing to incrementing byte addresses
	channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
	channel_config_set_read_increment(&cfg, false);
	channel_config_set_write_increment(&cfg, true);

	// Pace transfers based on availability of ADC samples
	channel_config_set_dreq(&cfg, DREQ_ADC);

	dma_channel_configure(dma_chan, &cfg,
		capture_buf,    // dst
		&adc_hw->fifo,  // src
		CAPTURE_DEPTH,  // transfer count
		true            // start immediately
	);

	adc_run(true);
	// Once DMA finishes, stop any new conversions from starting, and clean up
	// the FIFO in case the ADC was still mid-conversion.
	dma_channel_wait_for_finish_blocking(dma_chan);
//	printf("Capture finished\n");
	adc_run(false);
	adc_fifo_drain();

	for (uint16_t i = 0; i < CAPTURE_DEPTH; ++i) {
		current += capture_buf[i];
	}
	current = current / CAPTURE_DEPTH;
	if (current != pot[channel])
	{
		pot[channel] = (uint8_t)current;
		updated[channel] = true;
		if (potCallback[channel] != nullptr) potCallback[channel]();
	}
	else {
		updated[channel] = false;
	}
	dma_channel_unclaim(dma_chan);
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

void cPots::setPotCallback(uint8_t pot, void(*callback)())
{
	potCallback[pot] = callback;
}
