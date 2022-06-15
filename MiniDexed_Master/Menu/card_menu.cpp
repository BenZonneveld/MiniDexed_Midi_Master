#include "f_util.h"
#include "ff.h"
#include "hw_config.h"

#include "string.h"
#include "MDMA.h"
#include "MainMenu.h"
#include "card_menu.h"
#include "midicore.h"

#define PROGMEM
//#include "fonts/FreeSans9pt7b.h"
//#include "fonts/FreeSans12pt7b.h"
//#include "fonts/FreeSans18pt7b.h"
//#include "fonts/FreeSans24pt7b.h"
#include "fonts/RobotoMono12.h"
#include "fonts/RobotoCondensed.h"
#include "fonts/OpenSansBoldCondensed16.h"

char cMenu::mCard::mCurrentDir[256];
char cMenu::mCard::mCurrentFile[256];
//char cMenu::mCard::mFilenames[25][256];
uint32_t cMenu::mCard::mFileCount;
uint32_t cMenu::mCard::mCurrentEntry;
FILINFO cMenu::mCard::mfileEntry[MAX_FILES];

void cMenu::mCard::CardMenu(uint8_t button)
{
//    mCard cardmenu;
    menu = M_CARD;
//    buildMenu(M_CARD);
    if (tryMount() != 0) menuBack(0);

    clearCallbacks();
    buttons.setCallback(BUT8, &cMenu::menuBack);
    setButtonCallback(BUT8,PNOPARAM, &cMenu::menuBack);
    buttons.setCallback(BUT2, &cMenu::mCard::ListUp);
    buttons.setCallback(BUT1, &cMenu::mCard::ListDown);
    buttons.setCallback(BUT5, &cMenu::mCard::OpenEntry);
    buttons.setCallback(BUT6, &cMenu::mCard::CloseEntry);

    getDir();
    printf("mFileCount after return: %i\n", mFileCount);
    if (mFileCount > 0) ShowEntries();
}

uint8_t cMenu::mCard::tryMount()
{
    sd_card_t* pSD = sd_get_by_num(0);
    tft.writeFillRect(0,0,160,128,LIGHTGREY);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    printf("FRESULT= %i\n", fr);
    if (fr != FR_OK)
    {
        tft.writeFillRect(0, 0, 160, 128, BLACK);
        return fr;
    }
    tft.writeFillRect(0, 0, 160, 128, DARKERGREY);
    mCurrentDir[0] = '\0';
    mCurrentEntry = 0;
    return FR_OK;
}

uint32_t cMenu::mCard::getDir()
{
    uint32_t i = 0, j = 0;
    FRESULT res;
    FILINFO MyFileInfo;
    DIR MyDirectory;

    res = f_opendir(&MyDirectory, mCurrentDir);
    if (res == FR_OK) {
        i = strlen(mCurrentDir);
        for (;;) {
            res = f_readdir(&MyDirectory, &MyFileInfo);
            if (res != FR_OK || MyFileInfo.fname[0] == 0) break;
            if (MyFileInfo.fname[0] == '.') continue;
            do {
                i++;
            } while (MyFileInfo.fname[i] != 0x2E);
            if (j < MAX_FILES) {
                mfileEntry[j] = MyFileInfo;
                j++;
                mFileCount = j;
            }
            i = 0;
        }
    }
    return j;
}

void cMenu::mCard::ShowEntries()
{
    tft.setFont(&Roboto_Condensed);
    tft.writeFillRect(0, 0, 160, 128, DARKERGREY);
    for (size_t i = 0; i < mFileCount; i++)
    {
        if ((mfileEntry[i].fattrib & AM_DIR) == 0x10)
        {
            tft.setTextColor(BLUE);
        }
        else {
            tft.setTextColor(WHITE);
        }

        //printf("%s size %i\n", mfileEntry[i].fname, mfileEntry[i].fsize);
        tft.setCursor(DIRLEFTPOS - 24, 9 + 12 * i);
        
        char lowercase[256];
        for (size_t lc = 0; lc <= strlen(mfileEntry[i].fname); lc++)
        {
            lowercase[lc] = tolower(mfileEntry[i].fname[lc]);
        }

        if (strstr(lowercase, ".syx") != NULL)
        {
            switch (mfileEntry[i].fsize)
            {
            case 163:
                tft.print("V");
                break;
            case 4104:
                tft.print("B");
                break;
            case 8208:
                tft.print("C");
                break;
            default:
                break;
            }
        }

        tft.setCursor(DIRLEFTPOS, 9 + 12 * i);
        if (i == mCurrentEntry)
        {
            tft.setTextColor(BLACK);
            tft.writeFillRect(DIRLEFTPOS, 12 * i, 160 - DIRLEFTPOS, 12, WHITE);
        }
//        else {
//            tft.setTextColor(WHITE);
//        }
        
        tft.print(mfileEntry[i].fname);
    }
}

void cMenu::mCard::OpenEntry(uint8_t button)
{
    char path[256];
    sprintf(path, "%s", mCurrentDir);
    // Entry is a directory
    if ((mfileEntry[mCurrentEntry].fattrib & AM_DIR) == 0x10)
    {
        sprintf(mCurrentDir, "%s/%s", path, mfileEntry[mCurrentEntry].fname);
        getDir();
        mCurrentEntry = 0;
        ShowEntries();
        return;
    }
    char lowercase[256];
    for (size_t lc = 0; lc <= strlen(mfileEntry[mCurrentEntry].fname); lc++)
    {
        lowercase[lc] = tolower(mfileEntry[mCurrentEntry].fname[lc]);
    }

    if (strstr(lowercase, ".syx") == NULL) return; // Not a sysex file

    FIL file;              ///< Current Open file

    sprintf(mCurrentFile, "%s/%s", path, mfileEntry[mCurrentEntry].fname);

    uint8_t sysexbuf[10000];

    FRESULT fr = f_open(&file, mCurrentFile, FA_READ);
    if ( fr == FR_OK) {
        f_read(&file, sysexbuf, mfileEntry[mCurrentEntry].fsize, NULL);
     }
    f_close(&file);
    if (fr == FR_OK)
    {
        uint32_t sysexsize = 0;
        while (sysexsize < mfileEntry[mCurrentEntry].fsize)
        {
            printf("%02X, ", sysexbuf[sysexsize]);
            if (sysexbuf[sysexsize] == 0xF7)
            {
//                sysexsize++;
                printf("\n End at %i bytes", sysexsize);
                break;
            }
            if ((sysexsize + 1) % 16 == 0) printf("\n");
            sysexsize++;
        }
        printf("\n");

        switch (sysexsize)
        {
        case 162:
            sendToAllPorts(sysexbuf, sysexsize);
            //        tft.print("V");
            break;
        case 4104:
            //        tft.print("B");
            break;
        case 8208:
            //        tft.print("C");
            break;
        default:
            break;
        }
    }
}

void cMenu::mCard::CloseEntry(uint8_t button)
{
    char* last = strrchr(mCurrentDir, '/');
    if (last != NULL)
    {
        mCurrentDir[last-mCurrentDir] = '\0';
        mCurrentEntry = 0;
        getDir();
        ShowEntries();
    }
}

void cMenu::mCard::ListUp(uint8_t button)
{
    if (mCurrentEntry < (mFileCount-1) )
    {
        mCurrentEntry++;
    }
    ShowEntries();
}

void cMenu::mCard::ListDown(uint8_t button)
{
    if (mCurrentEntry > 0)
    {
        mCurrentEntry--;
    }
    ShowEntries();
}