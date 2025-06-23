#ifndef S_LISTEN
#define S_LISTEN

#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

static inline void S_SET_HEIGHT(Level *level, listener *currentListener) {
    sector *Sector = currentListener->Sector;
    float e0 = Sector->elevation;
    float h0 = Sector->height;

    for (int o = 0; o < currentListener->otherSize; o++) {
        wall *Wall = currentListener->otherWalls[o]; // This is beside our scripted sector.
        if (Wall->is_portal) {
            float e1 = level->sectors[Wall->oid]->elevation;
            float h1 = level->sectors[Wall->oid]->height;

            Wall->portal_top = -min((e0+h0)-(e1+h1), 0);
            for (int w = 0; w < currentListener->linkSize; w++) {
                wall *linkWall = currentListener->linkWalls[w];
                if (linkWall->is_portal) {
                    if ((Wall->portal_link == linkWall->oid) && (Wall->oid == linkWall->portal_link)) {
                        linkWall->portal_top = max((e0+h0)-(e1+h1), 0);
                    }
                }
            }
        }
    }
}

static inline void S_HANDLE_EVENT(Level *level, listener *currentListener) {
    int8_t *state = &currentListener->state;
    int8_t type = currentListener->type;
    int8_t *delay = &currentListener->delay;

    float arg1 = currentListener->arg1;
    float arg2 = currentListener->arg2;

    sector *Sector = currentListener->Sector;

    switch (type) {
        case CRUSHER:
        {
            if (*delay == 0) {
                if (*state == 0) {
                    // arg1 is original height
                    Sector->height += 0.2;
                    if (Sector->height > arg1) {
                        Sector->height = arg1;
                        *delay = 20; // Wait for 20 ticks
                        *state = 1;
                    }
                } // GO up
                if (*state == 1) {
                    // arg1 is the original height
                    Sector->height -= 0.2;
                    if (Sector->height < 0) {
                        Sector->height = 0;
                        *delay = 20; // Wait for 20 ticks
                        *state = 0;
                    }
                }
            } else {
                *delay -= 1;
            }
            break;
        }
        case LIGHT_FLICKER:
        {
            if (((*delay > 10) || (*delay == 4)) && (*delay != 22)) {
                Sector->light = arg1;
                *delay -= 1;
            } else {
                Sector->light = -3;
                if (*delay <= 0) {
                    *delay = 35;
                }
                *delay -= 1;
            }
            break;
        }
    }
}

void S_START_LISTEN(Level *level, sector_listeners *Listeners) {
    for (int index = 0; index < Listeners->size; index++) {
        listener *currentListener = Listeners->listeners;
        S_HANDLE_EVENT(level, currentListener);
        S_SET_HEIGHT(level, currentListener);
    }
}

#endif