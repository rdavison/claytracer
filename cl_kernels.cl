#include "shape_defines.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// typedefs, enums, and structs
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


enum material {
    MATTE,
    MIRROR,
    METAL,
    LIGHT
};


enum intersect_result_t {
    OUTER_HIT,
    INNER_HIT,
    MISS
};


enum ray_type_t {
    REFLECT,
    REFRACT,
    BOTH,
    DONE,
    BLOCKED
};


typedef float3 vec3;
typedef float4 color4;


struct sphere {

    vec3 position;
    float radius;
    color4 color;
    enum material material;
    bool isLight;
};


struct plane {
    vec3 position;
    vec3 normal;
    color4 color;
    enum material material;
};


struct ray {
    vec3 position;
    vec3 direction;
};


struct scene {
    struct sphere sphere;
    struct sphere light;
    struct plane plane;
};


struct intersect_t {
    float t;
    int nearest_index;
    float3 normal_at_t;
    enum intersect_result_t result;
};


struct intersect_scene_t {
    color4 color;
    enum ray_type_t next_ray_type;
    vec3 refl_ray_pos;
    vec3 refl_ray_dir;
    vec3 refr_ray_pos;
    vec3 refr_ray_dir;
};


struct pixel_status_t {
    struct intersect_scene_t s[5];
};


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// function prototypes
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


struct intersect_scene_t intersect_scene_buffer_2_16(
    constant float16 *buf,
    int bufsize,
    float3 ray_pos,
    float3 ray_dir);


struct intersect_t intersect_plane(
    float3 plane_pos, 
    float3 plane_normal,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t);


struct intersect_t intersect_sphere(
    float3 sphere_pos, 
    float sphere_radius,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t);


struct intersect_scene_t intersect_scene(
    constant float16 *scene,
    int scene_size,
    float3 ray_pos,
    float3 ray_dir);


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// function implementations
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

struct intersect_scene_t intersect_scene_buffer_2_16(
    constant float16 *buff,
    int bufsize,
    float3 ray_pos,
    float3 ray_dir)
{
    color4 black = {0,0,0,0};
    // shape sizes = float16 + float16
    // see compilation information in compile.c

    // loop through objects in scene buffer
    int nearest_index = -1;
    enum intersect_result_t nearest_intersect_result = MISS;
    float t = 1000000.f;
    for(int i = 0; i < bufsize; i+=2) {
        int upper = i;
        //int lower = i + 1;
        struct intersect_t object_intersect;

        float3 pos;
        switch((int)(buff[upper].s0)) {

        case SHAPE_SPHERE: 
            pos = buff[upper].s123;
            float radius = buff[upper].s4;
            object_intersect = intersect_sphere(pos, radius, ray_pos, ray_dir, t);
            break;

        case SHAPE_PLANE:
            pos = buff[upper].s123;
            float3 normal = buff[upper].s456;
            object_intersect = intersect_plane(pos, normal, ray_pos, ray_dir, t);
            break;

        default:
            printf("ERROR: Invalid shape.\n");
            //abort();
            break;
        }

        nearest_intersect_result = 
            object_intersect.result == MISS ? MISS : OUTER_HIT;

        if(object_intersect.t <= t) {
            t = object_intersect.t;
            nearest_index = i;
            //nearest_intersect_result.result
        }
    }

    //float16 nearest_object = buf[nearest_index];
    struct intersect_t retval;
    retval.nearest_index = nearest_index;
    retval.t = t;
    retval.result = nearest_intersect_result;

    struct intersect_scene_t return_data;
    
    return_data.color = retval.nearest_index >= 0 ? (float4)buff[retval.nearest_index+1].s0123 : black;
    return_data.next_ray_type = DONE;
    return return_data;
}

struct intersect_t intersect_plane(
    float3 plane_pos, 
    float3 plane_normal,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t)
{
    struct intersect_t jieguo;
    jieguo.t = max_t;
    jieguo.normal_at_t = plane_normal;

    float d = sqrt(dot(plane_pos, plane_pos));
    float num = -d - dot(ray_pos, plane_normal);
    float denom = dot(ray_dir, plane_normal);
    if(denom == 0) {
        jieguo.result = MISS;
        return jieguo;
    }

    float t = num / denom;

    if(t > 0.1f && t < max_t) {
        jieguo.t = t;
        jieguo.result = OUTER_HIT;
        return jieguo;
    } else {
        jieguo.result = MISS;
        return jieguo;
    }
}
// implementation
struct intersect_t intersect_sphere(
    float3 sphere_pos, 
    float sphere_radius,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t)
{
    float t0, t1;
    struct intersect_t jieguo;
    jieguo.t = max_t;

    vec3 L = sphere_pos - ray_pos;
    float tca = dot(L, ray_dir);
    if(tca < 0) {
        jieguo.result = MISS;
        return jieguo; // try removing this
    }

    float d2 = dot(L,L) - tca*tca;
    float radius2 = sphere_radius * sphere_radius;
    if(d2 > radius2) {
        jieguo.result = MISS;
        return jieguo;
    }
    float thc = sqrt(radius2 - d2);
    t0 = tca - thc;
    t1 = tca + thc;
    if(t0 > max_t) {
        jieguo.result = MISS;
        return jieguo;
    } else {
        jieguo.result = OUTER_HIT;
        jieguo.t = t0;
        jieguo.normal_at_t = normalize((ray_pos + t0*ray_dir) - sphere_pos);
        return jieguo;
    }
}

