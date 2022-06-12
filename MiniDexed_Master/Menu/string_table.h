#include "TG.h"

const char* const menus[][8] = { {"TG1", "TG2", "TG3", "TG4","TG5", "TG6", "TG7", "TG8" },
   { "MIDI", "Filter", "Tuning", "Out", "Pitch", " ", " ", "Back" },
	{ "Bnk-", "Pch-", "Chn-", " ", "Bnk+","Pch+","Chn+","Back"},
	{ "CFrq-", "Reso-", "Rvrb-", " ", "CFrq+", "Reso+", "Rvrb+", "Back"},
	{ "Oct -", "Tune-"," "," ", "Oct +", "Tune+", " ", "Back" },
	{ "Pan -", "Vol -", " " , " ", "Pan +", "Vol +", " ", "Back"},
	{ "PBRng-", "PBStp-", "Porta", "Mono", "PBRng+", "PBStep+", " ", "Back"}
};

const char* const shortParamNames[] = { "---", "BNK", "PGM", "CHN", "CUT", "RES", "REV", "CMP", "SHF", "DET", "PAN", "VOL", "PRA","MOD", "MON", "PST", "GLS", "TIM" };
// global: Comp, Reverb Settings

enum menulvl { M_MAIN=0, M_TG, M_TG_MIDI, M_TG_FILT, M_TG_TUNE, M_TG_OUT, M_TG_PITCH};