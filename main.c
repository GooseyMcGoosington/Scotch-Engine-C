#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"
#include "raster.h"
//#include "gui.c"
#include "ini_parser.c"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <stdalign.h>

player character = {20.0, -10.0, 0.0, 5.0, 0.0, 70.0, 90.0, 0.0};
Level *level = NULL;

float sn[360];
float cs[360];

int control_locked = 0;
int fI = 0;
int inputs[256];
double fAvg = 0;

int timer = 0;

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
// clang -std=c17 main.c -O3 -ffast-math -flto -march=native -funroll-loops -IC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2 -LC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2\lib -Wall -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf render.obj -o main

SDL_Rect destRect = {0, 0, 1920, 1080};

void RecalcDestRect(SDL_Window *win) {
    int winW, winH;
    SDL_GetWindowSize(win, &winW, &winH);

    float windowAspect = (float)winW / (float)winH;
    float targetAspect = (float)SW / (float)SH;

    if (windowAspect > targetAspect) {
        // window is too wide → pillarbox
        destRect.h = winH;
        destRect.w = (int)(winH * targetAspect);
        destRect.x = (winW - destRect.w) / 2;
        destRect.y = 0;
    } else {
        // window is too tall → letterbox
        destRect.w = winW;
        destRect.h = (int)(winW / targetAspect);
        destRect.x = 0;
        destRect.y = (winH - destRect.h) / 2;
    }
}

int main(int argc, char* argv[]) {
    // Initialize Code
    INI_PARSE("settings.ini");
    CFG_Init(INI_FIND_VALUE("res"));
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

    sect->amnt_entities = 3;
    sect->entities = malloc(sect->amnt_entities*sizeof(entity));
    sect->entities[0] = (entity){1, 10, 10, 1, 2, 1, 0};
    sect->entities[1] = (entity){1, 10, 15, 1, 6, 0, 0};
    sect->entities[2] = (entity){1, 10, 20, 1, 7, 0, 0};


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
            Sector->walls[y].length = dst;
        }
    }

    SDL_Window *window = SDL_CreateWindow("SDL2 Software Renderer Example",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1920, 1080, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
    RecalcDestRect(window);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    SDL_Surface *rgb565_surface = SDL_CreateRGBSurfaceWithFormat(0, SW, SH, 16, SDL_PIXELFORMAT_RGB565);
    SDL_Event event;
    
    //G_INIT();
    init_textures();
    while (!quit) {
        struct timeval begin, end;
        gettimeofday(&begin, NULL);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_EXPOSED ||
                    event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {

                    surface = SDL_GetWindowSurface(window);  // <-- Refresh pointer!
                    SDL_BlitScaled(rgb565_surface, NULL, surface, &destRect);
                    SDL_UpdateWindowSurface(window);
                }
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
            R_startDrawSector((Uint16 *)rgb565_surface->pixels, level, playerSector, character, pSn, pCs, portalBounds, characterSector);
            gettimeofday(&end, NULL);
        }
        SDL_BlitScaled(rgb565_surface, NULL, surface, &destRect);
        SDL_UpdateWindowSurface(window);

        double time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1000000.0;
        time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1000000.0;
        fI++;
        fAvg += time_spent;
        if (fI == 60) {
            char str[10];
            fAvg /= fI;
            sprintf(str, "%.4lf", (double)(1/fAvg));
            SDL_SetWindowTitle(window, str);
            fI = 0;
            fAvg = 0;
        }
        timer ++;
        tick = (timer % 7 == 0) ? 1 : 0;
        SDL_Delay(16);
    }
    //G_QUIT();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

//clang -std=c17 main.c -O3 -ffast-math -flto -march=native -funroll-loops -IC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2 -LC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2\lib -Wall -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -o main