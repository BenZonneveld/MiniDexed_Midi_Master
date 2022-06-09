#include <stdlib.h>
#include <stdio.h>

#include "bsp/board.h"
#include "tusb.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/util/queue.h"
#include "MidiCore.h"
#include "gpio_pins.h"
//#include "MIDI/midi.h"

queue_t midi_fifo;
queue_t sysex_fifo;
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
    static bool SEX = false;
    static sysex_t sysex_buf;

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
//        printf("buf valid: %i\n", buf_valid);
        for (size_t i = 0; i < buf_valid; i++)
        {
            if (SEX == true)
            {
                sysex_buf.length++;
                if (sysex_buf.length <= VOICEDATA_SIZE)
                    sysex_buf.buffer[sysex_buf.length] = buffer[i];
//                printf("%02X, ", buffer[i]);
//                if ((sysex_buf.length +1) % 16 == 0) printf("\n");
            }
            if (buffer[i] == 0xF0)
            {
                SEX = true;
                sysex_buf.length = 0;
                sysex_buf.buffer[sysex_buf.length] = buffer[i];
 //               printf("Start of Sysex\n");
 //              printf("%02X, ", buffer[i]);
            }
            if (buffer[i] == 0xF7)
            {
                if (SEX == true)
                {
                    if (sysex_buf.length <= VOICEDATA_SIZE)
                    {
                        queue_add_blocking(&sysex_fifo, &sysex_buf);
//                        printf("End of Sysex\n");
//                        printf("Sysex size: %i\n", sysex_buf.length);
                    }
                }
                SEX = false;
            }
        }
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
#ifndef MIDIPORT
    printf("tusb_init done");
#endif
    // Initialise UARTs
    uart_init(DEXED, 31250);
    gpio_set_function(TX1, GPIO_FUNC_UART);
    gpio_set_function(RX1, GPIO_FUNC_UART);
#ifdef MIDIPORT
    uart_init(MIDIPORT, 31250);
    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(TX0, GPIO_FUNC_UART);
    gpio_set_function(RX0, GPIO_FUNC_UART);
#else
    printf(", UARTS Setup entering task loop\r\n");
#endif
    while (1)
    {
        tud_task();   // tinyusb device task
        led_task();
        midi_task();
        while (queue_try_remove(&tg_fifo, &mididata))
        {
            dispatcher(mididata);
        }
    }
}

void dispatcher(dexed_t mididata)
{
    printf("rx chan: %i\tinstance: %i cmd: %02X parm: %02X value: %04X\n", mididata.channel, mididata.instance, mididata.cmd, mididata.parm, mididata.value);
    if (mididata.cmd == 1)
    {
        dexedPatchRequest(mididata);
    }
    else {
        switch (mididata.parm)
        {
        case PBANK:
            sendCtrl(0x00, mididata);
            break;
        case PPATCH:
            sendCtrl(128, mididata);
            break;
        case PCHANNEL:
            dx7sysex(80, mididata);
            break;
        case PFREQ:
            sendCtrl(74, mididata);
            break;
        case PRESO:
            sendCtrl(71, mididata);
            break;
        case PVERB:
            dx7sysex(81, mididata);
            break;
        case PCOMP:
            dx7sysex(82, mididata);
            break;
        case PTRANS:
            dx7sysex(83, mididata); // Handle transpose on the pico
            break;
        case PTUNE:
            dx7sysex(84, mididata);
            //                sendCtrl(94, mididata + 99);
            break;
        case PPAN:
            break;
            dx7sysex(85, mididata);
            //                sendCtrl(10, mididata + 99);
        case PVOL:
            dx7sysex(86, mididata);
            //                sendCtrl(7, mididata);
            break;
        case PBEND:
            dx7sysex(87, mididata);
            break;
        case PPORTA:
            dx7sysex(88, mididata);
        default:
            break;
        }
    }
}

void sendToAllPorts(uint8_t *message, uint8_t len)
{
//    printf("Send to all ports\t");
    for (size_t i = 0; i < len; i++)
    {
        tud_midi_stream_write(0, message+i, 1);
//        printf("%02X, ", message[i]);
    }
//    printf("\n");
//    tud_midi_stream_write(0, message, len); // Should be serial !!!
    uart_write_blocking(DEXED, message, len);
#ifdef MIDIPORT
    uart_write_blocking(MIDIPORT, messages, len);
#endif
}

void dx7sysex(uint16_t parm, dexed_t mididata)
{
    // 0xF0   Start Sysex
    // 0x43   ID # i = 67; Yamaha
    // 0x10     Sub status ( s = 1 & channel number ; n = 0; ch 1;
    // 0x02   Parameter group g=0; voice; g=2; function, we want function
    // parm_no  78 and up
    // xx       data byte
    // 0xF7   end sysex
    uint8_t val1 = mididata.value & 0x7f;
    uint8_t pgrp = (0x02<<2)|((parm >> 7) & 0x03);
        uint8_t prm = parm & 0x7F;
//    uint8_t val2 = (mididata.value >> 7) & 0x7f;
    uint8_t sysex[7] = { 
        0xF0,  // Start sysex
        0x43,  // ID #: Yamaha
        (0x10|(mididata.instance&0xF)), // Sub status s=1 & channel number
        pgrp,
        prm,
        val1,
        0xF7 };
    sendToAllPorts(sysex, 7);
}

void dexedPatchRequest(dexed_t mididata)
{
    uint8_t voice_req[5] = {
        0xF0,
        0x43,
        0x20 | (mididata.instance & 0xF),
        mididata.value,
        0xF7
    };
    sendToAllPorts(voice_req,5);
}

void sendCtrl(uint8_t ctrl, dexed_t mididata)
{
    uint8_t val1 = mididata.value & 0x7f;
    uint8_t val2 = (mididata.value >> 7) & 0x7f;

    if (mididata.parm == PBANK)
    {
        uint8_t bankmsb[3] = { 0xB0 | (mididata.channel & 0xF), 0, val2 };
        sendToAllPorts(bankmsb, 3);
        uint8_t banklsb[3] = { 0xB0 | (mididata.channel & 0xF), 32, val1 };
        sendToAllPorts(banklsb, 3);
    }
    else if (mididata.parm == PPATCH)
    {
        uint8_t pgmchange[2] = { 0xC0 | (mididata.channel & 0xF), val1 };
        sendToAllPorts(pgmchange, 2);
    } else {
        uint8_t ctrlchange[3] = { 0xB0 | (mididata.channel & 0xF), ctrl, val1 };
        sendToAllPorts(ctrlchange, 3);
    }
}