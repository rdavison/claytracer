#include "opencl_renderer.h"
#include "rtcl.h"
#include "util.h"
#include "motion.h"


void __renderer_opencl_init(struct renderer *renderer)
{
    //rtcl_opencl_info();
    rtcl_init(renderer);
    rtcl_init_buffers();
    rtcl_copy_scene_to_device();
}

void __renderer_opencl_cleanup()
{
    rtcl_cleanup();
}

void __renderer_opencl_generate_rays(struct ray **rays, int *size)
{
    rtcl_generate_rays_kernel_init();
    rtcl_run();
    rtcl_read_rays_from_device(rays, size);
}

void __renderer_opencl_trace_rays(struct pixel **pixel_board, const struct ray *rays, int num_rays, int rays_per_pixel)
{
    rtcl_trace_rays_kernel_init(rays, num_rays, rays_per_pixel);
    rtcl_run();
    rtcl_read_pixel_board(pixel_board);
}

void __renderer_opencl_update_scene()
{
    rtcl_move(MOVE_DIR_FORWARD,0.1); // MOVE FORWARDS 0.5 UNITS PER FRAME
    rtcl_run();
    rtcl_update_scene();
    rtcl_run();
}
