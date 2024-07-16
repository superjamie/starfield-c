/* https://github.com/sink666/starfield-d/blob/main/source/app.d */

#include <math.h>     // atan
#include <stdbool.h>
#include <stdlib.h>   // rand

#include "SDL.h"

/* macros */

#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif

#define WIDTH      640
#define HEIGHT     480
#define PIXELS     (WIDTH * HEIGHT)
#define NUM_STARS  300
#define SPEED       30
#define FOV         70

/* types */

struct pixel_s {  // was p_screen
	int x;
	int y;
};

struct star_s {  // was: p_world
	double x;
	double y;
	double z;
	double v;  // velocity
};

struct world_s {  // was: stars
	struct star_s *star;
	size_t nstars;
};

enum brightness {
	OFF,
	DIM,
	HALF,
	FULL
};

/* globals */

const Uint32 palette[4] = { 0x00000000, 0x00666666, 0x00A8A8A8, 0x00FFFFFF };
const struct pixel_s midp = { WIDTH / 2, HEIGHT / 2 };

SDL_Window *stars_win;
SDL_Texture *stars_tex;
SDL_Renderer *stars_ren;
Uint32 *pixel_buffer;
const int buf_pitch = sizeof(*pixel_buffer) * WIDTH;
const int buf_stride = WIDTH;
double half_FOV;

/* prototypes */

bool do_sdl_init(void);
void do_sdl_close(void);
bool check_quit(SDL_Event *e);
void change_pixel(int x, int y, Uint32 color);
void draw_square(struct pixel_s *p, int lw, enum brightness b);
void do_effect(struct world_s *world);
void do_render(void);

static double uniform(double min, double max);
static void reset_star(struct star_s *s);
static struct world_s *create_world(size_t nstars);
static void destroy_world(struct world_s *world);

/* functions */

static double uniform(double min, double max) {
	double scale = rand() / (double)RAND_MAX; /* [0, 1.0] */
	return min + scale * (max - min); /* translate and scale */
}

static void reset_star(struct star_s *s) {
	s->x = uniform(-1.0f, 1.0f);
	s->y = uniform(-1.0f, 1.0f);
	s->z = uniform(-1.0f, 0.0000001f);
	s->v = uniform(0.0003f, 0.0019f);
	//SDL_Log("Placing star at %lf,%lf,%lf with v %lf", s->x, s->y, s->z, s->v);
}

static struct world_s *create_world(size_t nstars)
{
	struct world_s *w = calloc(1, sizeof(*w));
	if (w == NULL)
		goto out;

	struct star_s *s = calloc(nstars, sizeof(*s));
	if (s == NULL)
		goto free_world_out;

	w->star = s;
	w->nstars = nstars;

	for (size_t i = 0; i < w->nstars; i++)
		reset_star(&w->star[i]);

	return w;

free_world_out:
	free(w);
out:
	return NULL;
}

static void destroy_world(struct world_s *world)
{
	if (world != NULL) {
		if (world->star != NULL) {
			free(world->star);
		}
		free(world);
	}
}

bool do_sdl_init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) == -1)
		goto init_err;

	stars_win = SDL_CreateWindow("starfield",
				     SDL_WINDOWPOS_CENTERED,
				     SDL_WINDOWPOS_CENTERED,
				     WIDTH, HEIGHT, 0);
	if (stars_win == NULL)
		goto init_err;

	Uint32 flags = SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;
	stars_ren = SDL_CreateRenderer(stars_win, -1, flags);
	if (stars_ren == NULL)
		goto init_err;

	stars_tex = SDL_CreateTexture(stars_ren, SDL_PIXELFORMAT_RGBA32,
				      SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);
	if (stars_tex == NULL)
		goto init_err;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(stars_ren, WIDTH, HEIGHT);

	return true;

init_err:
	SDL_Log("SDL error: %s\n", SDL_GetError());
	return false;
}

