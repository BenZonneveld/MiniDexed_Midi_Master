#include <stdlib.h>
#include <stdio.h>
#include <pico/stdlib.h>

#include "MainMenu.h"
#include "string_table.h"
#include "mdma.h"
#include "tools.h"
//#include "callback_arrays.h"

#define PROGMEM
//#include "fonts/FreeSans9pt7b.h"
//#include "fonts/FreeSans12pt7b.h"
//#include "fonts/FreeSans18pt7b.h"
//#include "fonts/FreeSans24pt7b.h"
#include "fonts/RobotoMono12.h"
#include "fonts/RobotoCondensed.h"
#include "fonts/OpenSansBoldCondensed16.h"

void  (*menucallbacks[][8])() = { &cMenu::selectTG, &cMenu::selectTG ,&cMenu::selectTG ,&cMenu::selectTG ,&cMenu::selectTG ,&cMenu::selectTG ,&cMenu::selectTG ,&cMenu::selectTG };

Adafruit_SPITFT tft = Adafruit_SPITFT();

int8_t cMenu::currentTG;
uint8_t cMenu::menu;
uint8_t cMenu::prev_menu; 
bool cMenu::pflag[3] = { false, false, false};
int16_t cMenu::potparam[4];
int16_t cMenu::potpos[3];
int16_t cMenu::bparam[8];
int16_t cMenu::parampos[8];
bool cMenu::menuNeedFlush;
bool cMenu::TGEnabled[8] = { true , true, true, true, true, true, true, true};

void cMenu::Init()
{
	tft.init(160, 128);
	tft.fillScreen(GREY);
	tft.setTextSize(0);
	tft.setTextColor(WHITE);
	tft.setFont(&Open_Sans_Condensed_Bold_16);
//	tft.println("INIT");

	currentTG = -1;
//	mainmenu();
}

void cMenu::ShowButtonText(uint8_t button)
{
	tft.setFont(&Open_Sans_Condensed_Bold_16);
	int16_t offset = 30;
	int16_t col = 0;
	int16_t row = 0;
	int16_t  x1, y1;
	uint16_t w, h;

	switch (button)
	{
	case BUT5:
		col = MIPI_DISPLAY_WIDTH - (2 * offset);
	case BUT1:
		row = POSA;
		break;
	case BUT6:
		col = MIPI_DISPLAY_WIDTH - (2 * offset);
	case BUT2:
		row = POSB;
		break;
	case BUT7:
		col = MIPI_DISPLAY_WIDTH - (2 * offset);
	case BUT3:
		row = POSC;
		break;
	case BUT8:
		col = MIPI_DISPLAY_WIDTH - (2 * offset) -1;
	case BUT4:
		row = POSD;
		break;
	}

	uint16_t color = tft.color565(0xff, 251, 0x25);
	tft.setTextColor(BLACK);
	if (TGEnabled[button] == false && menu == M_MAIN ) {
		color = LIGHTGREY;
		tft.setTextColor(GREY);
	}
	if (button < 4)
	{
		tft.getTextBounds(menus[menu][button], col, row, &x1, &y1, &w, &h);
		h = 12;
	//	tft.fillCircleHelper(col + (2*offset)-9, row + 4, 4, 1, 9, color);
		tft.writeFillRect(col, row, (2 * offset) - 8, BUTTONHEIGHT, color);
		tft.setCursor(col + offset - (w/2) - 4 , row + h + 2);
		tft.print(menus[menu][button]);
//		printf("%s\n", menus[menu][button]);
	}
	else {
		tft.getTextBounds(menus[menu][button], col, row, &x1 ,&y1, &w, &h);
		h = 12;
	//	tft.fillCircleHelper(col + 4, row + 4, 4, 2, 9, color);
		tft.writeFillRect(col + 4, row, (2 * offset) - 4, BUTTONHEIGHT, color);
		tft.setCursor(col+offset - (w/2) + 4, row + h + 2);
		tft.print(menus[menu][button]);
//		printf("%s\n", menus[menu][button]);
	}
//	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
}

