#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"
#include "raster.h"
// include stuff for the game
#include "s_listener.c"
#include "s_level.c"
#include "s_physics.c"
#include "s_ai.c"

//#include "gui.c"
#include "ini_parser.c"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <stdalign.h>

player character = {20.0, 10.0, 0.0, 5.0, 180.0, -15.0, 70.0, 90.0, 0.0, 3.0};
sector *playerSector = NULL;

int8_t control_locked = 0;
int8_t inputs[256];
int fI = 0;
float sn[361];
float cs[361];
double fAvg = 0;

int timer = 0;

// clang -std=c17 main.c -O3 -ffast-math -flto -march=native -funroll-loops -IC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2 -LC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2\lib -Wall -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf render.obj -o main

SDL_Rect destRect = {0, 0, 1920, 1080};
SDL_Color rgb_palette[256];

void fill_palette(SDL_Color *palette) {
    static const SDL_Color standard16[16] = {
        {0x00,0x00,0x00}, {0x00,0x00,0xAA}, {0x00,0xAA,0x00}, {0x00,0xAA,0xAA},
        {0xAA,0x00,0x00}, {0xAA,0x00,0xAA}, {0xAA,0x55,0x00}, {0xAA,0xAA,0xAA},
        {0x55,0x55,0x55}, {0x55,0x55,0xFF}, {0x55,0xFF,0x55}, {0x55,0xFF,0xFF},
        {0xFF,0x55,0x55}, {0xFF,0x55,0xFF}, {0xFF,0xFF,0x55}, {0xFF,0xFF,0xFF}
    };
    for (int i = 0; i < 16; i++) {
        palette[i] = standard16[i];
    }
    const uint8_t levels[6] = {0x00, 0x2A, 0x55, 0x80, 0xAA, 0xD5};
    int index = 16;
    for (int r = 0; r < 6; r++) {
        for (int g = 0; g < 6; g++) {
            for (int b = 0; b < 6; b++) {
                palette[index].r = levels[r];
                palette[index].g = levels[g];
                palette[index].b = levels[b];
                index++;
            }
        }
    }
    for (int i = 0; i < 24; i++) {
        float norm = i / 23.0f;
        uint8_t val = (uint8_t)(powf(norm, 2.2f) * 255.0f);
        palette[232 + i].r = val;
        palette[232 + i].g = val;
        palette[232 + i].b = val;
    }
    static const SDL_Color glow[16] = {
        {0x10, 0x04, 0x00}, {0x20, 0x08, 0x00}, {0x40, 0x10, 0x00}, {0x60, 0x20, 0x00},
        {0x80, 0x30, 0x00}, {0xA0, 0x40, 0x00}, {0xC0, 0x60, 0x00}, {0xE0, 0x80, 0x00},
        {0xFF, 0xA0, 0x10}, {0xFF, 0xC0, 0x20}, {0xFF, 0xE0, 0x40}, {0xFF, 0xFF, 0x80},
        {0xFF, 0xFF, 0xA0}, {0xFF, 0xFF, 0xC0}, {0xFF, 0xFF, 0xE0}, {0xFF, 0xFF, 0xFF}
    };
    for (int i = 0; i < 16; i++) {
        palette[248 + i] = glow[i];
    }
}

