#include <stdlib.h>
#include <stdio.h>

#include "bsp/board.h"
#include "tusb.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/util/queue.h"
#include "MidiCore.h"
#include "gpio_pins.h"

//#define DEBUGSYSEX
//#define DEBUGMIDI
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
    static bool SysExState = false;
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
        for (size_t i = 0; i < buf_valid; i++)
        {
            // Todo Detect bank and program changes
#ifdef DEBUGMIDI
            if ((buffer[i] & 80) == 0x80) printf("\n%02X, ", buffer[i]);
            else printf("%02X, ", buffer[i]);
#endif
            if (SysExState == true)
            {
                sysex_buf.length++;
                if (sysex_buf.length <= SYSEX_BUFFER)
                    sysex_buf.buffer[sysex_buf.length] = buffer[i];
#ifdef DEBUGSYSEX
                printf("%02X, ", buffer[i]);
                if ((sysex_buf.length +1) % 16 == 0) printf("\n");
#endif
            }
            if (buffer[i] == 0xF0)
            {
                SysExState = true;
                sysex_buf.length = 0;
                sysex_buf.buffer[sysex_buf.length] = buffer[i];
#ifdef DEBUGSYSEX
                printf("Start of Sysex\n");
                printf("%02X, ", buffer[i]);
#endif
            }
            if (sysex_buf.length == 3) // Check if this is valid data for us
            {
                if (sysex_buf.buffer[2] == 0x31 ||     // config dump
                    ((sysex_buf.buffer[2] & 0x70) == 10 && (sysex_buf.buffer[3] >> 2) == 2) || // Function parameters
                    ((sysex_buf.buffer[2] & 0x70) == 0 && sysex_buf.buffer[3] == 0)) // Voice dumps
                {
                    ;
                }
                else {
                    SysExState = false;
                }
            }
            if (sysex_buf.buffer[sysex_buf.length] == 0xF7)
            {
                if (SysExState == true)
                {
                    queue_add_blocking(&sysex_fifo, &sysex_buf);
#ifdef DEBUGSYSEX
                    printf("\nEnd of Sysex\n");
                    printf("Sysex size: %i\n", sysex_buf.length);
#endif
                }
                SysExState = false;
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
    uart_init(DEXED, 115200);
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
    int32_t temp = 0;
    printf("rx chan: %i\tinstance: %i cmd: %02X parm: %02X value: %04X\n", mididata.channel, mididata.instance, mididata.cmd, mididata.parm, mididata.value);
    if (mididata.cmd == 1)
    {
        dexedPatchRequest(mididata);
    }
    if (mididata.cmd == 2)
    {
        dexedConfigRequest();
    }
    if ( mididata.cmd == 0 )
    {
        switch (mididata.parm)
        {
        case PBANK:
            dx7sysex(78, mididata);
//            sendCtrl(0x00, mididata);
            break;
        case PPATCH:
            dx7sysex(79, mididata);
//            sendCtrl(128, mididata);
            break;
        case PCHANNEL:
            temp = mididata.value;
            mididata.value = 0;
            sendCtrl(120, mididata);
            sendCtrl(120, mididata);
            mididata.value = temp;
            dx7sysex(80, mididata);
            break;
        case PFREQ:
            dx7sysex(90, mididata);
            break;
        case PRESO:
            dx7sysex(91, mididata);
            break;
        case PVERB:
            dx7sysex(81, mididata);
            break;
        case PCOMP:
            dx7sysex(82, mididata);
            break;
        case PSHIFT:
            dx7sysex(83, mididata); // Handle transpose on the pico
            break;
        case PTUNE:
            dx7sysex(84, mididata);
            break;
        case PPAN:
            dx7sysex(85, mididata);
            break;
        case PVOL:
            dx7sysex(86, mididata);
            break;
        case PBRANGE:
            dx7sysex(87, mididata);
            break;
        case PPMODE:
            dx7sysex(88, mididata);
            break;
        case PMONO:
            dx7sysex(89, mididata);
            break;
        default:
            break;
        }
    }
}

void sendToAllPorts(uint8_t *message, uint8_t len)
{
//    printf("Send to all ports\t");
#ifdef DEBUGSYSEX
    for (size_t i = 0; i < len; i++)
    {
        tud_midi_stream_write(0, message+i, 1);
        printf("%02X, ", message[i]);
        if (i % 16 == 15) printf("\n");
    }
    printf("\n");
#endif
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

void dexedConfigRequest()
{
    uint8_t config_req[4] = {
        0xF0,
        0x43,
        0x30,
        0xF7
    };
    sendToAllPorts(config_req, 4);
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