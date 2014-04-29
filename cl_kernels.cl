#include "shape_defines.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// typedefs, enums, and structs
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define MATTE 1
#define MIRROR 2
#define LIGHT 3

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


typedef float3 vec3;
typedef float4 color4;


//struct sphere {
//    vec3                        position;
//    float                       radius;
//    color4                      color;
//    enum material               material;
//    //bool isLight;
//};
//
//
//struct plane {
//    vec3                        position;
//    vec3                        normal;
//    color4                      color;
//    enum material               material;
//};
//
//struct cube {
//    vec3                        near;
//    vec3                        far;
//    color4                      color;
//    enum material               material;
//};


struct ray {
    vec3                        position;
    vec3                        direction;
};


//struct scene {
//    struct sphere               sphere;
//    struct sphere               light;
//    struct plane                plane;
//};


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
    enum RayType                ray_type;
    enum RayType                next_ray_type;
    vec3                        next_ray_pos[2]; // [reflect|refract|none, refract|none]
    vec3                        next_ray_dir[2]; // [reflect|refract|none, refract|none]
};


// struct pixel_status_t {
//     struct intersect_scene_t s[5];
// };


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// function prototypes
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


/*
struct TraceInfo intersect_scene_buffer_2_16(
    constant float16 *buf,
    int bufsize,
    float3 ray_pos,
    float3 ray_dir);
*/


struct Intersection intersect_plane(
    float3 plane_pos, 
    float3 plane_normal,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t);


struct Intersection intersect_sphere(
    float3 sphere_pos, 
    float sphere_radius,
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
    enum RayType ray_type);


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// function implementations
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*
struct TraceInfo intersect_scene_buffer_2_16(
    constant float16 *buff,
    int bufsize,
    float3 ray_pos,
    float3 ray_dir)
{
    color4 black = {0,0,0,0};
    // shape sizes = float16 + float16
    // see compilation information in compile.c

    // loop through objects in scene buffer
    int obj_index = -1;
    enum IntersectionType nearest_intersect_result = MISS;
    float t = 1000000.f;
    for(int i = 0; i < bufsize; i+=2) {
        int upper = i;
        //int lower = i + 1;
        struct Intersection object_intersect;

        float3 pos;
        float3 near;
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

        case SHAPE_CUBE:
            near = buff[upper].s123;
            float3 far = buff[upper].s456;
            object_intersect = intersect_cube(near, far, ray_pos, ray_dir, t);
            break;
//        case SHAPE_CUBE:
//            float3 near = buff[upper].s123;
//            float3 far = buff[upper].s456;
//            object_intersect = intersect_cube(near, far, ray_pos, ray_dir, t);
//            break;

        default:
            printf("ERROR: Invalid shape.\n");
            //abort();
            break;
        }

        nearest_intersect_result = 
            object_intersect.type == MISS ? MISS : OUTER_HIT;

        if(object_intersect.dist <= t) {
            t = object_intersect.dist;
            obj_index = i;
            //nearest_intersect_result.type
        }
    }

    //float16 nearest_object = buf[obj_index];
    struct Intersection retval;
    retval.obj_index = obj_index;
    retval.dist = t;
    retval.type = nearest_intersect_result;

    struct TraceInfo return_data;
    
    return_data.color = retval.obj_index >= 0 ? (float4)buff[retval.obj_index].s0123 : black;
    return_data.next_ray_type = DONE;
    return return_data;
}
*/

struct Intersection intersect_cube(
    float3 cube_near, 
    float3 cube_far,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t)
{
//    float t1, t2, tnear = -1000.0f, tfar = 1000.0f, temp, tCube;
//    float3 b1 = cube_near;
//    float3 b2 = cube_far;
//    int intersectFlag = 1;
//    if(ray_dir == 0.f) {
//        if(ray_pos < b1 || ray_pos > b2)
//            intersectFlag = -1;
//    }
//
//    struct Intersection jieguo;
//    jieguo.dist = max_t;
//    jieguo.obj_normal = plane_normal;
//
//    float d = sqrt(dot(plane_pos, plane_pos));
//    float num = -d - dot(ray_pos, plane_normal);
//    float denom = dot(ray_dir, plane_normal);
//    if(denom == 0) {
//        jieguo.type = MISS;
//        return jieguo;
//    }
//
//    float t = num / denom;
//
//    if(t > 0.1f && t < max_t) {
//        jieguo.dist = t;
//        jieguo.type = OUTER_HIT;
//        return jieguo;
//    } else {
//        jieguo.type = MISS;
//        return jieguo;
//    }
    struct Intersection jieguo;
    return jieguo;
}

