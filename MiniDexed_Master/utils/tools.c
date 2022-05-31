#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
//#include <wchar.h>
#include <pico/stdlib.h>
#include <string.h>

#include "hagl.h"
#include "hagl_hal.h"
#include "font6x13B-ISO8859-15.h"
#include "font9x18B-ISO8859-15.h"
#include "font10x20-ISO8859-15.h"

wchar_t wmessage[64];

void charToWChar(const char* text)
{
    const size_t size = strlen(text) + 1;
    mbstowcs(wmessage, text, size);
}

void hagl_print(const char* charstring, uint16_t x, uint16_t y, uint16_t color, uint8_t fontsize)
{
    charToWChar(charstring);
    switch (fontsize)
    {
    case 0:
        hagl_put_text(wmessage, x, y, color, font6x13B_ISO8859_15);
        break;
    case 1:
        hagl_put_text(wmessage, x, y, color, font9x18B_ISO8859_15);
        break;
    case 2:
        hagl_put_text(wmessage, x, y, color, font10x20_ISO8859_15);
        break;
    default:
        break;
    }
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (((x - in_min) * (out_max - out_min)) / (in_max - in_min)) + out_min;
}

int min(int a, int b)
{
    return (a > b) ? b : a;
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}

