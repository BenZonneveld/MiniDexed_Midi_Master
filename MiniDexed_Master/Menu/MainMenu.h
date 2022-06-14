//void InitMainMenu();
#include "Adafruit_GFX.h"
#include "Adafruit_SPITFT.h"
#include "TG.h"
#ifndef MAIN_MENU_H
#define MAIN_MENU_H
#define VALUEWIDTH 37
#define BUTTONHEIGHT 18
#define VALUEPOS	59
#define TOPPOT	0
#define MIDPOT	1
#define BOTPOT	2

#define MIDI_CC_BANK_SELECT_MSB          0       // TODO
#define MIDI_CC_MODULATION               1
#define MIDI_CC_VOLUME                   7
#define MIDI_CC_PAN_POSITION            10
#define MIDI_CC_BANK_SELECT_LSB         32
#define MIDI_CC_BANK_SUSTAIN            64
#define MIDI_CC_RESONANCE               71
#define MIDI_CC_CUTOFF					74
#define MIDI_CC_REVERB				    91
#define MIDI_CC_DETUNE		            94
#define MIDI_CC_ALL_SOUND_OFF           120
#define MIDI_CC_ALL_NOTES_OFF           123
//extern const GFXfont FreeSans9pt7b;
//extern const GFXfont FreeSans12pt7b;
//extern const GFXfont FreeSans18pt7b;
//extern const GFXfont FreeSans24pt7b;
//extern const GFXfont Open_Sans_Regular_8;

enum POS { POSA = 38, POSB = 60, POSC = 82, POSD = 104};
//extern Adafruit_SPITFT tft;
enum menulvl { M_MAIN = 0,
	M_TG,
	M_TG_MIDI,
	M_TG_FILT,
	M_TG_TUNE,
	M_TG_OUT,
	M_TG_PITCH,
	M_TG_PORTA,
	M_DEXED,
	M_FX1,
	M_FX2,
	M_ROUTING,
	M_CARD
};

typedef struct {
//	const uint8_t menulvl;
	uint8_t prev;
	uint16_t potparams[3];
	bool potflags[3];
	uint16_t showvalue[4];
	struct {
		char name[8];
		uint16_t parameter;
		void (*callback)(uint8_t cbparam);
		void (*dblcallback)(uint8_t cbparam);
		void (*longcallback)(uint8_t cbparam);
	} button[8];
} s_menu;

typedef struct {
	bool comp_enable;
	bool reverb_enable;
	uint8_t verbsize;
	uint8_t highdamp;
	uint8_t lowdamp;
	uint8_t lowpass;
	uint8_t diffusion;
	uint8_t level;
	uint8_t mastervolume;
} s_fx;

extern s_fx fx_settings;

class cMenu {
public:
	void Init();
	//	void ClearNeedUpdate() { menuNeedFlush = false; }
	//	bool NeedUpdate() { return menuNeedFlush; }
	void Show() { mainmenu(0); }
	uint8_t currentMenu() { return menu; }
	static void menuBack(uint8_t button);
	static void mainmenu(uint8_t cbparam);
	static void selectTGcb(uint8_t cbparam);
	static void selectTG(uint8_t button);
	static void Midi(uint8_t button);
	static void TGFilter(uint8_t button);
	static void TGTune(uint8_t button);
	static void TGOut(uint8_t button);
	static void TGPitch(uint8_t button);
	static void TGPorta(uint8_t button);
	static void ParmSelect(uint8_t button);
	static void ParmToggle(uint8_t button);
	static void ParmPot(uint8_t channel);
	static void TGEnable(uint8_t button);
	static void setDexedParm(uint16_t parm, int32_t val, uint8_t instance);
	static void showTGInfo(int16_t param);
	static void showDexedInfo(int16_t param);
	static void ShowValue(uint16_t param);
	static void Dexed(uint8_t cbparam);
	static void FX1(uint8_t cbparam);
	static void FX2(uint8_t cbparam);
	static void Routing(uint8_t cbparam);
	static void Card(uint8_t cbparam);
private:
	static void buildMenu(uint8_t men);
	static void ShowButtonText(uint8_t button);
	static void setButtonCallback(uint8_t button,uint16_t param, void (*callback)(uint8_t button));
	static void clearCallbacks();
	static void setPotCallback(uint8_t channel, uint16_t param);
	static void resetPotCB(uint8_t channel);
	static uint8_t menu;
	static uint8_t currentTG;
	static bool pflag[3];
	static uint16_t potparam[3];
	static uint16_t bparam[8];
	static int16_t parampos[PLAST];
	static bool TGEnabled[8];
};

#define BLACK 0x0000
#define DARKERGREY 0x18E3
#define DARKGREY 0x4208 
#define GREY 0x8430
#define LIGHTGREY 0xBDF7 
#define WHITE 0xFFFF

#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define ORANGE 0xFC00

#endif
