#include <SDL2/SDL.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>

#include "rtcl.h"
#include "renderer.h"
#include "util.h"

int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}

void timeval_print(struct timeval *tv)
{
    char buffer[30];
    time_t curtime;

    printf("%ld.%06ld", tv->tv_sec, tv->tv_usec);
    curtime = tv->tv_sec;
    strftime(buffer, 30, "%m-%d-%Y  %T", localtime(&curtime));
    printf(" = %s.%06ld\n", buffer, tv->tv_usec);
}
int InitWindow(SDL_Window **win, SDL_Renderer **ren, SDL_Texture **tex) {
    int pxscale = 30;
    int posX = (1920 - 320) / 2, posY = (1080 - 240) / 2, width = 16 * pxscale, height = 9 * pxscale;

    posX = 0;
    posY = 0;
    width = 320;
    height = 240;
    (*win) = SDL_CreateWindow(
                "[OpenCL] Inefficiently Raytraced!", 
                SDL_WINDOWPOS_CENTERED, 
                SDL_WINDOWPOS_CENTERED, 
                width, 
                height, 
                SDL_WINDOW_ALLOW_HIGHDPI);
    (*ren) = SDL_CreateRenderer(
                *win, 
                -1, 
                SDL_RENDERER_ACCELERATED);
    (*tex) = SDL_CreateTexture(
                *ren, 
                SDL_PIXELFORMAT_RGB888, 
                SDL_TEXTUREACCESS_STREAMING, 
                width, 
                height);
    return 0;
}

int DestroyWindow(SDL_Window *win, SDL_Renderer *ren, SDL_Texture *tex) {
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    return 0;
}

bool quitGame() {
    SDL_Event e;
    SDL_PollEvent(&e);
    return e.type == SDL_QUIT;
}

int main(int argc, char *argv[])
{
    struct timeval tvBegin, tvEnd, tvDiff;
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *sdlWindow;
    SDL_Renderer *sdlRenderer;
    SDL_Texture *sdlTexture;
    InitWindow(&sdlWindow, &sdlRenderer, &sdlTexture);

    int recursion_depth = 1;
    int width = 320;                                          
    int height = 240;
    int pitch = 4;

    // begin
    gettimeofday(&tvBegin, NULL);
    timeval_print(&tvBegin);

    renderer_init(recursion_depth, RENDERER_USE_OPENCL_RENDERER, width, height, pitch);

    struct pixel *pixel_board;
    renderer_render(&pixel_board);
    
    Uint32 frame[width*height];
    for(Uint32 i = 0; i < width*height; i++) {
        int x = i % width;
        int y = i / width;
        frame[i] = ((int)floor(pixel_board[i].color.s[c_R]*255)) << 16 |
                   ((int)floor(pixel_board[i].color.s[c_G]*255)) << 8 |
                   ((int)floor(pixel_board[i].color.s[c_B]*255));
    }
    SDL_UpdateTexture(sdlTexture, NULL, frame, width*4);

    //end
    gettimeofday(&tvEnd, NULL);
    timeval_print(&tvEnd);

    //diff
    timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
    printf("%ld.%06ld\n", tvDiff.tv_sec, tvDiff.tv_usec);

    SDL_RenderClear(sdlRenderer);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);

    while(!quitGame()) {
        // waste CPU
    }

    DestroyWindow(sdlWindow, sdlRenderer, sdlTexture);
    
    return 0;
}
