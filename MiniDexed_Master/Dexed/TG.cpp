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
	mbank = 0;
	mpatch = 0;
	mchannel = 0;
	mfreq = 127;
	mreso = 0;
	mrev = 0;
	mcomp = false;
	mshift = 0;
	mtune = 0;
	mpan = 0;
	mvol = 127;
	mrange = 2;
	mstep = 0;
	mmono = false;
	mpmode = 0;
	mgliss = 0;
	mtime = 0;
	sprintf(mvoicename,"Unknown");
	setParmType(PBANK, &mbank);
	setParmType(PPATCH, &mpatch);
	setParmType(PCHANNEL, &mchannel);
	setParmType(PFREQ, &mfreq);
	setParmType(PRESO, &mreso);
	setParmType(PVERB, &mrev);
	setParmType(PCOMP, &mcomp);
	setParmType(PSHIFT, &mshift);
	setParmType(PTUNE, &mtune);
	setParmType(PPAN, &mpan);
	setParmType(PVOL, &mvol);
	setParmType(PBRANGE, &mrange);
	setParmType(PPMODE, &mpmode);
	setParmType(PMONO, &mmono);
	setParmType(PBSTEP, &mstep);
	setParmType(PGLISS, &mgliss);
	setParmType(PTIME, &mtime);
	setParmType(PLOW, &mlow);
	setParmType(PHIGH, &mhigh);
}

void cTG::setParmType(uint16_t parm, int8_t* data)
{
	mparms[parm].type = 0;
	mparms[parm].val8s = data;
}

void cTG::setParmType(uint16_t parm, uint8_t* data)
{
	mparms[parm].type = 1;
	mparms[parm].val8t = data;
}

void cTG::setParmType(uint16_t parm, uint16_t* data)
{
	mparms[parm].type = 2;
	mparms[parm].val16t = data;
}

int32_t cTG::parmUp(uint16_t parm)
{
	int8_t v8s;
	uint8_t v8t;
	uint16_t v16t;

	switch (mparms[parm].type)
	{
	case 0:
		v8s = *mparms[parm].val8s;
		if (v8s < ranges[parm][RMAX])
		{
			v8s++;
		}
		*mparms[parm].val8s = v8s;
		return v8s;
		break;
	case 1:
		v8t = *mparms[parm].val8t;
		if (v8t < ranges[parm][RMAX])
		{
			v8t++;
		}
		*mparms[parm].val8t = v8t;
		return v8t;
		break;
	case 2:
		v16t = *mparms[parm].val16t;
		if (v16t < ranges[parm][RMAX])
		{
			v16t++;
		}
		*mparms[parm].val16t = v16t;
		return v16t;
		break;
	default:
		return -2;
		break;
	}
}

int32_t cTG::parmDown(uint16_t parm)
{
	int8_t v8s;
	uint8_t v8t;
	uint16_t v16t;

	switch (mparms[parm].type)
	{
	case 0:
		v8s = *mparms[parm].val8s;
		if (v8s > ranges[parm][RMIN])
		{
			v8s--;
		}
		*mparms[parm].val8s = v8s;
		return v8s;
		break;
	case 1:
		v8t = *mparms[parm].val8t; 
		if (v8t > ranges[parm][RMIN])
		{
			v8t--;
		}
		*mparms[parm].val8t = v8t;		
		return v8t;
		break;
	case 2:
		v16t = *mparms[parm].val16t;
		if (v16t > ranges[parm][RMIN])
		{
			v16t--;
		}
		*mparms[parm].val16t = v16t;
		return v16t;
		break;
	default:
		return -2;
		break;
	}
}

int32_t cTG::setValue(uint16_t parm, int32_t value)
{
	if (value >= ranges[parm][RMIN] && value < ranges[parm][RMAX])
	{
		switch (mparms[parm].type)
		{
		case 0:
			*mparms[parm].val8s = value;
			break;
		case 1:
			*mparms[parm].val8t = value;
			break;
		case 2:
			*mparms[parm].val16t = value;
			break;
		default:
			break;
		}
//		sendParam(parm, value);
//		printf("data: %08X channel: %i\r\n", tgdata.parm, tgdata.channel);
	}
	return value;
}

int32_t cTG::getValue(uint16_t parm)
{
	switch (mparms[parm].type)
	{
	case 0:
		return *mparms[parm].val8s;
		break;
	case 1:
		return *mparms[parm].val8t;
		break;
	case 2:
		return *mparms[parm].val16t;
		break;
	default:
		printf("Unknown datatype\r\n");
		return 0;
		break;
	}
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
