#include "gui.h"

int gui_init(int width, int height)
{
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO);

    gui.window = SDL_CreateWindow(
        "[OpenCL] Inefficiently Raytraced!",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_ALLOW_HIGHDPI);

    gui.renderer = SDL_CreateRenderer(
        gui.window,
        -1,
        SDL_RENDERER_ACCELERATED);

    gui.texture = SDL_CreateTexture(
        gui.renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height);

    return 0;
}

int gui_destroy()
{
    SDL_DestroyTexture(gui.texture);
    SDL_DestroyRenderer(gui.renderer);
    SDL_DestroyWindow(gui.window);
    
    return 0;
}

bool gui_quit_pressed() {
    SDL_Event e;
    SDL_PollEvent(&e);
    return e.type == SDL_QUIT;
}
