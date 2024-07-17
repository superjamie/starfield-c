#include <stdlib.h>
#include <stdbool.h>

#include <allegro.h>

#include "display.h"

/*
 * TODO
 * look at optimised drawing routines, eg:
 * https://www.allegro.cc/manual/4/api/truecolor-pixel-formats/makecol32
 * https://www.allegro.cc/manual/4/api/drawing-primitives/_putpixel32
 */

/* types */

struct display_context {
	BITMAP *bitmap;
	size_t width;
	size_t height;
	size_t pixel_count;
};

/* static variables */

static struct display_context *ctx;

bool display_start(size_t width, size_t height)
{
	width = 320;
	height = 200;
	ctx = calloc(1, sizeof(*ctx));
	if (ctx == NULL)
		return false;

	ctx->width = width;
	ctx->height = height;
	ctx->pixel_count = (width * height);

	allegro_init();
	install_keyboard();
	install_timer();
	set_gfx_mode(GFX_AUTODETECT, width, height, 0, 0);

	PALETTE pal;
	pal[0].r = pal[0].g = pal[0].b = 0x00;
	pal[1].r = pal[1].g = pal[1].b = 0x66;
	pal[2].r = pal[2].g = pal[2].b = 0xa8;
	pal[3].r = pal[3].g = pal[3].b = 0xff;
	set_palette(pal);
	clear_bitmap(screen);

	ctx->bitmap = create_bitmap(SCREEN_W, SCREEN_H);
	if (ctx->bitmap == NULL) {
		free(ctx);
		return false;
	}
	clear_bitmap(ctx->bitmap);

	return true;
}

void display_stop(void)
{
	if (ctx)
		free(ctx);

	allegro_exit();
}

size_t display_get_width(void)
{
	return ctx->width;
}

size_t display_get_height(void)
{
	return ctx->height;
}

bool display_check_quit(void)
{
	if (keypressed())
		return true;

	return false;
}

void display_draw_pixel(size_t x, size_t y, enum brightness b)
{
	//rectfill(ctx->bitmap, x, y, x + 1, y + 1, palette_color[b]);
	putpixel(ctx->bitmap, x, y, palette_color[b]);
}

void display_flush(void)
{
	blit(ctx->bitmap, screen, 0,0, 0,0, SCREEN_W,SCREEN_H);
}

void display_sleep(size_t ms)
{
	// https://www.allegro.cc/manual/4/api/timer-routines/rest
	//rest(ms);
}

