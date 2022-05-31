//void InitMainMenu();

enum menus { MAIN, TG };
enum levels { TOP, LEV1, LEV2 };

#define VALUEWIDTH 37
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
	static void TGMain(uint8_t button);
private:
	static void ShowValue(uint16_t value, int16_t x0, int16_t y0, int16_t w0, int16_t h0, bool colorflag, uint8_t fontsize);
	static uint8_t menu;
	static int8_t selectedTG;
	static bool pflag[3];
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

