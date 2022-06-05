#ifndef _TG_H
#define _TG_H
#include "pico/util/queue.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern queue_t tg_fifo;
#define PARMS	16
#define MAXBANKS 256
#define MAXPATCH 32
#define MAXCHANNEL 16
#define MAXFREQ 128
#define MAXRESO	128
#define MAXREV	128

#define RMIN	0
#define RMAX	1
#define ROFFSET	2

enum TGPARAMS { PBANK, PPATCH, PCHANNEL, PFREQ, PRESO, PVERB, PCOMP, PTRANS, PTUNE, PPAN, PVOL};

const int16_t ranges[][3] = {
	{0,MAXBANKS,1}, // PBANK
	{ 0, MAXPATCH,1}, // PPATCH
	{ 0, MAXCHANNEL,1}, // PCHANNEL
	{ 0, MAXFREQ,0}, // PFREQ
	{ 0, MAXRESO,0}, // PRESO
	{ 0, MAXREV,0}, // PVERB
	{0,1,0}, // PCOMP
	{-8,8,0}, // PTRANS
	{ -99,99,0}, // PTUNE
	{ -63, 63,0}, // PPAN
	{ 0, 127,0} // PVOL
};

typedef struct {
	uint8_t channel;
	uint8_t instance;
	uint8_t cmd;
	uint16_t parm;
	uint8_t val1;
	uint8_t val2;
	uint16_t data;
} dexed_t;

//template <class Obj>
class CountedObj
{
public:
	CountedObj() { ++total_; }
	CountedObj(const CountedObj& obj) { if (this != &obj) ++total_; }
	~CountedObj() { --total_; }

	static size_t showCount() { return total_; }

private:
	static size_t total_;
};

class cTG: public CountedObj {
public:
	cTG();
//	size_t getObjectID() { return mobject_id; }
	int32_t parmDown(uint16_t parm);
	int32_t parmUp(uint16_t parm);
	int8_t setValue(uint16_t parm, int8_t value);
	uint8_t setValue(uint16_t parm, uint8_t value);
	int32_t setValue(uint16_t parm, int32_t value);
	int32_t getValue(uint16_t parm);
	uint8_t getParmType(uint16_t parm) { return parms[parm].type; }
	void sendParam(uint16_t parm, uint16_t value);

	void Patch(uint8_t patch);
	void Channel(uint8_t channel);
	void Cutoff(uint8_t freq);
	void Reso(uint8_t reso);
	void Rvrb(uint8_t level);
	void Detune(int8_t detune);
	void Panning(int8_t pan);
	void Volume(uint8_t vol);
private:
	void setParmType(uint16_t parm, int8_t* data);
	void setParmType(uint16_t parm, uint8_t* data);
	void setParmType(uint16_t parm, uint16_t* data);
	uint16_t mbank;
	uint8_t mpatch;
	uint8_t mchannel;
	uint8_t mobject_id;
	uint8_t mfreq;
	uint8_t mreso;
	uint8_t mrev;
	uint8_t mcomp;
	int8_t mtranspose;
	int8_t mtune;
	int8_t mpan;
	uint8_t mvol;
	struct s_parms{
		uint8_t type;
		int8_t *val8s;
		uint8_t *val8t;
		uint16_t *val16t;
	} parms[PARMS];
	unsigned char notes[16]; // Maybe not an array...
};

#ifdef __cplusplus
}
#endif

#endif