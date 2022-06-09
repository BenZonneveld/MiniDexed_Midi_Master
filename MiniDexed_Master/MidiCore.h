#define DEXED       uart1
//#define MIDIPORT    uart0
#include "pico/util/queue.h"
#include "TG.h"

extern queue_t midi_fifo;
extern bool led_usb_state;
extern bool led_uart_state;

void midi_task(void); 
void led_task(void);
void midicore();
void sendToAllPorts(uint8_t* message, uint8_t len);
void sendCtrl(uint8_t ctrl, dexed_t mididata);
void dx7sysex(uint16_t parm, dexed_t mididata);

