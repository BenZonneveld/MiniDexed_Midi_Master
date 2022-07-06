/*!
 * @file Adafruit_SPITFT.cpp
 *
 * @mainpage Adafruit SPI TFT Displays (and some others)
 *
 * @section intro_sec Introduction
 *
 * Part of Adafruit's GFX graphics library. Originally this class was
 * written to handle a range of color TFT displays connected via SPI,
 * but over time this library and some display-specific subclasses have
 * mutated to include some color OLEDs as well as parallel-interfaced
 * displays. The name's been kept for the sake of older code.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!

 * @section dependencies Dependencies
 *
 * This library depends on <a href="https://github.com/adafruit/Adafruit_GFX">
 * Adafruit_GFX</a> being present on your system. Please make sure you have
 * installed the latest version before using this library.
 *
 * @section author Author
 *
 * Written by Limor "ladyada" Fried for Adafruit Industries,
 * with contributions from the open source community.
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 */

//#include "gpio_pins.h"
#include "pico/binary_info.h"
#include <limits.h>
//#include <pico/stdlib.h>
#include <hardware/spi.h>
#include <hardware/pwm.h>
#include "Adafruit_SPITFT.h"


// Possible values for Adafruit_SPITFT.connection:
#define TFT_HARD_SPI 0 ///< Display interface = hardware SPI
#define TFT_SOFT_SPI 1 ///< Display interface = software SPI
#define TFT_PARALLEL 2 ///< Display interface = 8- or 16-bit parallel

// CONSTRUCTORS ------------------------------------------------------------

/*!
    @brief   Adafruit_SPITFT constructor for software (bitbang) SPI.
    @param   w     Display width in pixels at default rotation setting (0).
    @param   h     Display height in pixels at default rotation setting (0).
    @param   cs    Arduino pin # for chip-select (-1 if unused, tie CS low).
    @param   dc    Arduino pin # for data/command select (required).
    @param   mosi  Arduino pin # for bitbang SPI MOSI signal (required).
    @param   sck   Arduino pin # for bitbang SPI SCK signal (required).
    @param   rst   Arduino pin # for display reset (optional, display reset
                   can be tied to MCU reset, default of -1 means unused).
    @param   miso  Arduino pin # for bitbang SPI MISO signal (optional,
                   -1 default, many displays don't support SPI read).
    @note    Output pins are not initialized; application typically will
             need to call subclass' begin() function, which in turn calls
             this library's initSPI() function to initialize pins.
*/
Adafruit_SPITFT::Adafruit_SPITFT()
    : Adafruit_GFX(240, 320) {}

// end constructors -------

// CLASS MEMBER FUNCTIONS --------------------------------------------------

// -------------------------------------------------------------------------
// Lower-level graphics operations. These functions require a chip-select
// and/or SPI transaction around them (via startWrite(), SPI_CS_HIGH() above).
// Higher-level graphics primitives might start a single transaction and
// then make multiple calls to these functions (e.g. circle or text
// rendering might make repeated lines or rects) before ending the
// transaction. It's more efficient than starting a transaction every time.

/*!
    @brief  Draw a single pixel to the display at requested coordinates.
            Not self-contained; should follow a startWrite() call.
    @param  x      Horizontal position (0 = left).
    @param  y      Vertical position   (0 = top).
    @param  color  16-bit pixel color in '565' RGB format.
*/
void Adafruit_SPITFT::writePixel(int16_t x, int16_t y, uint16_t color) {
  if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
    setAddrWindow(x, y, 1, 1);
    SPI_CS_LOW();
    SPI_WRITE16(color);
    SPI_CS_HIGH();
  }
}

/*!
    @brief  Swap bytes in an array of pixels; converts little-to-big or
            big-to-little endian. Used by writePixels() below in some
            situations, but may also be helpful for user code occasionally.
    @param  src   Source address of 16-bit pixels buffer.
    @param  len   Number of pixels to byte-swap.
    @param  dest  Optional destination address if different than src --
                  otherwise, if NULL (default) or same address is passed,
                  pixel buffer is overwritten in-place.
*/
void Adafruit_SPITFT::swapBytes(uint16_t *src, uint32_t len, uint16_t *dest) {
  if (!dest)
    dest = src; // NULL -> overwrite src buffer
  for (uint32_t i = 0; i < len; i++) {
    dest[i] = __builtin_bswap16(src[i]);
  }
}

