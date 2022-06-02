//void InitMainMenu();

#define VALUEWIDTH 37
#define TOPPOT	2
#define MIDPOT	1
#define BOTPOT	0

enum POS { POS0 = 0, POS1 = 34, POS2 = 68, POS3 = 102 };

class cMenu {
public:
	void Init();
	void Show() { mainmenu(); }
	static void ShowButtonText(uint8_t button);
	static void menuBack();
	static void mainmenu();
	static void selectTG();
	static void selectTG(uint8_t button);
	static void Midi(uint8_t button);
	static void TGMain(uint8_t button);
	static void TGTune(uint8_t button);
	static void CompToggle();
	static void ParmSelect(uint8_t button);
	static void ParmPot(uint8_t channel);
private:
	static void ShowValue(int32_t param, int16_t x0, int16_t y0, int16_t w0, int16_t h0, bool colorflag, uint8_t fontsize);
	static void setButtonCallback(uint8_t button,uint16_t param, int8_t pos, void (*callback)(uint8_t button));
	static void setButtonParm(uint8_t button, uint16_t param, int8_t pos, bool haslongpress);
	static void clearButtonCB(uint8_t button);
	static void setPotCallback(uint8_t channel, uint16_t param, int8_t pos);
	static void resetPotCB(uint8_t channel);
	static uint8_t menu;
	static uint8_t prev_menu;
	static int8_t currentTG;
	static bool pflag[3];
	static int8_t potpos[3];
	static uint16_t potparam[4];
	static int16_t bparam[8];
	static int8_t parampos[8];
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

