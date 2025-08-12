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

#ifndef TEST_I_CAMERA_ML_CONFIGURATION_H
#define TEST_I_CAMERA_ML_CONFIGURATION_H

#include "ace/Synch_Traits.h"

#include "common_configuration.h"
#include "common_isubscribe.h"

#include "stream_messageallocatorheap_base.h"

#include "test_i_camera_ml_stream_common.h"

#include "test_i_configuration.h"

// forward declarations
template <typename NotificationType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_CameraML_EventHandler_T;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CameraML_DirectShow_Configuration
 : Test_I_Configuration
{
  Stream_CameraML_DirectShow_Configuration ()
   : Test_I_Configuration ()
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration  direct3DConfiguration;
  Stream_CameraML_DirectShow_StreamConfiguration_t streamConfiguration;
};

struct Stream_CameraML_MediaFoundation_Configuration
 : Test_I_Configuration
{
  Stream_CameraML_MediaFoundation_Configuration ()
   : Test_I_Configuration ()
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration       direct3DConfiguration;
  Stream_CameraML_MediaFoundation_StreamConfiguration_t streamConfiguration;
};
#else
struct Stream_CameraML_Configuration
 : Test_I_Configuration
{
  Stream_CameraML_Configuration ()
   : Test_I_Configuration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  Stream_CameraML_StreamConfiguration_t streamConfiguration;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_DirectShowAllocatorBase_T<struct Stream_AllocatorConfiguration,
//                                         Stream_ControlMessage_t,
//                                         Stream_CameraML_Message,
//                                         Stream_CameraML_SessionMessage> Stream_CameraML_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_CameraML_DirectShow_Message_t,
                                          Stream_CameraML_DirectShow_SessionMessage_t> Stream_CameraML_DirectShow_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_CameraML_MediaFoundation_Message_t,
                                          Stream_CameraML_MediaFoundation_SessionMessage_t> Stream_CameraML_MediaFoundation_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_CameraML_Message_t,
                                          Stream_CameraML_SessionMessage_t> Stream_CameraML_MessageAllocator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_ISubscribe_T<Stream_CameraML_DirectShow_ISessionNotify_t> Stream_CameraML_DirectShow_ISubscribe_t;
typedef Common_ISubscribe_T<Stream_CameraML_MediaFoundation_ISessionNotify_t> Stream_CameraML_MediaFoundation_ISubscribe_t;

typedef Stream_CameraML_EventHandler_T<Stream_CameraML_DirectShow_ISessionNotify_t,
                                       Stream_CameraML_DirectShow_Message_t,
                                       Stream_CameraML_DirectShow_SessionMessage_t> Stream_CameraML_DirectShow_EventHandler_t;
typedef Stream_CameraML_EventHandler_T<Stream_CameraML_MediaFoundation_ISessionNotify_t,
                                       Stream_CameraML_MediaFoundation_Message_t,
                                       Stream_CameraML_MediaFoundation_SessionMessage_t> Stream_CameraML_MediaFoundation_EventHandler_t;
#else
typedef Common_ISubscribe_T<Stream_CameraML_ISessionNotify_t> Stream_CameraML_ISubscribe_t;

typedef Stream_CameraML_EventHandler_T<Stream_CameraML_ISessionNotify_t,
                                       Stream_CameraML_Message_t,
                                       Stream_CameraML_SessionMessage_t> Stream_CameraML_EventHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif // TEST_I_CAMERA_ML_CONFIGURATION_H
