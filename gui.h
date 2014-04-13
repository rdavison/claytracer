#ifndef __RTCL_GUI_H
#define __RTCL_GUI_H
#include <SDL2/SDL.h>
#include <stdbool.h>

struct gui {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} gui;

int gui_init(int width, int height);
int gui_destroy();
bool gui_quit_pressed();

#endif
