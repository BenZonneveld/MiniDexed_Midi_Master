/*
  machine_i2s.c -
    I2S digital audio input C library for the Raspberry Pi Pico RP2040

    Copyright (C) 2022 Sfera Labs S.r.l. - All rights reserved.

    For information, see:
    http://www.sferalabs.cc/

  This code is adapted from the I2S implementation of the RP2 MicroPython port
  by Mike Teachman, available at:
  https://github.com/micropython/micropython/blob/master/ports/rp2/machine_i2s.c
  Retrieved on January 25 2022.
*/

/* Original header */

/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Mike Teachman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#include "i2s_reader.pio.h"

#define MAX_I2S_RP2 (2)

// The DMA buffer size was empirically determined.  It is a tradeoff between:
// 1. memory use (smaller buffer size desirable to reduce memory footprint)
// 2. interrupt frequency (larger buffer size desirable to reduce interrupt frequency)
#define SIZEOF_DMA_BUFFER_IN_WORDS (384)
#define SIZEOF_HALF_DMA_BUFFER_IN_WORDS (SIZEOF_DMA_BUFFER_IN_WORDS / 2)
#define I2S_NUM_DMA_CHANNELS (2)

#define IN_SHIFT    32 // Needs to be 32 for stereo, 16 msb for left, 16 lsb for right.

#define mp_hal_pin_obj_t uint
#define m_new(type, num) ((type *)(malloc(sizeof(type) * (num))))
#define m_new_obj(type) (m_new(type, 1))

typedef struct _ring_buf_t {
    uint32_t *buffer;
    size_t head;
    size_t tail;
    size_t size;
} ring_buf_t;

typedef struct _machine_i2s_obj_t {
    uint8_t i2s_id;
    mp_hal_pin_obj_t sck;
    mp_hal_pin_obj_t ws;
    mp_hal_pin_obj_t sd;
    int32_t ibuf;
    PIO pio;
    uint8_t sm;
    const pio_program_t *pio_program;
    uint prog_offset;
    uint dma_channel[I2S_NUM_DMA_CHANNELS];
    uint32_t dma_buffer[SIZEOF_DMA_BUFFER_IN_WORDS];
    ring_buf_t ring_buffer;
    uint32_t *ring_buffer_storage;
} machine_i2s_obj_t;

// Buffer protocol
typedef struct _mp_buffer_info_t {
    uint32_t *buf;      // can be NULL if len == 0
    size_t len;     // in bytes
} mp_buffer_info_t;

static machine_i2s_obj_t* machine_i2s_obj[MAX_I2S_RP2] = {NULL, NULL};

static const PIO pio_instances[NUM_PIOS] = {pio0, pio1};

static void dma_irq0_handler(void);
static void dma_irq1_handler(void);

// Ring Buffer
// Thread safe when used with these constraints:
// - Single Producer, Single Consumer
// - Sequential atomic operations
// One byte of capacity is used to detect buffer empty/full

static void ringbuf_init(ring_buf_t *rbuf, uint32_t *buffer, size_t size) {
    rbuf->buffer = buffer;
    rbuf->size = size;
    rbuf->head = 0;
    rbuf->tail = 0;
}

static bool ringbuf_push(ring_buf_t *rbuf, uint32_t data) {
    size_t next_tail = (rbuf->tail + 1) % rbuf->size;

    if (next_tail != rbuf->head) {
        rbuf->buffer[rbuf->tail] = data;
        rbuf->tail = next_tail;
        return true;
    }

    // full
    return false;
}

static bool ringbuf_pop(ring_buf_t *rbuf, uint32_t *data) {
    if (rbuf->head == rbuf->tail) {
        // empty
        return false;
    }

    *data = rbuf->buffer[rbuf->head];

    rbuf->head = (rbuf->head + 1) % rbuf->size;
    return true;
}

static bool ringbuf_is_empty(ring_buf_t *rbuf) {
    return rbuf->head == rbuf->tail;
}

static bool ringbuf_is_full(ring_buf_t *rbuf) {
    return ((rbuf->tail + 1) % rbuf->size) == rbuf->head;
}

static size_t ringbuf_available_data(ring_buf_t *rbuf) {
    return (rbuf->tail - rbuf->head + rbuf->size) % rbuf->size;
}

