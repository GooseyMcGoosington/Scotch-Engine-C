#ifndef rdisp
#define rdisp

#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"

static inline void draw_wall1920x1080x64(float sx0, float sx1, 
    float sy0, float sy1, float sy2, float sy3, Uint16 * restrict pixels, 
    portalCull portalBounds, int * restrict ceilingLut, int * restrict floorLut, int flat, uint16_t * restrict w_pixels, float t0, float t1, float wy0, float wy1, float length, float height, int tScale) {

    float dx = (sx1 > sx0) ? (sx1 - sx0) : 1.0f;
    int startX = clamp((int)sx0, portalBounds.x0, portalBounds.x1);
    int endX   = clamp((int)sx1, portalBounds.x0, portalBounds.x1);
    float invDx = 1.0f / dx;
    float invCullDx = 1.0f / portalBounds.dxCull;
    float z0 = 1/wy0;
    float z1 = 1/wy1;
    float sf = length/4;
    float tLeft = (1-t0) * 0 + t0*sf;
    float tRight = (1-t1) * 0 + t1*sf;
    float tLZ0 = tLeft/z0;
    float tRZ1 = tRight/z1;
    float wz0 = wy0/z0;
    float wz1 = wy1/z1;
    float wh = height/4;

    const int SHIFT = 6;
    const int TEXTURE_SHIFT = (1<<SHIFT);
    const float TEXTURE_DETAIL = (TEXTURE_SHIFT*wh);
    const unsigned int MASK = 0x3F;

    for (int x = startX; x < endX; x++) {
        float t = (x - sx0) * invDx;
        float v = (1-t);
        float st = (x - portalBounds.x0) * invCullDx;
        float wS = (1-v) * wy0 + v*wy1;
        float dS = ((1-v) * wz1 + v*wz0);
        int_fast16_t yTop    = (int)((1.0f - t) * sy0 + t * sy1);
        int_fast16_t yBottom = (int)((1.0f - t) * sy2 + t * sy3);
        int_fast16_t tTop = yTop;
        int_fast16_t tBottom = yBottom;
        int_fast16_t clampY0 = (int_fast16_t)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        int_fast16_t clampY1 = (int_fast16_t)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);
        int_fast16_t U = (int_fast16_t)(((1-v) * tLZ0 + v*tRZ1) / wS * TEXTURE_SHIFT) & MASK;
        yTop    = clamp(yTop, clampY0, clampY1);
        yBottom = clamp(yBottom, clampY0, clampY1);
        int_fast16_t clampSY0 = clamp(yTop, 1, 1079);
        int_fast16_t clampSY1 = clamp(yBottom, 1, 1079);
        int row = clampSY0 * 1920 + x;
        if (flat == 0) {
            ceilingLut[x] = clampSY0;
            floorLut[x] = clampSY1;
        } else if (flat == 1) {
            ceilingLut[x] = clampSY0;
        } else if (flat == 2) {
            floorLut[x] = clampSY1;
        }
        float dxY = (tBottom - tTop);
        float idxY = (1/dxY) * TEXTURE_DETAIL;
        int_fast16_t shade = (int_fast16_t)(1.0f + dS * 0.0025f);        
        uint16_t *base = &w_pixels[U];
        for (int y = clampSY0; y < clampSY1; y++) {
            int_fast16_t V = (((int)((y - tTop) * idxY) & MASK) << SHIFT);
            uint16_t baseColour = base[V];
            uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
            uint8_t g = (((baseColour >> 5)  & 0x3F) >> shade);
            uint8_t b = ((baseColour & 0x1F) >> shade);
            pixels[row] = (r << 11) | (g << 5) | b;
            row += 1920;
        }
    }
};

static inline void draw_wall1920x1080x128(float sx0, float sx1, 
    float sy0, float sy1, float sy2, float sy3, Uint16 * restrict pixels, 
    portalCull portalBounds, int * restrict ceilingLut, int * restrict floorLut, int flat, uint16_t * restrict w_pixels, float t0, float t1, float wy0, float wy1, float length, float height, int tScale) {

    float dx = (sx1 > sx0) ? (sx1 - sx0) : 1.0f;
    int startX = clamp((int)sx0, portalBounds.x0, portalBounds.x1);
    int endX   = clamp((int)sx1, portalBounds.x0, portalBounds.x1);
    float invDx = 1.0f / dx;
    float invCullDx = 1.0f / portalBounds.dxCull;
    float z0 = 1/wy0;
    float z1 = 1/wy1;
    float sf = length/4;
    float tLeft = (1-t0) * 0 + t0*sf;
    float tRight = (1-t1) * 0 + t1*sf;
    float tLZ0 = tLeft/z0;
    float tRZ1 = tRight/z1;
    float wz0 = wy0/z0;
    float wz1 = wy1/z1;
    float wh = height/4;

    const int SHIFT = 6;
    const int TEXTURE_SHIFT = (1<<SHIFT);
    const float TEXTURE_DETAIL = (TEXTURE_SHIFT*wh);
    const unsigned int MASK = 0x3F;

    for (int x = startX; x < endX; x++) {
        float t = (x - sx0) * invDx;
        float v = (1-t);
        float st = (x - portalBounds.x0) * invCullDx;
        float wS = (1-v) * wy0 + v*wy1;
        float dS = ((1-v) * wz1 + v*wz0);
        int_fast16_t yTop    = (int)((1.0f - t) * sy0 + t * sy1);
        int_fast16_t yBottom = (int)((1.0f - t) * sy2 + t * sy3);
        int_fast16_t tTop = yTop;
        int_fast16_t tBottom = yBottom;
        int_fast16_t clampY0 = (int_fast16_t)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        int_fast16_t clampY1 = (int_fast16_t)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);
        int_fast16_t U = (int_fast16_t)(((1-v) * tLZ0 + v*tRZ1) / wS * TEXTURE_SHIFT) & MASK;
        yTop    = clamp(yTop, clampY0, clampY1);
        yBottom = clamp(yBottom, clampY0, clampY1);
        int_fast16_t clampSY0 = clamp(yTop, 1, 1079);
        int_fast16_t clampSY1 = clamp(yBottom, 1, 1079);
        int row = clampSY0 * 1920 + x;
        if (flat == 0) {
            ceilingLut[x] = clampSY0;
            floorLut[x] = clampSY1;
        } else if (flat == 1) {
            ceilingLut[x] = clampSY0;
        } else if (flat == 2) {
            floorLut[x] = clampSY1;
        }
        float dxY = (tBottom - tTop);
        float idxY = (1/dxY) * TEXTURE_DETAIL;
        int_fast16_t shade = (int_fast16_t)(1.0f + dS * 0.0025f);        
        uint16_t *base = &w_pixels[U];
        for (int y = clampSY0; y < clampSY1; y++) {
            int_fast16_t V = (((int)((y - tTop) * idxY) & MASK) << SHIFT);
            uint16_t baseColour = base[V];
            uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
            uint8_t g = (((baseColour >> 5)  & 0x3F) >> shade);
            uint8_t b = ((baseColour & 0x1F) >> shade);
            pixels[row] = (r << 11) | (g << 5) | b;
            row += 1920;
        }
    }
};

