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
} wall;

typedef struct {
    float elevation;
    float height;
    size_t count;
    wall walls[];
} sector;

typedef struct {
    size_t count;
    sector *sectors[];
} Level;

// 256*32*8 = 8.192 KB
#endif // MY_STRUCTS_H