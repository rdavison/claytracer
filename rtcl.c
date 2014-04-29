#include "rtcl.h"
#include "util.h"
#include "compile.h"
#include "raytracer.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define DATA_SIZE (1024)

static int err;                 // error code returned from api calls
static size_t global;           // global domain size for our calculation
static size_t local;            // local domain size for our calculation

cl_float3 *h_output_rays_position;
cl_float3 *h_output_rays_direction;

struct buffers {
    cl_mem rays_positions;
    cl_mem rays_directions;
    cl_mem pixel_board;
    cl_mem scene_buffer;
    cl_mem scene_update_positions;
    cl_mem lights_buffer;
} buffers;


void rtcl_init(struct renderer *renderer)
{
    srand(time(NULL));

    strcpy(rtcl.kernel_name, "INVALID");
    rtcl.num_scene_objects = 301;
    rtcl.recursion_depth = 5;
    rtcl.renderer = renderer;
    rtcl.num_pixels = renderer->width * renderer->height;
    rtcl.dimension = 1;
    rtcl.camera.position.s[v_X] = 0.0f;
    rtcl.camera.position.s[v_Y] = 0.0f;
    rtcl.camera.position.s[v_Z] = 0.0f;
    rtcl.camera.direction.s[v_X] = 0.0f;
    rtcl.camera.direction.s[v_Y] = 0.0f;
    rtcl.camera.direction.s[v_Z] = -1.0f;
    rtcl.camera.screen_width = rtcl.renderer->width;
    rtcl.camera.screen_height = rtcl.renderer->height;
    rtcl.camera.focal_length = 60.0;
    rtcl.num_pixels = rtcl.renderer->width * rtcl.renderer->height;

    
    char *source;
    const char *source_filename = "cl_kernels.cl";

    // Connect to a compute device
    int gpu = 1; // TODO make this more robust
    err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &rtcl.device_id, NULL);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to create a device group!\n");
        exit(EXIT_FAILURE);
    }

    // Create compute context
    rtcl.context = clCreateContext(0, 1, &rtcl.device_id, NULL, NULL, &err);
    if(!rtcl.context) {
        printf( "Error: Failed to create a compute context!\n");
        exit(EXIT_FAILURE);
    }

    // Create a command queue
    rtcl.commands = clCreateCommandQueue(rtcl.context, rtcl.device_id, 0, &err);
    if(!rtcl.commands) {
        printf( "Error: Failed to create a command queue!\n");
        exit(EXIT_FAILURE);
    }

    // Read in the kernel source code
    read_file(&source, source_filename);

    // Create the compute program from the source buffer
    rtcl.program = clCreateProgramWithSource(rtcl.context, 1, (const char **) &source, NULL, &err);
    if(!rtcl.program) {
        printf( "Error: Failed to create compute program!\n");
        exit(EXIT_FAILURE);
    }

    // Build the program executable
    err = clBuildProgram(rtcl.program, 0, NULL, NULL, NULL, NULL);
    if(err != CL_SUCCESS) {
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(rtcl.program, rtcl.device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char log[log_size];
        
        // Get the log
        clGetProgramBuildInfo(rtcl.program, rtcl.device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        printf("%s\n", log);
//        size_t len;
//        char buffer[2048];
//        printf( "Error: Failed to bulid program executable!\n");
//        clGetProgramBuildInfo(rtcl.program, rtcl.device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
//        printf( "%s\n", buffer);
//        exit(1);
    }
}
void rtcl_init_buffers()
{
    // SCENE BUFFER
    buffers.scene_buffer = clCreateBuffer(rtcl.context, CL_MEM_READ_ONLY, sizeof(cl_float16) * rtcl.num_scene_objects * 2, NULL, NULL);
    if(!buffers.scene_buffer) {
        printf("Error: Failed to allocate scene buffer!\n");
        exit(1);
    }

    // RAY POSITIONS
    buffers.rays_positions = clCreateBuffer(rtcl.context, CL_MEM_WRITE_ONLY, sizeof(cl_float3) * rtcl.num_pixels, NULL, NULL);
    if(!buffers.rays_positions) {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }

    // RAY DIRECTIONS
    buffers.rays_directions = clCreateBuffer(rtcl.context, CL_MEM_WRITE_ONLY, sizeof(cl_float3) * rtcl.num_pixels, NULL, NULL);
    if(!buffers.rays_directions) {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }

    // PIXEL BOARD
    buffers.pixel_board = clCreateBuffer(rtcl.context, CL_MEM_WRITE_ONLY, sizeof(cl_float4) * rtcl.num_pixels, NULL, NULL);
    if(!buffers.pixel_board) {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }

    // SCENE UPDATE POSITIONS
    buffers.scene_update_positions = clCreateBuffer(rtcl.context, CL_MEM_READ_ONLY, sizeof(cl_float3) * rtcl.num_scene_objects, NULL, NULL);
    if(!buffers.pixel_board) {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }

    // SCENE UPDATE POSITIONS
    buffers.lights_buffer = clCreateBuffer(rtcl.context, CL_MEM_READ_ONLY, sizeof(cl_float3) * rtcl.num_lights, NULL, NULL);
    if(!buffers.pixel_board) {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }
}

void rtcl_copy_scene_to_device()
{
    struct plane plane = {
        .position = { 0.0f, -2.0f, 0.0f },
        .normal = {0.0f, 1.0f, 0.0f },
        .color = {0.2f, 0.2f, 0.2f, 1.0f },
        .material_type = MIRROR
    };

    struct plane left = {
        .position = { -8.0f, 0.0f, 0.0f },
        .normal = {1.0f, 0.0f, 0.0f },
        .color = {1.0f, 0.5f, 0.3f, 1.0f },
        .material_type = MATTE
    };

    struct plane right = {
        .position = { 8.0f, 0.0f, 0.0f },
        .normal = {-1.0f, 0.0f, 0.0f },
        .color = {0.3f, 0.5f, 1.0f, 1.0f },
        .material_type = MATTE
    };

    struct plane back = {
        .position = { 0.0f, 0.0f, -8.0f },
        .normal = {0.0f, 0.0f, 1.0f },
        .color = {0.3f, 1.0f, 0.5f, 1.0f },
        .material_type = MATTE
    };

    struct sphere light = {
        .position = { 0.0f, 0.0f, -10.0f },
        .radius = 0.1,
        .color = { 1.0f, 1.0f, 1.0f, 1.0f },
        .material_type = LIGHT
    };

    struct llist *llist = (struct llist *)calloc(1, sizeof(struct llist));
    struct sphere *spheres = (struct sphere *)calloc(rtcl.num_scene_objects-1, sizeof(struct sphere));
    //llist_append(llist, compile_plane(&back));
    llist_append(llist, compile_plane(&plane));
    llist_append(llist, compile_plane(&left));
    llist_append(llist, compile_plane(&right));
    llist_append(llist, compile_sphere(&light));
    for(int i = 0; i < rtcl.num_scene_objects-1; i++) {
        float x, y, z, r, g, b, d;
        x = (float)rand() / (float)RAND_MAX;
        y = (float)rand() / (float)RAND_MAX;
        z =-(float)rand() / (float)RAND_MAX;
        r = (float)rand() / (float)RAND_MAX;
        g = (float)rand() / (float)RAND_MAX;
        b = (float)rand() / (float)RAND_MAX;
        d = (float)rand() / (float)RAND_MAX+1;
        spheres[i].position.s[0] = x*20.f-10.f;
        spheres[i].position.s[1] = y*20.f-10.f;
        spheres[i].position.s[2] = -z*33.f-43.f;
        spheres[i].radius = d;
        //spheres[i].material_type = (float)rand() / (float)RAND_MAX < 0.5 ? MATTE : MIRROR;
        spheres[i].material_type = MIRROR;
        spheres[i].color.s[0] = spheres[i].material_type == MIRROR ? 1 : r;
        spheres[i].color.s[1] = spheres[i].material_type == MIRROR ? 1 : g;
        spheres[i].color.s[2] = spheres[i].material_type == MIRROR ? 1 : b;
        spheres[i].color.s[3] = 1.0f;
        llist_append(llist, compile_sphere(spheres + i));
    }

    cl_float16 *h_scene_buffer = (cl_float16 *)malloc(sizeof(cl_float16)*llist->size*2); // multiply by 2 because each object is 2

    struct node *curr = llist->head;
    int i = 0;
    int j = i+1;
    while(curr != NULL) {
        h_scene_buffer[i] = curr->upper;
        h_scene_buffer[j] = curr->lower;

        i+=2;
        j+=2;
        curr = curr->next;
    }

    err = clEnqueueWriteBuffer(rtcl.commands, buffers.scene_buffer, CL_TRUE, 0, sizeof(cl_float16) * rtcl.num_scene_objects * 2, h_scene_buffer, 0, NULL, NULL);
    if(err != CL_SUCCESS) {
        printf("Error: Failed to write scene buffer to device! %d\n", err);
        exit(1);
    }

    llist_delete(llist);
    free(h_scene_buffer);
}

void rtcl_generate_rays_kernel_init()
{
    // select the kernel
    rtcl_select_kernel("generate_rays");

    h_output_rays_position = (cl_float3 *)malloc(rtcl.renderer->width * rtcl.renderer->height * sizeof(cl_float3));
    h_output_rays_direction = (cl_float3 *)malloc(rtcl.renderer->width * rtcl.renderer->height * sizeof(cl_float3));

    // ZERO OUT THE ARRAY // TODO IS THIS NECESSARY?
    for(int i = 0; i < rtcl.num_pixels; i++) {
        h_output_rays_position[i].s[0] = 0.0f;
        h_output_rays_position[i].s[1] = 0.0f;
        h_output_rays_position[i].s[2] = 0.0f;
    }

    for(int i = 0; i < rtcl.num_pixels; i++) {
        h_output_rays_direction[i].s[0] = 0.0f;
        h_output_rays_direction[i].s[1] = 0.0f;
        h_output_rays_direction[i].s[2] = 0.0f;
    }

    // write buffers to device
    err = clEnqueueWriteBuffer(rtcl.commands, buffers.rays_positions, CL_TRUE, 0, sizeof(cl_float3) * rtcl.num_pixels, h_output_rays_position, 0, NULL, NULL);
    if(err != CL_SUCCESS) {
        printf("Error: Failed to write to source array! %d\n", err);
        exit(1);
    }

    err = clEnqueueWriteBuffer(rtcl.commands, buffers.rays_directions, CL_TRUE, 0, sizeof(cl_float3) * rtcl.num_pixels, h_output_rays_direction, 0, NULL, NULL);
    if(err != CL_SUCCESS) {
        printf("Error: Failed to write to source array! %d\n", err);
        exit(1);
    }


    // Set the arguments to our compute kernel
    err = 0;
    err |= clSetKernelArg(rtcl.kernel, 0, sizeof(rtcl.camera.position), &rtcl.camera.position);
    err |= clSetKernelArg(rtcl.kernel, 1, sizeof(rtcl.camera.direction), &rtcl.camera.direction);
    err |= clSetKernelArg(rtcl.kernel, 2, sizeof(rtcl.camera.focal_length), &rtcl.camera.focal_length);
    err |= clSetKernelArg(rtcl.kernel, 3, sizeof(rtcl.camera.screen_width), &rtcl.camera.screen_width);
    err |= clSetKernelArg(rtcl.kernel, 4, sizeof(rtcl.camera.screen_height), &rtcl.camera.screen_height);
    err |= clSetKernelArg(rtcl.kernel, 5, sizeof(cl_mem), &buffers.rays_positions);
    err |= clSetKernelArg(rtcl.kernel, 6, sizeof(cl_mem), &buffers.rays_directions);
    err |= clSetKernelArg(rtcl.kernel, 7, sizeof(unsigned int), &rtcl.num_pixels);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to set kernel arguments! %d\n", err);
        exit(1);
    }
}

