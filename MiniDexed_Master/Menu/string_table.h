#include "TG.h"

const char* const menus[][8] = { {"TG1", "TG2", "TG3", "TG4","TG5", "TG6", "TG7", "TG8" },
   { "MIDI", "MAIN", "TUNE", " ", " ", " ", " ", "Back" },
	{ "Bnk-", "Pch-", "Chn-", " ", "Bnk+","Pch+","Chn+","Back"},
	{ "CFrq-", "Reso-", "Rvrb-", "Comp", "CFrq+", "Reso+", "Rvrb+", "Back"},
	{ "Oct -", "Tune-"," "," ", "Oct +", "Tune+", " ", "Back" },
	{"Pan -", "Vol -"," " ," ", "Pan +", "Vol +", " ", "Back"}
};

enum menulvl { M_MAIN=0, M_TG, M_TG_MIDI, M_TG_MAIN, M_TG_TUNE, M_TG_OUT};