/*
 */
#include <stdio.h>
#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#include "gpio_pins.h"

#include "MIDI/midi.h"
#include "Pots/Pots.h"
#include "Buttons/Buttons.h"
#include "Screen/Screen.h"
#include "Screen/MainMenu.h"
#include "Dexed/TG.h"

 //--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

//#define IGNORE_MIDI_CC

#define DEXED       uart1
#define MIDIPORT    uart0

static bool led_usb_state = false;
static bool led_uart_state = false;

void midi_task(void);
void led_task(void);

cPots Pots = cPots();
cButtons buttons = cButtons();
cScreen screen;// = cScreen();
cTG dexed[8] = { cTG() };

/*------------- MAIN -------------*/
int main(void)
{
  set_sys_clock_khz(133000, true);
  board_init();

  stdio_init_all();
  printf("MDMA Booting\r\n");
  tusb_init();
  printf("MDMA Booting\r\n");

    // Initialise UART 0
//  uart_init(MIDIPORT, 31250);
//  uart_init(DEXED, 31250);
  // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
//  gpio_set_function(TX0, GPIO_FUNC_UART);
//  gpio_set_function(RX0, GPIO_FUNC_UART);
//  gpio_set_function(TX1, GPIO_FUNC_UART);
//  gpio_set_function(RX1, GPIO_FUNC_UART);
  char text[64];
  printf("MDMA Booting\r\n");

  screen = cScreen();
// hagl_init();
  screen.cls();
  InitMainMenu();

  while (1)
  {
    tud_task();   // tinyusb device task
    led_task();
    midi_task();
    Pots.readAll();
    sleep_ms(50);
    buttons.getButtons();
//    printf("BUttons State: %02X\r\n", buttons.getState());

    if ( buttons.getState())
        printf("Buttons State: %02X\r\n", buttons.getState());

    if (Pots.isUpdated(0))
    {
        snprintf(text, sizeof(text), "Pot 0: %i", Pots.getPot(0));
        printf("%s\r\n", text);
    }
    if (Pots.isUpdated(1))
    {
        snprintf(text, sizeof(text), "Pot 1: %i", Pots.getPot(1));
        printf("%s\r\n", text);
    }
    if (Pots.isUpdated(2))
    {
        snprintf(text, sizeof(text), "Pot 2: %i", Pots.getPot(2));
        printf("%s\r\n", text);
    }

    //   screen.print(10,10, BLACK,text);
 //   snprintf(text, sizeof(text), "Pot 1: %i", Pots.getPot(1));
 //   screen.print(10, 30, BLACK, text);
 //   snprintf(text, sizeof(text), "Pot 2: %i", Pots.getPot(2));
 //   screen.print(10, 50, BLACK, text);
  }

  return 0;
}

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
      {0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1};

    uint8_t cid = packet[0] & 0xF;
#ifdef IGNORE_MIDI_CC
    if (cid != 0x0B && cid != 0x0C)
#endif
    {
      size_t length = cin_to_length[cid];
      if (length)
      {
        uart_write_blocking(DEXED, packet+1, length);
      }
    }

    led_usb_state = true;
  }
  else
  {
    led_usb_state = false;
  }

  // Handle UART to USB direction
  static uint8_t buffer[CFG_TUD_MIDI_TX_BUFSIZE/4*3];
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
