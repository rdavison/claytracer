#ifndef __RTCL_COMPILE_H
#define __RTCL_COMPILE_H

#define RTCL_PRIMITIVE_ENTRY_SIZE (32)

#include "shape_defines.h"
#include "raytracer.h"

struct node {
    cl_float16 upper;
    cl_float16 lower;
    struct node *next;
};

struct llist {
    struct node *head;
    struct node *tail;
    unsigned int size;
};

void llist_append(struct llist *a_llist, struct node *a_node);
void llist_delete(struct llist *a_llist);
struct node *compile_sphere( const struct sphere *a_sphere);
struct node *compile_plane(const struct plane *a_plane);
struct llist *get_compiled_scene(struct llist *a_llist);

#endif