/*!
    @brief  Issue a series of pixels from memory to the display. Not self-
            contained; should follow startWrite() and setAddrWindow() calls.
    @param  colors     Pointer to array of 16-bit pixel values in '565' RGB
                       format.
    @param  len        Number of elements in 'colors' array.
    @param  block      If true (default case if unspecified), function blocks
                       until DMA transfer is complete. This is simply IGNORED
                       if DMA is not enabled. If false, the function returns
                       immediately after the last DMA transfer is started,
                       and one should use the dmaWait() function before
                       doing ANY other display-related activities (or even
                       any SPI-related activities, if using an SPI display
                       that shares the bus with other devices).
    @param  bigEndian  If true, bitmap in memory is in big-endian order (most
                       significant byte first). By default this is false, as
                       most microcontrollers seem to be little-endian and
                       16-bit pixel values must be byte-swapped before
                       issuing to the display (which tend toward big-endian
                       when using SPI or 8-bit parallel). If an application
                       can optimize around this -- for example, a bitmap in a
                       uint16_t array having the byte values already ordered
                       big-endian, this can save time here, ESPECIALLY if
                       using this function's non-blocking DMA mode.
*/
void Adafruit_SPITFT::writePixels(uint16_t* colors, uint32_t len, bool block,
    bool bigEndian) {
    if (!len)
    {
        return; // Avoid 0-byte transfers
    }
      // avoid paramater-not-used complaints
    (void)block;
    (void)bigEndian;
    SPI_CS_LOW();

    if (!bigEndian) {
        // switch to 16-bit writes
        hw_write_masked(&spi_get_hw(spi1)->cr0, 15 << SPI_SSPCR0_DSS_LSB,
            SPI_SSPCR0_DSS_BITS);
        spi_write16_blocking(spi1, colors, len);
        // switch back to 8-bit
        hw_write_masked(&spi_get_hw(spi1)->cr0, 7 << SPI_SSPCR0_DSS_LSB,
            SPI_SSPCR0_DSS_BITS);
    }
    else {
        spi_write_blocking(spi1, (uint8_t*)colors,len * 2);
    }
    SPI_CS_HIGH();
    return;
}

/*!
    @brief  Issue a series of pixels, all the same color. Not self-
            contained; should follow startWrite() and setAddrWindow() calls.
    @param  color  16-bit pixel color in '565' RGB format.
    @param  len    Number of pixels to draw.
*/
void Adafruit_SPITFT::writeColor(uint16_t color, uint32_t len) {

    if (!len)
        return; // Avoid 0-byte transfers

//    uint8_t hi = color >> 8, lo = color;

    // All other cases (non-DMA hard SPI, bitbang SPI, parallel)...

    color = __builtin_bswap16(color);
    SPI_CS_LOW();
    while (len--)
        spi_write_blocking(spi1, (uint8_t*)&color, 2);
    SPI_CS_HIGH();
}


/*!
    @brief  Draw a filled rectangle to the display. Not self-contained;
            should follow startWrite(). Typically used by higher-level
            graphics primitives; user code shouldn't need to call this and
            is likely to use the self-contained fillRect() instead.
            writeFillRect() performs its own edge clipping and rejection;
            see writeFillRectPreclipped() for a more 'raw' implementation.
    @param  x      Horizontal position of first corner.
    @param  y      Vertical position of first corner.
    @param  w      Rectangle width in pixels (positive = right of first
                   corner, negative = left of first corner).
    @param  h      Rectangle height in pixels (positive = below first
                   corner, negative = above first corner).
    @param  color  16-bit fill color in '565' RGB format.
    @note   Written in this deep-nested way because C by definition will
            optimize for the 'if' case, not the 'else' -- avoids branches
            and rejects clipped rectangles at the least-work possibility.
*/
void Adafruit_SPITFT::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                                    uint16_t color) {
  if (w && h) {   // Nonzero width and height?
    if (w < 0) {  // If negative width...
      x += w + 1; //   Move X to left edge
      w = -w;     //   Use positive width
    }
    if (x < _width) { // Not off right
      if (h < 0) {    // If negative height...
        y += h + 1;   //   Move Y to top edge
        h = -h;       //   Use positive height
      }
      if (y < _height) { // Not off bottom
        int16_t x2 = x + w - 1;
        if (x2 >= 0) { // Not off left
          int16_t y2 = y + h - 1;
          if (y2 >= 0) { // Not off top
            // Rectangle partly or fully overlaps screen
            if (x < 0) {
              x = 0;
              w = x2 + 1;
            } // Clip left
            if (y < 0) {
              y = 0;
              h = y2 + 1;
            } // Clip top
            if (x2 >= _width) {
              w = _width - x;
            } // Clip right
            if (y2 >= _height) {
              h = _height - y;
            } // Clip bottom
            writeFillRectPreclipped(x, y, w, h, color);
          }
        }
      }
    }
  }
}

