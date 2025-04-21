#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"
#include "raster.h"
//#include "gui.c"
#include "ini_parser.c"
//#include "config.c"

//#include "config.h"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

player character = {20.0, -10.0, 0.0, 5.0, 90.0, 70.0, 90.0, 0.0};
Level *level = NULL;

float sn[360];
float cs[360];
int control_locked = 0;
int fI = 0;
int inputs[256];

double fAvg = 0;

static inline void optimized_blt_and_update(SDL_Surface *rgb565_surface, SDL_Surface *surface, SDL_Window *window) {
    int width = rgb565_surface->w;
    int height = rgb565_surface->h;

    Uint16 *src_pixels = (Uint16*)rgb565_surface->pixels;
    Uint32 *dst_pixels = (Uint32*)surface->pixels;

    Uint16 *src = src_pixels;
    Uint32 *dst = dst_pixels;
    //G_HANDLE_IMAGE(rgb565_surface, window);
    for (int i = 1; i < (width*height); i++) {
        Uint16 src_pixel = *src++;
        int r1 = (src_pixel >> 11) & 0x1F; 
        int g1 = (src_pixel >> 5) & 0x3F;
        int b1 = src_pixel & 0x1F;

        Uint32 pixel = 0xFF000000 |
            (r1<<19) |
            (g1<<10)  |
            (b1<<3);

        *dst++ = pixel;
    }
    //G_HANDLE_IMAGE(rgb565_surface, window, &texture_list);
    //G_HANDLE(surface, window, inputs);
    // Update the window surface with the new pixel data
    SDL_UpdateWindowSurface(window);
};

int isChrWithinConvexSector() {
    int i = 0;
    for (int s=0; s<(level->count); s++) {
        sector *Sector = level->sectors[s];
        int inside = 1;
        wall *walls = Sector->walls;
        size_t wallCount = Sector->count;
        for (int w=0; w<wallCount; w++) {
            wall Wall = walls[w];
            float cross = (Wall.x1-Wall.x0) * (character.y-Wall.y0) - (Wall.y1-Wall.y0) * (character.x-Wall.x0);
            if (cross > 0) {
                inside = 0;
            }
        }
        if (inside == 1) {
            return i;
        }
        i ++;
    }
    return -1;
}

