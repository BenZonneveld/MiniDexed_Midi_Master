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
#define SYSEX_BUFFER	204
#define VNAME_OFFSET	145

#define MAXBANKS 127
#define MAXPATCH 31
#define MAXCHANNEL 16
#define MAXFREQ 99
#define MAXRESO	99
#define MAXREV	99

enum TGPARAMS {
	PNOPARAM,
	PBANK,
	PPATCH,
	PCHANNEL,
	PFREQ,
	PRESO,
	PVERB,
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
	PNHIGH,
	FCOMP_EN,
	FREV_EN,
	FSIZE,
	FLOWDAMP,
	FHIGHDAMP,
	FLOWPASS,
	FDIFF,
	FRLEVEL,
	PLAST
};

enum sysfunc {
	F_NOPARAM = 0 ,
	F_BANK = 78,
	F_PATCH = 79,
	F_CHANNEL = 80,
	F_FREQ = 81,
	F_RESO = 82,
	F_VERB = 83,
	F_SHIFT = 84,
	F_TUNE = 85,
	F_PAN = 86,
	F_VOL = 76,
	F_BRANGE = 65,
	F_PMODE = 67,
	F_MONO = 64,
	F_BSTEP = 66,
	F_GLISS = 68,
	F_TIME = 69,
	F_NLOW = 87,
	F_NHIGH = 88,
	F_COMP_EN = 89,
	F_REV_EN = 90,
	F_SIZE = 91,
	F_LOWDAMP = 92,
	F_HIGHDAMP = 93,
	F_LOWPASS = 94,
	F_DIFF = 95,
	F_RLEVEL = 96,
};

typedef struct {
	int16_t	low;
	int16_t	high;
	uint8_t offset;
	char	shortname[4];
	uint8_t sysex_function;
} tg_params;

extern tg_params tg_parameters[];
extern uint8_t function_param[];

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
	void setParm(int16_t parm, int16_t value);
	uint16_t mobject_id;
	char mvoicename[11];
	char mbankname[32];
//	uint8_t msysex[162];
	int16_t mparms[PLAST];
	unsigned char notes[16]; // Maybe not an array...
};

#ifdef __cplusplus
}
#endif

#endif