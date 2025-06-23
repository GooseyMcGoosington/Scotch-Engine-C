#ifndef classes
#define classes

#include "SDL2/SDL_mixer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    int x0;
    int x1;
    int y0;
    int y1;
    int8_t is_portal;
    float portal_top;
    float portal_bottom;
    int16_t portal_link;
    float length;
    int16_t tIndex;
    int16_t uid;
    int16_t oid;
}   wall;

typedef struct {
    int8_t anchored;
    float x;
    float y;
    float vx;
    float vy;
    float mass;
    float h;
    int8_t tid;
    int8_t animated;
    int8_t a;
    int8_t oid;
}   entity;

typedef struct {
    entity  *ref_entity;     // ptr to the current entity
    uint8_t  entity_index;   // <-- its slot in entities[]
    uint8_t  behaviour;
    uint8_t  moving;
    uint8_t  goal_reached;
    _Float16 gx, gy;
    uint8_t  state;
    uint8_t  goal_sector;    // <-- rename entity_id to this
} ai_entity;

typedef struct {
    ai_entity *ref_ai;
    uint16_t timer; 
}   gc_entity;

typedef struct {
    float elevation;
    float height;
    int cIndex;
    int fIndex;
    //int amnt_entities;
    float cx; // centroid x
    float cy; // centroid y
    int8_t light;
    int8_t id;
    //entity *entities;
    size_t count;
    wall walls[];
}   sector;

typedef struct {
    size_t count;
    sector *sectors[];
}   Level;

typedef union {
    struct {
        int8_t wep, health, armor, flags;
    };
    int8_t vars[4];
    int32_t packed;
} playerInfo;

typedef struct {
    float x;
    float y;
    float z;
    float h;
    float yaw;
    float pitch;
    float focalLength;
    float fov;
    float fovWidth;
    float radius;
    playerInfo stats;
}   player;

typedef struct {
    float x0;
    float x1;
    float y0;
    float y1;
    float y2;
    float y3;
    float dxCull;
}   portalCull;

typedef struct {
    int sector_link;
    portalCull portalBounds;
    int clipped;
    int uid;
}   portalRender;

typedef struct {
    uint8_t *pixels;
    int width;
    int height;
    int scale;
}   tfile;

typedef struct {
    tfile *files;
}   tlut;

typedef enum {
    CRUSHER,
    DOOR_RAISE,
    LIGHT_FLICKER,
}   event_type_e;

const int8_t event_types[3] = {
    [CRUSHER] = 0,
    [DOOR_RAISE] = 1,
    [LIGHT_FLICKER] = 2,
};

// Scriptable Sectors
typedef struct {
    int16_t id;
    sector *Sector; // Pointer to Sector
    int8_t type; // What is it meant to do? can it crush, or open?
    float arg1;
    float arg2;
    int8_t state;
    int8_t delay;

    int8_t linkSize;
    wall **linkWalls;

    int8_t otherSize;
    wall **otherWalls;
}   listener;

typedef struct {
    size_t size;
    listener *listeners;
}   sector_listeners;

typedef struct {
    float x0;
    float y0;
    float x1;
    float y1;
}   segment;

typedef struct {
    uint8_t canReach[32]; // 1 if this portal can eventually lead to that sector. The index is the sector id. 256 max sectors defined by bitfield
}   portal_reach;

typedef struct {
    portal_reach portals[256]; // 256 max portals in the engine
}   portal_graph;

typedef struct {
    int x0; 
    int x1; 
    int y0;
    int y1;
    int y2;
    int y3;
} cull;

typedef struct {
    Mix_Chunk *chunk;
    char name[24];
} WAV_FILE;

typedef struct {
    float x;
    float y;
    int active;
} SOUND_POINT;

#endif // MY_STRUCTS_H