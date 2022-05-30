#include <stdlib.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <string.h>


#include "MainMenu.h"
#include "string_table.h"
#include "mdma.h"
#include "Buttons.h"
#include "screen.h"

int8_t cMenu::selectedTG;
uint8_t cMenu::menu;
uint8_t cMenu::level;

void cMenu::Init()
{
	menu = level = 0;
	selectedTG = -1;
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
}

void cMenu::ShowMenu()
{
	for (uint8_t i = 0; i < 8; i++)
	{
		printf("%s\r\n", menus[level*menu + level][i]);
	}
}

void cMenu::menuLevelUp()
{
	if (level > 0)
	{
		level--;
	}
	if (level == 0 && menu > 0)
		menu--;
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
	printf("TG %i bank: %i\r\n", selectedTG+1, bank);
}

void cMenu::BankSelectPot()
{
	uint16_t bank = Pots.getPot(0);
	if (bank != dexed[selectedTG].getBank())
	{
		bank = dexed[selectedTG].setBank(bank);
		printf("TG %i bank: %i\r\n", selectedTG + 1, bank);
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
	printf("TG %i patch: %i\r\n", selectedTG + 1, patch);
}

void cMenu::PatchSelectPot()
{
	uint8_t patch = Pots.getPot(1) / 8;
	if (patch != dexed[selectedTG].getPatch())
	{
		patch = dexed[selectedTG].setPatch(patch);
		printf("TG %i patch: %i\r\n", selectedTG + 1, patch);
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
	printf("TG %i channel: %i\r\n", selectedTG + 1, channel);
}

void cMenu::ChannelSelectPot()
{
	uint8_t channel = Pots.getPot(2)/15;
	if (channel != dexed[selectedTG].getChannel())
	{
		channel = dexed[selectedTG].setChannel(channel);
		printf(" channel: %i\r\n", channel);
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
	Pots.setPotCallback(0, &cMenu::BankSelectPot);
	Pots.setPotCallback(1, &cMenu::PatchSelectPot);
	Pots.setPotCallback(2, &cMenu::ChannelSelectPot);

	menu++;
	ShowMenu();
	printf("Bank: %i, Patch %i, Channel %i", dexed[selectedTG].getBank(), 
										dexed[selectedTG].getPatch(),
										dexed[selectedTG].getChannel());
}

void cMenu::selectTG(uint8_t button )
{
	selectedTG = button;
	menu = 0;
	level = 1;
	ShowMenu();
	buttons.setCallback(0, &cMenu::Midi);
	buttons.setCallback(1, NULL);
	buttons.setCallback(2, NULL);
	buttons.setCallback(3, NULL);
	buttons.setCallback(4, NULL);
	buttons.setCallback(5, NULL);
	buttons.setCallback(6, NULL);
	buttons.setCallback(7, NULL);
//	buttons.setCallback(0, &cMenu::BankSelect);
//	buttons.setCallback(1, &cMenu::PatchSelect);
//	buttons.setCallback(2, &cMenu::ChannelSelect);
//	buttons.setCallback(4, &cMenu::BankSelect);
//	buttons.setCallback(5, &cMenu::PatchSelect);
//	buttons.setCallback(6, &cMenu::ChannelSelect);

	printf("Select TG %i function\r\n", button);
}