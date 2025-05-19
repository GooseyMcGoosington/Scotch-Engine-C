#ifndef raster
#define raster

#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"
#include "tloader.h"
#include "ini_parser.c"
#include "r_dispatcher.c"

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
            dispatch_wall(sx0, sx1, sy0, sy1, sy2, sy3, pixels, portalBounds, ceilingLut, floorLut, 0, wall_texture.pixels, t0, t1, ty0, ty1, cWall.length, Sector->height, wall_texture.scale);
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
            
            dispatch_wall(sx0, sx1, sy0, sy1, sy2, sy3, pixels, portalBounds, ceilingLut, floorLut, 2, wall_texture.pixels, t0, t1, ty0, ty1, cWall.length, cWall.portal_top, wall_texture.scale);
            dispatch_wall(sx0, sx1, sy4, sy5, sy6, sy7, pixels, portalBounds, ceilingLut, floorLut, 1, wall_texture.pixels, t0, t1, ty0, ty1, cWall.length, cWall.portal_bottom, wall_texture.scale);
            
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
    //draw_flat(pixels, ceilingLut, 1, portalBounds, ez0, character.fov, character.yaw, character.focalLength, character.x, character.y, ceiling_texture.pixels);
    //draw_flat(pixels, floorLut, 2, portalBounds, ez1, character.fov, character.yaw, character.focalLength, character.x, character.y, floor_texture.pixels);
    dispatch_flat(pixels, ceilingLut, 1, portalBounds, ez0, character.fov, character.yaw, character.focalLength, character.x, character.y, ceiling_texture.pixels, ceiling_texture.scale);
    dispatch_flat(pixels, floorLut, 2, portalBounds, ez1, character.fov, character.yaw, character.focalLength, character.x, character.y, floor_texture.pixels, floor_texture.scale);

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