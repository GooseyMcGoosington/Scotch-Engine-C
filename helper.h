#ifndef helper
#define helper

#include <math.h>

#define PI 3.14159265359
#define TAU PI*2
#define DEG2RAD(_d) ((_d) * (PI / 180.0f))
#define RAD2DEG(_d) ((_d) * (180.0f / PI))

#define RED 0xF800;
#define GREY 0x8080;

#define SW 1920
#define SH 1080
#define SW2 SW/2
#define SH2 SH/2
#define SW1 SW-1
#define SH1 SH-1
#define TEXTURE_DETAIL_MODE 2 // This controls how detailed textures are.
#define DEPTH_SHADING 1 // This controls whether depth shading is enabled.

inline double to_degrees(double radians) {
    return radians * (180.0 / PI);
}

#endif