void rtcl_read_rays_from_device(struct ray **rays, int *size)
{
    struct ray *tmp_rays = (struct ray *)malloc(sizeof(struct ray) * rtcl.num_pixels);
    cl_float3 *ray_pos = (cl_float3 *)malloc(sizeof(cl_float3) * rtcl.num_pixels);
    cl_float3 *ray_dir = (cl_float3 *)malloc(sizeof(cl_float3) * rtcl.num_pixels);
    //cl_float3 ray_dir[rtcl.num_pixels];

    // copy rays on device to host
    err = clEnqueueReadBuffer(rtcl.commands, buffers.rays_positions, CL_TRUE, 0, sizeof(cl_float3) * rtcl.num_pixels, ray_pos, 0, NULL, NULL);
    err = clEnqueueReadBuffer(rtcl.commands, buffers.rays_directions, CL_TRUE, 0, sizeof(cl_float3) * rtcl.num_pixels, ray_dir, 0, NULL, NULL);

    for(int i = 0; i  < rtcl.num_pixels; i++) {
        tmp_rays[i].position.s[v_X] = ray_pos[i].s[v_X];
        tmp_rays[i].position.s[v_Y] = ray_pos[i].s[v_Y];
        tmp_rays[i].position.s[v_Z] = ray_pos[i].s[v_Z];
        tmp_rays[i].direction.s[v_X] = ray_dir[i].s[v_X];
        tmp_rays[i].direction.s[v_Y] = ray_dir[i].s[v_Y];
        tmp_rays[i].direction.s[v_Z] = ray_dir[i].s[v_Z];
    }

    *rays = tmp_rays;
    *size = rtcl.num_pixels;
    free(ray_pos);
    free(ray_dir);
}

