#ifndef __RTCL_H
#define __RTCL_H

#include <OpenCL/opencl.h>
#include "raytracer.h"
#include "renderer.h"

// forward declaration
//cl_mem input;                   // device memory used for th input array
//cl_mem output;                  // device memory used for the output array

void rtcl_init(struct renderer *renderer);
void rtcl_run();
void rtcl_validate();
void rtcl_cleanup();
void rtcl_opencl_info();
void rtcl_generate_rays_kernel_init();
void rtcl_read_rays_from_device(struct ray **rays, int *size);
void rtcl_trace_rays_kernel_init(const struct ray *rays, int num_rays, int rays_per_pixel);
void rtcl_read_pixel_board(struct pixel **pixel_board);
void rtcl_update_scene();
void rtcl_opencl_kernel_workgroup_info();
void rtcl_select_kernel(const char *kernel_name);
void rtcl_copy_scene_to_device();

struct rtcl {
    cl_device_id device_id;         // compute device id
    cl_context context;             // compute context
    cl_command_queue commands;      // compute command queue
    cl_program program;             // compute program
    cl_kernel kernel;               // compute kernel

    struct renderer *renderer;
    char kernel_name[256];
    struct camera camera;
    size_t dimension;
} rtcl;

#endif
