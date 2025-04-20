#include "gui.h"

static TTF_Font *mainFont = NULL;
static G_SCENE *G_GUI = NULL;

static G_SCENE *G_MENU = NULL;
static G_SCENE *G_SETTINGS = NULL;
static G_SCENE_LIST *G_SCENES = NULL;

void G_HANDLE(SDL_Surface *RGB_SURFACE, SDL_Window *WINDOW, int *inputs) {
    int UP = inputs[82];
    int DOWN = inputs[81];
    int ENTER = inputs[40];

    if ((inputs[82]==0) & (inputs[81]==0)) {
        row_db = 7;
    }
    if (UP) {
        row_db++;
        if (row_db == 8) {
            row--;
            row_db = 0;
        }
    }
    if (DOWN) {
        row_db++;
        if (row_db == 8) {
            row++;
            row_db = 0;
        }
    }
    if (row < 0) {
        row = G_GUI->txtbtn_count-1;
    }
    if (row > (G_GUI->txtbtn_count-1)) {
        row = 0;
    }

    if (G_GUI->txtbtn_count > 0) {
        G_TEXTBUTTON *current_button = &G_GUI->buttons[row];
        if (ENTER) {
            if (current_button->id == 0) {
                SDL_DestroyWindow(WINDOW);
                SDL_Quit();
            } else if (current_button->id == 1) {
                G_GUI_SCENE = 1;
                G_GUI = *G_SCENES->scenes[G_GUI_SCENE];
                row = 0;
                //G_TEXTBUTTON *current_button = &G_GUI->buttons[row];
            } else if (current_button->id == 3) {
                G_GUI_SCENE = 0;
                G_GUI = *G_SCENES->scenes[G_GUI_SCENE];
                row = 0;
                //G_TEXTBUTTON *current_button = &G_GUI->buttons[row];
            }
            //strcpy(current_button->text, "Copied String");
            current_button->txt_surface = TTF_RenderText_Solid(mainFont, current_button->text, FONT_WHITE);
        }
        G_GUI->image[0].x = (G_GUI->x + current_button->x + (strlen(current_button->text)*18));
        G_GUI->image[0].y = (G_GUI->y + current_button->y - 8);
    }

    for (int text = 0; text<(G_GUI->txt_count); text++) {
        G_TEXT txt = G_GUI->text[text];
        SDL_Rect destRect = {G_GUI->x + txt.x, G_GUI->y + txt.y, G_GUI->w, G_GUI->h};
        SDL_BlitSurface(txt.txt_surface, NULL, RGB_SURFACE, &destRect);
    }
    for (int text = 0; text<(G_GUI->txtbtn_count); text++) {
        G_TEXTBUTTON txt = G_GUI->buttons[text];
        SDL_Rect destRect = {G_GUI->x + txt.x, G_GUI->y + txt.y, G_GUI->w, G_GUI->h};
        SDL_BlitSurface(txt.txt_surface, NULL, RGB_SURFACE, &destRect);
    }
}

void G_HANDLE_IMAGE(SDL_Surface *RGB_SURFACE, SDL_Window *WINDOW) {
    for (int img = 0; img < G_GUI->img_count; img++) {
        G_IMAGE image = G_GUI->image[img];
        if (image.visible == 0) continue;
        Uint16 *pixels = (Uint16 *)image.img_surface->pixels;
        int width = image.img_surface->w;
        int height = image.img_surface->h;

        float sx = image.sx;
        float sy = image.sy;
        int destX = image.x;
        int destY = image.y;
        int scaledWidth = (int)clamp(width * sx, 1, SW1);
        int scaledHeight = (int)clamp(height * sy, 1, SH1);

        for (int scaledY = 0; scaledY < scaledHeight; scaledY++) {
            int destPosY = destY + scaledY;
            int destPosYW = destPosY * RGB_SURFACE->w;
            for (int scaledX = 0; scaledX < scaledWidth; scaledX++) {
                int destPosX = destX + scaledX;
                int originalX = (int)(scaledX / sx);  // Reverse scaling for x
                int originalY = (int)(scaledY / sy);  // Reverse scaling for y
                Uint16 pixelColor = pixels[originalY * width + originalX];
                
                if (pixelColor != 0) {
                    ((Uint16*)RGB_SURFACE->pixels)[destPosYW + destPosX] = pixelColor;
                }
            }
        }
    }
}

void G_CREATE_TEXT(char *txt, int x, int y, G_SCENE **GUI) {
    SDL_Surface *surf = TTF_RenderText_Solid(mainFont, txt, FONT_WHITE);
    G_TEXT word;
    word.x = x;
    word.y = y;
    word.txt_surface = surf;
    strcpy(word.text, txt);

    (*GUI)->text = realloc((*GUI)->text, sizeof(G_TEXT) * ((*GUI)->txt_count + 1));
    (*GUI)->text[(*GUI)->txt_count++] = word;
}

