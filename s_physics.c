#define S_PHYSICS

#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

// Ported from Scotch Engine V1 from Python. Everything is done so that it is compatible with the modern code.

static inline void S_RESOLVE_COLLISION(player *Character, float lenSq, float dx, float dy) {
    float dist = fast_sqrtf(lenSq);
    if (dist == 0.0f) {
        dx = 0.0f; dy = 1.0f; dist = 1.0f;
    } else {
        dx /= dist; dy /= dist;
    }

    float pushDist = Character->radius - dist;
    Character->x += dx * pushDist;
    Character->y += dy * pushDist;
}

static inline void S_SEGMENT_COLLISION_CHR(sector *Sector, player *Character) {
    float hx, hy, dx, dy, lenSq;
    float chrRad = Character->radius * Character->radius;
    float E = Sector->elevation;
    float H = Sector->height;
    float EH = E + H;

    float chrBase = Character->z;
    float chrTop  = chrBase + Character->h;

    for (int w = 0; w < Sector->count; w++) {
        wall Wall = Sector->walls[w];
        segment l1 = {Wall.x0, Wall.y0, Wall.x1, Wall.y1};

        dx = l1.x1 - l1.x0;
        dy = l1.y1 - l1.y0;
        float segLenSq = dx*dx + dy*dy;

        if (segLenSq == 0.0f) {
            hx = Wall.x0; hy = Wall.y0;
        } else {
            float t = ((Character->x - Wall.x0)*dx + (Character->y - Wall.y0)*dy) / segLenSq;
            t = fminf(fmaxf(t, 0.0f), 1.0f);
            hx = Wall.x0 + t * dx;
            hy = Wall.y0 + t * dy;
        }

        dx = Character->x - hx;
        dy = Character->y - hy;
        lenSq = dx*dx + dy*dy;

        if (lenSq <= chrRad) {
            if (Wall.is_portal) {
                float wallBase = E + Wall.portal_bottom;
                float wallTop  = EH - Wall.portal_top;
                if ((chrBase < wallBase) || (chrTop > wallTop)) {
                    float wHeight = wallTop - wallBase;
                    float fDiff = E - chrBase;
                    if (!((fDiff < Character->h/2.0f) && (wHeight > Character->h))) {
                        S_RESOLVE_COLLISION(Character, lenSq, dx, dy);
                    }
                }
            } else {
                S_RESOLVE_COLLISION(Character, lenSq, dx, dy);
            }
        }
    }
}

static inline void S_LINE_INTERSECT_TEST(segment l1, segment l2, int8_t *intersect, float *ix, float *iy) {
    float dx0 = l1.x1 - l1.x0;
    float dy0 = l1.y1 - l1.y0;
    float dx1 = l2.x1 - l2.x0;
    float dy1 = l2.y1 - l2.y0;
    float det = dx0*dy1 - dy0*dx1;
    if (fabsf(det) < 1e-4f) {
        *intersect = 0; return;
    }

    float dx2 = l2.x0 - l1.x0;
    float dy2 = l2.y0 - l1.y0;
    float t0 = (dx2*dy1 - dy2*dx1) / det;
    float t1 = (dx2*dy0 - dy2*dx0) / det;

    const float EPS = 1e-3f;
    if (t0 > EPS && t0 <= 1.0f && t1 >= 0.0f && t1 <= 1.0f) {
        *intersect = 1;
        *ix = l1.x0 + t0 * dx0;
        *iy = l1.y0 + t0 * dy0;
    } else {
        *intersect = 0;
    }
}

static float rdx;
static float rdy;

static inline void S_TRAVERSE_SECTOR(Level *level, sector *Sector, segment l1, wall **intersectWall, int8_t *intersect, float *ix, float *iy, uint8_t recursionDepth, int range) {
    for (int w = 0; w < Sector->count; w++) {
        wall *Wall = &Sector->walls[w];
        segment wallSeg = {(float)Wall->x0, (float)Wall->y0, (float)Wall->x1, (float)Wall->y1};

        S_LINE_INTERSECT_TEST(l1, wallSeg, intersect, ix, iy);
        if (*intersect) {
            if (Wall->is_portal) {
                if (++recursionDepth > 16) {
                    *intersectWall = NULL;
                    return;
                }
                sector *next = level->sectors[Wall->portal_link];
                segment nl1 = {*ix, *iy, *ix + rdx * range, *iy + rdy * range};
                S_TRAVERSE_SECTOR(level, next, nl1, intersectWall, intersect, ix, iy, recursionDepth, range);
                if (*intersectWall) return;
            } else {
                *intersectWall = Wall;
                return;
            }
        }
    }
    *intersectWall = NULL;
}

void S_RAYCAST(Level *level, wall **retWall, float *ix, float *iy, sector *Sector, float x, float y, float z, float dx, float dy, int range) {
    rdx = dx;
    rdy = dy;
    segment l1 = {x, y, x + dx * range, y + dy * range};
    int8_t intersect = 0;
    wall *hitWall = NULL;
    float rix = 0, riy = 0;
    S_TRAVERSE_SECTOR(level, Sector, l1, &hitWall, &intersect, &rix, &riy, 0, range);
    *ix = rix;
    *iy = riy;
    *retWall = intersect ? hitWall : NULL;
}