static inline void draw_wall1024x768x64(float sx0, float sx1, 
    float sy0, float sy1, float sy2, float sy3, Uint16 * restrict pixels, 
    portalCull portalBounds, int * restrict ceilingLut, int * restrict floorLut, int flat, uint16_t * restrict w_pixels, float t0, float t1, float wy0, float wy1, float length, float height, int tScale) {

    float dx = (sx1 > sx0) ? (sx1 - sx0) : 1.0f;
    int startX = clamp((int)sx0, portalBounds.x0, portalBounds.x1);
    int endX   = clamp((int)sx1, portalBounds.x0, portalBounds.x1);
    float invDx = 1.0f / dx;
    float invCullDx = 1.0f / portalBounds.dxCull;
    float z0 = 1/wy0;
    float z1 = 1/wy1;
    float sf = length/4;
    float tLeft = (1-t0) * 0 + t0*sf;
    float tRight = (1-t1) * 0 + t1*sf;
    float tLZ0 = tLeft/z0;
    float tRZ1 = tRight/z1;
    float wz0 = wy0/z0;
    float wz1 = wy1/z1;
    float wh = height/4;

    const int SHIFT = 6;
    const int TEXTURE_SHIFT = (1<<SHIFT);
    const float TEXTURE_DETAIL = (TEXTURE_SHIFT*wh);
    const unsigned int MASK = 0x3F;

    for (int x = startX; x < endX; x++) {
        float t = (x - sx0) * invDx;
        float v = (1-t);
        float st = (x - portalBounds.x0) * invCullDx;
        float wS = (1-v) * wy0 + v*wy1;
        float dS = ((1-v) * wz1 + v*wz0);
        int_fast16_t yTop    = (int)((1.0f - t) * sy0 + t * sy1);
        int_fast16_t yBottom = (int)((1.0f - t) * sy2 + t * sy3);
        int_fast16_t tTop = yTop;
        int_fast16_t tBottom = yBottom;
        int_fast16_t clampY0 = (int_fast16_t)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        int_fast16_t clampY1 = (int_fast16_t)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);
        int_fast16_t U = (int_fast16_t)(((1-v) * tLZ0 + v*tRZ1) / wS * TEXTURE_SHIFT) & MASK;
        yTop    = clamp(yTop, clampY0, clampY1);
        yBottom = clamp(yBottom, clampY0, clampY1);
        int_fast16_t clampSY0 = clamp(yTop, 1, 767);
        int_fast16_t clampSY1 = clamp(yBottom, 1, 767);
        int row = clampSY0 * 1024 + x;
        if (flat == 0) {
            ceilingLut[x] = clampSY0;
            floorLut[x] = clampSY1;
        } else if (flat == 1) {
            ceilingLut[x] = clampSY0;
        } else if (flat == 2) {
            floorLut[x] = clampSY1;
        }
        float dxY = (tBottom - tTop);
        float idxY = (1/dxY) * TEXTURE_DETAIL;
        int_fast16_t shade = (int_fast16_t)(1.0f + dS * 0.0025f);        
        uint16_t *base = &w_pixels[U];
        for (int y = clampSY0; y < clampSY1; y++) {
            int_fast16_t V = (((int)((y - tTop) * idxY) & MASK) << SHIFT);
            uint16_t baseColour = base[V];
            uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
            uint8_t g = (((baseColour >> 5)  & 0x3F) >> shade);
            uint8_t b = ((baseColour & 0x1F) >> shade);
            pixels[row] = (r << 11) | (g << 5) | b;
            row += 1024;
        }
    }
};

static inline void draw_wall1024x768x128(float sx0, float sx1, 
    float sy0, float sy1, float sy2, float sy3, Uint16 * restrict pixels, 
    portalCull portalBounds, int * restrict ceilingLut, int * restrict floorLut, int flat, uint16_t * restrict w_pixels, float t0, float t1, float wy0, float wy1, float length, float height, int tScale) {

    float dx = (sx1 > sx0) ? (sx1 - sx0) : 1.0f;
    int startX = clamp((int)sx0, portalBounds.x0, portalBounds.x1);
    int endX   = clamp((int)sx1, portalBounds.x0, portalBounds.x1);
    float invDx = 1.0f / dx;
    float invCullDx = 1.0f / portalBounds.dxCull;
    float z0 = 1/wy0;
    float z1 = 1/wy1;
    float sf = length/4;
    float tLeft = (1-t0) * 0 + t0*sf;
    float tRight = (1-t1) * 0 + t1*sf;
    float tLZ0 = tLeft/z0;
    float tRZ1 = tRight/z1;
    float wz0 = wy0/z0;
    float wz1 = wy1/z1;
    float wh = height/4;

    const int SHIFT = 7;
    const int TEXTURE_SHIFT = (1<<SHIFT);
    const float TEXTURE_DETAIL = (TEXTURE_SHIFT*wh);
    const unsigned int MASK = 0x3F;

    for (int x = startX; x < endX; x++) {
        float t = (x - sx0) * invDx;
        float v = (1-t);
        float st = (x - portalBounds.x0) * invCullDx;
        float wS = (1-v) * wy0 + v*wy1;
        float dS = ((1-v) * wz1 + v*wz0);
        int_fast16_t yTop    = (int)((1.0f - t) * sy0 + t * sy1);
        int_fast16_t yBottom = (int)((1.0f - t) * sy2 + t * sy3);
        int_fast16_t tTop = yTop;
        int_fast16_t tBottom = yBottom;
        int_fast16_t clampY0 = (int_fast16_t)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        int_fast16_t clampY1 = (int_fast16_t)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);
        int_fast16_t U = (int_fast16_t)(((1-v) * tLZ0 + v*tRZ1) / wS * TEXTURE_SHIFT) & MASK;
        yTop    = clamp(yTop, clampY0, clampY1);
        yBottom = clamp(yBottom, clampY0, clampY1);
        int_fast16_t clampSY0 = clamp(yTop, 1, 767);
        int_fast16_t clampSY1 = clamp(yBottom, 1, 767);
        int row = clampSY0 * 1024 + x;
        if (flat == 0) {
            ceilingLut[x] = clampSY0;
            floorLut[x] = clampSY1;
        } else if (flat == 1) {
            ceilingLut[x] = clampSY0;
        } else if (flat == 2) {
            floorLut[x] = clampSY1;
        }
        float dxY = (tBottom - tTop);
        float idxY = (1/dxY) * TEXTURE_DETAIL;
        int_fast16_t shade = (int_fast16_t)(1.0f + dS * 0.0025f);        
        uint16_t *base = &w_pixels[U];
        for (int y = clampSY0; y < clampSY1; y++) {
            int_fast16_t V = (((int)((y - tTop) * idxY) & MASK) << SHIFT);
            uint16_t baseColour = base[V];
            uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
            uint8_t g = (((baseColour >> 5)  & 0x3F) >> shade);
            uint8_t b = ((baseColour & 0x1F) >> shade);
            pixels[row] = (r << 11) | (g << 5) | b;
            row += 1024;
        }
    }
};

