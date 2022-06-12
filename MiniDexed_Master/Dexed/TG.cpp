#include <stdlib.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <string.h>
#include "pico/util/queue.h"
#include "midicore.h"
#include "TG.h"
#include "tools.h"

queue_t tg_fifo;
size_t CountedObj::total_;

cTG::cTG()
{
	mobject_id = CountedObj::showCount() -1;
	sprintf(mvoicename,"Unknown");
	mbank = 0;
	setParm(PBANK, &mbank);
	mpatch = 0;
	setParm(PPATCH, &mpatch);
	mchannel = 17;
	setParm(PCHANNEL, &mchannel);
	mfreq = 127;
	setParm(PFREQ, &mfreq);
	mreso = 0;
	setParm(PRESO, &mreso);
	mrev = 0;
	setParm(PVERB, &mrev);
	mcomp = 0;
	setParm(PCOMP, &mcomp);
	mnote_shift = 0;
	setParm(PSHIFT, &mnote_shift);
	mtune = 0;
	setParm(PTUNE, &mtune);
	mpan = 64;
	setParm(PPAN, &mpan);
	mvol = 127;
	setParm(PVOL, &mvol);
	mrange = 2;
	setParm(PBRANGE, &mrange);
	mpmode = 0;
	setParm(PPMODE, &mpmode);
	mmono = 0;
	setParm(PMONO, &mmono);
	mstep = 0;
	setParm(PBSTEP, &mstep);
	mgliss = 0;
	setParm(PGLISS, &mgliss);
	mtime = 0;
	setParm(PTIME, &mtime);
	mnote_low = 0;
	setParm(PNLOW, &mnote_low);
	mnote_high = 127;
	setParm(PNHIGH, &mnote_high);
}

void cTG::setParm(uint16_t parm, int16_t* data)
{
	mparms[parm].val16t = data;
}

int32_t cTG::parmUp(uint16_t parm)
{
	int16_t v16t;

	v16t = *mparms[parm].val16t;
	if (v16t < ranges[parm][RMAX])
	{
		v16t++;
	}
	*mparms[parm].val16t = v16t;
	return v16t;
}

int32_t cTG::parmDown(uint16_t parm)
{
	int16_t v16t;

	v16t = *mparms[parm].val16t;
	if (v16t > ranges[parm][RMIN])
	{
		v16t--;
	}
	*mparms[parm].val16t = v16t;
	return v16t;
}

int32_t cTG::setValue(uint16_t parm, int32_t value)
{
	if (value >= ranges[parm][RMIN] && value < ranges[parm][RMAX])
	{
		*mparms[parm].val16t = value;
	}
	return value;
}

int32_t cTG::getValue(uint16_t parm)
{
	return *mparms[parm].val16t;
}

void cTG::sendParam(uint16_t parm, int32_t value)
{
	dexed_t tgdata;
	if (parm != PCHANNEL)

		tgdata.channel = mchannel;
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
		value = map(value, ranges[PTUNE][RMIN], ranges[PTUNE][RMAX], 3, 126);
		break;
	case PPAN:
//		printf("value: %i\n", value);
		value = map(value, ranges[PPAN][RMIN], ranges[PPAN][RMAX], 0, 127);
		break;
	default:
		break;
	}
	tgdata.value = value & 0xff;
	queue_add_blocking(&tg_fifo, &tgdata);
}

void cTG::getPatch()
{
//	sysex_t raw_sysex;
	dexed_t tgdata;

	tgdata.channel = 0;
	tgdata.instance = mobject_id;
	printf("getPatch data  for instanceID %i\n", mobject_id);
	tgdata.cmd = 1;
	tgdata.parm = 0;
	tgdata.value = mpatch;
	queue_add_blocking(&tg_fifo, &tgdata);
//	queue_remove_blocking(&sysex_fifo, &raw_sysex);
//	setSysex(raw_sysex);
}

void cTG::getConfig()
{
//	sysex_t raw_sysex;
	dexed_t tgdata;

	tgdata.channel = 0;
	tgdata.instance = mobject_id;
	tgdata.cmd = 2;
	tgdata.parm = 0;
	tgdata.value = 0;
	queue_add_blocking(&tg_fifo, &tgdata);
}

void cTG::setSysex(sysex_t sysex)
{
	//printf("setSysex size: %i\n", sysex.length);
	uint8_t offset = VOICEDATA_HEADER;
	if (sysex.length > VOICEDATA_SIZE ) offset = offset + (sysex.length - VOICEDATA_SIZE);
	//printf("offset %i\n", offset);
		
//	memcpy(msysex, sysex.buffer+offset, VOICEDATA_SIZE);
	//for (size_t i = VNAME_OFFSET; i <= VOICEDATA_SIZE; i++)
	//{
	//	printf("%02X, ", msysex[i]);
	//}
	//printf("\n");
	memcpy(mvoicename, sysex.buffer + offset + VNAME_OFFSET, 10);
	mvoicename[11] = '\0';
//	printf("Voice Name in setSysex: %s\n", mvoicename);
}
