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
	uint16_t BankUp();
	uint16_t BankDown();
	uint16_t setBank(uint16_t bank);
	uint8_t PatchUp();
	uint8_t PatchDown();
	uint8_t setPatch(uint8_t patch);
	uint8_t ChannelUp();
	uint8_t ChannelDown();
	uint8_t setChannel(uint8_t channel);
	uint16_t getBank() { return mbank; }
	uint8_t getPatch() { return mpatch; }
	uint8_t getChannel() { return mchannel; }

	void Patch(uint8_t patch);
	void Channel(uint8_t channel);
	void Cutoff(uint8_t freq);
	void Reso(uint8_t reso);
	void Rvrb(uint8_t level);
	void Detune(int8_t detune);
	void Panning(int8_t pan);
	void Volume(uint8_t vol);
private:
	uint16_t mbank;
	uint8_t mpatch;
	uint8_t mchannel;
	uint8_t mfreq;
	uint8_t mreso;
	int8_t mdetune;
	int8_t mpan;
	int8_t mvol;
	unsigned char notes[16]; // Maybe not an array...
};

#ifdef __cplusplus
}
#endif

#endif