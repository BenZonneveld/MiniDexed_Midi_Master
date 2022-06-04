#include <stdlib.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <string.h>

#include "hagl.h"
#include "hagl_hal.h"
#include "MainMenu.h"
#include "string_table.h"
#include "mdma.h"
#include "Buttons.h"
#include "tools.h"

int8_t cMenu::currentTG;
uint8_t cMenu::menu;
uint8_t cMenu::prev_menu; 
bool cMenu::pflag[3];
uint16_t cMenu::potparam[4];
int8_t cMenu::potpos[3];
int16_t cMenu::bparam[8];
int8_t cMenu::parampos[8];
bool cMenu::menuNeedFlush;

void cMenu::Init()
{
	menuNeedFlush = false;
	hagl_init();
	hagl_flush();
	currentTG = -1;
	mainmenu();
}

void cMenu::mainmenu()
{
	hagl_clear_screen();
//	hagl_fill_rectangle(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1, BLACK);
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
	buttons.setDBLCallback(BUT1, &cMenu::menuBack);
	buttons.setDBLCallback(BUT2, &cMenu::menuBack);
	buttons.setDBLCallback(BUT3, &cMenu::menuBack);
	buttons.setDBLCallback(BUT4, &cMenu::menuBack);
	buttons.setDBLCallback(BUT5, &cMenu::menuBack);
	buttons.setDBLCallback(BUT6, &cMenu::menuBack);
	buttons.setDBLCallback(BUT7, &cMenu::menuBack);
	buttons.setDBLCallback(BUT8, &cMenu::menuBack);
	Pots.setPotCallback(TOPPOT, NULL);
	Pots.setPotCallback(MIDPOT, NULL);
	Pots.setPotCallback(BOTPOT, NULL);
}

void cMenu::ShowButtonText(uint8_t button)
{
	int16_t offset = 30;
	int16_t col = 0;
	int16_t row = 0;

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
		hagl_fill_rounded_rectangle(col, row, col + (2 * offset) - 4, row + 21, 4, hagl_color(128, 64, 64));
		hagl_fill_rectangle(col, row, col + (2 * offset) - 8, row + 21, hagl_color(128, 64, 64));
		hagl_print(menus[menu][button], col + 5, row + 1, WHITE, 1);
		printf("%s\n", menus[menu][button]);
		menuNeedFlush = true;
	}
	else {
		hagl_fill_rounded_rectangle(col + 4, row, col + (2 * offset)-8, row + 21, 4, hagl_color(128, 64, 64));
		hagl_fill_rectangle(col + 8, row, col + (2 * offset), row + 21, hagl_color(128, 64, 64));
		hagl_print(menus[menu][button], col + 9, row + 1, WHITE, 1);
		printf("%s\n", menus[menu][button]);
		menuNeedFlush = true;
	}
}

void cMenu::ShowValue(int32_t param, int16_t x0, int16_t y0, int16_t w0, int16_t h0, bool colorflag, uint8_t fontsize)
{	
	int32_t value = dexed[currentTG].getValue(param);

	char text[7];
	uint8_t offset = 3;
	itoa(value, text, 10);
//	hagl_set_clip_window(x0, y0, x0 + w0, y0 + h0);
//	hagl_clear_clip_window();
	//if ( colorflag)
	//{
	//	hagl_fill_rectangle(x0, y0, x0+w0, y0+h0, WHITE);
	//}
	//else {
	//	hagl_fill_rectangle(x0, y0, x0+w0, y0+h0+1, hagl_color(64,64,255));
	//}

	//if (abs(value) > 9)
	//	offset = 8;
	//if (abs(value) > 99)
	//	offset = 13;
	//if (abs(value) > 999)
	//	offset = 16;
	//if (value < 0)
	//	offset = offset + 6;
	//hagl_print(text,(x0 + (w0/2)) - offset, y0+1, BLACK, fontsize);
	//hagl_set_clip_window(0, 0, MIPI_DISPLAY_WIDTH-1, MIPI_DISPLAY_HEIGHT-1);
	menuNeedFlush = true;
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
	hagl_fill_rectangle(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1, BLACK);
	menu = M_TG_MIDI;
	prev_menu = M_TG;

	setButtonParm(BUT1, PBANK, POS0, true);
	setButtonParm(BUT2, PPATCH, POS1, true);
	setButtonParm(BUT3, PCHANNEL, POS2, true);
	clearButtonCB(BUT4);
	setButtonParm(BUT5, PBANK, POS0, true);
	setButtonParm(BUT6, PPATCH, POS1, true);
	setButtonParm(BUT7, PCHANNEL, POS2, true);
	clearButtonCB(BUT8);

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
	hagl_fill_rectangle(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1, BLACK);
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

	buttons.setCallback(0, &cMenu::Midi);
	buttons.setCallback(1, &cMenu::TGMain);
	buttons.setCallback(2, &cMenu::TGTune);

	resetPotCB(TOPPOT);
	resetPotCB(MIDPOT);
	resetPotCB(BOTPOT);
}

