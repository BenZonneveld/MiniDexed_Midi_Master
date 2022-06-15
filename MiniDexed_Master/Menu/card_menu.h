
//#include "MainMenu.h"

#define MAX_FILES	10

	// GUI
#define DIRLEFTPOS 30

class mCard {
public:
	static void CardMenu(uint8_t button);
	static void ListUp(uint8_t button);
	static void ListDown(uint8_t button);
	static void OpenEntry(uint8_t button);
	static void CloseEntry(uint8_t button);
private:
	static uint8_t tryMount();
	static uint32_t getDir();
	static void ShowEntries();

	static FILINFO mfileEntry[MAX_FILES];
	static char mCurrentDir[256];
	static char mCurrentFile[256];
//	static char mFilenames[25][256];
	static uint32_t mFileCount;
	static uint32_t mCurrentEntry;
};
