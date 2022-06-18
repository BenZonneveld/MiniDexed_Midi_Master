#ifndef _BUTTONS_H
#define _BUTTONS_H
#include <array>

#ifdef __cplusplus
extern "C"
{
#endif

#define BUT1	0
#define BUT2	1
#define BUT3	2
#define BUT4	3
#define BUT5	4
#define BUT6	5
#define BUT7	6
#define BUT8	7

    typedef enum { BTNONE, BTCLICK, BTDBLC, BTHOLD } BTNEVNT;
#define NBR_OF_BUTTONS 8
    class cButtons {
    public:
        cButtons();
        void handleButtons();
        void setCallback(uint8_t mask, void (* callback)(uint8_t button));
        void setDBLCallback(uint8_t mask, void (*callback)(uint8_t button));
        void setLongCallback(uint8_t mask, void (*callback)(uint8_t button));
        void doCallback(uint8_t button); 
        BTNEVNT getState(uint8_t btn);
    private:
        BTNEVNT checkButton(bool buttonval, uint8_t btn); 
        uint8_t state;
        uint64_t last_seen;
        uint8_t cs;
        static std::array<void(*)(uint8_t), 8>buttonCallback;
        static std::array<void(*)(uint8_t), 8>buttonDBLCallback;
        static std::array<void(*)(uint8_t), 8>buttonLongCallback;

        // Button timing variables
        unsigned int debounce;          // ms debounce period to prevent flickering when pressing or releasing the button
        unsigned int DCgap;            // max ms between clicks for a double click event
        unsigned int holdTime;        // ms hold period: how long to wait for press+hold event
        unsigned int longHoldTime;    // ms long hold period: how long to wait for press+hold event
        // Button variables
//            bool buttonVal;   // value read from button
        bool buttonLast[NBR_OF_BUTTONS];  // buffered value of the button's previous state
        bool DCwaiting[NBR_OF_BUTTONS];  // whether we're waiting for a double click (down)
        bool DConUp[NBR_OF_BUTTONS];     // whether to register a double click on next release, or whether to wait and click
        bool singleOK[NBR_OF_BUTTONS];    // whether it's OK to do a single click
        long downTime[NBR_OF_BUTTONS];         // time the button was pressed down
        long upTime[NBR_OF_BUTTONS];           // time the button was released
        bool ignoreUp[NBR_OF_BUTTONS];   // whether to ignore the button release because the click+hold was triggered
        bool waitForUp[NBR_OF_BUTTONS];        // when held, whether to wait for the up event
        bool holdEventPast[NBR_OF_BUTTONS];    // whether or not the hold event happened already
        bool longHoldEventPast[NBR_OF_BUTTONS];// whether or not the long hold event happened already

        BTNEVNT event[8];
    };
#ifdef __cplusplus
}
#endif
#endif