/*!
    @brief  Draw a horizontal line on the display. Performs edge clipping
            and rejection. Not self-contained; should follow startWrite().
            Typically used by higher-level graphics primitives; user code
            shouldn't need to call this and is likely to use the self-
            contained drawFastHLine() instead.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  w      Line width in pixels (positive = right of first point,
                   negative = point of first corner).
    @param  color  16-bit line color in '565' RGB format.
*/
void inline Adafruit_SPITFT::writeFastHLine(int16_t x, int16_t y, int16_t w,
                                            uint16_t color) {
  if ((y >= 0) && (y < _height) && w) { // Y on screen, nonzero width
    if (w < 0) {                        // If negative width...
      x += w + 1;                       //   Move X to left edge
      w = -w;                           //   Use positive width
    }
    if (x < _width) { // Not off right
      int16_t x2 = x + w - 1;
      if (x2 >= 0) { // Not off left
        // Line partly or fully overlaps screen
        if (x < 0) {
          x = 0;
          w = x2 + 1;
        } // Clip left
        if (x2 >= _width) {
          w = _width - x;
        } // Clip right
        writeFillRectPreclipped(x, y, w, 1, color);
      }
    }
  }
}

/*!
    @brief  Draw a vertical line on the display. Performs edge clipping and
            rejection. Not self-contained; should follow startWrite().
            Typically used by higher-level graphics primitives; user code
            shouldn't need to call this and is likely to use the self-
            contained drawFastVLine() instead.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  h      Line height in pixels (positive = below first point,
                   negative = above first point).
    @param  color  16-bit line color in '565' RGB format.
*/
void inline Adafruit_SPITFT::writeFastVLine(int16_t x, int16_t y, int16_t h,
                                            uint16_t color) {
    printf("writeFastVline(x,y,h) (%i,%i,%i)\n", x, y, h);
  if ((x >= 0) && (x < _width) && h) { // X on screen, nonzero height
    if (h < 0) {                       // If negative height...
      y += h + 1;                      //   Move Y to top edge
      h = -h;                          //   Use positive height
    }
    if (y < _height) { // Not off bottom
      int16_t y2 = y + h - 1;
      if (y2 >= 0) { // Not off top
        // Line partly or fully overlaps screen
        if (y < 0) {
          y = 0;
          h = y2 + 1;
        } // Clip top
        if (y2 >= _height) {
          h = _height - y;
        } // Clip bottom
        printf("to WriteFillRectClipped(x,y,h) (%i,%i,%i)\n", x, y, h);

        writeFillRectPreclipped(x, y, 1, h, color);
      }
    }
  }
}

/*!
    @brief  A lower-level version of writeFillRect(). This version requires
            all inputs are in-bounds, that width and height are positive,
            and no part extends offscreen. NO EDGE CLIPPING OR REJECTION IS
            PERFORMED. If higher-level graphics primitives are written to
            handle their own clipping earlier in the drawing process, this
            can avoid unnecessary function calls and repeated clipping
            operations in the lower-level functions.
    @param  x      Horizontal position of first corner. MUST BE WITHIN
                   SCREEN BOUNDS.
    @param  y      Vertical position of first corner. MUST BE WITHIN SCREEN
                   BOUNDS.
    @param  w      Rectangle width in pixels. MUST BE POSITIVE AND NOT
                   EXTEND OFF SCREEN.
    @param  h      Rectangle height in pixels. MUST BE POSITIVE AND NOT
                   EXTEND OFF SCREEN.
    @param  color  16-bit fill color in '565' RGB format.
    @note   This is a new function, no graphics primitives besides rects
            and horizontal/vertical lines are written to best use this yet.
*/
inline void Adafruit_SPITFT::writeFillRectPreclipped(int16_t x, int16_t y,
                                                     int16_t w, int16_t h,
                                                     uint16_t color) {
  setAddrWindow(x, y, w, h);
  writeColor(color, (uint32_t)w * h);
}

// -------------------------------------------------------------------------
// Ever-so-slightly higher-level graphics operations. Similar to the 'write'
// functions above, but these contain their own chip-select and SPI
// transactions as needed (via startWrite(), SPI_CS_HIGH()). They're typically
// used solo -- as graphics primitives in themselves, not invoked by higher-
// level primitives (which should use the functions above for better
// performance).

/*!
    @brief  Draw a single pixel to the display at requested coordinates.
            Self-contained and provides its own transaction as needed
            (see writePixel(x,y,color) for a lower-level variant).
            Edge clipping is performed here.
    @param  x      Horizontal position (0 = left).
    @param  y      Vertical position   (0 = top).
    @param  color  16-bit pixel color in '565' RGB format.
*/
void Adafruit_SPITFT::drawPixel(int16_t x, int16_t y, uint16_t color) {
  // Clip first...
  if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
    // THEN set up transaction (if needed) and draw...
    SPI_CS_LOW();
    setAddrWindow(x, y, 1, 1);
    SPI_WRITE16(color);
    SPI_CS_HIGH();
  }
}

