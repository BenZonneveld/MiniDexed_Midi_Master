#include <stdlib.h>
#include <pico/stdlib.h>

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

