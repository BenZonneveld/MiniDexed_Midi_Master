//void InitMainMenu();

#include "Adafruit_GFX.h"

extern const GFXfont FreeSans9pt7b;
extern const GFXfont FreeSans12pt7b;
extern const GFXfont FreeSans18pt7b;
extern const GFXfont FreeSans24pt7b;

enum menus { MAIN, TG };
enum levels { TOP, LEV1, LEV2 };

#define BANKPOT 2
#define PATCHPOT 1
#define CHANPOT 0
class cMenu {
public:
	void Init();
	static void ShowMenu();
	static void menuLevelUp();
	static void selectTG(uint8_t button);
	static void BankSelect(uint8_t button);
	static void PatchSelect(uint8_t button);
	static void ChannelSelect(uint8_t button);
	static void Midi(uint8_t button);
	static void BankSelectPot();
	static void PatchSelectPot();
	static void ChannelSelectPot();
	static void mainmenu();
private:
	static void ShowValue(uint16_t value, int16_t x0, int16_t y0, int16_t w0, int16_t h0);
	static uint8_t menu;
	static int8_t selectedTG;
};