/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Reinhard Panhuber
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "stdlib.h"
#include <pico/stdlib.h>
#include "bsp/board.h"
#include "tusb.h"
#include "usb_audio.h"
#include "MidiCore.h"

bool mute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1]; 						// +1 for master channel 0
uint16_t volume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1]; 					// +1 for master channel 0
uint32_t sampFreq;
uint8_t clkValid;
// Range states
audio_control_range_2_n_t(1) volumeRng[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX+1]; 			// Volume range state
audio_control_range_4_n_t(1) sampleFreqRng; 						// Sample frequency range state

uint32_t i2s_buffer[512];   // Ensure half word aligned

/*------------- MAIN -------------*/
void usb_audio_init()
{
  // Init values
  sampFreq = SAMPLE_RATE;
  clkValid = 1;

  sampleFreqRng.wNumSubRanges = 1;
  sampleFreqRng.subrange[0].bMin = SAMPLE_RATE;
  sampleFreqRng.subrange[0].bMax = SAMPLE_RATE;
  sampleFreqRng.subrange[0].bRes = 0;
}

void usb_audio_write()
{

#if USE_MONO

    int32_t* dat = (int32_t *)data;
    //uint16_t* dest = (uint16_t*)calloc(sizeof(uint16_t), len);
//    for (size_t i = 0; i < len; i++)
//    {
//        int32_t val = bswap(dat[i]);
//        dat[i] = val;
//    }
    tud_audio_write((uint8_t*)i2s_buffer, SAMPLE_BUFFER_SIZE);
#else
    //int16_t tx_buffer[SAMPLE_BUFFER_SIZE][4];
    //size_t t = 0;
    //for (size_t i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    //{
    //    tx_buffer[i][0] = i2s_buffer[i][0];
    //    tx_buffer[i][1] = 0;
    //    tx_buffer[i][2] = 0;
    //    tx_buffer[i][3] = 0;
    //}
    tud_audio_write((uint8_t*)i2s_buffer, SAMPLE_BUFFER_SIZE*2);
#endif

    return;
}

//--------------------------------------------------------------------+
// Application Callback API Implementations
//--------------------------------------------------------------------+

// Invoked when audio class specific set request received for an EP
bool tud_audio_set_req_ep_cb(uint8_t rhport, tusb_control_request_t const * p_request, uint8_t *pBuff)
{
  (void) rhport;
  (void) pBuff;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t ep = TU_U16_LOW(p_request->wIndex);

  (void) channelNum; (void) ctrlSel; (void) ep;

  return false; 	// Yet not implemented
}

// Invoked when audio class specific set request received for an interface
bool tud_audio_set_req_itf_cb(uint8_t rhport, tusb_control_request_t const * p_request, uint8_t *pBuff)
{
  (void) rhport;
  (void) pBuff;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);

  (void) channelNum; (void) ctrlSel; (void) itf;

  return false; 	// Yet not implemented
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const * p_request, uint8_t *pBuff)
{
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

  (void) itf;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // If request is for our feature unit
  if ( entityID == 2 )
  {
    switch ( ctrlSel )
    {
      case AUDIO_FU_CTRL_MUTE:
        // Request uses format layout 1
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_1_t));

        mute[channelNum] = ((audio_control_cur_1_t*) pBuff)->bCur;

        TU_LOG2("    Set Mute: %d of channel: %u\r\n", mute[channelNum], channelNum);

      return true;

      case AUDIO_FU_CTRL_VOLUME:
        // Request uses format layout 2
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_2_t));

        volume[channelNum] = ((audio_control_cur_2_t*) pBuff)->bCur;

        TU_LOG2("    Set Volume: %d dB of channel: %u\r\n", volume[channelNum], channelNum);

     return true;

        // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
      return false;
    }
  }
  return false;    // Yet not implemented
}

// Invoked when audio class specific get request received for an EP
bool tud_audio_get_req_ep_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t ep = TU_U16_LOW(p_request->wIndex);

  (void) channelNum; (void) ctrlSel; (void) ep;

  //	return tud_control_xfer(rhport, p_request, &tmp, 1);

  return false; 	// Yet not implemented
}

