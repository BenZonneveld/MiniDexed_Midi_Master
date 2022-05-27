#ifndef _SCREEN_H
#define _SCREEN_H
#include "hagl.h"

extern const unsigned char font6x13B_ISO8859_15[];
extern const unsigned char font9x18B_ISO8859_15[];
extern const unsigned char font10x20_ISO8859_15[];
#define BLACK (hagl_color(0,0,0))
#define WHITE (hagl_color(255,255,255))
class cScreen {
public:
	cScreen();
	void print(uint8_t x, uint8_t y, color_t color, const char* msg);
	void cls();
private:
	wchar_t wmessage[128];
};

#endif