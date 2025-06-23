#include "g_gui.h"
#include "s_sound.h"

extern int control_locked;
extern SDL_Palette *PALETTE8;
extern player character;
extern tlut *texture_list;

G_SCENE_LIST G_GUI_SCENE_LIST;
unsigned int G_CURRENT_SCENE = 1;

G_TEXT *G_HUD_HEALTH=NULL;
G_TEXT *G_HUD_ARMOR=NULL;
G_TEXT *G_HUD_WEAPON=NULL;

SDL_Surface* G_GUI_CREATE_TEXT_SURF(char *txt) {
    TTF_Font *font = TTF_OpenFont("fonts/pxlbold.ttf", 16);
    SDL_Surface *textSurface = TTF_RenderUTF8_Solid(font, txt, FONT_WHITE);
    TTF_CloseFont(font);

    SDL_Surface *palettizedSurface = SDL_CreateRGBSurface(0, textSurface->w, textSurface->h, 8, 0, 0, 0, 0);
    SDL_SetSurfacePalette(palettizedSurface, PALETTE8);
    SDL_SetColorKey(palettizedSurface, SDL_TRUE, 0);
    SDL_BlitSurface(textSurface, NULL, palettizedSurface, NULL);
    SDL_FreeSurface(textSurface);

    return palettizedSurface;
}

void G_GUI_CREATE_NEW_TXTBTN_SURFACE(G_TEXT_BUTTON *TEXT, char *txt) {
    strncpy(TEXT->text, txt, sizeof(TEXT->text)-1);
    TEXT->text[sizeof(TEXT->text) - 1] = '\0';
    SDL_FreeSurface(TEXT->txt_surface);
    TEXT->txt_surface = G_GUI_CREATE_TEXT_SURF(txt);
}

void G_GUI_CREATE_NEW_TXT_SURFACE(G_TEXT *TEXT, char *txt) {
    strncpy(TEXT->text, txt, sizeof(TEXT->text)-1);
    TEXT->text[sizeof(TEXT->text) - 1] = '\0';
    SDL_FreeSurface(TEXT->txt_surface);
    TEXT->txt_surface = G_GUI_CREATE_TEXT_SURF(txt);
}

void G_GUI_MOUSE_CLICK(int x, int y) {
    if ((!control_locked) || (G_CURRENT_SCENE==-1)) return; // We are not in the menu, so we do not have to register inputs.
    G_SCENE *SCENE = G_GUI_SCENE_LIST.scenes[G_CURRENT_SCENE];
    for (int btn=0; btn<SCENE->txtbtn_count; btn++) {
        G_TEXT_BUTTON *txtbtn = SCENE->textbtns[btn];
        int bx = SCENE->x+txtbtn->x;
        int by = SCENE->y+txtbtn->y;
        int bw = txtbtn->w;
        int bh = txtbtn->h;
        int left = bx;
        int right = bx+bw;
        int bottom = by;
        int top = by+bh;

        if ((x > left) && (x < right) && (y < top) && (y > bottom)) {
            // I'll use a big if chain here. Obviously, I could make it cleaner, but I'm dreadfully lazy. It works anyways.
            int tag = txtbtn->tag;
            S_SOUND_PLAY_MONO("ui_clk.wav");
            switch(tag) {
                case 0:
                {
                    G_CURRENT_SCENE = 0;
                    break;
                }
                case 1:
                {
                    G_CURRENT_SCENE = 1;
                    break;
                }
                case 2:
                {
                    control_locked = 0;
                    G_CURRENT_SCENE = -1; // Not in Menu
                    break;
                }
                case 3:
                {
                    resPos ++;
                    if (resPos>3) {
                        resPos = 0;
                    }
                    char label[32];
                    snprintf(label, sizeof(label), "%s: %d", "RESOLUTION", resPos);
                    G_GUI_CREATE_NEW_TXTBTN_SURFACE(txtbtn, label);

                    char numstr[2];
                    snprintf(numstr, sizeof(numstr), "%d", resPos);
                    INI_WRITE("settings.ini", "res", numstr);
                    break;
                }
            }
        }
    }
};


G_TEXT* G_GUI_CREATE_TEXT(char *txt, int x, int y) {
    G_TEXT *TEXT = (G_TEXT*)malloc(sizeof(G_TEXT));
    TEXT->x=x;
    TEXT->y=y;
    strncpy(TEXT->text, txt, sizeof(TEXT->text)-1);
    TEXT->text[sizeof(TEXT->text) - 1] = '\0';
    TEXT->txt_surface = G_GUI_CREATE_TEXT_SURF(txt);

    return TEXT;
}

