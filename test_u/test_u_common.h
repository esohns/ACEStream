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

#ifndef TEST_U_COMMON_H
#define TEST_U_COMMON_H

#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "linux/videodev2.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#include "ace/OS.h"
#endif // GUI_SUPPORT

#include "common_configuration.h"
#include "common_file_common.h"
#include "common_istatistic.h"
#include "common_statistic_handler.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_session_data.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//#include "stream_dev_defines.h"

#include "stream_lib_alsa_common.h"

#include "stream_lib_v4l_common.h"
#include "stream_lib_v4l_defines.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_lib_common.h"
#include "stream_lib_defines.h"

#include "test_u_defines.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_u_gtk_common.h"
#endif // GTK_SUPPORT
#if defined (QT_SUPPORT)
#include "test_u_qt_common.h"
#endif // QT_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "test_u_wxwidgets_common.h"
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IMediaSample;
struct IMFSample;
#endif // ACE_WIN32 || ACE_WIN64
class ACE_Message_Queue_Base;
struct Test_U_Configuration;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_DirectShow_MessageData
{
  Test_U_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {}

  IMediaSample* sample;
  double        sampleTime;
};
typedef Stream_DataBase_T<struct Test_U_DirectShow_MessageData> Test_U_DirectShow_MessageData_t;

struct Test_U_MediaFoundation_MessageData
{
  Test_U_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {}

  IMFSample* sample;
  LONGLONG   sampleTime;
};
typedef Stream_DataBase_T<struct Test_U_MediaFoundation_MessageData> Test_U_MediaFoundation_MessageData_t;
#else
struct Test_U_V4L2_MessageData
{
  Test_U_V4L2_MessageData ()
   : fileDescriptor (-1)
   , index (0)
   , method (STREAM_LIB_V4L_DEFAULT_IO_METHOD)
   , release (false)
  {}

  int         fileDescriptor; // (capture) device file descriptor
  __u32       index;  // 'index' field of v4l2_buffer
  v4l2_memory method;
  bool        release;
};
typedef Stream_DataBase_T<struct Test_U_V4L2_MessageData> Test_U_V4L2_MessageData_t;
#endif // ACE_WIN32 || ACE_WIN64

