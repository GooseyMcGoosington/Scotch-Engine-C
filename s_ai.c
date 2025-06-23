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

/*
example;
ref 0 => uid 13 (portal)
ref 1 => uid 14 (portal)
ref 2 => uid 17 (portal)
*/

/*
    'S_GENERATE_GRAPH' generates portal graphs. It is a way of figuring out if a sector is reachable from another.
    An AI will want to reach a target in another sector, so first, it will see if it can reach that Sector through the Portal Graph. If it can, it will walk towards a portal that goes there.
    The code here will control entities in a Sector. It will also "garbage collect" entities that are meant to be removed after a timer reaches 0.
*/

portal_graph portal_tree;
uint8_t portal_ref_array[MAX_WALLS]; // What portal links to what in the portal tree?
uint16_t portal_ref = 0; // What are we tracking?
int8_t intOriginSector = 0;
uint8_t traversedPortals[MAX_WALLS];

uint8_t AIEntities_amnt = 0;
ai_entity *AIEntities[(MAX_ENTITIES)];

uint8_t GCEntities_amnt = 0;
gc_entity *GCEntities[(MAX_ENTITIES)]; // Fixed amount.

extern int amnt_entities; // Global count of all entities
extern entity entities[];  // Global entity array

static inline void S_SET_REACHABLE(uint8_t *bitfield, uint8_t sector) {
    bitfield[sector / 8] |= (1 << (sector % 8));
}
static inline int S_IS_REACHABLE(const uint8_t *bitfield, uint8_t sector) {
    return (bitfield[sector / 8] & (1 << (sector % 8))) != 0;
}
static inline void S_CLEAR_REACHABLE(uint8_t *bitfield, uint8_t sector) {
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

static inline entity *S_MOVE_ENTITY_TO_SECTOR(Level *level, int entityIndex, int newSectorId) {
    if (entityIndex < 0 || entityIndex >= amnt_entities) return NULL;

    entities[entityIndex].oid = newSectorId;
    return &entities[entityIndex];
}

static inline void S_TRAVERSE(Level *level, sector *OriginSector, sector *Sector) {
    for (int p=0; p<Sector->count; p++) {
        wall *Portal = &Sector->walls[p];
        if (Portal->is_portal && (Portal->portal_link != intOriginSector)) {
            if (traversedPortals[Portal->uid] != 1) {
                traversedPortals[Portal->uid] = 1;

                S_SET_REACHABLE(portal_tree.portals[portal_ref].canReach, Portal->portal_link);
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

static void S_ADD_DEBRIS(ai_entity *ref_ai, uint16_t timer) {
    gc_entity *ent = malloc(sizeof(gc_entity));
    ent->ref_ai = ref_ai;
    ent->timer = timer;

    GCEntities[GCEntities_amnt] = ent;
    GCEntities_amnt ++;
}

static entity *S_ADD_ENTITY(sector *Sector, float x, float y, uint8_t tid, uint8_t animated) {
    if (amnt_entities >= MAX_ENTITIES) return NULL;
    entity *ent = &entities[amnt_entities];
    amnt_entities ++;
    ent->x = x;
    ent->y = y;
    ent->vx = 0;
    ent->vy = 0;
    ent->tid = tid;
    ent->animated = animated;
    ent->oid = Sector->id;
    return ent;
}

ai_entity *S_ADD_AI(entity *ref_ent, uint8_t behaviour, uint8_t index) {
    ai_entity *ent = malloc(sizeof(*ent));
    ent->ref_entity = ref_ent;
    ent->entity_index = index;
    ent->behaviour = behaviour;
    ent->moving = 0;
    ent->goal_reached = 0;
    ent->goal_sector = ref_ent->oid;
    AIEntities[AIEntities_amnt++] = ent;
    return ent;
}



static void S_HANDLE_GARBAGE(Level *level) {
    for (int g = GCEntities_amnt - 1; g >= 0; g--) {
        gc_entity *gc = GCEntities[g];
        if (--gc->timer > 0) continue;

        ai_entity *dead = gc->ref_ai;
        int del = dead->entity_index;

        if (del >= 0 && del < amnt_entities) {
            for (int i = del; i < amnt_entities - 1; i++)
                entities[i] = entities[i + 1];
            amnt_entities--;
        }
        for (int i = 0; i < AIEntities_amnt; i++) {
            if (AIEntities[i] == dead) {
                free(dead);
                for (int j = i; j < AIEntities_amnt - 1; j++)
                    AIEntities[j] = AIEntities[j + 1];
                AIEntities_amnt--;
                break;
            }
        }
        for (int i = 0; i < AIEntities_amnt; i++) {
            ai_entity *ai = AIEntities[i];
            if (ai->entity_index > del)
                ai->entity_index--;
            ai->ref_entity = &entities[ ai->entity_index ];
        }
        free(gc);
        for (int i = g; i < GCEntities_amnt - 1; i++)
            GCEntities[i] = GCEntities[i + 1];
        GCEntities_amnt--;
    }
}


static void S_WALK_PORTAL(wall *Portal, ai_entity *Entity) {
    if (!Entity->moving) {
        _Float16 cx = (Portal->x1+Portal->x0)/2;
        _Float16 cy = (Portal->y1+Portal->y0)/2;
        Entity->gx = cx;
        Entity->gy = cy;
    }
}

static void S_WALK_CENTROID(sector *Sector, ai_entity *Entity) {
    if (!Entity->moving) {
        Entity->gx = Sector->cx;
        Entity->gy = Sector->cy;
    }
}

void S_AI_WALK(Level *level, ai_entity *Entity, sector *originSector, int8_t gid) {
    Entity->ref_entity->x += Entity->ref_entity->vx;
    Entity->ref_entity->y += Entity->ref_entity->vy;

    if (Entity->ref_entity->oid == gid) {
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
    if (!Entity->moving) {
        for (int p = 0; p < originSector->count; p++) {
            wall *Portal = &originSector->walls[p];
            if (Portal->is_portal) {
                int8_t ref_id = portal_ref_array[Portal->uid];
                if (S_IS_REACHABLE(portal_tree.portals[ref_id].canReach, gid)) {
                    S_WALK_PORTAL(Portal, Entity);
                    S_APPLY_IMPULSE(Entity, Entity->gx, Entity->gy);
                    Entity->moving = 1;
                    break;
                }
            }
        }
    } else {
        int reachedGoal = S_APPLY_IMPULSE(Entity, Entity->gx, Entity->gy);
        if (reachedGoal) {
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
                Entity->goal_sector = newIndex;
                int eidx = Entity->entity_index;
                Entity->ref_entity = S_MOVE_ENTITY_TO_SECTOR(level, eidx, newIndex);
            }
            Entity->moving = 0;
            Entity->ref_entity->vx = 0;
            Entity->ref_entity->vy = 0;
        }
    }
}

static inline void S_INIT_AI(Level *level) {
    AIEntities_amnt = 0;
    if (amnt_entities > 0) {
        S_ADD_AI(&entities[0], 0, 0);
    }
}

static inline void S_HANDLE_AI(Level *level) {
    S_HANDLE_GARBAGE(level);
    for (int e=0; e<AIEntities_amnt; e++) {
        ai_entity *Entity = AIEntities[e];
        switch(Entity->behaviour) {
            case 0:
            {
                S_AI_WALK(level, Entity, level->sectors[Entity->ref_entity->oid], 2); // example; walk to sector 2
                break;
            }
            case 1:
            {
                entity *ref_entity = Entity->ref_entity;
                ref_entity->h += 0.1;
                break;
            }
        }
    }
}

static inline void S_GENERATE_GRAPH(Level *level) { 
    for (int hs=0; hs<level->count; hs++) {
        sector *OriginSector = level->sectors[hs];
        intOriginSector = hs;
        for (int p=0; p<OriginSector->count; p++) {
            wall *Portal = &OriginSector->walls[p];
            if (Portal->is_portal) {
                portal_ref_array[Portal->uid] = portal_ref;
                S_SET_REACHABLE(portal_tree.portals[portal_ref].canReach, Portal->portal_link);
                traversedPortals[Portal->uid] = 1;

                S_TRAVERSE(level, OriginSector, level->sectors[Portal->portal_link]);
                portal_ref++;
            }
        }
    }
}

#endif
