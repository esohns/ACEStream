#ifndef STREAM_MODULE_VIS_DEFINES_H
#define STREAM_MODULE_VIS_DEFINES_H

#include "ace/config-lite.h"

//#include "gtk/gtk.h"

#define MODULE_VIS_RENDERER_NULL_MODULE_NAME            "DisplayNull"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define MODULE_VIS_RENDERER_VIDEO_DEFAULT_SAMPLES       60
#else
// *NOTE*: "...each pixel is a 32-bit quantity, with the upper 8 bits unused.
//         Red, Green, and Blue are stored in the remaining 24 bits in that
//         order. ..."
#define MODULE_VIS_RENDERER_CAIRO_DEFAULT_FORMAT        CAIRO_FORMAT_RGB24
//// *NOTE*: "...each pixel is a 32-bit quantity, with alpha in the upper 8 bits,
////          then red, then green, then blue. The 32-bit quantities are stored
////          native-endian. Pre-multiplied alpha is used. (That is, 50%
////          transparent red is 0x80800000, not 0x80ff0000.) ..."
//#define MODULE_VIS_DEFAULT_CAIRO_FORMAT      CAIRO_FORMAT_ARGB32
#endif

// spectrum analyzer
// *NOTE*: process this many samples in one 'sweep'
// *IMPORTANT NOTE*: must be a power of 2
#define MODULE_VIS_SPECTRUMANALYZER_DEFAULT_BUFFER_SIZE 1024 // samples
// *NOTE*: needed for computation; these need to correspond with the input data
// *TODO*: remove ASAP
#define MODULE_VIS_SPECTRUMANALYZER_DEFAULT_CHANNELS    2
#define MODULE_VIS_SPECTRUMANALYZER_DEFAULT_SAMPLE_RATE 44100

#define MODULE_VIS_SPECTRUMANALYZER_DEFAULT_FRAME_RATE  30 // frames/s
#define MODULE_VIS_SPECTRUMANALYZER_DEFAULT_OPENGLMODE  STREAM_MODULE_VIS_GTK_CAIRO_SPECTRUMANALYZER_OPENGLMODE_DEFAULT
#define MODULE_VIS_SPECTRUMANALYZER_DEFAULT_SIGNALMODE  STREAM_MODULE_VIS_GTK_CAIRO_SPECTRUMANALYZER_SIGNALMODE_SPECTRUM

#endif
