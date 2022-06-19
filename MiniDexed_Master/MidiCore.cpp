#include <stdlib.h>
#include <stdio.h>

#include "bsp/board.h"
#include "tusb.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/util/queue.h"
#include "MidiCore.h"
#include "gpio_pins.h"

#include "mdma.h"

#include "I2s.h"
#include "tud_audio_callbacks.h"

//#define DEBUGSYSEX
//#define DEBUGMIDI

queue_t midi_fifo;
queue_t sysex_fifo;
queue_t tx_fifo;

bool led_usb_state = false;
bool led_uart_state = false;

I2SClass I2S;
uint16_t buff[APP_BUFFER_SIZE];

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
        midiParser(buffer, buf_valid);
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
#ifdef USE_USB_AUDIO
    sampleFreqRng.wNumSubRanges = 1;
    sampleFreqRng.subrange[0].bMin = AUDIO_SAMPLE_RATE;
    sampleFreqRng.subrange[0].bMax = AUDIO_SAMPLE_RATE;
    sampleFreqRng.subrange[0].bRes = 0;
#endif

    dexed_t mididata;
    sysex_t rawsysex;
    printf("MidiCore Launched on core 1:\r\n");
    tusb_init();
#ifndef MIDIPORT
    printf("tusb_init done");
#endif
    // Setup i2s
    I2S.setSCK(I2S_SCK);
    I2S.setWS(I2S_WS);
    I2S.setSD(I2S_SD);
    I2S.setBufferSize(40000);
    uint8_t i2sState = I2S.begin(I2S_MODE_STEREO, 44100, 16);
    printf("i2s begin returned %i\n", i2sState);

    // Initialise UARTs
    uart_init(DEXED, 230400);
    gpio_set_function(TX1, GPIO_FUNC_UART);
    gpio_set_function(RX1, GPIO_FUNC_UART);
//    uint dexedbaudrate = uart_set_baudrate(DEXED, 230400);
 //   printf("UART to DEXED baudrate: %lu\n", dexedbaudrate);
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

        while (queue_try_remove(&tx_fifo, &rawsysex))
        {
            sendToAllPorts(rawsysex.buffer, rawsysex.length);
        }

        int ret = I2S.read(buff, APP_BUFFER_SIZE);
        if (ret < 0) {
            printf("Microphone read error: %i\n", ret);
            //    sleep_ms(1000);
            //    return;
        }
//        else {
//            for (size_t p = 0; p < ret; p++)
//            {
////                if ( buff[p] != 0 ) printf("%i\n", buff[p]);
//            }
//        }
//        //printf("Read %i bytes\n",ret);
    }
}

