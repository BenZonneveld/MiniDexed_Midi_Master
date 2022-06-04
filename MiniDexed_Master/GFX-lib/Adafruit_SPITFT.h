/*!
 * @file Adafruit_SPITFT.h
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
 *
 * Written by Limor "ladyada" Fried for Adafruit Industries,
 * with contributions from the open source community.
 *
 * BSD license, all text here must be included in any redistribution.
 */

#ifndef _ADAFRUIT_SPITFT_H_
#define _ADAFRUIT_SPITFT_H_

#include "../gpio_pins.h"
#include "Adafruit_GFX.h"
#include "hardware/spi.h"

typedef enum {
    L2R_U2D = 0,	//The display interface is displayed , left to right, up to down
    L2R_D2U,
    R2L_U2D,
    R2L_D2U,

    U2D_L2R,
    U2D_R2L,
    D2U_L2R,
    D2U_R2L,
} LCD_SCAN_DIR;
#define SCAN_DIR_DFT  D2U_L2R  //Default scan direction = L2R_U2D


// This is kind of a kludge. Needed a way to disambiguate the software SPI
// and parallel constructors via their argument lists. Originally tried a
// bool as the first argument to the parallel constructor (specifying 8-bit
// vs 16-bit interface) but the compiler regards this as equivalent to an
// integer and thus still ambiguous. SO...the parallel constructor requires
// an enumerated type as the first argument: tft8 (for 8-bit parallel) or
// tft16 (for 16-bit)...even though 16-bit isn't fully implemented or tested
// and might never be, still needed that disambiguation from soft SPI.
/*! For first arg to parallel constructor */
//enum tftBusWidth { tft8bitbus, tft16bitbus };

// CLASS DEFINITION --------------------------------------------------------

/*!
  @brief  Adafruit_SPITFT is an intermediary class between Adafruit_GFX
          and various hardware-specific subclasses for different displays.
          It handles certain operations that are common to a range of
          displays (address window, area fills, etc.). Originally these were
          all color TFT displays interfaced via SPI, but it's since expanded
          to include color OLEDs and parallel-interfaced TFTs. THE NAME HAS
          BEEN KEPT TO AVOID BREAKING A LOT OF SUBCLASSES AND EXAMPLE CODE.
          Many of the class member functions similarly live on with names
          that don't necessarily accurately describe what they're doing,
          again to avoid breaking a lot of other code. If in doubt, read
          the comments.
*/
class Adafruit_SPITFT : public Adafruit_GFX {

public:
    // CONSTRUCTORS --------------------------------------------------------

    // Software SPI constructor: expects width & height (at default rotation
    // setting 0), 4 signal pins (cs, dc, mosi, sclk), 2 optional pins
    // (reset, miso). cs argument is required but can be -1 if unused --
    // rather than moving it to the optional arguments, it was done this way
    // to avoid breaking existing code (-1 option was a later addition).
    Adafruit_SPITFT();

    // DESTRUCTOR ----------------------------------------------------------

    ~Adafruit_SPITFT() {};

    // CLASS MEMBER FUNCTIONS ----------------------------------------------

    // These first two functions MUST be declared by subclasses:

    /*!
        @brief  Display-specific initialization function.
        @param  freq  SPI frequency, in hz (or 0 for default or unused).
    */
    //  virtual void begin(uint32_t freq) = 0;

      /*!
          @brief  Set up the specific display hardware's "address window"
                  for subsequent pixel-pushing operations.
          @param  x  Leftmost pixel of area to be drawn (MUST be within
                     display bounds at current rotation setting).
          @param  y  Topmost pixel of area to be drawn (MUST be within
                     display bounds at current rotation setting).
          @param  w  Width of area to be drawn, in pixels (MUST be >0 and,
                     added to x, within display bounds at current rotation).
          @param  h  Height of area to be drawn, in pixels (MUST be >0 and,
                     added to x, within display bounds at current rotation).
      */
      //  virtual void setAddrWindow(uint16_t x, uint16_t y, uint16_t w,
      //                             uint16_t h) = 0;

    void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void enableDisplay(bool enable);
    void enableTearing(bool enable);
    void enableSleep(bool enable);

