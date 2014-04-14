#ifndef __RTCL_OPENCL_RENDERER__
#define __RTCL_OPENCL_RENDERER__

#include "raytracer.h"

void __renderer_opencl_init();
void __renderer_opencl_render();
void __renderer_opencl_generate_rays(struct ray **rays, int *size);
void __renderer_opencl_trace_rays(struct pixel **pixel_board, const struct ray *rays, int num_rays, int rays_per_pixel);
void __renderer_opencl_cleanup();
void __renderer_opencl_update_scene();

#endif
