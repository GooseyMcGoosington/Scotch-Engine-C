#ifndef S_LEVEL
#define S_LEVEL

#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

Level *level = NULL;

int amnt_entities = 0;
entity entities[(MAX_ENTITIES)];

sector_listeners *ScriptedSectors = NULL;

static inline int S_CHAR_IN_CONVEX_BOUNDS(player character) {
    int i = 0;
    for (int s=0; s<(level->count); s++) {
        sector *Sector = level->sectors[s];
        int inside = 1;
        wall *walls = Sector->walls;
        size_t wallCount = Sector->count;
        for (int w=0; w<wallCount; w++) {
            wall Wall = walls[w];
            float cross = (Wall.x1-Wall.x0) * (character.y-Wall.y0) - (Wall.y1-Wall.y0) * (character.x-Wall.x0);
            if (cross > 0) {
                inside = 0;
            }
        }
        if (inside == 1) {
            return i;
        }
        i ++;
    }
    return -1;
}

static inline int S_GET_ENTITY_SECTOR(entity Entity) {
    int i = 0;
    for (int s=0; s<(level->count); s++) {
        sector *Sector = level->sectors[s];
        int inside = 1;
        wall *walls = Sector->walls;
        size_t wallCount = Sector->count;
        for (int w=0; w<wallCount; w++) {
            wall Wall = walls[w];
            float cross = (Wall.x1-Wall.x0) * (Entity.y-Wall.y0) - (Wall.y1-Wall.y0) * (Entity.x-Wall.x0);
            if (cross > 0) {
                inside = 0;
            }
        }
        if (inside == 1) {
            return i;
        }
        i ++;
    }
    return -1;
}

void S_SECTOR_CENTROID_APPROX(sector *s, float *out_x, float *out_y) {
    float sumX = 0.0f;
    float sumY = 0.0f;
    int count = 0;

    for (int i = 0; i < s->count; i++) {
        sumX += s->walls[i].x0;
        sumY += s->walls[i].y0;
        count++;
    }

    *out_x = sumX / count;
    *out_y = sumY / count;
}

extern int amnt_entities;
extern entity entities[];

static void S_LOAD_LEVEL() {
    // Later, we should load from a file. For now, we will set the level below with some neat stuff.
    level = malloc(sizeof(Level) + 3 * sizeof(sector *));
    level->count = 3;

    level->sectors[0] = malloc(sizeof(sector) + 6 * sizeof(wall));
    sector *sect = level->sectors[0];
    sect->count = 6;
    sect->height = 10.0;
    sect->elevation = 0.0;
    sect->cIndex = 1;
    sect->fIndex = 1;
    sect->walls[0] = (wall){20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0};
    sect->walls[1] = (wall){80, 60, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0};
    sect->walls[2] = (wall){0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 3, 0};
    sect->walls[3] = (wall){80, 80, 80, 0, 0, 0, 0, 0, 0, 0, 4, 0};
    sect->walls[4] = (wall){0, 80, 80, 80, 0, 0, 0, 0, 0, 0, 5, 0};
    sect->walls[5] = (wall){60, 20, 0, 0, 1, 2, 2, 1, 0, 0, 6, 0}; // Portal into Sector 1
    sect->id = 0;
    sect->light = 0;    
    
    entities[0] = (entity){1, 10, 10, 0, 0, 10, .5, 2, 1, 0};
    entities[1] = (entity){1, 10, 10, 0, 0, 10, .5, 6, 0, 0};
    amnt_entities = 2;

    //entities[1] = (entity){1, 10, 15, 0, 0, 10, 3, 6, 0, 0};
    //entities[2] = (entity){1, 10, 20, 0, 0, 10, 3, 8, 0, 0};


    level->sectors[1] = malloc(sizeof(sector) + 4 * sizeof(wall));
    sector *sect1 = level->sectors[1];
    //sect1->amnt_entities = 1;
    //sect1->entities = malloc(sect1->amnt_entities*sizeof(entity));
    //sect1->entities[0] = (entity){1, 15, -8, 0, 0, 10, 0, 2, 1, 0};
    sect1->count = 4;
    sect1->height = 18.0;
    sect1->elevation = 2.0;
    sect1->cIndex = 0;
    sect1->fIndex = 0;
    sect1->light = 2;
    sect1->id = 1;
    sect1->walls[0] = (wall){80, 20, -30, -30, 1, 0, 0, 2, 0, 1, 7, 1};
    sect1->walls[1] = (wall){20, 20, -30, 0, 0, 0, 0, 0, 0, 1, 8, 1};
    sect1->walls[2] = (wall){60, 80, 0, -30, 0, 0, 0, 0, 0, 1, 9, 1};
    sect1->walls[3] = (wall){20, 60, 0, 0, 1, 0, 0, 0, 0, 0, 10, 1}; // Portal into Sector 0
    
    level->sectors[2] = malloc(sizeof(sector) + 5 * sizeof(wall));
    sector *sect2 = level->sectors[2];
    sect2->count = 5;
    sect2->height = 18.0;
    sect2->elevation = 2.0;
    sect2->cIndex = 1;
    sect2->fIndex = 1;
    sect2->light = -5;
    sect2->id = 2;
    //sect2->entities = NULL;
    //sect2->amnt_entities = 0;
    sect2->walls[0] = (wall){100, 20, -70, -70, 0, 0, 0, 0, 0, 0, 11, 2};
    sect2->walls[1] = (wall){20, 20, -70, -30, 0, 0, 0, 0, 0, 0, 12, 2};
    sect2->walls[2] = (wall){100, 100, -30, -70, 0, 0, 0, 0, 0, 0, 13, 2};
    sect2->walls[3] = (wall){80, 100, -30, -30, 0, 0, 0, 0, 0, 0, 14, 2};
    sect2->walls[4] = (wall){20, 80, -30, -30, 1, 0, 0, 1, 0, 1, 14, 2};

    for (int s=0; s<level->count; s++) {
        sector *Sector = level->sectors[s];
        S_SECTOR_CENTROID_APPROX(Sector, &Sector->cx, &Sector->cy);
    }
    ScriptedSectors = malloc(sizeof(sector_listeners));
    ScriptedSectors->size = 1;

    int8_t otherSize = 2;
    wall **otherWalls = malloc(sizeof(wall*)*otherSize);
    otherWalls[0] = &sect->walls[5];
    otherWalls[1] = &sect2->walls[4];

    int8_t linkSize = 1;
    wall **linkWalls = malloc(sizeof(wall*)*linkSize);
    linkWalls[0] = &sect1->walls[3];

    listener Listener00 = {0, sect1, LIGHT_FLICKER, 0, 0, 0, 0, linkSize, linkWalls, otherSize, otherWalls};
    Listener00.arg1 = sect1->height;

    ScriptedSectors->listeners = malloc(sizeof(listener)*ScriptedSectors->size);
    ScriptedSectors->listeners[0] = Listener00;

    size_t lt = level->count;
    for (int x = 0; x<lt; x++) {
        sector *Sector = level->sectors[x];
        size_t wC = Sector->count;

        for (int y = 0; y<wC; y++) {
            float x0 = (float)Sector->walls[y].x0;
            float x1 = (float)Sector->walls[y].x1;
            float y0 = (float)Sector->walls[y].y0;
            float y1 = (float)Sector->walls[y].y1;
            float dx = x1-x0;
            float dy = y1-y0;
            float dst = sqrtf(dx*dx + dy*dy);
            Sector->walls[y].length = dst;
        }
    }
}

#endif