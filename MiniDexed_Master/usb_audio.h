/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef _USB_AUDIO_H_
#define _USB_AUDIO_H_

#include "tusb.h"

#define AUDIO_SAMPLE_RATE 48000

#ifndef SAMPLE_RATE
#define SAMPLE_RATE (((CFG_TUD_AUDIO_EP_SZ_IN/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)/ 2) - 1) * 1000
#endif

#ifndef SAMPLE_BUFFER_SIZE
#define SAMPLE_BUFFER_SIZE ((CFG_TUD_AUDIO_EP_SZ_IN/2) - 1)
#endif

void usb_audio_init();
void usb_audio_task();
void usb_audio_write(const void * data, uint16_t len);

#endif
