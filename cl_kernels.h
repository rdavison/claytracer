#define MATTE 1
#define MIRROR 2
#define LIGHT 3
#define MAX_TRACE_DEPTH 5 // > 5 maxing out private memory? TODO: fix this

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// typedefs, enums, and structs
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

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
    bool                        blocked;
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
    constant float16 *scene,
    int obj_index,
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

struct Intersection Intersection_init(struct Intersection *intersection);

void find_nearest_intersection(
    constant float16 *scene,
    int scene_size,
    vec3 ray_pos,
    vec3 ray_dir,
    struct Intersection *nearest);

void TraceInfo_init(struct TraceInfo *info);

kernel void generate_rays(
    const float3 camera_pos,
    const float3 camera_dir,
    const float camera_focal_length,
    const int width,
    const int height,
    global float3 *output_rays_pos,
    global float3 *output_rays_dir,
    const unsigned int count);

kernel void update_scene(
    global float16 *scene_objects, 
    constant float3 *update_pos,
    const int num_objects);

color4 parse_color(struct TraceInfo *info, int size);

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
    const unsigned int count);
