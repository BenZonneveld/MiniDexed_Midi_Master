#include <stdlib.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <string.h>

#include "mdma.h"
#include "screen.h"

void InitMainMenu()
{
	screen.print(0, 20, WHITE, "TG 1");
	screen.print(0, 50, WHITE, "TG 2");
	screen.print(0, 70, WHITE, "TG 3");
	screen.print(0, 90, WHITE, "TG 4");

	screen.print(100, 20, WHITE, "TG 5");
	screen.print(100, 50, WHITE, "TG 6");
	screen.print(100, 70, WHITE, "TG 7");
	screen.print(100, 90, WHITE, "TG 8");
}