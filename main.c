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
    int width = 320, height = 320;
    //int width = 160, height = 120;
    //int width = 640, height = 480;
    //int width = 1920, height = 1080;
    int recursion_depth = 1;
    int pitch = 4;
    int scale = 4;
    int renderer_flags = RENDERER_USE_OPENCL_RENDERER;

    struct timeval time_start;
    struct timeval time_end;
    struct timeval time_diff;

    // init GUI
    gui_init(width, height, scale);


    // init Renderer
    renderer_init(recursion_depth, renderer_flags, width, height, pitch);

    // pixel board
    struct pixel *pixel_board;
    for(int i = 0; i < 500; i++) {
        if(gui_quit_pressed()) {
            break;
        }

        // get start time
        gettimeofday(&time_start, NULL);
        //time_print(&time_start);

        renderer_render(&pixel_board);
        
        Uint32 frame[width*height*scale*scale];
        int p = 0;
        for(int y = 0; y < height; y++) {
            for(int m = 0; m < scale; m++) {
                for(int n = 0; n < width; n++) {
                    for(int q = 0; q < scale; q++) {
                        frame[p+q] = ((int)floor(pixel_board[n + width * y].color.s[c_R]*255)) << 16 |
                                   ((int)floor(pixel_board[n + width * y].color.s[c_G]*255)) << 8 |
                                   ((int)floor(pixel_board[n + width * y].color.s[c_B]*255));
                    }
                    p+=scale;
                }
            }
        }

        SDL_UpdateTexture(gui.texture, NULL, frame, width*4*scale);

        // get end time
        gettimeofday(&time_end, NULL);
        //time_print(&time_end);

        // get time difference
        time_subtract(&time_diff, &time_end, &time_start);
        //printf("%d.%06d\n", (int)time_diff.tv_sec, (int)time_diff.tv_usec);

        SDL_RenderClear(gui.renderer);
        SDL_RenderCopy(gui.renderer, gui.texture, NULL, NULL);
        SDL_RenderPresent(gui.renderer);
        renderer_update_scene();
    }


    // destroy GUI
    gui_destroy();
    
    return 0;
}
