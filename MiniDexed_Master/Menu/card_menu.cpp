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
uint32_t cMenu::mCard::mFirstEntryToShow; 
FILINFO cMenu::mCard::mfileEntry[MAXDISPLAYFILES];
char cMenu::mCard::mVoiceNames[32][12];
uint8_t cMenu::mCard::mCurrentVoice;
uint8_t cMenu::mCard::mSysexbuf[9000];

void cMenu::mCard::CardMenu(uint8_t button)
{
//    mCard cardmenu;
    menu = M_CARD;
//    buildMenu(M_CARD);
    if (tryMount() != 0) menuBack(0);

    clearCallbacks();
    buttons.setCallback(BUT8, &cMenu::menuBack);
    buttons.setDBLCallback(BUT8, &cMenu::menuBack);
    buttons.setLongCallback(BUT8, &cMenu::menuBack);
//    setButtonCallback(BUT8,PNOPARAM, &cMenu::menuBack);
    buttons.setCallback(BUT2, &cMenu::mCard::ListUp);
    buttons.setCallback(BUT1, &cMenu::mCard::ListDown);
    buttons.setDBLCallback(BUT2, &cMenu::mCard::ListUp);
    buttons.setDBLCallback(BUT1, &cMenu::mCard::ListDown);
    buttons.setLongCallback(BUT2, &cMenu::mCard::ListUp);
    buttons.setLongCallback(BUT1, &cMenu::mCard::ListDown);
    buttons.setCallback(BUT5, &cMenu::mCard::OpenEntry);
    buttons.setCallback(BUT6, &cMenu::mCard::CloseEntry);

    getDir();
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
    mFirstEntryToShow = 0;
    return FR_OK;
}

uint32_t cMenu::mCard::getDir()
{
    uint32_t j = 0;
    FRESULT res;
    FILINFO MyFileInfo;
    DIR MyDirectory;
 
    // Clear old fileinfo
    memset(mfileEntry, 0, MAXDISPLAYFILES*sizeof(FILINFO));
    
    mFileCount = 0;
    res = f_opendir(&MyDirectory, mCurrentDir);
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&MyDirectory, &MyFileInfo);
            if (res != FR_OK || MyFileInfo.fname[0] == 0) break;
            if (MyFileInfo.fname[0] == '.') continue;   // Hide . files


            if (mFileCount < MAXDIRENTRIES && mFileCount >= mFirstEntryToShow) {
                if (j < MAXDISPLAYFILES)
                {
                    mfileEntry[j] = MyFileInfo;
                    //                    printf("%s\n", mfileEntry[j].fname);
                }
                j++;
            }

            mFileCount++;
        }
    }
    return j;
}

void cMenu::mCard::ShowEntries()
{ 
    tft.setFont(&Roboto_Condensed);

    for (size_t i = 0; i < MAXDISPLAYFILES; i++)
    {
        if (strlen(mfileEntry[i].fname) > 0)
        {
            if ((mfileEntry[i].fattrib & AM_DIR) == 0x10)
            {
                tft.setTextColor(BLUE);
            }
            else {
                tft.setTextColor(WHITE);
            }

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
            if (i == (mCurrentEntry - mFirstEntryToShow))
            {
                tft.setTextColor(BLACK);
                tft.writeFillRect(DIRLEFTPOS, 12 * i, 160 - DIRLEFTPOS, 12, WHITE);
            }
            else {
                tft.writeFillRect(DIRLEFTPOS, 12 * i, 160 - DIRLEFTPOS, 12, DARKERGREY);
            }
            tft.print(mfileEntry[i].fname);
        }
        else {
            tft.writeFillRect(DIRLEFTPOS-24, 12 * i, 160 - DIRLEFTPOS + 24, 12, DARKERGREY);
        }
    }
}

