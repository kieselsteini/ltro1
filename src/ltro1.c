/*
================================================================================

    LTRO-1 fantasy game console (Lospec Game Jam 2021)
    written by Sebastian Steinhauer <s.steinhauer@yahoo.de>

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    For more information, please refer to <http://unlicense.org/>

================================================================================
*/
/*
================================================================================

        INCLUDES

================================================================================
*/
/*----------------------------------------------------------------------------*/
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/fetch.h>
#endif


/*----------------------------------------------------------------------------*/
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/*----------------------------------------------------------------------------*/
#include "SDL.h"


/*
================================================================================

        DEFINES / TYPES

================================================================================
*/
/*----------------------------------------------------------------------------*/
#define LTRO_VERSION        "0.4.0"
#define LTRO_AUTHOR         "Sebastian Steinhauer <s.steinhauer@yahoo.de>"


/*----------------------------------------------------------------------------*/
#define SCREEN_WIDTH        240
#define SCREEN_HEIGHT       135
#define SCREEN_PADDING      64
#define SCREEN_TITLE        "LTRO-1 (Lospec game console)"


/*----------------------------------------------------------------------------*/
enum {
    BUTTON_UP               = 1 << 0,
    BUTTON_DOWN             = 1 << 1,
    BUTTON_LEFT             = 1 << 2,
    BUTTON_RIGHT            = 1 << 3,
    BUTTON_A                = 1 << 4,
    BUTTON_B                = 1 << 5,
    BUTTON_START            = 1 << 6
};


/*----------------------------------------------------------------------------*/
#define FPS_TICKS           (1000.0 / 60.0)


/*----------------------------------------------------------------------------*/
#define AUDIO_FREQUENCY     44100
#define AUDIO_VOICES        2

enum { PSG_50, PSG_25, PSG_12 };

typedef struct audio_voice_t {
    char                    song[4096], *mml;
    int                     psg;
    float                   octave, tempo, length;
    int                     ttl, t0, t1, t2;
    float                   e0, e1;
} audio_voice_t;


/*
================================================================================

        STATIC DATA

================================================================================
*/
/*----------------------------------------------------------------------------*/
static const SDL_Color      palette[10] = {
    { 0xea, 0xe1, 0xf0, 0xff }, /* 0 - #eae1f0 */
    { 0x7e, 0x71, 0x85, 0xff }, /* 1 - #7e7185 */
    { 0x37, 0x31, 0x3b, 0xff }, /* 2 - #37313b */
    { 0x1d, 0x1c, 0x1f, 0xff }, /* 3 - #1d1c1f */
    { 0x89, 0x42, 0x3f, 0xff }, /* 4 - #89423f */
    { 0xf6, 0x3f, 0x4c, 0xff }, /* 5 - #f63f4c */
    { 0xfd, 0xbb, 0x27, 0xff }, /* 6 - #fdbb27 */
    { 0x8d, 0x90, 0x2e, 0xff }, /* 7 - #8d902e */
    { 0x41, 0x59, 0xcb, 0xff }, /* 8 - #4159cb */
    { 0x59, 0xa7, 0xaf, 0xff }  /* 9 - #59a7af */
};