void RecalcDestRect(SDL_Window *win) {
    int winW, winH;
    SDL_GetWindowSize(win, &winW, &winH);
    float windowAspect = (float)winW / (float)winH;
    float targetAspect = (float)SW / (float)SH;
    if (windowAspect > targetAspect) {
        destRect.h = winH;
        destRect.w = (int)(winH * targetAspect);
        destRect.x = (winW - destRect.w) / 2;
        destRect.y = 0;
    } else {
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

    for (int i = 0; i < 361; i++) {
        double a = (double)(i) * PI / 180;
        sn[i] = sin(a);
        cs[i] = cos(a);
    }
    for (int i = 0; i<256; i++) {
        inputs[i] = 0;
    }

    SDL_Window *window = SDL_CreateWindow("SDL2 Software Renderer Example",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SW, SH, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_ShowCursor(SDL_DISABLE);
    RecalcDestRect(window);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    SDL_Surface* framebuffer = SDL_CreateRGBSurface(0, SW, SH, 8, 0, 0, 0, 0);
    SDL_Event event;

    fill_palette(rgb_palette);
    SDL_SetPaletteColors(framebuffer->format->palette, rgb_palette, 0, 256);
    //G_INIT();
    S_LOAD_LEVEL(); // Load the level
    S_GENERATE_GRAPH(level);
    S_INIT_AI(level);
    init_textures();
    while (!quit) {
        struct timeval begin, end;
        gettimeofday(&begin, NULL);
        int yaw = character.yaw;
        float pSn = sn[yaw];
        float pCs = cs[yaw];

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_EXPOSED ||
                    event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {

                    surface = SDL_GetWindowSurface(window);
                    SDL_BlitScaled(framebuffer, NULL, surface, &destRect);
                    SDL_UpdateWindowSurface(window);
                }
            }
            if (event.type == SDL_KEYDOWN) {
                inputs[event.key.keysym.scancode] = 1;
                if (control_locked == 0) {
                    if (event.key.keysym.sym == SDLK_UP) {
                        character.x += 1*pSn;
                        character.y += 1*pCs;
                    }
                    if (event.key.keysym.sym == SDLK_DOWN) {
                        character.x -= 1*pSn;
                        character.y -= 1*pCs;
                    }
                    /*if (event.key.keysym.sym == SDLK_LEFT) {
                            character.yaw -= 3;
                    }
                    if (event.key.keysym.sym == SDLK_RIGHT) {
                            character.yaw += 3;
                    }*/
                    if (event.key.keysym.sym == SDLK_e) {
                        // Interact
                        if (playerSector != NULL) {
                            wall *retWall = NULL; 
                            S_RAYCAST(level, &retWall, playerSector, character.x, character.y, character.z, pSn, pCs);
                            if (retWall != NULL) {
                                //retWall->tIndex = 3;
                            }
                        }
                    }
                }
            }
            if (event.type == SDL_KEYUP) {
                inputs[event.key.keysym.scancode] = 0;
            }
            if (event.type == SDL_MOUSEMOTION) {
                character.pitch -= event.motion.yrel * MOUSE_SENSITIVITY_Y;
                character.pitch = CLAMP_F16(character.pitch, -MAX_CAMERA_PITCH, MAX_CAMERA_PITCH);

                character.yaw += event.motion.xrel * MOUSE_SENSITIVITY_X;
                if (character.yaw < 0) character.yaw = 360+character.yaw;
                if (character.yaw > 360) character.yaw = 0;
            }
        }
        int characterSector = S_CHAR_IN_CONVEX_BOUNDS(character);
        if ((characterSector > -1) & (characterSector <= level->count)) {
            playerSector = level->sectors[characterSector];
            character.z = playerSector->elevation;
            S_START_LISTEN(level, ScriptedSectors);
            S_SEGMENT_COLLISION_CHR(playerSector, &character);
            S_HANDLE_AI(level);
            R_startDrawSector((Uint8 *)framebuffer->pixels, level, playerSector, character, pSn, pCs, portalBounds, characterSector);
            gettimeofday(&end, NULL);
        } else {
            playerSector = NULL;
        }
        SDL_BlitScaled(framebuffer, NULL, surface, &destRect);
        SDL_UpdateWindowSurface(window);
        double time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1000000.0;
        time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1000000.0;
        fI++;
        fAvg += time_spent;
        if (fI == 30) {
            char str[10];
            fAvg /= fI;
            sprintf(str, "%.4lf", (double)(1/fAvg));
            SDL_SetWindowTitle(window, str);
            fI = 0;
            fAvg = 0;
        }
        timer ++;
        tick = (timer % 7 == 0) ? 1 : 0;
        SDL_Delay(10);
    }
    //G_QUIT();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

//clang -std=c17 main.c -O3 -ffast-math -flto -march=native -funroll-loops -IC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2 -LC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2\lib -Wall -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -o main