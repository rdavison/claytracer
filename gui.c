#include "gui.h"

int gui_init(int width, int height, int scale)
{
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO);

    gui.window = SDL_CreateWindow(
        "[OpenCL] Inefficiently Raytraced!",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width * scale,
        height * scale,
        SDL_WINDOW_ALLOW_HIGHDPI);

    gui.renderer = SDL_CreateRenderer(
        gui.window,
        -1,
        SDL_RENDERER_ACCELERATED);

    gui.texture = SDL_CreateTexture(
        gui.renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING,
        width * scale,
        height * scale);

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
    SDL_Event event;
    SDL_PollEvent(&event);
    switch(event.type) {

    case SDL_WINDOWEVENT:
        switch(event.window.event) {

        case SDL_WINDOWEVENT_ENTER:
            printf("entered\n");
            break;

        case SDL_WINDOWEVENT_LEAVE:
            printf("left\n");
            break;
        }
        break;

    case SDL_KEYDOWN:
        printf("key pressed: %d\n", event.key.keysym.sym);
        break;
    }
    return event.type == SDL_QUIT;
}