/*----------------------------------------------------------------------------*/
static const Uint8          font8x8[256 * 8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x81, 0xa5, 0x81, 0xbd, 0x99, 0x81, 0x7e,
    0x7e, 0xff, 0xdb, 0xff, 0xc3, 0xe7, 0xff, 0x7e, 0x36, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c, 0x08, 0x00,
    0x08, 0x1c, 0x3e, 0x7f, 0x3e, 0x1c, 0x08, 0x00, 0x1c, 0x3e, 0x1c, 0x7f, 0x7f, 0x6b, 0x08, 0x1c,
    0x08, 0x08, 0x1c, 0x3e, 0x7f, 0x3e, 0x08, 0x1c, 0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00,
    0xff, 0xff, 0xe7, 0xc3, 0xc3, 0xe7, 0xff, 0xff, 0x00, 0x3c, 0x66, 0x42, 0x42, 0x66, 0x3c, 0x00,
    0xff, 0xc3, 0x99, 0xbd, 0xbd, 0x99, 0xc3, 0xff, 0xf0, 0xe0, 0xf0, 0xbe, 0x33, 0x33, 0x33, 0x1e,
    0x3c, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x7e, 0x18, 0xfc, 0xcc, 0xfc, 0x0c, 0x0c, 0x0e, 0x0f, 0x07,
    0xfe, 0xc6, 0xfe, 0xc6, 0xc6, 0xe6, 0x67, 0x03, 0x18, 0xdb, 0x3c, 0xe7, 0xe7, 0x3c, 0xdb, 0x18,
    0x01, 0x07, 0x1f, 0x7f, 0x1f, 0x07, 0x01, 0x00, 0x40, 0x70, 0x7c, 0x7f, 0x7c, 0x70, 0x40, 0x00,
    0x18, 0x3c, 0x7e, 0x18, 0x18, 0x7e, 0x3c, 0x18, 0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00,
    0xfe, 0xdb, 0xdb, 0xde, 0xd8, 0xd8, 0xd8, 0x00, 0x7c, 0xc6, 0x1c, 0x36, 0x36, 0x1c, 0x33, 0x1e,
    0x00, 0x00, 0x00, 0x00, 0x7e, 0x7e, 0x7e, 0x00, 0x18, 0x3c, 0x7e, 0x18, 0x7e, 0x3c, 0x18, 0xff,
    0x18, 0x3c, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x3c, 0x18, 0x00,
    0x00, 0x18, 0x30, 0x7f, 0x30, 0x18, 0x00, 0x00, 0x00, 0x0c, 0x06, 0x7f, 0x06, 0x0c, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x03, 0x03, 0x7f, 0x00, 0x00, 0x00, 0x24, 0x66, 0xff, 0x66, 0x24, 0x00, 0x00,
    0x00, 0x18, 0x3c, 0x7e, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x1e, 0x1e, 0x0c, 0x0c, 0x00, 0x0c, 0x00,
    0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x36, 0x7f, 0x36, 0x7f, 0x36, 0x36, 0x00,
    0x0c, 0x3e, 0x03, 0x1e, 0x30, 0x1f, 0x0c, 0x00, 0x00, 0x63, 0x33, 0x18, 0x0c, 0x66, 0x63, 0x00,
    0x1c, 0x36, 0x1c, 0x6e, 0x3b, 0x33, 0x6e, 0x00, 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x0c, 0x06, 0x06, 0x06, 0x0c, 0x18, 0x00, 0x06, 0x0c, 0x18, 0x18, 0x18, 0x0c, 0x06, 0x00,
    0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x3f, 0x0c, 0x0c, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x06, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x01, 0x00,
    0x3e, 0x63, 0x73, 0x7b, 0x6f, 0x67, 0x3e, 0x00, 0x0c, 0x0e, 0x0c, 0x0c, 0x0c, 0x0c, 0x3f, 0x00,
    0x1e, 0x33, 0x30, 0x1c, 0x06, 0x33, 0x3f, 0x00, 0x1e, 0x33, 0x30, 0x1c, 0x30, 0x33, 0x1e, 0x00,
    0x38, 0x3c, 0x36, 0x33, 0x7f, 0x30, 0x78, 0x00, 0x3f, 0x03, 0x1f, 0x30, 0x30, 0x33, 0x1e, 0x00,
    0x1c, 0x06, 0x03, 0x1f, 0x33, 0x33, 0x1e, 0x00, 0x3f, 0x33, 0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x00,
    0x1e, 0x33, 0x33, 0x1e, 0x33, 0x33, 0x1e, 0x00, 0x1e, 0x33, 0x33, 0x3e, 0x30, 0x18, 0x0e, 0x00,
    0x00, 0x0c, 0x0c, 0x00, 0x00, 0x0c, 0x0c, 0x00, 0x00, 0x0c, 0x0c, 0x00, 0x00, 0x0c, 0x0c, 0x06,
    0x18, 0x0c, 0x06, 0x03, 0x06, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x3f, 0x00, 0x00,
    0x06, 0x0c, 0x18, 0x30, 0x18, 0x0c, 0x06, 0x00, 0x1e, 0x33, 0x30, 0x18, 0x0c, 0x00, 0x0c, 0x00,
    0x3e, 0x63, 0x7b, 0x7b, 0x7b, 0x03, 0x1e, 0x00, 0x0c, 0x1e, 0x33, 0x33, 0x3f, 0x33, 0x33, 0x00,
    0x3f, 0x66, 0x66, 0x3e, 0x66, 0x66, 0x3f, 0x00, 0x3c, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3c, 0x00,
    0x1f, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1f, 0x00, 0x7f, 0x46, 0x16, 0x1e, 0x16, 0x46, 0x7f, 0x00,
    0x7f, 0x46, 0x16, 0x1e, 0x16, 0x06, 0x0f, 0x00, 0x3c, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7c, 0x00,
    0x33, 0x33, 0x33, 0x3f, 0x33, 0x33, 0x33, 0x00, 0x1e, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x1e, 0x00,
    0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1e, 0x00, 0x67, 0x66, 0x36, 0x1e, 0x36, 0x66, 0x67, 0x00,
    0x0f, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7f, 0x00, 0x63, 0x77, 0x7f, 0x7f, 0x6b, 0x63, 0x63, 0x00,
    0x63, 0x67, 0x6f, 0x7b, 0x73, 0x63, 0x63, 0x00, 0x1c, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1c, 0x00,
    0x3f, 0x66, 0x66, 0x3e, 0x06, 0x06, 0x0f, 0x00, 0x1e, 0x33, 0x33, 0x33, 0x3b, 0x1e, 0x38, 0x00,
    0x3f, 0x66, 0x66, 0x3e, 0x36, 0x66, 0x67, 0x00, 0x1e, 0x33, 0x06, 0x0c, 0x18, 0x33, 0x1e, 0x00,
    0x3f, 0x2d, 0x0c, 0x0c, 0x0c, 0x0c, 0x1e, 0x00, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3f, 0x00,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x1e, 0x0c, 0x00, 0x63, 0x63, 0x63, 0x6b, 0x7f, 0x77, 0x63, 0x00,
    0x63, 0x63, 0x36, 0x1c, 0x1c, 0x36, 0x63, 0x00, 0x33, 0x33, 0x33, 0x1e, 0x0c, 0x0c, 0x1e, 0x00,
    0x7f, 0x63, 0x31, 0x18, 0x4c, 0x66, 0x7f, 0x00, 0x1e, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1e, 0x00,
    0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x40, 0x00, 0x1e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1e, 0x00,
    0x08, 0x1c, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x0c, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x30, 0x3e, 0x33, 0x6e, 0x00,
    0x07, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x3b, 0x00, 0x00, 0x00, 0x1e, 0x33, 0x03, 0x33, 0x1e, 0x00,
    0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6e, 0x00, 0x00, 0x00, 0x1e, 0x33, 0x3f, 0x03, 0x1e, 0x00,
    0x1c, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0f, 0x00, 0x00, 0x00, 0x6e, 0x33, 0x33, 0x3e, 0x30, 0x1f,
    0x07, 0x06, 0x36, 0x6e, 0x66, 0x66, 0x67, 0x00, 0x0c, 0x00, 0x0e, 0x0c, 0x0c, 0x0c, 0x1e, 0x00,
    0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1e, 0x07, 0x06, 0x66, 0x36, 0x1e, 0x36, 0x67, 0x00,
    0x0e, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x1e, 0x00, 0x00, 0x00, 0x33, 0x7f, 0x7f, 0x6b, 0x63, 0x00,
    0x00, 0x00, 0x1f, 0x33, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x1e, 0x33, 0x33, 0x33, 0x1e, 0x00,
    0x00, 0x00, 0x3b, 0x66, 0x66, 0x3e, 0x06, 0x0f, 0x00, 0x00, 0x6e, 0x33, 0x33, 0x3e, 0x30, 0x78,
    0x00, 0x00, 0x3b, 0x6e, 0x66, 0x06, 0x0f, 0x00, 0x00, 0x00, 0x3e, 0x03, 0x1e, 0x30, 0x1f, 0x00,
    0x08, 0x0c, 0x3e, 0x0c, 0x0c, 0x2c, 0x18, 0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6e, 0x00,
    0x00, 0x00, 0x33, 0x33, 0x33, 0x1e, 0x0c, 0x00, 0x00, 0x00, 0x63, 0x6b, 0x7f, 0x7f, 0x36, 0x00,
    0x00, 0x00, 0x63, 0x36, 0x1c, 0x36, 0x63, 0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x3e, 0x30, 0x1f,
    0x00, 0x00, 0x3f, 0x19, 0x0c, 0x26, 0x3f, 0x00, 0x38, 0x0c, 0x0c, 0x07, 0x0c, 0x0c, 0x38, 0x00,
    0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00, 0x07, 0x0c, 0x0c, 0x38, 0x0c, 0x0c, 0x07, 0x00,
    0x6e, 0x3b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x1c, 0x36, 0x63, 0x63, 0x7f, 0x00,
    0x1e, 0x33, 0x03, 0x33, 0x1e, 0x18, 0x30, 0x1e, 0x00, 0x33, 0x00, 0x33, 0x33, 0x33, 0x7e, 0x00,
    0x38, 0x00, 0x1e, 0x33, 0x3f, 0x03, 0x1e, 0x00, 0x7e, 0xc3, 0x3c, 0x60, 0x7c, 0x66, 0xfc, 0x00,
    0x33, 0x00, 0x1e, 0x30, 0x3e, 0x33, 0x7e, 0x00, 0x07, 0x00, 0x1e, 0x30, 0x3e, 0x33, 0x7e, 0x00,
    0x0c, 0x0c, 0x1e, 0x30, 0x3e, 0x33, 0x7e, 0x00, 0x00, 0x00, 0x1e, 0x03, 0x03, 0x1e, 0x30, 0x1c,
    0x7e, 0xc3, 0x3c, 0x66, 0x7e, 0x06, 0x3c, 0x00, 0x33, 0x00, 0x1e, 0x33, 0x3f, 0x03, 0x1e, 0x00,
    0x07, 0x00, 0x1e, 0x33, 0x3f, 0x03, 0x1e, 0x00, 0x33, 0x00, 0x0e, 0x0c, 0x0c, 0x0c, 0x1e, 0x00,
    0x3e, 0x63, 0x1c, 0x18, 0x18, 0x18, 0x3c, 0x00, 0x07, 0x00, 0x0e, 0x0c, 0x0c, 0x0c, 0x1e, 0x00,
    0x63, 0x1c, 0x36, 0x63, 0x7f, 0x63, 0x63, 0x00, 0x0c, 0x0c, 0x00, 0x1e, 0x33, 0x3f, 0x33, 0x00,
    0x38, 0x00, 0x3f, 0x06, 0x1e, 0x06, 0x3f, 0x00, 0x00, 0x00, 0xfe, 0x30, 0xfe, 0x33, 0xfe, 0x00,
    0x7c, 0x36, 0x33, 0x7f, 0x33, 0x33, 0x73, 0x00, 0x1e, 0x33, 0x00, 0x1e, 0x33, 0x33, 0x1e, 0x00,
    0x00, 0x33, 0x00, 0x1e, 0x33, 0x33, 0x1e, 0x00, 0x00, 0x07, 0x00, 0x1e, 0x33, 0x33, 0x1e, 0x00,
    0x1e, 0x33, 0x00, 0x33, 0x33, 0x33, 0x7e, 0x00, 0x00, 0x07, 0x00, 0x33, 0x33, 0x33, 0x7e, 0x00,
    0x00, 0x33, 0x00, 0x33, 0x33, 0x3e, 0x30, 0x1f, 0xc3, 0x18, 0x3c, 0x66, 0x66, 0x3c, 0x18, 0x00,
    0x33, 0x00, 0x33, 0x33, 0x33, 0x33, 0x1e, 0x00, 0x18, 0x18, 0x7e, 0x03, 0x03, 0x7e, 0x18, 0x18,
    0x1c, 0x36, 0x26, 0x0f, 0x06, 0x67, 0x3f, 0x00, 0x33, 0x33, 0x1e, 0x3f, 0x0c, 0x3f, 0x0c, 0x0c,
    0x1f, 0x33, 0x33, 0x5f, 0x63, 0xf3, 0x63, 0xe3, 0x70, 0xd8, 0x18, 0x3c, 0x18, 0x18, 0x1b, 0x0e,
    0x38, 0x00, 0x1e, 0x30, 0x3e, 0x33, 0x7e, 0x00, 0x1c, 0x00, 0x0e, 0x0c, 0x0c, 0x0c, 0x1e, 0x00,
    0x00, 0x38, 0x00, 0x1e, 0x33, 0x33, 0x1e, 0x00, 0x00, 0x38, 0x00, 0x33, 0x33, 0x33, 0x7e, 0x00,
    0x00, 0x1f, 0x00, 0x1f, 0x33, 0x33, 0x33, 0x00, 0x3f, 0x00, 0x33, 0x37, 0x3f, 0x3b, 0x33, 0x00,
    0x3c, 0x36, 0x36, 0x7c, 0x00, 0x7e, 0x00, 0x00, 0x1c, 0x36, 0x36, 0x1c, 0x00, 0x3e, 0x00, 0x00,
    0x0c, 0x00, 0x0c, 0x06, 0x03, 0x33, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x03, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3f, 0x30, 0x30, 0x00, 0x00, 0xc3, 0x63, 0x33, 0x7b, 0xcc, 0x66, 0x33, 0xf0,
    0xc3, 0x63, 0x33, 0xdb, 0xec, 0xf6, 0xf3, 0xc0, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00,
    0x00, 0xcc, 0x66, 0x33, 0x66, 0xcc, 0x00, 0x00, 0x00, 0x33, 0x66, 0xcc, 0x66, 0x33, 0x00, 0x00,
    0x44, 0x11, 0x44, 0x11, 0x44, 0x11, 0x44, 0x11, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55,
    0xdb, 0xee, 0xdb, 0x77, 0xdb, 0xee, 0xdb, 0x77, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x18, 0x1f, 0x18, 0x18, 0x18,
    0x6c, 0x6c, 0x6c, 0x6c, 0x6f, 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x6c, 0x6c, 0x6c,
    0x00, 0x00, 0x1f, 0x18, 0x1f, 0x18, 0x18, 0x18, 0x6c, 0x6c, 0x6f, 0x60, 0x6f, 0x6c, 0x6c, 0x6c,
    0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x7f, 0x60, 0x6f, 0x6c, 0x6c, 0x6c,
    0x6c, 0x6c, 0x6f, 0x60, 0x7f, 0x00, 0x00, 0x00, 0x6c, 0x6c, 0x6c, 0x6c, 0x7f, 0x00, 0x00, 0x00,
    0x18, 0x18, 0x1f, 0x18, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0xf8, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0xff, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xf8, 0x18, 0x18, 0x18,
    0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0xff, 0x18, 0x18, 0x18,
    0x18, 0x18, 0xf8, 0x18, 0xf8, 0x18, 0x18, 0x18, 0x6c, 0x6c, 0x6c, 0x6c, 0xec, 0x6c, 0x6c, 0x6c,
    0x6c, 0x6c, 0xec, 0x0c, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x0c, 0xec, 0x6c, 0x6c, 0x6c,
    0x6c, 0x6c, 0xef, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xef, 0x6c, 0x6c, 0x6c,
    0x6c, 0x6c, 0xec, 0x0c, 0xec, 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00,
    0x6c, 0x6c, 0xef, 0x00, 0xef, 0x6c, 0x6c, 0x6c, 0x18, 0x18, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00,
    0x6c, 0x6c, 0x6c, 0x6c, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x18, 0x18, 0x18,
    0x00, 0x00, 0x00, 0x00, 0xff, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0xfc, 0x00, 0x00, 0x00,
    0x18, 0x18, 0xf8, 0x18, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x18, 0xf8, 0x18, 0x18, 0x18,
    0x00, 0x00, 0x00, 0x00, 0xfc, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0xff, 0x6c, 0x6c, 0x6c,
    0x18, 0x18, 0xff, 0x18, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xf8, 0x18, 0x18, 0x18, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x6e, 0x3b, 0x13, 0x3b, 0x6e, 0x00, 0x00, 0x1e, 0x33, 0x1f, 0x33, 0x1f, 0x03, 0x03,
    0x00, 0x3f, 0x33, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x7f, 0x36, 0x36, 0x36, 0x36, 0x36, 0x00,
    0x3f, 0x33, 0x06, 0x0c, 0x06, 0x33, 0x3f, 0x00, 0x00, 0x00, 0x7e, 0x1b, 0x1b, 0x1b, 0x0e, 0x00,
    0x00, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x06, 0x03, 0x00, 0x6e, 0x3b, 0x18, 0x18, 0x18, 0x18, 0x00,
    0x3f, 0x0c, 0x1e, 0x33, 0x33, 0x1e, 0x0c, 0x3f, 0x1c, 0x36, 0x63, 0x7f, 0x63, 0x36, 0x1c, 0x00,
    0x1c, 0x36, 0x63, 0x63, 0x36, 0x36, 0x77, 0x00, 0x38, 0x0c, 0x18, 0x3e, 0x33, 0x33, 0x1e, 0x00,
    0x00, 0x00, 0x7e, 0xdb, 0xdb, 0x7e, 0x00, 0x00, 0x60, 0x30, 0x7e, 0xdb, 0xdb, 0x7e, 0x06, 0x03,
    0x1c, 0x06, 0x03, 0x1f, 0x03, 0x06, 0x1c, 0x00, 0x1e, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x00,
    0x00, 0x3f, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0x00, 0x0c, 0x0c, 0x3f, 0x0c, 0x0c, 0x00, 0x3f, 0x00,
    0x06, 0x0c, 0x18, 0x0c, 0x06, 0x00, 0x3f, 0x00, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x00, 0x3f, 0x00,
    0x70, 0xd8, 0xd8, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1b, 0x1b, 0x0e,
    0x0c, 0x0c, 0x00, 0x3f, 0x00, 0x0c, 0x0c, 0x00, 0x00, 0x6e, 0x3b, 0x00, 0x6e, 0x3b, 0x00, 0x00,
    0x1c, 0x36, 0x36, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0xf0, 0x30, 0x30, 0x30, 0x37, 0x36, 0x3c, 0x38,
    0x1e, 0x36, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x0e, 0x18, 0x0c, 0x06, 0x1e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x3c, 0x3c, 0x3c, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


/*----------------------------------------------------------------------------*/
static const float          frequencies[88] = {
      27.500f,   29.135f,   30.868f,   32.703f,   34.648f,   36.708f,   38.891f,   41.203f,
      43.654f,   46.249f,   48.999f,   51.913f,   55.000f,   58.270f,   61.735f,   65.406f,
      69.296f,   73.416f,   77.782f,   82.407f,   87.307f,   92.499f,   97.999f,  103.826f,
     110.000f,  116.541f,  123.471f,  130.813f,  138.591f,  146.832f,  155.563f,  164.814f,
     174.614f,  184.997f,  195.998f,  207.652f,  220.000f,  233.082f,  246.942f,  261.626f,
     277.183f,  293.665f,  311.127f,  329.628f,  349.228f,  369.994f,  391.995f,  415.305f,
     440.000f,  466.164f,  493.883f,  523.251f,  554.365f,  587.330f,  622.254f,  659.255f,
     698.456f,  739.989f,  783.991f,  830.609f,  880.000f,  932.328f,  987.767f, 1046.502f,
    1108.731f, 1174.659f, 1244.508f, 1318.510f, 1396.913f, 1479.978f, 1567.982f, 1661.219f,
    1760.000f, 1864.655f, 1975.533f, 2093.005f, 2217.461f, 2349.318f, 2489.016f, 2637.020f,
    2793.826f, 2959.955f, 3135.963f, 3322.438f, 3520.000f, 3729.310f, 3951.066f, 4186.009f
};


/*----------------------------------------------------------------------------*/
static const int            pixeldecoder[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


/*
================================================================================

        GLOBAL VARIABLES

================================================================================
*/
/*----------------------------------------------------------------------------*/
static int                  running = -1;
static Uint8                btn_down;
static Uint8                btn_pressed;
static int                  clear_color = 0;


/*----------------------------------------------------------------------------*/
static SDL_Window           *window = NULL;
static SDL_Renderer         *renderer = NULL;
static SDL_Texture          *texture = NULL;
static SDL_Surface          *surface32 = NULL;
static SDL_Surface          *surface8 = NULL;


/*----------------------------------------------------------------------------*/
static SDL_AudioDeviceID    audio_device = 0;
static float                audio_gain = 1.0f;
static float                audio_frequency;
static audio_voice_t        audio_voices[AUDIO_VOICES];


/*
================================================================================

        HELPER FUNCTIONS

================================================================================
*/
/*----------------------------------------------------------------------------*/
#define minimum(a, b)       ((a) < (b) ? (a) : (b))
#define maximum(a, b)       ((a) > (b) ? (a) : (b))
#define clamp(x, min, max)  maximum(minimum(x, max), min)


/*----------------------------------------------------------------------------*/
static int push_callback(lua_State *L, const char *name) {
    if (lua_getfield(L, LUA_REGISTRYINDEX, "ltro_callbacks") != LUA_TTABLE) {
        lua_pop(L, 1);
        return 0;
    }
    if (lua_getfield(L, -1, name) != LUA_TFUNCTION) {
        lua_pop(L, 2);
        return 0;
    }
    lua_remove(L, -2);
    return 1;
}


/*----------------------------------------------------------------------------*/
static Uint8 check_button(lua_State *L, const int n) {
    static const char       *names[] = { "up", "down", "left", "right", "a", "b", "start", NULL };
    static const Uint8      masks[] = { BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT, BUTTON_A, BUTTON_B, BUTTON_START };
    return masks[luaL_checkoption(L, n, NULL, names)];
}


/*----------------------------------------------------------------------------*/
static audio_voice_t* check_voice(lua_State *L, const int n) {
    int                     i = (int)luaL_checkinteger(L, n);
    luaL_argcheck(L, i >= 1 && i <= AUDIO_VOICES, n, "invalid audio voice");
    return &audio_voices[i - 1];
}


/*----------------------------------------------------------------------------*/
static void render_screen(lua_State *L) {
    const SDL_Color         *color = &palette[clear_color];

    if (SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, 255))
        luaL_error(L, "SDL_SetRenderDrawColor() failed: %s", SDL_GetError());
    if (SDL_RenderClear(renderer))
        luaL_error(L, "SDL_RenderClear() failed: %s", SDL_GetError());
    if (SDL_BlitSurface(surface8, NULL, surface32, NULL))
        luaL_error(L, "SDL_BlitSurface() failed: %s", SDL_GetError());
    if (SDL_UpdateTexture(texture, NULL, surface32->pixels, surface32->pitch))
        luaL_error(L, "SDL_UpdateTexture() failed: %s", SDL_GetError());
    if (SDL_RenderCopy(renderer, texture, NULL, NULL))
        luaL_error(L, "SDL_RenderCopy() failed: %s", SDL_GetError());
    SDL_RenderPresent(renderer);
}


/*----------------------------------------------------------------------------*/
static void draw_pixel(int x, int y, Uint8 color) {
    if ((surface8 != NULL) && (x >= 0) && (x < surface8->w) && (y >= 0) && (y < surface8->h)) {
        ((Uint8*)surface8->pixels)[surface8->pitch * y + x] = color;
    }
}


/*----------------------------------------------------------------------------*/
static int mml_parse_number(audio_voice_t *voice) {
    int                     value = 0;

    for (; voice->mml && *voice->mml; ++voice->mml) {
        if (*voice->mml >= '0' && *voice->mml <= '9') {
            value = (value * 10) + (*voice->mml - '0');
        } else {
            break;
        }
    }

    return value;
}


/*----------------------------------------------------------------------------*/
static void mml_parse_mode(audio_voice_t *voice) {
    if (!*voice->mml) return;
    switch (*voice->mml) {
        case '5': voice->psg = PSG_50; break;
        case '2': voice->psg = PSG_25; break;
        case '1': voice->psg = PSG_12; break;
    }
}


/*----------------------------------------------------------------------------*/
static void mml_parse_note(audio_voice_t *voice, int key) {
    int                     tmp;
    float                   length;

    // if not a pause check note modifiers
    if (key) {
        if (*voice->mml == '+' || *voice->mml == '#')   { ++key; ++voice->mml; }
        else if (*voice->mml == '-')                    { --key; ++voice->mml; }
        key += voice->octave * 12;
        key = clamp(key, 1, 88);
    }
    // check for length and length modifiers
    if ((tmp = mml_parse_number(voice)))    { length = 1.0f / (float)tmp; }
    else                                    { length = voice->length; }
    while (*voice->mml == '.')              { length *= 1.5f; ++voice->mml; }
    voice->ttl = (int)(length * voice->tempo);
    // setup note playback
    if (key) {
        voice->t0 = 0;
        voice->t1 = (int)(audio_frequency / frequencies[key]);
        voice->e0 = 1.0f;
        voice->e1 = 1.0f / (float)voice->ttl;
        switch (voice->psg) {
            case PSG_50: voice->t2 = voice->t1 / 2; break;
            case PSG_25: voice->t2 = voice->t1 / 4; break;
            case PSG_12: voice->t2 = voice->t1 / 8; break;
        }
    } else {
        voice->t0 = voice->t1 = 0;
        voice->e0 = voice->e1 = 0.0f;
    }
}


/*----------------------------------------------------------------------------*/
static int mml_parse_next(audio_voice_t *voice) {
    int                     tmp;

    while (voice->mml && *voice->mml) {
        switch (*voice->mml++) {
            case ':': voice->mml = voice->song; break;
            case '<': if (voice->octave > 0) --voice->octave; break;
            case '>': if (voice->octave < 7) ++voice->octave; break;
            case 'o': case 'O': tmp = mml_parse_number(voice); voice->octave = clamp(tmp, 0, 6); break;
            case 'l': case 'L': tmp = mml_parse_number(voice); voice->length = 1.0f / clamp(tmp, 1, 64); break;
            case 't': case 'T': tmp = mml_parse_number(voice); voice->tempo = 60.0f / (clamp(tmp, 32, 200) / 4) * audio_frequency; break;
            case 'm': case 'M': mml_parse_mode(voice); break;
            case 'p': case 'P': mml_parse_note(voice, 0); return 1;
            case 'r': case 'R': mml_parse_note(voice, 0); return 1;
            case 'c': case 'C': mml_parse_note(voice, 4); return 1;
            case 'd': case 'D': mml_parse_note(voice, 6); return 1;
            case 'e': case 'E': mml_parse_note(voice, 8); return 1;
            case 'f': case 'F': mml_parse_note(voice, 9); return 1;
            case 'g': case 'G': mml_parse_note(voice, 11); return 1;
            case 'a': case 'A': mml_parse_note(voice, 13); return 1;
            case 'b': case 'B': mml_parse_note(voice, 15); return 1;
        }
    }
    return 0;
}


/*----------------------------------------------------------------------------*/
static void mix_audio_voices(void *userdata, Uint8 *stream8, int len8) {
    float                   *stream = (float*)stream8;
    int                     i, j, len = len8 / sizeof(float);
    float                   total, sample;
    audio_voice_t           *voice;

    (void)userdata;
    // generate all samples for this callback
    for (i = 0; i < len; ++i) {
        total = 0.0f;

        // iterate over all voices
        for (j = 0; j < AUDIO_VOICES; ++j) {
            voice = &audio_voices[j];
            if (voice->ttl > 0) {
                --voice->ttl;
                if (++voice->t0 >= voice->t1) voice->t0 = 0;
                sample = (voice->t0 < voice->t2) ? 1.0f : -1.0f;
                voice->e0 -= voice->e1;
                total += sample * 0.125f * voice->e0;
            } else {
                mml_parse_next(voice);
            }
        }

        total *= audio_gain;
        *stream++ = clamp(total, -1.0f, 1.0f);
    }
}


/*
================================================================================

        LUA API

================================================================================
*/
/*----------------------------------------------------------------------------*/
static int f_quit(lua_State *L) {
    (void)L;
    running = 0;
    return 0;
}


/*----------------------------------------------------------------------------*/
static int f_btn(lua_State *L) {
    Uint8                   mask = check_button(L, 1);

    lua_pushboolean(L, btn_down & mask);
    return 1;
}


/*----------------------------------------------------------------------------*/
static int f_btnp(lua_State *L) {
    Uint8                   mask = check_button(L, 1);

    lua_pushboolean(L, btn_pressed & mask);
    return 1;
}


/*----------------------------------------------------------------------------*/
static int f_clearcolor(lua_State *L) {
    if (lua_gettop(L) > 0) {
        lua_Integer color = (int)luaL_checkinteger(L, 1);
        clear_color = clamp(color, 0, 9);
    }
    lua_pushinteger(L, clear_color);
    return 1;
}


/*----------------------------------------------------------------------------*/
static int f_clear(lua_State *L) {
    Uint8                   color = (Uint8)luaL_optinteger(L, 1, clear_color);

    if (surface8 != NULL)
        SDL_FillRect(surface8, NULL, clamp(color, 0, 9));
    return 0;
}


/*----------------------------------------------------------------------------*/
static int f_pixel(lua_State *L) {
    Uint8                   color = (Uint8)luaL_checkinteger(L, 1);
    int                     x0 = (int)luaL_checknumber(L, 2);
    int                     y0 = (int)luaL_checknumber(L, 3);

    draw_pixel(x0, y0, clamp(color, 0, 9));
    return 0;
}


/*----------------------------------------------------------------------------*/
static int f_line(lua_State *L) {
    Uint8                   color = (Uint8)luaL_checkinteger(L, 1);
    int                     x0 = (int)luaL_checknumber(L, 2);
    int                     y0 = (int)luaL_checknumber(L, 3);
    int                     x1 = (int)luaL_checknumber(L, 4);
    int                     y1 = (int)luaL_checknumber(L, 5);
    int                     dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int                     dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int                     err = dx + dy, e2;

    color = clamp(color, 0, 9);
    for (;;) {
        draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = err * 2;
        if (e2 > dy) { err += dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }

    return 0;
}


/*----------------------------------------------------------------------------*/
static int f_rect(lua_State *L) {
    Uint8                   color = (Uint8)luaL_checkinteger(L, 1);
    int                     x0 = (int)luaL_checknumber(L, 2);
    int                     y0 = (int)luaL_checknumber(L, 3);
    int                     x1 = (int)luaL_checknumber(L, 4);
    int                     y1 = (int)luaL_checknumber(L, 5);
    int                     fill = lua_toboolean(L, 6);
    int                     x, y;

    color = clamp(color, 0, 9);
    if (fill) {
        for (y = y0; y <= y1; ++y) {
            for (x = x0; x <= x1; ++x) {
                draw_pixel(x, y, color);
            }
        }
    } else {
        for (x = x0; x <= x1; ++x) {
            draw_pixel(x, y0, color);
            draw_pixel(x, y1, color);
        }
        for (y = y0; y <= y1; ++y) {
            draw_pixel(x0, y, color);
            draw_pixel(x1, y, color);
        }
    }

    return 0;
}


/*----------------------------------------------------------------------------*/
static int f_circle(lua_State *L) {
    Uint8                   color = (Uint8)luaL_checkinteger(L, 1);
    int                     x0 = (int)luaL_checknumber(L, 2);
    int                     y0 = (int)luaL_checknumber(L, 3);
    int                     radius = (int)luaL_checknumber(L, 4);
    int                     fill = lua_toboolean(L, 5);
    int                     r0sq = fill ? 0 : ((radius - 1) * (radius - 1));
    int                     r1sq = radius * radius;
    int                     x, y, dx, dy, dist;

    color = clamp(color, 0, 9);
    for (y = -radius; y <= radius; ++y) {
        dy = y * y;
        for (x = -radius; x <= radius; ++x) {
            dx = x * x;
            dist = dx + dy;
            if ((dist >= r0sq) && (dist <= r1sq))
                draw_pixel(x0 + x, y0 + y, color);
        }
    }
    return 0;
}


/*----------------------------------------------------------------------------*/
static int f_print(lua_State *L) {
    Uint8                   color = (Uint8)luaL_checkinteger(L, 1);
    int                     x0 = (int)luaL_checknumber(L, 2);
    int                     y0 = (int)luaL_checknumber(L, 3);
    const Uint8             *text = (const Uint8*)luaL_checkstring(L, 4);
    int                     x, y, bits;

    color = clamp(color, 0, 9);
    for (; *text; ++text, x0 += 8) {
        for (y = 0; y < 8; ++y) {
            bits = font8x8[*text * 8 + y];
            for (x = 0; x < 8; ++x) {
                if (bits & (1 << x))
                    draw_pixel(x0 + x, y0 + y, color);
            }
        }
    }

    return 0;
}


/*----------------------------------------------------------------------------*/
static int f_draw(lua_State *L) {
    int                     x, y, w, h, color;
    size_t                  length;
    const Uint8             *pixels = (const Uint8*)luaL_checklstring(L, 1, &length);
    int                     x0 = (int)luaL_checknumber(L, 2);
    int                     y0 = (int)luaL_checknumber(L, 3);
    int                     mask = (int)luaL_optinteger(L, 4, 255);

    luaL_argcheck(L, length >= 4, 1, "pixel string too small");
    w = pixeldecoder[pixels[0]] * 10 + pixeldecoder[pixels[1]];
    h = pixeldecoder[pixels[2]] * 10 + pixeldecoder[pixels[3]];
    luaL_argcheck(L, (int)length >= 4 + w * h, 1, "pixel string too small");

    for (y = 0, pixels += 4; y < h; ++y) {
        for (x = 0; x < w; ++x, ++pixels) {
            color = pixeldecoder[*pixels];
            if (color != mask)
                draw_pixel(x0 + x, y0 + y, color);
        }
    }

    return 0;
}


/*----------------------------------------------------------------------------*/
static int f_gain(lua_State *L) {
    if (lua_gettop(L) > 0) {
        lua_Number          gain = luaL_checknumber(L, 1);

        SDL_LockAudioDevice(audio_device);
        audio_gain = clamp(gain, 0.0, 1.0);
        SDL_UnlockAudioDevice(audio_device);
    }

    lua_pushnumber(L, audio_gain);
    return 1;
}


/*----------------------------------------------------------------------------*/
static int f_play(lua_State *L) {
    size_t                  length;
    audio_voice_t           *voice = check_voice(L, 1);
    const char              *song = luaL_checklstring(L, 2, &length);

    luaL_argcheck(L, length > 0 && length < sizeof(voice->song), 2, "invalid length of MML string");
    SDL_LockAudioDevice(audio_device);
    SDL_strlcpy(voice->song, song, sizeof(voice->song));
    voice->psg = PSG_50;
    voice->ttl = 0;
    voice->mml = voice->song;
    voice->octave = 3;
    voice->tempo = 60.0f / (120.0f / 4.0f) * audio_frequency;
    voice->length = 1.0f / 4.0f;
    SDL_UnlockAudioDevice(audio_device);
    return 0;
}


/*----------------------------------------------------------------------------*/
static int f_stop(lua_State *L) {
    audio_voice_t           *voice = check_voice(L, 1);

    SDL_LockAudioDevice(audio_device);
    voice->ttl = 0;
    voice->mml = NULL;
    SDL_UnlockAudioDevice(audio_device);
    return 0;
}


/*----------------------------------------------------------------------------*/
static const luaL_Reg       funcs[] = {
    { "quit",               f_quit          },
    { "btn",                f_btn           },
    { "btnp",               f_btnp          },
    { "clearcolor",         f_clearcolor    },
    { "clear",              f_clear         },
    { "pixel",              f_pixel         },
    { "line",               f_line          },
    { "rect",               f_rect          },
    { "circle",             f_circle        },
    { "print",              f_print         },
    { "draw",               f_draw          },
    { "gain",               f_gain          },
    { "play",               f_play          },
    { "stop",               f_stop          },
    { NULL,                 NULL            }
};


/*----------------------------------------------------------------------------*/
static int luaopen_ltro1(lua_State *L) {
    luaL_newlib(L, funcs);
    lua_pushstring(L, LTRO_VERSION); lua_setfield(L, -2, "_VERSION");
    lua_pushstring(L, LTRO_AUTHOR); lua_setfield(L, -2, "_AUTHOR");
    return 1;
}


/*
================================================================================

        EVENT LOOP

================================================================================
*/
/*----------------------------------------------------------------------------*/
static void handle_SDL_key(const SDL_Keycode code, const int down) {
    Uint8                   mask;

    switch (code) {
        case SDLK_ESCAPE: running = 0; return;
        case SDLK_UP: mask = BUTTON_UP; break;
        case SDLK_DOWN: mask = BUTTON_DOWN; break;
        case SDLK_LEFT: mask = BUTTON_LEFT; break;
        case SDLK_RIGHT: mask = BUTTON_RIGHT; break;
        case SDLK_SPACE: mask = BUTTON_A; break;
        case SDLK_LALT: mask = BUTTON_B; break;
        case SDLK_RETURN: mask = BUTTON_START; break;
        default: mask = 0; break;
    }

    if (down)   { btn_down |= mask; btn_pressed |= mask; }
    else        { btn_down &= ~mask;                     }
}


/*----------------------------------------------------------------------------*/
static void handle_controller_button(const int button, const int down) {
    Uint8                   mask;

    switch (button) {
        case SDL_CONTROLLER_BUTTON_DPAD_UP: mask = BUTTON_UP; break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: mask = BUTTON_DOWN; break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: mask = BUTTON_LEFT; break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: mask = BUTTON_RIGHT; break;
        case SDL_CONTROLLER_BUTTON_A: mask = BUTTON_A; break;
        case SDL_CONTROLLER_BUTTON_START: mask = BUTTON_START; break;
        default: mask = 0; break;
    }

    if (down)   { btn_down |= mask; btn_pressed |= mask; }
    else        { btn_down &= ~mask;                     }
}


/*----------------------------------------------------------------------------*/
static void handle_SDL_events() {
    SDL_Event               ev;

    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT:
                running = 0;
                break;

            case SDL_KEYDOWN:
                handle_SDL_key(ev.key.keysym.sym, 1);
                break;

            case SDL_KEYUP:
                handle_SDL_key(ev.key.keysym.sym, 0);
                break;
            
            case SDL_CONTROLLERBUTTONDOWN:
                handle_controller_button(ev.cbutton.button, 1);
                break;
            
            case SDL_CONTROLLERBUTTONUP:
                handle_controller_button(ev.cbutton.button, 0);
                break;
        }
    }
}


