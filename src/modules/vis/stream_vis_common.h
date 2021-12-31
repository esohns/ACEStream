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

#ifndef STREAM_VISUALIZATION_COMMON_H
#define STREAM_VISUALIZATION_COMMON_H

#include "ace/config-lite.h"

enum Stream_Visualization_Framework
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  STREAM_VISUALIZATION_FRAMEWORK_DIRECTDRAW,
  STREAM_VISUALIZATION_FRAMEWORK_DIRECTSHOW,
  STREAM_VISUALIZATION_FRAMEWORK_GDI,
  STREAM_VISUALIZATION_FRAMEWORK_MEDIAFOUNDATION,
#else
  STREAM_VISUALIZATION_FRAMEWORK_X11,
  STREAM_VISUALIZATION_FRAMEWORK_WAYLAND,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (OPENGL_SUPPORT)
  STREAM_VISUALIZATION_FRAMEWORK_OPENGL,
#endif // OPENGL_SUPPORT
#if defined (OPENCV_SUPPORT)
  STREAM_VISUALIZATION_FRAMEWORK_OPENCV,
#endif // OPENCV_SUPPORT
#if defined (GTK_SUPPORT)
  STREAM_VISUALIZATION_FRAMEWORK_GTK,
#endif // GTK_SUPPORT
  ////////////////////////////////////////
  STREAM_VISUALIZATION_FRAMEWORK_MAX,
  STREAM_VISUALIZATION_FRAMEWORK_INVALID
};

enum Stream_Visualization_AudioRenderer
{
  STREAM_VISUALIZATION_AUDIORENDERER_NULL = 0,
#if defined (GTK_SUPPORT)
  STREAM_VISUALIZATION_AUDIORENDERER_GTK_CAIRO_SPECTRUMANALYZER,
#endif // GTK_SUPPORT
  ////////////////////////////////////////
  STREAM_VISUALIZATION_AUDIORENDERER_MAX,
  STREAM_VISUALIZATION_AUDIORENDERER_INVALID
};

enum Stream_Visualization_VideoRenderer
{
  STREAM_VISUALIZATION_VIDEORENDERER_NULL = 0,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_2D,
  STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D,
  STREAM_VISUALIZATION_VIDEORENDERER_DIRECTSHOW,
  STREAM_VISUALIZATION_VIDEORENDERER_GDI,
  STREAM_VISUALIZATION_VIDEORENDERER_MEDIAFOUNDATION,
#else
  STREAM_VISUALIZATION_VIDEORENDERER_X11,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GTK_SUPPORT)
  STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO,
  STREAM_VISUALIZATION_VIDEORENDERER_GTK_PIXBUF,
  STREAM_VISUALIZATION_VIDEORENDERER_GTK_WINDOW,
#endif // GTK_SUPPORT
  ////////////////////////////////////////
  STREAM_VISUALIZATION_VIDEORENDERER_MAX,
  STREAM_VISUALIZATION_VIDEORENDERER_INVALID
};

enum Stream_Visualization_SpectrumAnalyzer_2DMode
{ // *TODO*: implement discrete modes of operation
  STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE = 0,
  STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM,
  ////////////////////////////////////////
  STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_MAX,
  STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_INVALID
};
enum Stream_Visualization_SpectrumAnalyzer_3DMode
{
  STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_DEFAULT = 0,
  ////////////////////////////////////////
  STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_MAX,
  STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_INVALID
};

enum Stream_Visualization_InstructionType
{
  STREAM_VISUALIZATION_INSTRUCTION_CHANGE_ROTATION,
  STREAM_VISUALIZATION_INSTRUCTION_SET_COLOR_BG,
  STREAM_VISUALIZATION_INSTRUCTION_SET_COLOR_FG,
  ////////////////////////////////////////
  STREAM_VISUALIZATION_INSTRUCTION_MAX,
  STREAM_VISUALIZATION_INSTRUCTION_INVALID
};

#endif
