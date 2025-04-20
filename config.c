#include "config.h"
#include "ini_parser.c"

static int SW = 1920;
static int SH = 1080;
static int SW1 = 1919;
static int SH1 = 1079;
static int SW2 = 960;
static int SH2 = 540;

static inline void CFG_INIT(void) {
    INI_PARSE("settings.ini");
    //SW = INI_FIND_VALUE("width");
    /*SH = INI_FIND_VALUE("height");

    SW2 = INI_FIND_VALUE("halfwidth");
    SH2 = INI_FIND_VALUE("halfheight");
    
    SW1 = INI_FIND_VALUE("subwidth");
    SH1 = INI_FIND_VALUE("subheight");*/
}