void cMenu::TGMain(uint8_t button)
{
	hagl_fill_rectangle(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1, BLACK);
	menu = M_TG_MAIN;
	prev_menu = M_TG;

	setButtonParm(BUT1, PFREQ, POS0, true);
	setButtonParm(BUT2, PRESO, POS1, true);
	setButtonParm(BUT3, PVERB, POS2, true);
	clearButtonCB(BUT4);
	setButtonParm(BUT5, PFREQ, POS0, true);
	setButtonParm(BUT6, PRESO, POS1, true);
	setButtonParm(BUT7, PVERB, POS2, true);
	clearButtonCB(BUT8);

	setPotCallback(TOPPOT, PFREQ, POS0);
	setPotCallback(MIDPOT, PRESO, POS1);
	setPotCallback(BOTPOT, PVERB, POS2);

	ShowValue(PFREQ, 61, POS0, VALUEWIDTH, 21, pflag[TOPPOT], 1);
	ShowValue(PRESO, 61, POS1, VALUEWIDTH, 21, pflag[MIDPOT], 1);
	ShowValue(PVERB, 61, POS2, VALUEWIDTH, 21, pflag[BOTPOT], 1);
}

void cMenu::TGTune(uint8_t button)
{
	hagl_fill_rectangle(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1, BLACK);
	menu = M_TG_TUNE;
	prev_menu = M_TG;

	setButtonParm(BUT1, PTRANS, POS0, true);
	setButtonParm(BUT2, PTUNE, POS1, true);
	setButtonParm(BUT3, PPAN, POS2, true);
	setButtonParm(BUT4, PVOL, POS3, true);
	setButtonParm(BUT5, PTRANS, POS0, true);
	setButtonParm(BUT6, PTUNE, POS1, true);
	setButtonParm(BUT7, PPAN, POS2, true);
	setButtonParm(BUT8, PVOL, POS3, true);

	setPotCallback(TOPPOT, PTUNE, POS1);
	setPotCallback(MIDPOT, PPAN, POS2);
	setPotCallback(BOTPOT, PVOL, POS3);

	ShowValue(PTRANS, 61, parampos[BUT1], VALUEWIDTH, 21, true, 1);
	ShowValue(PTUNE, 61, parampos[BUT2], VALUEWIDTH, 21, pflag[TOPPOT], 1);
	ShowValue(PPAN, 61, parampos[BUT3], VALUEWIDTH, 21, pflag[MIDPOT], 1);
	ShowValue(PVOL, 61, parampos[BUT4], VALUEWIDTH, 21, pflag[BOTPOT], 1);
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
//	dexed[currentTG].sendMidi(bparam[button]);
	ShowValue(bparam[button], 61, parampos[button], VALUEWIDTH, 21, true, 1);
}

void cMenu::ParmPot(uint8_t channel)
{
	uint16_t param = potparam[channel];
	int32_t value = map(Pots.getPot(channel), POT_MIN, POT_MAX, ranges[param][0], 1+ranges[param][1]);

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

void cMenu::setButtonCallback(uint8_t button, uint16_t param, int8_t pos,void (*callback)(uint8_t button))
{
	buttons.setCallback(button, callback);
	if (callback == nullptr)
		bparam[button] = -1;
	bparam[button] = param;
	parampos[button] = pos;
	ShowButtonText(button);
}

void cMenu::setButtonParm(uint8_t button, uint16_t param, int8_t pos, bool haslongpress)
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

void cMenu::setPotCallback(uint8_t channel, uint16_t param, int8_t pos)
{
	Pots.setPotCallback(channel, &cMenu::ParmPot);
	potparam[channel] = param;
	potpos[channel] = pos;
	pflag[channel] = false;

	uint16_t pot = map(Pots.getPot(channel), POT_MIN, POT_MAX, ranges[param][0], ranges[param][1]);

	if (pot == dexed[currentTG].getValue(param))
		pflag[channel] = true;
}

void cMenu::resetPotCB(uint8_t channel)
{
	Pots.setPotCallback(channel, NULL);
}