void rtcl_trace_rays_kernel_init(const struct ray *rays, int num_rays, int rays_per_pixel)
{
    int num_pixels = num_rays / rays_per_pixel;

    rtcl_select_kernel("trace_rays");

    cl_float3 *h_ray_pos = (cl_float3 *)malloc(sizeof(cl_float3) * num_rays);
    cl_float3 *h_ray_dir = (cl_float3 *)malloc(sizeof(cl_float3) * num_rays);
    cl_float4 *h_pixel_board = (cl_float4 *)malloc(sizeof(cl_float4) * num_pixels);

    for(int i = 0; i < num_rays; i++) {
        h_ray_pos[i].s[v_X] = rays[i].position.s[v_X];
        h_ray_pos[i].s[v_Y] = rays[i].position.s[v_Y];
        h_ray_pos[i].s[v_Z] = rays[i].position.s[v_Z];
        h_ray_dir[i].s[v_X] = rays[i].direction.s[v_X];
        h_ray_dir[i].s[v_Y] = rays[i].direction.s[v_Y];
        h_ray_dir[i].s[v_Z] = rays[i].direction.s[v_Z];
    }

    for(int i = 0; i < num_pixels; i++) {
        h_pixel_board[i].s[c_R] = 0;
        h_pixel_board[i].s[c_G] = 0;
        h_pixel_board[i].s[c_B] = 0;
        h_pixel_board[i].s[c_A] = 0;
    }

    // write buffers to device
    err = clEnqueueWriteBuffer(rtcl.commands, buffers.rays_positions, CL_TRUE, 0, sizeof(cl_float3) * num_rays, h_ray_pos, 0, NULL, NULL);
    if(err != CL_SUCCESS) {
        printf("Error: Failed to write to source array! %d\n", err);
        exit(1);
    }

    err = clEnqueueWriteBuffer(rtcl.commands, buffers.rays_directions, CL_TRUE, 0, sizeof(cl_float3) * num_rays, h_ray_dir, 0, NULL, NULL);
    if(err != CL_SUCCESS) {
        printf("Error: Failed to write to source array! %d\n", err);
        exit(1);
    }

    err = clEnqueueWriteBuffer(rtcl.commands, buffers.pixel_board, CL_TRUE, 0, sizeof(cl_float3) * num_pixels, h_pixel_board, 0, NULL, NULL);
    if(err != CL_SUCCESS) {
        printf("Error: Failed to write to source array! %d\n", err);
        exit(1);
    }

    err = 0;
    err |= clSetKernelArg(rtcl.kernel, 0, sizeof(buffers.scene_buffer), &buffers.scene_buffer);
    err |= clSetKernelArg(rtcl.kernel, 1, sizeof(buffers.lights_buffer), &buffers.lights_buffer);
    err |= clSetKernelArg(rtcl.kernel, 2, sizeof(rtcl.num_scene_objects), &rtcl.num_scene_objects);
    err |= clSetKernelArg(rtcl.kernel, 3, sizeof(buffers.rays_positions), &buffers.rays_positions);
    err |= clSetKernelArg(rtcl.kernel, 4, sizeof(buffers.rays_directions), &buffers.rays_directions);
    err |= clSetKernelArg(rtcl.kernel, 5, sizeof(rays_per_pixel), &rays_per_pixel);
    err |= clSetKernelArg(rtcl.kernel, 6, sizeof(buffers.pixel_board), &buffers.pixel_board);
    err |= clSetKernelArg(rtcl.kernel, 7, sizeof(rtcl.camera.screen_width), &rtcl.camera.screen_width);
    err |= clSetKernelArg(rtcl.kernel, 8, sizeof(rtcl.camera.screen_height), &rtcl.camera.screen_height);
    err |= clSetKernelArg(rtcl.kernel, 9, sizeof(rtcl.recursion_depth), &rtcl.recursion_depth);
    err |= clSetKernelArg(rtcl.kernel, 10, sizeof(unsigned int), &rtcl.num_pixels);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to set kernel arguments! %d\n", err);
        exit(1);
    }

    free(h_ray_pos);
    free(h_ray_dir);
    free(h_pixel_board);
}

