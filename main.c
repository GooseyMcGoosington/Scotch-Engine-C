#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"
#include "raster.h"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

#include <omp.h>
#include <xmmintrin.h>
#define PREFETCH_PIXELS(addr) _mm_prefetch((const char*)addr, _MM_HINT_T0)
#define ALIGNMENT 64
#define PREFETCH_PIXELS(addr) _mm_prefetch((const char*)addr, _MM_HINT_T0)

player character = {20.0, 10.0, 0.0, 5.0, 0.0, 70.0, 90.0, 0.0};

Level *level = NULL;

float sn[360];
float cs[360];

portalCull portalBounds = {1.0f, (float)SW1, 1.0f, 1.0f, SH1, SH1, SW1-1.0f};

int fI = 0;
double fAvg = 0;

static void optimized_blt_and_update(SDL_Surface *rgb565_surface, SDL_Surface *surface, SDL_Window *window) {
    int width = rgb565_surface->w;
    int height = rgb565_surface->h;

    Uint16 *src_pixels = (Uint16*)rgb565_surface->pixels;
    Uint32 *dst_pixels = (Uint32*)surface->pixels;

    Uint16 *src = src_pixels;
    Uint32 *dst = dst_pixels;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) { // Process two pixels per iteration
            // Extract RGB565 values
            Uint16 src_pixel = *src++;

            // Convert first pixel
            int r1 = (src_pixel >> 11) & 0x1F; 
            int g1 = (src_pixel >> 5) & 0x3F;
            int b1 = src_pixel & 0x1F;

            Uint32 pixel = 0xFF000000 |
                (r1*524288) |
                (g1*1024)  |
                (b1*8);

            *dst++ = pixel;
        }
    }

    // Update the window surface with the new pixel data
    SDL_UpdateWindowSurface(window);
}

int main(int argc, char* argv[]) {
    float f = character.fov*PI/180;
    float tanFOV = tan(f/2);
    float focalLength = SW2/tanFOV;
    character.focalLength = focalLength;
    character.fovWidth = tanFOV;
    
    for (int i = 0; i < 360; i++) {
        double a = (double)(i) * PI / 180;
        sn[i] = sin(a);
        cs[i] = cos(a);
    }

    level = malloc(sizeof(Level) + 1 * sizeof(sector *));
    if (!level) {
        perror("Failed to allocate level");
        return -1;
    }
    level->count = 2;

    level->sectors[0] = malloc(sizeof(sector) + 6 * sizeof(wall));
    sector *sect = level->sectors[0];
    sect->count = 6;
    sect->height = 10.0;
    sect->elevation = 0.0;
    sect->cIndex = 1;
    sect->fIndex = 1;
    sect->walls[0] = (wall){10, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    sect->walls[1] = (wall){40, 30, 0, 0, 0, 0, 0, 0, 0, 0};
    sect->walls[2] = (wall){0, 0, 0, 40, 0, 0, 0, 0, 0, 0};
    sect->walls[3] = (wall){40, 40, 40, 0, 0, 0, 0, 0, 0, 0};
    sect->walls[4] = (wall){0, 40, 40, 40, 0, 0, 0, 0, 0, 0};
    sect->walls[5] = (wall){30, 10, 0, 0, 1, 2, 2, 1, 0, 0};

    level->sectors[1] = malloc(sizeof(sector) + 4 * sizeof(wall));
    sector *sect1 = level->sectors[1];
    sect1->count = 4;
    sect1->height = 6.0;
    sect1->elevation = 2.0;
    sect1->cIndex = 0;
    sect1->fIndex = 0;
    sect1->walls[0] = (wall){40, 10, -15, -15, 1, 0, 0, 2, 0, 1};
    sect1->walls[1] = (wall){10, 10, -15, 0, 0, 0, 0, 0, 0, 1};
    sect1->walls[2] = (wall){30, 40, 0, -15, 0, 0, 0, 0, 0, 1};
    sect1->walls[3] = (wall){10, 30, 0, 0, 0, 0, 0, 0, 0, 1};

    level->sectors[2] = malloc(sizeof(sector) + 4 * sizeof(wall));
    sector *sect2 = level->sectors[2];
    sect2->count = 4;
    sect2->height = 6.0;
    sect2->elevation = 2.0;
    sect2->cIndex = 1;
    sect2->fIndex = 1;
    sect2->walls[0] = (wall){50, 10, -35, -35, 0, 0, 0, 0, 0, 0};
    sect2->walls[1] = (wall){10, 10, -35, -15, 0, 0, 0, 0, 0, 0};
    sect2->walls[2] = (wall){50, 50, -15, -35, 0, 0, 0, 0, 0, 0};
    sect2->walls[3] = (wall){10, 50, -15, -15, 0, 0, 0, 0, 0, 0};
    
    size_t lt = level->count + 1;
    for (int x = 0; x<lt; x++) {
        sector *Sector = level->sectors[x];
        size_t wC = Sector->count;

        for (int y = 0; y<wC; y++) {
            float x0 = (float)Sector->walls[y].x0;
            float x1 = (float)Sector->walls[y].x1;
            float y0 = (float)Sector->walls[y].y0;
            float y1 = (float)Sector->walls[y].y1;
            float dx = x1-x0;
            float dy = y1-y0;
            float dst = sqrtf(dx*dx + dy*dy);

            //printf("%.4f %s", dst, " ");
            Sector->walls[y].length = dst;
        }
    }

    SDL_Window *window = SDL_CreateWindow("SDL2 Software Renderer Example",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SW, SH, SDL_WINDOW_SHOWN);

    SDL_Surface *surface = SDL_GetWindowSurface(window);
    SDL_Surface *rgb565_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGB565, 0);

    SDL_Event event;
    int quit = 0;
    
    //SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

    init_textures();
    while (!quit) {
        struct timeval begin, end;
        gettimeofday(&begin, NULL);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
        }

        int yaw = character.yaw;
        float pSn = sn[yaw];
        float pCs = cs[yaw];
        character.yaw = 180;//+= 0.1;
        if (character.yaw < 0) character.yaw = 360;
        if (character.yaw > 360) character.yaw = 0;
        startDrawSector((Uint16 *)rgb565_surface->pixels, level, level->sectors[0], character, pSn, pCs, portalBounds);
        optimized_blt_and_update(rgb565_surface, surface, window);
        gettimeofday(&end, NULL);
        double time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1000000.0;

        fI++;
        fAvg += time_spent;
        if (fI == 240) {
            char str[10];
            fAvg /= fI;
            sprintf(str, "%.4f", (float)(1/fAvg));
            SDL_SetWindowTitle(window, str);
            fI = 0;
            fAvg = 0;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

//clang -std=c17 main.c -O3 -ffast-math -flto -march=native -funroll-loops -ftree-vectorize -IC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2 -LC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2\lib -Wall -lmingw32 -lSDL2main -lSDL2 -fuse-ld=lld -o main