// Invoked when audio class specific get request received for an interface
bool tud_audio_get_req_itf_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);

  (void) channelNum; (void) ctrlSel; (void) itf;

  return false; 	// Yet not implemented
}

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  // uint8_t itf = TU_U16_LOW(p_request->wIndex); 			// Since we have only one audio function implemented, we do not need the itf value
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

  // Input terminal (Microphone input)
  if (entityID == 1)
  {
    switch (ctrlSel)
    {
      case AUDIO_TE_CTRL_CONNECTOR:;
      // The terminal connector control only has a get request with only the CUR attribute.

      audio_desc_channel_cluster_t ret;

      // Those are dummy values for now
      ret.bNrChannels = 1;
      ret.bmChannelConfig = (audio_channel_config_t)0;
      ret.iChannelNames = 0;

      TU_LOG2("    Get terminal connector\r\n");

      return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*)&ret, sizeof(ret));

      // Unknown/Unsupported control selector
      default: TU_BREAKPOINT(); return false;
    }
  }

  // Feature unit
  if (entityID == 2)
  {
    switch (ctrlSel)
    {
      case AUDIO_FU_CTRL_MUTE:
	// Audio control mute cur parameter block consists of only one byte - we thus can send it right away
	// There does not exist a range parameter block for mute
	TU_LOG2("    Get Mute of channel: %u\r\n", channelNum);
	return tud_control_xfer(rhport, p_request, &mute[channelNum], 1);

      case AUDIO_FU_CTRL_VOLUME:

	switch (p_request->bRequest)
	{
	  case AUDIO_CS_REQ_CUR:
	    TU_LOG2("    Get Volume of channel: %u\r\n", channelNum);
	    return tud_control_xfer(rhport, p_request, &volume[channelNum], sizeof(volume[channelNum]));
	  case AUDIO_CS_REQ_RANGE:
	    TU_LOG2("    Get Volume range of channel: %u\r\n", channelNum);

	    // Copy values - only for testing - better is version below
	    audio_control_range_2_n_t(1) ret;

	    ret.wNumSubRanges = 1;
	    ret.subrange[0].bMin = -90; 	// -90 dB
	    ret.subrange[0].bMax = 90;		// +90 dB
	    ret.subrange[0].bRes = 1; 		// 1 dB steps

	    return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*)&ret, sizeof(ret));

	    // Unknown/Unsupported control
	  default: TU_BREAKPOINT(); return false;
	}

	// Unknown/Unsupported control
	  default: TU_BREAKPOINT(); return false;
    }
  }

  // Clock Source unit
  if (entityID == 4)
  {
    switch (ctrlSel)
    {
      case AUDIO_CS_CTRL_SAM_FREQ:

	// channelNum is always zero in this case

	switch (p_request->bRequest)
	{
	  case AUDIO_CS_REQ_CUR:
	    TU_LOG2("    Get Sample Freq.\r\n");
	    return tud_control_xfer(rhport, p_request, &sampFreq, sizeof(sampFreq));
	  case AUDIO_CS_REQ_RANGE:
	    TU_LOG2("    Get Sample Freq. range\r\n");
	    return tud_control_xfer(rhport, p_request, &sampleFreqRng, sizeof(sampleFreqRng));

	    // Unknown/Unsupported control
	  default: TU_BREAKPOINT(); return false;
	}

	  case AUDIO_CS_CTRL_CLK_VALID:
	    // Only cur attribute exists for this request
	    TU_LOG2("    Get Sample Freq. valid\r\n");
	    return tud_control_xfer(rhport, p_request, &clkValid, sizeof(clkValid));

	    // Unknown/Unsupported control
	  default: TU_BREAKPOINT(); return false;
    }
  }

  TU_LOG2("  Unsupported entity: %d\r\n", entityID);
  return false; 	// Yet not implemented
}

bool tud_audio_tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting)
{
  (void) rhport;
  (void) itf;
  (void) ep_in;
  (void) cur_alt_setting;

//#ifndef USE_MONO
//  finalize_dma();
//  tud_audio_write_support_ff(0,i2s_buffer, SAMPLE_RATE / 1000 * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_CHANNEL_PER_FIFO_TX);
//  tud_audio_write_support_ff(1, i2s_buffer, SAMPLE_RATE / 1000 * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_CHANNEL_PER_FIFO_TX);
//#endif
  return true;
}

bool tud_audio_tx_done_post_load_cb(uint8_t rhport, uint16_t n_bytes_copied, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting)
{
  (void) rhport;
  (void) n_bytes_copied;
  (void) itf;
  (void) ep_in;
  (void) cur_alt_setting;

  //uint16_t dataVal;
//  start_dma(i2s_buffer, APP_BUFFER_SIZE);
  //// Generate dummy data
  //for (uint16_t cnt = 0; cnt < CFG_TUD_AUDIO_FUNC_1_N_TX_SUPP_SW_FIFO; cnt++)
  //{
  //    uint16_t* p_buff = i2s_dummy_buffer[cnt];              // 2 bytes per sample
  //    dataVal = 1;
  //    for (uint16_t cnt2 = 0; cnt2 < SAMPLE_RATE / 1000; cnt2++)
  //    {
  //        for (uint8_t cnt3 = 0; cnt3 < CFG_TUD_AUDIO_FUNC_1_CHANNEL_PER_FIFO_TX; cnt3++)
  //        {
  //            *p_buff++ = dataVal;
  //        }
  //        dataVal++;
  //    }
  //}
  return true;
}

bool tud_audio_set_itf_close_EP_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
  (void) rhport;
  (void) p_request;

  return true;
}