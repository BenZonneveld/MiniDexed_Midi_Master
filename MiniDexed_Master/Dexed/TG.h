#ifndef _TG_H
#define _TG_H
#ifdef __cplusplus
extern "C"
{
#endif

#define MAXBANKS 256
#define MAXPATCH 32
#define MAXCHANNEL 16

class cTG {
public:
	cTG();
	static uint16_t BankUp();
	static uint16_t BankDown();
	static uint16_t setBank(uint16_t bank);
	static uint8_t PatchUp();
	static uint8_t PatchDown();
	static uint8_t setPatch(uint8_t patch);
	static uint8_t ChannelUp();
	static uint8_t ChannelDown();
	static uint8_t setChannel(uint8_t channel);
	static uint16_t getBank() { return mbank; }
	static uint8_t getPatch() { return mpatch; }
	static uint8_t getChannel() { return mchannel; }

	void Patch(uint8_t patch);
	void Channel(uint8_t channel);
	void Cutoff(uint8_t freq);
	void Reso(uint8_t reso);
	void Rvrb(uint8_t level);
	void Detune(int8_t detune);
	void Panning(int8_t pan);
	void Volume(uint8_t vol);
private:
	static uint16_t mbank;
	static uint8_t mpatch;
	static uint8_t mchannel;
	static uint8_t mfreq;
	static uint8_t mreso;
	static uint8_t mrev;
	static int8_t mdetune;
	static int8_t mpan;
	static int8_t mvol;
	unsigned char notes[16]; // Maybe not an array...
};

#ifdef __cplusplus
}
#endif

#endif