/*----------------------------------------------------------------------------*/
#ifdef __EMSCRIPTEN__
static lua_State            *global_L;
static Uint32               last_tick;
static int                  file_fetched = 0;


/*----------------------------------------------------------------------------*/
static void run_event_step() {
    static double           delta_ticks = 0.0;
    static lua_Integer      frame_counter = 0;
    Uint32                  current_tick;

    if (!file_fetched) return; // do nothing when the file is not fetched
    if (!running) {
        SDL_PauseAudioDevice(audio_device, SDL_TRUE);
        emscripten_cancel_main_loop();
        return;
    }

    // do single event step
    handle_SDL_events();

    current_tick = SDL_GetTicks();
    delta_ticks += current_tick - last_tick;
    last_tick = current_tick;

    for (; delta_ticks >= FPS_TICKS; delta_ticks -= FPS_TICKS) {
        if (push_callback(global_L, "on_tick")) {
            lua_pushinteger(global_L, frame_counter);
            lua_call(global_L, 1, 0);
        }
        ++frame_counter;
        btn_pressed = 0;
    }

    render_screen(global_L);    
}


/*----------------------------------------------------------------------------*/
static void fetch_on_success(emscripten_fetch_t *fetch) {
    int                     status;
    
    // compile the downloaded Lua script
    status = luaL_loadbuffer(global_L, (const char*)fetch->data, (size_t)fetch->numBytes, "@game.lua");
    emscripten_fetch_close(fetch);
    if (status != LUA_OK) lua_error(global_L);
    lua_call(global_L, 0, 0);

    // call "on_init"
    if (push_callback(global_L, "on_init"))
        lua_call(global_L, 0, 0);
    
    // make sure the event loop will continue
    file_fetched = -1;
}


