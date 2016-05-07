#ifndef STREAM_MODULE_VIS_DEFINES_H
#define STREAM_MODULE_VIS_DEFINES_H

//#include "gtk/gtk.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define MODULE_VIS_DEFAULT_VIDEO_SAMPLES 60
#else
// *NOTE*: "...each pixel is a 32-bit quantity, with the upper 8 bits unused.
//         Red, Green, and Blue are stored in the remaining 24 bits in that
//         order. ..."
#define MODULE_VIS_DEFAULT_CAIRO_FORMAT  CAIRO_FORMAT_RGB24
#endif

#endif
