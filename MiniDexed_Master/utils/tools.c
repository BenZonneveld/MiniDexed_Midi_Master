#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
//#include <wchar.h>
#include <pico/stdlib.h>
#include <string.h>

#include "cdc.h"

wchar_t wmessage[64];

void charToWChar(const char* text)
{
    const size_t size = strlen(text) + 1;
    mbstowcs(wmessage, text, size);
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

