
//#include "MainMenu.h"

#define MAXDISPLAYFILES	9
#define MAXDIRENTRIES 512
#define CARDSYSEXBYFFER 9000
// GUI
#define DIRLEFTPOS 26
#define FBROWSERBC	DARKERGREY
#define FOLDERCOLOR	tft.color565(255,255,200)
#define VOICECOLOR WHITE
#define BANKCOLOR tft.color565(64,127,127)
#define CARTCOLOR tft.color565(100,100,160)

class mCard {
public:
	static void CardMenu(uint8_t button);
	static void ListUp(uint8_t button);
	static void ListDown(uint8_t button);
	static void OpenEntry(uint8_t button);
	static void CloseEntry(uint8_t button);
	static void VoiceHandling();
	static void BankHandling();
	static void CartHandling();
	static void exitCardMenu(uint8_t button);
	static void VoiceUp(uint8_t button);
	static void VoiceDown(uint8_t button);
	static void BankVoiceSelect(uint8_t button);
	static void ExtractAndSendVoiceFromBank(uint8_t button);
	static void SendVoice(uint8_t button);
	static void ShowEntries(uint8_t button);
private:
	static void ShowBankVoices();
	static uint8_t tryMount();
	static uint32_t getDir();
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
	static bool mNeedRefresh;
};