/*----------------------------------------------------------------------------*/
static void fetch_on_error(emscripten_fetch_t *fetch) {
    emscripten_fetch_close(fetch);
    luaL_error(global_L, "Failed to download 'game.lua'");
}


/*----------------------------------------------------------------------------*/
static void run_event_loop(lua_State *L) {
    emscripten_fetch_attr_t attr;
    
    global_L = L;

    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = fetch_on_success;
    attr.onerror = fetch_on_error;
    emscripten_fetch(&attr, "game.lua");

    last_tick = SDL_GetTicks();
    emscripten_set_main_loop(run_event_step, 0, 1);    

    if (push_callback(global_L, "on_quit"))
        lua_call(global_L, 0, 0);
}
#else
/*----------------------------------------------------------------------------*/
static void run_event_loop(lua_State *L) {
    Uint32                  last_tick, current_tick;
    double                  delta_ticks = 0.0;
    lua_Integer             frame_counter = 0;

    // load script and execute
    if (luaL_loadfile(L, "game.lua") != LUA_OK)
        lua_error(L);
    lua_call(L, 0, 0);

    // call on_init
    if (push_callback(L, "on_init"))
        lua_call(L, 0, 0);

    // run the whole event loop
    last_tick = SDL_GetTicks();
    while (running) {
        handle_SDL_events();

        current_tick = SDL_GetTicks();
        delta_ticks += current_tick - last_tick;
        last_tick = current_tick;

        for (; delta_ticks >= FPS_TICKS; delta_ticks -= FPS_TICKS) {
            if (push_callback(L, "on_tick")) {
                lua_pushinteger(L, frame_counter);
                lua_call(L, 1, 0);
            }
            ++frame_counter;
            btn_pressed = 0;
        }

        render_screen(L);
    }

    // call on_quit
    if (push_callback(L, "on_quit"))
        lua_call(L, 0, 0);
}
#endif /* __EMSCRIPTEN__ */


