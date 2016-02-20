#ifndef STREAM_MODULE_VIS_DEFINES_H
#define STREAM_MODULE_VIS_DEFINES_H

//#include "gtk/gtk.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#define MODULE_VIS_DEFAULT_CAIRO_FORMAT CAIRO_FORMAT_RGB24
#endif

#endif