struct Test_U_SessionData
 : Stream_SessionData
{
  Test_U_SessionData ()
   : Stream_SessionData ()
   , fileIdentifier ()
  {}

  struct Test_U_SessionData& operator+= (const struct Test_U_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    fileIdentifier =
      (fileIdentifier.empty () ? rhs_in.fileIdentifier : fileIdentifier);

    return *this;
  }

  struct Common_File_Identifier fileIdentifier; // target-
};
typedef Stream_SessionData_T<struct Test_U_SessionData> Test_U_SessionData_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_U_DirectShow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                        struct _AMMediaType,
                                        struct Stream_State,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_U_DirectShow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                   struct _AMMediaType,
                                   struct Stream_State,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_U_DirectShow_SessionData& operator+= (const Test_U_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  struct _AMMediaType,
                                  struct Stream_State,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_U_DirectShow_SessionData> Test_U_DirectShow_SessionData_t;

class Test_U_MediaFoundation_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                        IMFMediaType*,
                                        struct Stream_State,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_U_MediaFoundation_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                   IMFMediaType*,
                                   struct Stream_State,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_U_MediaFoundation_SessionData& operator+= (const Test_U_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  IMFMediaType*,
                                  struct Stream_State,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_U_MediaFoundation_SessionData> Test_U_MediaFoundation_SessionData_t;
#else
class Test_U_ALSA_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                        struct Stream_MediaFramework_ALSA_MediaType,
                                        struct Stream_State,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_U_ALSA_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                   struct Stream_MediaFramework_ALSA_MediaType,
                                   struct Stream_State,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_U_ALSA_SessionData& operator+= (const Test_U_ALSA_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  struct Stream_MediaFramework_ALSA_MediaType,
                                  struct Stream_State,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_U_ALSA_SessionData> Test_U_ALSA_SessionData_t;

class Test_U_V4L_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                        struct Stream_MediaFramework_V4L_MediaType,
                                        struct Stream_State,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_U_V4L_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                   struct Stream_MediaFramework_V4L_MediaType,
                                   struct Stream_State,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_U_V4L_SessionData& operator+= (const Test_U_V4L_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  struct Stream_MediaFramework_V4L_MediaType,
                                  struct Stream_State,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_U_V4L_SessionData> Test_U_V4L_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

//typedef int Stream_HeaderType_t;
typedef int Stream_CommandType_t;

//////////////////////////////////////////

struct Test_U_ProgressData
{
  Test_U_ProgressData ()
   : sessionId (0)
   , statistic ()
  {
    ACE_OS::memset (&statistic, 0, sizeof (struct Stream_Statistic));
  }

  Stream_SessionId_t      sessionId;
  struct Stream_Statistic statistic;
};

struct Test_U_CBData
{
  Test_U_CBData ()
   : allowUserRuntimeStatistic (true)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , progressData ()
  {}

  bool                            allowUserRuntimeStatistic;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  struct Test_U_ProgressData      progressData;
};

struct Test_U_ThreadData
{
  Test_U_ThreadData ()
   : CBData (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
  {}

  struct Test_U_CBData*           CBData;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
};

#if defined (GUI_SUPPORT)
struct Test_U_UI_ProgressData
#if defined (GTK_USE)
 : Test_U_GTK_ProgressData
#elif defined (QT_USE)
 : Test_U_Qt_ProgressData
#elif defined (WXWIDGETS_USE)
 : Test_U_wxWidgets_ProgressData
#else
 : Test_U_ProgressData
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
{
  Test_U_UI_ProgressData ()
#if defined (GTK_USE)
   : Test_U_GTK_ProgressData ()
#elif defined (QT_USE)
   : Test_U_Qt_ProgressData ()
#elif defined (WXWIDGETS_USE)
   : Test_U_wxWidgets_ProgressData ()
#else
   : Test_U_ProgressData ()
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
   , sessionId (0)
   , statistic ()
  {
    ACE_OS::memset (&statistic, 0, sizeof (struct Stream_Statistic));
  }

  Stream_SessionId_t      sessionId;
  struct Stream_Statistic statistic;
};

struct Test_U_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#elif defined (QT_USE)
 : Test_U_Qt_CBData
#elif defined (WXWIDGETS_USE)
 : Test_U_wxWidgets_CBData
#else
 : Test_U_CBData
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
{
  Test_U_UI_CBData ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
#elif defined (QT_USE)
   : Test_U_Qt_CBData ()
#elif defined (WXWIDGETS_USE)
   : Test_U_wxWidgets_CBData ()
#else
   : Test_U_CBData ()
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
   , allowUserRuntimeStatistic (true)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , progressData ()
  {
#if defined (GTK_USE) || defined (QT_USE) || defined (WXWIDGETS_USE)
    progressData.state = UIState;
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
  }

  bool                            allowUserRuntimeStatistic;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  struct Test_U_UI_ProgressData   progressData;
};

struct Test_U_UI_ThreadData
#if defined (GTK_USE)
 : Test_U_GTK_ThreadData
#elif defined (QT_USE)
 : Test_U_Qt_ThreadData
#elif defined (WXWIDGETS_USE)
 : Test_U_wxWidgets_ThreadData
#else
 : Test_U_ThreadData
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
{
  Test_U_UI_ThreadData ()
#if defined (GTK_USE)
   : Test_U_GTK_ThreadData ()
#elif defined (QT_USE)
   : Test_U_Qt_ThreadData ()
#elif defined (WXWIDGETS_USE)
   : Test_U_wxWidgets_ThreadData ()
#else
   : Test_U_ThreadData ()
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
   , CBData (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
  {}

  struct Test_U_UI_CBData*        CBData;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
};
#endif // GUI_SUPPORT

#endif
