/*
  I2S.cpp -
    I2S digital audio input Arduino library for the Raspberry Pi Pico RP2040

    Copyright (C) 2022 Sfera Labs S.r.l. - All rights reserved.

    For information, see:
    http://www.sferalabs.cc/

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  See file LICENSE.txt for further informations on licensing terms.
*/

#include <stdlib.h>
#include <stdint.h>
#include <pico/stdlib.h>
#include <stdbool.h>

#include "I2S.h"
#include "machine_i2s.c"

I2SClass::I2SClass() {
}

bool I2SClass::setSCK(uint8_t pin) {
  _pin_sck = pin;
  return true;
}

bool I2SClass::setWS(uint8_t pin) {
  _pin_ws = pin;
  return true;
}

bool I2SClass::setSD(uint8_t pin) {
  _pin_sd = pin;
  return true;
}

bool I2SClass::setBufferSize(int bufferSize) {
  _bufferSize = bufferSize;
  return true;
}

bool I2SClass::begin(int mode, long sampleRate, int bitsPerSample) {
  machine_i2s_obj_t* i2s0 = machine_i2s_make_new(0, _pin_sck, _pin_ws, _pin_sd, bitsPerSample, _bufferSize, sampleRate);
  if (i2s0 == NULL) {
    return false;
  }
  return true;
}

int I2SClass::read(void* buffer, size_t size) {
  return machine_i2s_stream_read(machine_i2s_obj[0], buffer, size);
}
