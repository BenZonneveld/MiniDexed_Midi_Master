#include "MainMenu.h"

void  (*menucallbacks[][8])() = {

	&cMenu::selectTG,   // M_MAIN
	&cMenu::selectTG,
	&cMenu::selectTG,
	&cMenu::selectTG,
	&cMenu::selectTG,
	&cMenu::selectTG,
	&cMenu::selectTG,
	&cMenu::selectTG },
	{					// M_TG
		&cMenu::Midi,
		&cMenu::TGMain,
		&cMenu::TGTune,
		NULL,
		NULL,
		NULL,
		NULL,
		&cMenu::menuBack
	}, {				 // M_TG_MAIN

	};



void  (*potcallbacks[][3])() = {
	{ NULL, NULL, NULL},   // M_MAIN
	{ NULL, NULL, NULL}   // M_TG
}