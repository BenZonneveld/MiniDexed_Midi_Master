#include <stdlib.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include "MainMenu.h"
#include "mdma.h"
#include "tools.h"

#define PROGMEM
//#include "fonts/FreeSans9pt7b.h"
//#include "fonts/FreeSans12pt7b.h"
//#include "fonts/FreeSans18pt7b.h"
//#include "fonts/FreeSans24pt7b.h"
#include "fonts/RobotoMono12.h"
#include "fonts/RobotoCondensed.h"
#include "fonts/OpenSansBoldCondensed16.h"

cPots Pots = cPots();
cButtons buttons = cButtons();

s_menu menuEntry[] = {
	{ 
		M_MAIN,
		M_MAIN,
		{PNOPARAM, PNOPARAM, PNOPARAM}, { false, false, false},
		{PNOPARAM, PNOPARAM, PNOPARAM, PNOPARAM},
		{
			{"TG1", M_TG, &cMenu::OpenMenu, &cMenu::TGEnable, &cMenu::Dexed },
			{"TG2", M_TG, &cMenu::OpenMenu, &cMenu::TGEnable, NULL },
			{"TG3", M_TG, &cMenu::OpenMenu, &cMenu::TGEnable, NULL },
			{"TG4", M_TG, &cMenu::OpenMenu, &cMenu::TGEnable, NULL },
			{"TG5", M_TG, &cMenu::OpenMenu, &cMenu::TGEnable, NULL },
			{"TG6", M_TG, &cMenu::OpenMenu, &cMenu::TGEnable, NULL },
			{"TG7", M_TG, &cMenu::OpenMenu, &cMenu::TGEnable, NULL },
			{"TG8", M_TG, &cMenu::OpenMenu, &cMenu::TGEnable, NULL },
		},
	},
	{ 
		M_TG,
		M_MAIN,
		{PFREQ, PRESO, PVERB}, { true, true, true},
		{PFREQ, PRESO, PVERB, PNOPARAM},
		{
			{ "MIDI", M_TG_MIDI, &cMenu::OpenMenu, &cMenu::OpenMenu, NULL},
			{ "Filter", M_TG_FILT, &cMenu::OpenMenu, &cMenu::OpenMenu, NULL},
			{ "Tune", M_TG_TUNE, &cMenu::OpenMenu, &cMenu::OpenMenu, NULL},
			{ "Out", M_TG_OUT, &cMenu::OpenMenu, &cMenu::OpenMenu, NULL},
			{ "Pitch", M_TG_PITCH, &cMenu::OpenMenu, &cMenu::OpenMenu, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Back", PNOPARAM, &cMenu::menuBack, &cMenu::menuBack, NULL},
		},
	},
	{ 
		M_TG_MIDI,
		M_TG,
		{PBANK, PPATCH, PCHANNEL}, { false, false, false},
		{PBANK, PPATCH, PCHANNEL, PNOPARAM},
		{
			{ "Bank -", PBANK, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Patch -", PPATCH, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Chan -", PCHANNEL, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Bank +", PBANK, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Patch +", PPATCH, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Chan +", PCHANNEL, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Back", PNOPARAM, &cMenu::menuBack, &cMenu::menuBack, NULL},
		},
	},
	{ 
		M_TG_FILT,
		M_TG,
		{PFREQ, PRESO, PVERB}, { false, false, false},
		{PFREQ, PRESO, PVERB, PNOPARAM},
		{
			{ "Cutoff-", PFREQ, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Reso -", PRESO, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Rvrb -", PVERB, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Cutoff+", PFREQ, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Reso +", PRESO, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Rvrb +", PVERB, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Back", PNOPARAM, &cMenu::menuBack, &cMenu::menuBack, NULL},
		},
	},
	{ 
		M_TG_TUNE,
		M_TG,
		{PSHIFT, PTUNE, PNOPARAM}, { false, false, false},
		{PSHIFT, PTUNE, PNOPARAM, PNOPARAM},
		{
			{ "Note -", PSHIFT, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Tune -", PTUNE, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Note +", PSHIFT, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Tune +", PTUNE, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Back", PNOPARAM, &cMenu::menuBack, &cMenu::menuBack, NULL},
		},
	},
	{ 
		M_TG_OUT,
		M_TG,
		{PPAN, PVOL, PNOPARAM}, { false, false, false},
		{PPAN, PVOL, PNOPARAM, PNOPARAM},
		{
			{ "Pan -", PPAN, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Vol -", PVOL, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Pan +", PPAN, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Vol +", PVOL, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Back", PNOPARAM, &cMenu::menuBack, &cMenu::menuBack, NULL},
		},
	},
	{ 
		M_TG_PITCH,
		M_TG,
		{PBRANGE, PBSTEP, PNOPARAM}, { false, false, false},
		{PBRANGE, PBSTEP, PNOPARAM, PMONO},
		{
			{ "PBRn -", PBRANGE, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "PBSt-", PBSTEP, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Porta", M_TG_PORTA, &cMenu::OpenMenu, &cMenu::OpenMenu, NULL},
			{ "Mono", PMONO, &cMenu::ParmToggle, &cMenu::ParmToggle, NULL},
			{ "PBRn+", PBRANGE, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "PBSt+", PBSTEP, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Back", PNOPARAM, &cMenu::menuBack, &cMenu::menuBack, NULL},
		},
	},
	{ 
		M_TG_PORTA,
		M_TG_PITCH,
		{PTIME, PNOPARAM, PNOPARAM}, { false, false, false},
		{PTIME, PPMODE, PGLISS, PNOPARAM},
		{
			{ "Time -", PTIME, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Mode", PPMODE, &cMenu::ParmToggle, NULL},
			{ "Gliss", PGLISS, &cMenu::ParmToggle, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Time +", PTIME, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Back", PNOPARAM, &cMenu::menuBack, &cMenu::menuBack, NULL},
		},
	},
	{
		M_DEXED,
		M_MAIN,
		{PNOPARAM, PNOPARAM, PNOPARAM}, { false, false, false},
		{PNOPARAM, PNOPARAM, FCOMP_EN, PNOPARAM},
		{
			{ "Rev 1", M_FX1, &cMenu::OpenMenu, &cMenu::OpenMenu, NULL},
			{ "Rev 2", M_FX2, &cMenu::OpenMenu, &cMenu::OpenMenu, NULL},
			{ "Comp", FCOMP_EN, &cMenu::ParmToggle, &cMenu::ParmToggle, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Card", M_CARD, &cMenu::mCard::CardMenu, &cMenu::mCard::CardMenu, NULL},
			{ "Router", PNOPARAM, &cMenu::OpenMenu, &cMenu::OpenMenu, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Back", PNOPARAM, &cMenu::menuBack, &cMenu::menuBack, NULL},
		},
	},
	{ 
		M_FX1,
		M_DEXED,
		{FSIZE, FDIFF, FRLEVEL}, { false, false, false},
		{FSIZE, FDIFF, FRLEVEL, FREV_EN},
		{
			{ "Size -", FSIZE, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Diff -", FDIFF,&cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Level -", FRLEVEL, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Active", FREV_EN, &cMenu::ParmToggle, &cMenu::ParmToggle, NULL},
			{ "Size +", FSIZE, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Diff +", FDIFF, &cMenu::ParmSelect,&cMenu::ParmSelect,  &cMenu::ParmSelect},
			{ "Level +", FRLEVEL, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Back", PNOPARAM, &cMenu::menuBack, &cMenu::menuBack, NULL},
		},
	},
	{ 
		M_FX2,
		M_DEXED,
		{ FHIGHDAMP, FLOWDAMP, FLOWPASS }, { false, false, false },
		{ FHIGHDAMP, FLOWDAMP, FLOWPASS, PNOPARAM },
		{
			{ "HDamp -", FHIGHDAMP, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "LDamp -", FLOWDAMP, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "LPass -", FLOWPASS, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "HDamp +", FHIGHDAMP, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "LDamp +", FLOWDAMP, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "LPass +", FLOWPASS, &cMenu::ParmSelect, &cMenu::ParmSelect, &cMenu::ParmSelect},
			{ "Back", PNOPARAM, &cMenu::menuBack, &cMenu::menuBack, NULL},
		},
	},
	{   
		M_ROUTING,
		M_DEXED,
		{ PNOPARAM, PNOPARAM, PNOPARAM }, { false, false, false },
		{ PNOPARAM, PNOPARAM, PNOPARAM, PNOPARAM },
		{
			{ "ROUTE", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Back", PNOPARAM, &cMenu::menuBack, &cMenu::menuBack, NULL},
		},
	},
	{ 
		M_CARD,
		M_DEXED,
		{ PNOPARAM, PNOPARAM, PNOPARAM }, { false, false, false },
		{ PNOPARAM, PNOPARAM, PNOPARAM, PNOPARAM },
		{
			{ "CARD", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ " ", PNOPARAM, NULL, NULL, NULL},
			{ "Back", PNOPARAM, &cMenu::menuBack, &cMenu::menuBack, NULL},
		},
	},
	{
		M_BANKVOICE,
		M_CARD,
		{ PNOPARAM, PNOPARAM, PNOPARAM }, { false, false, false },
		{ PNOPARAM, PNOPARAM, PNOPARAM, PNOPARAM },
		{
			{ "TG1", PNOPARAM, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ShowEntries},
			{ "TG2", PNOPARAM, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ShowEntries},
			{ "TG3", PNOPARAM, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ShowEntries},
			{ "TG4", PNOPARAM, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ShowEntries},
			{ "TG5", PNOPARAM, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ShowEntries},
			{ "TG6", PNOPARAM, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ShowEntries},
			{ "TG7", PNOPARAM, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ShowEntries},
			{ "TG8", PNOPARAM, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ExtractAndSendVoiceFromBank, &cMenu::mCard::ShowEntries},
		},
	},
	{
		M_VOICE,
		M_CARD,
		{ PNOPARAM, PNOPARAM, PNOPARAM }, { false, false, false },
		{ PNOPARAM, PNOPARAM, PNOPARAM, PNOPARAM },
		{
			{ "TG1", PNOPARAM, &cMenu::mCard::SendVoice, &cMenu::mCard::SendVoice, &cMenu::mCard::ShowEntries},
			{ "TG2", PNOPARAM, &cMenu::mCard::SendVoice, &cMenu::mCard::SendVoice, &cMenu::mCard::ShowEntries},
			{ "TG3", PNOPARAM, &cMenu::mCard::SendVoice, &cMenu::mCard::SendVoice, &cMenu::mCard::ShowEntries},
			{ "TG4", PNOPARAM, &cMenu::mCard::SendVoice, &cMenu::mCard::SendVoice, &cMenu::mCard::ShowEntries},
			{ "TG5", PNOPARAM, &cMenu::mCard::SendVoice, &cMenu::mCard::SendVoice, &cMenu::mCard::ShowEntries},
			{ "TG6", PNOPARAM, &cMenu::mCard::SendVoice, &cMenu::mCard::SendVoice, &cMenu::mCard::ShowEntries},
			{ "TG7", PNOPARAM, &cMenu::mCard::SendVoice, &cMenu::mCard::SendVoice, &cMenu::mCard::ShowEntries},
			{ "TG8", PNOPARAM, &cMenu::mCard::SendVoice, &cMenu::mCard::SendVoice, &cMenu::mCard::ShowEntries}
		}
	}

};

s_fx fx_settings = {
	true,
	true,
	70,
	50,
	50,
	30,
	65,
	99 };


Adafruit_SPITFT tft = Adafruit_SPITFT();

uint8_t cMenu::currentTG;
uint8_t cMenu::menu;
bool cMenu::pflag[3] = { false, false, false};
uint16_t cMenu::potparam[3];
uint16_t cMenu::bparam[8];
int16_t cMenu::parampos[PLAST];

bool cMenu::TGEnabled[8] = { true , true, true, true, true, true, true, true};

void cMenu::Init()
{
	tft.init(160, 128);
	tft.fillScreen(GREY);
	tft.setTextSize(0);
	tft.setTextColor(WHITE);
	tft.setFont(&Open_Sans_Condensed_Bold_16);
	currentTG = -1;
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

	uint16_t color = BUTTONCOLOR;
	tft.setTextColor(WHITE);
	if (TGEnabled[button] == false && menu == M_MAIN ) {
		color = LIGHTGREY;
		tft.setTextColor(GREY);
	}
	tft.getTextBounds(menuEntry[menu].button[button].name, col, row, &x1, &y1, &w, &h);
	if (button < 4)
	{
		h = 12;
		tft.writeFillRect(col, row, (2 * offset) - 8, BUTTONHEIGHT, color);
		tft.setCursor(col + offset - (w/2) - 4 , row + h + 4);
	}
	else {
		h = 12;
		tft.writeFillRect(col + 4, row, (2 * offset) - 4, BUTTONHEIGHT, color);
		tft.setCursor(col+offset - (w/2) + 4, row + h + 4);
	}
	tft.print(menuEntry[menu].button[button].name);
}

void cMenu::ShowValue(uint16_t param)
{
	bool colorflag = false;
	int16_t  x1, y1;
	uint16_t w, h;
	int32_t value;

	for (size_t i = 0; i < 3; i++)
	{
		if (param == potparam[i] && pflag[i] == true)
		{
			colorflag = true;
			i=2;
		}
	}
	if (param > PNHIGH)
	{
		switch (param)
		{
		case FCOMP_EN:
			value = fx_settings.comp_enable;
			colorflag = true;
			break;
		case FREV_EN:
			value = fx_settings.reverb_enable;
			colorflag = true; 
			break;
		case FSIZE:
			value = fx_settings.verbsize;
			break;
		case FDIFF:
			value = fx_settings.diffusion;
			break;
		case FRLEVEL:
			value = fx_settings.level;
			break;
		case FHIGHDAMP:
			value = fx_settings.highdamp;
			break;
		case FLOWDAMP:
			value = fx_settings.lowdamp;
			break;
		case FLOWPASS:
			value = fx_settings.lowpass;
			break;
		case MASTERVOLUME:
			value = fx_settings.mastervolume;
			break;
		default:
			break;
		}
	} else {
		value = dexed[currentTG].getValue(param);
	}
	//	printf("Showvalue parm %i, value %i\n", param, value);
	char text[7];
	itoa(value + tg_parameters[param].offset, text, 10);
	tft.setAddrWindow(VALUEPOS, parampos[param], VALUEWIDTH, BUTTONHEIGHT);
	if (colorflag)
	{
		tft.writeFillRect(VALUEPOS, parampos[param], VALUEWIDTH, BUTTONHEIGHT, WHITE);
	}
	else {
		tft.writeFillRect(VALUEPOS, parampos[param], VALUEWIDTH, BUTTONHEIGHT, tft.color565(255, 64, 64));
	}

	if (param == PCHANNEL)
	{
		if (value == 16)
			sprintf(text, "OMN");
		if (value == 17)
			sprintf(text, "OFF");
	}

	if (param == PMONO || param == PGLISS || param == FCOMP_EN || param == FREV_EN )
	{
		colorflag = true;
		sprintf(text, "OFF");
		if (value == 1)
			sprintf(text, "ON");
	}

	if (param == PPMODE)
	{
		colorflag = true;
		sprintf(text, "Fin");
		if (value == 1)
			sprintf(text, "Full");
	}

	tft.setTextColor(BLACK);
	tft.setFont(&Open_Sans_Condensed_Bold_16);
	tft.getTextBounds(text, VALUEPOS, parampos[param], &x1, &y1, &w, &h);
	tft.setCursor((VALUEPOS +(VALUEWIDTH /2) - (w / 2)), parampos[param] + h + 2);
	tft.print(text);
	tft.setTextColor(WHITE);
	tft.setAddrWindow(0,0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
}

void cMenu::menuBack(uint8_t cbparam)
{
	bparam[BUT1] = menuEntry[menu].prev;
	OpenMenu(0);
}

void cMenu::ParmToggle(uint8_t button)
{
	uint16_t parm = bparam[button];
	if (parm < FCOMP_EN)
	{
		uint16_t val = dexed[currentTG].getValue(parm);
		if (val == 1)
		{
			dexed[currentTG].parmDown(bparam[button]);
		}
		else {
			dexed[currentTG].parmUp(bparam[button]);
		}
		dexed[currentTG].sendParam(bparam[button]);
	}
	else {
		switch (parm)
		{
		case FCOMP_EN:
			if (fx_settings.comp_enable)
			{
				fx_settings.comp_enable = false;
			}
			else
			{
				fx_settings.comp_enable = true;
			}
			break;
		case FREV_EN:
			if (fx_settings.reverb_enable)
				fx_settings.reverb_enable = false;
			else
				fx_settings.reverb_enable = true;
			break;
		default:
			break;
		}
		dexed[0].sendParam(bparam[button]);
	}
	//	dexed[currentTG].sendMidi(bparam[button]);
	ShowValue(parm);
	showTGInfo(parm);
}

void cMenu::ParmSelect(uint8_t button)
{
	uint16_t parm = bparam[button];
	bool up = false;
	switch (button)
	{
	case BUT1:
	case BUT2:
	case BUT3:
	case BUT4:
		up = false;
		break;
	case BUT5:
	case BUT6:
	case BUT7:
	case BUT8:
		up = true;
		break;
	default:
		break;
	}

	if (parm < FCOMP_EN)
	{
		if (up)
		{
			dexed[currentTG].parmUp(parm);
			printf("Dexed param %i up\n", parm);
		}
		else
		{
			dexed[currentTG].parmDown(parm);
		}
		dexed[currentTG].sendParam(parm);
		if (parm == PPATCH || parm == PBANK) dexed[currentTG].getPatch();
	}
	else
	{
		switch (parm)
		{
		case FSIZE:
			if (up)
			{
				if (fx_settings.verbsize < tg_parameters[FSIZE].high)
					fx_settings.verbsize++;
			}
			else {
				if (fx_settings.verbsize > tg_parameters[FSIZE].low)
					fx_settings.verbsize--;
			}
			break;
		case FLOWDAMP:
			if (up)
			{
				if (fx_settings.lowdamp < tg_parameters[FLOWDAMP].high)
					fx_settings.lowdamp++;
			}
			else {
				if (fx_settings.lowdamp > tg_parameters[FLOWDAMP].low)
					fx_settings.lowdamp--;
			}
			break;
		case FHIGHDAMP:
			if (up)
			{
				if (fx_settings.highdamp < tg_parameters[FHIGHDAMP].high)
					fx_settings.highdamp++;
			}
			else {
				if (fx_settings.highdamp > tg_parameters[FHIGHDAMP].low)
					fx_settings.highdamp--;
			}
			break;
		case FLOWPASS:
			if (up)
			{
				if (fx_settings.lowpass < tg_parameters[FLOWPASS].high)
					fx_settings.lowpass++;
			}
			else {
				if (fx_settings.lowpass > tg_parameters[FLOWPASS].low)
					fx_settings.lowpass--;
			}
			break;
		case FDIFF:
			if (up)
			{
				if (fx_settings.diffusion < tg_parameters[FDIFF].high)
					fx_settings.diffusion++;
			}
			else {
				if (fx_settings.diffusion > tg_parameters[FDIFF].low)
					fx_settings.diffusion--;
			}
			break;
		case FRLEVEL:
			if (up)
			{
				if (fx_settings.level < tg_parameters[FRLEVEL].high)
					fx_settings.level++;
			}
			else {
				if (fx_settings.level > tg_parameters[FRLEVEL].low)
					fx_settings.level--;
			}
			break;
		case MASTERVOLUME:
			if (up)
			{
				if (fx_settings.mastervolume < tg_parameters[MASTERVOLUME].high)
					fx_settings.mastervolume++;
			}
			else {
				if (fx_settings.mastervolume > tg_parameters[MASTERVOLUME].low)
					fx_settings.mastervolume--;
			}
			break;
		}
		dexed[0].sendParam(parm);
	}
	ShowValue(bparam[button]);
	showTGInfo(bparam[button]);
}

void cMenu::ParmPot(uint8_t channel)
{
	uint16_t param = potparam[channel];
	int32_t value = map(Pots.getPot(channel), POT_MIN, POT_MAX, tg_parameters[param].low, tg_parameters[param].high);
	if (param < FCOMP_EN)
	{
		printf("TG Param\n");
		if (value == dexed[currentTG].getValue(param) && !pflag[channel])
		{
			pflag[channel] = true;
			ShowValue(param);
		}
		if (value != dexed[currentTG].getValue(param) && pflag[channel])
		{
			value = dexed[currentTG].setValue(param, value);
			ShowValue(param);
			dexed[currentTG].sendParam(param);
			if (param == PPATCH || param == PBANK) dexed[currentTG].getPatch();
			showTGInfo(param);
		}
	}
	else {
		uint8_t *fxvalue;
		switch (param)
		{
		case FCOMP_EN:
			if (fx_settings.comp_enable)
				*fxvalue = 1;
			else
				*fxvalue = 0;
			break;
		case FREV_EN:
			if (fx_settings.reverb_enable)
				*fxvalue = 1;
			else
				*fxvalue = 0;
			break;
		case FSIZE:
			fxvalue = &fx_settings.verbsize;
			break;
		case FDIFF:
			fxvalue = &fx_settings.diffusion;
			break;
		case FRLEVEL:
			fxvalue = &fx_settings.level;
			break;
		case FHIGHDAMP:
			fxvalue = &fx_settings.highdamp;
			break;
		case FLOWDAMP:
			fxvalue = &fx_settings.lowdamp;
			break;
		case FLOWPASS:
			fxvalue = &fx_settings.lowpass;
			break;
		case MASTERVOLUME:
			fxvalue = &fx_settings.mastervolume;
			break;
		default:
			break;
		}
		uint8_t v = *fxvalue;
		if (value == v && !pflag[channel])
		{
			pflag[channel] = true;
			ShowValue(param);
		}
		if (value != v && pflag[channel])
		{
			*fxvalue = value;
			ShowValue(param);
			dexed[0].sendParam(param);
			showTGInfo(param);
		}
	}
}

void cMenu::setButtonCallback(uint8_t button, uint16_t param, void (*callback)(uint8_t button))
{
	int16_t pos = 0;
	switch (button)
	{
	case BUT1:
	case BUT5:
		pos = POSA;
		break;
	case BUT2:
	case BUT6:
		pos = POSB;
		break;
	case BUT3:
	case BUT7:
		pos = POSC;
		break;
	case BUT4:
	case BUT8:
		pos = POSD;
		break;
	}

	buttons.setCallback(button, callback);

	if (callback == nullptr)
		bparam[button] = PNOPARAM;
	bparam[button] = param;
	parampos[param] = pos;
	ShowButtonText(button);
}

void cMenu::clearCallbacks()
{
	for (uint8_t button = 0; button < 8; button++)
	{
		buttons.setCallback(button, NULL);		
		buttons.setLongCallback(button, NULL);
		buttons.setDBLCallback(button, NULL);
	}
	resetPotCB(TOPPOT);
	resetPotCB(MIDPOT);
	resetPotCB(BOTPOT);
}

void cMenu::setPotCallback(uint8_t channel, uint16_t param)
{
	if (param == PNOPARAM)
	{
		resetPotCB(channel);
		return;
	}
	else {
		Pots.setPotCallback(channel, &cMenu::ParmPot);
	}
	potparam[channel] = param;
	pflag[channel] = false;
	uint16_t pot = map(Pots.getPot(channel), POT_MIN, POT_MAX, tg_parameters[param].low, tg_parameters[param].high);

	if (pot == dexed[currentTG].getValue(param))
		pflag[channel] = true;
}

void cMenu::resetPotCB(uint8_t channel)
{
	potparam[channel] = PNOPARAM;
	pflag[channel] = false;

	Pots.setPotCallback(channel, NULL);
}

void cMenu::showTGInfo(int16_t param)
{
	int16_t  x1, y1;
	uint16_t w, h;
	char infostring[11] = { 0 };
	tft.setTextColor(WHITE);
	if (param == -1)
		tft.writeFillRect(0, 0, 160, 37, BLACK);
	if (menu != M_MAIN && menu < M_DEXED)
	{
//		tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
		
		// Current TG
		if (param == -1 )
		{	
			tft.setFont(&Roboto_Bold_10);
			tft.writeFillRect(0, 0, 23, 9, BLACK);
			sprintf(infostring, "T %1i", currentTG + 1);
			tft.setCursor(0, 9);
			tft.print(infostring);
		}
		// Bank Number
		if (param == -1 || param == PBANK)
		{
			tft.setFont(&Roboto_Bold_10);
			tft.writeFillRect(24, 0, 26, 9, BLACK);
			sprintf(infostring, "B %3i", dexed[currentTG].getValue(PBANK) + 1);
			tft.setCursor(24, 9);
			tft.print(infostring);

			sprintf(infostring, "%s", dexed[currentTG].getBankName());
			tft.setFont(&Open_Sans_Condensed_Bold_16);
			tft.getTextBounds(infostring, 0, 14, &x1, &y1, &w, &h);
			tft.setCursor(55, 16);
			tft.fillRect(50, 0, 110, 16, GREY);
			tft.print(infostring);
		}
		// Channel
		if (param == -1 || param == PCHANNEL)
		{
			tft.setFont(&Roboto_Bold_10);
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
			tft.setFont(&Roboto_Bold_10);
			tft.writeFillRect(24, 10, 26, 8, BLACK);
			sprintf(infostring, "P %3i", dexed[currentTG].getValue(PPATCH) + 1);
			tft.setCursor(24, 18);
			tft.print(infostring);

			sprintf(infostring, "%s", dexed[currentTG].getVoiceName());
			tft.setFont(&Open_Sans_Condensed_Bold_16);
			tft.getTextBounds(infostring, 0, 14, &x1, &y1, &w, &h);
			tft.setCursor(105 - (w / 2), 32);
			tft.fillRect(50, 16, 110, 20, GREY);
			tft.print(infostring);
		}
		// Volume 
		if (param == -1 || param == PVOL)
		{
			tft.setFont(&Roboto_Bold_10);
			tft.writeFillRect(0, 19, 19, 8, BLACK);
			uint8_t vol = dexed[currentTG].getValue(PVOL);
			if (vol != 0)
				tft.fillTriangle(0, 27, (vol / 8), 27, (vol / 8), 27 - (vol / 16), WHITE);
		}

		// Panning
		if (param == -1 || param == PPAN)
		{
			tft.setFont(&Roboto_Bold_10);
			int8_t pan = map(dexed[currentTG].getValue(PPAN), -63,63,0,25);
			tft.writeFillRect(21, 19, 26, 8, BLACK);
			tft.fillTriangle(34, 23, 21, 19, 21, 27, GREY);
			tft.fillTriangle(34, 23, 47, 19, 47, 27, GREY);
			tft.writeFillRect(21 + pan, 19, 2, 8, WHITE);
		}
		
		tft.drawFastHLine(50, 17, 110, BLACK);
	}
	if (menu == M_MAIN || menu >= M_DEXED) showDexedInfo(param);
}

void cMenu::showDexedInfo(int16_t param)
{
	char infostring[11] = { 0 };
	tft.setTextColor(WHITE);
	tft.setFont(&Roboto_Condensed);
	if (param == -1 || param == FCOMP_EN)
	{
//		tft.writeFillRect(0, 0, 23, 9, BLACK);
		tft.setTextColor(tft.color565(0, 0, 0xFF)); // Green
		if (!fx_settings.comp_enable)
			tft.setTextColor(tft.color565(0xff, 0, 0)); // Red
		sprintf(infostring, "CMP");
		tft.setCursor(0, 35);
		tft.print(infostring);
		tft.setTextColor(WHITE);
	}
	//MASTERVOLUME Must be placed here to get the correct text color
	if (param == -1 || param == MASTERVOLUME)
	{
		tft.fillTriangle(0, 22, 32, 12, 32, 22, WHITE);
		tft.setFont(&Roboto_Bold_10);
		//	tft.writeFillRect(0, 0, 23, 9, BLACK);
		//sprintf(infostring, "MVol %1i", fx_settings.reverb_enable);
		tft.setCursor(5, 9);
		tft.print("Main");
	}

	tft.setFont(&Roboto_Condensed);

	if (fx_settings.reverb_enable)
	{
		tft.setTextColor(WHITE);
	}
	else {
		tft.setTextColor(tft.color565(0xFF, 0, 0)); // A Darker Red
	}

	if (param == FREV_EN || param == FCOMP_EN) param = -1;
	char names[6][7] = {"Level", "Size", "Diff", "H Damp", "L Damp", "L Pass"};
	uint8_t xpos = 35;
	uint8_t ypos = 9;
	for (size_t i = 0; i < 6; i++)
	{
		tft.setCursor(xpos, ypos);
		tft.print(names[i]);
		ypos = ypos + 13;
		if (ypos > 35)
		{
			ypos = 9;
			xpos = 95;
		}
	}
	//		tft.setFont(&Roboto_Bold_10);
	if (param == -1 || param == FSIZE)
	{
		tft.writeFillRect(70, 12, 25, 12, BLACK);
		sprintf(infostring, "%2i", fx_settings.verbsize);
		tft.setCursor(70, 22);
		tft.print(infostring);
	}
	if (param == -1 || param == FDIFF)
	{
		tft.writeFillRect(70, 24, 25, 12, BLACK);
		sprintf(infostring, "%2i", fx_settings.diffusion);
		tft.setCursor(70, 35);
		tft.print(infostring);
	}
	if (param == -1 || param == FRLEVEL)
	{
		tft.writeFillRect(70, 0, 25, 12, BLACK);
		sprintf(infostring, "%2i", fx_settings.level);
		tft.setCursor(70, 9);
		tft.print(infostring);
	}
	if (param == -1 || param == FHIGHDAMP)
	{
		tft.writeFillRect(145, 0, 25, 12, BLACK);
		sprintf(infostring, "%1i", fx_settings.highdamp);
		tft.setCursor(145, 9);
		tft.print(infostring);
	}
	if (param == -1 || param == FLOWDAMP)
	{
		tft.writeFillRect(145, 12, 25, 12, BLACK);
		sprintf(infostring, "%1i", fx_settings.lowdamp);
		tft.setCursor(145, 22);
		tft.print(infostring);
	}
	if (param == -1 || param == FLOWPASS)
	{
		tft.writeFillRect(145, 24, 25, 12, BLACK);
		sprintf(infostring, "%1i", fx_settings.lowpass);
		tft.setCursor(145, 35);
		tft.print(infostring);
	}
}

void cMenu::setDexedParm(uint16_t parm, int32_t val, uint8_t instance)
{
	if (parm < FCOMP_EN)
	{
		dexed[instance].setValue(parm, val);
	}
}

void cMenu::TGEnable(uint8_t button)
{
	static uint8_t channels[8];
	if (TGEnabled[button] == true)
	{
		channels[button] = dexed[button].getValue(PCHANNEL);
		TGEnabled[button] = false;
		dexed[button].setValue(PCHANNEL, 17);
		dexed[button].sendParam(PCHANNEL); // Set TG Off
		printf("TG Enable OFF: %i\n", dexed[button].getValue(PCHANNEL));
	}
	else {
		TGEnabled[button] = true;
		dexed[button].setValue(PCHANNEL, channels[button]);
		dexed[button].sendParam(PCHANNEL);
		printf("TG Enable ON: %i\n", dexed[button].getValue(PCHANNEL));
	}
	ShowButtonText(button);
}

void cMenu::buildMenu()
{
//	menu = men;
	clearCallbacks();
	tft.setAddrWindow(0, 0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT);
	if ( menu == M_MAIN  || menu == M_VOICE )
		tft.writeFillRect(0, 0, MIPI_DISPLAY_WIDTH, MIPI_DISPLAY_HEIGHT, BLACK);
	else
		tft.writeFillRect(VALUEPOS, POSA, VALUEWIDTH, MIPI_DISPLAY_HEIGHT - POSA, BLACK);

	for (size_t i = 0; i < 8; i++)
	{
		setButtonCallback(i,
			menuEntry[menu].button[i].parameter,
			menuEntry[menu].button[i].callback);
		buttons.setDBLCallback(i, menuEntry[menu].button[i].dblcallback);
		buttons.setLongCallback(i, menuEntry[menu].button[i].longcallback);
	}
	for (size_t pot = TOPPOT; pot <= BOTPOT; pot++)
	{
		setPotCallback(pot, menuEntry[menu].potparams[pot]);
		pflag[pot] = menuEntry[menu].potflags[pot];
	}

	if ( menu != M_BANKVOICE && menu != M_VOICE)
		showTGInfo(-1);

	for (size_t i = 0; i < 4; i++)
	{
		if (menuEntry[menu].showvalue[i] != PNOPARAM)
		{
			parampos[menuEntry[menu].showvalue[i]] = 38 + (i * 22);
			ShowValue(menuEntry[menu].showvalue[i]);
		}
	}
}

void cMenu::OpenMenu(uint8_t button)
{
	if (bparam[button] != PNOPARAM)
	{
		if (menu == M_MAIN && bparam[button] == M_TG)
		{
			currentTG = button;
		}
		menu = bparam[button];
	}
	else {
		menu = M_MAIN;
	}

	if ( menu == M_MAIN )
		dexed[0].getConfig();

	if (menu == M_TG)
	{
//		currentTG = button;   // TODO: Only when entering from main menu!!
		dexed[currentTG].getPatch();
		dexed[currentTG].getBank();
	}
	buildMenu();
}

void cMenu::Dexed(uint8_t button)
{
	menu = M_DEXED;
	dexed[0].getConfig();
	buildMenu();
}
