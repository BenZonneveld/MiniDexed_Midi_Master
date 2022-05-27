#ifndef _TG_H
#define _TG_H
#ifdef __cplusplus
extern "C"
{
#endif

class cTG {
public:
	void BankSelect(uint16_t bank);
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