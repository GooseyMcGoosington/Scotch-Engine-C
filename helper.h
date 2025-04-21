#ifndef helper
#define helper

#include <math.h>

#define PI 3.14159265359f
#define TAU PI*2
#define DEG2RAD(_d) ((_d) * (PI / 180.0f))
#define RAD2DEG(_d) ((_d) * (180.0f / PI))

#define RED 0xF800;
#define GREY 0x8080;
#define BLACK 0x0000;

//#define SW 1920
//#define SH 1080
//#define SW2 SW/2
//#define SH2 SH/2
//#define SW1 SW-1
//#define SH1 SH-1

static int SW  = 1920;
static int SH  = 1080;
static int SW1 = 1919;
static int SH1 = 1079;
static int SW2 = 960;
static int SH2 = 540;


#define TEXTURE_DETAIL_MODE 1 // This controls how detailed textures are.
#define DEPTH_SHADING 1 // This controls whether depth shading is enabled.
#define MAX_WALLS 512
#define MAX_TEXTURES 64
#define MAX_TEXTURESIZE (128 * 128)

inline int clamp(int val, int minVal, int maxVal) {
    return val < minVal ? minVal : (val > maxVal ? maxVal : val);
}

inline double to_degrees(double radians) {
    return radians * (180.0 / PI);
}

static inline void CFG_Init(int _SW, int _SH) {
    SW = _SW;
    SH = _SH;
    SW1 = _SW-1;
    SH1 = _SH-1;
    SW2 = _SW/2;
    SH2 = _SH/2;
};

#endif