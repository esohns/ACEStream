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

#include "stream_vis_gtk_cairo_gl.h"

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_directsound_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_vis_defines.h"
#include "stream_vis_tools.h"

Stream_Visualization_GTK_Cairo_OpenGL::Stream_Visualization_GTK_Cairo_OpenGL ()
 : backgroundColor_ ()
 , foregroundColor_ ()
//#if defined (GTKGL_SUPPORT)
// , mode3D_ (NULL)
//#endif // GTKGL_SUPPORT
#if GTK_CHECK_VERSION(3,0,0)
 , randomDistribution_ (0, 255)
#else
 , randomDistribution_ (0, 65535)
#endif // GTK_CHECK_VERSION(3,0,0)
 , randomEngine_ ()
 , randomGenerator_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_OpenGL::Stream_Visualization_GTK_Cairo_OpenGL"));

#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_ENTER ();
#endif // GTK_CHECK_VERSION (3,6,0)
  gboolean result_2 =
    gdk_rgba_parse (&backgroundColor_,
                    ACE_TEXT_ALWAYS_CHAR ("rgba (0, 0, 0, 1.0)"));       // opaque black
  ACE_ASSERT (result_2);
  result_2 =
    gdk_rgba_parse (&foregroundColor_,
                    ACE_TEXT_ALWAYS_CHAR ("rgba (255, 255, 255, 1.0)")); // opaque white
  ACE_ASSERT (result_2);
#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_LEAVE ();
#endif // GTK_CHECK_VERSION (3,6,0)
#else
  ACE_OS::memset (&backgroundColor_, 0, sizeof (struct _GdkColor));                            // opaque black
  foregroundColor_.pixel = 0;
  foregroundColor_.red = 65535; foregroundColor_.green = 65535; foregroundColor_.blue = 65535; // opaque white
#endif // GTK_CHECK_VERSION (3,0,0)
#endif // GTKGL_SUPPORT

  randomGenerator_ = std::bind (randomDistribution_, randomEngine_);
}

//bool
//Stream_Visualization_GTK_Cairo_OpenGL::initialize (const ConfigurationType& configuration_in,
//                                                   Stream_IAllocator* allocator_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_OpenGL::initialize"));
//
//  if (unlikely (inherited::isInitialized_))
//  {
//#if defined (GTKGL_SUPPORT)
//#if GTK_CHECK_VERSION (3,0,0)
//#if GTK_CHECK_VERSION (3,6,0)
//#else
//    GDK_THREADS_ENTER ();
//#endif // GTK_CHECK_VERSION (3,6,0)
//    gboolean result_2 =
//      gdk_rgba_parse (&backgroundColor_,
//                      ACE_TEXT_ALWAYS_CHAR ("rgba (0, 0, 0, 1.0)"));       // opaque black
//    ACE_ASSERT (result_2);
//    result_2 =
//      gdk_rgba_parse (&foregroundColor_,
//                      ACE_TEXT_ALWAYS_CHAR ("rgba (255, 255, 255, 1.0)")); // opaque white
//    ACE_ASSERT (result_2);
//#if GTK_CHECK_VERSION (3,6,0)
//#else
//    GDK_THREADS_LEAVE ();
//#endif // GTK_CHECK_VERSION (3,6,0)
//#else
//    ACE_OS::memset (&backgroundColor_, 0, sizeof (struct _GdkColor));                            // opaque black
//    foregroundColor_.pixel = 0;
//    foregroundColor_.red = 65535; foregroundColor_.green = 65535; foregroundColor_.blue = 65535; // opaque white
//#endif /* GTK_CHECK_VERSION (3,0,0) */
//#endif /* GTKGL_SUPPORT */
//  } // end IF
//
//  return inherited::initialize (configuration_in,
//                                allocator_in);
//}

void
Stream_Visualization_GTK_Cairo_OpenGL::dispatch (const enum Stream_Statistic_AnalysisEventType& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_OpenGL::dispatch"));

#if defined (GTKGL_SUPPORT)
  // sanity check(s)
  //ACE_ASSERT (inherited::configuration_->OpenGLInstructionsLock);
  //ACE_ASSERT (inherited::configuration_->OpenGLInstructions);

  struct Stream_Visualization_GTKGL_Instruction visualization_instruction_s;

  switch (event_in)
  {
    case STREAM_STATISTIC_ANALYSIS_EVENT_ACTIVITY:
    {
//#if GTK_CHECK_VERSION(3,0,0)
//      foregroundColor_.red   = randomGenerator_ () / 255.0;
//      foregroundColor_.green = randomGenerator_ () / 255.0;
//      foregroundColor_.blue  = randomGenerator_ () / 255.0;
//      //foregroundColor_.alpha = ;
//#else
//      foregroundColor_.red   = randomGenerator_ ();
//      foregroundColor_.green = randomGenerator_ ();
//      foregroundColor_.blue  = randomGenerator_ ();
//      //foregroundColor_.alpha = ;
//#endif // GTK_CHECK_VERSION(3,0,0)
//      opengl_instruction.color = foregroundColor_;
//      opengl_instruction.type =
//        STREAM_VISUALIZATION_INSTRUCTION_SET_COLOR_FG;
      visualization_instruction_s.type =
        STREAM_VISUALIZATION_INSTRUCTION_CHANGE_ROTATION;
      break;
    }
    case STREAM_STATISTIC_ANALYSIS_EVENT_PEAK:
    {
#if GTK_CHECK_VERSION(3,0,0)
      backgroundColor_.red   = randomGenerator_ () / 255.0;
      backgroundColor_.green = randomGenerator_ () / 255.0;
      backgroundColor_.blue  = randomGenerator_ () / 255.0;
      //backgroundColor_.alpha = ;
#else
      backgroundColor_.red   = randomGenerator_ ();
      backgroundColor_.green = randomGenerator_ ();
      backgroundColor_.blue  = randomGenerator_ ();
      //backgroundColor_.alpha = ;
#endif // GTK_CHECK_VERSION(3,0,0)
      visualization_instruction_s.color = backgroundColor_;
      visualization_instruction_s.type =
        STREAM_VISUALIZATION_INSTRUCTION_SET_COLOR_BG;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown event (was: %d), returning\n"),
                  event_in));
      return;
    }
  } // end SWITCH

  //{ ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *inherited::configuration_->OpenGLInstructionsLock);
  //  inherited::configuration_->OpenGLInstructions->push_back (visualization_instruction_s);
  //} // end lock scope
#endif // GTKGL_SUPPORT
}