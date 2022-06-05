#include <stdlib.h>
#include <stdio.h>

#include "bsp/board.h"
#include "tusb.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/util/queue.h"
#include "MidiCore.h"
#include "gpio_pins.h"
#include "MIDI/midi.h"
#include "TG.h"

queue_t midi_fifo;
bool led_usb_state = false;
bool led_uart_state = false;

//--------------------------------------------------------------------+
// UART Helper
//--------------------------------------------------------------------+

static size_t
__time_critical_func(uart_read) (uart_inst_t* uart, uint8_t* dst, size_t maxlen)
{
    size_t result = 0;

    while (maxlen && uart_is_readable(uart))
    {
        *dst++ = uart_getc(uart);

        result++;
        maxlen--;
    }

    return result;
}

//--------------------------------------------------------------------+
// MIDI Task
//--------------------------------------------------------------------+

void midi_task(void)
{
    if (!tud_midi_mounted())
    {
        return;
    }

    // Handle USB to UART direction
    uint8_t packet[4];
    if (tud_midi_available() && uart_is_writable(DEXED))
    {
        tud_midi_packet_read(packet);

        static const size_t cin_to_length[] =
        { 0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1 };

        uint8_t cid = packet[0] & 0xF;
#ifdef IGNORE_MIDI_CC
        if (cid != 0x0B && cid != 0x0C)
#endif
        {
            size_t length = cin_to_length[cid];
            if (length)
            {
                uart_write_blocking(DEXED, packet + 1, length);
            }
        }

        led_usb_state = true;
    }
    else
    {
        led_usb_state = false;
    }

    // Handle UART to USB direction
    static uint8_t buffer[CFG_TUD_MIDI_TX_BUFSIZE / 4 * 3];
    static uint8_t buf_pos = 0;
    static uint8_t buf_valid = 0;

    size_t length = buf_valid - buf_pos;
    if (length)
    {
        buf_pos += tud_midi_stream_write(0, buffer + buf_pos, length);
        if (buf_pos < buf_valid)
        {
            return;
        }
    }

    buf_valid = uart_read(DEXED, buffer, sizeof buffer);
    if (buf_valid)
    {
        buf_pos = tud_midi_stream_write(0, buffer, buf_valid);

        led_uart_state = true;
    }
    else
    {
        buf_pos = 0;

        led_uart_state = false;
    }
}

//--------------------------------------------------------------------+
// LED Task
//--------------------------------------------------------------------+

void led_task(void)
{
    static uint32_t last_active_ms = 0;

    if (led_usb_state || led_uart_state)
    {
        board_led_write(true);

        last_active_ms = board_millis();
    }
    else if (board_millis() - last_active_ms > 10)
    {
        board_led_write(false);
    }
}

void midicore()
{
    dexed_t mididata;
    printf("MidiCore Launched on core 1:\r\n");
    tusb_init();

    printf("tusb_init done");
    // Initialise UART 0
//  uart_init(MIDIPORT, 31250);
    uart_init(DEXED, 31250);
    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
  //  gpio_set_function(TX0, GPIO_FUNC_UART);
  //  gpio_set_function(RX0, GPIO_FUNC_UART);
    gpio_set_function(TX1, GPIO_FUNC_UART);
    gpio_set_function(RX1, GPIO_FUNC_UART);

    printf(", UARTS Setup entering task loop\r\n");
    while (1)
    {
        tud_task();   // tinyusb device task
        led_task();
        midi_task();
        while (queue_try_remove(&tg_fifo, &mididata))
        {
//            if ( mididata.channel == 0 || mididata.channel == 0x80)
                
                printf("rx chan: %i\tinstance: %i cmd: %02X data1: %02X data2: %02X value: %04X\n", mididata.channel, mididata.instance, mididata.cmd, mididata.val1, mididata.val2,mididata.data);
        }
    }
}

