#define DEXED       uart1
//#define MIDIPORT    uart0
#include "pico/util/queue.h"
#include "TG.h"

#define APP_BUFFER_SIZE (((CFG_TUD_AUDIO_EP_SZ_IN/CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)/2) - 1)

extern queue_t midi_fifo;
extern queue_t sysex_fifo;
extern queue_t tx_fifo;

extern bool led_usb_state;
extern bool led_uart_state;

#define bswap(x) \
  ((((x) & 0xff000000u) >> 24) | (((x) & 0x00ff0000u) >> 8)	\
   | (((x) & 0x0000ff00u) << 8) | (((x) & 0x000000ffu) << 24))

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
void on_usb_audio_tx_ready();

void i2s_init();
static void start_dma(int32_t* buf, size_t len);
static void finalize_dma();
static void i2s_print_samples(int32_t* samples, size_t len);
