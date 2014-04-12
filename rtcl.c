#include "rtcl.h"
#include "util.h"
#include "compile.h"
#include "raytracer.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>

int times = 0;
#define DATA_SIZE (1024)

extern int __renderer_width;
extern int __renderer_height;

int err;                        // error code returned from api calls
cl_float3 data[DATA_SIZE];      // original data set give to device
size_t global;                  // global domain size for our calculation
size_t local;                   // local domain size for our calculation
unsigned int __rtcl_num_pixels;
int i;

char *__kernel_name;
#define __KERNEL_NAME_SIZE__ (256)

size_t __rtcl_dimension;

cl_float3 camera_pos;
cl_float3 camera_dir;

cl_mem d_output_rays_position;
cl_mem d_output_rays_direction;
cl_float3 *h_output_rays_position;
cl_float3 *h_output_rays_direction;



cl_mem d_pixel_board;
cl_mem d_scene_buffer;
int d_scene_size = 0;
cl_int scene_size;

struct rtcl {
    char kernel_name[__KERNEL_NAME_SIZE__];
    struct camera camera;
} rtcl;

//struct rtcl rtcl;


void rtcl_copy_scene_to_device()
{
    int NUM_SPHERES = 500;

    srand(time(NULL));
    struct plane plane = {
        .position = { 0.0f, -2.0f, 0.0f },
        .normal = {0.0f, 1.0f, 0.0f },
        .color = {0.3f, 0.5f, 1.0f, 1.0f },
        .material_type = MATTE
    };

//    struct sphere sphere = {
//        .position = { 1.0f, 0.0f, -4.0f },
//        .radius = 1.0f,
//        .color = {1.0f, 0.5f, 0.5f, 1.0f },
//        .material_type = MATTE
//    };
//
//    struct sphere sphere2 = {
//        .position = { 0.0f, 0.0f, -4.0f },
//        .radius = 1.0f,
//        .color = {0.5f, 1.0f, 0.5f, 1.0f },
//        .material_type = MATTE
//    };
//
//  struct sphere light = {
//      .position = { 1.0f, 0.0f, -4.0f },
//      .radius = 1.0f,
//      .color = {1.0f, 1.0f, 1.0f, 1.0f },
//      .material_type = LIGHT
//  };
    
    struct llist *llist = (struct llist *)calloc(1, sizeof(struct llist));
    struct sphere *spheres = (struct sphere *)calloc(NUM_SPHERES, sizeof(struct sphere));
    for(int i = 0; i < NUM_SPHERES; i++) {
        float x, y, z, r, g, b, d;
        x = (float)rand() / (float)RAND_MAX;
        y = (float)rand() / (float)RAND_MAX;
        z =-(float)rand() / (float)RAND_MAX;
        r = (float)rand() / (float)RAND_MAX;
        g = (float)rand() / (float)RAND_MAX;
        b = (float)rand() / (float)RAND_MAX;
        d = (float)rand() / (float)RAND_MAX;
        spheres[i].position.s[0] = x*20.f-10.f;
        spheres[i].position.s[1] = y*20.f-10.f;
        spheres[i].position.s[2] = -z*100.f-110.f;
        spheres[i].radius = d;
        spheres[i].color.s[0] = r;
        spheres[i].color.s[1] = g;
        spheres[i].color.s[2] = b;
        spheres[i].color.s[3] = 1.0f;
        spheres[i].material_type = MATTE;
        llist_append(llist, compile_sphere(spheres + i));
    }
//    llist_append(llist, compile_sphere(&sphere));
//    llist_append(llist, compile_sphere(&sphere2));
//    llist_append(llist, compile_sphere(&light));
    llist_append(llist, compile_plane(&plane));


    cl_float16 *h_scene_buffer = (cl_float16 *)malloc(sizeof(cl_float16)*llist->size*2); // multiply by 2 because each object is 2

    // TODO FINISH THIS LOOP
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

    d_scene_size = llist->size*2;
    d_scene_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float16) * llist->size * 2, NULL, NULL);
    if(!d_scene_buffer) {
        printf("Error: Failed to allocate scene buffer!\n");
        exit(1);
    }

    err = clEnqueueWriteBuffer(commands, d_scene_buffer, CL_TRUE, 0, sizeof(cl_float16) * llist->size * 2, h_scene_buffer, 0, NULL, NULL);
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
    
    // create buffers
    d_output_rays_position = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float3) * __rtcl_num_pixels, NULL, NULL);
    if(!d_output_rays_position) {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }

    d_output_rays_direction = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float3) * __rtcl_num_pixels, NULL, NULL);
    if(!d_output_rays_direction) {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }

    // write buffers to device
    err = clEnqueueWriteBuffer(commands, d_output_rays_position, CL_TRUE, 0, sizeof(cl_float3) * __rtcl_num_pixels, h_output_rays_position, 0, NULL, NULL);
    if(err != CL_SUCCESS) {
        printf("Error: Failed to write to source array! %d\n", err);
        exit(1);
    }

    err = clEnqueueWriteBuffer(commands, d_output_rays_direction, CL_TRUE, 0, sizeof(cl_float3) * __rtcl_num_pixels, h_output_rays_direction, 0, NULL, NULL);
    if(err != CL_SUCCESS) {
        printf("Error: Failed to write to source array! %d\n", err);
        exit(1);
    }

    // prepare kernel arguments
    rtcl.camera.position.s[v_X] = 0.0f;
    rtcl.camera.position.s[v_Y] = 0.0f;
    rtcl.camera.position.s[v_Z] = 0.0f;

    rtcl.camera.direction.s[v_X] = 0.0f;
    rtcl.camera.direction.s[v_Y] = 0.0f;
    rtcl.camera.direction.s[v_Z] = -1.0f;

    rtcl.camera.screen_width = __renderer_width;
    rtcl.camera.screen_height = __renderer_height;
    rtcl.camera.focal_length = 60.0;

    // Set the arguments to our compute kernel
    err = 0;
    err |= clSetKernelArg(kernel, 0, sizeof(rtcl.camera.position), &rtcl.camera.position);
    err |= clSetKernelArg(kernel, 1, sizeof(rtcl.camera.direction), &rtcl.camera.direction);
    err |= clSetKernelArg(kernel, 2, sizeof(rtcl.camera.focal_length), &rtcl.camera.focal_length);
    err |= clSetKernelArg(kernel, 3, sizeof(rtcl.camera.screen_width), &rtcl.camera.screen_width);
    err |= clSetKernelArg(kernel, 4, sizeof(rtcl.camera.screen_height), &rtcl.camera.screen_height);
    err |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &d_output_rays_position);
    err |= clSetKernelArg(kernel, 6, sizeof(cl_mem), &d_output_rays_direction);
    err |= clSetKernelArg(kernel, 7, sizeof(unsigned int), &__rtcl_num_pixels);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to set kernel arguments! %d\n", err);
        exit(1);
    }
}

