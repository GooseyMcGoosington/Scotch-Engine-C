#include "SDL2/SDL.h"
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>

#define WIDTH  1920
#define HEIGHT 1080

int fI = 0;
double fAvg = 0;

static inline void fillBuffer(Uint8 *pixels, Uint8 colourIndex) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            pixels[y * WIDTH + x] = colourIndex;
        }
    }
}

// Your 256-color palette as RGB triplets
SDL_Color rgb_palette[256] = {
    {0x00, 0x00, 0x00}, {0x20, 0x00, 0x00}, {0x80, 0x00, 0x00}, {0xFF, 0x00, 0x00},
    {0x00, 0x20, 0x00}, {0x00, 0x80, 0x00}, {0x00, 0xFF, 0x00}, {0x00, 0x00, 0x20},
    {0x00, 0x00, 0x80}, {0x00, 0x00, 0xFF}, {0x80, 0x80, 0x80}, {0xFF, 0xFF, 0xFF},
    // Fill with more colors...
};

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Software Renderer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

    SDL_Surface* screenSurface = SDL_GetWindowSurface(window);

    // Create 8-bit indexed surface
    SDL_Surface* framebuffer = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 8, 0, 0, 0, 0);

    // Set the palette
    SDL_SetPaletteColors(framebuffer->format->palette, rgb_palette, 0, 256);

    bool running = true;
    SDL_Event event;        
    SDL_Rect dstRect = { 0, 0, screenSurface->w, screenSurface->h };

    while (running) {
        struct timeval begin, end;
        gettimeofday(&begin, NULL);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        // Fill framebuffer with a palette index (e.g., 2 = dark red)
        Uint8* pixels = (Uint8*)framebuffer->pixels;
        fillBuffer(pixels, 2);

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
