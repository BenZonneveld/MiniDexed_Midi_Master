#include <stdlib.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <string.h>

#include "MainMenu.h"
#include "string_table.h"
#include "mdma.h"
#include "Buttons.h"

#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans24pt7b.h"

int8_t cMenu::selectedTG;
uint8_t cMenu::menu;

void cMenu::Init()
{
	menu = M_MAIN;
	selectedTG = -1;
	mainmenu();
	printf("cMenu::Init done\r\n");
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
	printf("Main Menu callbacks set\r\n");
}

void cMenu::ShowMenu()
{
	printf("ShowMenu()\r\n");
	int16_t offset = 30;
	int16_t col = offset;
	int16_t row = 0;
	int16_t x = 0;
	int16_t y = 0;
	uint16_t w = 0;
	uint16_t h = 0;
	uint16_t last_h = 0;

//	tft.setAddrWindow(0, 0, 160, 128);
	tft.fillRect(0,0,159,127 ,BLACK);
	tft.setFont(&FreeSans9pt7b);
	tft.setTextColor(WHITE);
	tft.setTextSize(0);

	printf("Printing Text\r\n");
	for (uint8_t i = 0; i < 8; i++)
	{
		tft.getTextBounds(menus[menu][i], col, row, &x, &y, &w, &h);
		if (h > last_h)
			last_h = h;
		tft.fillRoundRect(col-offset, row * ((128 + last_h) / 4), (2*offset) , 21, 4,GREY);
		tft.setTextColor(WHITE);
		tft.setCursor(col-(w/2), last_h + (row * ((128 + last_h) / 4))+3);
		tft.print(menus[menu][i]);
		printf("%s\r\n", menus[menu][i]);
		row++;
		if (row > 3)
		{
			col = 158-offset;
			row = 0;
		}
	}
	tft.flush();
}

void cMenu::ShowValue(uint16_t value, int16_t x0, int16_t y0, int16_t w0, int16_t h0)
{
	int16_t x = 0;
	int16_t y = 0;
	uint16_t w = 0;
	uint16_t h = 0;
	
	char text[6];
	itoa(value, text, 10);
//	tft.setAddrWindow(x0, y0, w0, h0);
	tft.fillRect(x0, y0, w0, h0, WHITE);
	tft.setTextColor(BLACK);
	tft.getTextBounds(text, 80, y0+2, &x, &y, &w, &h);
	tft.setCursor((x0 + (w0/2)) - (w / 2) -3, y0 + h + 4);
	tft.print(text);
	tft.flush();
//	tft.setAddrWindow(0, 0, 160, 128);
}

void cMenu::menuLevelUp()
{
	printf("menuLevelUp\r\n");
	if ( menu > M_MAIN)
		menu--;
	switch (menu)
	{
	case M_MAIN:
		mainmenu();
		break;
	case M_TG:
		selectTG(selectedTG);
		break;
	default:
		break;
	}
	ShowMenu();
}

void cMenu::BankSelect(uint8_t button)
{
	int8_t bank = -1;
	switch (button)
	{
	case BUT1:
		bank = dexed[selectedTG].BankDown();
		break;
	case BUT5:
		bank = dexed[selectedTG].BankUp();
		break;
	default:
		break;
	}
	ShowValue(bank + 1, 61, 0, 38, 21);
//	printf("TG %i bank: %i\r\n", selectedTG+1, bank);
}

void cMenu::BankSelectPot()
{
	printf("Bank Select Pot\r\n");
	uint16_t bank = Pots.getPot(BANKPOT);
	if (bank != dexed[selectedTG].getBank())
	{
		bank = dexed[selectedTG].setBank(bank);
		ShowValue(bank + 1, 61, 0, 38, 21);
	}
}

void cMenu::PatchSelect(uint8_t button)
{
	int8_t patch = -1;
	switch (button)
	{
	case BUT2:
		patch = dexed[selectedTG].PatchDown();
		break;
	case BUT6:
		patch = dexed[selectedTG].PatchUp();
		break;
	default:
		break;
	}
	ShowValue(patch + 1, 61, ((128 + 13) / 4), 38, 21);
//	printf("TG %i patch: %i\r\n", selectedTG + 1, patch);
}

void cMenu::PatchSelectPot()
{
	uint8_t patch = Pots.getPot(PATCHPOT) / 8;
	if (patch != dexed[selectedTG].getPatch())
	{
		patch = dexed[selectedTG].setPatch(patch);
		ShowValue(patch + 1, 61, ((128 + 13) / 4), 38, 21);
//		printf("TG %i patch: %i\r\n", selectedTG + 1, patch);
	}
}

void cMenu::ChannelSelect(uint8_t button)
{
	int8_t channel = -1;
	switch (button)
	{
	case BUT3:
		channel = dexed[selectedTG].ChannelDown();
		break;
	case BUT7:
		channel = dexed[selectedTG].ChannelUp();
		break;
	default:
		break;
	}
	ShowValue(channel + 1, 61, 2 * ((128 + 13) / 4), 38, 21);
//	printf("TG %i channel: %i\r\n", selectedTG + 1, channel);
}

void cMenu::ChannelSelectPot()
{
	uint8_t channel = Pots.getPot(CHANPOT)/15;
	if (channel != dexed[selectedTG].getChannel())
	{
		channel = dexed[selectedTG].setChannel(channel);
		ShowValue(channel + 1, 61, 2* ((128 + 13) / 4), 38, 21);
//		printf(" channel: %i\r\n", channel);
	}
}

void cMenu::Midi(uint8_t button)
{
	buttons.setCallback(0, &cMenu::BankSelect);
	buttons.setCallback(1, &cMenu::PatchSelect);
	buttons.setCallback(2, &cMenu::ChannelSelect);
	buttons.setCallback(3, NULL);
	buttons.setCallback(4, &cMenu::BankSelect);
	buttons.setCallback(5, &cMenu::PatchSelect);
	buttons.setCallback(6, &cMenu::ChannelSelect);
	buttons.setCallback(7, NULL);
	Pots.setPotCallback(BANKPOT, &cMenu::BankSelectPot);
	Pots.setPotCallback(PATCHPOT, &cMenu::PatchSelectPot);
	Pots.setPotCallback(CHANPOT, &cMenu::ChannelSelectPot);

	menu = M_TG_MIDI;
	ShowMenu();
	ShowValue(dexed[selectedTG].getBank() +1, 61, 0, 38, 21);
	ShowValue(dexed[selectedTG].getPatch() +1, 61, ((128 + 13) / 4), 38, 21);
	ShowValue(dexed[selectedTG].getChannel() + 1, 61, 2*((128 + 13) / 4), 38, 21);

	printf("Bank: %i, Patch %i, Channel %i", dexed[selectedTG].getBank(), 
										dexed[selectedTG].getPatch(),
										dexed[selectedTG].getChannel());
}

void cMenu::selectTG(uint8_t button )
{
	selectedTG = button;
	menu = M_TG;

	buttons.setCallback(0, &cMenu::Midi);
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
//	printf("Select TG %i function\r\n", button);
}