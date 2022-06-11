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

//extern const GFXfont FreeSans9pt7b;
//extern const GFXfont FreeSans12pt7b;
//extern const GFXfont FreeSans18pt7b;
//extern const GFXfont FreeSans24pt7b;
//extern const GFXfont Open_Sans_Regular_8;

enum POS { POS0 = 0, POS1 = 34, POS2 = 68, POS3 = 102 , POSA = 38, POSB = 60, POSC = 82, POSD = 104};
extern Adafruit_SPITFT tft;

class cMenu {
public:
	void Init();
	void ClearNeedUpdate() { menuNeedFlush = false; }
	bool NeedUpdate() { return menuNeedFlush; }
	void Show() { mainmenu(); }
	static void ShowButtonText(uint8_t button);
	static void menuBack(uint8_t button);
	static void menuBack();
	static void mainmenu();
	static void selectTG();
	static void selectTG(uint8_t button);
	static void Midi(uint8_t button);
	static void TGFilter(uint8_t button);
	static void TGTune(uint8_t button);
	static void TGOut(uint8_t button);
	static void TGPitch(uint8_t button);
	static void CompToggle();
	static void ParmSelect(uint8_t button);
	static void ParmToggle(uint8_t button);
	static void ParmPot(uint8_t channel);
	static void showTGInfo(int16_t param);
	static void handleSysex(sysex_t raw_sysex);
	static void TGEnable(uint8_t button);
private:
	static void setDexedParm(uint16_t parm, int32_t val, uint8_t instance);
	static void ShowValue(int32_t param, int16_t x0, int16_t y0, int16_t w0, int16_t h0, bool colorflag, uint8_t fontsize);
	static void setButtonCallbackWithParam(uint8_t button,int16_t param, int16_t pos, void (*callback)(uint8_t button));
	static void setButtonParm(uint8_t button, int16_t param, int16_t pos, bool haslongpress);
	static void clearCallbacks();
	static void setPotCallback(uint8_t channel, int16_t param, int16_t pos);
	static void resetPotCB(uint8_t channel);
	static uint8_t menu;
	static uint8_t prev_menu;
	static int8_t currentTG;
	static bool pflag[3];
	static int16_t potpos[3];
	static int16_t potparam[4];
	static int16_t bparam[8];
	static int16_t parampos[8];
	static bool menuNeedFlush;
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