void rtcl_update_scene()
{
    rtcl_select_kernel("update_scene");

    cl_float3 *new_positions = (cl_float3 *)malloc(sizeof(cl_float3) * rtcl.num_scene_objects);
    for(int i = 0; i < rtcl.num_scene_objects; i++) {
        new_positions->s[0] = rand() % 2 == 0 ? -0.1 : 0.1f; //((float)rand() / (float)RAND_MAX) * 2.f - 1.f;
        new_positions->s[1] = rand() % 2 == 0 ? -0.1 : 0.1f; //((float)rand() / (float)RAND_MAX) * 2.f - 1.f;
        new_positions->s[2] = -0.1; //((float)rand() / (float)RAND_MAX) * 2.f - 1.f;
    }

    err = clEnqueueWriteBuffer(rtcl.commands, buffers.scene_update_positions, CL_TRUE, 0, sizeof(cl_float3) * rtcl.num_scene_objects, new_positions, 0, NULL, NULL);
    

    err = 0;
    err |= clSetKernelArg(rtcl.kernel, 0, sizeof(buffers.scene_buffer), &buffers.scene_buffer);
    err |= clSetKernelArg(rtcl.kernel, 1, sizeof(buffers.scene_update_positions), &buffers.scene_update_positions);
    err |= clSetKernelArg(rtcl.kernel, 2, sizeof(rtcl.num_scene_objects), &rtcl.num_scene_objects);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to set kernel arguments! %d\n", err);
        exit(1);
    }

    free(new_positions);
}

