/*
  I2S.h -
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

#ifndef I2S_h
#define I2S_h

class I2SClass {
  public:
    I2SClass();

    bool setSCK(uint8_t pin);
    bool setWS(uint8_t pin);
    bool setSD(uint8_t pin);
    bool setBufferSize(uint32_t bufferSize);
    bool begin();
    void end();
    int read(uint32_t* buffer, size_t size);

  private:
    uint8_t _pin_sck;
    uint8_t _pin_ws;
    uint8_t _pin_sd;
    uint32_t _bufferSize;

};

#endif
