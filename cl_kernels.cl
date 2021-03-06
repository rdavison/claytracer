#include "cl_kernels.h"
#include "shape_defines.h"
#include "motion.h"


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// function implementations
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


struct Intersection intersect_box(
    constant float16 *scene,
    int obj_index,
    float3 ray_pos,
    float3 ray_dir,
    float max_t)
{
    struct Intersection jieguo;
    jieguo.type = MISS;
    jieguo.dist = max_t;

    vec3 vmin = scene[obj_index].s456;
    vec3 vmax = scene[obj_index].s789;

    float tmin, tmax, tymin, tymax, tzmin, tzmax;
    if(ray_dir.x >= 0) {
        tmin = (vmin.x - ray_pos.x) / ray_dir.x;
        tmax = (vmax.x - ray_pos.x) / ray_dir.x;
    } else {
        tmin = (vmax.x - ray_pos.x) / ray_dir.x;
        tmax = (vmin.x - ray_pos.x) / ray_dir.x;
    }
    if(ray_dir.y >= 0) {
        tymin = (vmin.y - ray_pos.y) / ray_dir.y;
        tymax = (vmax.y - ray_pos.y) / ray_dir.y;
    } else {
        tymin = (vmax.y - ray_pos.y) / ray_dir.y;
        tymax = (vmin.y - ray_pos.y) / ray_dir.y;
    }
    if((tmin > tymax) || (tymin > tmax)) {
        return jieguo;
    }
    if(tymin > tmin) {
        tmin = tymin;
    }
    if(tymax < tmax) {
        tmax = tymax;
    }
    if(ray_dir.z >= 0) {
        tzmin = (vmin.z - ray_pos.z) / ray_dir.z;
        tzmax = (vmax.z - ray_pos.z) / ray_dir.z;
    } else {
        tzmax = (vmax.z - ray_pos.z) / ray_dir.z;
        tzmin = (vmin.z - ray_pos.z) / ray_dir.z;
    }
    if((tmin > tzmax) || (tzmin > tmax)) {
        return jieguo;
    }
    if(tzmin > tmin) {
        tmin = tzmin;
    }
    if(tzmax < tmax) {
        tmax = tzmax;
    }
    
    jieguo.type = OUTER_HIT;
    jieguo.dist = tmin;

    // calculate normal
    //vec3 center = scene[obj_index].s123;
    vec3 point = ray_pos + jieguo.dist * ray_dir;
    float eps = 0.01;
    //float minimum = 1000000.0;
    if(fabs(point.x - vmin.x) < eps)
        jieguo.obj_normal = (vec3)(-1.f,0.f,0.f);
    else if(fabs(point.x - vmax.x) < eps)
        jieguo.obj_normal = (vec3)(1.f,0.f,0.f);
    else if(fabs(point.y - vmin.y) < eps)
        jieguo.obj_normal = (vec3)(0.f,-1.f,0.f);
    else if(fabs(point.y - vmax.y) < eps)
        jieguo.obj_normal = (vec3)(0.f,1.f,0.f);
    else if(fabs(point.z - vmin.z) < eps)
        jieguo.obj_normal = (vec3)(0.f,0.f,1.f);
    else if(fabs(point.z - vmax.z) < eps)
        jieguo.obj_normal = (vec3)(0.f,0.f,-1.f);

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
            case SHAPE_BOX:
                obj_intersection = intersect_box(
                                        scene,
                                        obj_index,
                                        ray_pos,
                                        ray_dir,
                                        nearest->dist);
                break;
            default:
                //exit(-1);
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
    info->blocked = false;
    info->light_dir = (vec3)(0,0,0);
    info->obj_normal = (vec3)(0,0,0);
    info->refl_dir = (vec3)(0,0,0);
    info->viewer_dir = (vec3)(0,0,0);
    info->material = 0;
    for(int i = 0; i < 3; i++) {
        info->next_ray_pos[i] = (vec3)(0,0,0);
        info->next_ray_dir[i] = (vec3)(0,0,0);
    }
}