/*!
    @brief  Draw a filled rectangle to the display. Self-contained and
            provides its own transaction as needed (see writeFillRect() or
            writeFillRectPreclipped() for lower-level variants). Edge
            clipping and rejection is performed here.
    @param  x      Horizontal position of first corner.
    @param  y      Vertical position of first corner.
    @param  w      Rectangle width in pixels (positive = right of first
                   corner, negative = left of first corner).
    @param  h      Rectangle height in pixels (positive = below first
                   corner, negative = above first corner).
    @param  color  16-bit fill color in '565' RGB format.
    @note   This repeats the writeFillRect() function almost in its entirety,
            with the addition of a transaction start/end. It's done this way
            (rather than starting the transaction and calling writeFillRect()
            to handle clipping and so forth) so that the transaction isn't
            performed at all if the rectangle is rejected. It's really not
            that much code.
*/
void Adafruit_SPITFT::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t color) {
  if (w && h) {   // Nonzero width and height?
    if (w < 0) {  // If negative width...
      x += w + 1; //   Move X to left edge
      w = -w;     //   Use positive width
    }
    if (x < _width) { // Not off right
      if (h < 0) {    // If negative height...
        y += h + 1;   //   Move Y to top edge
        h = -h;       //   Use positive height
      }
      if (y < _height) { // Not off bottom
        int16_t x2 = x + w - 1;
        if (x2 >= 0) { // Not off left
          int16_t y2 = y + h - 1;
          if (y2 >= 0) { // Not off top
            // Rectangle partly or fully overlaps screen
            if (x < 0) {
              x = 0;
              w = x2 + 1;
            } // Clip left
            if (y < 0) {
              y = 0;
              h = y2 + 1;
            } // Clip top
            if (x2 >= _width) {
              w = _width - x;
            } // Clip right
            if (y2 >= _height) {
              h = _height - y;
            } // Clip bottom
            SPI_CS_LOW();
            writeFillRectPreclipped(x, y, w, h, color);
            SPI_CS_HIGH();
          }
        }
      }
    }
  }
}

/*!
    @brief  Draw a horizontal line on the display. Self-contained and
            provides its own transaction as needed (see writeFastHLine() for
            a lower-level variant). Edge clipping and rejection is performed
            here.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  w      Line width in pixels (positive = right of first point,
                   negative = point of first corner).
    @param  color  16-bit line color in '565' RGB format.
    @note   This repeats the writeFastHLine() function almost in its
            entirety, with the addition of a transaction start/end. It's
            done this way (rather than starting the transaction and calling
            writeFastHLine() to handle clipping and so forth) so that the
            transaction isn't performed at all if the line is rejected.
*/
void Adafruit_SPITFT::drawFastHLine(int16_t x, int16_t y, int16_t w,
                                    uint16_t color) {
  if ((y >= 0) && (y < _height) && w) { // Y on screen, nonzero width
    if (w < 0) {                        // If negative width...
      x += w + 1;                       //   Move X to left edge
      w = -w;                           //   Use positive width
    }
    if (x < _width) { // Not off right
      int16_t x2 = x + w - 1;
      if (x2 >= 0) { // Not off left
        // Line partly or fully overlaps screen
        if (x < 0) {
          x = 0;
          w = x2 + 1;
        } // Clip left
        if (x2 >= _width) {
          w = _width - x;
        } // Clip right
        SPI_CS_LOW();
        writeFillRectPreclipped(x, y, w, 1, color);
        SPI_CS_HIGH();
      }
    }
  }
}

/*!
    @brief  Draw a vertical line on the display. Self-contained and provides
            its own transaction as needed (see writeFastHLine() for a lower-
            level variant). Edge clipping and rejection is performed here.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  h      Line height in pixels (positive = below first point,
                   negative = above first point).
    @param  color  16-bit line color in '565' RGB format.
    @note   This repeats the writeFastVLine() function almost in its
            entirety, with the addition of a transaction start/end. It's
            done this way (rather than starting the transaction and calling
            writeFastVLine() to handle clipping and so forth) so that the
            transaction isn't performed at all if the line is rejected.
*/
void Adafruit_SPITFT::drawFastVLine(int16_t x, int16_t y, int16_t h,
                                    uint16_t color) {
  if ((x >= 0) && (x < _width) && h) { // X on screen, nonzero height
    if (h < 0) {                       // If negative height...
      y += h + 1;                      //   Move Y to top edge
      h = -h;                          //   Use positive height
    }
    if (y < _height) { // Not off bottom
      int16_t y2 = y + h - 1;
      if (y2 >= 0) { // Not off top
        // Line partly or fully overlaps screen
        if (y < 0) {
          y = 0;
          h = y2 + 1;
        } // Clip top
        if (y2 >= _height) {
          h = _height - y;
        } // Clip bottom
        SPI_CS_LOW();
        writeFillRectPreclipped(x, y, 1, h, color);
        SPI_CS_HIGH();
      }
    }
  }
}

