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
bool cMenu::pflag[3];
int16_t cMenu::potparam[4];
int16_t cMenu::potpos[3];
int16_t cMenu::bparam[8];
int16_t cMenu::parampos[8];
bool cMenu::menuNeedFlush;

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
	if (button < 4)
	{
		tft.getTextBounds(menus[menu][button], col, row, &x1, &y1, &w, &h);
 		h = 12;
//		tft.fillRoundRect(col, row, (2 * offset) - 4, 21, 4, tft.color565(128, 64, 64));
		tft.writeFillRect(col, row, (2 * offset) - 8, h + 4, tft.color565(0xff, 0x99, 0));
		tft.setCursor(col + offset - (w/2) - 4 , row + h + 2);
		tft.print(menus[menu][button]);
//		printf("%s\n", menus[menu][button]);
	}
	else {
		tft.getTextBounds(menus[menu][button], col, row, &x1 ,&y1, &w, &h);
		h = 12;
		//		tft.drawRoundRect(col + 4, row, (2 * offset) - 8, 21, 4, tft.color565(128, 64, 64));
		tft.fillRect(col + 4, row, (2 * offset) - 4, h + 4, tft.color565(0xff, 0x99, 0));
		tft.setCursor(col+offset - (w/2) + 4, row + h + 2);
		tft.print(menus[menu][button]);
//		printf("%s\n", menus[menu][button]);
	}
}

void cMenu::ShowValue(int32_t param, int16_t x0, int16_t y0, int16_t w0, int16_t h0, bool colorflag, uint8_t fontsize)
{	
	int16_t  x1, y1;
	uint16_t w, h;
	int32_t value = dexed[currentTG].getValue(param);

	char text[7];
	itoa(value + ranges[param][ROFFSET], text, 10);
	tft.setAddrWindow(x0, y0, w0, h0);
	if ( colorflag)
	{
		tft.fillRect(x0,y0,w0,h0,WHITE);
	}
	else {
		tft.fillRect(x0, y0, w0, h0, tft.color565(64, 64, 255));
	}

	if (param == PCHANNEL)
	{
		if ( value == 16 )
			sprintf(text, "OMN");
		if (value == 17)
			sprintf(text, "OFF");
	}

	tft.setTextColor(BLACK);
	tft.setFont(&Open_Sans_Condensed_Bold_16);
	tft.getTextBounds(text, x0, y0, &x1, &y1, &w, &h);
	tft.setCursor((x0 +(w0/2) - (w / 2)), y0 + h + 2);
	tft.print(text);
	tft.setTextColor(WHITE);
	tft.setAddrWindow(0,0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1);
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
		printf("default\r\n");
		break;
	}
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
	ShowValue(bparam[button], 61, parampos[button], VALUEWIDTH, 21, true, 1);
	showTGInfo();
}

void cMenu::ParmPot(uint8_t channel)
{
	uint16_t param = potparam[channel];
	int32_t value = map(Pots.getPot(channel), POT_MIN, POT_MAX, ranges[param][RMIN], ranges[param][RMAX]);
	if (value == dexed[currentTG].getValue(param) && !pflag[channel])
	{
		pflag[channel] = true;
		ShowValue(param, 61, potpos[channel], VALUEWIDTH, 21, pflag[channel], 1);
	}
	if (value != dexed[currentTG].getValue(param) && pflag[channel])
	{
		value = dexed[currentTG].setValue(param,value);
		ShowValue(param, 61, potpos[channel], VALUEWIDTH, 21, pflag[channel], 1);
		dexed[currentTG].sendParam(param, dexed[currentTG].getValue(param));
	}
	showTGInfo();
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

void cMenu::showTGInfo()
{
	int16_t  x1, y1;
	uint16_t w, h;
	char voicename[11] = { 0 };
	if (menu != M_MAIN)
	{
		tft.writeFillRect(0, 0, 50, 32, BLACK);
		tft.setFont(&Roboto_Bold_10);
		sprintf(voicename, "T%1i B %3i", currentTG + 1, dexed[currentTG].getValue(PBANK)+1);
		tft.getTextBounds(voicename, 0, 14, &x1, &y1, &w, &h);
		tft.setCursor(0, 9);
		tft.print(voicename);

		sprintf(voicename, "C %2i P %2i", dexed[currentTG].getValue(PCHANNEL) + 1, dexed[currentTG].getValue(PPATCH) + 1);
		if (dexed[currentTG].getValue(PCHANNEL) == 16)
		{
			sprintf(voicename, "C OM P %2i", dexed[currentTG].getValue(PPATCH) + 1);
		}
		if (dexed[currentTG].getValue(PCHANNEL) == 17)
		{
			sprintf(voicename, "C -- P %2i", dexed[currentTG].getValue(PPATCH) + 1);
		}
		tft.getTextBounds(voicename, 0, 14, &x1, &y1, &w, &h);
		tft.setCursor(0, 18);
		tft.print(voicename);

		sprintf(voicename, "%s", dexed[currentTG].getVoiceName());
		tft.setFont(&Open_Sans_Condensed_Bold_16);
		tft.getTextBounds(voicename, 0, 14, &x1, &y1, &w, &h);
		tft.setCursor(105 - (w / 2), 30);
		tft.writeFillRect(50, 16, 110, 16, GREY);
		tft.print(voicename);
	}
}

void cMenu::handleSysex(sysex_t raw_sysex)
{
	dexed[currentTG].setSysex(raw_sysex);
	showTGInfo();
}

void cMenu::mainmenu()
{
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1);
	tft.fillScreen(BLACK);
	menu = M_MAIN;
	prev_menu = M_MAIN;

	clearCallbacks();
	setButtonCallbackWithParam(BUT1, 0, POSA, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT2, 0, POSB, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT3, 0, POSC, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT4, 0, POSD, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT5, 0, POSA, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT6, 0, POSB, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT7, 0, POSC, &cMenu::selectTG);
	setButtonCallbackWithParam(BUT8, 0, POSD, &cMenu::selectTG);
}

