#ifndef raster
#define raster

#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"
#include "tloader.h"
#include "ini_parser.c"

/*
extern int draw_wall_Y(
    float x0, 
    float x1, 
    float y0, 
    float y1, 
    float y2, 
    float y3, 
    long boundsPtr,
    long pixelsPtr,
    int SW,
    int SH1
) __fastcall;
*/

extern void draw_wall_Y(
    long long pxPtr,
    int SW,
    int y0,
    int y1,
    int tTop,
    float idxY,
    int shade,
    int U,
    long long wtPtr
) __fastcall;

//#include "config.h"

//     clang -std=c17 main.c -O3 -ffast-math -flto -march=native -funroll-loops -IC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2 -LC:\Users\frase\OneDrive\Desktop\CProjects\C\SDL2\lib -Wall -lmingw32 -lSDL2main -lSDL2 -o main

int *ceilingLut;
int *floorLut;

uint8_t traversedPortals[MAX_WALLS];
int originSector = 0;

static inline float lerp(float a, float b, float t) {
    return (1 - a) * b + a * t;
}

static inline void clip(float *ax, float *ay, float *T, float bx, float by, float px1, float py1, float px2, float py2) {
    float a = (px1 - px2) * (*ay - py2) - (py1 - py2) * (*ax - px2);
    float b = (py1 - py2) * (*ax - bx) - (px1 - px2) * (*ay - by);
    float t = a / (b+1);
    
    *T = t;
    *ax = *ax - t * (bx - *ax);
    *ay = *ay - t * (by - *ay);
};

int pointWallCollision(float px, float py, float ax, float ay, float bx, float by, float radius) {
    float abx = bx-ax;
    float aby = by-ay;
    float apx = px-ax;
    float apy = py-ay;

    float ab2 = abx*abx + aby*aby;
    float ap_ab = apx*abx + apy*aby;
    float t = ap_ab/ab2;

    float cX = ax+t*abx;
    float cY = ay+t*aby;

    float dx = px-cX;
    float dy = py-cY;
    float colliding = (dx*dx + dy*dy);
    if (colliding < (radius*radius)) {
        return 1;
    } else {
        return 0;
    }
}

static inline void draw_wall(float sx0, float sx1, 
    float sy0, float sy1, float sy2, float sy3, Uint16 * restrict pixels, 
    portalCull portalBounds, int * restrict ceilingLut, int * restrict floorLut, int flat, uint16_t * restrict w_pixels, float t0, float t1, float wy0, float wy1, float length, float height) {

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

    float TEXTURE_DETAIL = ((1 << 6)*wh);

    for (int x = startX; x < endX; x++) {
        float t = (x - sx0) * invDx;
        float v = (1-t);
        float st = (x - portalBounds.x0) * invCullDx;

        float wS = (1-v) * wy0 + v*wy1;
        float dS = ((1-v) * wz1 + v*wz0);// / ((1-v) * wy0 + v*wy1);

        int_fast16_t yTop    = (int)((1.0f - t) * sy0 + t * sy1);
        int_fast16_t yBottom = (int)((1.0f - t) * sy2 + t * sy3);

        int_fast16_t tTop = yTop;
        int_fast16_t tBottom = yBottom;

        int_fast16_t clampY0 = (int_fast16_t)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
        int_fast16_t clampY1 = (int_fast16_t)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);

        int_fast16_t U __attribute__ ((aligned (4))) = (int_fast16_t)(((1-v) * tLZ0 + v*tRZ1) / wS * (1<<6)) & 0x3F;

        yTop    = clamp(yTop, clampY0, clampY1);
        yBottom = clamp(yBottom, clampY0, clampY1);

        int_fast16_t clampSY0 = clamp(yTop, 1, SH1);
        int_fast16_t clampSY1 = clamp(yBottom, 1, SH1);
        int row = clampSY0 * SW + x;  // Start row at the correct position

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
        //draw_wall_Y((long long)&pixels[row], SW, clampSY0, clampSY1, tTop, idxY, shade, U, (long long) &w_pixels);
        uint16_t *base = &w_pixels[U];

        for (int y = clampSY0; y < clampSY1; y++) {
            // Calculate wrapped texY directly using merged constants
            int_fast16_t V = (((int)((y - tTop) * idxY) & 0x3F) << 6);
            uint16_t baseColour = base[V];
            uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
            uint8_t g = (((baseColour >> 5)  & 0x3F) >> shade);
            uint8_t b = ((baseColour & 0x1F) >> shade);
            
            pixels[row] = (r << 11) | (g << 5) | b;
            row += SW; // check if ASM output on C loads in SW from memory instead of registers
        }
    }
};

