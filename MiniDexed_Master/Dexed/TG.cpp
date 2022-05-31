#include <stdlib.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <string.h>

#include "TG.h"

//uint16_t cTG::mbank;
//uint8_t cTG::mpatch;
//uint8_t cTG::mchannel;
//uint8_t cTG::mfreq;
//uint8_t cTG::mreso;
//uint8_t cTG::mrev;
//int8_t cTG::mdetune;
//int8_t cTG::mpan;
//int8_t cTG::mvol;

cTG::cTG()
{
	mbank = 0;
	mpatch = 0;
	mchannel = 0;
	mfreq = 127;
	mreso = 127;
	mrev = 0;
	mdetune = 0;
	mpan = 64;
	mvol = 127;
}

uint16_t cTG::BankUp()
{
	if (mbank < MAXBANKS)
		mbank++;
	return mbank;
}

uint16_t cTG::BankDown()
{
	if (mbank > 0)
		mbank--;
	return mbank;
}

uint16_t cTG::setBank(uint16_t bank)
{
	if ( bank < MAXBANKS )
		mbank = bank;
	return mbank;
};

uint8_t cTG::PatchUp()
{
	if (mpatch < MAXPATCH)
		mpatch++;
	return mpatch;
}

uint8_t cTG::PatchDown()
{
	if (mpatch > 0)
		mpatch--;
	return mpatch;
}

uint8_t cTG::setPatch(uint8_t patch)
{
	if (patch >= 0 && patch < MAXPATCH)
		mpatch = patch;
	return mpatch;
};

uint8_t cTG::ChannelUp()
{
	if (mchannel < MAXCHANNEL)
		mchannel++;
	return mchannel;
}

uint8_t cTG::ChannelDown()
{
	if (mchannel > 0)
		mchannel--;
	return mchannel;
}

uint8_t cTG::setChannel(uint8_t channel)
{
	if (channel <= MAXCHANNEL && channel >= 0)
		mchannel = channel;
	return mchannel;
};

void cTG::Cutoff(uint8_t freq)
{
};

void cTG::Reso(uint8_t reso)
{
};

void cTG::Rvrb(uint8_t level)
{
};

void cTG::Detune(int8_t detune)
{
};

void cTG::Panning(int8_t pan)
{
};

void cTG::Volume(uint8_t vol)
{
};
