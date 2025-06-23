#include "classes.h"
#include "helper.h"
#include "g_gui.h"

extern int quit;
extern player character;
extern SDL_Surface *framebuffer;
extern SDL_Surface *surface;
extern SDL_Window *window;
extern Level *level;
extern float sn[361];
extern float cs[361];
extern sector *playerSector;

extern void P_CHARACTER_DISPATCH_WEP();

extern SDL_Rect destRect;
extern unsigned int G_CURRENT_SCENE;