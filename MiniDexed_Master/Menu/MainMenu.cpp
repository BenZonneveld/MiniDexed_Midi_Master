#include <stdlib.h>
#include <stdio.h>
#include <pico/stdlib.h>

#include "MainMenu.h"
#include "string_table.h"
#include "mdma.h"
#include "tools.h"

#define PROGMEM
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans24pt7b.h"

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
	tft.setFont(&FreeSans9pt7b);
//	tft.println("INIT");

	currentTG = -1;
//	mainmenu();
}

void cMenu::mainmenu()
{
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1);
	menu = M_MAIN;
	prev_menu = M_MAIN;

	setButtonCallback(BUT1, 0, POS0, &cMenu::selectTG);
	setButtonCallback(BUT2, 0, POS0, &cMenu::selectTG);
	setButtonCallback(BUT3, 0, POS0, &cMenu::selectTG);
	setButtonCallback(BUT4, 0, POS0, &cMenu::selectTG);
	setButtonCallback(BUT5, 0, POS0, &cMenu::selectTG);
	setButtonCallback(BUT6, 0, POS0, &cMenu::selectTG);
	setButtonCallback(BUT7, 0, POS0, &cMenu::selectTG);
	setButtonCallback(BUT8, 0, POS0, &cMenu::selectTG);
//	buttons.setDBLCallback(BUT1, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT2, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT3, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT4, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT5, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT6, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT7, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT8, &cMenu::menuBack);
	Pots.setPotCallback(TOPPOT, NULL);
	Pots.setPotCallback(MIDPOT, NULL);
	Pots.setPotCallback(BOTPOT, NULL);
}

void cMenu::ShowButtonText(uint8_t button)
{
	int16_t offset = 30;
	int16_t col = 0;
	int16_t row = 0;
	printf("SHowButtonText %i\n", button);
	switch (button)
	{
	case BUT5:
		col = MIPI_DISPLAY_WIDTH - (2 * offset);
	case BUT1:
		row = POS0;
		break;
	case BUT6:
		col = MIPI_DISPLAY_WIDTH - (2 * offset);
	case BUT2:
		row = POS1;
		break;
	case BUT7:
		col = MIPI_DISPLAY_WIDTH - (2 * offset);
	case BUT3:
		row = POS2;
		break;
	case BUT8:
		col = MIPI_DISPLAY_WIDTH - (2 * offset) -1;
	case BUT4:
		row = POS3;
		break;
	}
	if (button < 4)
	{
//		tft.fillRoundRect(col, row, (2 * offset) - 4, 21, 4, tft.color565(128, 64, 64));
		tft.writeFillRect(col, row, (2 * offset) - 8, 21, tft.color565(128, 64, 64));
		tft.setCursor(col + 5, row+14);
		tft.print(menus[menu][button]);
		printf("%s\n", menus[menu][button]);
	}
	else {
//		tft.drawRoundRect(col + 4, row, (2 * offset) - 8, 21, 4, tft.color565(128, 64, 64));
		tft.fillRect(col + 4, row, (2 * offset) - 4, 21, tft.color565(128, 64, 64));
		tft.setCursor(col + 9, row+14);
		tft.print(menus[menu][button]);
		printf("%s\n", menus[menu][button]);
	}
}

void cMenu::ShowValue(int32_t param, int16_t x0, int16_t y0, int16_t w0, int16_t h0, bool colorflag, uint8_t fontsize)
{	
	int32_t value = dexed[currentTG].getValue(param);

	char text[7];
	uint8_t offset = 3;
	itoa(value + ranges[param][ROFFSET], text, 10);
	tft.setAddrWindow(x0, y0, w0, h0);
	if ( colorflag)
	{
		tft.fillRect(x0,y0,w0,h0,WHITE);
	}
	else {
		tft.fillRect(x0, y0, w0, h0, tft.color565(64, 64, 255));
	}

	if (abs(value) > 9)
		offset = 8;
	if (abs(value) > 99)
		offset = 13;
	if (abs(value) > 999)
		offset = 16;
	if (value < 0)
		offset = offset + 6;
	if (param == PCHANNEL && value == 16)
	{
		sprintf(text, "OMNI");
		offset = 16;
	}

	tft.setTextColor(BLACK);
	tft.setCursor((x0 + (w0 / 2)) - offset, y0 + 14);
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
		tft.writeFillRect(61, 0, VALUEWIDTH, 128, BLACK);
		selectTG();
		break;
	case M_TG_MAIN:
	case M_TG_MIDI:
	case M_TG_TUNE:
	default:
		printf("default\r\n");
		break;
	}
}

void cMenu::Midi(uint8_t button)
{
	printf("MIDI Menu\n");
//	tft.fillScreen(BLACK);
	menu = M_TG_MIDI;
	prev_menu = M_TG;

	setButtonParm(BUT1, PBANK, POS0, true);
	setButtonParm(BUT2, PPATCH, POS1, true);
	setButtonParm(BUT3, PCHANNEL, POS2, true);
	clearButtonCB(BUT4);
	setButtonParm(BUT5, PBANK, POS0, true);
	setButtonParm(BUT6, PPATCH, POS1, true);
	setButtonParm(BUT7, PCHANNEL, POS2, true);
	//setButtonParm(BUT8,32768,POS3, false);
	clearButtonCB(BUT8);
	buttons.setCallback(BUT8, &cMenu::menuBack);

	setPotCallback(TOPPOT, PBANK, POS0);
	setPotCallback(MIDPOT, PPATCH, POS1);
	setPotCallback(BOTPOT, PCHANNEL, POS2);

	ShowValue(PBANK, 61, POS0, VALUEWIDTH, 21, pflag[TOPPOT],1);
	ShowValue(PPATCH, 61, POS1, VALUEWIDTH, 21, pflag[MIDPOT],1);
	ShowValue(PCHANNEL, 61, POS2, VALUEWIDTH, 21, pflag[BOTPOT],1);
}

