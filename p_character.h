#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

extern Level *level;
extern sector *playerSector;
extern float sn[361];
extern float cs[361];
extern ai_entity *AIEntities[(MAX_ENTITIES)];
extern int amnt_entities;

extern SDL_Window *window;