void rtcl_read_rays_from_device(struct ray **rays, int *size)
{
    struct ray *tmp_rays = (struct ray *)malloc(sizeof(struct ray) * __rtcl_num_pixels);
    cl_float3 ray_pos[__rtcl_num_pixels];
    cl_float3 ray_dir[__rtcl_num_pixels];

    // copy rays on device to host
    err = clEnqueueReadBuffer(commands, d_output_rays_position, CL_TRUE, 0, sizeof(cl_float3) * __rtcl_num_pixels, ray_pos, 0, NULL, NULL);
    err = clEnqueueReadBuffer(commands, d_output_rays_direction, CL_TRUE, 0, sizeof(cl_float3) * __rtcl_num_pixels, ray_dir, 0, NULL, NULL);

    for(int i = 0; i  < __rtcl_num_pixels; i++) {
        tmp_rays[i].position.s[v_X] = ray_pos[i].s[v_X];
        tmp_rays[i].position.s[v_Y] = ray_pos[i].s[v_Y];
        tmp_rays[i].position.s[v_Z] = ray_pos[i].s[v_Z];
        tmp_rays[i].direction.s[v_X] = ray_dir[i].s[v_X];
        tmp_rays[i].direction.s[v_Y] = ray_dir[i].s[v_Y];
        tmp_rays[i].direction.s[v_Z] = ray_dir[i].s[v_Z];
    }

    *rays = tmp_rays;
    *size = __rtcl_num_pixels;
}