G_TEXT_BUTTON* G_GUI_CREATE_TEXTBUTTON(char *txt, int x, int y, int w, int h, int tag) {
    G_TEXT_BUTTON *TEXTBUTTON = (G_TEXT_BUTTON*)malloc(sizeof(G_TEXT_BUTTON));
    TEXTBUTTON->x=x;
    TEXTBUTTON->y=y;
    TEXTBUTTON->w=w;
    TEXTBUTTON->h=h;
    TEXTBUTTON->tag=tag;
    strncpy(TEXTBUTTON->text, txt, sizeof(TEXTBUTTON->text)-1);
    TEXTBUTTON->text[sizeof(TEXTBUTTON->text) - 1] = '\0';
    TEXTBUTTON->txt_surface = G_GUI_CREATE_TEXT_SURF(txt);

    return TEXTBUTTON;
}

G_SCENE* G_GUI_CREATE_SCENE(int x, int y, int w, int h, int txt_count, int txtbtn_count) {
    G_SCENE *SCENE = (G_SCENE*)malloc(sizeof(G_SCENE));
    SCENE->x = x;
    SCENE->y = y;
    SCENE->w = w;
    SCENE->h = h;
    SCENE->txt_count = txt_count;
    SCENE->txtbtn_count = txtbtn_count;
    return SCENE;
}


/*void G_GUI_DRAW_FIT_IMAGE(uint8_t *pixels, int tid) {
    tfile texture = texture_list->files[tid];
    int W = texture.width;
    int H = texture.height;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int texIndex = y * W + x;        // index in texture pixel array
            int screenIndex = y * SW + x;    // index in screen pixels buffer

            uint8_t index = texture.pixels[texIndex];
            pixels[screenIndex] = index;
        }
    }
}*/

void G_GUI_DRAW_FIT_IMAGE(uint8_t *pixels, int tid) {
    tfile texture = texture_list->files[tid];
    int W = texture.width;
    int H = texture.height;

    float scaleX = (float)W / SW;
    float scaleY = (float)H / SH;

    for (int y = 0; y < SH; y++) {
        int texY = (int)(y * scaleY);
        if (texY >= H) texY = H - 1;

        for (int x = 0; x < SW; x++) {
            int texX = (int)(x * scaleX);
            if (texX >= W) texX = W - 1;

            int texIndex = texY * W + texX;
            int screenIndex = y * SW + x;

            pixels[screenIndex] = texture.pixels[texIndex];
        }
    }
}

void G_GUI_DRAW_FIT_TILED_IMAGE(uint8_t *pixels, int tid, int tileScale) {
    tfile texture = texture_list->files[tid];
    int W = texture.width;

    for (int y = 0; y < SH; y++) {
        int texY = y;
        for (int x = 0; x < SW; x++) {
            int texX = x;

            int texIndex = texY%tileScale * W + texX%tileScale;
            int screenIndex = y * SW + x;

            pixels[screenIndex] = texture.pixels[texIndex];
        }
    }
}

void G_GUI_BLIT_SURF(uint8_t *fpixels, G_TEXT *TEXT) {
    SDL_Surface *SURF = TEXT->txt_surface;

    Uint8 *pixels = SURF->pixels;

    int X = TEXT->x;
    for (int x = 0; x<SURF->w; x++) {
        X++;
        int Y = TEXT->y;
        for (int y = 0; y<SURF->h; y++) {
            Y++;
            Uint8 index = pixels[y*SURF->pitch+x];
            if (index==0 || index == 232) continue;
            fpixels[Y*SW+X] = index;
        }
    }
    return;
}