/*
================================================================================

        INIT / SHUTDOWN

================================================================================
*/
/*----------------------------------------------------------------------------*/
static int initialize_ltro1(lua_State *L) {
    int                     w, h, bpp;
    Uint32                  pixel_format, rmask, gmask, bmask, amask;
    SDL_DisplayMode         dm;
    SDL_AudioSpec           want, have;

    SDL_zero(audio_voices);

    #ifdef __EMSCRIPTEN__
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER))
            luaL_error(L, "SDL_Init() failed: %s", SDL_GetError());
    #else
        if (SDL_Init(SDL_INIT_EVERYTHING))
            luaL_error(L, "SDL_Init() failed: %s", SDL_GetError());
    #endif /* __EMSCRIPTEN__ */

    // determine best window size
    w = SCREEN_WIDTH; h = SCREEN_HEIGHT;
    if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        dm.w -= SCREEN_PADDING; dm.h -= SCREEN_PADDING;
        while ((w < dm.w) && (h < dm.h)) { w *= 2; h *= 2; }
        while ((w > dm.w) || (h > dm.h)) { w /= 2; h /= 2; }
    }

    // initialize screen
    if ((window = SDL_CreateWindow(SCREEN_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_RESIZABLE)) == NULL)
        luaL_error(L, "SDL_CreateWindow() failed: %s", SDL_GetError());
    if ((pixel_format = SDL_GetWindowPixelFormat(window)) == SDL_PIXELFORMAT_UNKNOWN)
        luaL_error(L, "SDL_GetWindowPixelFormat() failed: %s", SDL_GetError());
    if (SDL_PixelFormatEnumToMasks(pixel_format, &bpp, &rmask, &gmask, &bmask, &amask) == SDL_FALSE)
        luaL_error(L, "SDL_PixelFormatEnumToMasks() failed: %s", SDL_GetError());
    if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC)) == NULL)
        luaL_error(L, "SDL_CreateRenderer() failed: %s", SDL_GetError());
    if (SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT))
        luaL_error(L, "SDL_RenderSetLogicalSize() failed: %s", SDL_GetError());
    if ((texture = SDL_CreateTexture(renderer, pixel_format, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT)) == NULL)
        luaL_error(L, "SDL_CreateTexture() failed: %s", SDL_GetError());
    if ((surface32 = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, bpp, rmask, gmask, bmask, amask)) == NULL)
        luaL_error(L, "SDL_CreateRGBSurface() failed: %s", SDL_GetError());
    if ((surface8 = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, 0, 0, 0)) == NULL)
        luaL_error(L, "SDL_CreateRGBSurface() failed: %s", SDL_GetError());
    if (SDL_SetPaletteColors(surface8->format->palette, palette, 0, 10))
        luaL_error(L, "SDL_SetPaletteColors() failed: %s", SDL_GetError());

    // initialize audio
    SDL_zero(want); SDL_zero(have);
    want.freq = AUDIO_FREQUENCY;
    want.channels = 1;
    want.format = AUDIO_F32SYS;
    want.callback = mix_audio_voices;

    if ((audio_device = SDL_OpenAudioDevice(NULL, SDL_FALSE, &want, &have, 0)) == 0)
        luaL_error(L, "SDL_OpenAudioDevice() failed: %s", SDL_GetError());
    if ((have.format != AUDIO_F32SYS) || (have.channels != 1))
        luaL_error(L, "SDL_OpenAudioDevice() returned with wrong configuration");
    audio_frequency = have.freq;
    SDL_PauseAudioDevice(audio_device, SDL_FALSE);

    // run event loop
    run_event_loop(L);
    return 0;
}


/*----------------------------------------------------------------------------*/
static void shutdown_ltro1() {
    if (audio_device != 0)
        SDL_CloseAudioDevice(audio_device);
    if (surface8 != NULL)
        SDL_FreeSurface(surface8);
    if (surface32 != NULL)
        SDL_FreeSurface(surface32);
    if (texture != NULL)
        SDL_DestroyTexture(texture);
    if (renderer != NULL)
        SDL_DestroyRenderer(renderer);
    if (window != NULL)
        SDL_DestroyWindow(window);

    SDL_Quit();
}


/*----------------------------------------------------------------------------*/
int main(int argc, char **argv) {
    lua_State               *L;

    (void)argc; (void)argv;

    L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "ltro1", luaopen_ltro1, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, "ltro_callbacks");

    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_remove(L, -2);

    lua_pushcfunction(L, initialize_ltro1);
    if (lua_pcall(L, 0, 0, -2) != LUA_OK) {
        const char          *message = luaL_gsub(L, lua_tostring(L, -1), "\t", "    ");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "LTRO-1 Panic!", message, window);
    }

    lua_close(L);
    shutdown_ltro1();

    return 0;
}