/*!
    @brief  Essentially writePixel() with a transaction around it. I don't
            think this is in use by any of our code anymore (believe it was
            for some older BMP-reading examples), but is kept here in case
            any user code relies on it. Consider it DEPRECATED.
    @param  color  16-bit pixel color in '565' RGB format.
*/
void Adafruit_SPITFT::pushColor(uint16_t color) {
    SPI_CS_LOW();
    SPI_WRITE16(color);
    SPI_CS_HIGH();
}

/*!
    @brief  Draw a 16-bit image (565 RGB) at the specified (x,y) position.
            For 16-bit display devices; no color reduction performed.
            Adapted from https://github.com/PaulStoffregen/ILI9341_t3
            by Marc MERLIN. See examples/pictureEmbed to use this.
            5/6/2017: function name and arguments have changed for
            compatibility with current GFX library and to avoid naming
            problems in prior implementation.  Formerly drawBitmap() with
            arguments in different order. Handles its own transaction and
            edge clipping/rejection.
    @param  x        Top left corner horizontal coordinate.
    @param  y        Top left corner vertical coordinate.
    @param  pcolors  Pointer to 16-bit array of pixel values.
    @param  w        Width of bitmap in pixels.
    @param  h        Height of bitmap in pixels.
*/
void Adafruit_SPITFT::drawRGBBitmap(int16_t x, int16_t y, uint16_t *pcolors,
                                    int16_t w, int16_t h) {

  int16_t x2, y2;                 // Lower-right coord
  if ((x >= _width) ||            // Off-edge right
      (y >= _height) ||           // " top
      ((x2 = (x + w - 1)) < 0) || // " left
      ((y2 = (y + h - 1)) < 0))
    return; // " bottom

  int16_t bx1 = 0, by1 = 0, // Clipped top-left within bitmap
      saveW = w;            // Save original bitmap width value
  if (x < 0) {              // Clip left
    w += x;
    bx1 = -x;
    x = 0;
  }
  if (y < 0) { // Clip top
    h += y;
    by1 = -y;
    y = 0;
  }
  if (x2 >= _width)
    w = _width - x; // Clip right
  if (y2 >= _height)
    h = _height - y; // Clip bottom

  pcolors += by1 * saveW + bx1; // Offset bitmap ptr to clipped top-left
  setAddrWindow(x, y, w, h); // Clipped area
  while (h--) {              // For each (clipped) scanline...
    writePixels(pcolors, w); // Push one (clipped) row
    pcolors += saveW;        // Advance pointer by one full (unclipped) line
  }
}

// -------------------------------------------------------------------------
// Miscellaneous class member functions that don't draw anything.

/*!
    @brief  Invert the colors of the display (if supported by hardware).
            Self-contained, no transaction setup required.
    @param  i  true = inverted display, false = normal display.
*/
void Adafruit_SPITFT::invertDisplay(bool i) {
    SPI_CS_LOW();
    writeCommand(i ? invertOnCommand : invertOffCommand);
    SPI_CS_HIGH();
}

/*!
    @brief   Given 8-bit red, green and blue values, return a 'packed'
             16-bit color value in '565' RGB format (5 bits red, 6 bits
             green, 5 bits blue). This is just a mathematical operation,
             no hardware is touched.
    @param   red    8-bit red brightnesss (0 = off, 255 = max).
    @param   green  8-bit green brightnesss (0 = off, 255 = max).
    @param   blue   8-bit blue brightnesss (0 = off, 255 = max).
    @return  'Packed' 16-bit color value (565 format).
*/
uint16_t Adafruit_SPITFT::color565(uint8_t red, uint8_t green, uint8_t blue) {
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}

/*!
@brief   Adafruit_SPITFT Send Command handles complete sending of commands and
data
@param   commandByte       The Command Byte
@param   dataBytes         A pointer to the Data bytes to send
@param   numDataBytes      The number of bytes we should send
*/
void Adafruit_SPITFT::sendCommand(uint8_t commandByte, uint8_t* dataBytes,
    uint8_t numDataBytes) {
    SPI_CS_LOW();

    SPI_DC_LOW();          // Command mode
    spiWrite(commandByte); // Send the command byte

    SPI_DC_HIGH();
    for (int i = 0; i < numDataBytes; i++) {
        spiWrite(*dataBytes); // Send the data bytes
        dataBytes++;
    }

    SPI_CS_HIGH();
}

/*!
 @brief   Adafruit_SPITFT Send Command handles complete sending of commands and
 data
 @param   commandByte       The Command Byte
 @param   dataBytes         A pointer to the Data bytes to send
 @param   numDataBytes      The number of bytes we should send
 */
