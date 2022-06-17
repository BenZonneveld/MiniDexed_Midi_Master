
//#include "MainMenu.h"

#define MAXDISPLAYFILES	9
#define MAXDIRENTRIES 512
#define CARDSYSEXBYFFER 9000
// GUI
#define DIRLEFTPOS 30
#define FBROWSERBC	DARKERGREY

class mCard {
public:
	static void CardMenu(uint8_t button);
	static void ListUp(uint8_t button);
	static void ListDown(uint8_t button);
	static void OpenEntry(uint8_t button);
	static void CloseEntry(uint8_t button);
	static void VoiceHandling(uint8_t button);
	static void BankHandling(uint8_t button);
	static void CartHandling(uint8_t button);
	static void exitCardMenu(uint8_t button);
	static void VoiceUp(uint8_t button);
	static void VoiceDown(uint8_t button);
	static void VoiceSelect(uint8_t button);
	static void ShowVoices(uint8_t button);
private:
	static uint8_t tryMount();
	static uint32_t getDir();
	static void ShowEntries();
	static void ExtractVoiceFromBank();
	static uint8_t mCurrentVoice;
	static char mVoiceNames[32][12];
	static FILINFO mfileEntry[MAXDISPLAYFILES];
	static char mCurrentDir[256];
	static char mCurrentFile[256];
//	static char mFilenames[25][256];
	static uint32_t mFileCount;
	static uint32_t mCurrentEntry;
	static uint32_t mFirstEntryToShow;
	static uint8_t mSysexbuf[CARDSYSEXBYFFER];
};