static inline void draw_wall800x600x64(float sx0, float sx1, 
    float sy0, float sy1, float sy2, float sy3, Uint16 * restrict pixels, 
    portalCull portalBounds, int * restrict ceilingLut, int * restrict floorLut, int flat, uint16_t * restrict w_pixels, float t0, float t1, float wy0, float wy1, float length, float height, int tScale) {

    float dx = (sx1 > sx0) ? (sx1 - sx0) : 1.0f;
    int startX = clamp((int)sx0, portalBounds.x0, portalBounds.x1);
    int endX   = clamp((int)sx1, portalBounds.x0, portalBounds.x1);
    float invDx = 1.0f / dx;
    float invCullDx = 1.0f / portalBounds.dxCull;
    float z0 = 1/wy0;
    float z1 = 1/wy1;
    float sf = length/4;
    float tLeft = (1-t0) * 0 + t0*sf;
    float tRight = (1-t1) * 0 + t1*sf;
    float tLZ0 = tLeft/z0;
    float tRZ1 = tRight/z1;
    float wz0 = wy0/z0;
    float wz1 = wy1/z1;
    float wh = height/4;

    const int SHIFT = 6;
    const int TEXTURE_SHIFT = (1<<SHIFT);
    const float TEXTURE_DETAIL = (TEXTURE_SHIFT*wh);
    const unsigned int MASK = 0x3F;

    for (int x = startX; x < endX; x++) {
        float t = (x - sx0) * invDx;
        float v = (1-t);
        float st = (x - portalBounds.x0) * invCullDx;
        float wS = (1-v) * wy0 + v*wy1;
        float dS = ((1-v) * wz1 + v*wz0);
        int_fast16_t yTop    = (int)((1.0f - t) * sy0 + t * sy1);
        int_fast16_t yBottom = (int)((1.0f - t) * sy2 + t * sy3);
        int_fast16_t tTop = yTop;
        int_fast16_t tBottom = yBottom;
        int_fast16_t clampY0 = (int_fast16_t)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        int_fast16_t clampY1 = (int_fast16_t)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);
        int_fast16_t U = (int_fast16_t)(((1-v) * tLZ0 + v*tRZ1) / wS * TEXTURE_SHIFT) & MASK;
        yTop    = clamp(yTop, clampY0, clampY1);
        yBottom = clamp(yBottom, clampY0, clampY1);
        int_fast16_t clampSY0 = clamp(yTop, 1, 599);
        int_fast16_t clampSY1 = clamp(yBottom, 1, 599);
        int row = clampSY0 * 800 + x;
        if (flat == 0) {
            ceilingLut[x] = clampSY0;
            floorLut[x] = clampSY1;
        } else if (flat == 1) {
            ceilingLut[x] = clampSY0;
        } else if (flat == 2) {
            floorLut[x] = clampSY1;
        }
        float dxY = (tBottom - tTop);
        float idxY = (1/dxY) * TEXTURE_DETAIL;
        int_fast16_t shade = (int_fast16_t)(1.0f + dS * 0.0025f);        
        uint16_t *base = &w_pixels[U];
        for (int y = clampSY0; y < clampSY1; y++) {
            int_fast16_t V = (((int)((y - tTop) * idxY) & MASK) << SHIFT);
            uint16_t baseColour = base[V];
            uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
            uint8_t g = (((baseColour >> 5)  & 0x3F) >> shade);
            uint8_t b = ((baseColour & 0x1F) >> shade);
            pixels[row] = (r << 11) | (g << 5) | b;
            row += 800;
        }
    }
};

static inline void draw_wall800x600x128(float sx0, float sx1, 
    float sy0, float sy1, float sy2, float sy3, Uint16 * restrict pixels, 
    portalCull portalBounds, int * restrict ceilingLut, int * restrict floorLut, int flat, uint16_t * restrict w_pixels, float t0, float t1, float wy0, float wy1, float length, float height, int tScale) {

    float dx = (sx1 > sx0) ? (sx1 - sx0) : 1.0f;
    int startX = clamp((int)sx0, portalBounds.x0, portalBounds.x1);
    int endX   = clamp((int)sx1, portalBounds.x0, portalBounds.x1);
    float invDx = 1.0f / dx;
    float invCullDx = 1.0f / portalBounds.dxCull;
    float z0 = 1/wy0;
    float z1 = 1/wy1;
    float sf = length/4;
    float tLeft = (1-t0) * 0 + t0*sf;
    float tRight = (1-t1) * 0 + t1*sf;
    float tLZ0 = tLeft/z0;
    float tRZ1 = tRight/z1;
    float wz0 = wy0/z0;
    float wz1 = wy1/z1;
    float wh = height/4;

    const int SHIFT = 7;
    const int TEXTURE_SHIFT = (1<<SHIFT);
    const float TEXTURE_DETAIL = (TEXTURE_SHIFT*wh);
    const unsigned int MASK = 0x3F;

    for (int x = startX; x < endX; x++) {
        float t = (x - sx0) * invDx;
        float v = (1-t);
        float st = (x - portalBounds.x0) * invCullDx;
        float wS = (1-v) * wy0 + v*wy1;
        float dS = ((1-v) * wz1 + v*wz0);
        int_fast16_t yTop    = (int)((1.0f - t) * sy0 + t * sy1);
        int_fast16_t yBottom = (int)((1.0f - t) * sy2 + t * sy3);
        int_fast16_t tTop = yTop;
        int_fast16_t tBottom = yBottom;
        int_fast16_t clampY0 = (int_fast16_t)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        int_fast16_t clampY1 = (int_fast16_t)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);
        int_fast16_t U = (int_fast16_t)(((1-v) * tLZ0 + v*tRZ1) / wS * TEXTURE_SHIFT) & MASK;
        yTop    = clamp(yTop, clampY0, clampY1);
        yBottom = clamp(yBottom, clampY0, clampY1);
        int_fast16_t clampSY0 = clamp(yTop, 1, 599);
        int_fast16_t clampSY1 = clamp(yBottom, 1, 599);
        int row = clampSY0 * 800 + x;
        if (flat == 0) {
            ceilingLut[x] = clampSY0;
            floorLut[x] = clampSY1;
        } else if (flat == 1) {
            ceilingLut[x] = clampSY0;
        } else if (flat == 2) {
            floorLut[x] = clampSY1;
        }
        float dxY = (tBottom - tTop);
        float idxY = (1/dxY) * TEXTURE_DETAIL;
        int_fast16_t shade = (int_fast16_t)(1.0f + dS * 0.0025f);        
        uint16_t *base = &w_pixels[U];
        for (int y = clampSY0; y < clampSY1; y++) {
            int_fast16_t V = (((int)((y - tTop) * idxY) & MASK) << SHIFT);
            uint16_t baseColour = base[V];
            uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
            uint8_t g = (((baseColour >> 5)  & 0x3F) >> shade);
            uint8_t b = ((baseColour & 0x1F) >> shade);
            pixels[row] = (r << 11) | (g << 5) | b;
            row += 800;
        }
    }
};

