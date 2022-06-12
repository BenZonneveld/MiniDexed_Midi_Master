#ifndef _TG_H
#define _TG_H
#include "pico/util/queue.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern queue_t tg_fifo;
#define VOICEDATA_HEADER 6
#define VOICEDATA_SIZE 162
#define SYSEX_BUFFER	200
#define VNAME_OFFSET	145

#define PARMS	24
#define MAXBANKS 128
#define MAXPATCH 32
#define MAXCHANNEL 17
#define MAXFREQ 128
#define MAXRESO	128
#define MAXREV	128

#define RMIN	0
#define RMAX	1
#define ROFFSET	2

enum TGPARAMS { PNOPARAM,
	PBANK, 
	PPATCH,
	PCHANNEL,
	PFREQ,
	PRESO,
	PVERB,
	PCOMP,
	PSHIFT,
	PTUNE,
	PPAN,
	PVOL,
	PBRANGE,
	PPMODE,
	PMONO,
	PBSTEP,
	PGLISS,
	PTIME,
	PNLOW,
	PNHIGH
};

const int16_t ranges[][3] = {
	{0,0,0}, // PNOPARAM
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
	{ 0, 127,0}, // PVOL
	{ -12,12,0}, // PBRANGE
	{ 0,1,0}, //PPORTA
	{ 0,1,0}, // PMONO
	{0,12,0}, // PBSTEP
	{0,99,0}, // PGLISS
	{0,99,0}, // PTIME
	{0,127,0}, // PNLOW
	{0,127,0} // PNHIGH
};

typedef struct {
	uint8_t channel;
	uint8_t instance;
	uint8_t cmd;
	uint16_t parm;
	int32_t value;
} dexed_t;

typedef struct {
	uint16_t length;
	uint8_t buffer[SYSEX_BUFFER];
} sysex_t;

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
	int32_t parmDown(int16_t parm);
	int32_t parmUp(int16_t parm);
	int32_t setValue(int16_t parm, int32_t value);
	int32_t getValue(int16_t parm);
	void sendParam(int16_t parm);
	void getPatch();
	void getConfig();
	void getBank();
	char* getBankName() { return mbankname; }
	void setBankName(sysex_t sysex);
	void setSysex(sysex_t sysex);
	char* getVoiceName() { return mvoicename; }
	/*void Channel(uint8_t channel);
	void Cutoff(uint8_t freq);
	void Reso(uint8_t reso);
	void Rvrb(uint8_t level);
	void Detune(int8_t detune);
	void Panning(int8_t pan);
	void Volume(uint8_t vol);*/
private:
	void setParm(int16_t parm, int16_t* data);
	uint16_t mobject_id;
	int16_t mbank;
	int16_t mpatch;
	int16_t mchannel;
	int16_t mfreq;
	int16_t mreso;
	int16_t mrev;
	int16_t mcomp;
	int16_t mnote_shift;
	int16_t mtune;
	int16_t mpan;
	int16_t mvol;
	int16_t mpmode;
	int16_t mmono;
	int16_t mrange;
	int16_t mstep;
	int16_t mgliss;
	int16_t mtime;
	int16_t mnote_low;
	int16_t mnote_high;
	char mvoicename[11];
	char mbankname[32];
	uint8_t msysex[162];
	struct s_parms{
		int16_t *val16t;
	} mparms[PARMS];
	unsigned char notes[16]; // Maybe not an array...
};

#ifdef __cplusplus
}
#endif

#endif