void cMenu::selectTG(uint8_t button)
{
	currentTG = button;
	selectTG();
}

void cMenu::selectTG()
{
	printf("Select TG\n");
//	tft.fillScreen(BLACK);
	menu = M_TG;
	prev_menu = M_MAIN;

	clearButtonCB(BUT1);
	clearButtonCB(BUT2);
	clearButtonCB(BUT3);
	clearButtonCB(BUT4);
	clearButtonCB(BUT5);
	clearButtonCB(BUT6);
	clearButtonCB(BUT7);
	clearButtonCB(BUT8);

	buttons.setCallback(BUT1, &cMenu::Midi);
	buttons.setCallback(BUT2, &cMenu::TGMain);
	buttons.setCallback(BUT3, &cMenu::TGTune);
	buttons.setCallback(BUT8, &cMenu::menuBack);
//	setButtonParm(BUT8, -1, POS3, false);

	resetPotCB(TOPPOT);
	resetPotCB(MIDPOT);
	resetPotCB(BOTPOT);
	printf("End of selectTG\n");
}

void cMenu::TGMain(uint8_t button)
{
	printf("TG Main\n");
//	tft.fillScreen(BLACK);
	menu = M_TG_MAIN;
	prev_menu = M_TG;

	setButtonParm(BUT1, PFREQ, POS0, true);
	setButtonParm(BUT2, PRESO, POS1, true);
	setButtonParm(BUT3, PVERB, POS2, true);
	clearButtonCB(BUT4);
	setButtonParm(BUT5, PFREQ, POS0, true);
	setButtonParm(BUT6, PRESO, POS1, true);
	setButtonParm(BUT7, PVERB, POS2, true);
//	setButtonParm(BUT8, 32768, POS3, false);
	clearButtonCB(BUT8);
	buttons.setCallback(BUT8, &cMenu::menuBack);

	setPotCallback(TOPPOT, PFREQ, POS0);
	setPotCallback(MIDPOT, PRESO, POS1);
	setPotCallback(BOTPOT, PVERB, POS2);

	ShowValue(PFREQ, 61, POS0, VALUEWIDTH, 21, pflag[TOPPOT], 1);
	ShowValue(PRESO, 61, POS1, VALUEWIDTH, 21, pflag[MIDPOT], 1);
	ShowValue(PVERB, 61, POS2, VALUEWIDTH, 21, pflag[BOTPOT], 1);
}

void cMenu::TGTune(uint8_t button)
{
//	tft.fillScreen(BLACK);
	menu = M_TG_TUNE;
	prev_menu = M_TG;

	setButtonParm(BUT1, PTRANS, POS0, true);
	setButtonParm(BUT2, PTUNE, POS1, true);
	clearButtonCB(BUT3);
	clearButtonCB(BUT4);
//	setButtonParm(BUT3, PPAN, POS2, true);
//	setButtonParm(BUT4, PVOL, POS3, true);
	setButtonParm(BUT5, PTRANS, POS0, true);
	setButtonParm(BUT6, PTUNE, POS1, true);
	clearButtonCB(BUT7);
	clearButtonCB(BUT8);
	buttons.setCallback(BUT8, &cMenu::menuBack);

//	setButtonParm(BUT7, PPAN, POS2, true);
//	setButtonParm(BUT8, PVOL, POS3, true);
//	buttons.setDBLCallback(BUT1, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT2, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT3, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT4, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT5, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT6, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT7, &cMenu::menuBack);
//	buttons.setDBLCallback(BUT8, &cMenu::menuBack);
	setPotCallback(TOPPOT, PTRANS, POS1);
	setPotCallback(MIDPOT, PTUNE, POS2);
	resetPotCB(BOTPOT);
	//	setPotCallback(BOTPOT, PVOL, POS3);

	ShowValue(PTRANS, 61, parampos[BUT1], VALUEWIDTH, 21, pflag[TOPPOT], 1);
	ShowValue(PTUNE, 61, parampos[BUT2], VALUEWIDTH, 21, pflag[MIDPOT], 1);
//	ShowValue(PPAN, 61, parampos[BUT3], VALUEWIDTH, 21, pflag[MIDPOT], 1);
//	ShowValue(PVOL, 61, parampos[BUT4], VALUEWIDTH, 21, pflag[BOTPOT], 1);
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
	}
}

void cMenu::setButtonCallback(uint8_t button, int16_t param, int16_t pos,void (*callback)(uint8_t button))
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
	setButtonCallback(button, param, pos, &cMenu::ParmSelect);
	if ( haslongpress )
		buttons.setLongCallback(button, &cMenu::ParmSelect);
	else
		buttons.setLongCallback(0, NULL);
}

void cMenu::clearButtonCB(uint8_t button)
{
	buttons.setCallback(button, NULL);
	buttons.setLongCallback(button, NULL);
	ShowButtonText(button);
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