static inline void draw_wall640x480x64(float sx0, float sx1, 
    float sy0, float sy1, float sy2, float sy3, Uint16 * restrict pixels, 
    portalCull portalBounds, int * restrict ceilingLut, int * restrict floorLut, int flat, uint16_t * restrict w_pixels, float t0, float t1, float wy0, float wy1, float length, float height, int tScale) {

    float dx = (sx1 > sx0) ? (sx1 - sx0) : 1.0f;
    int startX = clamp((int)sx0, portalBounds.x0, portalBounds.x1);
    int endX   = clamp((int)sx1, portalBounds.x0, portalBounds.x1);
    float invDx = 1.0f / dx;
    float invCullDx = 1.0f / portalBounds.dxCull;
    float z0 = 1/wy0;
    float z1 = 1/wy1;
    float sf = length/4;
    float tLeft = (1-t0) * 0 + t0*sf;
    float tRight = (1-t1) * 0 + t1*sf;
    float tLZ0 = tLeft/z0;
    float tRZ1 = tRight/z1;
    float wz0 = wy0/z0;
    float wz1 = wy1/z1;
    float wh = height/4;

    const int SHIFT = 6;
    const int TEXTURE_SHIFT = (1<<SHIFT);
    const float TEXTURE_DETAIL = (TEXTURE_SHIFT*wh);
    const unsigned int MASK = 0x3F;

    for (int x = startX; x < endX; x++) {
        float t = (x - sx0) * invDx;
        float v = (1-t);
        float st = (x - portalBounds.x0) * invCullDx;
        float wS = (1-v) * wy0 + v*wy1;
        float dS = ((1-v) * wz1 + v*wz0);
        int_fast16_t yTop    = (int)((1.0f - t) * sy0 + t * sy1);
        int_fast16_t yBottom = (int)((1.0f - t) * sy2 + t * sy3);
        int_fast16_t tTop = yTop;
        int_fast16_t tBottom = yBottom;
        int_fast16_t clampY0 = (int_fast16_t)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        int_fast16_t clampY1 = (int_fast16_t)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);
        int_fast16_t U = (int_fast16_t)(((1-v) * tLZ0 + v*tRZ1) / wS * TEXTURE_SHIFT) & MASK;
        yTop    = clamp(yTop, clampY0, clampY1);
        yBottom = clamp(yBottom, clampY0, clampY1);
        int_fast16_t clampSY0 = clamp(yTop, 1, 479);
        int_fast16_t clampSY1 = clamp(yBottom, 1, 479);
        int row = clampSY0 * 640 + x;
        if (flat == 0) {
            ceilingLut[x] = clampSY0;
            floorLut[x] = clampSY1;
        } else if (flat == 1) {
            ceilingLut[x] = clampSY0;
        } else if (flat == 2) {
            floorLut[x] = clampSY1;
        }
        float dxY = (tBottom - tTop);
        float idxY = (1/dxY) * TEXTURE_DETAIL;
        int_fast16_t shade = (int_fast16_t)(1.0f + dS * 0.0025f);        
        uint16_t *base = &w_pixels[U];
        for (int y = clampSY0; y < clampSY1; y++) {
            int_fast16_t V = (((int)((y - tTop) * idxY) & MASK) << SHIFT);
            uint16_t baseColour = base[V];
            uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
            uint8_t g = (((baseColour >> 5)  & 0x3F) >> shade);
            uint8_t b = ((baseColour & 0x1F) >> shade);
            pixels[row] = (r << 11) | (g << 5) | b;
            row += 640;
        }
    }
};

static inline void draw_wall640x480x128(float sx0, float sx1, 
    float sy0, float sy1, float sy2, float sy3, Uint16 * restrict pixels, 
    portalCull portalBounds, int * restrict ceilingLut, int * restrict floorLut, int flat, uint16_t * restrict w_pixels, float t0, float t1, float wy0, float wy1, float length, float height, int tScale) {

    float dx = (sx1 > sx0) ? (sx1 - sx0) : 1.0f;
    int startX = clamp((int)sx0, portalBounds.x0, portalBounds.x1);
    int endX   = clamp((int)sx1, portalBounds.x0, portalBounds.x1);
    float invDx = 1.0f / dx;
    float invCullDx = 1.0f / portalBounds.dxCull;
    float z0 = 1/wy0;
    float z1 = 1/wy1;
    float sf = length/4;
    float tLeft = (1-t0) * 0 + t0*sf;
    float tRight = (1-t1) * 0 + t1*sf;
    float tLZ0 = tLeft/z0;
    float tRZ1 = tRight/z1;
    float wz0 = wy0/z0;
    float wz1 = wy1/z1;
    float wh = height/4;

    const int SHIFT = 7;
    const int TEXTURE_SHIFT = (1<<SHIFT);
    const float TEXTURE_DETAIL = (TEXTURE_SHIFT*wh);
    const unsigned int MASK = 0x3F;

    for (int x = startX; x < endX; x++) {
        float t = (x - sx0) * invDx;
        float v = (1-t);
        float st = (x - portalBounds.x0) * invCullDx;
        float wS = (1-v) * wy0 + v*wy1;
        float dS = ((1-v) * wz1 + v*wz0);
        int_fast16_t yTop    = (int)((1.0f - t) * sy0 + t * sy1);
        int_fast16_t yBottom = (int)((1.0f - t) * sy2 + t * sy3);
        int_fast16_t tTop = yTop;
        int_fast16_t tBottom = yBottom;
        int_fast16_t clampY0 = (int_fast16_t)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        int_fast16_t clampY1 = (int_fast16_t)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);
        int_fast16_t U = (int_fast16_t)(((1-v) * tLZ0 + v*tRZ1) / wS * TEXTURE_SHIFT) & MASK;
        yTop    = clamp(yTop, clampY0, clampY1);
        yBottom = clamp(yBottom, clampY0, clampY1);
        int_fast16_t clampSY0 = clamp(yTop, 1, 479);
        int_fast16_t clampSY1 = clamp(yBottom, 1, 479);
        int row = clampSY0 * 640 + x;
        if (flat == 0) {
            ceilingLut[x] = clampSY0;
            floorLut[x] = clampSY1;
        } else if (flat == 1) {
            ceilingLut[x] = clampSY0;
        } else if (flat == 2) {
            floorLut[x] = clampSY1;
        }
        float dxY = (tBottom - tTop);
        float idxY = (1/dxY) * TEXTURE_DETAIL;
        int_fast16_t shade = (int_fast16_t)(1.0f + dS * 0.0025f);        
        uint16_t *base = &w_pixels[U];
        for (int y = clampSY0; y < clampSY1; y++) {
            int_fast16_t V = (((int)((y - tTop) * idxY) & MASK) << SHIFT);
            uint16_t baseColour = base[V];
            uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
            uint8_t g = (((baseColour >> 5)  & 0x3F) >> shade);
            uint8_t b = ((baseColour & 0x1F) >> shade);
            pixels[row] = (r << 11) | (g << 5) | b;
            row += 640;
        }
    }
};