void Adafruit_SPITFT::sendCommand(uint8_t commandByte, const uint8_t *dataBytes,
                                  uint8_t numDataBytes) {
  SPI_CS_LOW();

  SPI_DC_LOW();          // Command mode
  spiWrite(commandByte); // Send the command byte
  SPI_DC_HIGH();
  for (int i = 0; i < numDataBytes; i++) {
      spiWrite(*(dataBytes++));
  }

  SPI_CS_HIGH();
}

/*!
 @brief  Adafruit_SPITFT sendCommand16 handles complete sending of
         commands and data for 16-bit parallel displays. Currently somewhat
         rigged for the NT35510, which has the odd behavior of wanting
         commands 16-bit, but subsequent data as 8-bit values, despite
         the 16-bit bus (high byte is always 0). Also seems to require
         issuing and incrementing address with each transfer.
 @param  commandWord   The command word (16 bits)
 @param  dataBytes     A pointer to the data bytes to send
 @param  numDataBytes  The number of bytes we should send
 */
void Adafruit_SPITFT::sendCommand16(uint16_t commandWord,
                                    const uint8_t *dataBytes,
                                    uint8_t numDataBytes) {
  SPI_CS_LOW();

  if (numDataBytes == 0) {
    SPI_DC_LOW();             // Command mode
    SPI_WRITE16(commandWord); // Send the command word
    SPI_DC_HIGH();            // Data mode
  }
  for (int i = 0; i < numDataBytes; i++) {
    SPI_DC_LOW();             // Command mode
    SPI_WRITE16(commandWord); // Send the command word
    SPI_DC_HIGH();            // Data mode
    commandWord++;
    SPI_WRITE16((uint16_t)*(dataBytes++));
  }

  SPI_CS_HIGH();
}

// -------------------------------------------------------------------------
// Lowest-level hardware-interfacing functions. Many of these are inline and
// compile to different things based on #defines -- typically just a few
// instructions. Others, not so much, those are not inlined.

/*!
    @brief  Issue a single 8-bit value to the display. Chip-select,
            transaction and data/command selection must have been
            previously set -- this ONLY issues the byte. This is another of
            those functions in the library with a now-not-accurate name
            that's being maintained for compatibility with outside code.
            This function is used even if display connection is parallel.
    @param  b  8-bit value to write.
*/
void Adafruit_SPITFT::spiWrite(uint8_t b) {
    spi_write_blocking(spi1, &b, 1);
}

/*!
    @brief  Write a single command byte to the display. Chip-select and
            transaction must have been previously set -- this ONLY sets
            the device to COMMAND mode, issues the byte and then restores
            DATA mode. There is no corresponding explicit writeData()
            function -- just use spiWrite().
    @param  cmd  8-bit command to write.
*/
void Adafruit_SPITFT::writeCommand(uint8_t cmd) {
  SPI_DC_LOW();
  spiWrite(cmd);
  SPI_DC_HIGH();
}


/*!
    @brief  Issue a single 16-bit value to the display. Chip-select,
            transaction and data/command selection must have been
            previously set -- this ONLY issues the word.
            Thus operates ONLY on 'wide' (16-bit) parallel displays!
    @param  w  16-bit value to write.
*/
void Adafruit_SPITFT::write16(uint16_t w) {
}

/*!
    @brief  Write a single command word to the display. Chip-select and
            transaction must have been previously set -- this ONLY sets
            the device to COMMAND mode, issues the byte and then restores
            DATA mode. This operates ONLY on 'wide' (16-bit) parallel
            displays!
    @param  cmd  16-bit command to write.
*/
void Adafruit_SPITFT::writeCommand16(uint16_t cmd) {
  SPI_DC_LOW();
  write16(cmd);
  SPI_DC_HIGH();
}


/*!
    @brief  Issue a single 16-bit value to the display. Chip-select,
            transaction and data/command selection must have been
            previously set -- this ONLY issues the word. Despite the name,
            this function is used even if display connection is parallel;
            name was maintaned for backward compatibility. Naming is also
            not consistent with the 8-bit version, spiWrite(). Sorry about
            that. Again, staying compatible with outside code.
    @param  w  16-bit value to write.
*/
void Adafruit_SPITFT::SPI_WRITE16(uint16_t w) {
    spi_write_blocking(spi1, (uint8_t *)&w, 2);
}