void cMenu::mCard::OpenEntry(uint8_t button)
{
    char path[256];
    uint32_t entry = mCurrentEntry - mFirstEntryToShow;
    sprintf(path, "%s", mCurrentDir);
    // Entry is a directory
    if ((mfileEntry[entry].fattrib & AM_DIR) == 0x10)
    {
        sprintf(mCurrentDir, "%s/%s", path, mfileEntry[entry].fname);
        printf("OpenEntry %s/%s\n", path, mfileEntry[entry].fname);
        mFirstEntryToShow = 0;
        getDir();
        mCurrentEntry = 0;
//        tft.writeFillRect(0, 0, 160, 128, DARKERGREY);
        ShowEntries();
        return;
    }
    char lowercase[256];
    for (size_t lc = 0; lc <= strlen(mfileEntry[entry].fname); lc++)
    {
        lowercase[lc] = tolower(mfileEntry[entry].fname[lc]);
    }

    if (strstr(lowercase, ".syx") == NULL) return; // Not a sysex file

    FIL file;              ///< Current Open file

    sprintf(mCurrentFile, "%s/%s", path, mfileEntry[entry].fname);

    FRESULT fr = f_open(&file, mCurrentFile, FA_READ);
    if ( fr == FR_OK) {
        f_read(&file, mSysexbuf, mfileEntry[entry].fsize, NULL);
     }
    f_close(&file);
    if (fr == FR_OK)
    {
        uint32_t sysexsize = 0;
        while (sysexsize < mfileEntry[entry].fsize)
        {
            if (mSysexbuf[sysexsize] == 0xF7)
            {
                break;
            }
            sysexsize++;
        }
        printf("Sysex Size: %i\n", sysexsize++);
        switch (sysexsize)
        {
        case 162:
            VoiceHandling(8);
            break;
        case 4104:
            BankHandling(8);
            break;
        case 8208:
            CartHandling(8);
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
        mFirstEntryToShow = 0;
        getDir();
//        tft.writeFillRect(0, 0, 160, 128, DARKERGREY);
        ShowEntries();
    }
}

void cMenu::mCard::ListUp(uint8_t button)
{
    if (mCurrentEntry < (mFileCount - 1))
    {
        mCurrentEntry++;
    }
    if (mCurrentEntry > 5)
    {
        if (mFirstEntryToShow < (mFileCount - 5))
        {
            mFirstEntryToShow++;
            getDir();
        }
    }
    ShowEntries();
}

void cMenu::mCard::ListDown(uint8_t button)
{
    if (mCurrentEntry > 0)
    {
        mCurrentEntry--;
    }
    if (mFirstEntryToShow > 0)
    {
        mFirstEntryToShow--;
        getDir();
    }
    ShowEntries();
}

void cMenu::mCard::VoiceHandling(uint8_t button)
{
    char voiceName[12];
    memset(voiceName, 0, 12);
    memcpy(voiceName, mSysexbuf + DATA_HEADER + VNAME_OFFSET, 10);
    printf("Voicename: %s\n", voiceName);
    printf("Byte 155: %02X\n", mSysexbuf + DATA_HEADER + 155);
    //    sendToAllPorts(sysexbuf, sysexsize);
}

void cMenu::mCard::BankHandling(uint8_t button)
{

    printf("BankHandling\n");
    //for (size_t i = 0; i < 9000; i++)
    //{
    //    printf("%02X, ", mSysexbuf[i]);
    //    if (mSysexbuf[i] == 0xF7) break;
    //    if ((i + 1) % 16 == 0) printf("\n");
    //}
    //printf("\n");

    char voiceName[12];
    memset(voiceName, 0, 12);
    for (size_t v = 0; v < MAXPATCH; v++)
    {
        memcpy(voiceName, mSysexbuf + DATA_HEADER + (v*BANKVOICE_SIZE) + BANKVNAME_OFFSET, 10);
        printf("Voice[%i]: %s\n",v+1, voiceName);
    }
    ExtractVoiceFromBank();
//    sendToAllPorts(sysexbuf, sysexsize);
}

void cMenu::mCard::ExtractVoiceFromBank()
{
    uint8_t voicebuffer[162];
    uint8_t packedvoice[128];
    uint16_t v = mCurrentVoice;
    memcpy(packedvoice, mSysexbuf + DATA_HEADER + (v * BANKVOICE_SIZE), BANKVOICE_SIZE);
    uint8_t i = 0, o = DATA_HEADER;
    uint8_t temp;
    while (i<128)
    {
        // Parse data for the 6 operators
        for (size_t osc = 0; osc < 6; osc++)
        {
            voicebuffer[o++] = packedvoice[i++]; // r1
            voicebuffer[o++] = packedvoice[i++]; // r2
            voicebuffer[o++] = packedvoice[i++]; // r3
            voicebuffer[o++] = packedvoice[i++]; // r4
            voicebuffer[o++] = packedvoice[i++]; // l1
            voicebuffer[o++] = packedvoice[i++]; // l2
            voicebuffer[o++] = packedvoice[i++]; // l3
            voicebuffer[o++] = packedvoice[i++]; // l4
            voicebuffer[o++] = packedvoice[i++]; // bp
            voicebuffer[o++] = packedvoice[i++]; // ld
            voicebuffer[o++] = packedvoice[i++]; // rd
            temp = packedvoice[i++];
            voicebuffer[o++] = temp & 0x3;      // scl left curve
            voicebuffer[o++] = (temp >> 2) & 0x3; // scl right curve
            temp = packedvoice[i++];
            voicebuffer[o++] = temp & 0x7; // OSC Rate scale
// Special case
            voicebuffer[7+o++] = (temp >> 3); // Osc Detune

            temp = packedvoice[i++];
            voicebuffer[o++] = (temp & 0x3); // AMS
            voicebuffer[o++] = (temp >> 2); // Key Vel Sens
            voicebuffer[o++] = packedvoice[i++]; // Output level;
            temp = packedvoice[i++];
            voicebuffer[o++] = temp & 0x1; // Osc mode
            voicebuffer[o++] = (temp >> 1); // Freq coarse
            voicebuffer[o++] = packedvoice[i++]; // Freq fine;
        }
        printf("i == %i\n", i);
        voicebuffer[o++] = packedvoice[i++]; // PE R1
        voicebuffer[o++] = packedvoice[i++]; // PE R2
        voicebuffer[o++] = packedvoice[i++]; // PE R3
        voicebuffer[o++] = packedvoice[i++]; // PE R4
        voicebuffer[o++] = packedvoice[i++]; // PE L1
        voicebuffer[o++] = packedvoice[i++]; // PE L2
        voicebuffer[o++] = packedvoice[i++]; // PE L3
        voicebuffer[o++] = packedvoice[i++]; // PE L4
        voicebuffer[o++] = packedvoice[i++]; // Algorithm
        temp = packedvoice[i++];
        voicebuffer[o++] = temp & 0x7; // Feedback
        voicebuffer[o++] = (temp >> 3); // Key Sync
        voicebuffer[o++] = packedvoice[i++]; // LFO Speed
        voicebuffer[o++] = packedvoice[i++]; // LFO Delay
        voicebuffer[o++] = packedvoice[i++]; // LF PT Mod Dep
        voicebuffer[o++] = packedvoice[i++]; // LF AM Mod Dep
        temp = packedvoice[i++];
        voicebuffer[o++] = temp & 0x1; // LF Sync 
        voicebuffer[o++] = (temp >> 1) & 0x7; // LF Wave
        voicebuffer[o++] = (temp >> 4) & 0x7; // LF PT Mod Sns
        voicebuffer[o++] = packedvoice[i++]; // Transpose
        printf("i == %i\n", i);
        voicebuffer[o++] = packedvoice[i++]; // Voice name 1
        voicebuffer[o++] = packedvoice[i++]; // Voice name 2
        voicebuffer[o++] = packedvoice[i++]; // Voice name 3
        voicebuffer[o++] = packedvoice[i++]; // Voice name 4
        voicebuffer[o++] = packedvoice[i++]; // Voice name 5
        voicebuffer[o++] = packedvoice[i++]; // Voice name 6
        voicebuffer[o++] = packedvoice[i++]; // Voice name 7
        voicebuffer[o++] = packedvoice[i++]; // Voice name 8
        voicebuffer[o++] = packedvoice[i++]; // Voice name 9
        voicebuffer[o++] = packedvoice[i++]; // Voice name 10

        printf("i == %i\n", i);
        printf("o == %i\n", o- DATA_HEADER);

        i = 128;
    }
    printf("\n");
}

void cMenu::mCard::CartHandling(uint8_t button)
{
//    sendToAllPorts(sysexbuf, sysexsize);
}