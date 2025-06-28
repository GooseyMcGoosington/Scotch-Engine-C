#ifndef S_LEVEL
#define S_LEVEL
#include "s_classes.h"


Level *level;

static void S_LEVEL_LOAD() { 
    level = malloc(sizeof(Level) + 1 * sizeof(sector *));
    level->count = 1;

    level->sectors[0] = malloc(sizeof(sector) + 6 * sizeof(wall));
    sector *sect = level->sectors[0];
    sect->count = 6;
    sect->height = 10.0;
    sect->elevation = 0.0;

    // L-shape layout:
    //   (0,0) ----------- (50,0)
    //     |                 |
    //     |                 |
    //   (0,100)--(25,100)--(25,50)

    sect->walls[0] = (wall){0, 0, 0, 100};     // Left vertical
    sect->walls[1] = (wall){0, 25, 100, 100};  // Top left horizontal
    sect->walls[2] = (wall){25, 25, 100, 50};  // Inner vertical drop
    sect->walls[3] = (wall){25, 50, 50, 50};   // Top right horizontal
    sect->walls[4] = (wall){50, 50, 50, 0};    // Right vertical
    sect->walls[5] = (wall){50, 0, 0, 0};      // Bottom horizontal
}

#endif