#define S_PHYSICS

#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"
#include "raster.h"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

// Ported from Scotch Engine V1 from Python. Everything is done so that it is compatible with the modern code.

static inline void S_RESOLVE_COLLISION(player *Character, _Float16 lenSq, _Float16 dx, _Float16 dy) {
    _Float16 dist = fast_sqrtf(lenSq);
    if (dist == 0) {
        // Character is exactly on the wall point — push in arbitrary direction (e.g., up)
        dx = 0;
        dy = 1;
        dist = 1; // avoid div by 0
    } else {
        dx /= dist;
        dy /= dist;
    }

    // Push the character to radius distance away from the wall
    _Float16 pushDist = Character->radius - dist;
    Character->x += dx * pushDist;
    Character->y += dy * pushDist;
}

static inline void S_SEGMENT_COLLISION_CHR(sector *Sector, player *Character) {
    _Float16 hx;
    _Float16 hy;
    _Float16 dx;
    _Float16 dy;
    _Float16 lenSq;

    _Float16 chrRad = Character->radius*Character->radius;

    _Float16 E = Sector->elevation;
    _Float16 H = Sector->height;
    _Float16 EH = E+H;

    _Float16 chrBase = Character->z;
    _Float16 chrTop = chrBase+Character->h;

    for (int w=0; w<Sector->count; w++) {
        wall Wall = Sector->walls[w];
        segment l1 = {Wall.x0, Wall.y0, Wall.x1, Wall.y1};
        dx = l1.x1-l1.x0;
        dy = l1.y1-l1.y0;
        _Float16 pow_dxdy = dx*dx+dy*dy;

        if (pow_dxdy == 0) { // Degenerate line segment; start == end
            hx = Wall.x0;
            hy = Wall.y0;
        } else {
            _Float16 t = ((Character->x-Wall.x0) * dx + (Character->y-Wall.y0) * dy) / pow_dxdy;
            t = CLAMP_F16(t, 0.0f, 1.0f);
            hx = Wall.x0+t*dx;
            hy = Wall.y0+t*dy;
        }
        dx = (Character->x - hx);
        dy = (Character->y - hy);
        lenSq = (dx*dx)+(dy*dy);
        if (lenSq <= chrRad) {
            if (Wall.is_portal) {
                _Float16 wallBase = E+Wall.portal_bottom;
                _Float16 wallTop = EH-Wall.portal_top;
                // Test if the character fits through the window, if it doesn't, resolve the collision.
                if ((chrBase < wallBase) || (chrTop > wallTop)) {
                    // We could simply just let the character through. Is the window large enough, and is the floor low enough?
                    _Float16 wHeight = wallTop-wallBase;
                    _Float16 fDiff = E-chrBase; // 0-4 = -4. We would check if this is below h/2.
                    if (!((fDiff < Character->h/2) && (wHeight > Character->h))) {
                        // Resolve collision
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
    // Half accuracy; it doesn't matter much and raycasting can be quite taxing.
    _Float16 dx0 = l1.x1-l1.x0;
    _Float16 dy0 = l1.y1-l1.y0;
    _Float16 dx1 = l2.x1-l2.x0;
    _Float16 dy1 = l2.y1-l2.y0;
    _Float16 determinant = dx0*dy1-dy0*dx1;
    if (fabs(determinant) < 1e-4) {
        *intersect = 0;
        *ix = 0;
        *iy = 0;
        return;
    }
    _Float16 dx2 = l2.x0-l1.x0;
    _Float16 dy2 = l2.y0-l1.y0;
    _Float16 t0 = (dx2*dy1-dy2*dx1) / determinant;
    _Float16 t1 = (dx2*dy0-dy2*dx0) / determinant;

    if ((0 <= t0) && (t0 <= 1) && (0 <= t1) && (t1 <= 1)) {
        *intersect = 1;
        *ix = l1.x0 + t0*dx0;
        *iy = l1.y0 + t0*dy0;
        return;
    }
    *intersect = 0;
    *ix = 0;
    *iy = 0;
    return;
}

float rdx;
float rdy;

static inline void S_TRAVERSE_SECTOR(Level *level, sector *Sector, segment l1, wall **intersectWall, int8_t *intersect, float *ix, float *iy, uint8_t recursionDepth) {
    for (int w = 0; w < Sector->count; w++) {
        wall *Wall = &Sector->walls[w];
        segment l2 = {(float)Wall->x0, (float)Wall->y0, (float)Wall->x1, (float)Wall->y1};

        S_LINE_INTERSECT_TEST(l1, l2, intersect, ix, iy);
        if (*intersect) {
            if (Wall->is_portal) {
                // Traverse further
                sector *nextSector = level->sectors[Wall->portal_link];
                recursionDepth ++;
                if (recursionDepth > 16) {
                    // Quit to prevent a Stack Overflow
                    *intersectWall = NULL;
                    return;
                }
                // We can construct a new line.
                segment nl1 = {*ix, *iy, *ix+rdx*32, *iy+rdy*32}; 
                S_TRAVERSE_SECTOR(level, nextSector, nl1, intersectWall, intersect, ix, iy, recursionDepth);
                if (*intersectWall != NULL) {
                    // Wall found
                    return;
                }
            } else {
                // Early quit
                *intersectWall = Wall;
                return;
            }
        }
    }
    *intersectWall = NULL;
}

static void S_RAYCAST(Level *level, wall **retWall, sector *Sector, float x, float y, float z, float dx, float dy) {
    float l1x = x;
    float l1y = y;
    // We will move in the direction of our ray, completing the line
    float l1rx = l1x+dx*32;
    float l1ry = l1y+dy*32;
    // Save variables for later
    rdx = dx;
    rdy = dy;

    segment l1 = {l1x, l1y, l1rx, l1ry};
    int8_t intersect;
    float ix;
    float iy;
    wall *Wall = NULL;

    S_TRAVERSE_SECTOR(level, Sector, l1, &Wall, &intersect, &ix, &iy, 0);

    if (intersect) {
        *retWall = Wall;
        return;
    }
    return;
}