void rtcl_trace_rays_kernel_init(const struct ray *rays, int num_rays, int rays_per_pixel)
{
    int num_pixels = num_rays / rays_per_pixel;

    rtcl_select_kernel("trace_rays");

    cl_float3 h_ray_pos[num_rays];
    cl_float3 h_ray_dir[num_rays];
    cl_float4 h_pixel_board[num_pixels];

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

    cl_mem d_ray_pos;
    cl_mem d_ray_dir;

    // prepare buffers
    d_ray_pos = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float3) * num_rays, NULL, NULL);
    if(!d_ray_pos) {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }

    d_ray_dir = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float3) * num_rays, NULL, NULL);
    if(!d_ray_dir) {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }

    d_pixel_board = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float4) * num_pixels, NULL, NULL);
    if(!d_pixel_board) {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }

    // write buffers to device
    err = clEnqueueWriteBuffer(commands, d_ray_pos, CL_TRUE, 0, sizeof(cl_float3) * num_rays, h_ray_pos, 0, NULL, NULL);
    if(err != CL_SUCCESS) {
        printf("Error: Failed to write to source array! %d\n", err);
        exit(1);
    }

    err = clEnqueueWriteBuffer(commands, d_ray_dir, CL_TRUE, 0, sizeof(cl_float3) * num_rays, h_ray_dir, 0, NULL, NULL);
    if(err != CL_SUCCESS) {
        printf("Error: Failed to write to source array! %d\n", err);
        exit(1);
    }

    err = clEnqueueWriteBuffer(commands, d_pixel_board, CL_TRUE, 0, sizeof(cl_float3) * num_pixels, h_pixel_board, 0, NULL, NULL);
    if(err != CL_SUCCESS) {
        printf("Error: Failed to write to source array! %d\n", err);
        exit(1);
    }

    int err = 0;
    err |= clSetKernelArg(kernel, 0, sizeof(d_scene_buffer), &d_scene_buffer);
    err |= clSetKernelArg(kernel, 1, sizeof(d_scene_size), &d_scene_size);
    err |= clSetKernelArg(kernel, 2, sizeof(d_ray_pos), &d_ray_pos);
    err |= clSetKernelArg(kernel, 3, sizeof(d_ray_dir), &d_ray_dir);
    err |= clSetKernelArg(kernel, 4, sizeof(rays_per_pixel), &rays_per_pixel);
    err |= clSetKernelArg(kernel, 5, sizeof(d_pixel_board), &d_pixel_board);
    err |= clSetKernelArg(kernel, 6, sizeof(rtcl.camera.screen_width), &rtcl.camera.screen_width);
    err |= clSetKernelArg(kernel, 7, sizeof(rtcl.camera.screen_height), &rtcl.camera.screen_height);
    err |= clSetKernelArg(kernel, 8, sizeof(unsigned int), &__rtcl_num_pixels);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to set kernel arguments! %d\n", err);
        exit(1);
    }
}

void rtcl_update_scene()
{
    NOT_IMPLEMENTED(__FILE__, __LINE__, __func__);
}