static size_t ringbuf_available_space(ring_buf_t *rbuf) {
    return rbuf->size - ringbuf_available_data(rbuf) - 1;
}

static uint32_t fill_appbuf_from_ringbuf(machine_i2s_obj_t *self, mp_buffer_info_t *appbuf) {

    // copy audio samples from the ring buffer to the app buffer
    // loop, copying samples until the app buffer is filled
    // For uasyncio mode, the loop will make an early exit if the ring buffer becomes empty
    // Example:
    //   a MicroPython I2S object is configured for 16-bit mono (2 bytes per audio sample).
    //   For every frame coming from the ring buffer (8 bytes), 2 bytes are "cherry picked" and
    //   copied to the supplied app buffer.
    //   Thus, for every 1 byte copied to the app buffer, 4 bytes are read from the ring buffer.
    //   If a 8kB app buffer is supplied, 32kB of audio samples is read from the ring buffer.

    uint32_t num_words_copied_to_appbuf = 0;
    uint32_t *app_p = (uint32_t *)appbuf->buf;
//    uint8_t appbuf_sample_size_in_bytes = (self->bits == 16? 2 : 4) * 2;
    uint32_t num_words_needed_from_ringbuf = appbuf->len;
    while (num_words_needed_from_ringbuf) 
    {
        while (ringbuf_pop(&self->ring_buffer, app_p) == false)
        {
            asm volatile(
                "mov  r0, #01\n"    		// 1 cycle
                "loop1: sub  r0, r0, #1\n"	// 1 cycle
                "bne   loop1\n"          	// 2 cycles if loop taken, 1 if not
                );
        }
        num_words_copied_to_appbuf++;
        num_words_needed_from_ringbuf--;
        app_p += 1;
    }
exit:
    return num_words_copied_to_appbuf;
}

// function is used in IRQ context
static void empty_dma(machine_i2s_obj_t *self, uint32_t *dma_buffer_p) {
    // when space exists, copy samples into ring buffer
    if (ringbuf_available_space(&self->ring_buffer) >= SIZEOF_HALF_DMA_BUFFER_IN_WORDS) {
//        printf("Empty DMA\n");
        for (uint32_t i = 0; i < SIZEOF_HALF_DMA_BUFFER_IN_WORDS; i++) {
            ringbuf_push(&self->ring_buffer, dma_buffer_p[i]);
        }
    }
}

static void irq_configure(machine_i2s_obj_t *self) {
    if (self->i2s_id == 0) {
        irq_set_exclusive_handler(DMA_IRQ_0, dma_irq0_handler);
        irq_set_enabled(DMA_IRQ_0, true);
    } else {
        irq_set_exclusive_handler(DMA_IRQ_1, dma_irq1_handler);
        irq_set_enabled(DMA_IRQ_1, true);
    }
}

static int pio_configure(machine_i2s_obj_t *self) {
//    self->pio_program = &pio_read_32;
    self->pio_program = &i2s_reader_program;

    // find a PIO with a free state machine and adequate program space
    PIO candidate_pio;
    bool is_free_sm;
    bool can_add_program;
    for (uint8_t p = 0; p < NUM_PIOS; p++) {
        candidate_pio = pio_instances[p];
        is_free_sm = false;
        can_add_program = false;

        for (uint8_t sm = 0; sm < NUM_PIO_STATE_MACHINES; sm++) {
            if (!pio_sm_is_claimed(candidate_pio, sm)) {
                is_free_sm = true;
                break;
            }
        }

        if (pio_can_add_program(candidate_pio,  self->pio_program)) {
            can_add_program = true;
        }

        if (is_free_sm && can_add_program) {
            break;
        }
    }

    if (!is_free_sm) {
        return -1;
    }

    if (!can_add_program) {
        return -2;
    }

    self->pio = candidate_pio;
    self->sm = pio_claim_unused_sm(self->pio, false);
    self->prog_offset = pio_add_program(self->pio, self->pio_program);
    pio_sm_init(self->pio, self->sm, self->prog_offset, NULL);

    pio_sm_config config = pio_get_default_sm_config();

    //float pio_freq = self->rate *
    //    2 *
    //    DMA_BITS *
    //    PIO_INSTRUCTIONS_PER_BIT;
    //float clkdiv = clock_get_hz(clk_sys) / pio_freq;
    //printf("pio_freq: %.3f\nclockdiv: %.3f\n", pio_freq, clkdiv);
    sm_config_set_clkdiv(&config, 1.0f);

    sm_config_set_in_pins(&config, self->sd);
    sm_config_set_in_shift(&config, false, true, IN_SHIFT);
    sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_RX);  // double RX FIFO size

    sm_config_set_sideset(&config, 2, false, false);
    sm_config_set_sideset_pins(&config, self->sck);
