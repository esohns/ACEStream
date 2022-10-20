/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef STREAM_VIS_DEFINES_H
#define STREAM_VIS_DEFINES_H

#include "ace/config-lite.h"

#include "stream_dev_defines.h"

#if defined (FFMPEG_SUPPORT)
#define STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING                       "LibAVResize"
#endif // FFMPEG_SUPPORT
#if defined (IMAGEMAGICK_SUPPORT)
#define STREAM_VIS_IMAGEMAGICK_RESIZE_DEFAULT_NAME_STRING                 "ImageMagickResize"
#endif // IMAGEMAGICK_SUPPORT
#define STREAM_VIS_CURSES_WINDOW_DEFAULT_NAME_STRING                      "Curses"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define STREAM_VIS_DIRECT2D_DEFAULT_NAME_STRING                           "Direct2D"
#define STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING                           "Direct3D"
#define STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING                         "DirectShow"
#define STREAM_VIS_GDI_DEFAULT_NAME_STRING                                "GDI"
#define STREAM_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING                    "MediaFoundation"
#else
#define STREAM_VIS_WAYLAND_WINDOW_DEFAULT_NAME_STRING                     "Wayland"
#define STREAM_VIS_X11_WINDOW_DEFAULT_NAME_STRING                         "X11"
#endif // ACE_WIN32 || ACE_WIN64
#define STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING                          "GTK_Cairo"
#define STREAM_VIS_GTK_PIXBUF_DEFAULT_NAME_STRING                         "GTK_Pixbuf"
#define STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING              "GTK_SpectrumAnalyzer"
#define STREAM_VIS_GTK_WINDOW_DEFAULT_NAME_STRING                         "GTK_Window"

#define STREAM_VIS_OPENCV_DEFAULT_NAME_STRING                             "OpenCV"
#define STREAM_VIS_OPENCV_CLASSIFIER_DEFAULT_NAME_STRING                  "OpenCV_Classifier"

#define STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING                        "OpenGL_GLUT"

#define STREAM_VIS_NULL_DEFAULT_NAME_STRING                               "Null"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define STREAM_VIS_FRAMEWORK_DEFAULT                                      STREAM_VISUALIZATION_FRAMEWORK_DIRECTDRAW
#endif // ACE_WIN32 || ACE_WIN64

// renderers
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define STREAM_VIS_RENDERER_VIDEO_DIRECTDRAW_3D_SCREENSHOT_DEFAULT_FORMAT D3DXIFF_JPG

#define STREAM_VIS_RENDERER_VIDEO_DIRECTSHOW_DEFAULT_SAMPLES              60

#define STREAM_VIS_RENDERER_WINDOW_DEFAULT_MESSAGE_PUMP_THREAD_NAME       "message pump"
#endif // ACE_WIN32 || ACE_WIN64

// *NOTE*: "...each pixel is a 32-bit quantity, with the upper 8 bits unused.
//         Red, Green, and Blue are stored in the remaining 24 bits in that
//         order. ..."
#define STREAM_VIS_RENDERER_VIDEO_GTK_CAIRO_DEFAULT_FORMAT                CAIRO_FORMAT_RGB24
//// *NOTE*: "...each pixel is a 32-bit quantity, with alpha in the upper 8 bits,
////          then red, then green, then blue. The 32-bit quantities are stored
////          native-endian. Pre-multiplied alpha is used. (That is, 50%
////          transparent red is 0x80800000, not 0x80ff0000.) ..."
//#define STREAM_VIS_DEFAULT_CAIRO_FORMAT      CAIRO_FORMAT_ARGB32

#define STREAM_VIS_DEFAULT_SCREENSHOT_FILENAME_PREFIX_STRING              "screenshot"

#define STREAM_VIS_DEFAULT_WINDOW_HEIGHT                                  240
#define STREAM_VIS_DEFAULT_WINDOW_WIDTH                                   320
#define STREAM_VIS_DEFAULT_WINDOW_TITLE                                   "ACEStream window"

// spectrum analyzer
// *IMPORTANT NOTE*: must be a power of 2 (FFT-specific)
// *IMPORTANT NOTE*: only half of the buffer (-1) contains meaningful results
//                   (FFT-specific)
#define STREAM_VIS_SPECTRUMANALYZER_DEFAULT_BUFFER_SIZE                   2048 // samples
// *NOTE*: needed for computation; these need to correspond with the input data
// *TODO*: remove ASAP
#define STREAM_VIS_SPECTRUMANALYZER_DEFAULT_CHANNELS                      STREAM_DEV_MIC_DEFAULT_CHANNELS
#define STREAM_VIS_SPECTRUMANALYZER_DEFAULT_SAMPLE_RATE                   STREAM_DEV_MIC_DEFAULT_SAMPLE_RATE

#define STREAM_VIS_SPECTRUMANALYZER_DEFAULT_FRAME_RATE                    30 // frames/s
#define STREAM_VIS_SPECTRUMANALYZER_DEFAULT_2DMODE                        STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM
#define STREAM_VIS_SPECTRUMANALYZER_DEFAULT_3DMODE                        STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_DEFAULT

#endif