void rtcl_read_pixel_board(struct pixel **pixel_board)
{
    struct pixel *pixels = (struct pixel *)malloc(sizeof(struct pixel) * rtcl.num_pixels);

    // struct ray *tmp_rays = (struct ray *)malloc(sizeof(struct ray) * rtcl.num_pixels);
    // cl_float3 ray_pos[rtcl.num_pixels];
    // cl_float3 ray_dir[rtcl.num_pixels];

    cl_float4 *raw_pixels = (cl_float4 *)malloc(sizeof(cl_float4) * rtcl.num_pixels);
    // copy rays on device to host
    err = clEnqueueReadBuffer(rtcl.commands, buffers.pixel_board, CL_TRUE, 0, sizeof(cl_float4) * rtcl.num_pixels, raw_pixels, 0, NULL, NULL);

    for(int i = 0; i  < rtcl.num_pixels; i++) {
        pixels[i].color.s[c_R] = raw_pixels[i].s[c_R];
        pixels[i].color.s[c_G] = raw_pixels[i].s[c_G];
        pixels[i].color.s[c_B] = raw_pixels[i].s[c_B];
        pixels[i].color.s[c_A] = raw_pixels[i].s[c_A];
        // tmp_rays[i].position.s[v_X] = ray_pos[i].s[v_X];
        // tmp_rays[i].position.s[v_Y] = ray_pos[i].s[v_Y];
        // tmp_rays[i].position.s[v_Z] = ray_pos[i].s[v_Z];
        // tmp_rays[i].direction.s[v_X] = ray_dir[i].s[v_X];
        // tmp_rays[i].direction.s[v_Y] = ray_dir[i].s[v_Y];
        // tmp_rays[i].direction.s[v_Z] = ray_dir[i].s[v_Z];
    }

    *pixel_board = pixels;
    free(raw_pixels);
}