void dispatcher(dexed_t mididata)
{
    int32_t temp = 0;
//    printf("rx chan: %i\tinstance: %i cmd: %02X parm: %02X value: %04X\n", mididata.channel, mididata.instance, mididata.cmd, mididata.parm, mididata.value);
    if (mididata.cmd == 1)
    {
        dexedPatchRequest(mididata);
        return;
    }
    if (mididata.cmd == 2)
    {
        dexedConfigRequest();
        return;
    }

    if (mididata.cmd == 3)
    {
        dexedGetBankName(mididata);
        return;
    }
    
    if (mididata.cmd == 4)
    {
        printf("mididata.cmd = %02X\n", mididata.cmd);
        return;
    }

    if ( mididata.cmd == 0 )
    {
        switch (mididata.parm)
        {
        case PBANK:
            dx7sysex(F_BANK, mididata);
            break;
        case PPATCH:
            dx7sysex(F_PATCH, mididata);
            break;
        case PCHANNEL:
            temp = mididata.value;
            mididata.value = 0;
            sendCtrl(123, mididata);
            sendCtrl(120, mididata);
            mididata.value = temp;
            dx7sysex(F_CHANNEL, mididata);
            break;
        case PFREQ:
            dx7sysex(F_FREQ, mididata);
            break;
        case PRESO:
            dx7sysex(F_RESO, mididata);
            break;
        case PVERB:
            dx7sysex(F_VERB, mididata);
            break;
        case PSHIFT:
            dx7sysex(F_SHIFT, mididata); // Handle transpose on the pico
            break;
        case PTUNE:
            dx7sysex(F_TUNE, mididata);
            break;
        case PPAN:
            dx7sysex(F_PAN, mididata);
            break;
        case PVOL:
            dx7sysex(F_VOL, mididata);
            break;
        case PBRANGE:
            dx7sysex(F_BRANGE, mididata);
            break;
        case PPMODE:
            dx7sysex(F_PMODE, mididata);
            break;
        case PMONO:
            dx7sysex(F_MONO, mididata);
            break;
        case PBSTEP:
            dx7sysex(F_BSTEP, mididata);
            break;
        case PGLISS:
            dx7sysex(F_GLISS, mididata);
            break;
        case PTIME:
            dx7sysex(F_TIME, mididata);
            break;
        case PNLOW:
            dx7sysex(F_NLOW, mididata);
            break;
        case PNHIGH:
            dx7sysex(F_NHIGH, mididata);
            break;
        case FCOMP_EN:
            dx7sysex(F_COMP_EN, mididata);
            break;
        case FREV_EN:
            dx7sysex(F_REV_EN, mididata);
            break;
        case FSIZE:
            dx7sysex(F_SIZE, mididata);
            break;
        case FLOWDAMP:
            dx7sysex(F_LOWDAMP, mididata);
            break;
        case FHIGHDAMP:
            dx7sysex(F_HIGHDAMP, mididata);
            break;
        case FLOWPASS:
            dx7sysex(F_LOWPASS, mididata);
            break;
        case FDIFF:
            dx7sysex(F_DIFF, mididata);
            break;
        case FRLEVEL:
            dx7sysex(F_RLEVEL, mididata);
            break;
        default:
            break;
        }
    }
}

void sendToAllPorts(uint8_t *message, uint8_t len)
{
//    printf("Send to all ports\t");
#if SYSEXDEBUG
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
        (uint8_t)(0x10|(mididata.instance&0xF)), // Sub status s=1 & channel number
        pgrp,
        prm,
        val1,
        0xF7 };
    sendToAllPorts(sysex, 7);
}

void dexedGetBankName(dexed_t mididata)
{
    uint8_t bankname_req[4] = {
        0xF0,
        0x43,
        (uint8_t)(0x40 | (mididata.instance & 0xF)),
        0xF7
    };
    printf("Bank Name Request\n");
    sendToAllPorts(bankname_req, 4);
}

void dexedPatchRequest(dexed_t mididata)
{
    uint8_t voice_req[5] = {
        0xF0,
        0x43,
        (uint8_t)(0x20 | (mididata.instance & 0xF)),
        (uint8_t)(mididata.value&0x7F),
        0xF7
    };
#ifdef DEBUGSYSEX
    printf("Send Patch Request\n");
#endif
    sendToAllPorts(voice_req,5);
}

void dexedConfigRequest()
{
//#ifdef DEBUGSYSEX
    printf("Send Config Request\n");
//#endif
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
        uint8_t bankmsb[3] = { (uint8_t)(0xB0 | (mididata.channel & 0xF)), 0, val2 };
        sendToAllPorts(bankmsb, 3);
        uint8_t banklsb[3] = { (uint8_t)(0xB0 | (mididata.channel & 0xF)), 32, val1 };
        sendToAllPorts(banklsb, 3);
    }
    else if (mididata.parm == PPATCH)
    {
        uint8_t pgmchange[2] = { (uint8_t)(0xC0 | (mididata.channel & 0xF)), val1 };
        sendToAllPorts(pgmchange, 2);
    } else {
        uint8_t ctrlchange[3] = { (uint8_t)(0xB0 | (mididata.channel & 0xF)), ctrl, val1 };
        sendToAllPorts(ctrlchange, 3);
    }
}

