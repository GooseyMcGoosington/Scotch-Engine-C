#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"
#include "raster.c"
// include stuff for the game
#include "s_listener.c"
#include "s_level.c"
#include "s_physics.c"
#include "s_ai.c"
#include "s_sound.c"
#include "p_keyboard.c"
#include "p_character.c"

#include "g_gui.c"
#include "ini_parser.c"

#include <stdio.h> 
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

sector *playerSector = NULL;

float sn[361];
float cs[361];

int timer = 0;
int quit = 0;

// clang -std=c17 main.c -O3 -ffast-math -flto -march=native -funroll-loops -IC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2 -LC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2\lib -Wall -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf render.obj -o main

SDL_Window *window = NULL;
SDL_Surface *framebuffer = NULL;
SDL_Surface *surface = NULL;
portalCull portalBounds;

SDL_Rect destRect = {0, 0, 1920, 1080};
SDL_Color rgb_palette[256];
SDL_Palette *PALETTE8 = NULL;

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
    PALETTE8 = SDL_AllocPalette(256);
    SDL_SetPaletteColors(PALETTE8, palette, 0, 256);
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
    SDL_Init(SDL_INIT_AUDIO);
    TTF_Init();
    
    INI_PARSE("settings.ini");
    CFG_Init(INI_FIND_VALUE("res"));
    R_INIT();
    portalBounds = (portalCull){1.0f, (float)SW1, 1.0f, 1.0f, SH1, SH1, SW1-1.0f};

    for (int i = 0; i < 361; i++) {
        double a = (double)(i) * PI / 180;
        sn[i] = sin(a);
        cs[i] = cos(a);
    }
    window = SDL_CreateWindow("SDL2 Software Renderer Example",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SW, SH, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
    RecalcDestRect(window);
    surface = SDL_GetWindowSurface(window);
    framebuffer = SDL_CreateRGBSurface(0, SW, SH, 8, 0, 0, 0, 0);
    fill_palette(rgb_palette);
    SDL_SetPaletteColors(framebuffer->format->palette, rgb_palette, 0, 256);
    
    S_LOAD_LEVEL();
    S_GENERATE_GRAPH(level);
    S_INIT_AI(level);
    S_INIT_SOUND();
    P_KEYBOARD_INIT();
    P_CHARACTER_INIT(20.0, 0.0, 0.0, 90.0, 3.0);
    init_textures();
    G_GUI_INIT();
    
    while (!quit) {
        G_GUI_UPDATE_HUD((Uint8*)framebuffer->pixels);
        P_KEYBOARD_UPDATE();
        int yaw = character.yaw;
        float pSn = sn[yaw];
        float pCs = cs[yaw];
        // Update sin and cos for S_SOUND
        S_SOUND_SIN = pSn;
        S_SOUND_COS = pCs;

        unsigned int cS = 0;
        int inside = P_CHARACTER_UPDATE_SECTOR(&cS);

        if (inside) {
            // Logic Updates
            S_START_LISTEN(level, ScriptedSectors);
            S_SEGMENT_COLLISION_CHR(playerSector, &character);
            S_HANDLE_AI(level);
            S_SOUND_UPDATE(character, pCs, pSn);
            // Rasterise Sectors First
            R_startDrawSector((Uint8 *)framebuffer->pixels, level, playerSector, character, pSn, pCs, portalBounds, cS);
            // Rasterise Entities Last
            R_DRAW_ENTITIES((Uint8*)framebuffer->pixels, level, character, pCs, pSn);
            gettimeofday(&end, NULL);
        } else {
            playerSector = NULL;
        }
        // Render to Screen
        G_GUI_BLIT(framebuffer, &destRect);
        SDL_BlitScaled(framebuffer, NULL, surface, &destRect);
        SDL_UpdateWindowSurface(window);
        // Tick Engine
        tick = (timer % 7 == 0) ? 1 : 0;
        SDL_Delay(30);
    }
    G_GUI_QUIT();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

//clang -std=c17 main.c -O3 -ffast-math -flto -march=native -funroll-loops -IC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2 -LC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2\lib -Wall -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -o main