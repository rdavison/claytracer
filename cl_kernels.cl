#include "shape_defines.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// typedefs, enums, and structs
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define MATTE 1
#define MIRROR 2
#define LIGHT 3
#define MAX_TRACE_DEPTH 5 // < 5 maxing out private memory?

typedef float3 vec3;
typedef float4 color4;

enum IntersectionType {
    OUTER_HIT,
    INNER_HIT,
    MISS
};

enum IntersectStatus {
    CONTINUE, STOP
};

enum RayType {
    PRIMARY,
    REFLECT,
    REFRACT,
    REFLECT_AND_REFRACT,
    LIGHT_SEEK,
    SHADOW,
    LIGHT_FOUND,
    INFINITE
};

struct Intersection {
    float                       dist;
    int                         obj_index;
    vec3                        obj_normal;
    enum IntersectionType       type;
};

struct TraceInfo {
    enum IntersectStatus        status;
    color4                      color;
    float                       ray_dot;
    float                       dist;
    color4                      fade_amount;
/**/enum RayType                ray_type;
    enum RayType                next_ray_type;
    vec3                        next_ray_pos[3]; // [reflect|refract|none, refract|none]
    vec3                        next_ray_dir[3]; // [reflect|refract|none, refract|none]
};


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// function prototypes
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

struct Intersection intersect_plane(
    constant float16 *scene,
    int obj_index,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t);


struct Intersection intersect_sphere(
    constant float16 *scene,
    int obj_index,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t);

struct Intersection intersect_cube(
    float3 cube_near, 
    float3 cube_far,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t);


struct TraceInfo intersect_scene(
    constant float16 *scene,
    int scene_size,
    float3 ray_pos,
    float3 ray_dir,
    enum RayType ray_type,
    float previous_distance,
    int trace_depth);


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// function implementations
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


struct Intersection intersect_cube(
    float3 cube_near, 
    float3 cube_far,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t)
{
    struct Intersection jieguo;
    //
    // TODO
    //
    return jieguo;
}

struct Intersection intersect_plane(
    constant float16 *scene,
    int obj_index,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t)
{
    float3 plane_pos = scene[obj_index].s123;
    float3 plane_normal = scene[obj_index].s456;

    struct Intersection jieguo;
    jieguo.dist = max_t;
    jieguo.obj_normal = plane_normal;

    float d = sqrt(dot(plane_pos, plane_pos));
    float num = -d - dot(ray_pos, plane_normal);
    float denom = dot(ray_dir, plane_normal);
    if(denom == 0) {
        jieguo.type = MISS;
        return jieguo;
    }

    float t = num / denom;

    if(t > 0.1f && t < max_t) {
        jieguo.dist = t;
        jieguo.type = OUTER_HIT;
        return jieguo;
    } else {
        jieguo.type = MISS;
        return jieguo;
    }
}

struct Intersection intersect_sphere(
    constant float16 *scene,
    int obj_index,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t)
{
    float3 sphere_pos = scene[obj_index].s123;
    float sphere_radius = scene[obj_index].s4;

    float t0, t1;
    struct Intersection jieguo;
    jieguo.dist = max_t;

    vec3 L = sphere_pos - ray_pos;
    float tca = dot(L, ray_dir);
    if(tca < 0) {
        jieguo.type = MISS;
        return jieguo; // try removing this
    }

    float d2 = dot(L,L) - tca*tca;
    float radius2 = sphere_radius * sphere_radius;
    if(d2 > radius2) {
        jieguo.type = MISS;
        return jieguo;
    }
    float thc = sqrt(radius2 - d2);
    t0 = tca - thc;
    t1 = tca + thc;
    if(t0 > max_t) {
        jieguo.type = MISS;
        return jieguo;
    } else {
        jieguo.type = OUTER_HIT;
        jieguo.dist = t0;
        jieguo.obj_normal = normalize((ray_pos + t0*ray_dir) - sphere_pos);
        return jieguo;
    }
}

