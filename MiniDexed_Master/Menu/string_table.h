#include "TG.h"

const char* const menus[][8] = { {"TG1", "TG2", "TG3", "TG4","TG5", "TG6", "TG7", "TG8" },
   { "MIDI", "MAIN", "TUNE", " ", " ", " ", " ", " "},
	{ "Bnk-", "Pch-", "Chn-", " ", "Bnk+","Pch+","Chn+"," "},
	{ "CFrq-", "Reso-", "Rvrb-", "Comp", "CFrq+", "Reso+", "Rvrb+", " "},
	{ "Oct -", "Tune-", "Pan -", "Vol -", "Oct +", "Tune+", "Pan +", "Vol +"}};

enum menulvl { M_MAIN=0, M_TG, M_TG_MIDI, M_TG_MAIN, M_TG_TUNE};