static inline void draw_flat(Uint16 *restrict pixels, int *restrict lut, int flat, portalCull portalBounds, float elevation, float fov, float yaw, float f, float cx, float cy, uint16_t *restrict t_pixels) {
    int X0 = clamp(clamp((int)0, portalBounds.x0, portalBounds.x1), 1, SW1);
    int X1 = clamp(clamp((int)SW1, portalBounds.x0, portalBounds.x1), 1, SW1);

    float radFOV = (float)fov*PI/180.0;
    float halfRFOV = radFOV/2.0;
    float radYaw = (float)((int)yaw)*PI/180.0;
    float elevationFactor = elevation * f;
    switch (flat) {
        case 1:
            for (int x=X0; x<X1; x++) {
                float st = (x-portalBounds.x0)/portalBounds.dxCull;
                float beta = ((float)x/(float)SW1)*radFOV+halfRFOV;
                float alpha = radYaw+beta;
                float sin_beta = sinf(beta);
                float sin_alpha = sinf(alpha);
                float cos_alpha = cosf(alpha);
                
                float inv_sin_beta = 1/sin_beta;
                
                int clampY0 = (int)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
                int clampY1 = (int)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);

                // Clamp the Y values once for the whole x-range
                int Y0 = clamp(1, clampY0, clampY1);
                int Y1 = clamp(lut[x], clampY0, clampY1);

                int row = Y0 * SW + x;  // Start row at the correct position

                // Precompute values that don't change in the loop
                for (int y = Y0; y < Y1; y++) {
                    // Perform the color lookup and set the pixel
                    float R = abs(y - SH2);
                    float straightDist = elevationFactor / R;
                    float d = (straightDist * inv_sin_beta);
                    float wx = cy + (sin_alpha * d);
                    float wy = cx - (cos_alpha * d);
                    // Calculate texture coordinates
                    register int tx = ((int)(wx * (1<<3))) & 0x3F;
                    register int ty = ((int)(wy * (1<<3))) & 0x3F;

                    // YX indexing
                    register int index = ty * (1 << 6) + tx;
                    int dS = ((int)straightDist<<6)/f;
                    int shade = (int)(1+dS);
                    
                    register uint16_t baseColour = t_pixels[index];
                    register uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
                    register uint8_t g = (((baseColour >> 5)  & 0x3F) >> shade);
                    register uint8_t b = ((baseColour & 0x1F) >> shade);
                    
                    pixels[row] = (r << 11) | (g << 5) | b;
                    row += SW;
                }
            }
            break;
        case 2:
        for (int x=X0; x<X1; x++) {
            float st = (x-portalBounds.x0)/portalBounds.dxCull;
            float beta = ((float)x/(float)SW1)*radFOV+halfRFOV;
            float alpha = radYaw+beta;
            float sin_beta = sinf(beta);
            float sin_alpha = sinf(alpha);
            float cos_alpha = cosf(alpha);
            
            int clampY0 = (int)((1.0f - st) * portalBounds.y0 + st * portalBounds.y1);
            int clampY1 = (int)((1.0f - st) * portalBounds.y2 + st * portalBounds.y3);

            int Y1 = clamp(SH1, clampY0, clampY1);
            int Y0 = clamp(lut[x], clampY0, clampY1);

            int row = Y0*SW+x;
            for (int y = Y0; y < Y1; y++) {
                // Perform the color lookup and set the pixel
                float R = (y-SH2);
                float straightDist = elevationFactor / R;
                float d = (straightDist / sin_beta);
                float wx = cy - (sin_alpha * d);
                float wy = cx + (cos_alpha * d);
                // Calculate texture coordinates
                register int tx = ((int)(wx * (1<<3))) & 0x3F;
                register int ty = ((int)(wy * (1<<3))) & 0x3F;

                // YX indexing
                register int index = ty * (1 << 6) + tx;
                int dS = abs((int)straightDist<<6)/f;
                int shade = (int)(1+dS);
                register uint16_t baseColour = t_pixels[index];
                register uint8_t r = (((baseColour >> 11) & 0x1F) >> shade);
                register uint8_t g = (((baseColour >> 5)  & 0x3F) >> shade);
                register uint8_t b = ((baseColour & 0x1F) >> shade);
                
                pixels[row] = (r << 11) | (g << 5) | b;
                // Move to the next row
                row += SW;
            }
        }
        break;
    }
};