void cMenu::ShowValue(int32_t param, int16_t x0, int16_t y0, int16_t w0, int16_t h0, bool colorflag, uint8_t fontsize)
{
	int16_t  x1, y1;
	uint16_t w, h;
	int32_t value = dexed[currentTG].getValue(param);
	//	printf("Showvalue parm %i, value %i\n", param, value);
	char text[7];
	itoa(value + ranges[param][ROFFSET], text, 10);
	tft.setAddrWindow(x0, y0, w0, h0);
	if (colorflag)
	{
		tft.writeFillRect(x0, y0, w0, h0, WHITE);
	}
	else {
		tft.writeFillRect(x0, y0, w0, h0, tft.color565(64, 64, 255));
	}

	if (param == PCHANNEL)
	{
		if (value == 16)
			sprintf(text, "OMN");
		if (value == 17)
			sprintf(text, "OFF");
	}

	if (param == PMONO)
	{
		sprintf(text, "OFF");
		if (value == 1)
			sprintf(text, "ON");
	}

	tft.setTextColor(BLACK);
	tft.setFont(&Open_Sans_Condensed_Bold_16);
	tft.getTextBounds(text, x0, y0, &x1, &y1, &w, &h);
	tft.setCursor((x0 +(w0/2) - (w / 2)), y0 + h + 2);
	tft.print(text);
	tft.setTextColor(WHITE);
	tft.setAddrWindow(0,0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
}

void cMenu::menuBack(uint8_t button)
{
	menuBack();
}

void cMenu::menuBack()
{
	
	menu = prev_menu;
	switch (menu)
	{
	case M_MAIN:
		mainmenu();
		break;
	case M_TG:
		selectTG();
		break;
	case M_TG_FILT:
	case M_TG_MIDI:
	case M_TG_TUNE:
	case M_TG_OUT:
	case M_TG_PITCH:
	default:
//		printf("default\r\n");
		break;
	}
}

void cMenu::ParmToggle(uint8_t button)
{
	uint16_t parm = bparam[button];
	uint16_t val = dexed[currentTG].getValue(parm);
	if ( val == 1)
	{
		dexed[currentTG].parmDown(bparam[button]);
	}
	else {
		dexed[currentTG].parmUp(bparam[button]);
	}
	dexed[currentTG].sendParam(bparam[button], dexed[currentTG].getValue(bparam[button]));
	//	dexed[currentTG].sendMidi(bparam[button]);
	ShowValue(bparam[button], 60, parampos[button], VALUEWIDTH, BUTTONHEIGHT, true, 1);
	if (menu == M_MAIN)
		showTGInfo(bparam[button]);
}

void cMenu::ParmSelect(uint8_t button)
{
	switch (button)
	{
	case BUT1:
	case BUT2:
	case BUT3:
	case BUT4:
		dexed[currentTG].parmDown(bparam[button]);
		break;
	case BUT5:
	case BUT6:
	case BUT7:
	case BUT8:
		dexed[currentTG].parmUp(bparam[button]);
		break;
	default:
		break;
	}
	dexed[currentTG].sendParam(bparam[button], dexed[currentTG].getValue(bparam[button]));
//	dexed[currentTG].sendMidi(bparam[button]);
	if (bparam[button] == PPATCH || bparam[button] == PBANK) dexed[currentTG].getPatch();
	ShowValue(bparam[button], 60, parampos[button], VALUEWIDTH, BUTTONHEIGHT, true, 1);
	if ( menu != M_MAIN)
		showTGInfo(bparam[button]);
}

void cMenu::ParmPot(uint8_t channel)
{
	uint16_t param = potparam[channel];
	int32_t value = map(Pots.getPot(channel), POT_MIN, POT_MAX, ranges[param][RMIN], ranges[param][RMAX]);
	if (value == dexed[currentTG].getValue(param) && !pflag[channel])
	{
		pflag[channel] = true;
		ShowValue(param, 60, potpos[channel], VALUEWIDTH, BUTTONHEIGHT, pflag[channel], 1);
	}
	if (value != dexed[currentTG].getValue(param) && pflag[channel])
	{
		value = dexed[currentTG].setValue(param,value);
		ShowValue(param, 60, potpos[channel], VALUEWIDTH, BUTTONHEIGHT, pflag[channel], 1);
		dexed[currentTG].sendParam(param, dexed[currentTG].getValue(param));
		sleep_ms(10);
		if (param == PPATCH || param == PBANK) dexed[currentTG].getPatch();
	}
	showTGInfo(param);
}

void cMenu::setButtonCallbackWithParam(uint8_t button, int16_t param, int16_t pos,void (*callback)(uint8_t button))
{
	buttons.setCallback(button, callback);
	if (callback == nullptr)
		bparam[button] = -1;
	bparam[button] = param;
	parampos[button] = pos;
	ShowButtonText(button);
}

void cMenu::setButtonParm(uint8_t button, int16_t param, int16_t pos, bool haslongpress)
{
	setButtonCallbackWithParam(button, param, pos, &cMenu::ParmSelect);
	if (haslongpress)
	{
		buttons.setLongCallback(button, &cMenu::ParmSelect);
		buttons.setDBLCallback(button, &cMenu::ParmSelect);
	}
	else
	{
		buttons.setLongCallback(button, NULL);
		buttons.setDBLCallback(button, NULL);
	}
}

void cMenu::clearCallbacks()
{
	for (uint8_t button = 0; button < 8; button++)
	{
		buttons.setCallback(button, NULL);		buttons.setLongCallback(button, NULL);
		buttons.setDBLCallback(button, NULL);
		ShowButtonText(button);
	}
	resetPotCB(TOPPOT);
	resetPotCB(MIDPOT);
	resetPotCB(BOTPOT);

}

void cMenu::setPotCallback(uint8_t channel, int16_t param, int16_t pos)
{
	Pots.setPotCallback(channel, &cMenu::ParmPot);
	potparam[channel] = param;
	potpos[channel] = pos;
	pflag[channel] = false;

	uint16_t pot = map(Pots.getPot(channel), POT_MIN, POT_MAX, ranges[param][RMIN], ranges[param][RMAX]);

	if (pot == dexed[currentTG].getValue(param))
		pflag[channel] = true;
}

void cMenu::resetPotCB(uint8_t channel)
{
	Pots.setPotCallback(channel, NULL);
}

void cMenu::showTGInfo(int16_t param)
{
	int16_t  x1, y1;
	uint16_t w, h;
	char infostring[11] = { 0 };
	if (menu != M_MAIN)
	{
//		tft.writeFillRect(0, 0, 50, 32, BLACK);
//		tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
		tft.setFont(&Roboto_Bold_10);
		// Current TG
		if (param == -1 )
		{	
			tft.writeFillRect(0, 0, 23, 9, BLACK);
			sprintf(infostring, "T %1i", currentTG + 1);
			tft.setCursor(0, 9);
			tft.print(infostring);
		}
		// Bank Number
		if (param == -1 || param == PBANK)
		{
			tft.writeFillRect(24, 0, 36, 9, BLACK);
			sprintf(infostring, "B %3i", dexed[currentTG].getValue(PBANK) + 1);
			tft.setCursor(24, 9);
			tft.print(infostring);
		}
		// Channel
		if (param == -1 || param == PCHANNEL)
		{
			tft.writeFillRect(0, 10, 23, 8, BLACK);
			sprintf(infostring, "C %2i", dexed[currentTG].getValue(PCHANNEL) + 1);
			if (dexed[currentTG].getValue(PCHANNEL) == 16)
			{
				sprintf(infostring, "C OM");
			}
			if (dexed[currentTG].getValue(PCHANNEL) == 17)
			{
				sprintf(infostring, "C --");
			}
			tft.setCursor(0, 18);
			tft.print(infostring);
		}
		// Patch Number
		if (param == -1 || param == PPATCH)
		{
			tft.writeFillRect(24, 10, 26, 8, BLACK);
			sprintf(infostring, "P %3i", dexed[currentTG].getValue(PPATCH) + 1);
			tft.setCursor(24, 18);
			tft.print(infostring);
		}
		// Volume 
		if (param == -1 || param == PVOL)
		{
			tft.writeFillRect(0, 19, 19, 8, BLACK);
			uint8_t vol = dexed[currentTG].getValue(PVOL);
			if (vol != 0)
				tft.fillTriangle(0, 27, (vol / 8), 27, (vol / 8), 27 - (vol / 16), WHITE);
		}

		if (param == -1 || param == PPAN)
		{
			// Panning
			int8_t pan = (dexed[currentTG].getValue(PPAN) + 63) / 4.8f;
			printf("pan: %i\n", pan);
			tft.writeFillRect(21, 19, 26, 8, BLACK);
			tft.fillTriangle(34, 23, 21, 19, 21, 27, GREY);
			tft.fillTriangle(34, 23, 47, 19, 47, 27, GREY);
			tft.writeFillRect(21 + pan, 19, 2, 8, WHITE);
		}
		//		tft.fillTriangle(24, 34, 34 - pan/2), 19, 34-pan, 27 - pan/2, WHITE);
		// Voice Name
		if (param == -1)
		{
			sprintf(infostring, "%s", dexed[currentTG].getVoiceName());
			tft.setFont(&Open_Sans_Condensed_Bold_16);
			tft.getTextBounds(infostring, 0, 14, &x1, &y1, &w, &h);
			tft.setCursor(105 - (w / 2), 32);
			tft.fillRect(50, 16, 110, 20, GREY);
			tft.print(infostring);
		}
	}
}

void cMenu::setDexedParm(uint16_t parm, int32_t val, uint8_t instance)
{
	int32_t rval = dexed[instance].setValue(parm, val);
//	printf("parm %02X sysex: %02X dexed[%i] %04X\n", parm, val, instance, rval);
}

void cMenu::handleSysex(sysex_t raw_sysex)
{
	if ( (raw_sysex.buffer[2] & 0x70) == 0 && raw_sysex.buffer[3] == 0)
	{
		uint8_t channel = (raw_sysex.buffer[2] & 0xf);
		if (channel < 8)
		{
			dexed[channel].setSysex(raw_sysex);
		}
		else {
			dexed[currentTG].setSysex(raw_sysex);
		}
		showTGInfo(-1);
	}
	if (raw_sysex.buffer[2] == 0x31)
	{
		uint8_t config = 3;
		for (uint8_t i = 0; i < 8; i++)
		{
			setDexedParm(PBANK, raw_sysex.buffer[config++], i);
			setDexedParm(PPATCH, raw_sysex.buffer[config++], i);
			setDexedParm(PCHANNEL, raw_sysex.buffer[config++], i);
			printf("Channel %i for TG %i\n", dexed[i].getValue(PCHANNEL), i);
			setDexedParm(PVOL, raw_sysex.buffer[config++], i);
			setDexedParm(PPAN, raw_sysex.buffer[config++], i);
			uint8_t DetuneMSB = raw_sysex.buffer[config++];
			uint8_t DetuneLSB = raw_sysex.buffer[config++];
			int16_t detune = (DetuneMSB << 7) | DetuneLSB;
			if ((detune >> 13) == 1) {
				detune = detune | 0xf000; // Make the number negative
			}
			setDexedParm(PTUNE, detune, i);
			setDexedParm(PFREQ, raw_sysex.buffer[config++], i);
			setDexedParm(PRESO, raw_sysex.buffer[config++], i);
			setDexedParm(PLOW, raw_sysex.buffer[config++],i); // Note Limit Low
			setDexedParm(PHIGH, raw_sysex.buffer[config++],i); // Note Limit High
			setDexedParm(PSHIFT, raw_sysex.buffer[config++], i); // Note Shift
			setDexedParm(PVERB, raw_sysex.buffer[config++], i);
			setDexedParm(PBRANGE, raw_sysex.buffer[config++], i);
			setDexedParm(PBSTEP, raw_sysex.buffer[config++], i); // Pitch Bend Step
			setDexedParm(PPMODE, raw_sysex.buffer[config++], i); // Porta Mode
			setDexedParm(PGLISS, raw_sysex.buffer[config++], i); // Porta Gliss
			setDexedParm(PTIME, raw_sysex.buffer[config++], i); // Porta Time
			uint8_t VoiceData = raw_sysex.buffer[config++];
			uint8_t fu = raw_sysex.buffer[config++];
			fu = raw_sysex.buffer[config++];
			fu= raw_sysex.buffer[config++];
			fu = raw_sysex.buffer[config++];
			fu = raw_sysex.buffer[config++];
		}
	}
}

void cMenu::TGEnable(uint8_t button)
{
	if (TGEnabled[button] == true)
	{
		TGEnabled[button] = false;
		dexed[button].sendParam(PCHANNEL, 17); // Set TG Off
	}
	else {
		TGEnabled[button] = true;
		dexed[button].sendParam(PCHANNEL, dexed[button].getValue(PCHANNEL));
	}
	ShowButtonText(button);
	printf("TGEnable");
}

void cMenu::mainmenu()
{
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
	tft.writeFillRect(0,0,MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT, BLACK);
	menu = M_MAIN;
	prev_menu = M_MAIN;
	dexed[0].getConfig();
	clearCallbacks();
	setButtonCallbackWithParam(BUT1, 0, POSA, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT2, 0, POSB, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT3, 0, POSC, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT4, 0, POSD, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT5, 0, POSA, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT6, 0, POSB, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT7, 0, POSC, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT8, 0, POSD, &cMenu::selectTG);
	buttons.setDBLCallback(BUT1, &cMenu::TGEnable);
	buttons.setDBLCallback(BUT2, &cMenu::TGEnable);
	buttons.setDBLCallback(BUT3, &cMenu::TGEnable);
	buttons.setDBLCallback(BUT4, &cMenu::TGEnable);
	buttons.setDBLCallback(BUT5, &cMenu::TGEnable);
	buttons.setDBLCallback(BUT6, &cMenu::TGEnable);
	buttons.setDBLCallback(BUT7, &cMenu::TGEnable);
	buttons.setDBLCallback(BUT8, &cMenu::TGEnable);
}

void cMenu::Midi(uint8_t button)
{
	printf("MIDI Menu\n");
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
	tft.writeFillRect(VALUEPOS, POSA, VALUEWIDTH, MIPI_DISPLAY_HEIGHT-POSA, BLACK);
	//	tft.fillScreen(BLACK);
	menu = M_TG_MIDI;
	prev_menu = M_TG;

	clearCallbacks();
	setButtonParm(BUT1, PBANK, POSA, true);
	setButtonParm(BUT2, PPATCH, POSB, true);
	setButtonParm(BUT3, PCHANNEL, POSC, true);
	setButtonParm(BUT5, PBANK, POSA, true);
	setButtonParm(BUT6, PPATCH, POSB, true);
	setButtonParm(BUT7, PCHANNEL, POSC, true);
	buttons.setCallback(BUT8, &cMenu::menuBack);

	setPotCallback(TOPPOT, PBANK, POSA);
	setPotCallback(MIDPOT, PPATCH, POSB);
	setPotCallback(BOTPOT, PCHANNEL, POSC);

	ShowValue(PBANK, VALUEPOS, potpos[TOPPOT], VALUEWIDTH, BUTTONHEIGHT, pflag[TOPPOT], 1);
	ShowValue(PPATCH, VALUEPOS, potpos[MIDPOT], VALUEWIDTH, BUTTONHEIGHT, pflag[MIDPOT], 1);
	ShowValue(PCHANNEL, VALUEPOS, potpos[BOTPOT], VALUEWIDTH, BUTTONHEIGHT, pflag[BOTPOT], 1);
	showTGInfo(-1);
}

void cMenu::selectTG(uint8_t button)
{
	currentTG = button;
	dexed[currentTG].getPatch();
	selectTG();
}

void cMenu::selectTG()
{
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
	tft.writeFillRect(VALUEPOS, POSA, VALUEWIDTH, MIPI_DISPLAY_HEIGHT - POSA, BLACK);
	printf("TG: %i Voice: %s\n", currentTG, dexed[currentTG].getVoiceName());
	menu = M_TG;
	prev_menu = M_MAIN;

	clearCallbacks();

	buttons.setCallback(BUT1, &cMenu::Midi);
	buttons.setCallback(BUT2, &cMenu::TGFilter);
	buttons.setCallback(BUT3, &cMenu::TGTune);
	buttons.setCallback(BUT4, &cMenu::TGOut);
	buttons.setCallback(BUT5, &cMenu::TGPitch);
	buttons.setCallback(BUT8, &cMenu::menuBack);

	setPotCallback(TOPPOT, PFREQ, POSA);
	setPotCallback(MIDPOT, PRESO, POSB);
	setPotCallback(BOTPOT, PVERB, POSC);
	pflag[TOPPOT] = pflag[MIDPOT] = pflag[BOTPOT] = true;

	ShowValue(PFREQ, VALUEPOS, potpos[TOPPOT], VALUEWIDTH, BUTTONHEIGHT, pflag[TOPPOT], 1);
	ShowValue(PRESO, VALUEPOS, potpos[MIDPOT], VALUEWIDTH, BUTTONHEIGHT, pflag[MIDPOT], 1);
	ShowValue(PVERB, VALUEPOS, potpos[BOTPOT], VALUEWIDTH, BUTTONHEIGHT, pflag[BOTPOT], 1);

	showTGInfo(-1);
}

void cMenu::TGFilter(uint8_t button)
{
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
	tft.writeFillRect(VALUEPOS, POSA, VALUEWIDTH, MIPI_DISPLAY_HEIGHT - POSA, BLACK);

	menu = M_TG_FILT;
	prev_menu = M_TG;

	clearCallbacks();
	setButtonParm(BUT1, PFREQ, POSA, true);
	setButtonParm(BUT2, PRESO, POSB, true);
	setButtonParm(BUT3, PVERB, POSC, true);
	setButtonParm(BUT5, PFREQ, POSA, true);
	setButtonParm(BUT6, PRESO, POSB, true);
	setButtonParm(BUT7, PVERB, POSC, true);
	buttons.setCallback(BUT8, &cMenu::menuBack);

	setPotCallback(TOPPOT, PFREQ, POSA);
	setPotCallback(MIDPOT, PRESO, POSB);
	setPotCallback(BOTPOT, PVERB, POSC);
	pflag[TOPPOT] = pflag[MIDPOT] = pflag[BOTPOT] = true;

	ShowValue(PFREQ, VALUEPOS, potpos[TOPPOT], VALUEWIDTH, BUTTONHEIGHT, pflag[TOPPOT], 1);
	ShowValue(PRESO, VALUEPOS, potpos[MIDPOT], VALUEWIDTH, BUTTONHEIGHT, pflag[MIDPOT], 1);
	ShowValue(PVERB, VALUEPOS, potpos[BOTPOT], VALUEWIDTH, BUTTONHEIGHT, pflag[BOTPOT], 1);
	showTGInfo(-1);
}

void cMenu::TGTune(uint8_t button)
{
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
	tft.writeFillRect(VALUEPOS, POSA, VALUEWIDTH, MIPI_DISPLAY_HEIGHT - POSA, BLACK);

	menu = M_TG_TUNE;
	prev_menu = M_TG;
	clearCallbacks();

	setButtonParm(BUT1, PSHIFT, POSA, true);
	setButtonParm(BUT2, PTUNE, POSB, true);
	setButtonParm(BUT5, PSHIFT, POSA, true);
	setButtonParm(BUT6, PTUNE, POSB, true);
	buttons.setCallback(BUT8, &cMenu::menuBack);

	setPotCallback(TOPPOT, PSHIFT, POSA);
	setPotCallback(MIDPOT, PTUNE, POSB);

	ShowValue(PSHIFT, VALUEPOS, parampos[BUT1], VALUEWIDTH, BUTTONHEIGHT, pflag[TOPPOT], 1);
	ShowValue(PTUNE, VALUEPOS, parampos[BUT2], VALUEWIDTH, BUTTONHEIGHT, pflag[MIDPOT], 1);
	showTGInfo(-1);
}

void cMenu::TGOut(uint8_t button)
{
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
	tft.writeFillRect(VALUEPOS, POSA, VALUEWIDTH, MIPI_DISPLAY_HEIGHT - POSA, BLACK);

	menu = M_TG_OUT;
	prev_menu = M_TG;
	clearCallbacks();

	setButtonParm(BUT1, PPAN, POSA, true);
	setButtonParm(BUT2, PVOL, POSB, true);
	setButtonParm(BUT5, PPAN, POSA, true);
	setButtonParm(BUT6, PVOL, POSB, true);
	buttons.setCallback(BUT8, &cMenu::menuBack);

	setPotCallback(TOPPOT, PPAN, POSA);
	setPotCallback(MIDPOT, PVOL, POSB);

	ShowValue(PPAN, VALUEPOS, parampos[BUT1], VALUEWIDTH, BUTTONHEIGHT, pflag[TOPPOT], 1);
	ShowValue(PVOL, VALUEPOS, parampos[BUT2], VALUEWIDTH, BUTTONHEIGHT, pflag[MIDPOT], 1);
	showTGInfo(-1);
}

void cMenu::TGPitch(uint8_t button)
{
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
	tft.writeFillRect(VALUEPOS, POSA, VALUEWIDTH, MIPI_DISPLAY_HEIGHT - POSA, BLACK);

	menu = M_TG_PITCH;
	prev_menu = M_TG;
	clearCallbacks();
	setButtonParm(BUT1, PBRANGE, POSA, true);
	setButtonParm(BUT2, PBSTEP, POSB, true);
	// Todo: Portamento menu
	setButtonCallbackWithParam(BUT4, PMONO, POSC, &cMenu::ParmToggle);
//	setButtonParm(BUT3, PMONO, POSC, true);
	setButtonParm(BUT5, PBRANGE, POSA, true);
	setButtonParm(BUT6, PBSTEP, POSB, true);
	buttons.setCallback(BUT8, &cMenu::menuBack);

	setPotCallback(TOPPOT, PBRANGE, POSA);
	setPotCallback(MIDPOT, PBSTEP, POSB);
	setPotCallback(BOTPOT, PMONO, POSC);

	ShowValue(PBRANGE, VALUEPOS, parampos[BUT1], VALUEWIDTH, BUTTONHEIGHT, pflag[TOPPOT], 1);
	ShowValue(PBSTEP, VALUEPOS, parampos[BUT2], VALUEWIDTH, BUTTONHEIGHT, pflag[MIDPOT], 1);
	ShowValue(PMONO, VALUEPOS, parampos[BUT3], VALUEWIDTH, BUTTONHEIGHT, pflag[BOTPOT], 1);
	showTGInfo(-1);
}