/**************************************************************************/
/*!
    @brief  Companion code to the initiliazation tables. Reads and issues
            a series of LCD commands stored in PROGMEM byte array.
    @param  addr  Flash memory array with commands and data to send
*/
/**************************************************************************/
void Adafruit_SPITFT::displayInit(const uint8_t* addr) {

    uint8_t numCommands, cmd, numArgs;
    uint16_t ms;

    SPI_CS_LOW();
    numCommands = *(addr++); // Number of commands to follow
    while (numCommands--) {              // For each command...
        cmd = *(addr++);       // Read command
        numArgs = *(addr++);   // Number of args to follow
        ms = numArgs & ST_CMD_DELAY;       // If hibit set, delay follows args
        numArgs &= ~ST_CMD_DELAY;          // Mask out delay bit
        sendCommand(cmd, addr, numArgs);
        addr += numArgs;

        if (ms) {
            ms = *(addr++); // Read post-command delay time (ms)
            if (ms == 255)
                ms = 500; // If 255, delay for 500 ms
            sleep_ms(ms);
        }
    }
    SPI_CS_HIGH();
    setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1);
}

/**************************************************************************/
/*!
  @brief  SPI displays set an address window rectangle for blitting pixels
  @param  x  Top left corner x coordinate
  @param  y  Top left corner x coordinate
  @param  w  Width of window
  @param  h  Height of window
*/
/**************************************************************************/
void Adafruit_SPITFT::setAddrWindow(uint16_t x, uint16_t y, uint16_t w,
    uint16_t h) {
//    printf("Set Addr Window\n");
    SPI_CS_LOW();
    uint8_t data[4];
    static uint16_t prev_x, prev_y, prev_w, prev_h;

    x = x + MIPI_DISPLAY_OFFSET_X;
    y = y + MIPI_DISPLAY_OFFSET_Y;
    w = x+w + MIPI_DISPLAY_OFFSET_X - 1;
    h = y+h + MIPI_DISPLAY_OFFSET_Y;

    /* Change column address only if it has changed. */
    if ((prev_x != x || prev_w != w)) {
        writeCommand(MIPI_DCS_SET_COLUMN_ADDRESS); // Column addr set
        data[0] = x >> 8;
        data[1] = x & 0xff;
        data[2] = w >> 8;
        data[3] = w & 0xff;
        spi_write_blocking(spi1, data, 4);
        prev_x = x;
        prev_w = w;
    }

    /* Change page address only if it has changed. */
    if ((prev_y != y || prev_h != h)) {
        writeCommand(MIPI_DCS_SET_PAGE_ADDRESS); // Row addr set
        data[0] = y >> 8;
        data[1] = y & 0xff;
        data[2] = h >> 8;
        data[3] = h & 0xff;
        spi_write_blocking(spi1, data, 4); 
        prev_y = y;
        prev_h = h;
    }
    writeCommand(MIPI_DCS_WRITE_MEMORY_START); // write to RAM
    SPI_CS_HIGH();
}

/**************************************************************************/
/*!
    @brief  Set origin of (0,0) of display with offsets
    @param  col  The offset from 0 for the column address
    @param  row  The offset from 0 for the row address
*/
/**************************************************************************/
void Adafruit_SPITFT::setColRowStart(int8_t col, int8_t row) {
    _colstart = col;
    _rowstart = row;
}

/**************************************************************************/
/*!
 @brief  Change whether display is on or off
 @param  enable True if you want the display ON, false OFF
 */
 /**************************************************************************/
void Adafruit_SPITFT::enableDisplay(bool enable) {
    SPI_CS_LOW();
    sendCommand(enable ? MIPI_DCS_SET_DISPLAY_ON : MIPI_DCS_SET_DISPLAY_OFF);
    SPI_CS_HIGH();
}

/**************************************************************************/
/*!
 @brief  Change whether TE pin output is on or off
 @param  enable True if you want the TE pin ON, false OFF
 */
 /**************************************************************************/
void Adafruit_SPITFT::enableTearing(bool enable) {
    SPI_CS_LOW();
    sendCommand(enable ? MIPI_DCS_SET_TEAR_ON : MIPI_DCS_SET_TEAR_OFF);
    SPI_CS_HIGH();
}

/**************************************************************************/
/*!
 @brief  Change whether sleep mode is on or off
 @param  enable True if you want sleep mode ON, false OFF
 */
 /**************************************************************************/
void Adafruit_SPITFT::enableSleep(bool enable) {
    SPI_CS_LOW();
    sendCommand(enable ? MIPI_DCS_ENTER_SLEEP_MODE : MIPI_DCS_EXIT_SLEEP_MODE);
    SPI_CS_HIGH();
}

 // SCREEN INITIALIZATION ***************************************************

 // clang-format off