struct intersect_scene_t intersect_scene(
    constant float16 *scene,
    int scene_size,
    float3 ray_pos,
    float3 ray_dir)
{
    // constant definitions
    color4 white  = (color4)(1.f, 1.f, 1.f, 1.f);
    color4 black  = (color4)(0.f, 0.f, 0.f, 1.f);
    color4 red    = (color4)(1.f, 0.f, 0.f, 1.f);
    color4 blue   = (color4)(0.f, 0.f, 1.f, 1.f);
    color4 yellow = (color4)(1.f, 1.f, 0.f, 1.f);
    color4 green  = (color4)(0.f, 1.f, 0.f, 1.f);

    struct intersect_t nearest_object;
    nearest_object.t = 1000000.0f;
    nearest_object.result = MISS;
    nearest_object.nearest_index = -1;
    for(int i = 0; i < scene_size; i+=2) {
        struct intersect_t object;
        object.t = nearest_object.t;

       float16 upper = scene[i];


        switch((int)upper.s0) {
            case SHAPE_SPHERE:
                object = intersect_sphere((float3)upper.s123, upper.s4, ray_pos, ray_dir, nearest_object.t);
                break;
            case SHAPE_PLANE:
                object = intersect_plane((float3)upper.s123, (float3)upper.s456, ray_pos, ray_dir, nearest_object.t);
                break;
            default:
                printf("Error: impossible shape\n");
                break;
        }

        if(object.t < nearest_object.t && object.t > 0.01f) {
            nearest_object = object;
            nearest_object.nearest_index = i;
        }
    }

    struct intersect_scene_t return_data;
    color4 result;
    if(nearest_object.result != MISS) {
        result = (float4)scene[nearest_object.nearest_index+1].rgba;
        float ray_dot = dot(ray_dir, nearest_object.normal_at_t);
        ray_dot = ray_dot < 0 ? -ray_dot : ray_dot;
        result *= ray_dot;
    } else {
        result = black;
    }

    return_data.color = result;
    return_data.next_ray_type = DONE;
   
    return return_data;
}


kernel void generate_rays(
    const float3 camera_pos,
    const float3 camera_dir,
    const float camera_focal_length,
    const int width,
    const int height,
    global float3 *output_rays_pos,
    global float3 *output_rays_dir,
    const unsigned int count)
{
    unsigned int i = get_global_id(0);
    if(i < count) {

        int x = i % width;
        int y = height > 1 ? i / width : 0;

        float focal = camera_focal_length;
        float angle = tan(focal * 0.5f * 3.141592f / 180.f);
        float imageAspectRatio = (float)width / (float)height;

        float xx = (2.f * (x + 0.5f) / (float)width - 1.f) * angle * imageAspectRatio;
        float yy = (1.f - 2.f * (y + 0.5f) / (float)height) * angle;

        float3 ray_dir = (float3)(xx, yy, camera_dir.z);

        output_rays_pos[i] = camera_pos;
        output_rays_dir[i] = normalize(ray_dir);
    }
}


kernel void trace_rays(
    constant float16 *scene,  // read only
    const int scene_size, // size of the float16 array
    global float3 *ray_pos,
    global float3 *ray_dir,
    const int rays_per_pixel,
    global float4 *pixel_board,
    const int width,
    const int height,
    const unsigned int count)
{
    unsigned int i = get_global_id(0);
    if(i < count) {

        struct pixel_status_t pxs;

        pxs.s[0] = intersect_scene(scene, scene_size, ray_pos[i], ray_dir[i]);

        int depth = 0;
        while(pxs.s[depth].next_ray_type != DONE && depth < 4) {
            int next = depth+1;
            switch(pxs.s[depth].next_ray_type) {
                case BLOCKED: pxs.s[next].next_ray_type = DONE;
                             break;
                case REFLECT: pxs.s[next] = intersect_scene(scene, scene_size, pxs.s[depth].refl_ray_pos, pxs.s[depth].refl_ray_dir);
                              break;
                case REFRACT: pxs.s[next] = intersect_scene(scene, scene_size, pxs.s[depth].refr_ray_pos, pxs.s[depth].refr_ray_dir);
                              break;
                case BOTH: pxs.s[next] = intersect_scene(scene, scene_size, pxs.s[depth].refl_ray_pos, pxs.s[depth].refl_ray_dir);
                           pxs.s[next] = intersect_scene(scene, scene_size, pxs.s[depth].refr_ray_pos, pxs.s[depth].refr_ray_dir);
                           break;
                case DONE: pxs.s[next].next_ray_type = DONE;
                           break;
            }
            depth = next;
        }
        color4 color = pxs.s[0].color;
        pixel_board[i] = color;
    }
}
