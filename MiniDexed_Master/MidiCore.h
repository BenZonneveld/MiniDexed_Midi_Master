#define DEXED       uart1
//#define MIDIPORT    uart0
#include "pico/util/queue.h"
#include "TG.h"

extern queue_t midi_fifo;
extern queue_t sysex_fifo;
extern queue_t tx_fifo;

extern bool led_usb_state;
extern bool led_uart_state;

#define APP_BUFFER_SIZE 4096

void midi_task(void); 
void led_task(void);
void midicore();
void dispatcher(dexed_t mididata);
void sendToAllPorts(uint8_t* message, uint8_t len);
void sendCtrl(uint8_t ctrl, dexed_t mididata);
void dx7sysex(uint16_t parm, dexed_t mididata);
void dexedPatchRequest(dexed_t mididata);
void dexedGetBankName(dexed_t mididata);
void dexedConfigRequest();
void midiParser(uint8_t *buffer, size_t length);
void parseSysex(uint8_t buf);
void parseCtrls(uint8_t buf);
void handleMidi(sysex_t raw_sysex);