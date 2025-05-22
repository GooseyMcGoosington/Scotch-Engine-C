#ifndef classes
#define classes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    int x0;
    int x1;
    int y0;
    int y1;
    int is_portal;
    float portal_top;
    float portal_bottom;
    int portal_link;
    float length;
    int tIndex;
    int uid;
} wall;

typedef struct {
    int8_t anchored;
    float x;
    float y;
    float h;
    int8_t tid;
    int8_t animated;
    int8_t a;
} entity;

typedef struct {
    float elevation;
    float height;
    int cIndex;
    int fIndex;
    int amnt_entities;
    entity *entities;
    size_t count;
    wall walls[];
} sector;

typedef struct {
    size_t count;
    sector *sectors[];
} Level;

typedef struct {
    float x;
    float y;
    float z;
    float h;
    float yaw;
    float focalLength;
    float fov;
    float fovWidth;
} player;

typedef struct {
    float x0;
    float x1;
    float y0;
    float y1;
    float y2;
    float y3;
    float dxCull;
} portalCull;

typedef struct {
    int sector_link;
    portalCull portalBounds;
    int clipped;
    int uid;
} portalRender;

typedef struct {
    uint16_t *pixels;
    int width;
    int height;
    int scale;
} tfile;

typedef struct {
    tfile *files;
} tlut;

#endif // MY_STRUCTS_H