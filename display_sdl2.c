#include <stdbool.h>

#include "SDL.h"

#include "display.h"

/* types */

struct display_context {
	SDL_Window *win;
	SDL_Texture *tex;
	SDL_Renderer *ren;
	Uint32 *pixel_buffer;
	size_t width;
	size_t height;
	size_t pixel_count;
	int pixel_buffer_pitch;
};

/* static variables */

/* RGBA8888 aka RGBA32 */
static const Uint32 palette[BRIGHTNESS_MAX] = {
	0x00000000,  // OFF
	0x00666666,  // DIM
	0x00A8A8A8,  // HALF
	0x00FFFFFF   // FULL
};

static struct display_context *ctx;
static SDL_Event e = { 0 };

/* public functions */

bool display_start(size_t width, size_t height)
{
	ctx = calloc(1, sizeof(*ctx));
	if (ctx == NULL)
		return false;

	ctx->pixel_buffer = calloc(width * height, sizeof(*(ctx->pixel_buffer)));
	if (ctx->pixel_buffer == NULL) {
		free(ctx);
		return false;
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1)
		goto init_err;

	ctx->win = SDL_CreateWindow("starfield",
				     SDL_WINDOWPOS_CENTERED,
				     SDL_WINDOWPOS_CENTERED,
				     width, height, 0);
	if (ctx->win == NULL)
		goto init_err;

	Uint32 flags = SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;
	ctx->ren = SDL_CreateRenderer(ctx->win, -1, flags);
	if (ctx->ren == NULL)
		goto init_err;

	ctx->tex = SDL_CreateTexture(ctx->ren, SDL_PIXELFORMAT_RGBA32,
				      SDL_TEXTUREACCESS_TARGET, width, height);
	if (ctx->tex == NULL)
		goto init_err;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(ctx->ren, width, height);

	ctx->width = width;
	ctx->height = height;
	ctx->pixel_count = width * height;
	ctx->pixel_buffer_pitch = sizeof(*(ctx->pixel_buffer)) * width;
	return true;

init_err:
	SDL_Log("SDL error: %s\n", SDL_GetError());
	return false;
}

void display_stop(void)
{
	SDL_DestroyWindow(ctx->win);
	SDL_DestroyTexture(ctx->tex);
	SDL_DestroyRenderer(ctx->ren);

	SDL_QuitSubSystem(SDL_INIT_VIDEO|SDL_INIT_TIMER);
	SDL_Quit();
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
	SDL_PollEvent(&e);
	if ((e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) ||
	    (e.key.keysym.scancode == SDL_SCANCODE_Q) ||
	    (e.type == SDL_QUIT))
		return true;

	return false;
}

void display_draw_pixel(size_t x, size_t y, enum brightness b)
{
	for (size_t px = x; px <= x + 1; px++) {
		for (size_t py = y; py <= y + 1; py++) {

			// width was: stride
			size_t index = (ctx->width * py) + px;

			if (index < ctx->pixel_count)
				ctx->pixel_buffer[index] = palette[b];
		}
	}
}

void display_flush(void)
{
	SDL_UpdateTexture(ctx->tex, NULL, ctx->pixel_buffer,
			  ctx->pixel_buffer_pitch);
	SDL_RenderCopy(ctx->ren, ctx->tex, NULL, NULL);
	SDL_RenderPresent(ctx->ren);
}
