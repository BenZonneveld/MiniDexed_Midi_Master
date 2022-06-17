#include "f_util.h"
#include "ff.h"
#include "hw_config.h"

#include "string.h"
#include "MDMA.h"
#include "MainMenu.h"
#include "card_menu.h"
#include "midicore.h"
#include "tools.h"

#define PROGMEM
//#include "fonts/FreeSans9pt7b.h"
//#include "fonts/FreeSans12pt7b.h"
//#include "fonts/FreeSans18pt7b.h"
//#include "fonts/FreeSans24pt7b.h"
//#include "fonts/RobotoMono12.h"
#include "fonts/RobotoCondensed.h"
#include "fonts/RobotoCondensed10.h"
//#include "fonts/OpenSansBoldCondensed16.h"

char cMenu::mCard::mCurrentDir[256];
char cMenu::mCard::mCurrentFile[256];
//char cMenu::mCard::mFilenames[25][256];
uint32_t cMenu::mCard::mFileCount;
uint32_t cMenu::mCard::mCurrentEntry;
uint32_t cMenu::mCard::mFirstEntryToShow; 
FILINFO cMenu::mCard::mfileEntry[MAXDISPLAYFILES];
char cMenu::mCard::mVoiceNames[32][12];
uint8_t cMenu::mCard::mCurrentVoice;
uint8_t cMenu::mCard::mSysexbuf[CARDSYSEXBYFFER];

void cMenu::mCard::CardMenu(uint8_t button)
{
//    mCard cardmenu;
    menu = M_CARD;
//    buildMenu(M_CARD);
    if (tryMount() != 0) menuBack(0);

    clearCallbacks();
    buttons.setCallback(BUT8, &cMenu::mCard::exitCardMenu);
    buttons.setDBLCallback(BUT8, &cMenu::mCard::exitCardMenu);
    buttons.setLongCallback(BUT8, &cMenu::mCard::exitCardMenu);
//    setButtonCallback(BUT8,PNOPARAM, &cMenu::menuBack);
    buttons.setCallback(BUT4, &cMenu::mCard::ListUp);
    buttons.setCallback(BUT1, &cMenu::mCard::ListDown);
    buttons.setDBLCallback(BUT4, &cMenu::mCard::ListUp);
    buttons.setDBLCallback(BUT1, &cMenu::mCard::ListDown);
    buttons.setLongCallback(BUT4, &cMenu::mCard::ListUp);
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
    tft.setCursor(10, 72);
    tft.println("Mounting SD Card");

    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);

    if (fr != FR_OK)
    {
        tft.writeFillRect(0, 0, 160, 128, BLACK);
        return fr;
    }
    tft.writeFillRect(0, 0, 160, 128, FBROWSERBC);
    mCurrentDir[0] = '\0';
    mCurrentEntry = 0;
    mFirstEntryToShow = 0;
    return FR_OK;
}