struct Intersection intersect_plane(
    float3 plane_pos, 
    float3 plane_normal,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t)
{
    //plane_pos = (float3)(0, -2, 0);
    //plane_normal = (float3)(0, 1, 0);

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
// implementation
struct Intersection intersect_sphere(
    float3 sphere_pos, 
    float sphere_radius,
    float3 ray_pos, 
    float3 ray_dir,
    float max_t)
{
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

struct TraceInfo intersect_scene(
    constant float16 *scene,
    int scene_size,
    float3 ray_pos,
    float3 ray_dir,
    enum RayType ray_type)
{
    // constant definitions
    color4 white  = (color4)(1.f, 1.f, 1.f, 1.f);
    color4 black  = (color4)(0.f, 0.f, 0.f, 1.f);
    color4 red    = (color4)(1.f, 0.f, 0.f, 1.f);
    color4 blue   = (color4)(0.f, 0.f, 1.f, 1.f);
    color4 yellow = (color4)(1.f, 1.f, 0.f, 1.f);
    color4 green  = (color4)(0.f, 1.f, 0.f, 1.f);
    float max_distance = 1000000.0f; // TODO make settable elsewhere

    struct Intersection nearest_intersection;
    nearest_intersection.dist = max_distance;
    nearest_intersection.type = MISS;
    nearest_intersection.obj_index = -1;
    nearest_intersection.obj_normal = (vec3)(0.f, 0.f, 0.f);
    for(int i = 0; i < scene_size; i+=2) {
        struct Intersection obj_intersection;
        obj_intersection.dist = nearest_intersection.dist;

        float16 upper = scene[i];

        //switch((int)upper.s0) {
        switch((int)scene[i].s0) {
            case SHAPE_SPHERE:
                obj_intersection = intersect_sphere((float3)upper.s123, upper.s4, ray_pos, ray_dir, nearest_intersection.dist);
                break;
            case SHAPE_PLANE:
                obj_intersection = intersect_plane((float3)upper.s123, (float3)upper.s456, ray_pos, ray_dir, nearest_intersection.dist);
                break;
            default:
                printf("Error: impossible shape\n");
                break;
        }

        if(obj_intersection.dist < nearest_intersection.dist && obj_intersection.dist > 0.01f) {
            nearest_intersection = obj_intersection;
            nearest_intersection.obj_index = i;
        }
    }

    struct TraceInfo traceinfo;
    traceinfo.ray_type = ray_type;

    if(nearest_intersection.type == MISS) {
        traceinfo.status = STOP;
        traceinfo.color = black;
        traceinfo.next_ray_type = INFINITE;
        return traceinfo;
    }

    //float mdot = dot(ray_dir, nearest_intersection.obj_normal);
    //mdot = mdot < 0 ? -mdot : mdot;
    //traceinfo.color = mdot * scene[nearest_intersection.obj_index+1].rgba;
    traceinfo.ray_dot = dot(ray_dir, nearest_intersection.obj_normal);
    traceinfo.ray_dot = traceinfo.ray_dot < 0 ? -traceinfo.ray_dot : traceinfo.ray_dot;
    traceinfo.color = traceinfo.ray_dot * scene[nearest_intersection.obj_index+1].rgba;

    int nearest_material = (int) scene[nearest_intersection.obj_index+1].s4; 
    if(ray_type == LIGHT_SEEK) {
        if(nearest_material == LIGHT) {
            traceinfo.next_ray_type = LIGHT_FOUND;
            traceinfo.status = STOP;
        } else {
            traceinfo.next_ray_type = SHADOW;
            traceinfo.status = STOP;
        }
    } else {
        if(nearest_material == MATTE) {
            traceinfo.next_ray_type = LIGHT_SEEK;
            traceinfo.status = CONTINUE;
        } else if(nearest_material == MIRROR) {
            traceinfo.next_ray_type = REFLECT;
            traceinfo.status = CONTINUE;
        } else if(nearest_material == LIGHT) {
            traceinfo.next_ray_type = LIGHT_FOUND;
            traceinfo.status = STOP;
        }
    }

    if(traceinfo.status == CONTINUE) {
        if(traceinfo.next_ray_type == LIGHT_SEEK) {
            float t = nearest_intersection.dist;
            float3 light_pos = scene[6].s123;
            traceinfo.next_ray_pos[0] = ray_pos + t * ray_dir;
            traceinfo.next_ray_dir[0] = normalize(light_pos - traceinfo.next_ray_pos[0]);
        } else if(traceinfo.next_ray_type == REFLECT) {
            float t = nearest_intersection.dist;
            vec3 normal = normalize(nearest_intersection.obj_normal);

            traceinfo.next_ray_pos[0] = ray_pos + t * ray_dir;
            traceinfo.next_ray_dir[0] = ray_dir - 2 * dot(ray_dir, normal) * normal;
        }
    }





    //if(nearest_intersection.type != MISS) {
    //    result = (float4)scene[nearest_intersection.obj_index].rgba;
    //    float ray_dot = dot(ray_dir, nearest_intersection.obj_normal);
    //    ray_dot = ray_dot < 0 ? -ray_dot : ray_dot;
    //    result *= ray_dot;
    //} else {
    //    result = black;
    //}

    //return_data.color = result;
    //return_data.next_ray_type = DONE;
   
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
        scene_objects[j].s123 += clamp(sin(scene_objects[j].s123), -0.016f, 0.016f);
        //scene_objects[j+1].s123 += clamp(sin(scene_objects[j].s123), -0.01, 0.01);
    }
}

color4 parse_color(struct TraceInfo *info, int size)
{
    //color4 color = (float4)(0,0,0,0);
//    int count = 1;
//    for(int i = 0; i < size; i++) {
//        if(info[i].status == STOP) {
//            break;
//        }
//        count++;
//    }

    for(int i = 0; i < size; i++) {
        //color = info[i].color;
        if(info[i].status == STOP) {
            if(info[i].next_ray_type == SHADOW) {
                return (float4)(0,0,0,0);
            }

            color4 light = info[i].color;
            color4 acc = (color4)(0,0,0,0);
            if(i > 0){
                for(int j = i-1; j >= 0; j--) {
                    acc += info[j].color / (float)i;
                }
                return clamp(acc, 0.f, 1.f);
            }
            return light;
        }
    }
    return (float4)(0,0,0,1);
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
        int size = 100;
        struct TraceInfo traceinfo[100];
        //struct pixel_status_t pxs;

        traceinfo[0] = intersect_scene(scene, scene_size, ray_pos[i], ray_dir[i], PRIMARY);
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
                next_ray_type);
        }

        color4 color = parse_color(traceinfo, size);
        //color4 color = traceinfo[0].color;
        pixel_board[i] = color;