static inline void dispatch_wall(float sx0, float sx1, 
    float sy0, float sy1, float sy2, float sy3, Uint16 * restrict pixels, 
    portalCull portalBounds, int * restrict ceilingLut, int * restrict floorLut, int flat, uint16_t * restrict w_pixels, float t0, float t1, float wy0, float wy1, float length, float height, int tScale) {
    
    switch (resolutionSet) {
        case 0:
        {
            switch (tScale) {
                case 0:
                {
                    draw_wall1920x1080x64(sx0,sx1,sy0,sy1,sy2,sy3,pixels,portalBounds,ceilingLut,floorLut,flat,w_pixels,t0,t1,wy0,wy1,length,height, tScale);
                    break;
                }
                case 1:
                {
                    draw_wall1920x1080x128(sx0,sx1,sy0,sy1,sy2,sy3,pixels,portalBounds,ceilingLut,floorLut,flat,w_pixels,t0,t1,wy0,wy1,length,height, tScale);
                }
            }
            break;
        }
        case 1:
        {
            switch (tScale) {
                case 0:
                {
                    draw_wall1024x768x64(sx0,sx1,sy0,sy1,sy2,sy3,pixels,portalBounds,ceilingLut,floorLut,flat,w_pixels,t0,t1,wy0,wy1,length,height, tScale);
                    break;
                }
                case 1:
                {
                    draw_wall1024x768x128(sx0,sx1,sy0,sy1,sy2,sy3,pixels,portalBounds,ceilingLut,floorLut,flat,w_pixels,t0,t1,wy0,wy1,length,height, tScale);
                }
            }
            break;
        }
        case 2:
        {   
            switch (tScale) {
                case 0:
                {
                    draw_wall800x600x64(sx0,sx1,sy0,sy1,sy2,sy3,pixels,portalBounds,ceilingLut,floorLut,flat,w_pixels,t0,t1,wy0,wy1,length,height, tScale);
                    break;
                }
                case 1:
                {
                    draw_wall800x600x128(sx0,sx1,sy0,sy1,sy2,sy3,pixels,portalBounds,ceilingLut,floorLut,flat,w_pixels,t0,t1,wy0,wy1,length,height, tScale);
                }
            }
            break;
        }
        case 3:
        {
            switch (tScale) {
                case 0:
                {
                    draw_wall640x480x64(sx0,sx1,sy0,sy1,sy2,sy3,pixels,portalBounds,ceilingLut,floorLut,flat,w_pixels,t0,t1,wy0,wy1,length,height, tScale);
                    break;
                }
                case 1:
                {
                    draw_wall640x480x128(sx0,sx1,sy0,sy1,sy2,sy3,pixels,portalBounds,ceilingLut,floorLut,flat,w_pixels,t0,t1,wy0,wy1,length,height, tScale);
                }
            }
            break;
        }
    }
}

#define TWO_PI (2.0f * PI)
#define INV_TWO_PI (0.1591549430918953358f)  // 1/(2π)

// This should instead, later on, invert the output or something.
const float B =  1.27323954473516268f;
const float C = -0.40528473456935109f;
static inline __attribute__((always_inline))
float fast_sin(float x) {
    float t = x * INV_TWO_PI + (x >= 0 ? 0.5f : -0.5f);
    int   k = (int)t; 
    x = x - (float)k * TWO_PI;
    float ax = fabsf(x);
    return (B * x) + (C * x * ax);
}


static inline __attribute__((always_inline)) float fast_cos(float x) {
    return fast_sin(x + 0.5f * PI);
}

static float sinBetaArr[1920];
static float invSinBetaArr[1920];
static float sinAlphaArr[1920];
static float cosAlphaArr[1920];
static int clampY0Arr[1920];
static int clampY1Arr[1920];

