#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <string.h>

#include "hagl_hal.h"
#include "hagl.h"
#include "font6x13B-ISO8859-15.h"
#include "font9x18B-ISO8859-15.h"
#include "font10x20-ISO8859-15.h"

#include "Screen.h"

cScreen::cScreen()
{
	hagl_init();
}

void cScreen::print(uint8_t x, uint8_t y, color_t color, const char* msg)
{
	color_t green = hagl_color(0, 255, 0);
	const size_t size = strlen(msg) + 1;
	mbstowcs(wmessage, msg, size);

	hagl_put_text(wmessage, x, y, color, font6x13B_ISO8859_15);
}

void cScreen::cls()
{
	hagl_clear_screen();
}