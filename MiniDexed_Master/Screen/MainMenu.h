//void InitMainMenu();

enum menus { MAIN, TG };
enum levels { TOP, LEV1, LEV2 };

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

private:
	static uint8_t menu;
	static uint8_t level;
	static int8_t selectedTG;
};