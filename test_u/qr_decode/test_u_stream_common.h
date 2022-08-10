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

#ifndef TEST_U_STREAM_COMMON_H
#define TEST_U_STREAM_COMMON_H

#include <list>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#include "stream_dev_common.h"
#include "stream_dev_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

// forward declarations
//struct Test_U_AllocatorConfiguration;
class Test_U_Message;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_U_DirectShow_SessionMessage;
#else
class Test_U_SessionMessage;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_DirectShow_StreamState;
class QRDecode_DirectShow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                        struct _AMMediaType,
                                        struct Test_U_DirectShow_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  QRDecode_DirectShow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                   struct _AMMediaType,
                                   struct Test_U_DirectShow_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  QRDecode_DirectShow_SessionData& operator= (const QRDecode_DirectShow_SessionData& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                  struct _AMMediaType,
                                  struct Test_U_DirectShow_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator= (rhs_in);

    return *this;
  }
  QRDecode_DirectShow_SessionData& operator+= (const QRDecode_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                  struct _AMMediaType,
                                  struct Test_U_DirectShow_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    return *this;
  }
};
typedef Stream_SessionData_T<QRDecode_DirectShow_SessionData> QRDecode_DirectShow_SessionData_t;
#else
class QRDecode_SessionData
 : public Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                        struct Stream_MediaFramework_V4L_MediaType,
                                        struct Test_U_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  QRDecode_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                   struct Stream_MediaFramework_V4L_MediaType,
                                   struct Test_U_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  QRDecode_SessionData& operator= (const QRDecode_SessionData& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                  struct Stream_MediaFramework_V4L_MediaType,
                                  struct Test_U_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator= (rhs_in);

    return *this;
  }
  QRDecode_SessionData& operator+= (const QRDecode_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    return *this;
  }
};
typedef Stream_SessionData_T<QRDecode_SessionData> QRDecode_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_ISessionDataNotify_T<QRDecode_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_DirectShow_SessionMessage> Test_U_Notification_t;
typedef std::list<Test_U_Notification_t*> Test_U_Subscribers_t;
typedef Test_U_Subscribers_t::iterator Test_U_SubscribersIterator_t;
#else
typedef Stream_ISessionDataNotify_T<struct QRDecode_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> Test_U_Notification_t;
typedef std::list<Test_U_Notification_t*> Test_U_Subscribers_t;
typedef Test_U_Subscribers_t::iterator Test_U_SubscribersIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

// forward declarations
//extern const char stream_name_string_[];
struct QRDecode_StreamConfiguration
 : Stream_Configuration
{
  QRDecode_StreamConfiguration ()
   : Stream_Configuration ()
   , format ()
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_OS::memset (&format, 0, sizeof (struct _AMMediaType));
#endif // ACE_WIN32 || ACE_WIN64
  }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType format;
#else
  struct Stream_MediaFramework_V4L_MediaType format;
#endif // ACE_WIN32 || ACE_WIN64
};
struct QRDecode_ModuleHandlerConfiguration;
typedef Stream_Configuration_T< // stream_name_string_,
                               struct QRDecode_StreamConfiguration,
                               struct QRDecode_ModuleHandlerConfiguration> Test_U_StreamConfiguration_t;
struct QRDecode_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  QRDecode_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , builder (NULL)
#else
   , buffers (STREAM_LIB_V4L_DEFAULT_DEVICE_BUFFERS)
#endif // ACE_WIN32 || ACE_WIN64
   , deviceIdentifier ()
   , outputFormat ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , preview (STREAM_DEV_CAM_VIDEOFORWINDOW_DEFAULT_PREVIEW_MODE)
#else
   , method (STREAM_LIB_V4L_DEFAULT_IO_METHOD)
#endif // ACE_WIN32 || ACE_WIN64
   , pushStatisticMessages (true)
   , subscriber (NULL)
   , subscribers (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , window (NULL)
#endif // ACE_WIN32 || ACE_WIN64
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_OS::memset (&outputFormat, 0, sizeof (struct _AMMediaType));
#endif // ACE_WIN32 || ACE_WIN64  
  }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IGraphBuilder*                             builder; // DirectShow source
#else
  __u32                                      buffers; // v4l device buffers
#endif // ACE_WIN32 || ACE_WIN64
  struct Stream_Device_Identifier            deviceIdentifier;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType                        outputFormat; // DirectShow source
  bool                                       preview; // VfW source
#else
  struct Stream_MediaFramework_V4L_MediaType outputFormat;
  enum v4l2_memory                           method;
#endif // ACE_WIN32 || ACE_WIN64
  bool                                       pushStatisticMessages;
  Test_U_Notification_t*                     subscriber;
  Test_U_Subscribers_t*                      subscribers;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND                                       window;
#endif // ACE_WIN32 || ACE_WIN64
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_DirectShow_StreamState
 : Stream_State
{
  Test_U_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  QRDecode_DirectShow_SessionData* sessionData;
};
#else
struct Test_U_StreamState
 : Stream_State
{
  Test_U_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   //, userData (NULL)
  {}

  struct QRDecode_SessionData* sessionData;

  //struct Test_U_UserData*    userData;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_DirectShow_SessionMessage> Test_U_DirectShow_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> Test_U_MessageAllocator_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
