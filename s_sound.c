#ifndef S_SOUND
#define S_SOUND

#include "classes.h"
#include "helper.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <windows.h>

/*
This is the sound driver using SDL2's mixer.
This supports panning and volume attenuation!
Whatever you do, do NOT load 30MB sound files like I did once - it will balloon up to 4GB or more of memory usage.
*/

#define S_SOUND_CHANNELS_MAX 64
#define S_SOUND_RESERVED_CHANNELS 3
#define S_SOUND_WAV_MAX 3
#define S_SOUND_DIR "sounds\\*.*"

float S_SOUND_SIN = 0;
float S_SOUND_COS = 0;

unsigned int S_SOUND_CHANNELS[S_SOUND_CHANNELS_MAX];
SOUND_POINT S_SOUND_LOOPED_CHANNELS_REF[S_SOUND_CHANNELS_MAX];

WAV_FILE *S_SOUNDS_WAV[S_SOUND_WAV_MAX];

static inline void S_SOUND_FIND(WAV_FILE **ptr, char *F_NAME) {
    for (int s = 0; s < S_SOUND_WAV_MAX; s++) {
        if (S_SOUNDS_WAV[s] && strcmp(S_SOUNDS_WAV[s]->name, F_NAME) == 0) {
            *ptr = S_SOUNDS_WAV[s];
            return;
        }
    }
    *ptr = NULL;
}

static inline int S_SOUND_FIND_AVAILABLE_CHANNEL(int *channel) {
    for (int s=S_SOUND_RESERVED_CHANNELS+1; s<S_SOUND_CHANNELS_MAX; s++) {
        if (S_SOUND_CHANNELS[s] == 0) {
            *channel = s;
            return 1;
        }
    }
    return 0;
}

static inline void S_SOUND_CHANNEL_FINISHED(int channel) {
    if (channel >= 0 && channel < S_SOUND_CHANNELS_MAX) {
        S_SOUND_CHANNELS[channel] = 0;
    }
}

void S_SOUND_PLAY_INSTANTANEOUS(float x, float y, char *F_NAME) {
    int channel;
    int foundChannel = S_SOUND_FIND_AVAILABLE_CHANNEL(&channel);
    if (foundChannel) {
        WAV_FILE *SOUND = NULL;
        S_SOUND_FIND(&SOUND, F_NAME);
        if (SOUND) {
            S_SOUND_CHANNELS[channel] = 1;
            float dx = x;
            float dy = y;
            float pan = dx * S_SOUND_COS - dy * S_SOUND_SIN;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > 0.01f) {
                pan /= dist;
            } else {
                pan = 0.0f;
            }
            pan=clampf(pan, -1.0f, 1.0f);
            int left = (int)(255 * (1.0f - pan) * 0.5f);
            int right = (int)(255 * (1.0f + pan) * 0.5f);
            float volume=(1.0-(dist/80));
            volume=clampf(volume*MIX_MAX_VOLUME, 0.0f, MIX_MAX_VOLUME);

            Mix_Volume(channel, (int)volume);
            Mix_SetPanning(channel, left, right);
            Mix_PlayChannel(channel, SOUND->chunk, 0);
        }
    }
}

inline void S_SOUND_PLAY_MONO(char *F_NAME) {
    int channel;
    int foundChannel = S_SOUND_FIND_AVAILABLE_CHANNEL(&channel);
    if (foundChannel) {
        WAV_FILE *SOUND = NULL;
        S_SOUND_FIND(&SOUND, F_NAME);
        if (SOUND) {
            S_SOUND_CHANNELS[channel] = 1;

            Mix_Volume(channel, MIX_MAX_VOLUME);
            Mix_PlayChannel(channel, SOUND->chunk, 0);
        }
    }
}

inline void S_SOUND_HALT(int channel) {
    if (S_SOUND_LOOPED_CHANNELS_REF[channel].active == 1) {
        Mix_HaltChannel(channel);
        S_SOUND_CHANNEL_FINISHED(channel);
        S_SOUND_LOOPED_CHANNELS_REF[channel].active = 0;
    }
}

inline void S_SOUND_PLAY_LOOPED(float x, float y, char *F_NAME) {
    int channel;
    int foundChannel = S_SOUND_FIND_AVAILABLE_CHANNEL(&channel);
    if (foundChannel) {
        WAV_FILE *SOUND = NULL;
        S_SOUND_FIND(&SOUND, F_NAME);
        if (SOUND) {
            S_SOUND_CHANNELS[channel] = 1;
            S_SOUND_LOOPED_CHANNELS_REF[channel].x=x;
            S_SOUND_LOOPED_CHANNELS_REF[channel].y=y;
            S_SOUND_LOOPED_CHANNELS_REF[channel].active = 1;
            Mix_PlayChannel(channel, SOUND->chunk, -1);
        }
    }
}


static void S_INIT_SOUND() {
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    Mix_AllocateChannels(S_SOUND_CHANNELS_MAX);
    Mix_ReserveChannels(S_SOUND_RESERVED_CHANNELS);

    WIN32_FIND_DATA F_DATA;
    HANDLE H_HANDLE = FindFirstFile(S_SOUND_DIR, &F_DATA);

    if (H_HANDLE == INVALID_HANDLE_VALUE) {
        return;
    }

    unsigned int f = 0;
    do {
        if (F_DATA.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        char F_PATH[MAX_PATH];
        snprintf(F_PATH, sizeof(F_PATH), "sounds\\%s", F_DATA.cFileName);

        S_SOUNDS_WAV[f] = malloc(sizeof(WAV_FILE));
        S_SOUNDS_WAV[f]->chunk = Mix_LoadWAV(F_PATH);
        strncpy(S_SOUNDS_WAV[f]->name, F_DATA.cFileName, sizeof(S_SOUNDS_WAV[f]->name)-1);
        S_SOUNDS_WAV[f]->name[sizeof(S_SOUNDS_WAV[f]->name) - 1] = '\0';

        if (++f >= S_SOUND_WAV_MAX) break;
    } while (FindNextFile(H_HANDLE, &F_DATA));

    Mix_ChannelFinished(S_SOUND_CHANNEL_FINISHED);
    FindClose(H_HANDLE);
}

static void S_SOUND_UPDATE(player character, float pCs, float pSn) {
    S_SOUND_SIN = pSn;
    S_SOUND_COS = pCs;

    for (int s=0; s<S_SOUND_CHANNELS_MAX; s++) {
        if (S_SOUND_LOOPED_CHANNELS_REF[s].active == 1) {
            float dx = S_SOUND_LOOPED_CHANNELS_REF[s].x-character.x;
            float dy = S_SOUND_LOOPED_CHANNELS_REF[s].y-character.y;
            float pan = dx * S_SOUND_COS - dy * S_SOUND_SIN;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > 0.01f) {
                pan /= dist;
            } else {
                pan = 0.0f;
            }
            pan=clampf(pan, -1.0f, 1.0f);
            int left = (int)(255 * (1.0f - pan) * 0.5f);
            int right = (int)(255 * (1.0f + pan) * 0.5f);
            float volume=(1.0-(dist/80));
            volume=clampf(volume*MIX_MAX_VOLUME, 0.0f, MIX_MAX_VOLUME);

            Mix_Volume(s, (int)volume);
            Mix_SetPanning(s, left, right);
        }
    }
}
#endif