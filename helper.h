#ifndef helper
#define helper

#include <math.h>
#include <stdint.h>

#define PI 3.14159265359f
#define PI2 (PI*2)
#define TAU PI*2
#define DEG2RAD(_d) ((_d) * (PI / 180.0f))
#define RAD2DEG(_d) ((_d) * (180.0f / PI))

#define RED 0xF800;
#define GREY 0x8080;
#define BLACK 0x0000;
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

#define MOUSE_SENSITIVITY_Y 0.001f
#define MOUSE_SENSITIVITY_X 0.08f

#define MAX_CAMERA_PITCH (PI/2 - 0.01f)

static int SW  = 1920;
static int SH  = 1080;
static int SW1 = 1919;
static int SH1 = 1079;
static int SW2 = 960;
static int SH2 = 540;

#define MAX_WALLS 256
#define MAX_TEXTURES 255
#define MAX_TEXTURESIZE (128 * 128)

inline int clamp(int val, int minVal, int maxVal) {
    return val < minVal ? minVal : (val > maxVal ? maxVal : val);
}

inline _Float16 CLAMP_F16(_Float16 val, _Float16 minVal, _Float16 maxVal) {
    return val < minVal ? minVal : (val > maxVal ? maxVal : val);
}


inline float fast_sqrtf(float x) {
    if (x == 0.0f) return 0.0f;

    union {
        float f;
        uint32_t i;
    } u;

    u.f = x;
    u.i = (u.i >> 1) + 0x1FC00000;  // Magic number for sqrt approximation
    return u.f;
}

inline double to_degrees(double radians) {
    return radians * (180.0 / PI);
}

const int resolutions[10] = {
    1920, 1080,  // 1080p
    1024, 768,   // XGA
    800, 600,    // SVGA
    640, 480,    // VGA
};

static int resolutionSet = 0;
static int tick = 0;

static inline void CFG_Init(int x) {
    resolutionSet = x;
    int X = x*2+0;
    int Y = x*2+1;

    int _SW = resolutions[X];
    int _SH = resolutions[Y];
    SW = _SW;
    SH = _SH;
    SW1 = _SW-1;
    SH1 = _SH-1;
    SW2 = _SW/2;
    SH2 = _SH/2;
};

#endif