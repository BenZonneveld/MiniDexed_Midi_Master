#include "bsp/board.h"
#include "tusb.h"

#ifdef USE_USB_AUDIO
#define AUDIO_SAMPLE_RATE 48000

extern bool mute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1]; 				          // +1 for master channel 0
extern uint16_t volume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1]; 					// +1 for master channel 0
extern uint32_t sampFreq;
extern uint8_t clkValid;;
// Range states
extern audio_control_range_2_n_t(1) volumeRng[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1]; 			// Volume range state
audio_control_range_4_n_t(1) sampleFreqRng; 						// Sample frequency range state

// Audio test data
extern uint16_t i2s_dummy_buffer[CFG_TUD_AUDIO_FUNC_1_N_TX_SUPP_SW_FIFO][CFG_TUD_AUDIO_FUNC_1_TX_SUPP_SW_FIFO_SZ / 2];   // Ensure half word aligned


extern uint16_t test_buffer_audio[CFG_TUD_AUDIO_EP_SZ_IN / 2];
extern uint16_t startVal;

#endif