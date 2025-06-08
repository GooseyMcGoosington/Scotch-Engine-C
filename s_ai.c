#ifndef S_AI
#define S_AI

#include "classes.h"
#include "helper.h"
#include "SDL2/SDL.h"

#include "s_level.c"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

portal_graph portal_tree;
uint8_t portal_ref_array[256]; // What portal links to what in the portal tree?
uint16_t portal_ref = 0; // What are we tracking?
int8_t intOriginSector = 0;
uint8_t traversedPortals[MAX_WALLS];

uint8_t AIEntities_amnt = 0;
ai_entity **AIEntities;

/*
example;
ref 0 => uid 13 (portal)
ref 1 => uid 14 (portal)
ref 2 => uid 17 (portal)
*/

/*
    'S_GENERATE_GRAPH' generates portal graphs. It is a way of figuring out if a sector is reachable from another.
    An AI will want to reach a target in another sector, so first, it will see if it can reach that Sector through the Portal Graph. If it can, it will walk towards a portal that goes there.
    The code here will control entities in a Sector.
*/

static inline void setReachable(uint8_t *bitfield, uint8_t sector) {
    bitfield[sector / 8] |= (1 << (sector % 8));
}
static inline int isReachable(const uint8_t *bitfield, uint8_t sector) {
    return (bitfield[sector / 8] & (1 << (sector % 8))) != 0;
}
static inline void clearReachable(uint8_t *bitfield, uint8_t sector) {
    bitfield[sector / 8] &= ~(1 << (sector % 8));
}

static inline int S_FIND_ENTITY_INDEX(entity *entities, int count, entity *target) {
    for (int i = 0; i < count; i++) {
        if (&entities[i] == target) {
            return i;
        }
    }
    return -1;
}

static inline entity* S_PROTECTED_ENTITY_REALLOC(Level *level, sector *originSector, sector *newSector, int entityIndex) {
    if (entityIndex < 0 || entityIndex >= originSector->amnt_entities) {
        fprintf(stderr, "ERROR: Invalid entity index %d for originSector with %d entities\n", entityIndex, originSector->amnt_entities);
        return NULL;
    }
    entity entityCopy = originSector->entities[entityIndex];
    for (int i = entityIndex; i < originSector->amnt_entities - 1; i++) {
        originSector->entities[i] = originSector->entities[i + 1];
    }
    originSector->amnt_entities--;
    if (originSector->amnt_entities > 0) {
        entity *realloced = realloc(originSector->entities, originSector->amnt_entities * sizeof(entity));
        if (!realloced) {
            fprintf(stderr, "ERROR: realloc failed for originSector!\n");
            return NULL;
        }
        originSector->entities = realloced;
    } else {
        free(originSector->entities);
        originSector->entities = NULL;
    }
    entity *newArray;
    if (newSector->amnt_entities == 0) {
        newArray = malloc(sizeof(entity));
    } else {
        newArray = realloc(newSector->entities, (newSector->amnt_entities + 1) * sizeof(entity));
    }
    if (!newArray) {
        fprintf(stderr, "ERROR: malloc/realloc failed for newSector!\n");
        return NULL;
    }
    newArray[newSector->amnt_entities] = entityCopy;
    newSector->entities = newArray;
    newSector->amnt_entities++;

    return &newSector->entities[newSector->amnt_entities - 1];
}


static inline void S_TRAVERSE(Level *level, sector *OriginSector, sector *Sector) {
    for (int p=0; p<Sector->count; p++) {
        wall *Portal = &Sector->walls[p];
        if (Portal->is_portal && (Portal->portal_link != intOriginSector)) {
            if (traversedPortals[Portal->uid] != 1) {
                traversedPortals[Portal->uid] = 1;

                // DO NOT increment portal_ref here!
                // We're still filling the same portal's bitfield
                setReachable(portal_tree.portals[portal_ref].canReach, Portal->portal_link);
                S_TRAVERSE(level, Sector, level->sectors[Portal->portal_link]);
            }
        }
    }
}

static inline int S_APPLY_IMPULSE(ai_entity *Entity, _Float16 x1, _Float16 y1) {
    _Float16 dx = x1 - Entity->ref_entity->x;
    _Float16 dy = y1 - Entity->ref_entity->y;
    _Float16 dist = fast_sqrtf(dx * dx + dy * dy);
    _Float16 speed = 0.5f;
    if (dist <= speed) {
        Entity->ref_entity->x = x1;
        Entity->ref_entity->y = y1;
        Entity->ref_entity->vx = 0;
        Entity->ref_entity->vy = 0;
        return 1;
    }
    dx /= dist;
    dy /= dist;
    Entity->ref_entity->vx = dx * speed;
    Entity->ref_entity->vy = dy * speed;

    return 0;
}


