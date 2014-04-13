#include "renderer.h"
#include "opencl_renderer.h"
#include "software_renderer.h"
#include "util.h"


void renderer_init(int depth, int flags, int width, int height, int pitch)
{
    // init renderer struct
    renderer.width = width;
    renderer.height = height;
    renderer.pitch = pitch;
    renderer.depth = depth;

    // run command
    __renderer_opencl_init(&renderer);
}

void renderer_destroy()
{
    // nothing to destroy
}

void renderer_renderer2(struct pixel **pixel_board)
{
    // Need two buffers:
    //   - RAY_BUFFER => can never be larger than sizeof(struct ray)*num_pixels*rays_per_pixel
    //   - PIXEL_BUFFER => can never be larger than sizeof(struct pixel)*num_pixels
    // 
    // 1. Generate primary rays => save to RAY_BUFFER
    // 2. Trace rays
    //     return list of intersection distoances
    // 3. Generate secondary rays
    // 4. Trace secondary rays
/*


ray_buffer[] : rays_per_pixel * num_pixels * sizeof(struct ray)
pix_buffer[] : num_pixels * sizeof(struct pixel)

rays = cl_getRays()



--------------------------------------------
devic






















*/
    // 
}

void renderer_render(struct pixel **pixel_board)
{
    struct ray *rays = NULL;
    int num_rays;

    renderer_generate_rays(&rays, &num_rays);
    //dump_rays(rays, num_rays);

    int rays_per_pixel = 1; // TODO: make this settable elsewhere

    renderer_trace_rays(pixel_board, rays, num_rays, rays_per_pixel);
    //dump_pixel_board(*pixel_board, num_rays, rays_per_pixel);

    free(rays);
}

void renderer_trace_rays(struct pixel **pixel_board, const struct ray *rays, int num_rays, int rays_per_pixel)
{
    __renderer_opencl_trace_rays(pixel_board, rays, num_rays, rays_per_pixel);
}

void renderer_generate_rays(struct ray **rays, int *size)
{
    __renderer_opencl_generate_rays(rays, size);
}

void renderer_cleanup()
{
    __renderer_opencl_cleanup();
}

void dump_pixel_board(struct pixel *pixel_board, int num_rays, int rays_per_pixel)
{
    int width = renderer.width;
    for(int i = 0; i < num_rays / rays_per_pixel; i++) {
        int x = i % width;
        int y = i / width;
        printf("(%d, %d) => %2.2f, %2.2f, %2.2f, %2.2f\n", 
            x,
            y,
            pixel_board[i].color.s[0], 
            pixel_board[i].color.s[1], 
            pixel_board[i].color.s[2], 
            pixel_board[i].color.s[3]);
    }
}

void dump_rays(struct ray *rays, int size)
{
    for(int i = 0; i < size; i++) {
        int x = i % renderer.width;
        int y = renderer.height > 1 ? i / renderer.width : 0;
        printf("(%02i, %02i) <= ray.position(%5.2f, %5.2f, %5.2f) + ray.direction(%5.2f, %5.2f, %5.2f)\n",
            x, y, rays[i].position.s[0], rays[i].position.s[1], rays[i].position.s[2], rays[i].direction.s[0], rays[i].direction.s[1], rays[i].direction.s[2]);
    }
}