void cMenu::Midi(uint8_t button)
{
	printf("MIDI Menu\n");
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1);
	tft.fillScreen(BLACK);
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

	ShowValue(PBANK, 61, potpos[TOPPOT], VALUEWIDTH, 16, pflag[TOPPOT], 1);
	ShowValue(PPATCH, 61, potpos[MIDPOT], VALUEWIDTH, 16, pflag[MIDPOT], 1);
	ShowValue(PCHANNEL, 61, potpos[BOTPOT], VALUEWIDTH, 16, pflag[BOTPOT], 1);
	showTGInfo();
}

void cMenu::selectTG(uint8_t button)
{
	printf("Select TG with button\n");
	currentTG = button;
	dexed[currentTG].getPatch();
	selectTG();
}

void cMenu::selectTG()
{
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1);
	tft.fillScreen(BLACK);
	printf("Select TG\n");
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

	showTGInfo();
	printf("End of selectTG\n");
}

void cMenu::TGFilter(uint8_t button)
{
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1);
	tft.fillScreen(BLACK);

	printf("TG Filter\n");
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

	ShowValue(PFREQ, 61, potpos[TOPPOT], VALUEWIDTH, 16, pflag[TOPPOT], 1);
	ShowValue(PRESO, 61, potpos[MIDPOT], VALUEWIDTH, 16, pflag[MIDPOT], 1);
	ShowValue(PVERB, 61, potpos[BOTPOT], VALUEWIDTH, 16, pflag[BOTPOT], 1);
	showTGInfo();
}

void cMenu::TGTune(uint8_t button)
{
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1);
	tft.fillScreen(BLACK);

	menu = M_TG_TUNE;
	prev_menu = M_TG;
	clearCallbacks();

	setButtonParm(BUT1, PTRANS, POSA, true);
	setButtonParm(BUT2, PTUNE, POSB, true);
	setButtonParm(BUT5, PTRANS, POSA, true);
	setButtonParm(BUT6, PTUNE, POSB, true);
	buttons.setCallback(BUT8, &cMenu::menuBack);

	setPotCallback(TOPPOT, PTRANS, POSA);
	setPotCallback(MIDPOT, PTUNE, POSB);

	ShowValue(PTRANS, 61, parampos[BUT1], VALUEWIDTH, 16, pflag[TOPPOT], 1);
	ShowValue(PTUNE, 61, parampos[BUT2], VALUEWIDTH, 16, pflag[MIDPOT], 1);
	showTGInfo();
}

void cMenu::TGOut(uint8_t button)
{
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1);
	tft.fillScreen(BLACK);

	printf("TG Out\n");
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

	ShowValue(PPAN, 61, parampos[BUT1], VALUEWIDTH, 16, pflag[TOPPOT], 1);
	ShowValue(PVOL, 61, parampos[BUT2], VALUEWIDTH, 16, pflag[MIDPOT], 1);
	showTGInfo();
}

void cMenu::TGPitch(uint8_t button)
{
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1);
	tft.fillScreen(BLACK);

	printf("TG Pitch\n");
	menu = M_TG_PITCH;
	prev_menu = M_TG;
	clearCallbacks();
	setButtonParm(BUT1, PBEND, POSA, true);
	setButtonParm(BUT2, PPORTA, POSB, true);
	setButtonParm(BUT3, PMONO, POSC, true);
	setButtonParm(BUT5, PBEND, POSA, true);
	setButtonParm(BUT6, PPORTA, POSB, true);
	buttons.setCallback(BUT8, &cMenu::menuBack);

	setPotCallback(TOPPOT, PBEND, POSA);
	setPotCallback(MIDPOT, PPORTA, POSB);
	setPotCallback(BOTPOT, PMONO, POSC);

	ShowValue(PBEND, 61, parampos[BUT1], VALUEWIDTH, 16, pflag[TOPPOT], 1);
	ShowValue(PPORTA, 61, parampos[BUT2], VALUEWIDTH, 16, pflag[MIDPOT], 1);
	ShowValue(PMONO, 61, parampos[BUT3], VALUEWIDTH, 16, pflag[BOTPOT], 1);
	showTGInfo();
}