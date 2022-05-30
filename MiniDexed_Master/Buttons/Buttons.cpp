#include <stdio.h>
#include <stdlib.h>
#include <pico/stdlib.h>

#include <hardware/spi.h>
#include "hardware/gpio.h"

#include "Buttons.h"
#include "gpio_pins.h"
#include "spi_helper.h"

std::array<void(*) (uint8_t), 8>cButtons::buttonCallback;
std::array<void(*)(), 8>cButtons::buttonDBLCallback;

cButtons::cButtons()
{
    printf("cButtons INIT\r\n");
    buttonCallback[0] = NULL;
    buttonCallback[1] = NULL;
    buttonCallback[2] = NULL;
    buttonCallback[3] = NULL;
    buttonCallback[4] = NULL;
    buttonCallback[5] = NULL;
    buttonCallback[6] = NULL;
    buttonCallback[7] = NULL;
    buttonDBLCallback[0] = NULL;
    buttonDBLCallback[1] = NULL;
    buttonDBLCallback[2] = NULL;
    buttonDBLCallback[3] = NULL;
    buttonDBLCallback[4] = NULL;
    buttonDBLCallback[5] = NULL;
    buttonDBLCallback[6] = NULL;
    buttonDBLCallback[7] = NULL;
    cs = KEY_CS;
    gpio_init(cs);
    gpio_set_dir(cs, GPIO_OUT);
    gpio_put(cs, 0);

    // Button timing variables
    debounce = 20;          // ms debounce period to prevent flickering when pressing or releasing the button
    DCgap = 250;            // max ms between clicks for a double click event
    holdTime = 1000;        // ms hold period: how long to wait for press+hold event
    longHoldTime = 3000;    // ms long hold period: how long to wait for press+hold event

    // Button variables
    for (uint8_t i = 0; i < NBR_OF_BUTTONS; i++)
    {         
//        buttonVal[i] = true;   // value read from button
        buttonLast[i] = false;  // buffered value of the button's previous state
        DCwaiting[i] = false;  // whether we're waiting for a double click (down)
        DConUp[i] = false;     // whether to register a double click on next release, or whether to wait and click
        singleOK[i] = true;    // whether it's OK to do a single click
        downTime[i] = -1;         // time the button was pressed down
        upTime[i] = -1;           // time the button was released
        ignoreUp[i] = false;   // whether to ignore the button release because the click+hold was triggered
        waitForUp[i] = false;        // when held, whether to wait for the up event
        holdEventPast[i] = false;    // whether or not the hold event happened already
        longHoldEventPast[i] = false;// whether or not the long hold event happened already
        event[i] = BTNONE;
    }
    
    printf("MultiClick Init\r\n");

}

BTNEVNT cButtons::getState(uint8_t btn)
{

	return event[btn];
}

void cButtons::setCallback(uint8_t button, void (* callback)(uint8_t button))
{
    buttonCallback[button] = callback;
}

void cButtons::setDBLCallback(uint8_t button, void (*callback)())
{
    buttonDBLCallback[button] = callback;
}

void cButtons::handleButtons()
{
	uint8_t buf = 0;
	spi_cs(csKEY);
	spi_read_blocking(spi1, 0, &buf, 1);
	state = (buf & 0xF0)>>4 | ((buf & 0x8) >> 3| (buf&0x4) >> 1| (buf&0x2)<<1 | (buf&0x1)<< 3)<<4; // Swap nibble to get the correct order
	spiAllHigh();
    for (uint8_t i = 0; i < NBR_OF_BUTTONS; i++)
    {
        checkButton(((state >> i) & 0x1), i);
        doCallback(i);
    }
}

BTNEVNT cButtons::checkButton(bool buttonVal, uint8_t btn) {
    BTNEVNT butevent = BTNONE;

    unsigned long Millis = to_ms_since_boot(get_absolute_time());
    // Button pressed down
    if (buttonVal == true && buttonLast[btn] == false && (Millis - upTime[btn]) > debounce)
    {
        downTime[btn] = Millis;
        ignoreUp[btn] = false;
        waitForUp[btn] = false;
        singleOK[btn] = true;
        holdEventPast[btn] = false;
        longHoldEventPast[btn] = false;
        if ((Millis - upTime[btn]) < DCgap && DConUp[btn] == false && DCwaiting[btn] == true)  DConUp[btn] = true;
        else  DConUp[btn] = false;
        DCwaiting[btn] = false;
    }
    // Button released
    else if (buttonVal == false && buttonLast[btn] == true && (Millis - downTime[btn]) > debounce)
    {
        if (!ignoreUp[btn])
        {
            upTime[btn] = Millis;
            if (DConUp[btn] == false) DCwaiting[btn] = true;
            else
            {
                butevent = BTDBLC;
                DConUp[btn] = false;
                DCwaiting[btn] = false;
                singleOK[btn] = false;
//                printf("Double Click\r\n");
            }
        }
    }
    // Test for normal click event: DCgap expired
    if (buttonVal == false && (Millis - upTime[btn]) >= DCgap && DCwaiting[btn] == true && DConUp[btn] == false && singleOK[btn] == true && butevent != 2)
    {
        butevent = BTCLICK;
        DCwaiting[btn] = false;
    }
    // Test for hold
    if (buttonVal == true && (Millis - downTime[btn]) >= holdTime) {
        // Trigger "normal" hold
        if (!holdEventPast[btn])
        {
            butevent = BTHOLD;
            waitForUp[btn] = true;
            ignoreUp[btn] = true;
            DConUp[btn] = false;
            DCwaiting[btn] = false;
            //downTime = millis();
            holdEventPast[btn] = false;
        }
    }
    buttonLast[btn] = buttonVal;
  
    //switch (butevent)
    //{
    //case BTNONE:
    //    break;
    //case BTCLICK:
    //    printf("Click\r\n");
    //    break;
    //case BTDBLC:
    //    printf("Double Click\r\n");
    //    break;
    //case BTHOLD:
    //    printf("Hold\r\n");
    //    break;
    //case BTLONG:
    //    printf("Long Hold\r\n");
    //    break;
    //default:
    //    break;
    //}
   
    event[btn] = butevent;
    return butevent;
}

void cButtons::doCallback(uint8_t button)
{
    switch (event[button])
    {
    case BTNONE:
        break;
    case BTCLICK:
        buttonCallback[button](button);
        break;
    case BTDBLC:
        buttonDBLCallback[button]();
        break;
    case BTHOLD:
        buttonCallback[button](button);
        break;
    default:
        break;
    }
}