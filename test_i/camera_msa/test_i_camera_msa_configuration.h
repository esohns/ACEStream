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

#ifndef TEST_I_CAMERA_MSA_CONFIGURATION_H
#define TEST_I_CAMERA_MSA_CONFIGURATION_H

#include "ace/Synch_Traits.h"

#include "common_configuration.h"
#include "common_isubscribe.h"

#include "stream_messageallocatorheap_base.h"

#include "test_i_configuration.h"

#include "test_i_camera_msa_stream_common.h"

// forward declarations
template <typename NotificationType,
          typename DataMessageType,
          typename SessionMessageType>
class Test_I_EventHandler_T;
class MSAFluidSolver2D;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_Configuration
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
{
  Test_I_DirectShow_Configuration ()
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration direct3DConfiguration;
  Test_I_DirectShow_StreamConfiguration_t             streamConfiguration;
};

struct Test_I_MediaFoundation_Configuration
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
{
  Test_I_MediaFoundation_Configuration ()
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration direct3DConfiguration;
  Test_I_MediaFoundation_StreamConfiguration_t        streamConfiguration;
};
#else
struct Test_I_V4L_Configuration
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
{
  Test_I_V4L_Configuration ()
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  Test_I_StreamConfiguration_t streamConfiguration;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_DirectShow_Message_t,
                                          Test_I_DirectShow_SessionMessage_t> Test_I_DirectShow_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_MediaFoundation_Message_t,
                                          Test_I_MediaFoundation_SessionMessage_t> Test_I_MediaFoundation_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Message_t,
                                          Test_I_SessionMessage_t> Test_I_MessageAllocator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_ISubscribe_T<Test_I_DirectShow_ISessionNotify_t> Test_I_DirectShow_ISubscribe_t;
typedef Common_ISubscribe_T<Test_I_MediaFoundation_ISessionNotify_t> Test_I_MediaFoundation_ISubscribe_t;

typedef Test_I_EventHandler_T<Test_I_DirectShow_ISessionNotify_t,
                                           Test_I_DirectShow_Message_t,
                                           Test_I_DirectShow_SessionMessage_t> Test_I_DirectShow_EventHandler_t;
typedef Test_I_EventHandler_T<Test_I_MediaFoundation_ISessionNotify_t,
                                           Test_I_MediaFoundation_Message_t,
                                           Test_I_MediaFoundation_SessionMessage_t> Test_I_MediaFoundation_EventHandler_t;
#else
typedef Common_ISubscribe_T<Test_I_ISessionNotify_t> Test_I_ISubscribe_t;

typedef Test_I_EventHandler_T<Test_I_ISessionNotify_t,
                              Test_I_Message_t,
                              Test_I_SessionMessage_t> Test_I_EventHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (GTK_SUPPORT)
struct Test_I_UI_GTK_CBData
 : Common_UI_GTK_CBData
{
  Test_I_UI_GTK_CBData ()
   : solver (NULL)
  {}

  MSAFluidSolver2D* solver;
};
#endif // GTK_SUPPORT

#endif // TEST_I_CAMERA_MSA_CONFIGURATION_H