static inline void draw_flat1920x1080(Uint16 *restrict pixels, int *restrict lut, int flat, portalCull portalBounds, float elevation, float fov, float yaw, float f, float cx, float cy, uint16_t *restrict t_pixels) {
    const int SH2 = 1080 / 2;
    const int TX_SHIFT = 3;  // 1 << 3 = 8
    const int TY_SHIFT = 6;  // 1 << 6 = 64
    const int TX_MASK = 0x3F;
    const int TY_MASK = 0x3F;

    int X0 = clamp(clamp(0, portalBounds.x0, portalBounds.x1), 1, 1919);
    int X1 = clamp(clamp(1919, portalBounds.x0, portalBounds.x1), 1, 1919);

    float radFOV = fov * PI / 180.0f;
    float halfRFOV = radFOV / 2.0f;
    float radYaw = ((int)yaw) * PI / 180.0f;
    float elevationFactor = elevation * f;
    float radFOV_1919 = radFOV / 1919.0f;

    // Precompute per-x values
    for (int x = X0; x < X1; x++) {
        int i = x - X0;
        float st = (float)(x - portalBounds.x0) / portalBounds.dxCull;
        float beta = ((float)x) * radFOV_1919 + halfRFOV;
        float alpha = radYaw + beta;

        float sin_beta = fast_sin(beta);
        // Prevent division by zero - clamp sin_beta
        if (fabsf(sin_beta) < 1e-6f) sin_beta = 1e-6f;

        sinBetaArr[i] = sin_beta;
        invSinBetaArr[i] = 1.0f / sin_beta;

        sinAlphaArr[i] = fast_sin(alpha);
        cosAlphaArr[i] = fast_cos(alpha);

        clampY0Arr[i] = (int)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        clampY1Arr[i] = (int)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);
    }

    if (flat == 1) {
        // Precompute distLUT for R = abs(y - SH2)
        static float distLUT[1080];
        for (int y = 0; y < 1080; y++) {
            int R = abs(y - SH2);
            distLUT[y] = (R != 0) ? elevationFactor / (float)R : 0.0f;
        }

        for (int x = X0; x < X1; x++) {
            int i = x - X0;
            float inv_sin_beta = invSinBetaArr[i];
            float sin_alpha = sinAlphaArr[i];
            float cos_alpha = cosAlphaArr[i];

            int clampY0 = clampY0Arr[i];
            int clampY1 = clampY1Arr[i];

            int Y0 = clamp(1, clampY0, clampY1);
            int Y1 = clamp(lut[x], clampY0, clampY1);

            int row = Y0 * 1920 + x;
            for (int y = Y0; y < Y1; y++) {
                float straightDist = distLUT[y];
                float d = straightDist * inv_sin_beta;

                float wx = cy + sin_alpha * d;
                float wy = cx - cos_alpha * d;

                int tx = ((int)(wx * (1 << TX_SHIFT))) & TX_MASK;
                int ty = ((int)(wy * (1 << TX_SHIFT))) & TY_MASK;

                int index = ty * (1 << TY_SHIFT) + tx;

                int dS = ((int)(straightDist * (1 << TY_SHIFT))) / f;
                int shade = 1 + dS;

                uint16_t baseColour = t_pixels[index];
                uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
                uint8_t g = (((baseColour >> 5) & 0x3F) >> shade);
                uint8_t b = ((baseColour & 0x1F) >> shade);

                pixels[row] = (r << 11) | (g << 5) | b;
                row += 1920;
            }
        }
    } else if (flat == 2) {
        for (int x = X0; x < X1; x++) {
            int i = x - X0;
            float inv_sin_beta = invSinBetaArr[i];
            float sin_alpha = sinAlphaArr[i];
            float cos_alpha = cosAlphaArr[i];

            int clampY0 = clampY0Arr[i];
            int clampY1 = clampY1Arr[i];

            int Y1 = clamp(1080 - 1, clampY0, clampY1);
            int Y0 = clamp(lut[x], clampY0, clampY1);

            int row = Y0 * 1920 + x;
            for (int y = Y0; y < Y1; y++) {
                float R = (float)(y - SH2);
                if (fabsf(R) < 1e-6f) R = 1e-6f; // Avoid div zero
                float straightDist = elevationFactor / R;
                float d = straightDist * inv_sin_beta;

                float wx = cy - sin_alpha * d;
                float wy = cx + cos_alpha * d;

                int tx = ((int)(wx * (1 << TX_SHIFT))) & TX_MASK;
                int ty = ((int)(wy * (1 << TX_SHIFT))) & TY_MASK;

                int index = ty * (1 << TY_SHIFT) + tx;

                int dS = abs((int)(straightDist * (1 << TY_SHIFT))) / f;
                int shade = 1 + dS;

                uint16_t baseColour = t_pixels[index];
                uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
                uint8_t g = (((baseColour >> 5) & 0x3F) >> shade);
                uint8_t b = ((baseColour & 0x1F) >> shade);

                pixels[row] = (r << 11) | (g << 5) | b;
                row += 1920;
            }
        }
    }
}

static inline void draw_flat1024x768(Uint16 *restrict pixels, int *restrict lut, int flat, portalCull portalBounds, float elevation, float fov, float yaw, float f, float cx, float cy, uint16_t *restrict t_pixels) {
    const int SH2 = 768 / 2;
    const int TX_SHIFT = 3;  // 1 << 3 = 8
    const int TY_SHIFT = 6;  // 1 << 6 = 64
    const int TX_MASK = 0x3F;
    const int TY_MASK = 0x3F;

    int X0 = clamp(clamp(0, portalBounds.x0, portalBounds.x1), 1, 1023);
    int X1 = clamp(clamp(1023, portalBounds.x0, portalBounds.x1), 1, 1023);

    float radFOV = fov * PI / 180.0f;
    float halfRFOV = radFOV / 2.0f;
    float radYaw = ((int)yaw) * PI / 180.0f;
    float elevationFactor = elevation * f;
    float radFOV_1023 = radFOV / 1023.0f;

    // Precompute per-x values
    for (int x = X0; x < X1; x++) {
        int i = x - X0;
        float st = (float)(x - portalBounds.x0) / portalBounds.dxCull;
        float beta = ((float)x) * radFOV_1023 + halfRFOV;
        float alpha = radYaw + beta;

        float sin_beta = fast_sin(beta);
        // Prevent division by zero - clamp sin_beta
        if (fabsf(sin_beta) < 1e-6f) sin_beta = 1e-6f;

        sinBetaArr[i] = sin_beta;
        invSinBetaArr[i] = 1.0f / sin_beta;

        sinAlphaArr[i] = fast_sin(alpha);
        cosAlphaArr[i] = fast_cos(alpha);

        clampY0Arr[i] = (int)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        clampY1Arr[i] = (int)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);
    }

    if (flat == 1) {
        // Precompute distLUT for R = abs(y - SH2)
        static float distLUT[768];
        for (int y = 0; y < 768; y++) {
            int R = abs(y - SH2);
            distLUT[y] = (R != 0) ? elevationFactor / (float)R : 0.0f;
        }

        for (int x = X0; x < X1; x++) {
            int i = x - X0;
            float inv_sin_beta = invSinBetaArr[i];
            float sin_alpha = sinAlphaArr[i];
            float cos_alpha = cosAlphaArr[i];

            int clampY0 = clampY0Arr[i];
            int clampY1 = clampY1Arr[i];

            int Y0 = clamp(1, clampY0, clampY1);
            int Y1 = clamp(lut[x], clampY0, clampY1);

            int row = Y0 * 1024 + x;
            for (int y = Y0; y < Y1; y++) {
                float straightDist = distLUT[y];
                float d = straightDist * inv_sin_beta;

                float wx = cy + sin_alpha * d;
                float wy = cx - cos_alpha * d;

                int tx = ((int)(wx * (1 << TX_SHIFT))) & TX_MASK;
                int ty = ((int)(wy * (1 << TX_SHIFT))) & TY_MASK;

                int index = ty * (1 << TY_SHIFT) + tx;

                int dS = ((int)(straightDist * (1 << TY_SHIFT))) / f;
                int shade = 1 + dS;

                uint16_t baseColour = t_pixels[index];
                uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
                uint8_t g = (((baseColour >> 5) & 0x3F) >> shade);
                uint8_t b = ((baseColour & 0x1F) >> shade);

                pixels[row] = (r << 11) | (g << 5) | b;
                row += 1024;
            }
        }
    } else if (flat == 2) {
        for (int x = X0; x < X1; x++) {
            int i = x - X0;
            float inv_sin_beta = invSinBetaArr[i];
            float sin_alpha = sinAlphaArr[i];
            float cos_alpha = cosAlphaArr[i];

            int clampY0 = clampY0Arr[i];
            int clampY1 = clampY1Arr[i];

            int Y1 = clamp(767, clampY0, clampY1);
            int Y0 = clamp(lut[x], clampY0, clampY1);

            int row = Y0 * 1024 + x;
            for (int y = Y0; y < Y1; y++) {
                float R = (float)(y - SH2);
                if (fabsf(R) < 1e-6f) R = 1e-6f; // Avoid div zero
                float straightDist = elevationFactor / R;
                float d = straightDist * inv_sin_beta;

                float wx = cy - sin_alpha * d;
                float wy = cx + cos_alpha * d;

                int tx = ((int)(wx * (1 << TX_SHIFT))) & TX_MASK;
                int ty = ((int)(wy * (1 << TX_SHIFT))) & TY_MASK;

                int index = ty * (1 << TY_SHIFT) + tx;

                int dS = abs((int)(straightDist * (1 << TY_SHIFT))) / f;
                int shade = 1 + dS;

                uint16_t baseColour = t_pixels[index];
                uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
                uint8_t g = (((baseColour >> 5) & 0x3F) >> shade);
                uint8_t b = ((baseColour & 0x1F) >> shade);

                pixels[row] = (r << 11) | (g << 5) | b;
                row += 1024;
            }
        }
    }
}