    void setRotation(uint8_t m);
    void init(uint16_t width, uint16_t height);
    // Remaining functions do not need to be declared in subclasses
    // unless they wish to provide hardware-specific optimizations.
    // Brief comments here...documented more thoroughly in .cpp file.

    // Subclass' begin() function invokes this to initialize hardware.
    // freq=0 to use default SPI speed. spiMode must be one of the SPI_MODEn
    // values defined in SPI.h, which are NOT the same as 0 for SPI_MODE0,
    // 1 for SPI_MODE1, etc...use ONLY the SPI_MODEn defines! Only!
    // Name is outdated (interface may be parallel) but for compatibility:
    void sendCommand(uint8_t commandByte, uint8_t* dataBytes,
        uint8_t numDataBytes);
    void sendCommand(uint8_t commandByte, const uint8_t* dataBytes = NULL,
        uint8_t numDataBytes = 0);
    void sendCommand16(uint16_t commandWord, const uint8_t* dataBytes = NULL,
        uint8_t numDataBytes = 0);

    // These functions require a chip-select and/or SPI transaction
    // around them. Higher-level graphics primitives might start a
    // single transaction and then make multiple calls to these functions
    // (e.g. circle or text rendering might make repeated lines or rects)
    // before ending the transaction. It's more efficient than starting a
    // transaction every time.
    void writePixel(int16_t x, int16_t y, uint16_t color);
    void writePixels(uint16_t* colors, uint32_t len, bool block = true,
        bool bigEndian = false);
    void writeColor(uint16_t color, uint32_t len);
    void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
        uint16_t color);
    void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    // This is a new function, similar to writeFillRect() except that
    // all arguments MUST be onscreen, sorted and clipped. If higher-level
    // primitives can handle their own sorting/clipping, it avoids repeating
    // such operations in the low-level code, making it potentially faster.
    // CALLING THIS WITH UNCLIPPED OR NEGATIVE VALUES COULD BE DISASTROUS.
    inline void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w,
        int16_t h, uint16_t color);
    void swapBytes(uint16_t* src, uint32_t len, uint16_t* dest = NULL);

    // These functions are similar to the 'write' functions above, but with
    // a chip-select and/or SPI transaction built-in. They're typically used
    // solo -- that is, as graphics primitives in themselves, not invoked by
    // higher-level primitives (which should use the functions above).
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    // A single-pixel push encapsulated in a transaction. I don't think
    // this is used anymore (BMP demos might've used it?) but is provided
    // for backward compatibility, consider it deprecated:
    void pushColor(uint16_t color);

    using Adafruit_GFX::drawRGBBitmap; // Check base class first
    void drawRGBBitmap(int16_t x, int16_t y, uint16_t* pcolors, int16_t w,
        int16_t h);

    void invertDisplay(bool i);
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b);

    // Despite parallel additions, function names kept for compatibility:
    void spiWrite(uint8_t b);          // Write single byte as DATA
    void writeCommand(uint8_t cmd);    // Write single byte as COMMAND
    void write16(uint16_t w);          // Write 16-bit value as DATA
    void writeCommand16(uint16_t cmd); // Write 16-bit value as COMMAND

    // Most of these low-level functions were formerly macros in
    // Adafruit_SPITFT_Macros.h. Some have been made into inline functions
    // to avoid macro mishaps. Despite the addition of code for a parallel
    // display interface, the names have been kept for backward
    // compatibility (some subclasses may be invoking these):
    void SPI_WRITE16(uint16_t w); // Not inline
    // Old code had both a spiWrite16() function and SPI_WRITE16 macro
    // in addition to the SPI_WRITE32 macro. The latter two have been
    // made into functions here, and spiWrite16() removed (use SPI_WRITE16()
    // instead). It looks like most subclasses had gotten comfortable with
    // SPI_WRITE16 and SPI_WRITE32 anyway so those names were kept rather
    // than the less-obnoxious camelcase variants, oh well.

    // Placing these functions entirely in the class definition inlines
    // them implicitly them while allowing their use in other code:

    /*!
        @brief  Set the chip-select line HIGH. Does NOT check whether CS pin
                is set (>=0), that should be handled in calling function.
                Despite function name, this is used even if the display
                connection is parallel.
    */
    void SPI_CS_HIGH(void) {
        gpio_put(LCD_CS, 1);
    }

    /*!
        @brief  Set the chip-select line LOW. Does NOT check whether CS pin
                is set (>=0), that should be handled in calling function.
                Despite function name, this is used even if the display
                connection is parallel.
    */
    void SPI_CS_LOW(void) {
        gpio_put(LCD_CS, 0);
    }

    /*!
        @brief  Set the data/command line HIGH (data mode).
    */
    void SPI_DC_HIGH(void) {
        gpio_put(LCD_DC, 1);
    }

    /*!
        @brief  Set the data/command line LOW (command mode).
    */
    void SPI_DC_LOW(void) {
        gpio_put(LCD_DC, 0);
    }

