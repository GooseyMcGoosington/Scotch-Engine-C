#ifndef gui
#define gui

#include <stdio.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "helper.h"
#include "classes.h"

#include <unistd.h>

extern void G_GUI_MOUSE_CLICK(int x, int y);

int G_GUI_SCENE = 0;
static int row = 0;
static int row_db = 0;

typedef struct { // Struct for a Text. X/Y is relative to the SCENE. If you exceed your coordinate bounds, I will find you and I will kill you.
    int x;
    int y;
    char text[32];
    SDL_Surface *txt_surface;
} G_TEXT;

typedef struct { // Struct for a TextButton. X/Y is relative to the SCENE. If you exceed your coordinate bounds, I will find you and I will kill you.
    int x;
    int y;
    int w; // Bounding Box Width
    int h; // Bounding Box Height
    int tag; // What is it meant to do?
    char text[32];
    SDL_Surface *txt_surface;
} G_TEXT_BUTTON;


typedef struct {
    int x;
    int y;
    int w;
    int h;
    size_t txt_count;
    G_TEXT *text[8];
    size_t txtbtn_count;
    G_TEXT_BUTTON *textbtns[8];
} G_SCENE;

typedef struct {
    size_t count;
    G_SCENE *scenes[8];
} G_SCENE_LIST;

static const SDL_Color FONT_WHITE = {255, 255, 255};

#endif