static inline void draw_flat800x600(Uint16 *restrict pixels, int *restrict lut, int flat, portalCull portalBounds, float elevation, float fov, float yaw, float f, float cx, float cy, uint16_t *restrict t_pixels) {
    const int SH2 = 600 / 2;
    const int TX_SHIFT = 3;  // 1 << 3 = 8
    const int TY_SHIFT = 6;  // 1 << 6 = 64
    const int TX_MASK = 0x3F;
    const int TY_MASK = 0x3F;

    int X0 = clamp(clamp(0, portalBounds.x0, portalBounds.x1), 1, 799);
    int X1 = clamp(clamp(799, portalBounds.x0, portalBounds.x1), 1, 799);

    float radFOV = fov * PI / 180.0f;
    float halfRFOV = radFOV / 2.0f;
    float radYaw = ((int)yaw) * PI / 180.0f;
    float elevationFactor = elevation * f;
    float radFOV_799 = radFOV / 799.0f;

    // Precompute per-x values
    for (int x = X0; x < X1; x++) {
        int i = x - X0;
        float st = (float)(x - portalBounds.x0) / portalBounds.dxCull;
        float beta = ((float)x) * radFOV_799 + halfRFOV;
        float alpha = radYaw + beta;

        float sin_beta = fast_sin(beta);
        // Prevent division by zero - clamp sin_beta
        if (fabsf(sin_beta) < 1e-6f) sin_beta = 1e-6f;

        sinBetaArr[i] = sin_beta;
        invSinBetaArr[i] = 1.0f / sin_beta;

        sinAlphaArr[i] = fast_sin(alpha);
        cosAlphaArr[i] = fast_cos(alpha);

        clampY0Arr[i] = (int)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        clampY1Arr[i] = (int)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);
    }

    if (flat == 1) {
        // Precompute distLUT for R = abs(y - SH2)
        static float distLUT[600];
        for (int y = 0; y < 600; y++) {
            int R = abs(y - SH2);
            distLUT[y] = (R != 0) ? elevationFactor / (float)R : 0.0f;
        }

        for (int x = X0; x < X1; x++) {
            int i = x - X0;
            float inv_sin_beta = invSinBetaArr[i];
            float sin_alpha = sinAlphaArr[i];
            float cos_alpha = cosAlphaArr[i];

            int clampY0 = clampY0Arr[i];
            int clampY1 = clampY1Arr[i];

            int Y0 = clamp(1, clampY0, clampY1);
            int Y1 = clamp(lut[x], clampY0, clampY1);

            int row = Y0 * 800 + x;
            for (int y = Y0; y < Y1; y++) {
                float straightDist = distLUT[y];
                float d = straightDist * inv_sin_beta;

                float wx = cy + sin_alpha * d;
                float wy = cx - cos_alpha * d;

                int tx = ((int)(wx * (1 << TX_SHIFT))) & TX_MASK;
                int ty = ((int)(wy * (1 << TX_SHIFT))) & TY_MASK;

                int index = ty * (1 << TY_SHIFT) + tx;

                int dS = ((int)(straightDist * (1 << TY_SHIFT))) / f;
                int shade = 1 + dS;

                uint16_t baseColour = t_pixels[index];
                uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
                uint8_t g = (((baseColour >> 5) & 0x3F) >> shade);
                uint8_t b = ((baseColour & 0x1F) >> shade);

                pixels[row] = (r << 11) | (g << 5) | b;
                row += 800;
            }
        }
    } else if (flat == 2) {
        for (int x = X0; x < X1; x++) {
            int i = x - X0;
            float inv_sin_beta = invSinBetaArr[i];
            float sin_alpha = sinAlphaArr[i];
            float cos_alpha = cosAlphaArr[i];

            int clampY0 = clampY0Arr[i];
            int clampY1 = clampY1Arr[i];

            int Y1 = clamp(600 - 1, clampY0, clampY1);
            int Y0 = clamp(lut[x], clampY0, clampY1);

            int row = Y0 * 800 + x;
            for (int y = Y0; y < Y1; y++) {
                float R = (float)(y - SH2);
                if (fabsf(R) < 1e-6f) R = 1e-6f; // Avoid div zero
                float straightDist = elevationFactor / R;
                float d = straightDist * inv_sin_beta;

                float wx = cy - sin_alpha * d;
                float wy = cx + cos_alpha * d;

                int tx = ((int)(wx * (1 << TX_SHIFT))) & TX_MASK;
                int ty = ((int)(wy * (1 << TX_SHIFT))) & TY_MASK;

                int index = ty * (1 << TY_SHIFT) + tx;

                int dS = abs((int)(straightDist * (1 << TY_SHIFT))) / f;
                int shade = 1 + dS;

                uint16_t baseColour = t_pixels[index];
                uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
                uint8_t g = (((baseColour >> 5) & 0x3F) >> shade);
                uint8_t b = ((baseColour & 0x1F) >> shade);

                pixels[row] = (r << 11) | (g << 5) | b;
                row += 800;
            }
        }
    }
}

