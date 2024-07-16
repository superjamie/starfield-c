/* https://github.com/sink666/starfield-d/blob/main/source/app.d */

#include <math.h>     // atan
#include <stdbool.h>
#include <stdlib.h>   // rand
#include <time.h>     // nanosleep

#include "SDL.h"

#include "display.h"

/* macros */

#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif

#define NUM_STARS  300
#define SPEED       30
#define FOV         70

/* types */

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

/* globals */

static int midpoint_x;
static int midpoint_y;
static double half_FOV;

/* prototypes */

static void do_effect(struct world_s *world);

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

static void do_effect(struct world_s *world)
{
	// clear the screen
	for (size_t x = 0; x < display_get_width(); x++)
		for (size_t y = 0; y < display_get_height(); y++)
			display_draw_pixel(x, y, OFF);

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

		int px = ((star->x / (star->z * half_FOV)) * midpoint_x) + midpoint_x;
		int py = ((star->y / (star->z * half_FOV)) * midpoint_y) + midpoint_y;

		if (star->z > -0.3) {
			b = FULL;
		} else if (star->z > -0.6) {
			b = HALF;
		} else if (star->z > -0.9) {
			b = DIM;
		}

		// reset if oob
		if (px < 0 || px >= (int)display_get_width() ||
		    py < 0 || py >= (int)display_get_height() ) {
			reset_star(star);
		} else {
			display_draw_pixel(px, py, b);
		}
	}
}

int main(void)
{
	half_FOV = atan((FOV * (M_PI / 180)));

	struct world_s *world = NULL;
	world = create_world(NUM_STARS);
	if (world == NULL)
		return 1;

	if (display_start(640, 480) == false) {
		destroy_world(world);
		display_stop();
		return 1;
	}
	midpoint_x = display_get_width() / 2;
	midpoint_y = display_get_height() / 2;

	// 30FPS
	const struct timespec sleep_time = {
		.tv_sec = 0,
		.tv_nsec = (30 * 1000 * 1000)
	};

	while (true) {
		if (display_check_quit() == true)
			break;

		do_effect(world);
		display_flush();
		nanosleep(&sleep_time, NULL);
	}

	destroy_world(world);
	world = NULL;

	display_stop();

	return 0;
}