protected:
    // CLASS INSTANCE VARIABLES --------------------------------------------

    // Here be dragons! There's a big union of three structures here --
    // one each for hardware SPI, software (bitbang) SPI, and parallel
    // interfaces. This is to save some memory, since a display's connection
    // will be only one of these. The order of some things is a little weird
    // in an attempt to get values to align and pack better in RAM.
    uint32_t _freq; ///< SPI bitrate (if no SPI transactions)
    uint32_t _mode; ///< SPI data mode (transactions or no)

    int16_t _xstart = 0;          ///< Internal framebuffer X offset
    int16_t _ystart = 0;          ///< Internal framebuffer Y offset
    uint8_t invertOnCommand = 0;  ///< Command to enable invert mode
    uint8_t invertOffCommand = 0; ///< Command to disable invert mode
    uint8_t _colstart = 0, _rowstart = 0;
    uint8_t _colstart2 = 0, ///< Offset from the right
        _rowstart2 = 0;     ///< Offset from the bottom

    void displayInit(const uint8_t* addr);
    void setColRowStart(int8_t col, int8_t row);
    //uint32_t _freq = 0; ///< Dummy var to keep subclasses happy
private:
    uint16_t windowWidth;
    uint16_t windowHeight;
};


#define ST_CMD_DELAY 0x80 // special signifier for command lists

