/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef _USB_AUDIO_H_
#define _USB_AUDIO_H_

//#include "tusb.h"

void usb_audio_init();
void usb_audio_task();
void usb_audio_write();
void cdc_task(void);
#endif