void do_sdl_close(void)
{
	SDL_DestroyWindow(stars_win);
	SDL_DestroyTexture(stars_tex);
	SDL_DestroyRenderer(stars_ren);

	SDL_QuitSubSystem(SDL_INIT_VIDEO|SDL_INIT_TIMER);
	SDL_Quit();
}

bool check_quit(SDL_Event *e)
{
	SDL_PollEvent(e);
	if ((e->key.keysym.scancode == SDL_SCANCODE_ESCAPE) ||
	    (e->key.keysym.scancode == SDL_SCANCODE_Q) ||
	    (e->type == SDL_QUIT))
		return true;

	return false;
}

void change_pixel(int x, int y, Uint32 color)
{
	//SDL_Log("    changing pixel %d,%d to 0x%0x", x, y, color);
	size_t index = (buf_stride * y) + x;

	if (index < PIXELS) {
		pixel_buffer[index] = color;
		//SDL_Log("     placed color %x at %d,%d (index %zu)", color, x, y, index);
	}
}

void draw_square(struct pixel_s *p, int lw, enum brightness b)
{
	//SDL_Log("  drawing square at %d,%d brightness %d", p->x, p->y, b);
	int targ_x = p->x + lw;
	int targ_y = p->y + lw;

	for (int j = p->x; j <= targ_x; j++) {
		for (int k = p->y; k <= targ_y; k++) {
			change_pixel(j, k, palette[b]);
			//SDL_Log("   drawing pixel at %d,%d brightness %d", j, k, b);
		}
	}
}

void do_effect(struct world_s *world)
{
	//SDL_Log("do_effect");

	// clear the pixel buffer
	for (size_t pixel = 0; pixel < PIXELS; pixel++)
		pixel_buffer[pixel] = palette[OFF];

	// move stars along z axis, reset if they reach 0
	for (size_t i = 0; i < world->nstars; ++i) {
		struct star_s *star = &world->star[i];
		star->z += star->v;

		// reset if we get close enough
		if (star->z >= 0) {
			reset_star(star);
		}
	}

	// put a pixel on the canvas
	for (size_t i = 0; i < world->nstars; i++) {
		struct star_s *star = &world->star[i];

		enum brightness b = OFF;

		struct pixel_s pixel = {
			(int)((star->x / (star->z * half_FOV)) * midp.x) + midp.x,
			(int)((star->y / (star->z * half_FOV)) * midp.y) + midp.y
		};

		if (star->z > -0.3) {
			b = FULL;
		} else if (star->z > -0.6) {
			b = HALF;
		} else if (star->z > -0.9) {
			b = DIM;
		}

		// reset if oob
		if (pixel.x < 0 || pixel.x >= WIDTH || (pixel.y < 0 || pixel.y >= HEIGHT)) {
			reset_star(star);
		} else {
			draw_square(&pixel, 1, b);
		}
	}
}

void do_render(void)
{
	//SDL_Log("do_render");
	SDL_UpdateTexture(stars_tex, NULL, pixel_buffer, buf_pitch);
	SDL_RenderCopy(stars_ren, stars_tex, NULL, NULL);
	SDL_RenderPresent(stars_ren);
}

int main(void)
{
	half_FOV = atan((FOV * (M_PI / 180)));
	pixel_buffer = calloc(WIDTH * HEIGHT, sizeof(*pixel_buffer));
	if (pixel_buffer == NULL)
		return 1;

	struct world_s *world = NULL;
	world = create_world(NUM_STARS);
	if (world == NULL)
		return 1;

	if (do_sdl_init() == false) {
		destroy_world(world);
		do_sdl_close();
		return 1;
	}

	SDL_Event ev = { 0 };
	while (true) {
		if (check_quit(&ev) == true)
			break;

		do_effect(world);
		do_render();
		SDL_Delay(SPEED);
	}

	destroy_world(world);
	world = NULL;

	do_sdl_close();

	return 0;
}