static inline void draw_flat640x480(Uint16 *restrict pixels, int *restrict lut, int flat, portalCull portalBounds, float elevation, float fov, float yaw, float f, float cx, float cy, uint16_t *restrict t_pixels) {
    const int SH2 = 480 / 2;
    const int TX_SHIFT = 3;  // 1 << 3 = 8
    const int TY_SHIFT = 6;  // 1 << 6 = 64
    const int TX_MASK = 0x3F;
    const int TY_MASK = 0x3F;

    int X0 = clamp(clamp(0, portalBounds.x0, portalBounds.x1), 1, 639);
    int X1 = clamp(clamp(639, portalBounds.x0, portalBounds.x1), 1, 639);

    float radFOV = fov * PI / 180.0f;
    float halfRFOV = radFOV / 2.0f;
    float radYaw = ((int)yaw) * PI / 180.0f;
    float elevationFactor = elevation * f;
    float radFOV_639 = radFOV / 639.0f;

    // Precompute per-x values
    for (int x = X0; x < X1; x++) {
        int i = x - X0;
        float st = (float)(x - portalBounds.x0) / portalBounds.dxCull;
        float beta = ((float)x) * radFOV_639 + halfRFOV;
        float alpha = radYaw + beta;

        float sin_beta = fast_sin(beta);
        // Prevent division by zero - clamp sin_beta
        if (fabsf(sin_beta) < 1e-6f) sin_beta = 1e-6f;

        sinBetaArr[i] = sin_beta;
        invSinBetaArr[i] = 1.0f / sin_beta;

        sinAlphaArr[i] = fast_sin(alpha);
        cosAlphaArr[i] = fast_cos(alpha);

        clampY0Arr[i] = (int)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        clampY1Arr[i] = (int)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);
    }

    if (flat == 1) {
        // Precompute distLUT for R = abs(y - SH2)
        static float distLUT[480];
        for (int y = 0; y < 480; y++) {
            int R = abs(y - SH2);
            distLUT[y] = (R != 0) ? elevationFactor / (float)R : 0.0f;
        }

        for (int x = X0; x < X1; x++) {
            int i = x - X0;
            float inv_sin_beta = invSinBetaArr[i];
            float sin_alpha = sinAlphaArr[i];
            float cos_alpha = cosAlphaArr[i];

            int clampY0 = clampY0Arr[i];
            int clampY1 = clampY1Arr[i];

            int Y0 = clamp(1, clampY0, clampY1);
            int Y1 = clamp(lut[x], clampY0, clampY1);

            int row = Y0 * 640 + x;
            for (int y = Y0; y < Y1; y++) {
                float straightDist = distLUT[y];
                float d = straightDist * inv_sin_beta;

                float wx = cy + sin_alpha * d;
                float wy = cx - cos_alpha * d;

                int tx = ((int)(wx * (1 << TX_SHIFT))) & TX_MASK;
                int ty = ((int)(wy * (1 << TX_SHIFT))) & TY_MASK;

                int index = ty * (1 << TY_SHIFT) + tx;

                int dS = ((int)(straightDist * (1 << TY_SHIFT))) / f;
                int shade = 1 + dS;

                uint16_t baseColour = t_pixels[index];
                uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
                uint8_t g = (((baseColour >> 5) & 0x3F) >> shade);
                uint8_t b = ((baseColour & 0x1F) >> shade);

                pixels[row] = (r << 11) | (g << 5) | b;
                row += 640;
            }
        }
    } else if (flat == 2) {
        for (int x = X0; x < X1; x++) {
            int i = x - X0;
            float inv_sin_beta = invSinBetaArr[i];
            float sin_alpha = sinAlphaArr[i];
            float cos_alpha = cosAlphaArr[i];

            int clampY0 = clampY0Arr[i];
            int clampY1 = clampY1Arr[i];

            int Y1 = clamp(480 - 1, clampY0, clampY1);
            int Y0 = clamp(lut[x], clampY0, clampY1);

            int row = Y0 * 640 + x;
            for (int y = Y0; y < Y1; y++) {
                float R = (float)(y - SH2);
                if (fabsf(R) < 1e-6f) R = 1e-6f; // Avoid div zero
                float straightDist = elevationFactor / R;
                float d = straightDist * inv_sin_beta;

                float wx = cy - sin_alpha * d;
                float wy = cx + cos_alpha * d;

                int tx = ((int)(wx * (1 << TX_SHIFT))) & TX_MASK;
                int ty = ((int)(wy * (1 << TX_SHIFT))) & TY_MASK;

                int index = ty * (1 << TY_SHIFT) + tx;

                int dS = abs((int)(straightDist * (1 << TY_SHIFT))) / f;
                int shade = 1 + dS;

                uint16_t baseColour = t_pixels[index];
                uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
                uint8_t g = (((baseColour >> 5) & 0x3F) >> shade);
                uint8_t b = ((baseColour & 0x1F) >> shade);

                pixels[row] = (r << 11) | (g << 5) | b;
                row += 640;
            }
        }
    }
}

static inline void dispatch_flat(Uint16 *restrict pixels, int *restrict lut, int flat, portalCull portalBounds, float elevation, float fov, float yaw, float f, float cx, float cy, uint16_t *restrict t_pixels, int scale) {
    switch (resolutionSet) {
        case 0:
        {
            draw_flat1920x1080(pixels, lut, flat, portalBounds, elevation, fov, yaw, f, cx, cy, t_pixels);
            break;
        }
        case 1:
        {
            draw_flat1024x768(pixels, lut, flat, portalBounds, elevation, fov, yaw, f, cx, cy, t_pixels);
            break;
        }
        case 2:
        {
            draw_flat800x600(pixels, lut, flat, portalBounds, elevation, fov, yaw, f, cx, cy, t_pixels);
            break;
        }
        case 3:
        {
            draw_flat640x480(pixels, lut, flat, portalBounds, elevation, fov, yaw, f, cx, cy, t_pixels);
            break;
        }
    }
}

static inline void R_RENDER_ENTITY(Uint16 *restrict pixels, uint16_t *restrict t_pixels, portalCull portalBounds, int16_t x, int16_t y, float ty, float f, int W, int H) {
    if (ty <= 0.01f || portalBounds.dxCull <= 0.0001f) return;
    
    const float scaleX = (float)(W*64) / ty; // 64*64 = 4096
    const int tScaleX = (int)scaleX;
    const int tHalfX = tScaleX >> 1;

    const float scaleY = (float)(H*64) / ty; // 64*64 = 4096
    const int tScaleY = (int)scaleY;
    const int tHalfY = tScaleY >> 1;

    const int usx = x - tHalfX;
    const int uex = x + tHalfX;
    const int usy = y - tHalfY;
    const int uey = y + tHalfY;

    const int sx = clamp(usx, 1, SW1);
    const int ex = clamp(uex, 1, SW1);

    const int tex_w = W;
    const int tex_h = H;

    const float inv_dx = 1.0f / portalBounds.dxCull;
    const float inv_w = 1.0f / (float)(uex - usx);
    const float inv_h = 1.0f / (float)(uey - usy);

    for (int px = sx; px < ex; ++px) {
        float st = (px - portalBounds.x0) * inv_dx;
        if (st < 0.0f || st > 1.0f) continue;

        int clampY0 = (int)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        int clampY1 = (int)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);

        int sy = clamp(usy, clampY0, clampY1);
        int ey = clamp(uey, clampY0, clampY1);
        sy = clamp(sy, 1, SH1);
        ey = clamp(ey, 1, SH1);

        if (ey <= sy) continue;

        float v = (float)(px - usx) * inv_w;
        int tpx = (int)(v * tex_w);
        if ((unsigned)tpx >= tex_w) continue;

        for (int py = sy; py < ey; ++py) {
            float u = (float)(py - usy) * inv_h;
            int tpy = (int)(u * tex_h);
            if ((unsigned)tpy >= tex_h) continue;

            int tex_idx = tpy * tex_w + tpx;
            uint16_t color = t_pixels[tex_idx];
            if (!color) continue;

            pixels[py * SW + px] = color;
        }
    }
}





#endif