void rtcl_run()
{
    //rtcl_opencl_kernel_workgroup_info();

    // Execute the kernel over the entire range of our 1d input data set
    // using the maximum number of work group items for this device
    global = rtcl.num_pixels;
    local = 0;  // TODO I still have no idea what I'm doing here
    err = clEnqueueNDRangeKernel(rtcl.commands, rtcl.kernel, rtcl.dimension, NULL, &global, 0, 0, NULL, NULL);
    if(err) {
        printf( "Error: Failed to execute kernel! %d\n", err);
        exit(EXIT_FAILURE);
    }

    // Wait for the command queue to get serviced before reading back the results
    clFinish(rtcl.commands);
}

void rtcl_validate()
{
    return;
//    cl_float3 results[rtcl.num_pixels];
//    err = clEnqueueReadBuffer(rtcl.commands, d_output_rays, CL_TRUE, 0, sizeof(cl_float3) * rtcl.num_pixels, results, 0, NULL, NULL); 
//    for(int i = 0; i < rtcl.num_pixels; i++) {
//        cl_float3 out = results[i];
//        int x = i % rtcl.renderer->width;
//        int y = rtcl.renderer->height > 1 ? i / rtcl.renderer->width : 0;
//        printf("(%02i, %02i) <= ray.pos(%d, %d, %d) + ray.dir(%5.2f, %5.2f, %5.2f)\n",
//            x, y, 0, 0, 0, out.s[0], out.s[1], out.s[2]);
//    }
}