int main(int argc, char* argv[]) {
    // Initialize Code
    INI_PARSE("settings.ini");
    CFG_Init(INI_FIND_VALUE("width"), INI_FIND_VALUE("height"));
    R_INIT();
    portalCull portalBounds = {1.0f, (float)SW1, 1.0f, 1.0f, SH1, SH1, SW1-1.0f};
    //
    float f = DEG2RAD(character.fov);
    float tanFOV = tan(f/2);
    float focalLength = SW2/tanFOV;
    character.focalLength = focalLength;
    character.fovWidth = tanFOV;
    
    int quit = 0;

    for (int i = 0; i < 360; i++) {
        double a = (double)(i) * PI / 180;
        sn[i] = sin(a);
        cs[i] = cos(a);
    }
    for (int i = 0; i<256; i++) {
        inputs[i] = 0;
    }

    level = malloc(sizeof(Level) + 3 * sizeof(sector *));
    level->count = 3;

    level->sectors[0] = malloc(sizeof(sector) + 6 * sizeof(wall));
    sector *sect = level->sectors[0];
    sect->count = 6;
    sect->height = 10.0;
    sect->elevation = 0.0;
    sect->cIndex = 1;
    sect->fIndex = 1;

    sect->walls[0] = (wall){10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    sect->walls[1] = (wall){40, 30, 0, 0, 0, 0, 0, 0, 0, 0, 2};
    sect->walls[2] = (wall){0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 3};
    sect->walls[3] = (wall){40, 40, 40, 0, 0, 0, 0, 0, 0, 0, 4};
    sect->walls[4] = (wall){0, 40, 40, 40, 0, 0, 0, 0, 0, 0, 5};
    sect->walls[5] = (wall){30, 10, 0, 0, 1, 2, 2, 1, 0, 0, 6};

    level->sectors[1] = malloc(sizeof(sector) + 4 * sizeof(wall));
    sector *sect1 = level->sectors[1];
    sect1->count = 4;
    sect1->height = 6.0;
    sect1->elevation = 2.0;
    sect1->cIndex = 0;
    sect1->fIndex = 0;

    sect1->walls[0] = (wall){40, 10, -15, -15, 1, 0, 0, 2, 0, 1, 7};
    sect1->walls[1] = (wall){10, 10, -15, 0, 0, 0, 0, 0, 0, 1, 8};
    sect1->walls[2] = (wall){30, 40, 0, -15, 0, 0, 0, 0, 0, 1, 9};
    sect1->walls[3] = (wall){10, 30, 0, 0, 1, 0, 0, 0, 0, 0, 10};

    level->sectors[2] = malloc(sizeof(sector) + 4 * sizeof(wall));
    sector *sect2 = level->sectors[2];
    sect2->count = 4;
    sect2->height = 24.0;
    sect2->elevation = 2.0;
    sect2->cIndex = 1;
    sect2->fIndex = 1;

    sect2->walls[0] = (wall){50, 10, -35, -35, 0, 0, 0, 0, 0, 0, 11};
    sect2->walls[1] = (wall){10, 10, -35, -15, 0, 0, 0, 0, 0, 0, 12};
    sect2->walls[2] = (wall){50, 50, -15, -35, 0, 0, 0, 0, 0, 0, 13};
    sect2->walls[3] = (wall){10, 50, -15, -15, 0, 0, 0, 0, 0, 0, 14};
    
    size_t lt = level->count;
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
    
    //SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
    //G_INIT();
    init_textures();

    while (!quit) {
        struct timeval begin, end;
        gettimeofday(&begin, NULL);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
            if (event.type == SDL_KEYDOWN) {
                inputs[event.key.keysym.scancode] = 1;
                if (control_locked == 0) {
                    if (event.key.keysym.sym == SDLK_UP) {
                        float a = DEG2RAD(character.yaw);
                        character.x += 1*sin(a);
                        character.y += 1*cos(a);
                    }
                    if (event.key.keysym.sym == SDLK_DOWN) {
                            float a = DEG2RAD(character.yaw);
                            character.x -= 1*sin(a);
                            character.y -= 1*cos(a);
                    }
                    if (event.key.keysym.sym == SDLK_LEFT) {
                            character.yaw -= 3;
                    }
                    if (event.key.keysym.sym == SDLK_RIGHT) {
                            character.yaw += 3;
                    }
                }
                if (character.yaw < 0) character.yaw = 359;
                if (character.yaw > 360) character.yaw = 0;
            }
            if (event.type == SDL_KEYUP) {
                inputs[event.key.keysym.scancode] = 0;
            }
        }

        int yaw = character.yaw;
        float pSn = sn[yaw];
        float pCs = cs[yaw];

        int characterSector = isChrWithinConvexSector();        
        if ((characterSector > -1) & (characterSector <= level->count)) {
            sector *playerSector = level->sectors[characterSector];
            character.z = playerSector->elevation;
            
            //console.profile();
            startDrawSector((Uint16 *)rgb565_surface->pixels, level, playerSector, character, pSn, pCs, portalBounds, characterSector);
            gettimeofday(&end, NULL);
        }
        optimized_blt_and_update(rgb565_surface, surface, window);
        
        double time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1000000.0;
        time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1000000.0;

        fI++;
        fAvg += time_spent;
        if (fI == 240) {
            char str[10];
            fAvg /= fI;
            sprintf(str, "%.4lf", (double)(1/fAvg));
            SDL_SetWindowTitle(window, str);
            fI = 0;
            fAvg = 0;
        }
    }
    //G_QUIT();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

//clang -std=c17 main.c -O3 -ffast-math -flto -march=native -funroll-loops -IC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2 -LC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2\lib -Wall -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -o main