void G_GUI_INIT() {
    G_GUI_SCENE_LIST.count = 2; // 1 Scene
    // Scene 0
    G_SCENE *SCENE_00 = G_GUI_CREATE_SCENE(SW2, SH2, 32, 64, 0, 2);
    G_TEXT_BUTTON *TEXT_00 = G_GUI_CREATE_TEXTBUTTON("Play", 0, 0, 32, 32, 2);
    G_TEXT_BUTTON *TEXT_01 = G_GUI_CREATE_TEXTBUTTON("Settings", 0, 32, 32, 32, 1);
    SCENE_00->textbtns[0] = TEXT_00;
    SCENE_00->textbtns[1] = TEXT_01;
    G_GUI_SCENE_LIST.scenes[0] = SCENE_00;
    // Scene 1
    G_SCENE *SCENE_01 = G_GUI_CREATE_SCENE(SW2, SH2, 32, 96, 1, 2);
    G_TEXT *TEXT_04 = G_GUI_CREATE_TEXT("Restart Required", 0, 0);

    char label[32];
    snprintf(label, sizeof(label), "%s: %d", "RESOLUTION", resPos);
    G_TEXT_BUTTON *TEXT_02 =  G_GUI_CREATE_TEXTBUTTON(label, 0, 32, 32, 32, 3);
    G_TEXT_BUTTON *TEXT_03 = G_GUI_CREATE_TEXTBUTTON("Back", 0, 64, 32, 32, 0);
    SCENE_01->text[0] = TEXT_04;
    SCENE_01->textbtns[0] = TEXT_02;
    SCENE_01->textbtns[1] = TEXT_03;
    G_GUI_SCENE_LIST.scenes[1] = SCENE_01;

    // Set up the HUD
    char str[8];
    snprintf(str, sizeof(str), "%d", character.stats.health);
    G_HUD_HEALTH = G_GUI_CREATE_TEXT(str, SW*0.05, SH*0.93);
    snprintf(str, sizeof(str), "%d", character.stats.armor);
    G_HUD_ARMOR = G_GUI_CREATE_TEXT(str, SW*0.2, SH*0.93);
    snprintf(str, sizeof(str), "%d", character.stats.wep);
    G_HUD_WEAPON = G_GUI_CREATE_TEXT(str, SW*0.05, SH*0.87);
}

void G_GUI_BLIT_HUD(uint8_t *fpixels) {
    G_GUI_BLIT_SURF(fpixels, G_HUD_HEALTH);
    G_GUI_BLIT_SURF(fpixels, G_HUD_ARMOR);
    G_GUI_BLIT_SURF(fpixels, G_HUD_WEAPON);
    return;
}

void G_GUI_UPDATE_HUD(uint8_t *fpixels) {
    char str[8];
    snprintf(str, sizeof(str), "%d", character.stats.health);
    G_GUI_CREATE_NEW_TXT_SURFACE(G_HUD_HEALTH, str);
    snprintf(str, sizeof(str), "%d", character.stats.armor);
    G_GUI_CREATE_NEW_TXT_SURFACE(G_HUD_ARMOR, str);
    snprintf(str, sizeof(str), "%d", character.stats.wep);
    G_GUI_CREATE_NEW_TXT_SURFACE(G_HUD_WEAPON, str);
}

void G_GUI_BLIT(SDL_Surface *FRAME_BUFFER, SDL_Rect *destRect) {
    Uint8 *fpixels = FRAME_BUFFER->pixels;
    // Render HUD first
    G_GUI_BLIT_HUD(fpixels);
    if ((!control_locked) || (G_CURRENT_SCENE==-1)) return; // We are not in the menu, so we do not have to render it.
    G_GUI_DRAW_FIT_TILED_IMAGE(fpixels, 9, 64);

    G_SCENE *SCENE = G_GUI_SCENE_LIST.scenes[G_CURRENT_SCENE];
    for (int w=0; w<SCENE->txt_count; w++) {
        G_TEXT *TEXT = SCENE->text[w];
        SDL_Surface *SURF = TEXT->txt_surface;

        Uint8 *pixels = SURF->pixels;

        int X = SCENE->x+TEXT->x;
        for (int x = 0; x<SURF->w; x++) {
            X++;
            int Y = SCENE->y+TEXT->y;
            for (int y = 0; y<SURF->h; y++) {
                Y++;
                Uint8 index = pixels[y*SURF->pitch+x];
                if (index==0 || index == 232) continue;
                fpixels[Y*SW+X] = index;
            }
        }
    }
    // Text Buttons
    for (int w=0; w<SCENE->txtbtn_count; w++) {
        G_TEXT_BUTTON *TEXT = SCENE->textbtns[w];
        SDL_Surface *SURF = TEXT->txt_surface;

        Uint8 *pixels = SURF->pixels;

        int X = SCENE->x+TEXT->x;
        for (int x = 0; x<SURF->w; x++) {
            X++;
            int Y = SCENE->y+TEXT->y;
            for (int y = 0; y<SURF->h; y++) {
                Y++;
                Uint8 index = pixels[y*SURF->pitch+x];
                if (index==0 || index == 232) continue;
                fpixels[Y*SW+X] = index;
            }
        }
    }
}

void G_GUI_QUIT() {

}