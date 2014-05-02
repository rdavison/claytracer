#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>

#include "rtcl.h"
#include "renderer.h"
#include "util.h"
#include "gui.h"


int main(int argc, char *argv[])
{
    //int width = 1440, height = 852;
    //int width = 320, height = 240;
    int width = 640, height = 480;
    int recursion_depth = 1;
    int pitch = 4;
    int renderer_flags = RENDERER_USE_OPENCL_RENDERER;

    struct timeval time_start;
    struct timeval time_end;
    struct timeval time_diff;

    // init GUI
    gui_init(width, height);


    // init Renderer
    renderer_init(recursion_depth, renderer_flags, width, height, pitch);

    // pixel board
    struct pixel *pixel_board;
    for(int i = 0; i < 100; i++) {
        if(gui_quit_pressed()) {
            break;
        }

        // get start time
        gettimeofday(&time_start, NULL);
        time_print(&time_start);

        renderer_render(&pixel_board);
        
        Uint32 frame[width*height];
        for(Uint32 i = 0; i < width*height; i++) {
            //int x = i % width;
            //int y = i / width;
            frame[i] = ((int)floor(pixel_board[i].color.s[c_R]*255)) << 16 |
                       ((int)floor(pixel_board[i].color.s[c_G]*255)) << 8 |
                       ((int)floor(pixel_board[i].color.s[c_B]*255));
        }
        SDL_UpdateTexture(gui.texture, NULL, frame, width*4);

        // get end time
        gettimeofday(&time_end, NULL);
        time_print(&time_end);

        // get time difference
        time_subtract(&time_diff, &time_end, &time_start);
        printf("%ld.%06ld\n", time_diff.tv_sec, time_diff.tv_usec);

        SDL_RenderClear(gui.renderer);
        SDL_RenderCopy(gui.renderer, gui.texture, NULL, NULL);
        SDL_RenderPresent(gui.renderer);
        renderer_update_scene();
    }


    // destroy GUI
    gui_destroy();
    
    return 0;
}
