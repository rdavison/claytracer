#ifndef __RTCL_RENDERER__
#define __RTCL_RENDERER__

#include "raytracer.h"

#define RENDERER_USE_OPENCL_RENDERER     (1 << 0)
#define RENDERER_USE_SOFTWARE_RENDERER   (1 << 1)

struct renderer {
    int width;
    int height;
    int pitch;
    int depth;
} renderer;

void renderer_init(int recursion_depth, int flags, int width, int height, int pitch);
void renderer_destroy();
void renderer_render();
void renderer_generate_rays(struct ray **rays, int *size);
void renderer_trace_rays(struct pixel **pixel_board, const struct ray *rays, int num_rays, int rays_per_pixel);
void renderer_update_scene();
void renderer_cleanup();

void dump_rays(struct ray *rays, int size);
void dump_pixel_board(struct pixel *pixel_board, int num_rays, int rays_per_pixel);
#endif
