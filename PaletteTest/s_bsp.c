#ifndef BSP_H
#define BSP_H

#include <stdlib.h>
#include <stdio.h>
#include "s_classes.h"
#include "s_helper.h"

extern Level *level;

#define MAX_SEGMENTS 256

typedef struct {
    float x, y;
} Vector2;

typedef struct {
    Vector2 start;
    Vector2 end;
    int sector;
} line_t;

typedef struct node_s {
    line_t partition;
    struct node_s *front;
    struct node_s *back;
    int is_leaf;
    line_t *walls;
    int wall_count;
} node_t;

static unsigned int raw_segment_amnt = 0;
static line_t raw_segments[MAX_SEGMENTS];
node_t *root_node = NULL;

// Drawing function you provide
extern void R_LINE_WALL(uint8_t *pixels, int x0, int y0, int x1, int y1, uint8_t colour);

static Vector2 intersect_line(Vector2 A, Vector2 B, Vector2 C, Vector2 D) {
    float a1 = B.y - A.y;
    float b1 = A.x - B.x;
    float c1 = a1 * A.x + b1 * A.y;

    float a2 = D.y - C.y;
    float b2 = C.x - D.x;
    float c2 = a2 * C.x + b2 * C.y;

    float det = a1 * b2 - a2 * b1;
    if (det == 0) return (Vector2){0, 0};

    float x = (b2 * c1 - b1 * c2) / det;
    float y = (a1 * c2 - a2 * c1) / det;
    return (Vector2){x, y};
}

static node_t *bsp_partition(line_t *segs, int count) {
    if (count <= 0) return NULL;

    node_t *node = malloc(sizeof(node_t));
    if (!node) return NULL;

    if (count == 1) {
        node->is_leaf = 1;
        node->walls = malloc(sizeof(line_t));
        if (!node->walls) {
            free(node);
            return NULL;
        }
        node->walls[0] = segs[0];
        node->wall_count = 1;
        node->front = node->back = NULL;
        return node;
    }

    node->is_leaf = 0;
    node->partition = segs[0];
    node->walls = NULL;
    node->wall_count = 0;

    line_t *front = malloc(sizeof(line_t) * count);
    line_t *back = malloc(sizeof(line_t) * count);
    if (!front || !back) {
        free(front);
        free(back);
        free(node);
        return NULL;
    }
    int front_count = 0, back_count = 0;

    Vector2 A = segs[0].start;
    Vector2 B = segs[0].end;

    for (int i = 1; i < count; i++) {
        Vector2 P0 = segs[i].start;
        Vector2 P1 = segs[i].end;

        float cp0 = (P0.x - A.x)*(B.y - A.y) - (P0.y - A.y)*(B.x - A.x);
        float cp1 = (P1.x - A.x)*(B.y - A.y) - (P1.y - A.y)*(B.x - A.x);

        if (cp0 >= 0 && cp1 >= 0) {
            front[front_count++] = segs[i];
        } else if (cp0 <= 0 && cp1 <= 0) {
            back[back_count++] = segs[i];
        } else {
            Vector2 I = intersect_line(A, B, P0, P1);
            if (cp0 > 0) {
                front[front_count++] = (line_t){P0, I, segs[i].sector};
                back[back_count++]  = (line_t){I, P1, segs[i].sector};
            } else {
                front[front_count++] = (line_t){I, P1, segs[i].sector};
                back[back_count++]  = (line_t){P0, I, segs[i].sector};
            }
        }
    }

    node->front = bsp_partition(front, front_count);
    node->back  = bsp_partition(back,  back_count);

    free(front);
    free(back);
    return node;
}


void bsp_traverse_draw(node_t *node, uint8_t *pixels, uint8_t colour) {
    if (!node) return;
    if (node->is_leaf) {
        for (int i = 0; i < node->wall_count; i++) {
            int x0 = (int)node->walls[i].start.x + SW2;
            int y0 = (int)node->walls[i].start.y + SH2;
            int x1 = (int)node->walls[i].end.x + SW2;
            int y1 = (int)node->walls[i].end.y + SH2;
            R_LINE_WALL(pixels, x0, y0, x1, y1, colour);
        }
    } else {
        bsp_traverse_draw(node->front, pixels, colour);
        bsp_traverse_draw(node->back, pixels, colour);
    }
}

void debug_dump_bsp(node_t *node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; i++) printf("  ");
    if (node->is_leaf) {
        printf("Leaf: %d wall(s)\n", node->wall_count);
        for (int i = 0; i < node->wall_count; i++) {
            for (int j = 0; j < depth+1; j++) printf("  ");
            line_t *l = &node->walls[i];
            printf("Wall from (%f, %f) to (%f, %f)\n", l->start.x, l->start.y, l->end.x, l->end.y);
        }
    } else {
        printf("Node: split line (%f, %f) to (%f, %f)\n", node->partition.start.x, node->partition.start.y, node->partition.end.x, node->partition.end.y);
        debug_dump_bsp(node->front, depth + 1);
        debug_dump_bsp(node->back, depth + 1);
    }
}

void bsp_init() {
    raw_segment_amnt = 0;
    for (int s = 0; s < level->count; s++) {
        sector *Sector = level->sectors[s];
        for (int w = 0; w < Sector->count; w++) {
            wall Wall = Sector->walls[w];
            raw_segments[raw_segment_amnt++] = (line_t){ {Wall.x0, Wall.y0}, {Wall.x1, Wall.y1}, s };
        }
    }
    root_node = bsp_partition(raw_segments, 6);
    printf("BSP tree built with %u segments\n", 6);
    debug_dump_bsp(root_node, 0);
}

#endif // BSP_H