void G_CREATE_TEXTBUTTON(char *txt, int x, int y, int id, G_SCENE **GUI) {
    SDL_Surface *surf = TTF_RenderText_Solid(mainFont, txt, FONT_WHITE);
    G_TEXTBUTTON word;
    word.x = x;
    word.y = y;
    word.id = id;
    word.txt_surface = surf;
    strcpy(word.text, txt);

    (*GUI)->buttons = realloc((*GUI)->buttons, sizeof(G_TEXTBUTTON) * ((*GUI)->txtbtn_count + 1));
    (*GUI)->buttons[(*GUI)->txtbtn_count++] = word;
}

void G_CREATE_IMAGE(int x, int y, float sx, float sy, G_SCENE **GUI) {
    G_IMAGE word;
    word.x = x;
    word.y = y;
    word.sx = sx;
    word.sy = sy;
    word.image = 0;
    word.visible = 1;
    // Get the image path from G_IMAGES array
    char *imagePath = G_IMAGE_REF[word.image];
    printf("Loading image from path: %s\n", imagePath);

    SDL_Surface* loadedSurface = SDL_LoadBMP(imagePath);
    SDL_Surface* convertedSurface = SDL_ConvertSurfaceFormat(loadedSurface, SDL_PIXELFORMAT_RGB565, 0);
    SDL_FreeSurface(loadedSurface);
    
    word.img_surface = convertedSurface;

    (*GUI)->image = realloc((*GUI)->image, sizeof(G_IMAGE) * ((*GUI)->img_count + 1));
    (*GUI)->image[(*GUI)->img_count++] = word;
}

void G_INIT() {
    TTF_Init();    
    mainFont = TTF_OpenFont("fonts/main.ttf", 24);
    G_IMAGE_REF[0] = "textures/arrow.bmp";

    G_MENU = malloc(sizeof(G_SCENE));  // Allocate memory for the GUI scene structure
    G_MENU->x = 0;
    G_MENU->y = 0;
    G_MENU->w = SW1;
    G_MENU->h = SH1;
    G_MENU->txt_count = 0;
    G_MENU->img_count = 0;
    G_MENU->txtbtn_count = 0;
    G_MENU->text = NULL;
    G_MENU->image = NULL;
    G_MENU->buttons = NULL;
    G_CREATE_TEXTBUTTON("PLAY", SW2, SH2, 2, &G_MENU);
    G_CREATE_TEXTBUTTON("SETTINGS", SW2, SH2+24, 1, &G_MENU);
    G_CREATE_TEXTBUTTON("QUIT", SW2, SH2+48, 0, &G_MENU);
    //G_CREATE_TEXT("Hello, World!", SW2, SH2, &G_GUI);
    G_CREATE_IMAGE(0, 0, 1.0f, 1.0f, &G_MENU);
    
    G_SCENES = malloc(sizeof(G_SCENE_LIST));
    G_SCENES->scenes[0] = &G_MENU;
    G_SCENES = realloc(G_SCENES, 2*sizeof(G_SCENE)+sizeof(G_SCENE_LIST));

    G_SETTINGS = malloc(sizeof(G_SCENE));
    G_SETTINGS->x = 0;
    G_SETTINGS->y = 0;
    G_SETTINGS->w = SW1;
    G_SETTINGS->h = SH1;
    G_SETTINGS->txt_count = 0;
    G_SETTINGS->img_count = 0;
    G_SETTINGS->txtbtn_count = 0; 
    G_SETTINGS->text = NULL;
    G_SETTINGS->image = NULL;
    G_SETTINGS->buttons = NULL;

    G_CREATE_TEXT("GAME SETTINGS", SW2, SH2-96, &G_SETTINGS);
    G_CREATE_TEXT("SOME SETTINGS REQUIRE RESTART", SW2, SH2-128, &G_SETTINGS);
    G_CREATE_TEXTBUTTON("DEPTH SHADING", SW2, SH2-48, 2, &G_SETTINGS);
    G_CREATE_TEXTBUTTON("FULLSCREEN", SW2, SH2-24, 2, &G_SETTINGS);
    G_CREATE_TEXTBUTTON("RESOLUTION; 1920X1080", SW2, SH2, 2, &G_SETTINGS);
    G_CREATE_TEXTBUTTON("BACK", SW2, SH2+24, 3, &G_SETTINGS);

    G_CREATE_IMAGE(0, 0, 1.0f, 1.0f, &G_SETTINGS);

    G_SCENES->scenes[1] = &G_SETTINGS;
    G_GUI = *G_SCENES->scenes[G_GUI_SCENE];
}



void G_QUIT() {
    TTF_CloseFont(mainFont);
    TTF_Quit();
    for (int i = 0; i < G_GUI->txt_count; i++) {
        SDL_FreeSurface(G_GUI->text[i].txt_surface);
    }
    for (int i = 0; i < G_GUI->txtbtn_count; i++) {
        SDL_FreeSurface(G_GUI->buttons[i].txt_surface);
    }
    for (int i = 0; i < G_GUI->img_count; i++) {
        SDL_FreeSurface(G_GUI->image[i].img_surface);
    }
    free(G_GUI->text);
    free(G_GUI->buttons);
    free(G_GUI->image);
    free(G_GUI);
    free(G_SCENES);
}