#define MIPI_DCS_NOP                        0x00
#define MIPI_DCS_SOFT_RESET                 0x01
#define MIPI_DCS_GET_COMPRESSION_MODE       0x03
#define MIPI_DCS_GET_DISPLAY_ID             0x04 /* Not in spec? */
#define MIPI_DCS_GET_RED_CHANNEL            0x06
#define MIPI_DCS_GET_GREEN_CHANNEL          0x07
#define MIPI_DCS_GET_BLUE_CHANNEL           0x08
#define MIPI_DCS_GET_DISPLAY_STATUS         0x09 /* Not in spec? */
#define MIPI_DCS_GET_POWER_MODE             0x0A
#define MIPI_DCS_GET_ADDRESS_MODE           0x0B
#define MIPI_DCS_GET_PIXEL_FORMAT           0x0C
#define MIPI_DCS_GET_DISPLAY_MODE           0x0D
#define MIPI_DCS_GET_SIGNAL_MODE            0x0E
#define MIPI_DCS_GET_DIAGNOSTIC_RESULT      0x0F
#define MIPI_DCS_ENTER_SLEEP_MODE           0x10
#define MIPI_DCS_EXIT_SLEEP_MODE            0x11
#define MIPI_DCS_ENTER_PARTIAL_MODE         0x12
#define MIPI_DCS_ENTER_NORMAL_MODE          0x13
#define MIPI_DCS_EXIT_INVERT_MODE           0x20
#define MIPI_DCS_ENTER_INVERT_MODE          0x21
#define MIPI_DCS_SET_GAMMA_CURVE            0x26
#define MIPI_DCS_SET_DISPLAY_OFF            0x28
#define MIPI_DCS_SET_DISPLAY_ON             0x29
#define MIPI_DCS_SET_COLUMN_ADDRESS         0x2A
#define MIPI_DCS_SET_PAGE_ADDRESS           0x2B
#define MIPI_DCS_WRITE_MEMORY_START         0x2C
#define MIPI_DCS_WRITE_LUT                  0x2D
#define MIPI_DCS_READ_MEMORY_START          0x2E
#define MIPI_DCS_SET_PARTIAL_ROWS           0x30
#define MIPI_DCS_SET_PARTIAL_COLUMNS        0x31
#define MIPI_DCS_SET_SCROLL_AREA            0x33
#define MIPI_DCS_SET_TEAR_OFF               0x34
#define MIPI_DCS_SET_TEAR_ON                0x35
#define MIPI_DCS_SET_ADDRESS_MODE           0x36
#define MIPI_DCS_SET_SCROLL_START           0x37
#define MIPI_DCS_EXIT_IDLE_MODE             0x38
#define MIPI_DCS_ENTER_IDLE_MODE            0x39
#define MIPI_DCS_SET_PIXEL_FORMAT           0x3A
#define MIPI_DCS_WRITE_MEMORY_CONTINUE      0x3C
#define MIPI_DCS_SET_3D_CONTROL             0x3D
#define MIPI_DCS_READ_MEMORY_CONTINUE       0x3E
#define MIPI_DCS_GET_3D_CONTROL             0x3F
#define MIPI_DCS_SET_VSYNC_TIMING           0x40
#define MIPI_DCS_SET_TEAR_SCANLINE          0x44
#define MIPI_DCS_GET_SCANLINE               0x45
#define MIPI_DCS_SET_DISPLAY_BRIGHTNESS     0x51
#define MIPI_DCS_GET_DISPLAY_BRIGHTNESS     0x52
#define MIPI_DCS_WRITE_CONTROL_DISPLAY      0x53
#define MIPI_DCS_GET_CONTROL_DISPLAY        0x54
#define MIPI_DCS_WRITE_POWER_SAVE           0x55
#define MIPI_DCS_GET_POWER_SAVE             0x56
#define MIPI_DCS_SET_CABC_MIN_BRIGHTNESS    0x5E
#define MIPI_DCS_GET_CABC_MIN_BRIGHTNESS    0x5F
#define MIPI_DCS_READ_DDB_START             0xA1
#define MIPI_DCS_READ_DDB_CONTINUE          0xA8

#define MIPI_DCS_PIXEL_FORMAT_24BIT         0x77 /* 0b01110111 */
#define MIPI_DCS_PIXEL_FORMAT_18BIT         0x66 /* 0b01100110 */
#define MIPI_DCS_PIXEL_FORMAT_16BIT         0x55 /* 0b01010101 */
#define MIPI_DCS_PIXEL_FORMAT_12BIT         0x33 /* 0b00110011 */
#define MIPI_DCS_PIXEL_FORMAT_8BIT          0x22 /* 0b00100010 */
#define MIPI_DCS_PIXEL_FORMAT_3BIT          0x11 /* 0b00010001 */

#define MIPI_DCS_ADDRESS_MODE_MIRROR_Y      0x80
#define MIPI_DCS_ADDRESS_MODE_MIRROR_X      0x40
#define MIPI_DCS_ADDRESS_MODE_SWAP_XY       0x20
#define MIPI_DCS_ADDRESS_MODE_REFRESH_BT    0x10 /* Does not affect image */
#define MIPI_DCS_ADDRESS_MODE_BGR           0x08
#define MIPI_DCS_ADDRESS_MODE_RGB           0x00
#define MIPI_DCS_ADDRESS_MODE_LATCH_RL      0x04 /* Does not affect image */
#define MIPI_DCS_ADDRESS_MODE_FLIP_X        0x02
#define MIPI_DCS_ADDRESS_MODE_FLIP_Y        0x01
// Some ready-made 16-bit ('565') color settings:
#define BLACK 0x0000
#define DARKERGREY 0x18E3
#define DARKGREY 0x4208 
#define GREY 0x8430
#define LIGHTGREY 0xBDF7 
#define WHITE 0xFFFF

#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define ORANGE 0xFC00

#endif // end _ADAFRUIT_SPITFT_H_