//    sm_config_set_wrap(&config, self->prog_offset, self->prog_offset + self->pio_program->length - 1);
    pio_sm_set_config(self->pio, self->sm, &config);

    return 0;
}

static void gpio_init_i2s(PIO pio, uint8_t sm, mp_hal_pin_obj_t pin_num, uint8_t pin_val, uint8_t pin_dir) {
    uint32_t pinmask = 1 << pin_num;
    pio_sm_set_pins_with_mask(pio, sm, pin_val << pin_num, pinmask);
    pio_sm_set_pindirs_with_mask(pio, sm, pin_dir << pin_num, pinmask);
    pio_gpio_init(pio, pin_num);
}

static void gpio_configure(machine_i2s_obj_t* self) {
    gpio_init_i2s(self->pio, self->sm, self->sck, 0, 0);
    gpio_init_i2s(self->pio, self->sm, self->ws, 0, 0);
    gpio_init_i2s(self->pio, self->sm, self->sd, 0, 0);
}

// determine which DMA channel is associated to this IRQ
static int dma_map_irq_to_channel(uint irq_index) {
    for (uint ch = 0; ch < NUM_DMA_CHANNELS; ch++) {
        if ((dma_irqn_get_channel_status(irq_index, ch))) {
            return ch;
        }
    }
    // This should never happen
    return -1;
}

// note:  first DMA channel is mapped to the top half of buffer, second is mapped to the bottom half
static uint32_t *dma_get_buffer(machine_i2s_obj_t *i2s_obj, uint channel) {
    for (uint8_t ch = 0; ch < I2S_NUM_DMA_CHANNELS; ch++) {
        if (i2s_obj->dma_channel[ch] == channel) {
            return i2s_obj->dma_buffer + (SIZEOF_HALF_DMA_BUFFER_IN_WORDS * ch);
        }
    }
    // This should never happen
    return NULL;
}

static int dma_configure(machine_i2s_obj_t *self) {
    uint8_t num_free_dma_channels = 0;
    for (uint8_t ch = 0; ch < NUM_DMA_CHANNELS; ch++) {
        if (!dma_channel_is_claimed(ch)) {
            num_free_dma_channels++;
        }
    }
    if (num_free_dma_channels < I2S_NUM_DMA_CHANNELS) {
        return -1;
    }

    for (uint8_t ch = 0; ch < I2S_NUM_DMA_CHANNELS; ch++) {
        self->dma_channel[ch] = dma_claim_unused_channel(false);
    }

    // The DMA channels are chained together.  The first DMA channel is used to access
    // the top half of the DMA buffer.  The second DMA channel accesses the bottom half of the DMA buffer.
    // With chaining, when one DMA channel has completed a data transfer, the other
    // DMA channel automatically starts a new data transfer.

    for (uint8_t ch = 0; ch < I2S_NUM_DMA_CHANNELS; ch++) {
        dma_channel_config dma_config = dma_channel_get_default_config(self->dma_channel[ch]);
        channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
        channel_config_set_chain_to(&dma_config, self->dma_channel[(ch + 1) % I2S_NUM_DMA_CHANNELS]);

        uint32_t* dma_buffer = self->dma_buffer + (SIZEOF_HALF_DMA_BUFFER_IN_WORDS * ch);

        channel_config_set_dreq(&dma_config, pio_get_dreq(self->pio, self->sm, false));
        channel_config_set_read_increment(&dma_config, false);
        channel_config_set_write_increment(&dma_config, true);
        dma_channel_configure(self->dma_channel[ch],
            &dma_config,
            dma_buffer,                                             // dest = DMA buffer
            (void*)&self->pio->rxf[self->sm],                      // src = PIO RX FIFO
            SIZEOF_HALF_DMA_BUFFER_IN_WORDS,
            false);

    }

    for (uint8_t ch = 0; ch < I2S_NUM_DMA_CHANNELS; ch++) {
        dma_irqn_acknowledge_channel(self->i2s_id, self->dma_channel[ch]);  // clear pending.  e.g. from SPI
        dma_irqn_set_channel_enabled(self->i2s_id, self->dma_channel[ch], true);
    }

    return 0;
}