void rtcl_read_pixel_board(struct pixel **pixel_board)
{
    struct pixel *pixels = (struct pixel *)malloc(sizeof(struct pixel) * __rtcl_num_pixels);

    // struct ray *tmp_rays = (struct ray *)malloc(sizeof(struct ray) * __rtcl_num_pixels);
    // cl_float3 ray_pos[__rtcl_num_pixels];
    // cl_float3 ray_dir[__rtcl_num_pixels];

    cl_float4 raw_pixels[__rtcl_num_pixels];
    // copy rays on device to host
    err = clEnqueueReadBuffer(commands, d_pixel_board, CL_TRUE, 0, sizeof(cl_float4) * __rtcl_num_pixels, raw_pixels, 0, NULL, NULL);

    for(int i = 0; i  < __rtcl_num_pixels; i++) {
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

}

void rtcl_init()
{
    i = 0;
    __rtcl_num_pixels = __renderer_width * __renderer_height;
    __kernel_name = (char *)malloc(__KERNEL_NAME_SIZE__ * sizeof(char));
    __rtcl_dimension = 1;
    h_output_rays_position = (cl_float3 *)malloc(__renderer_width * __renderer_height * sizeof(cl_float3));
    h_output_rays_direction = (cl_float3 *)malloc(__renderer_width * __renderer_height * sizeof(cl_float3));

    
    char *source;
    const char *source_filename = "cl_kernels.cl";

    // ZERO OUT THE ARRAY
    for(i = 0; i < __rtcl_num_pixels; i++) {
        h_output_rays_position[i].s[0] = 0.0f;
        h_output_rays_position[i].s[1] = 0.0f;
        h_output_rays_position[i].s[2] = 0.0f;
    }

    for(i = 0; i < __rtcl_num_pixels; i++) {
        h_output_rays_direction[i].s[0] = 0.0f;
        h_output_rays_direction[i].s[1] = 0.0f;
        h_output_rays_direction[i].s[2] = 0.0f;
    }
    
////    // Fill our data set with random float values
////    for(i = 0; i < __rtcl_num_pixels; i++) {
////        data[i].s[0] = rand() / (float)RAND_MAX;
////        data[i].s[1] = rand() / (float)RAND_MAX;
////        data[i].s[2] = rand() / (float)RAND_MAX;
////    }

    // Connect to a compute device
    int gpu = 1;
    err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to create a device group!\n");
        exit(EXIT_FAILURE);
    }

    // Create compute context
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    if(!context) {
        printf( "Error: Failed to create a compute context!\n");
        exit(EXIT_FAILURE);
    }

    // Create a command queue
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if(!commands) {
        printf( "Error: Failed to create a command queue!\n");
        exit(EXIT_FAILURE);
    }

    // Read in the kernel source code
    read_file(&source, source_filename);

    // Create the compute program from the source buffer
    program = clCreateProgramWithSource(context, 1, (const char **) &source, NULL, &err);
    if(!program) {
        printf( "Error: Failed to create compute program!\n");
        exit(EXIT_FAILURE);
    }

    // Build the program executable
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(err != CL_SUCCESS) {
        size_t len;
        char buffer[2048];

        printf( "Error: Failed to bulid program executable!\n");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf( "%s\n", buffer);
        exit(1);
    }
}


void rtcl_run()
{
    rtcl_opencl_kernel_workgroup_info();

    // Execute the kernel over the entire range of our 1d input data set
    // using the maximum number of work group items for this device
    global = __rtcl_num_pixels;
    err = clEnqueueNDRangeKernel(commands, kernel, __rtcl_dimension, NULL, &global, &local, 0, NULL, NULL);
    if(err) {
        printf( "Error: Failed to execute kernel! %d\n", err);
        exit(EXIT_FAILURE);
    }

    // Wait for the command queue to get serviced before reading back the results
    clFinish(commands);
}

void rtcl_validate()
{
    return;
//    cl_float3 results[__rtcl_num_pixels];
//    err = clEnqueueReadBuffer(commands, d_output_rays, CL_TRUE, 0, sizeof(cl_float3) * __rtcl_num_pixels, results, 0, NULL, NULL); 
//    for(int i = 0; i < __rtcl_num_pixels; i++) {
//        cl_float3 out = results[i];
//        int x = i % __renderer_width;
//        int y = __renderer_height > 1 ? i / __renderer_width : 0;
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

    clReleaseMemObject(d_output_rays_position);
    clReleaseMemObject(d_output_rays_direction);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);
}

void rtcl_select_kernel(const char *kernel_name)
{
    strncpy(__kernel_name, kernel_name, __KERNEL_NAME_SIZE__);

    // Create the compute kernel in the program we wish to run
    kernel = clCreateKernel(program, __kernel_name, &err);
    if(!kernel || err != CL_SUCCESS) {
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

    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &wgs, NULL);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to retrieve kernel work group info! %d\n", err);
        exit(1);
    }
    local = wgs;

    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &pwgsm, NULL);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to retrieve kernel preferred work group size multiple! %d\n", err);
        exit(1);
    }

    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &lms, NULL);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to retrieve kernel local mem size! %d\n", err);
        exit(1);
    }
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(cl_ulong), &pms, NULL);
    if(err != CL_SUCCESS) {
        printf( "Error: Failed to retrieve kernel private mem size! %d\n", err);
        exit(1);
    }

    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t) * 3, &cwgs[0], NULL);
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
