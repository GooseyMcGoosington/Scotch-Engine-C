#include <SDL2/SDL.h>
#include "s_helper.h"

#include "s_level.c"
#include "r_render.c"
#include "s_bsp.c"

#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>

int fI = 0;
double fAvg = 0;

static inline void fillBuffer(Uint8 *pixels, Uint8 colourIndex) {
    for (int y = 0; y < SH; ++y) {
        for (int x = 0; x < SW; ++x) {
            pixels[y * SW + x] = colourIndex;
        }
    }
}
SDL_Color rgb_palette[256];
// Your 256-color palette as RGB triplets

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

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Software Renderer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SW, SH, SDL_WINDOW_SHOWN);

    SDL_Surface* screenSurface = SDL_GetWindowSurface(window);
    SDL_Surface* framebuffer = SDL_CreateRGBSurface(0, SW, SH, 8, 0, 0, 0, 0);
    fill_palette(rgb_palette);
    SDL_SetPaletteColors(framebuffer->format->palette, rgb_palette, 0, 256);
    bool running = true;
    SDL_Event event;        
    SDL_Rect dstRect = { 0, 0, screenSurface->w, screenSurface->h };
    
    S_LEVEL_LOAD();
    bsp_init();
    while (running) {
        struct timeval begin, end;
        gettimeofday(&begin, NULL);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        // Fill framebuffer with a palette index (e.g., 2 = dark red)
        Uint8* pixels = (Uint8*)framebuffer->pixels;
        //fillBuffer(pixels, 2);
        //R_RENDER_SECTORS(level, pixels, -1);
        bsp_traverse_draw(root_node, pixels, 0xcc);
        SDL_BlitScaled(framebuffer, NULL, screenSurface, &dstRect);
        SDL_UpdateWindowSurface(window);

        gettimeofday(&end, NULL);
        double time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1000000.0;
        fI++;
        fAvg += time_spent;
        if (fI == 240) {
            char str[64];
            fAvg /= fI;
            sprintf(str, "FPS: %.2lf", 1.0 / fAvg);
            SDL_SetWindowTitle(window, str);
            fI = 0;
            fAvg = 0;
        }
    }

    SDL_FreeSurface(framebuffer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