void midiParser(uint8_t* buffer, size_t length)
{
#ifdef DEBUGMIDI
    static uint8_t bc = 0;
#endif
    for (size_t i = 0; i < length; i++)
    {
        parseCtrls(buffer[i]);
        parseSysex(buffer[i]);
        // Todo Detect bank and program changes
#ifdef DEBUGMIDI
        if (buffer[i] == 0xF0)
        {
            printf("\n");
            bc = 0;
        }
        printf("%02X, ", buffer[i]);
        bc++;
        if (buffer[i] == 0xF7 && bc < 17) printf("\n");
        if (bc == 17 )
        {
            printf("\n");
            bc = 0;
        }
#endif
    }
}

void parseCtrls(uint8_t buf)
{
    static sysex_t ctrl_buf;
    static bool CtrlState = false;
    static uint8_t eventbytes = 0;
    if (CtrlState == true)
    {
        ctrl_buf.length++;
        if (ctrl_buf.length < eventbytes)
            ctrl_buf.buffer[ctrl_buf.length] = buf;
    }
    if ( (buf&0xF0) == 0xC0 || ( buf&0xF0) == 0xB0)
    {
        CtrlState = true;
        if ( (buf&0xf0) == 0xC0)  
            eventbytes = 2;
        else
            eventbytes = 3;

        ctrl_buf.length = 0;
        ctrl_buf.buffer[ctrl_buf.length] = buf;
    }
    if (  ctrl_buf.length == eventbytes-1 ) // 
    {
        if (CtrlState == true)
        {
            queue_add_blocking(&sysex_fifo, &ctrl_buf);
        }
        eventbytes = 0;
        ctrl_buf.length = 0;
        CtrlState = false;
    }
    if (eventbytes > 3)
    {
        eventbytes = 0;
        ctrl_buf.length = 0;
        CtrlState = false;
    }
}