void rtcl_opencl_info()
{
    cl_platform_id platforms[100];
    cl_uint platforms_n = 0;
    clGetPlatformIDs(100, platforms, &platforms_n);

    printf("=== %d OpenCL platform(s) found:\n", platforms_n);
    for(int i = 0; i < platforms_n; i++) {
        int bsize = 10240;
        char buffer[bsize];
        printf("  -- %d --\n", i);
        clGetPlatformInfo(platforms[i], CL_PLATFORM_PROFILE, bsize, buffer, NULL);
        printf("  PROFILE = %s\n", buffer);
        clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, bsize, buffer, NULL);
        printf("  VERSION = %s\n", buffer);
        clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, bsize, buffer, NULL);
        printf("  NAME = %s\n", buffer);
        clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, bsize, buffer, NULL);
        printf("  VENDOR = %s\n", buffer);
        clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, bsize, buffer, NULL);
        printf("  EXTENSIONS = %s\n", buffer);
    }

    cl_device_id devices[100];
    cl_uint devices_n;
    clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 100, devices, &devices_n);

    printf(" === %d OpenCL device(s) found on platform ===\n", devices_n);
    for(int i = 0; i < devices_n; i++) {
        int bsize = 10240;
        char buffer[bsize];
        cl_uint buf_uint;
        cl_ulong buf_ulong;
        printf("  -- %d --\n", i);
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(buffer), buffer, NULL);
        printf("  DEVICE_NAME = %s\n", buffer);
        clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL);
        printf("  DEVICE_VENDOR = %s\n", buffer);
        clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, sizeof(buffer), buffer, NULL);
        printf("  DEVICE_VERSION = %s\n", buffer);
        clGetDeviceInfo(devices[i], CL_DRIVER_VERSION, sizeof(buffer), buffer, NULL);
        printf("  DRIVER_VERSION = %s\n", buffer);
        clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(buf_uint), &buf_uint, NULL);
        printf("  CL_DEVICE_MAX_COMPUTE_UNITS = %u\n", (unsigned int)buf_uint);
        clGetDeviceInfo(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(buf_ulong), &buf_ulong, NULL);
        printf("  CL_DEVICE_GLOBAL_MEM_SIZE = %lluMiB\n", (unsigned long long)buf_ulong/1024/1024);
    }
}

void rtcl_cleanup()
{
    // Shutdown and cleanup
    free(h_output_rays_position);
    free(h_output_rays_direction);

    clReleaseMemObject(buffers.rays_positions);
    clReleaseMemObject(buffers.rays_directions);
    clReleaseProgram(rtcl.program);
    clReleaseKernel(rtcl.kernel);
    clReleaseCommandQueue(rtcl.commands);
    clReleaseContext(rtcl.context);
}

void rtcl_select_kernel(const char *kernel_name)
{
    strncpy(rtcl.kernel_name, kernel_name, 256);

    // Create the compute kernel in the program we wish to run
    rtcl.kernel = clCreateKernel(rtcl.program, rtcl.kernel_name, &err);
    if(!rtcl.kernel || err != CL_SUCCESS) {
        printf( "Error: Failed to create compute kernel! %d\n", err);
        exit(1);
    }
}

void rtcl_opencl_kernel_workgroup_info()
{
    // Get the maximum work group size for executing the kernel on the device
    size_t wgs;
    size_t pwgsm;
    cl_ulong lms;
    cl_ulong pms;
    size_t cwgs[3];

    err = clGetKernelWorkGroupInfo(rtcl.kernel, rtcl.device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &wgs, NULL);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to retrieve kernel work group info! %d\n", err);
        exit(1);
    }
    local = wgs;

    err = clGetKernelWorkGroupInfo(rtcl.kernel, rtcl.device_id, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &pwgsm, NULL);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to retrieve kernel preferred work group size multiple! %d\n", err);
        exit(1);
    }

    err = clGetKernelWorkGroupInfo(rtcl.kernel, rtcl.device_id, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &lms, NULL);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to retrieve kernel local mem size! %d\n", err);
        exit(1);
    }
    err = clGetKernelWorkGroupInfo(rtcl.kernel, rtcl.device_id, CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(cl_ulong), &pms, NULL);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to retrieve kernel private mem size! %d\n", err);
        exit(1);
    }

    err = clGetKernelWorkGroupInfo(rtcl.kernel, rtcl.device_id, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t) * 3, &cwgs[0], NULL);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to retrieve kernel compile work group size! %d\n", err);
        exit(1);
    }

    printf("* Work Group Size : %zu\n", wgs);
    printf("* Preferred Work Group Size Multiple : %zu\n", pwgsm);
    printf("* Local Mem Size:  %llu\n", lms);
    printf("* Private Mem Size : %lluKiB\n", pms/1024);
    printf("* Compile Work Group Size : [%zu, %zu, %zu]\n", cwgs[0], cwgs[1], cwgs[2]);
}