bool is_blocked(
    constant float16 *scene,
    int scene_size,
    float3 ray_pos,
    float3 ray_dir,
    float max_t,
    int self_index)
{
    for(int obj_index = 2; obj_index < scene_size; obj_index+=2) {
        if(obj_index == self_index) {
            continue;
        }
        struct Intersection obj_intersection;
        Intersection_init(&obj_intersection);

        //obj_intersection.dist = max_t;

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
                                        max_t);
                break;
            case SHAPE_PLANE:
                obj_intersection = intersect_plane(
                                        scene,
                                        obj_index,
                                        ray_pos,
                                        ray_dir,
                                        max_t);
                break;
            case SHAPE_BOX:
                obj_intersection = intersect_box(
                                        scene,
                                        obj_index,
                                        ray_pos,
                                        ray_dir,
                                        max_t);
            default:
                //printf("Error: impossible shape\n");
                //exit(-1);
                break;
        }

        // save nearest intersection
        if(obj_intersection.dist < max_t) {
            return true;
        }
    }
    return false;
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
    color4 intensity = (color4)(1,1,1,1); // TODO make settable elsewhere
    float max_distance = 1000000.0f; // TODO make settable elsewhere

    struct Intersection nearest_intersection;
    find_nearest_intersection(scene, scene_size, ray_pos, ray_dir, &nearest_intersection);

    vec3 point = ray_pos + nearest_intersection.dist * ray_dir;
    vec3 light_pos = scene[0].s123;
    vec3 light_dir = normalize(light_pos - point);
    float light_dist = distance(point, light_pos);
    

    struct Intersection light_intersection = intersect_sphere(
        scene,
        0,
        point,
        light_dir,
        light_dist);

    bool blocked = is_blocked(
        scene,
        scene_size,
        point,
        light_dir,
        light_dist,
        light_intersection.dist);

    struct TraceInfo traceinfo;
    TraceInfo_init(&traceinfo);

    traceinfo.ray_type = ray_type;
    traceinfo.blocked = blocked;

    float cos_theta = dot(light_dir, nearest_intersection.obj_normal);
    traceinfo.light_arriving = (intensity * cos_theta) / (4.f * 3.141592f * pow(light_intersection.dist, 2)) ;

    traceinfo.light_dir = light_dir;
    traceinfo.obj_normal = nearest_intersection.obj_normal;
    vec3 origin = (vec3)(0,0,0);
    traceinfo.viewer_dir = normalize(origin - point);
    traceinfo.refl_dir = normalize(2 * dot(light_dir, traceinfo.obj_normal)*traceinfo.obj_normal - light_dir);

    switch(nearest_intersection.type) {
        case MISS:
            traceinfo.status = STOP;
            traceinfo.color = (color4)(0,0,0,0);
            traceinfo.next_ray_type = INFINITE;
            return traceinfo;

        case OUTER_HIT:
        case INNER_HIT:
        default:
            //traceinfo.dist = nearest_intersection.dist;
            //traceinfo.ray_dot = dot(-ray_dir, nearest_intersection.obj_normal);
            //traceinfo.fade_amount = (light_power * traceinfo.ray_dot) / (4.f * 3.141592f * pow(traceinfo.dist, 2));
            //traceinfo.fade_amount *= pow(traceinfo.dist, 3.f/2.f);
            //traceinfo.fade_amount = clamp(traceinfo.fade_amount * 100, 0.f, 1.f);
            //traceinfo.fade_amount = 1.0f - traceinfo.fade_amount;
            //traceinfo.fade_amount = smoothstep(0.f, 1.f, traceinfo.fade_amount * 5000);
            traceinfo.color = scene[nearest_intersection.obj_index+1].rgba;
            break;
    }

    int nearest_material = (int) scene[nearest_intersection.obj_index+1].s4; 
    traceinfo.material = nearest_material;
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
        traceinfo.color = (color4)(0,0,0,0);
    }

    if(traceinfo.status == CONTINUE) {

        float t = nearest_intersection.dist;
        float3 light_pos = scene[0].s123;
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
    const int num_objects,
    const int frame_num)
{
    int i = get_global_id(0);

    if(i < 7) {
        return;
    }

    if(i < num_objects) {
        int sign = 1.f;
        unsigned int j = i * 2;
        float delta = sin((float)frame_num/4.f)/4.f;
        delta = i % 2 == 0 ? delta : -delta;
        scene_objects[j].s1 += delta;
        //scene_objects[j].s123 += clamp(sin(1.f/(scene_objects[j].s123)), -delta, delta);
        //scene_objects[j].s1 += clamp(sin((float)frame_num), (float)-i, (float)i);
        //scene_objects[j].s2 += clamp(tan((float)frame_num), -0.1f, 0.1f);
        //scene_objects[j].s3 += clamp(cos((float)frame_num), -0.1f, 0.1f);
    }
}