void parseSysex(uint8_t buf)
{
    static sysex_t sysex_buf;
    static bool SysExState = false;
    
    if (SysExState == true)
    {
        sysex_buf.length++;
        if (sysex_buf.length <= SYSEX_BUFFER)
            sysex_buf.buffer[sysex_buf.length] = buf;
#ifdef DEBUGSYSEX
        printf("%02X, ", buf);
        if ((sysex_buf.length + 1) % 16 == 0) printf("\n");
#endif
    }
    if (buf == 0xF0)
    {
        SysExState = true;
        sysex_buf.length = 0;
        sysex_buf.buffer[sysex_buf.length] = buf;
#ifdef DEBUGSYSEX
        printf("Start of Sysex\n");
        printf("%02X, ", buf);
#endif
    }
    if (sysex_buf.length == 3) // Check if this is valid data for us
    {
        if (sysex_buf.buffer[2] == 0x31 ||     // config dump
            ((sysex_buf.buffer[2] & 0x70) == 0x50 && sysex_buf.buffer[3] == 0) || // Bank Name Dump
            ((sysex_buf.buffer[2] & 0x70) == 0x10 && (sysex_buf.buffer[3] >> 2) == 2) || // Function parameters
            ((sysex_buf.buffer[2] & 0x70) == 0 && sysex_buf.buffer[3] == 0)) // Voice dumps
        {
            ;
        }
        else {
            SysExState = false;
#ifdef DEBUGSYSEX
            printf("\naborting\n");
#endif
        }
    }
    if (sysex_buf.buffer[sysex_buf.length] == 0xF7)
    {
#ifdef DEBUGSYSEX
        printf("\n");
#endif
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

void handleMidi(sysex_t raw_sysex)
{
//#ifdef DEBUGSYSEX
    printf("handle sysex: ");
//#endif
    if ((raw_sysex.buffer[0] & 0xF0) == 0xC0) // Program Change
    {
#ifdef DEBUGSYSEX
        printf("Program Change\n");
#endif
        uint8_t instanceID = raw_sysex.buffer[0] & 0xF;
        uint8_t program = raw_sysex.buffer[1];
        if (instanceID < 8)
        {
            dexed[instanceID].setValue(PPATCH, program);
            menu.showTGInfo(PPATCH);
            if (menu.currentMenu() == M_TG_MIDI)
            {
                menu.ShowValue(PPATCH);
            }
#ifdef DEBUGSYSEX
            printf("Program Change Received for instance: %i program %i\n", instanceID, program);
#endif
        }
        dexed[instanceID].getPatch();
        return;
    }
    if ((raw_sysex.buffer[0] & 0xF0) == 0xB0) // Ctrl Change
    {
        printf("Ctrl %02X Change\n", raw_sysex.buffer[1]);
        uint8_t instanceID = raw_sysex.buffer[0] & 0xF;
        uint8_t ctrl = raw_sysex.buffer[1];
        uint8_t val = raw_sysex.buffer[2];
        static int16_t bank;
        static int8_t tune;
        bool setValue = false;
        if (instanceID < 8)
        {
            uint16_t param = 0;
            switch (ctrl)
            {
            case MIDI_CC_VOLUME:
                param = PVOL;
                printf("Volume for instance %i, value: %i\n", instanceID, val);
                setValue = true;
                break;
            case MIDI_CC_PAN_POSITION:
                param = PPAN;
                setValue = true;
                printf("Pan for instance %i, value: %i\n", instanceID, val);
                break;
            case MIDI_CC_BANK_SELECT_LSB:
                param = PBANK;
                setValue = true;
                printf("Bank Select for instance %i, value: %i\n", instanceID, val);
                break;
            case MIDI_CC_BANK_SELECT_MSB:
                param = PBANK;
                bank = dexed[instanceID].getValue(PBANK);
                val = bank + 128 * val;
                setValue = true;
                dexed[instanceID].getBank();
                break;
            case MIDI_CC_RESONANCE:
                param = PRESO;
                printf("Resonance for instance %i, value: %i\n", instanceID, val);
                setValue = true;
                break;
            case MIDI_CC_CUTOFF:
                param = PFREQ;
                printf("Cutoff for instance %i, value: %i\n", instanceID, val);
                setValue = true;
                break;
            case MIDI_CC_REVERB:
                param = PVERB;
                printf("Reverb Level for instance %i, value: %i\n", instanceID, val);
                setValue = true;
                break;
            case MIDI_CC_DETUNE:         /// Ignore Detune
                return; 
                param = PTUNE;
                tune = (val << 7);
                break;
            case MIDI_CC_DETUNE + 32:
                param = PTUNE;
                tune = tune | val;
                dexed[instanceID].setValue(param, tune);
                val = tune;
                break;
            default:
                return;
                break;
            }
            if (setValue)
                dexed[instanceID].setValue(param, val);
            menu.showTGInfo(param);
            uint8_t currentMenu = menu.currentMenu();
            if (((currentMenu == M_TG_FILT || currentMenu == M_TG) && (param == PFREQ || param == PRESO || param == PVERB)) ||
                (currentMenu == M_TG_MIDI && (param == PBANK || param == PCHANNEL)) ||
                (currentMenu == M_TG_OUT && (param == PPAN || param == PVOL)) ||
                (currentMenu == M_TG_TUNE && (param == PSHIFT || param == PTUNE)) ||
                (currentMenu == M_TG_PITCH && (param == PBRANGE || param == PBSTEP)))
            {
                menu.ShowValue(param);
            }
        }
        return;
    }
    if ((raw_sysex.buffer[2] & 0xF0) == 0x0 && raw_sysex.buffer[3] == 0)
    {
#ifdef DEBUGSYSEX
        printf("Sysex Voice Dump\n");
#endif
        uint8_t channel = (raw_sysex.buffer[2] & 0xf && raw_sysex.length == 162 );
        if (channel < 8)
        {
            printf("Voice Dump Size: %i\n", raw_sysex.length);
            dexed[channel].setSysex(raw_sysex);
        }
        menu.showTGInfo(PPATCH);
        return;
    }
    if ((raw_sysex.buffer[2] & 0x70) == 0x50 && raw_sysex.buffer[3] == 0)
    {
//#ifdef DEBUGSYSEX
        printf("Bank Name Received\n");
//#endif

        uint8_t instanceID = raw_sysex.buffer[2] & 0xF;
        dexed[instanceID].setBankName(raw_sysex);
        menu.showTGInfo(PBANK);
        return;
    }
    if (raw_sysex.buffer[2] == 0x31 && raw_sysex.length == 203 )
    {
#ifdef DEBUGSYSEX
        printf("Config Dump Received size %i\n", raw_sysex.length);
#endif

        uint8_t config = 3;
        uint8_t fx_state = raw_sysex.buffer[config++];
        fx_settings.comp_enable = (fx_state >> 1) & 0x1;
        fx_settings.reverb_enable = (fx_state & 0x1);
        fx_settings.verbsize = raw_sysex.buffer[config++];
        fx_settings.highdamp = raw_sysex.buffer[config++];
        fx_settings.lowdamp = raw_sysex.buffer[config++];
        fx_settings.lowpass = raw_sysex.buffer[config++];
        fx_settings.diffusion = raw_sysex.buffer[config++];
        fx_settings.level = raw_sysex.buffer[config++];
        fx_settings.mastervolume = raw_sysex.buffer[config++];

        printf("Comp: %i Verb: %i, Size: %i, HDamp: %i, LDamp: %i LPass %i Diff: %i RLevel: %i Master Volume: %i\n",
            fx_settings.comp_enable,
            fx_settings.reverb_enable,
            fx_settings.verbsize,
            fx_settings.highdamp,
            fx_settings.lowdamp,
            fx_settings.lowpass,
            fx_settings.diffusion,
            fx_settings.level,
            fx_settings.mastervolume);

        for (uint8_t i = 0; i < 8; i++)
        {
            menu.setDexedParm(PBANK, raw_sysex.buffer[config++], i);
            menu.setDexedParm(PPATCH, raw_sysex.buffer[config++], i);
            menu.setDexedParm(PCHANNEL, raw_sysex.buffer[config++], i);
            menu.setDexedParm(PVOL, raw_sysex.buffer[config++], i);
            menu.setDexedParm(PPAN, raw_sysex.buffer[config++], i);
            uint8_t DetuneMSB = raw_sysex.buffer[config++];
            uint8_t DetuneLSB = raw_sysex.buffer[config++];
            int16_t detune = (DetuneMSB << 7) | DetuneLSB;
            if ((detune >> 13) == 1) {
                detune = detune | 0xf000; // Make the number negative
            }
            menu.setDexedParm(PTUNE, detune, i);
            menu.setDexedParm(PFREQ, raw_sysex.buffer[config++], i);
            menu.setDexedParm(PRESO, raw_sysex.buffer[config++], i);
            menu.setDexedParm(PNLOW, raw_sysex.buffer[config++], i); // Note Limit Low
            menu.setDexedParm(PNHIGH, raw_sysex.buffer[config++], i); // Note Limit High
            menu.setDexedParm(PSHIFT, raw_sysex.buffer[config++], i); // Note Shift
            menu.setDexedParm(PVERB, raw_sysex.buffer[config++], i);
            menu.setDexedParm(PBRANGE, raw_sysex.buffer[config++], i);
            menu.setDexedParm(PBSTEP, raw_sysex.buffer[config++], i); // Pitch Bend Step
            menu.setDexedParm(PPMODE, raw_sysex.buffer[config++], i); // Porta Mode
            menu.setDexedParm(PGLISS, raw_sysex.buffer[config++], i); // Porta Gliss
            menu.setDexedParm(PTIME, raw_sysex.buffer[config++], i); // Porta Time
            config++; // Voice Data
            config++; // Unused
            config++; // Unused
            config++; // Unused
            config++; // Unused
            config++; // Unused
        }
        return;
    }
#ifdef DEBUGSYSEX
    printf("\nSysex should be handled\n");
#endif
}