void drawSector(Uint16 *pixels, Level *level, sector *Sector, player character, float pSn, float pCs, portalCull portalBounds) {
    wall *walls = Sector->walls;
    size_t wallCount = Sector->count;
    
    float px = character.x;
    float py = character.y;
    float pz = character.z+character.h;
    float wz0 = pz - Sector->height - Sector->elevation;
    float wz1 = pz - Sector->elevation;
    
    float ez0 = (Sector->height +  Sector->elevation)-pz;
    float ez1 = Sector->elevation-pz;

    portalRender *portalQueue = NULL;
    int portalCount = 0;

    tfile ceiling_texture = texture_list->files[Sector->cIndex];
    tfile floor_texture = texture_list->files[Sector->fIndex];

    for (int wallIndex=0;wallIndex<wallCount;wallIndex++) {
        wall cWall = walls[wallIndex];
        tfile wall_texture = texture_list->files[cWall.tIndex];

        float rx0 = cWall.x0 - px;
        float ry0 = cWall.y0 - py;
        float rx1 = cWall.x1 - px;
        float ry1 = cWall.y1 - py;

        float tx0 = rx0 * pCs - ry0 * pSn;
        float ty0 = ry0 * pCs + rx0 * pSn;
        float tx1 = rx1 * pCs - ry1 * pSn;
        float ty1 = ry1 * pCs + rx1 * pSn;
        if (ty0 < 1 & ty1 < 1) {
            continue;
        }
        if (((((cWall.y1-cWall.y0) * rx1) + (-(cWall.x1-cWall.x0)*ry1)) > -1)) {
            continue;
        }
        //int hasClipped = 0;
        float t0 = 0;
        float t1 = 1;

        float t = 0;
        if (ty0 < 1) {
            clip(&tx0, &ty0, &t, tx1, ty1, 1.0, 1.0, (float)SW, 1.0);
            //hasClipped = 1;
            t1 += t/character.fovWidth;
        } 
        if (ty1 < 1) {
            clip(&tx1, &ty1, &t, tx0, ty0, 1.0, 1.0, (float)SW, 1.0);
            //hasClipped = 2;
            t0 -= t/character.fovWidth;
        }
        float inv_ty0 = (1/ty0)*character.focalLength;
        float inv_ty1 = (1/ty1)*character.focalLength;
        
        if (cWall.is_portal == 0) {
            float sx0 = tx0 * inv_ty0 + SW2;
            float sx1 = tx1 * inv_ty1 + SW2;
            float sy0 = wz0 * inv_ty0 + SH2;
            float sy1 = wz0 * inv_ty1 + SH2;
            float sy2 = wz1 * inv_ty0 + SH2;
            float sy3 = wz1 * inv_ty1 + SH2;
            draw_wall(sx0, sx1, sy0, sy1, sy2, sy3, pixels, portalBounds, ceilingLut, floorLut, 0, wall_texture.pixels, t0, t1, ty0, ty1, cWall.length, Sector->height);
        } else {
            float pz1 = wz0 + Sector->height;
            float pz0 = pz1 - cWall.portal_bottom;
            float pz2 = wz0;
            float pz3 = pz2 + cWall.portal_top;

            // Perspective
            float sx0 = tx0 * inv_ty0 + SW2;
            float sy0 = pz0 * inv_ty0 + SH2;

            float sx1 = tx1 * inv_ty1 + SW2;
            float sy1 = pz0 * inv_ty1 + SH2;

            float sy2 = pz1 * inv_ty0 + SH2;
            float sy3 = pz1 * inv_ty1 + SH2;

            float sy4 = pz2 * inv_ty0 + SH2;
            float sy5 = pz2 * inv_ty1 + SH2;

            float sy6 = pz3 * inv_ty0 + SH2;
            float sy7 = pz3 * inv_ty1 + SH2;
            
            draw_wall(sx0, sx1, sy0, sy1, sy2, sy3, pixels, portalBounds, ceilingLut, floorLut, 2, wall_texture.pixels, t0, t1, ty0, ty1, cWall.length, cWall.portal_top);
            draw_wall(sx0, sx1, sy4, sy5, sy6, sy7, pixels, portalBounds, ceilingLut, floorLut, 1, wall_texture.pixels, t0, t1, ty0, ty1, cWall.length, cWall.portal_bottom);
            
            portalRender *temp = realloc(portalQueue, (portalCount+1) * sizeof(portalRender));
            if (!temp) {
                perror("Failed to realloc memory for portal queue");
                free(portalQueue);
                //exit(EXIT_FAILURE);
            }
            portalQueue = temp;
            
            portalRender newPortal;
            newPortal.clipped = 0;
            newPortal.sector_link = cWall.portal_link;
            newPortal.uid = cWall.uid;

            float dxCull = (sx1 > sx0) ? (sx1 - sx0) : 1.0f;

            if (sx0 < portalBounds.x0) {
                
                float t = (portalBounds.x0-sx0)/dxCull;
                sx0 = portalBounds.x0;

                sy6 = (1-t)*sy6 + t*sy7;
                sy0 = (1-t)*sy0 + t*sy1;
                dxCull = sx1-sx0;
            } 
            if (sx1 > portalBounds.x1) {
                
                float t = (portalBounds.x1-sx0)/dxCull;
                sx1 = portalBounds.x1;

                sy7 = (1-t)*sy6 + t*sy7;
                sy1 = (1-t)*sy0 + t*sy1;
                dxCull = sx1-sx0;
            }

            portalCull newPortalBounds = {sx0, sx1, sy6, sy7, sy0, sy1, dxCull};
            newPortal.portalBounds = newPortalBounds;
            portalQueue[portalCount++] = newPortal;
        }
    }
    draw_flat(pixels, ceilingLut, 1, portalBounds, ez0, character.fov, character.yaw, character.focalLength, character.x, character.y, ceiling_texture.pixels);
    draw_flat(pixels, floorLut, 2, portalBounds, ez1, character.fov, character.yaw, character.focalLength, character.x, character.y, floor_texture.pixels);
    
    for (int portalIndex=0; portalIndex<portalCount; portalIndex++) {
        portalRender portalInfo = portalQueue[portalIndex];
        if (portalInfo.sector_link == originSector) {
            continue;
        }
        if (traversedPortals[portalInfo.uid] == 1) {
            continue;
        } else {
            traversedPortals[portalInfo.uid] = 1;
        }
        sector *portalSector = level->sectors[portalInfo.sector_link];
        drawSector(pixels, level, portalSector, character, pSn, pCs, portalInfo.portalBounds);
    }
    free(portalQueue);
}

void startDrawSector(Uint16 *pixels, Level *level, sector *Sector, player character, float pSn, float pCs, portalCull portalBounds, int origin) {
    originSector = origin;
    for (int x = 0; x<MAX_WALLS; x++) {
        traversedPortals[x] = 0;
    }
    drawSector(pixels, level, Sector, character, pSn, pCs, portalBounds);
}

void R_INIT() {
    ceilingLut = (int*)malloc(sizeof(int)*SW1);
    floorLut = (int*)malloc(sizeof(int)*SW1);
}
#endif