color4 phong_illumination(struct TraceInfo *info, int i)
{
    if(info[i].blocked) {
        return (color4)(0,0,0,0);
    }
    float Ka = 0.25f;
    float Kd = 0.55f;
    float Ks = 0.2f;
    float a = 12;

    color4 Ia = info[i].color;
    color4 Is = (color4)(1,1,1,1);
    color4 Id = (color4)(1,1,1,1);

    float L_dot_N = dot(info[i].light_dir, info[i].obj_normal);
    float R_dot_V = dot(info[i].refl_dir, info[i].viewer_dir);

    color4 Ip = Ka * Ia + ((Kd * L_dot_N * Id) + (Ks * pow(R_dot_V, a) * Is));
    return Ip;
}

color4 parse_color(struct TraceInfo *info, int max_size)
{                                                            
    int trace_size = 1;
    for(int i = 0; i < max_size; i++) {
        if(info[i].status == STOP) {                                   
            break;
        }
        trace_size++;
    }

    //int stop = 2;
    //trace_size= trace_size > stop ? stop : trace_size;

    //color4 acc = (color4)(0,0,0,0);
    //float4 derp = smoothstep(0.f, 1.f, info[trace_size-2].light_arriving * 10000.f);
    float4 derp = (color4)(1,1,1,1);
    if(trace_size > 1) {
        derp = clamp(sqrt(info[trace_size-2].light_arriving)*24.f, 0.f, 1.f);
    }
    color4 acc = (color4)(0,0,0,0);
    //color4 acc = derp;
    for(int i = 0; i < trace_size; i++) {
        acc += info[i].color / (float)trace_size;
        if(i == trace_size-1) {
            acc *= derp;
        }
    }
//    for(int i = 0; i < trace_size; i++) {
//        if(info[i].material == MATTE) {
//            acc += phong_illumination(info, i);
//        } else if(info[i].material == MIRROR) {
//            //trace_size -= 1;
//            //acc += info[i].color / (float)trace_size;
//            //if(i == trace_size-1) {
//            //    acc *= derp;
//            //}
//        } else if(info[i].material == LIGHT) { 
//        }
//    }

//    for(int i = 0; i < max_size; i++) {
//        if(info[i].status == STOP) {
//
//            color4 light = info[i].color;
//            color4 acc = (color4)(0,0,0,0);
//
//            //color4 fade = smoothstep(0.f, 1.f, info[0].fade_amount * 5000);
//            //color4 fade = smoothstep(0.f, 1.f, info[0].fade_amount * 1000);
//            if(i > 0) {
//                for(int j = i; j >= 0; j--) {
//                    //color4 fade = clamp(info[j].fade_amount * 1000, 0.f, 1.f);
//                    color4 fade = info[j].fade_amount;
//                    //fade = length(info[j].fade_amount) > length(fade) ? info[j].fade_amount : fade;
//                    float m = 1.0f;
//
//            
//
//                    if(info[j].next_ray_type == SHADOW) {
//                        acc += 0.f / (float)(i+1);
//                        continue;
//                    } 
//                    
//                    else if(info[j].ray_type == REFLECT && 
//                            info[j-1].next_ray_type == REFLECT) {
//                        //acc += fade*((0.00 * info[j-1].color)+(1.0f * info[j].color)) / (float)(i+1);
//                        //acc += fade*(info[j].color) / (float)(i+1);
//                        
//                        //acc = (color4)(0,0,0,0);
//                        continue;
//                    } 
//
//                    else {
//                        acc += (info[j].color) / (float)(i+1);
//                        //acc = (color4)(0,1,0,0);
//                        //acc *= fade;
//                    }
//                }
//                return acc;
//            }
//            return light;
//        }
//    }
    return acc;
}


kernel void trace_rays(
    constant float16 *scene,  // read only
    constant float16 *lights,  // read only  // TODO IMPLEMENT
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
        pixel_board[i] = color;
        //pixel_board[i] = smoothstep(0.f, 1.f, color);
    }
}


kernel void translate_world(
    global float16 *scene,
    const int count,
    const int direction,
    const float amount)
{
    unsigned int i = get_global_id(0);
    if(i < count) {
        i *= 2;
        switch(direction) {
        case MOVE_DIR_UP:
            scene[i].s2 -= amount;
            break;
        case MOVE_DIR_DOWN:
            scene[i].s2 += amount;
            break;
        case MOVE_DIR_LEFT:
            scene[i].s1 += amount;
            break;
        case MOVE_DIR_RIGHT:
            scene[i].s1 -= amount;
            break;
        case MOVE_DIR_FORWARD:
            scene[i].s3 += amount;
            break;
        case MOVE_DIR_BACKWARD:
            scene[i].s3 -= amount;
            break;
        }
    }
}
    
