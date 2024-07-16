#ifndef STARS_DISPLAY
#define STARS_DISPLAY

#include <stdbool.h>

enum brightness {
	OFF,
	DIM,
	HALF,
	FULL,
	BRIGHTNESS_MAX
};

bool display_start(size_t width, size_t height);
void display_stop(void);

size_t display_get_width(void);
size_t display_get_height(void);

bool display_check_quit(void);
void display_draw_pixel(size_t x, size_t y, enum brightness b);
void display_flush(void);

#endif /* STARS_DISPLAY */
