#ifndef P_KEYBOARD
#define P_KEYBOARD

#include "p_keyboard.h"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

int control_locked = 0;
int8_t inputs[256];

void P_KEYBOARD_INIT() {
    printf("Keyboard initialised\n");
    for (int i = 0; i<256; i++) {
        inputs[i] = 0;
    }
}

void P_KEYBOARD_UPDATE() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        int a = (int)character.yaw;
        float pSn = sn[a];
        float pCs = cs[a];

        if (event.type == SDL_QUIT) {
            quit = 1;
        }
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_EXPOSED ||
                event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {

                surface = SDL_GetWindowSurface(window);
                SDL_BlitScaled(framebuffer, NULL, surface, &destRect);
                SDL_UpdateWindowSurface(window);
            }
        }
        if (event.type == SDL_KEYDOWN) {
            inputs[event.key.keysym.scancode] = 1;
            if (control_locked == 0) {
                if (event.key.keysym.sym == SDLK_UP) {
                    character.x += 1*pSn;
                    character.y += 1*pCs;
                }
                if (event.key.keysym.sym == SDLK_DOWN) {
                    character.x -= 1*pSn;
                    character.y -= 1*pCs;
                }
                if (event.key.keysym.sym == SDLK_e) {
                    // Interact
                    if (playerSector != NULL) {
                        wall *retWall = NULL;
                        float ix = 0;
                        float iy = 0;

                        S_RAYCAST(level, &retWall, &ix, &iy, playerSector, character.x, character.y, character.z, pSn, pCs, 32);
                        if (retWall != NULL) {
                            //retWall->tIndex = 3;
                        }
                    }
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    // Escape to Menu
                    control_locked = 1;
                    //G_CURRENT_SCENE = 0;
                }
            }
        }
        if (event.type == SDL_KEYUP) {
            inputs[event.key.keysym.scancode] = 0;
        }
        if (event.type == SDL_MOUSEMOTION && !control_locked) {
            character.pitch -= event.motion.yrel * MOUSE_SENSITIVITY_Y;
            character.pitch = CLAMP_F16(character.pitch, -MAX_CAMERA_PITCH, MAX_CAMERA_PITCH);

            character.yaw += event.motion.xrel * MOUSE_SENSITIVITY_X;
            if (character.yaw < 0) character.yaw = 360+character.yaw;
            if (character.yaw > 360) character.yaw = 0;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                int mX = event.button.x;
                int mY = event.button.y;
                if (control_locked) {
                    //G_GUI_MOUSE_CLICK(mX, mY);
                } else {
                    P_CHARACTER_DISPATCH_WEP();
                }
            }
        }
    }
    if (control_locked) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        SDL_ShowCursor(SDL_ENABLE);
    } else {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_ShowCursor(SDL_DISABLE);
    }
}

#endif