struct Intersection Intersection_init(struct Intersection *intersection)
{
    intersection->dist = 1000000.0f;
    intersection->obj_index = -1;
    intersection->obj_normal = (vec3)(0,0,0);
    intersection->type = MISS;
}

void find_nearest_intersection(
    constant float16 *scene,
    int scene_size,
    vec3 ray_pos,
    vec3 ray_dir,
    struct Intersection *nearest)
{
    Intersection_init(nearest);

    // loop through all objects
    for(int obj_index = 0; obj_index < scene_size; obj_index+=2) {
        struct Intersection obj_intersection;
        Intersection_init(&obj_intersection);

        obj_intersection.dist = nearest->dist;

        // refers to the upper 16 floats of the raw memory addresses
        float16 upper = scene[obj_index];
        int shape_type = (int)upper.s0;

        switch(shape_type) {
            case SHAPE_SPHERE:
                obj_intersection = intersect_sphere(
                                        scene,
                                        obj_index,
                                        ray_pos,
                                        ray_dir,
                                        nearest->dist);
                break;
            case SHAPE_PLANE:
                obj_intersection = intersect_plane(
                                        scene,
                                        obj_index,
                                        ray_pos,
                                        ray_dir,
                                        nearest->dist);
                break;
            default:
                printf("Error: impossible shape\n");
                break;
        }

        // save nearest intersection
        if(obj_intersection.dist < nearest->dist && obj_intersection.dist > 0.01f) {
            *nearest = obj_intersection;
            nearest->obj_index = obj_index;
        }
    }
}

void TraceInfo_init(struct TraceInfo *info)
{
    info->status = STOP;
    info->color = (color4)(0,0,0,0);
    info->ray_dot = 0.f;
    info->dist = 0.f;
    info->fade_amount = (color4)(0,0,0,0);
    info->ray_type = PRIMARY;
    info->next_ray_type = INFINITE;
    for(int i = 0; i < 3; i++) {
        info->next_ray_pos[i] = (vec3)(0,0,0);
        info->next_ray_dir[i] = (vec3)(0,0,0);
    }
}

