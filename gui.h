#ifndef gui
#define gui

#include <stdio.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "helper.h"
#include "classes.h"
#include <unistd.h>

int G_GUI_SCENE = 0;
static int row = 0;
static int row_db = 0;
static char *G_IMAGE_REF[8] = {NULL};

typedef struct {
    int x; // relative
    int y; // relative
    float sx;
    float sy;
    int image;
    SDL_Surface *img_surface;
    int visible;
} G_IMAGE;

typedef struct {
    int x;
    int y;
    char text[32];
    SDL_Surface *txt_surface;
} G_TEXT;

typedef struct {
    int x;
    int y;
    int id;
    char text[32];
    SDL_Surface *txt_surface;
} G_TEXTBUTTON;

typedef struct {
    int x;
    int y;
    int w;
    int h;
    size_t txt_count;
    size_t img_count;
    size_t txtbtn_count;
    G_TEXT *text;
    G_IMAGE *image;
    G_TEXTBUTTON *buttons;
} G_SCENE;

typedef struct {
    size_t count;
    G_SCENE **scenes[8];
} G_SCENE_LIST;

static const SDL_Color FONT_WHITE = {255, 255, 255};

#endif