static void S_WALK_PORTAL(wall *Portal, ai_entity *Entity) {
    if (!Entity->moving) {
        // Calculate the centre of the portal
        _Float16 cx = (Portal->x1+Portal->x0)/2;
        _Float16 cy = (Portal->y1+Portal->y0)/2;
        Entity->gx = cx;
        Entity->gy = cy;
    }
}

static void S_WALK_CENTROID(sector *Sector, ai_entity *Entity) {
    if (!Entity->moving) {
        // Calculate the centre of the portal
        Entity->gx = Sector->cx;
        Entity->gy = Sector->cy;
    }
}

static inline void S_AI_WALK(Level *level, ai_entity *Entity, sector *originSector, int8_t gid) {
    Entity->ref_entity->x += Entity->ref_entity->vx;
    Entity->ref_entity->y += Entity->ref_entity->vy;

    if (Entity->sector_id == gid) {
        if (!Entity->moving) {
            S_WALK_CENTROID(originSector, Entity);
            S_APPLY_IMPULSE(Entity, Entity->gx, Entity->gy);
            Entity->moving = 1;
        } else {
            int reachedGoal = S_APPLY_IMPULSE(Entity, Entity->gx, Entity->gy);
            if (reachedGoal) {
                Entity->moving = 0;
                Entity->ref_entity->vx = 0;
                Entity->ref_entity->vy = 0;
                Entity->goal_reached = 1;
            }
        }
        return;
    }

    // If not at goal sector, move towards a portal leading to the goal sector
    if (!Entity->moving) {
        for (int p = 0; p < originSector->count; p++) {
            wall *Portal = &originSector->walls[p];
            if (Portal->is_portal) {
                int8_t ref_id = portal_ref_array[Portal->uid];
                if (isReachable(portal_tree.portals[ref_id].canReach, gid)) {
                    // Set goal to center of portal and set velocity once
                    S_WALK_PORTAL(Portal, Entity);
                    S_APPLY_IMPULSE(Entity, Entity->gx, Entity->gy);
                    Entity->moving = 1;
                    break; // stop after setting one portal goal
                }
            }
        }
    } else {
        int reachedGoal = S_APPLY_IMPULSE(Entity, Entity->gx, Entity->gy);
        if (reachedGoal) {
            // Move entity to next sector
            int newIndex = -1;
            for (int p = 0; p < originSector->count; p++) {
                wall *Portal = &originSector->walls[p];
                if (Portal->is_portal) {
                    _Float16 cx = (Portal->x1 + Portal->x0) / 2;
                    _Float16 cy = (Portal->y1 + Portal->y0) / 2;
                    if (cx == Entity->gx && cy == Entity->gy) {
                        newIndex = Portal->portal_link;
                        break;
                    }
                }
            }

            if (newIndex >= 0) {
                sector *newSector = level->sectors[newIndex];
                int entityIndex = S_FIND_ENTITY_INDEX(originSector->entities, originSector->amnt_entities, Entity->ref_entity);
                if (entityIndex >= 0) {
                    Entity->ref_entity = S_PROTECTED_ENTITY_REALLOC(level, originSector, newSector, entityIndex);
                    Entity->sector_id = newIndex;
                }
            }
            Entity->moving = 0;
            Entity->ref_entity->vx = 0;
            Entity->ref_entity->vy = 0;
        }
    }
}


static inline void S_INIT_AI(Level *level) {
    AIEntities_amnt = 1;
    AIEntities = malloc(sizeof(ai_entity*)*AIEntities_amnt);

    AIEntities[0] = malloc(sizeof(ai_entity));
    AIEntities[0]->ref_entity = &level->sectors[0]->entities[0];
    AIEntities[0]->behaviour = 0;
    AIEntities[0]->moving = 0;
    AIEntities[0]->sector_id = 0;
}

static inline void S_HANDLE_AI(Level *level) {
    for (int e=0; e<AIEntities_amnt; e++) {
        ai_entity *Entity = AIEntities[e];
        S_AI_WALK(level, Entity, level->sectors[Entity->sector_id], 2); // For testing, make them walk over to sector 2
    }
}

static inline void S_GENERATE_GRAPH(Level *level) { 
    for (int hs=0; hs<level->count; hs++) {
        sector *OriginSector = level->sectors[hs];
        intOriginSector = hs;
        // We'll traverse the sectors portals and create a graph per portal.
        for (int p=0; p<OriginSector->count; p++) {
            wall *Portal = &OriginSector->walls[p];
            if (Portal->is_portal) {
                // Add them to the portal ref
                portal_ref_array[Portal->uid] = portal_ref;
                setReachable(portal_tree.portals[portal_ref].canReach, Portal->portal_link); // example; portal uid 15 given by ref 0 links up with sector 2
                traversedPortals[Portal->uid] = 1;
                //printf("ref %d  link %d origin %d\n", isReachable(portal_tree.portals[portal_ref].canReach, Portal->portal_link), Portal->portal_link, hs);

                S_TRAVERSE(level, OriginSector, level->sectors[Portal->portal_link]);
                portal_ref ++;
            }
        }
    }
}
#endif