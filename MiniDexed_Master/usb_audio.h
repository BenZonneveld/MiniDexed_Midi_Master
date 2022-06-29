/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef _USB_AUDIO_H_
#define _USB_AUDIO_H_

#include "tusb.h"

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 48000
#endif

#ifndef SAMPLE_BUFFER_SIZE
#define SAMPLE_BUFFER_SIZE ((CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX/2)-2)
#endif

static uint32_t i2s_buffer[SAMPLE_BUFFER_SIZE];
static uint8_t left_buffer[2][SAMPLE_BUFFER_SIZE+1];
static uint8_t right_buffer[2][SAMPLE_BUFFER_SIZE+1];

void usb_audio_init();
void usb_audio_task();
void usb_audio_write();

#endif
