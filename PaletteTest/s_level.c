#ifndef S_LEVEL
#define S_LEVEL
#include "s_classes.h"


Level *level;

static void S_LEVEL_LOAD() { 
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
    sect->amnt_entities = 3;
    sect->light = 0;
    sect->entities = malloc(sect->amnt_entities*sizeof(entity));
    sect->entities[0] = (entity){1, 10, 10, 0, 0, 10, 3, 2, 1, 0};
    sect->entities[1] = (entity){1, 10, 15, 0, 0, 10, 3, 6, 0, 0};
    sect->entities[2] = (entity){1, 10, 20, 0, 0, 10, 3, 8, 0, 0};


    level->sectors[1] = malloc(sizeof(sector) + 4 * sizeof(wall));
    sector *sect1 = level->sectors[1];
    sect1->amnt_entities = 1;
    sect1->entities = malloc(sect1->amnt_entities*sizeof(entity));
    sect1->entities[0] = (entity){1, 15, -8, 0, 0, 10, 0, 2, 1, 0};
    sect1->count = 4;
    sect1->height = 18.0;
    sect1->elevation = 2.0;
    sect1->cIndex = 0;
    sect1->fIndex = 0;
    sect1->light = 2;
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
    sect2->entities = NULL;
    sect2->amnt_entities = 0;
    sect2->walls[0] = (wall){100, 20, -70, -70, 0, 0, 0, 0, 0, 0, 11, 2};
    sect2->walls[1] = (wall){20, 20, -70, -30, 0, 0, 0, 0, 0, 0, 12, 2};
    sect2->walls[2] = (wall){100, 100, -30, -70, 0, 0, 0, 0, 0, 0, 13, 2};
    sect2->walls[3] = (wall){80, 100, -30, -30, 0, 0, 0, 0, 0, 0, 14, 2};
    sect2->walls[4] = (wall){20, 80, -30, -30, 1, 0, 0, 1, 0, 1, 14, 2};
}
#endif