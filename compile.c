#include "compile.h"
#include <stdio.h>

void llist_append(struct llist *a_llist, struct node *a_node)
{
    if(a_llist->head == NULL) {
        a_llist->head = a_node;
        a_llist->tail = a_llist->head;
        a_llist->size = 1;
    } else {
        a_llist->tail->next = a_node;
        a_llist->tail = a_node;
        a_llist->size++;
    }
}

struct node *compile_sphere(const struct sphere *a_sphere)
{
    struct node *sphere_node = (struct node *)calloc(1, sizeof(struct node));

    sphere_node->upper.s[0] = SHAPE_SPHERE;
    sphere_node->upper.s[1] = a_sphere->position.s[0];
    sphere_node->upper.s[2] = a_sphere->position.s[1];
    sphere_node->upper.s[3] = a_sphere->position.s[2];
    sphere_node->upper.s[4] = a_sphere->radius;
    // 5 - 15 are padding

    sphere_node->lower.s[0] = a_sphere->color.s[0];
    sphere_node->lower.s[1] = a_sphere->color.s[1];
    sphere_node->lower.s[2] = a_sphere->color.s[2];
    sphere_node->lower.s[3] = a_sphere->color.s[3];
    sphere_node->lower.s[4] = a_sphere->material_type;
    // 5 - 15 are padding

    sphere_node->next = NULL;
    
    return sphere_node;
}

struct node *compile_plane(const struct plane *a_plane)
{
    struct node *plane_node = (struct node *)calloc(1, sizeof(struct node));

    plane_node->upper.s[0] = SHAPE_PLANE;
    plane_node->upper.s[1] = a_plane->position.s[0];
    plane_node->upper.s[2] = a_plane->position.s[1];
    plane_node->upper.s[3] = a_plane->position.s[2];
    plane_node->upper.s[4] = a_plane->normal.s[0];
    plane_node->upper.s[5] = a_plane->normal.s[1];
    plane_node->upper.s[6] = a_plane->normal.s[2];
    // 7 - 15 are padding

    plane_node->lower.s[0] = a_plane->color.s[0];
    plane_node->lower.s[1] = a_plane->color.s[1];
    plane_node->lower.s[2] = a_plane->color.s[2];
    plane_node->lower.s[3] = a_plane->color.s[3];
    plane_node->lower.s[4] = a_plane->material_type;
    // 5 - 15 are padding

    plane_node->next = NULL;

    return plane_node;
}

void llist_delete(struct llist *a_llist)
{
    while(a_llist->head) {
        struct node *jlvwnnoooo = a_llist->head;
        a_llist->head = a_llist->head->next;
        free(jlvwnnoooo);
    }

    free(a_llist);
}

//struct llist *get_compiled_scene(struct llist *a_llist)
//{
//    struct sphere my_sphere = {
//        .position = { 0.0f, 0.0f, 4.0f },
//        .radius = 1.0f,
//        .color = { 1.0f, 0.0f, 0.0f, 1.0f },
//        .material_type = MATTE
//    };
//
//    struct sphere my_light = {
//        .position = { 2.0f, 3.0f, 3.0f },
//        .radius = 0.3f,
//        .color = { 1.0f, 1.0f, 1.0f, 1.0f },
//        .material_type = LIGHT
//    };
//
//    struct
//}

