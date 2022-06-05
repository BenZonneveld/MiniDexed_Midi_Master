#include <stdlib.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include "pico/util/queue.h"
#include "midicore.h"
#include "TG.h"

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
	mtranspose = 0;
	mtune = 0;
	mpan = 0;
	mvol = 127;
	setParmType(PBANK, &mbank);
	setParmType(PPATCH, &mpatch);
	setParmType(PCHANNEL, &mchannel);
	setParmType(PFREQ, &mfreq);
	setParmType(PRESO, &mreso);
	setParmType(PVERB, &mrev);
	setParmType(PCOMP, &mcomp);
	setParmType(PTRANS, &mtranspose);
	setParmType(PTUNE, &mtune);
	setParmType(PPAN, &mpan);
	setParmType(PVOL, &mvol);
}

void cTG::setParmType(uint16_t parm, int8_t* data)
{
	parms[parm].type = 0;
	parms[parm].val8s = data;
}

void cTG::setParmType(uint16_t parm, uint8_t* data)
{
	parms[parm].type = 1;
	parms[parm].val8t = data;
}

void cTG::setParmType(uint16_t parm, uint16_t* data)
{
	parms[parm].type = 2;
	parms[parm].val16t = data;
}

int32_t cTG::parmUp(uint16_t parm)
{
	int8_t v8s;
	uint8_t v8t;
	uint16_t v16t;

	switch (parms[parm].type)
	{
	case 0:
		v8s = *parms[parm].val8s;
		if (v8s < ranges[parm][RMAX])
		{
			v8s++;
		}
		*parms[parm].val8s = v8s;
		return v8s;
		break;
	case 1:
		v8t = *parms[parm].val8t;
		if (v8t < ranges[parm][RMAX])
		{
			v8t++;
		}
		*parms[parm].val8t = v8t;
		return v8t;
		break;
	case 2:
		v16t = *parms[parm].val16t;
		if (v16t < ranges[parm][RMAX])
		{
			v16t++;
		}
		*parms[parm].val16t = v16t;
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

	switch (parms[parm].type)
	{
	case 0:
		v8s = *parms[parm].val8s;
		if (v8s > ranges[parm][RMIN])
		{
			v8s--;
		}
		*parms[parm].val8s = v8s;
		return v8s;
		break;
	case 1:
		v8t = *parms[parm].val8t; 
		if (v8t > ranges[parm][RMIN])
		{
			v8t--;
		}
		*parms[parm].val8t = v8t;		
		return v8t;
		break;
	case 2:
		v16t = *parms[parm].val16t;
		if (v16t > ranges[parm][RMIN])
		{
			v16t--;
		}
		*parms[parm].val16t = v16t;
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
		switch (parms[parm].type)
		{
		case 0:
			*parms[parm].val8s = value;
			break;
		case 1:
			*parms[parm].val8t = value;
			break;
		case 2:
			*parms[parm].val16t = value;
			break;
		default:
			break;
		}
		sendParam(parm, value);
//		printf("data: %08X channel: %i\r\n", tgdata.parm, tgdata.channel);
	}
	return value;
}

int32_t cTG::getValue(uint16_t parm)
{
	switch (parms[parm].type)
	{
	case 0:
		return *parms[parm].val8s;
		break;
	case 1:
		return *parms[parm].val8t;
		break;
	case 2:
		return *parms[parm].val16t;
		break;
	default:
		printf("Unknown datatype\r\n");
		return 0;
		break;
	}
}

void cTG::sendParam(uint16_t parm, uint16_t value)
{
	dexed_t tgdata;
	tgdata.channel = mchannel;
	tgdata.instance = mobject_id;
	tgdata.cmd = parm;
	tgdata.val1 = value & 0x7f;
	tgdata.val2 = (value >> 7) & 0x7f;
	tgdata.parm = parm;
	tgdata.data = value;
	queue_add_blocking(&tg_fifo, &tgdata);
}

void cTG::Cutoff(uint8_t freq)
{
}

void cTG::Reso(uint8_t reso)
{
}

void cTG::Rvrb(uint8_t level)
{
}

void cTG::Detune(int8_t detune)
{
}

void cTG::Panning(int8_t pan)
{
}

void cTG::Volume(uint8_t vol)
{
}