struct TraceInfo intersect_scene(
    constant float16 *scene,
    int scene_size,
    float3 ray_pos,
    float3 ray_dir,
    enum RayType ray_type,
    float previous_distance,
    int trace_depth)
{
    float max_distance = 1000000.0f; // TODO make settable elsewhere

    struct Intersection nearest_intersection;
    find_nearest_intersection(scene, scene_size, ray_pos, ray_dir, &nearest_intersection);


    struct TraceInfo traceinfo;
    TraceInfo_init(&traceinfo);

    traceinfo.ray_type = ray_type;

    color4 light_power = (color4)(1,1,1,1); // TODO: make settable elsewhere

    // settings based on whether there was an intersection or not
    switch(nearest_intersection.type) {
        case MISS:
            traceinfo.status = STOP;
            traceinfo.color = (color4)(0,0,0,0);
            traceinfo.next_ray_type = INFINITE;
            return traceinfo;

        case OUTER_HIT:
        case INNER_HIT:
        default:
            traceinfo.dist = nearest_intersection.dist + previous_distance;
            traceinfo.ray_dot = dot(-ray_dir, nearest_intersection.obj_normal);
            traceinfo.fade_amount = (light_power * traceinfo.ray_dot) / (4.f * 3.141592f * pow(traceinfo.dist,2));
            traceinfo.fade_amount = clamp(traceinfo.fade_amount * 5000, 0.f, 1.f);
            //traceinfo.fade_amount = smoothstep(0.f, 1.f, traceinfo.fade_amount * 5000);
            traceinfo.color = scene[nearest_intersection.obj_index+1].rgba;
            break;
    }

    int nearest_material = (int) scene[nearest_intersection.obj_index+1].s4; 
    if(ray_type == LIGHT_SEEK) {
        traceinfo.status = STOP;
        traceinfo.next_ray_type = select(SHADOW, LIGHT_FOUND, nearest_material == LIGHT);
    } else {
        switch(nearest_material) {
        case MATTE:
            traceinfo.next_ray_type = LIGHT_SEEK;
            traceinfo.status = CONTINUE;
            break;
        case MIRROR:
            traceinfo.next_ray_type = REFLECT;
            traceinfo.status = CONTINUE;
            break;
        case LIGHT:
            traceinfo.next_ray_type = LIGHT_FOUND;
            traceinfo.status = STOP;
            break;
        default:
            break;
        }
    }

    if(trace_depth >= MAX_TRACE_DEPTH-1) {
        traceinfo.status = STOP;
    }

    if(traceinfo.status == CONTINUE) {

        float t = nearest_intersection.dist;
        float3 light_pos = scene[6].s123;
        vec3 normal = normalize(nearest_intersection.obj_normal);

        switch(traceinfo.next_ray_type) {
        case LIGHT_SEEK:
            traceinfo.next_ray_pos[0] = ray_pos + t * ray_dir;
            traceinfo.next_ray_dir[0] = normalize(light_pos - traceinfo.next_ray_pos[0]);
            break;

        case REFRACT:
            // TODO not yet implemented
            break;

        case REFLECT:
            traceinfo.next_ray_pos[0] = ray_pos + t * ray_dir;
            traceinfo.next_ray_dir[0] = ray_dir - 2 * dot(ray_dir, normal) * normal;
            break;

        default:
            break;
        }
    }
   
    return traceinfo;
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

kernel void update_scene(
    global float16 *scene_objects, 
    constant float3 *update_pos,
    const int num_objects)
{
    int i = get_global_id(0);
    if(i < num_objects) {
        unsigned int j = i * 2;
        float delta = 0.02f;
        scene_objects[j].s123 += clamp(sin(scene_objects[j].s123), -delta, delta);
    }
}

color4 parse_color(struct TraceInfo *info, int size)
{
    for(int i = 0; i < size; i++) {
        if(info[i].status == STOP) {
            if(info[i].next_ray_type == SHADOW) {
                return (float4)(0,0,0,0);
            }

            color4 light = info[i].fade_amount * info[i].color;
            //color4 acc = (color4)(0.5,0.5,0.5,0.5);
            color4 acc = (color4)(0,0,0,0);
            if(i > 0){
                for(int j = i; j >= 0; j--) {
                    float m = 1.0;
                    if(info[j].next_ray_type == REFLECT) {
                        m = 0.5;
                    }
                    acc += m * (info[j].fade_amount * info[j].color) / (float)(i+1);
                }
                return acc;
            }
            return light;
        }
    }
    return (float4)(0,1,0,1);
}


kernel void trace_rays(
    constant float16 *scene,  // read only
    constant float16 *lights,  // read only
    const int scene_size, // size of the float16 array
    global float3 *ray_pos,
    global float3 *ray_dir,
    const int rays_per_pixel,
    global float4 *pixel_board,
    const int width,
    const int height,
    const int max_depth,
    const unsigned int count)
{
    unsigned int i = get_global_id(0);
    if(i < count) {
        int size = MAX_TRACE_DEPTH;
        struct TraceInfo traceinfo[MAX_TRACE_DEPTH];

        traceinfo[0] = intersect_scene(scene, scene_size, ray_pos[i], ray_dir[i], PRIMARY, 0, 0);
        for(int depth = 1; depth < max_depth; depth++) {

            int previous = depth - 1;

            if(traceinfo[previous].status == STOP)
                break;

            // otherwise...
            vec3 next_ray_pos = traceinfo[previous].next_ray_pos[0];
            vec3 next_ray_dir = traceinfo[previous].next_ray_dir[0];
            enum RayType next_ray_type = traceinfo[previous].next_ray_type;
            traceinfo[depth] = intersect_scene(
                scene, 
                scene_size, 
                next_ray_pos, 
                next_ray_dir, 
                next_ray_type,
                traceinfo[depth-1].dist,
                depth);
        }

        color4 color = parse_color(traceinfo, MAX_TRACE_DEPTH);
        //pixel_board[i] = color;
        pixel_board[i] = smoothstep(0.f, 1.f, color);
    }
}
