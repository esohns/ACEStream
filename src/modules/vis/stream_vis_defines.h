#ifndef STREAM_MODULE_VIS_DEFINES_H
#define STREAM_MODULE_VIS_DEFINES_H

#include "ace/config-lite.h"

//#include "gtk/gtk.h"

#define MODULE_VIS_NULL_RENDERER_MODULE_NAME "DisplayNull"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define MODULE_VIS_DEFAULT_VIDEO_SAMPLES     60
#else
// *NOTE*: "...each pixel is a 32-bit quantity, with the upper 8 bits unused.
//         Red, Green, and Blue are stored in the remaining 24 bits in that
//         order. ..."
#define MODULE_VIS_DEFAULT_CAIRO_FORMAT      CAIRO_FORMAT_RGB24
//// *NOTE*: "...each pixel is a 32-bit quantity, with alpha in the upper 8 bits,
////          then red, then green, then blue. The 32-bit quantities are stored
////          native-endian. Pre-multiplied alpha is used. (That is, 50%
////          transparent red is 0x80800000, not 0x80ff0000.) ..."
//#define MODULE_VIS_DEFAULT_CAIRO_FORMAT      CAIRO_FORMAT_ARGB32
#endif

#endif
