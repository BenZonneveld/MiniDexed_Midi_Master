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
bool cMenu::pflag[3];

void cMenu::Init()
{
	hagl_init();
	hagl_clear_screen();
	menu = M_MAIN;
	currentTG = -1;
	mainmenu();
}

void cMenu::mainmenu()
{
	buttons.setCallback(0, &cMenu::selectTG);
	buttons.setCallback(1, &cMenu::selectTG);
	buttons.setCallback(2, &cMenu::selectTG);
	buttons.setCallback(3, &cMenu::selectTG);
	buttons.setCallback(4, &cMenu::selectTG);
	buttons.setCallback(5, &cMenu::selectTG);
	buttons.setCallback(6, &cMenu::selectTG);
	buttons.setCallback(7, &cMenu::selectTG);
	buttons.setDBLCallback(0, &cMenu::menuLevelUp);
	buttons.setDBLCallback(1, &cMenu::menuLevelUp);
	buttons.setDBLCallback(2, &cMenu::menuLevelUp);
	buttons.setDBLCallback(3, &cMenu::menuLevelUp);
	buttons.setDBLCallback(4, &cMenu::menuLevelUp);
	buttons.setDBLCallback(5, &cMenu::menuLevelUp);
	buttons.setDBLCallback(6, &cMenu::menuLevelUp);
	buttons.setDBLCallback(7, &cMenu::menuLevelUp);
	Pots.setPotCallback(0, NULL);
	Pots.setPotCallback(1, NULL);
	Pots.setPotCallback(2, NULL);
	menu = M_MAIN;
	ShowMenu();
	printf("Main Menu callbacks set\r\n");
}

void cMenu::ShowMenu()
{
	int16_t offset = 30;
	int16_t col = 0;
	int16_t row = 0;
	int16_t y = 0;
	hagl_fill_rectangle(0,0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT,BLACK);
	
	for (uint8_t i = 0; i < 8; i++)
	{
//		printf("dexed[%i].channel: %i\r\n", i, dexed[i].getChannel());
		y = row * 34;
		if (i < 4)
		{
			hagl_fill_rounded_rectangle(col, y, col + (2 * offset)-4, y + 21, 4, hagl_color(128, 64, 64));
			hagl_fill_rectangle(col, y, col + (2 * offset) - 8, y + 21, hagl_color(128, 64, 64));
			hagl_print(menus[menu][i], col + 5, y + 1, WHITE, 1);
		}
		else {
			hagl_fill_rounded_rectangle(col+4, y, col + (2 * offset), y + 21, 4, hagl_color(128, 64, 64));
			hagl_fill_rectangle(col + 8, y, col + (2 * offset), y + 21, hagl_color(128, 64, 64));
			hagl_print(menus[menu][i], col + 9, y + 1, WHITE, 1);
		}
//		printf("%s\r\n", menus[menu][i]);
		row++;
		if (row > 3)
		{
			col = MIPI_DISPLAY_WIDTH-(2*offset);
			row = 0;
		}
	}
}

