#ifndef TLOADER_H
#define TLOADER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "helper.h"
#include "classes.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

tlut *texture_list = NULL;

// Function to load texture using stb_image
tfile load_texture(char *filename, int scale) {
    int width, height, channels;
    unsigned char *image = stbi_load(filename, &width, &height, &channels, 0);

    if (!image) {
        printf("Error loading texture: %s\n", filename);
        tfile empty = {NULL, 0, 0};
        return empty;
    }

    int numPixels = width * height;
    uint16_t *rgb565_pixels = (uint16_t *)malloc(numPixels * sizeof(uint16_t));

    for (int i = 0; i < numPixels; i++) {
        int idx = i * channels;  // RGB or RGBA depending on the number of channels
        uint8_t r = image[idx];
        uint8_t g = image[idx + 1];
        uint8_t b = image[idx + 2];

        // Convert to RGB565 format
        rgb565_pixels[i]  = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
    }

    stbi_image_free(image);

    tfile file = {rgb565_pixels, width, height, scale};
    return file;
}

void init_textures() {
    texture_list = malloc(sizeof(tlut));
    texture_list->files = malloc(MAX_TEXTURES * sizeof(tfile));
    texture_list->files[0] = load_texture("textures/tile110.png", 0);
    texture_list->files[1] = load_texture("textures/tile115.png", 0);
    texture_list->files[2] = load_texture("textures/mega1.png", 0);
    texture_list->files[3] = load_texture("textures/mega2.png", 0);
    texture_list->files[4] = load_texture("textures/mega3.png", 0);
    texture_list->files[5] = load_texture("textures/mega4.png", 0);
    texture_list->files[6] = load_texture("textures/shotgun.png", 0);
    texture_list->files[7] = load_texture("textures/keycardred.png", 0);

    //if (texture_list->files[0].pixels != NULL) {
    //    printf("Loaded texture, first pixel value: %x\n", texture_list->files[0].pixels[0]);
    //}
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
