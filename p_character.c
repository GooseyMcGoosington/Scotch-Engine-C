#ifndef P_CHARACTER
#define P_CHARACTER

#include "p_character.h"

player character;
static struct timeval begin, end;
static int fI = 0;
static double fAvg = 0;

static unsigned int WEP_DELAY = 0;
static unsigned int WEP_STATE = 0;

static inline float RANDOM_SPREAD(float spread) {
    return ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * spread;
}

void P_CHARACTER_DISPATCH_WEP() {
    int yaw = character.yaw+RANDOM_SPREAD(1);
    float pSn = sn[yaw];
    float pCs = cs[yaw];
    switch(character.stats.wep) {
        case 0:
        {   
            /*switch(WEP_STATE) {
                case 0:
                {
                    WEP_STATE = 1;
                    WEP_DELAY = 5;
                    break;
                }
                case 1:
                {
                    WEP_DELAY --;
                    if (WEP_DELAY<=0) {
                        WEP_STATE = 0;
                    } else {
                        return;
                    }
                }
            }*/
            float ix = 0;
            float iy = 0;
            wall *retWall = NULL;
            S_SOUND_PLAY_MONO("sgn.wav");
            S_RAYCAST(level, &retWall, &ix, &iy, playerSector, character.x, character.y, character.z, pSn, pCs, 1024);
            if (retWall != NULL) {
                // Raycast was successful
                printf("Hit wall\n");
                // Let's create a smoke particle
                entity *newEntity = S_ADD_ENTITY(level->sectors[0], ix, iy, 10, 0);
                newEntity->h = character.z;

                int index = amnt_entities-1;
                ai_entity *newAI = S_ADD_AI(newEntity, 1, index); // Because we incremented amnt_entities, it is that with an offset of -1. We give it the behaviour of a rocket.
                
                S_ADD_DEBRIS(newAI, 10);
            }

            break;
        }
    }
}

void P_CHARACTER_INIT(float x, float y, float yaw, float fov, float radius) {
    character = (player){
        x, 
        y, 
        0.0, 
        5.0, 
        yaw, 
        -15.0, 
        70.0, 
        fov, 
        0.0, 
        radius, 
        (playerInfo){{0, 100, 100, 0}}
    };
    float f = DEG2RAD(character.fov);
    float tanFOV = tan(f/2);
    float focalLength = SW2/tanFOV;
    character.focalLength = focalLength;
    character.fovWidth = tanFOV;
}

void P_CHARACTER_RECORD_FRAMETIME() {
    gettimeofday(&begin, NULL);
    double time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1000000.0;
    time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1000000.0;
    fI++;
    fAvg += time_spent;
    
    if (fI == 15) {
        char str[10];
        fAvg /= fI;
        sprintf(str, "%.4lf", (double)(1/fAvg));
        SDL_SetWindowTitle(window, str);
        fI = 0;
        fAvg = 0;
        //S_SOUND_PLAY_INSTANTANEOUS(entities[1].x-character.x, entities[1].y-character.y, "sgn.wav");
    } 
}

int P_CHARACTER_UPDATE_SECTOR(unsigned int *cS) {
    int characterSector = S_CHAR_IN_CONVEX_BOUNDS(character);
    if ((characterSector > -1) & (characterSector <= level->count)) {
        playerSector = level->sectors[characterSector];
        character.z = playerSector->elevation;
        *cS = characterSector;
        return 1;
    }
    return 0;
}
#endif