#ifndef R_RENDER
#define R_RENDER

#include "s_classes.h"
#include "s_helper.h"
#include "../SDL2/SDL.h"

#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>

extern SDL_Color rgb_palette[256];

static inline void R_PIXEL(uint8_t *pixels, int x, int y, uint8_t Colour) {
    if ((x > 1) && (x < SW1)) {
        if ((y > 1) && (y < SH1)) {
            pixels[y*SW+x] = Colour;
        }
    }
};

void R_LINE(uint8_t *pixels, int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (true) {
        R_PIXEL(pixels, x0, y0, color); // Set the pixel
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

static void R_LINE_WALL(uint8_t *pixels, int x0, int y0, int x1, int y1, uint8_t colour) {
    // Draw the wall line
    R_LINE(pixels, x0, y0, x1, y1, colour);

    int mx = (x0 + x1) / 2;
    int my = (y0 + y1) / 2;
    int dx = x1 - x0;
    int dy = y1 - y0;

    float nx = dy;
    float ny = -dx;

    float len = sqrtf(nx * nx + ny * ny);
    if (len == 0) return;
    nx /= len;
    ny /= len;

    int scale = 10;
    int ex = mx + (int)(nx * scale);
    int ey = my + (int)(ny * scale);

    R_LINE(pixels, mx, my, ex, ey, 33);
}

static void R_RENDER_SECTORS(Level *level, uint8_t *pixels, int strictSector) {
    if (level->count == 0) {
        return;
    }
    for (int s=0; s<level->count; s++) {
        sector *Sector = level->sectors[s];
        if (strictSector != -1 && strictSector != s) {
            continue;
        }
        for (int w=0; w<Sector->count; w++) {
            wall *Wall = &Sector->walls[w];
            R_LINE_WALL(pixels, Wall->x0+SW2, Wall->y0+SH2, Wall->x1+SW2, Wall->y1+SH2, 0xcc);
        }
    }
};

#endif