void cMenu::mCard::exitCardMenu(uint8_t button)
{
    sd_card_t* pSD = sd_get_by_num(0);
    tft.writeFillRect(0, 0, 160, 128, BLACK);
    f_unmount(pSD->pcName);
    menuBack(PNOPARAM);
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
    char name[32];
    char lowercase[256];
 
    tft.setFont(&Roboto_Condensed);


    tft.setTextColor(WHITE);
//tft.writeFillRect()
    tft.writeFillRect(12, 0,44, 12, BUTTONCOLOR);
    tft.setCursor(14, 10);
    tft.print("5 Open");
    tft.writeFillRect(63, 0, 46, 12, BUTTONCOLOR);
    tft.setCursor(65, 10);
    tft.print("6 Close");
    tft.writeFillRect(118, 0, 42, 12, BUTTONCOLOR);
    tft.setCursor(120, 10);
    tft.print("8 Back");
    tft.setTextColor(WHITE);

    for (size_t i = 0; i < MAXDISPLAYFILES; i++)
    {
        // Check for valid filename
        if (strlen(mfileEntry[i].fname) > 0)
        {
            // Mark directories with a color
            if ((mfileEntry[i].fattrib & AM_DIR) == 0x10)
            {
                tft.setTextColor(BLUE);
            }
            else {
                tft.setTextColor(WHITE);
            }

            // Mark sysex files with a size based indication of their type
            tft.setCursor(DIRLEFTPOS - 12, 24 + 12 * i);
            for (size_t lc = 0; lc <= strlen(mfileEntry[i].fname); lc++)
            {
                lowercase[lc] = tolower(mfileEntry[i].fname[lc]);
            }
            tft.writeFillRect(DIRLEFTPOS - 12, 14 + 12 * i, 12, 12, FBROWSERBC);
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

            // Mark the selected entry
            tft.setCursor(DIRLEFTPOS, 24 + 12 * i);
            if (i == (mCurrentEntry - mFirstEntryToShow))
            {
                tft.setTextColor(BLACK);
                tft.writeFillRect(DIRLEFTPOS, 14 + 12 * i, 160 - DIRLEFTPOS, 12, WHITE);
            }
            else {
                tft.writeFillRect(DIRLEFTPOS, 14 + 12 * i, 160 - DIRLEFTPOS, 12, FBROWSERBC);
            }

            // Finally print the filename
            sprintf(name, "%.15s", mfileEntry[i].fname);
            tft.print(name);
        }
        else {
            // Fill in empty filenames
            tft.writeFillRect(DIRLEFTPOS - 12, 14 + 12 * i, 160 - DIRLEFTPOS + 12, 12, FBROWSERBC);
        }
    }

    tft.writeFillRect(0, 12, 12, 116, BLACK);
    tft.fillTriangle(5, 127, 0, 119, 10, 119, WHITE); // Downward arrow
    tft.fillTriangle(5, 13, 0, 21, 10, 21, WHITE); // Up Arrow
    tft.drawFastVLine(11, 13, 116, GREY);

    // 110
    if (mFileCount > MAXDISPLAYFILES)
    {
        float step = 95.0f / (mFileCount - (MAXDISPLAYFILES - 1));
        //        printf("fileCOunt %i\t First Entry to Show %i, step %f\n", mFileCount, mFirstEntryToShow, step);

        tft.writeFillRect(1, 23 + step * mFirstEntryToShow, 9, map(step, 0, 95, 2, 108), LIGHTGREY);
    }
    else {
        tft.writeFillRect(1, 23, 9, 95, LIGHTGREY);
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
                sysexsize++;
                break;
            }
            sysexsize++;
        }
        printf("Sysex Size: %i\n", sysexsize);
        switch (sysexsize)
        {
        case 163:
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
        if (mFirstEntryToShow < (mFileCount - MAXDISPLAYFILES))
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

// Voice and Bank Handling
void cMenu::mCard::VoiceUp(uint8_t button)
{
    if (mCurrentVoice < MAXPATCH)
    {
        mCurrentVoice++;
    }
}

void cMenu::mCard::VoiceDown(uint8_t button)
{
    if (mCurrentVoice > 0)
    {
        mCurrentVoice--;
    }
}

void cMenu::mCard::VoiceSelect(uint8_t button)
{}

void cMenu::mCard::VoiceHandling(uint8_t button)
{
    char voiceName[12];
    memset(voiceName, 0, 12);
    memcpy(voiceName, mSysexbuf + DATA_HEADER + VNAME_OFFSET, 10);
    printf("Voicename: %s\n", voiceName);

    uint32_t checksum = 0;
    for (size_t i = DATA_HEADER; i < VOICEDATA_SIZE-1; i++)
    {
        checksum += mSysexbuf[i];
    }
    checksum = (~checksum + 1) & 0x7f; 
    printf("Checksum %02X Calculated checksum: %02X\n", mSysexbuf[VOICEDATA_SIZE-1],checksum);
    //    sendToAllPorts(sysexbuf, sysexsize);
}

void cMenu::mCard::BankHandling(uint8_t button)
{
    tft.setTextColor(WHITE);
    tft.writeFillRect(0, 0, 160, 128, DARKERGREY);

//    tft.setFont(&Roboto_Bold_10);
    printf("BankHandling\n");
    for (size_t v = 0; v < MAXPATCH; v++)
    {
        memset(mVoiceNames[v], 0, 10);
        memcpy(mVoiceNames[v], mSysexbuf + DATA_HEADER + (v*BANKVOICE_SIZE) + BANKVNAME_OFFSET, 10);
    }

    char myname[14];
    for (uint8_t r = 0; r < 8; r++)
        {
       uint8_t patch = r;

       if (patch != mCurrentVoice)
       {
           tft.setFont(&Roboto_Condensed_10);
           tft.writeFillRect(0, 13 + r * 12, 79, 12, WHITE);
       }
       else {
           tft.setFont(&Roboto_Condensed);
           tft.writeFillRect(0, 13 + r * 12, 79, 12, FBROWSERBC);
       }
       tft.setCursor(0, 23+r * 12);
       sprintf(myname, "%2i %s", patch+1, mVoiceNames[patch]);
       tft.print(myname);
       if (patch < 24)
       {
           tft.setFont(&Roboto_Condensed_10);
           tft.setCursor(80, 23 + r * 12);
           sprintf(myname, "%2i %s", patch + 9, mVoiceNames[patch + 8]);
           tft.print(myname);
       }
    }
//    ExtractVoiceFromBank();
//    sendToAllPorts(sysexbuf, sysexsize);
}

void cMenu::mCard::ExtractVoiceFromBank()
{
    uint8_t voicebuffer[163];
    uint8_t packedvoice[128];
    uint16_t v = mCurrentVoice;
    memcpy(packedvoice, mSysexbuf + DATA_HEADER + (v * BANKVOICE_SIZE), BANKVOICE_SIZE);
    uint8_t i = 0, o = DATA_HEADER;
    uint8_t temp;
    voicebuffer[0] = 0xF0;
    voicebuffer[1] = 0x43;
    voicebuffer[2] = 0x00 | currentTG;
    voicebuffer[3] = 0;
    voicebuffer[4] = 0x01;
    voicebuffer[5] = 0x1B;
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
        voicebuffer[7 + o++] = (temp >> 3); // Osc Detune

        temp = packedvoice[i++];
        voicebuffer[o++] = (temp & 0x3); // AMS
        voicebuffer[o++] = (temp >> 2); // Key Vel Sens
        voicebuffer[o++] = packedvoice[i++]; // Output level;
        temp = packedvoice[i++];
        voicebuffer[o++] = temp & 0x1; // Osc mode
        voicebuffer[o++] = (temp >> 1); // Freq coarse
        voicebuffer[o++] = packedvoice[i++]; // Freq fine;
    }
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

    // From byte 6 (message start is byte 0) add all the data values until message length - 2 
    // (so do not include checksum and final F7 byte), which gives you "sum"
    //And then compute checksum by checksum = (NOT sum + 1) AND 0x7F where NOT is a bitwise invert
    // and the AND constrains the checksum to a seven bit value.
    uint32_t checksum = 0;
    for (size_t i = DATA_HEADER; i < VOICEDATA_SIZE - 1; i++)
    {
        checksum += mSysexbuf[i];
    }
    checksum = (~checksum + 1) & 0x7f;

    voicebuffer[o++] = checksum; // Checksum
    voicebuffer[o++] = 0xF7;
    memset(mSysexbuf, 0, CARDSYSEXBYFFER);
    memcpy(mSysexbuf, voicebuffer, 163);

}

void cMenu::mCard::CartHandling(uint8_t button)
{
//    sendToAllPorts(sysexbuf, sysexsize);
}