static void dma_irq_handler(uint8_t irq_index) {
    int dma_channel = dma_map_irq_to_channel(irq_index);
    if (dma_channel == -1) {
        // This should never happen
        return;
    }

    machine_i2s_obj_t* self = machine_i2s_obj[irq_index];
    if (self == NULL) {
        // This should never happen
        return;
    }

    uint32_t* dma_buffer = dma_get_buffer(self, dma_channel);
    if (dma_buffer == NULL) {
        // This should never happen
        return;
    }

//    printf("Handling DMA Channel %i\n", dma_channel);
//    for (size_t i = 0; i < SIZEOF_DMA_BUFFER_IN_BYTES/2; i++)
//    {
//        printf("%08X, ", dma_buffer[i]);
//    }
//    printf("\n");
    empty_dma(self, dma_buffer);
    dma_irqn_acknowledge_channel(irq_index, dma_channel);
    dma_channel_set_write_addr(dma_channel, dma_buffer, false);
}

static void dma_irq0_handler(void) {
    dma_irq_handler(0);
}

static void dma_irq1_handler(void) {
    dma_irq_handler(1);
}

static int machine_i2s_init_helper(machine_i2s_obj_t *self,
              mp_hal_pin_obj_t sck, mp_hal_pin_obj_t ws, mp_hal_pin_obj_t sd,
              int32_t ring_buffer_len) {
    //
    // ---- Check validity of arguments ----
    //

    // does WS pin follow SCK pin?
    // note:  SCK and WS are implemented as PIO sideset pins.  Sideset pins must be sequential.
    if (ws != (sck - 1)) {
        return -1;
    }

    // is Ibuf valid?
    if (ring_buffer_len > 0) {
        self->ring_buffer_storage = m_new(uint32_t, ring_buffer_len);
        ;
        ringbuf_init(&self->ring_buffer, self->ring_buffer_storage, ring_buffer_len);
    } else {
        return -5;
    }

    self->sck = sck;
    self->ws = ws;
    self->sd = sd;
    self->ibuf = ring_buffer_len;

    irq_configure(self);
    int err = pio_configure(self);
    if (err != 0) {
        return err;
    }
    gpio_configure(self);
    err = dma_configure(self);
    if (err != 0) {
        return err;
    }

    pio_sm_set_enabled(self->pio, self->sm, true);
    dma_channel_start(self->dma_channel[0]);

    return 0;
}

static machine_i2s_obj_t* machine_i2s_make_new(uint8_t i2s_id,
              mp_hal_pin_obj_t sck, mp_hal_pin_obj_t ws, mp_hal_pin_obj_t sd,
              int32_t ring_buffer_len) {
    if (i2s_id >= MAX_I2S_RP2) {
        return NULL;
    }

    machine_i2s_obj_t *self;
    if (machine_i2s_obj[i2s_id] == NULL) {
        self = m_new_obj(machine_i2s_obj_t);
        machine_i2s_obj[i2s_id] = self;
        self->i2s_id = i2s_id;
    } 

    if (machine_i2s_init_helper(self, sck, ws, sd, ring_buffer_len) != 0) {
        return NULL;
    }
    return self;
}

static int machine_i2s_stream_read(machine_i2s_obj_t *self, uint32_t *buf_in, size_t size) {
    //uint8_t appbuf_sample_size_in_words = (self->bits / 8) * 2;
    //if (size % appbuf_sample_size_in_words != 0) {
    //    printf("size: %i app_buf_sample_size_in_bytes %i\n", size, appbuf_sample_size_in_bytes);
    //    return -2;
    //}

    if (size == 0) {
        return 0;
    }

    mp_buffer_info_t appbuf;
    appbuf.buf = (uint32_t *)buf_in;
    appbuf.len = size;
    uint32_t num_words_read = fill_appbuf_from_ringbuf(self, &appbuf);
    return num_words_read;
}
