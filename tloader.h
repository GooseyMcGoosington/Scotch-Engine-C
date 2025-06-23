#ifndef TLOADER_H
#define TLOADER_H

#include "SDL2/SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "helper.h"
#include "classes.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

tlut *texture_list = NULL;

extern SDL_Color rgb_palette[256];

static inline int color_dist_sq(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2) {
    int dr = r1 - r2;
    int dg = g1 - g2;
    int db = b1 - b2;
    return dr*dr + dg*dg + db*db;
}

tfile load_texture(char *filename, int scale) {
    int width, height, channels;
    unsigned char *image = stbi_load(filename, &width, &height, &channels, 0);
    int numPixels = width * height;
    uint8_t *indexed_pixels = malloc(numPixels * 4 * sizeof(uint8_t));
    for (int i = 0; i < numPixels; i++) {
        int idx = i * channels;
        uint8_t r = image[idx];
        uint8_t g = image[idx + 1];
        uint8_t b = image[idx + 2];

        for (int shadeLevel = 0; shadeLevel < 4; shadeLevel++) {
            float div = (float)(shadeLevel + 1);
            uint8_t r_div = (uint8_t)(r / div);
            uint8_t g_div = (uint8_t)(g / div);
            uint8_t b_div = (uint8_t)(b / div);

            int best_index = 0;
            int best_dist = 256*256*3;

            for (int p = 0; p < 256; p++) {
                SDL_Color c = rgb_palette[p];
                int dist = color_dist_sq(r_div, g_div, b_div, c.r, c.g, c.b);
                if (dist < best_dist) {
                    best_dist = dist;
                    best_index = p;
                }
            }
            indexed_pixels[i + numPixels * shadeLevel] = (uint8_t)(best_index);
        }
    }
    stbi_image_free(image);
    tfile file = {(void *)indexed_pixels, width, height, scale};
    return file;
}



void init_textures() {
    texture_list = malloc(sizeof(tlut));
    texture_list->files = malloc(MAX_TEXTURES * sizeof(tfile));
    texture_list->files[0] = load_texture("textures/tile127.png", 0);
    texture_list->files[1] = load_texture("textures/tile085.png", 0);
    texture_list->files[2] = load_texture("textures/mega1.png", 0);
    texture_list->files[3] = load_texture("textures/mega2.png", 0);
    texture_list->files[4] = load_texture("textures/mega3.png", 0);
    texture_list->files[5] = load_texture("textures/mega4.png", 0);
    texture_list->files[6] = load_texture("textures/medkit.png", 0);
    texture_list->files[7] = load_texture("textures/keycardred.png", 0);
    texture_list->files[8] = load_texture("textures/enemy.png", 0);
    texture_list->files[9] = load_texture("textures/tile106.png", 0);
    texture_list->files[10] = load_texture("textures/dust.png", 0);
}

void free_textures() {
    if (texture_list != NULL) {
        for (int i = 0; i < MAX_TEXTURES; i++) {
            if (texture_list->files[i].pixels != NULL) {
                free(texture_list->files[i].pixels);
            }
        }
        free(texture_list->files);
        free(texture_list);
    }
}

#endif