void cMenu::ShowValue(uint16_t value, int16_t x0, int16_t y0, int16_t w0, int16_t h0, bool colorflag, uint8_t fontsize)
{	
	printf("currentTG: %i\r\n", currentTG);
	char text[6];
	uint8_t offset = 3;
	itoa(value, text, 10);
	hagl_set_clip_window(x0, y0, x0 + w0, y0 + h0);
	hagl_clear_clip_window();
	if ( colorflag)
	{
		hagl_fill_rectangle(x0, y0, x0+w0, y0+h0, WHITE);
	}
	else {
		hagl_fill_rectangle(x0, y0, x0+w0, y0+h0+1, hagl_color(64,64,255));
	}
	if (value > 9)
		offset = 8;
	if (value > 99)
		offset = 13;
	if (value > 999)
		offset = 16;
	hagl_print(text,(x0 + (w0/2)) - offset, y0+1, BLACK, fontsize);
	hagl_set_clip_window(0, 0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
}

void cMenu::menuLevelUp()
{
	uint8_t button = currentTG;
	switch (menu)
	{
	case M_MAIN:
	case M_TG:
		menu = M_MAIN;
		printf("menu: M_MAIN\r\n");
		mainmenu();
		break;
	case M_TG_MAIN:
	case M_TG_MIDI:
//		menu = M_TG;
		printf("menu: M_TG\r\n");
		printf("currentTG: %i\r\n", currentTG);
		selectTG(button);
		break;
	default:
		printf("default\r\n");
		break;
	}
//	ShowMenu();
}

void cMenu::BankSelect(uint8_t button)
{
	int8_t bank = -1;
	switch (button)
	{
	case BUT1:
		bank = dexed[currentTG].BankDown();
		break;
	case BUT5:
		bank = dexed[currentTG].BankUp();
		break;
	default:
		break;
	}

	ShowValue(bank + 1, 61, 0, VALUEWIDTH, 21, true, 1);
}

void cMenu::BankSelectPot()
{
	uint16_t bank = map(Pots.getPot(BANKPOT),POT_MIN, POT_MAX,0,MAXBANKS);
	if (bank == dexed[currentTG].getBank() && !pflag[BANKPOT])
	{
		pflag[BANKPOT] = true;
		ShowValue(bank + 1, 61, 0, VALUEWIDTH, 21, pflag[BANKPOT], 1);
	}
	if (bank != dexed[currentTG].getBank() && pflag[BANKPOT])
	{
		bank = dexed[currentTG].setBank(bank);
		ShowValue(bank + 1, 61, 0, VALUEWIDTH, 21, pflag[BANKPOT], 1);
	}
}

void cMenu::PatchSelect(uint8_t button)
{
	int8_t patch = -1;
	switch (button)
	{
	case BUT2:
		patch = dexed[currentTG].PatchDown();
		break;
	case BUT6:
		patch = dexed[currentTG].PatchUp();
		break;
	default:
		break;
	}
	ShowValue(patch + 1, 61, 34, VALUEWIDTH, 21, true, 1);
}

void cMenu::PatchSelectPot()
{
	uint8_t patch = map(Pots.getPot(PATCHPOT),POT_MIN, POT_MAX,0,MAXPATCH + 1);
	if (patch > 31) patch = 31;
	if (patch == dexed[currentTG].getPatch() && !pflag[PATCHPOT])
	{
		pflag[PATCHPOT] = true;
		ShowValue(patch + 1, 61, 34, VALUEWIDTH, 21, pflag[PATCHPOT], 1);
	}
	if (patch != dexed[currentTG].getPatch() && pflag[PATCHPOT])
	{
		patch = dexed[currentTG].setPatch(patch);
		ShowValue(patch + 1, 61, 34, VALUEWIDTH, 21, pflag[PATCHPOT], 1);
	}
}

void cMenu::ChannelSelect(uint8_t button)
{
	int8_t channel = -1;
	switch (button)
	{
	case BUT3:
		channel = dexed[currentTG].ChannelDown();
		break;
	case BUT7:
		channel = dexed[currentTG].ChannelUp();
		break;
	default:
		break;
	}
	ShowValue(channel + 1, 61, 68, VALUEWIDTH, 21, true,1);
}

void cMenu::ChannelSelectPot()
{
	uint8_t channel = map(Pots.getPot(CHANPOT), POT_MIN, POT_MAX, 0, MAXCHANNEL);
	if (channel == dexed[currentTG].getChannel() && !pflag[CHANPOT])
	{
		pflag[CHANPOT] = true;
		ShowValue(channel + 1, 61, 68, VALUEWIDTH, 21, pflag[CHANPOT], 1);
	}
	if (channel != dexed[currentTG].getChannel() && pflag[CHANPOT])
	{
		channel = dexed[currentTG].setChannel(channel);
		ShowValue(channel + 1, 61, 68, VALUEWIDTH, 21, pflag[CHANPOT], 1);
	}
}

void cMenu::Midi(uint8_t button)
{
	printf("Midi currentTG: %i\t", currentTG);
	buttons.setCallback(0, &cMenu::BankSelect);
	buttons.setCallback(1, &cMenu::PatchSelect);
	buttons.setCallback(2, &cMenu::ChannelSelect);
	buttons.setCallback(3, NULL);
	buttons.setCallback(4, &cMenu::BankSelect);
	buttons.setCallback(5, &cMenu::PatchSelect);
	buttons.setCallback(6, &cMenu::ChannelSelect);
	buttons.setCallback(7, NULL);

	buttons.setLongCallback(0, &cMenu::BankSelect);
	buttons.setLongCallback(1, &cMenu::PatchSelect);
	buttons.setLongCallback(2, &cMenu::ChannelSelect);
	buttons.setLongCallback(4, &cMenu::BankSelect);
	buttons.setLongCallback(5, &cMenu::PatchSelect);
	buttons.setLongCallback(6, &cMenu::ChannelSelect);

	Pots.setPotCallback(BANKPOT, &cMenu::BankSelectPot);
	Pots.setPotCallback(PATCHPOT, &cMenu::PatchSelectPot);
	Pots.setPotCallback(CHANPOT, &cMenu::ChannelSelectPot);

	menu = M_TG_MIDI;
	ShowMenu();

	pflag[BANKPOT] = false;
	pflag[CHANPOT] = false;
	pflag[PATCHPOT] = false;
	uint8_t channel = Pots.getPot(CHANPOT) / 15;
	uint8_t patch = Pots.getPot(PATCHPOT) / 8;
	uint16_t bank = Pots.getPot(BANKPOT);

	if (channel == dexed[currentTG].getChannel())
		pflag[CHANPOT] = true;
	if (patch == dexed[currentTG].getPatch())
		pflag[PATCHPOT] = true;
	if (bank == dexed[currentTG].getBank())
		pflag[BANKPOT] = true;
	ShowValue(dexed[currentTG].getBank() +1, 61, 0, VALUEWIDTH, 21, pflag[BANKPOT],1);
	ShowValue(dexed[currentTG].getPatch() +1, 61, 34, VALUEWIDTH, 21, pflag[PATCHPOT],1);
	ShowValue(dexed[currentTG].getChannel() + 1, 61, 68, VALUEWIDTH, 21, pflag[CHANPOT],1);

	printf("Bank: %i, Patch %i, Channel %i\r\n", dexed[currentTG].getBank(),
										dexed[currentTG].getPatch(),
										dexed[currentTG].getChannel());
}

void cMenu::selectTG(uint8_t button )
{
	currentTG = button;
	menu = M_TG;
	
	buttons.setCallback(0, &cMenu::Midi);
	buttons.setCallback(1, &cMenu::TGMain);
	buttons.setCallback(2, NULL);
	buttons.setCallback(3, NULL);
	buttons.setCallback(4, NULL);
	buttons.setCallback(5, NULL);
	buttons.setCallback(6, NULL);
	buttons.setCallback(7, NULL);
//	buttons.setDBLCallback(0, &cMenu::menuLevelUp);
	//buttons.setDBLCallback(1, &cMenu::menuLevelUp);
	//buttons.setDBLCallback(2, &cMenu::menuLevelUp);
	//buttons.setDBLCallback(3, &cMenu::menuLevelUp);
	//buttons.setDBLCallback(4, &cMenu::menuLevelUp);
	//buttons.setDBLCallback(5, &cMenu::menuLevelUp);
	//buttons.setDBLCallback(6, &cMenu::menuLevelUp);
	//buttons.setDBLCallback(7, &cMenu::menuLevelUp);
	Pots.setPotCallback(0, NULL);
	Pots.setPotCallback(1, NULL);
	Pots.setPotCallback(2, NULL);
	buttons.setLongCallback(0, NULL);
	buttons.setLongCallback(1, NULL);
	buttons.setLongCallback(2, NULL);
	buttons.setLongCallback(3, NULL);
	buttons.setLongCallback(4, NULL);
	buttons.setLongCallback(5, NULL);
	buttons.setLongCallback(6, NULL);
	buttons.setLongCallback(7, NULL);
	ShowMenu();
	printf("Select TG %i callbacks\r\n", button);
}

void cMenu::TGMain(uint8_t button)
{
	printf("TGMain callbacks set\r\n");
	menu = M_TG_MAIN;
	buttons.setCallback(0, NULL);
	buttons.setCallback(1, NULL);
	buttons.setCallback(2, NULL);
	buttons.setCallback(3, NULL);
	buttons.setCallback(4, NULL);
	buttons.setCallback(5, NULL);
	buttons.setCallback(6, NULL);
	buttons.setCallback(7, NULL);
	Pots.setPotCallback(0, NULL);
	Pots.setPotCallback(1, NULL);
	Pots.setPotCallback(2, NULL);
	ShowMenu();
}