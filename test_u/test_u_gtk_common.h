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

#ifndef TEST_U_GTK_COMMON_H
#define TEST_U_GTK_COMMON_H

#include "ace/OS.h"
#include "ace/Synch_Traits.h"

#include "common_ilock.h"
#include "common_itaskcontrol.h"

#include "common_ui_gtk_common.h"
#if defined (GTKGL_SUPPORT)
#include "common_ui_gtk_gl_common.h"
#endif // GTKGL_SUPPORT

#include "test_u_common.h"

// forward declarations
struct Test_U_Configuration;

struct Test_U_GTK_CBData;
struct Test_U_GTK_ProgressData
 : Common_UI_GTK_ProgressData
{
  Test_U_GTK_ProgressData ()
   : Common_UI_GTK_ProgressData ()
   , statistic ()
  {
    ACE_OS::memset (&statistic, 0, sizeof (Test_U_Statistic_t));
  }

  Test_U_Statistic_t statistic;
};

struct Test_U_GTK_CBData
#if defined (GTKGL_SUPPORT)
  : Common_UI_GTK_GLState
#else
  : Common_UI_GTK_State
#endif // GTKGL_SUPPORT
{
  Test_U_GTK_CBData ()
#if defined (GTKGL_SUPPORT)
   : Common_UI_GTK_GLState ()
#else
   : Common_UI_GTK_State ()
#endif // GTKGL_SUPPORT
   , allowUserRuntimeStatistic (true)
   , configuration (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (MODULE_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , progressData ()
   , progressEventSourceId (0)
  {}

  bool                            allowUserRuntimeStatistic;
  struct Test_U_Configuration*    configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  struct Test_U_GTK_ProgressData  progressData;
  guint                           progressEventSourceId;
};

struct Test_U_GTK_ThreadData
{
  Test_U_GTK_ThreadData ()
   : CBData (NULL)
   , eventSourceId (0)
   , sessionId (0)
  {}

  struct Test_U_GTK_CBData*       CBData;
  guint                           eventSourceId;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  size_t                          sessionId;
};

typedef Common_ITaskControl_T<ACE_MT_SYNCH,
                              Common_ILock_T<ACE_MT_SYNCH> > Test_U_GTK_Manager_t;

#endif
