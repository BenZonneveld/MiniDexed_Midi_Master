#include <stdlib.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <string.h>
#include "pico/util/queue.h"
#include "midicore.h"
#include "TG.h"
#include "tools.h"
#include "mainmenu.h"

queue_t tg_fifo;
size_t CountedObj::total_;

tg_params tg_parameters[] = {
	{ 0,0,0,"---", 0 },			// PNOPARAM
	{ 0, MAXBANKS, 1, "BNK", F_BANK },	// PBANK
	{ 0, MAXPATCH, 1, "PGM", F_PATCH },	// PPATCH
	{ 0, MAXCHANNEL, 1, "CHN",F_CHANNEL },// PCHANNEL
	{ 0, MAXFREQ,0, "CUT", F_FREQ },	// PFREQ
	{ 0, MAXRESO, 0, "RES", F_RESO },	// PRESO
	{ 0, MAXREV, 0, "REV", F_VERB },	// PVERB
	{ -48,48,0, "SHF", F_SHIFT },		// PSHIFT
	{ -99, 99, 0, "DET",F_TUNE },		// PTUNE
	{ -64,64,0, "PAN", F_PAN },		// PPAN
	{ 0, 127, 0, "VOL",F_VOL },		// PVOL
	{ 0,12,0, "PBR", F_BRANGE },			// PBRANGE
	{ 0, 1,0, "MOD", F_PMODE },			// PPMODE
	{ 0,1,0, "MON", F_MONO },			// PMONO
	{ 0,12, 0, "PBS", F_BSTEP },			// PBSTEP
	{ 0,1,0, "GLS", F_GLISS },			// PGLISS
	{ 0,99,0, "TIM", F_TIME },			// PTIME
	{ 0,127,0, "NLL", F_NLOW },			// PNLOW
	{ 0,127,0, "NLH", F_NHIGH },			// PNHIGH
	{ 0,1,0, "CMP", F_COMP_EN },			//	FCOMP_EN,
	{ 0,1,0, "VRB", F_REV_EN },			// FREV_EN,
	{0,99,0, "SIZ", F_SIZE },			// FSIZE,
	{0,99,0, "LDP", F_LOWDAMP},			// FLOWDAMP,
	{0,99,0, "HDP", F_HIGHDAMP},			// FHIGHDAMP,
	{0,99,0, "LPS", F_LOWPASS},			// FLOWPASS,
	{0,99,0, "DIF", F_DIFF},			// FDIFF,
	{0,99,0, "LEV", F_RLEVEL}				// FRLEVEL,
};

cTG::cTG()
{
	mobject_id = CountedObj::showCount() -1;
	sprintf(mvoicename,"Unknown");
	setParm(PBANK, 0);
	setParm(PPATCH, 0);
	setParm(PCHANNEL, 0);
	setParm(PFREQ, 99);
	setParm(PRESO, 0);
	setParm(PVERB, 0);
	setParm(PSHIFT, 0);
	setParm(PTUNE, 0);
	setParm(PPAN, 0);
	setParm(PVOL, 99);
	setParm(PBRANGE, 2);
	setParm(PPMODE, 0);
	setParm(PMONO, 0);
	setParm(PBSTEP, 0);
	setParm(PGLISS, 0);
	setParm(PTIME, 0);
	setParm(PNLOW, 0);
	setParm(PNHIGH, 127);
}

void cTG::setParm(int16_t parm, int16_t value)
{
	mparms[parm] = value;
}

int32_t cTG::parmUp(int16_t parm)
{
	if (mparms[parm] < tg_parameters[parm].high)
	{
		mparms[parm]++;
	}
	return mparms[parm];
}

int32_t cTG::parmDown(int16_t parm)
{
	if (mparms[parm] > tg_parameters[parm].low)
	{
		mparms[parm]--;
	}
	return mparms[parm];
}

int32_t cTG::setValue(int16_t parm, int32_t value)
{
	if (value >= tg_parameters[parm].low && value <= tg_parameters[parm].high)
	{
		mparms[parm] = value;
	}
	return value;
}

int32_t cTG::getValue(int16_t parm)
{
	return mparms[parm];
}

void cTG::sendParam(int16_t parm)
{
	int32_t value;
	if ( parm < FCOMP_EN )
		value = mparms[parm];
	else
	{
		switch(parm)
		{
		case FCOMP_EN:
			value = fx_settings.comp_enable;
			break;
		case FREV_EN:
			value = fx_settings.reverb_enable;
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
	}
	dexed_t tgdata;
	if (parm != PCHANNEL)

		tgdata.channel = mparms[PCHANNEL];
	else
		tgdata.channel = mobject_id;
	tgdata.instance = mobject_id;
	tgdata.cmd = 0;
//	tgdata.val1 = value & 0x7f;
//	tgdata.val2 = (value >> 7) & 0x7f;
	tgdata.parm = parm;
	switch (parm)
	{
	case PTUNE:
//		printf("value: %i\n", value);
		value = map(value, tg_parameters[PTUNE].low, tg_parameters[PTUNE].high, 3, 126);
		break;
	case PPAN:
//		printf("value: %i\n", value);
		value = map(value, tg_parameters[PPAN].low, tg_parameters[PPAN].high, 0, 127);
		break;
	default:
		break;
	}
	tgdata.value = value & 0xff;
	queue_add_blocking(&tg_fifo, &tgdata);
	if (parm == PBANK) getBank();
}

void cTG::getPatch()
{
	dexed_t tgdata;

	tgdata.channel = 0;
	tgdata.instance = mobject_id;
	printf("getPatch data  for instanceID %i\n", mobject_id);
	tgdata.cmd = 1;
	tgdata.parm = 0;
	tgdata.value = mparms[PPATCH];
	queue_add_blocking(&tg_fifo, &tgdata);
}

void cTG::getConfig()
{
	dexed_t tgdata;

	tgdata.channel = 0;
	tgdata.instance = mobject_id;
	tgdata.cmd = 2;
	tgdata.parm = 0;
	tgdata.value = 0;
	queue_add_blocking(&tg_fifo, &tgdata);
}

void cTG::getBank()
{
	dexed_t tgdata;

	tgdata.channel = 0;
	tgdata.instance = mobject_id;
	tgdata.cmd = 3;
	tgdata.parm = 0;
	tgdata.value = 0;
	queue_add_blocking(&tg_fifo, &tgdata);
}

void cTG::setSysex(sysex_t sysex)
{
	uint8_t offset = DATA_HEADER;
	if (sysex.length > VOICEDATA_SIZE ) offset = offset + (sysex.length - VOICEDATA_SIZE);
		
	memcpy(mvoicename, sysex.buffer + offset + VNAME_OFFSET, 10);
	mvoicename[10] = '\0';
}

void cTG::setBankName(sysex_t sysex)
{
	memcpy(mbankname, sysex.buffer + 6, 32);
}