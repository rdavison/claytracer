#ifndef __RTCL_RAYTRACER__
#define __RTCL_RAYTRACER__

#include <OpenCL/opencl.h>

#define v_X (0)
#define v_Y (1)
#define v_Z (2)
#define v_W (3)

#define c_R (0)
#define c_G (1)
#define c_B (2)
#define c_A (3)

#define MATTE 1
#define GLASS 2
#define LIGHT 3

typedef struct vec3 {
    float s[3];
} vec3;

typedef struct vec4 {
    float s[4];
} vec4;

typedef struct color3 {
    float s[3];
} color3;

typedef struct color4 {
    float s[4];
} color4;

struct ray {
    vec3 position;
    vec3 direction;
};

struct pixel {
    color4 color;
};

struct camera {
    vec3 position;
    vec3 direction;
    float clip_near;
    float clip_far;
    float focal_length;
    int screen_width;
    int screen_height;
};

struct sphere {
    vec3 position;
    float radius;
    color4 color;
    float material_type;
};

struct plane {
    vec3 position;
    vec3 normal;
    color4 color;
    float material_type;
};

#endif