//        pixel_board[i] = (float4)(1, 0, 0, 0);









//        pxs.s[0] = intersect_scene(scene, scene_size, ray_pos[i], ray_dir[i]);
//
//        int depth = 0;
//        while(pxs.s[depth].next_ray_type != DONE && depth < max_depth) {
//            int next = depth+1;
//            switch(pxs.s[depth].next_ray_type) {
//                case BLOCKED: pxs.s[next].next_ray_type = DONE;
//                             break;
//                case REFLECT: pxs.s[next] = intersect_scene(scene, scene_size, pxs.s[depth].refl_ray_pos, pxs.s[depth].refl_ray_dir);
//                              break;
//                case REFRACT: pxs.s[next] = intersect_scene(scene, scene_size, pxs.s[depth].refr_ray_pos, pxs.s[depth].refr_ray_dir);
//                              break;
//                case BOTH: pxs.s[next] = intersect_scene(scene, scene_size, pxs.s[depth].refl_ray_pos, pxs.s[depth].refl_ray_dir);
//                           pxs.s[next] = intersect_scene(scene, scene_size, pxs.s[depth].refr_ray_pos, pxs.s[depth].refr_ray_dir);
//                           break;
//                case DONE: pxs.s[next].next_ray_type = DONE;
//                           break;
//            }
//            depth = next;
//        }
//        color4 color = pxs.s[0].color;
 //       pixel_board[i] = color;
    }
}
