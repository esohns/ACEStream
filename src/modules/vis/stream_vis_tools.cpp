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
#include "stdafx.h"

#include "stream_vis_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "dshow.h"
#include "dvdmedia.h"
//#include "evr.h"
//#include "ksuuids.h"
#include "mfapi.h"
//#include "mtype.h"
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
//#include "wmcodecdsp.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_lib_common.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directdraw_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_vis_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
Stream_Visualization_Tools::initialize (enum Stream_Visualization_Framework framework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_Tools::initialize"));

  bool result = false;

  switch (framework_in)
  {
    case STREAM_VISUALIZATION_FRAMEWORK_DIRECTDRAW:
    {
      result =
        Stream_MediaFramework_DirectDraw_Tools::initialize ();
      if (!result)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_DirectDraw_Tools::initialize(), aborting\n")));
        return false;
      }
      break;
    }
    case STREAM_VISUALIZATION_FRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_VISUALIZATION_FRAMEWORK_GDI:
      break;
    case STREAM_VISUALIZATION_FRAMEWORK_MEDIAFOUNDATION:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown visualization framework (was: %d), aborting\n"),
                  framework_in));
      return false;
    }
  } // end SWITCH

  return result;
}

void
Stream_Visualization_Tools::finalize (enum Stream_Visualization_Framework framework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_Tools::finalize"));

  switch (framework_in)
  {
    case STREAM_VISUALIZATION_FRAMEWORK_DIRECTDRAW:
    {
      Stream_MediaFramework_DirectDraw_Tools::finalize ();
      break;
    }
    case STREAM_VISUALIZATION_FRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_VISUALIZATION_FRAMEWORK_GDI:
      break;
    case STREAM_VISUALIZATION_FRAMEWORK_MEDIAFOUNDATION:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown visualization framework (was: %d), returning\n"),
                  framework_in));
      return;
    }
  } // end SWITCH
}
#endif // ACE_WIN32 || ACE_WIN64

std::string
Stream_Visualization_Tools::rendererToModuleName (enum Stream_Visualization_AudioRenderer renderer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_Tools::rendererToModuleName"));

  std::string result;

  switch (renderer_in)
  {
    case STREAM_VISUALIZATION_AUDIORENDERER_NULL:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_NULL_DEFAULT_NAME_STRING); break;
#if defined (GTK_SUPPORT)
    case STREAM_VISUALIZATION_AUDIORENDERER_GTK_CAIRO_SPECTRUMANALYZER:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING); break;
#endif // GTK_SUPPORT
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown audio renderer (was: %d), aborting\n"),
                  renderer_in));
      break;
    }
  } // end SWITCH

  return result;
}
std::string
Stream_Visualization_Tools::rendererToModuleName (enum Stream_Visualization_VideoRenderer renderer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_Tools::rendererToModuleName"));

  std::string result;

  switch (renderer_in)
  {
    case STREAM_VISUALIZATION_VIDEORENDERER_NULL:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_NULL_DEFAULT_NAME_STRING); break;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_2D:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT2D_DEFAULT_NAME_STRING); break;
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING); break;
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D_11:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_11_DEFAULT_NAME_STRING); break;
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTSHOW:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING); break;
    case STREAM_VISUALIZATION_VIDEORENDERER_GDI:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GDI_DEFAULT_NAME_STRING); break;
    case STREAM_VISUALIZATION_VIDEORENDERER_MEDIAFOUNDATION:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING); break;
#else
    case STREAM_VISUALIZATION_VIDEORENDERER_WAYLAND:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_WAYLAND_WINDOW_DEFAULT_NAME_STRING);
      break;
    case STREAM_VISUALIZATION_VIDEORENDERER_X11:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_X11_WINDOW_DEFAULT_NAME_STRING); break;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (CURSES_SUPPORT)
    case STREAM_VISUALIZATION_VIDEORENDERER_CURSES:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_CURSES_WINDOW_DEFAULT_NAME_STRING); break;
#endif // CURSES_SUPPORT
#if defined (GTK_SUPPORT)
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING); break;
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_PIXBUF:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_PIXBUF_DEFAULT_NAME_STRING); break;
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_WINDOW:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_WINDOW_DEFAULT_NAME_STRING); break;
#endif // GTK_SUPPORT
#if defined (GLUT_SUPPORT)
    case STREAM_VISUALIZATION_VIDEORENDERER_OPENGL_GLUT:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING); break;
#endif // GLUT_SUPPORT
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown video renderer (was: %d), aborting\n"),
                  renderer_in));
      break;
    }
  } // end SWITCH

  return result;
}