static const uint8_t st7789[] = {                // Init commands for 7789 screens
    5,                              //  9 commands in list:
    MIPI_DCS_SOFT_RESET,   ST_CMD_DELAY, //  1: Software reset, no args, w/delay
      200,                          //     ~150 ms delay
    MIPI_DCS_SET_ADDRESS_MODE, 1 + ST_CMD_DELAY, //  3: Set color mode, 1 arg + delay:
      MIPI_DISPLAY_ADDRESS_MODE,                         //     16-bit color
      10,                           //     10 ms delay
    MIPI_DCS_SET_PIXEL_FORMAT, 1 + ST_CMD_DELAY,
     MIPI_DISPLAY_PIXEL_FORMAT,
     10,
    MIPI_DCS_EXIT_INVERT_MODE , ST_CMD_DELAY,    
      10,                           
    MIPI_DCS_EXIT_SLEEP_MODE , ST_CMD_DELAY,     
      200 //,                           
//    MIPI_DCS_SET_DISPLAY_ON , ST_CMD_DELAY,
//      200                           
};

// clang-format on

/**************************************************************************/
/*!
    @brief  Initialization code common to all ST7789 displays
    @param  width  Display width
    @param  height Display height
    @param  mode   SPI data mode; one of SPI_MODE0, SPI_MODE1, SPI_MODE2
                   or SPI_MODE3 (do NOT pass the numbers 0,1,2 or 3 -- use
                   the defines only, the values are NOT the same!)
*/
/**************************************************************************/
void Adafruit_SPITFT::init(uint16_t width, uint16_t height) {
    invertOnCommand = MIPI_DCS_ENTER_INVERT_MODE;
    invertOffCommand = MIPI_DCS_EXIT_INVERT_MODE;

    // GPIO Setup
    gpio_init(RESET);
    gpio_init(LCD_CS);
    gpio_init(LCD_DC);
    gpio_init(SD_CS);
    
    gpio_set_dir(RESET, GPIO_OUT);
    gpio_set_dir(LCD_CS, GPIO_OUT);
    gpio_set_dir(LCD_DC, GPIO_OUT);
    gpio_set_dir(SD_CS, GPIO_OUT);
//    
    gpio_put(LCD_CS, 1);
    gpio_put(SD_CS, 1);
    // SPI Init
    spi_init(spi1, MIPI_DISPLAY_SPI_CLOCK_SPEED_HZ);
    gpio_set_function(MISO, GPIO_FUNC_SPI);
    gpio_set_function(SCK, GPIO_FUNC_SPI);
    gpio_set_function(MOSI, GPIO_FUNC_SPI);
//    spi_set_baudrate(spi1, MIPI_DISPLAY_SPI_CLOCK_SPEED_HZ);
#ifndef MIDIPORT
    printf("Actual SPI Speed: %lu\n", spi_get_baudrate(spi1));
#endif
    sleep_ms(100);
    // LCD Reset
    gpio_put(RESET, 1);
    sleep_ms(500);
    gpio_put(RESET, 0);
    sleep_ms(500);
    gpio_put(RESET, 1);
    sleep_ms(500);

    _rowstart = (128 - height);
    _rowstart2 = 0;
    _colstart = _colstart2 = (160 - width);
 
    windowWidth = width;
    windowHeight = height;

    displayInit(st7789);
    writeFillRect(0, 0, width, height, BLACK);
    enableDisplay(true);
//    setRotation(1);
}

/**************************************************************************/
/*!
    @brief  Set origin of (0,0) and orientation of TFT display
    @param  m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Adafruit_SPITFT::setRotation(uint8_t m) {
    uint8_t madctl = 0;

    rotation = m & 3; // can't be higher than 3

    switch (rotation) {
    case 0:
        madctl = MIPI_DCS_ADDRESS_MODE_MIRROR_X | MIPI_DCS_ADDRESS_MODE_MIRROR_Y | MIPI_DCS_ADDRESS_MODE_BGR;
        _xstart = _colstart;
        _ystart = _rowstart;
        _width = windowWidth;
        _height = windowHeight;
        break;
    case 1:
        madctl = MIPI_DCS_ADDRESS_MODE_MIRROR_Y | MIPI_DCS_ADDRESS_MODE_SWAP_XY | MIPI_DCS_ADDRESS_MODE_BGR;
        _xstart = _rowstart;
        _ystart = _colstart2;
        _height = windowWidth;
        _width = windowHeight;
        break;
    case 2:
        madctl = MIPI_DCS_ADDRESS_MODE_BGR;
        _xstart = _colstart2;
        _ystart = _rowstart2;
        _width = windowWidth;
        _height = windowHeight;
        break;
    case 3:
        madctl = MIPI_DCS_ADDRESS_MODE_MIRROR_X | MIPI_DCS_ADDRESS_MODE_SWAP_XY | MIPI_DCS_ADDRESS_MODE_BGR;
        _xstart = _rowstart2;
        _ystart = _colstart;
        _height = windowWidth;
        _width = windowHeight;
        break;
    }

    sendCommand(MIPI_DCS_SET_ADDRESS_MODE, &madctl, 1);
}
