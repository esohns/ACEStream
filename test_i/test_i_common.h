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

#ifndef TEST_I_COMMON_H
#define TEST_I_COMMON_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "mfobjects.h"
#include "strmif.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/config-lite.h"
#include "ace/Synch_Traits.h"

#include "common.h"
#include "common_configuration.h"
#include "common_file_common.h"
#include "common_istatistic.h"
#include "common_statistic_handler.h"
#include "common_time_common.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_session_data.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#else
#include "stream_lib_v4l_defines.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_i_gtk_common.h"
#endif // GTK_SUPPORT
#if defined (QT_SUPPORT)
#include "test_i_qt_common.h"
#endif // QT_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "test_i_wxwidgets_common.h"
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

enum Test_I_ProgramMode
{
  TEST_I_PROGRAMMODE_PRINT_VERSION = 0,
  TEST_I_PROGRAMMODE_NORMAL,
  ////////////////////////////////////////
  TEST_I_PROGRAMMODE_MAX,
  TEST_I_PROGRAMMODE_INVALID
};

struct Test_I_MessageData
{
  Test_I_MessageData ()
  {}

  struct Test_I_MessageData& operator+= (const struct Test_I_MessageData& rhs_in)
  {
    return *this;
  }
};
typedef Stream_DataBase_T<struct Test_I_MessageData> Test_I_MessageData_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_MessageData
 : Test_I_MessageData
{
  Test_I_DirectShow_MessageData ()
   : Test_I_MessageData ()
   , sample (NULL)
   , sampleTime (0)
  {}

  // audio/video
  IMediaSample* sample;
  double        sampleTime;
};
typedef Stream_DataBase_T<struct Test_I_DirectShow_MessageData> Test_I_DirectShow_MessageData_t;

struct Test_I_MediaFoundation_MessageData
 : Test_I_MessageData
{
  Test_I_MediaFoundation_MessageData ()
   : Test_I_MessageData ()
   , sample (NULL)
   , sampleTime (0)
  {}

  // audio/video
  IMFSample* sample;
  LONGLONG   sampleTime;
};
typedef Stream_DataBase_T<struct Test_I_MediaFoundation_MessageData> Test_I_MediaFoundation_MessageData_t;
#else
struct Test_I_ALSA_MessageData
 : Test_I_MessageData
{
  Test_I_ALSA_MessageData ()
   : Test_I_MessageData ()
   , deviceHandle (NULL)
   , release (false)
  {}

  struct _snd_pcm* deviceHandle; // (capture) device handle
  bool             release;
};

struct Test_I_V4L_MessageData
 : Test_I_MessageData
{
  Test_I_V4L_MessageData ()
   : Test_I_MessageData ()
   , device (-1)
   , index (0)
   , method (STREAM_LIB_V4L_DEFAULT_IO_METHOD)
   , release (false)
  {}

  int         device; // (capture) device file descriptor
  __u32       index;  // 'index' field of v4l2_buffer
  v4l2_memory method;
  bool        release;
};
typedef Stream_DataBase_T<struct Test_I_V4L_MessageData> Test_I_V4L_MessageData_t;
#endif // ACE_WIN32 || ACE_WIN64

struct Test_I_SessionData
 : Stream_SessionData
{
  Test_I_SessionData ()
   : Stream_SessionData ()
   , fileIdentifier ()
   , userData (NULL)
  {}

  struct Test_I_SessionData& operator+= (const struct Test_I_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    fileIdentifier =
      (fileIdentifier.empty () ? rhs_in.fileIdentifier : fileIdentifier);

    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  struct Common_File_Identifier fileIdentifier; // target-

  struct Stream_UserData* userData;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_DirectShow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        struct _AMMediaType,
                                        struct Stream_State,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_DirectShow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   struct _AMMediaType,
                                   struct Stream_State,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_I_DirectShow_SessionData& operator+= (const Test_I_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  struct _AMMediaType,
                                  struct Stream_State,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_I_DirectShow_SessionData> Test_I_DirectShow_SessionData_t;

class Test_I_MediaFoundation_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        IMFMediaType*,
                                        struct Stream_State,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_MediaFoundation_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   IMFMediaType*,
                                   struct Stream_State,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , session (NULL)
  {}

  Test_I_MediaFoundation_SessionData& operator+= (const Test_I_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  IMFMediaType*,
                                  struct Stream_State,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }

  IMFMediaSession* session;
};
typedef Stream_SessionData_T<Test_I_MediaFoundation_SessionData> Test_I_MediaFoundation_SessionData_t;
#else
class Test_I_ALSA_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        struct Stream_MediaFramework_ALSA_MediaType,
                                        struct Stream_State,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_ALSA_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   struct Stream_MediaFramework_ALSA_MediaType,
                                   struct Stream_State,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_I_ALSA_SessionData& operator+= (const Test_I_ALSA_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  struct Stream_MediaFramework_ALSA_MediaType,
                                  struct Stream_State,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_I_ALSA_SessionData> Test_I_ALSA_SessionData_t;

class Test_I_V4L_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        struct Stream_MediaFramework_V4L_MediaType,
                                        struct Stream_State,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_V4L_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   struct Stream_MediaFramework_V4L_MediaType,
                                   struct Stream_State,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_I_V4L_SessionData& operator+= (const Test_I_V4L_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  struct Stream_MediaFramework_V4L_MediaType,
                                  struct Stream_State,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_I_V4L_SessionData> Test_I_V4L_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_StreamState
 : Stream_State
{
  Test_I_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_DirectShow_SessionData* sessionData;
};

struct Test_I_MediaFoundation_StreamState
 : Stream_State
{
  Test_I_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_MediaFoundation_SessionData* sessionData;
};
#else
struct Test_I_ALSA_StreamState
 : Stream_State
{
  Test_I_ALSA_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_ALSA_SessionData* sessionData;
};
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

struct Test_I_ProgressData
{
  Test_I_ProgressData ()
   : sessionId (0)
   , statistic ()
  {
    ACE_OS::memset (&statistic, 0, sizeof (struct Stream_Statistic));
  }

  Stream_SessionId_t      sessionId;
  struct Stream_Statistic statistic;
};

struct Test_I_CBData
{
  Test_I_CBData ()
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
  struct Test_I_ProgressData      progressData;
};

struct Test_I_ThreadData
{
  Test_I_ThreadData ()
   : CBData (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
  {}

  struct Test_I_CBData*           CBData;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
};

#if defined (GUI_SUPPORT)
struct Test_I_UI_ProgressData
#if defined (GTK_USE)
 : Test_I_GTK_ProgressData
#elif defined (QT_USE)
 : Test_I_Qt_ProgressData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_ProgressData
#else
 : Test_I_ProgressData
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
{
  Test_I_UI_ProgressData ()
#if defined (GTK_USE)
   : Test_I_GTK_ProgressData ()
#elif defined (QT_USE)
   : Test_I_Qt_ProgressData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_ProgressData ()
#else
   : Test_I_ProgressData ()
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
   , sessionId (0)
  {}

  Stream_SessionId_t sessionId;
};

struct Test_I_UI_CBData
#if defined (GTK_USE)
 : Test_I_GTK_CBData
#elif defined (QT_USE)
 : Test_I_Qt_CBData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_CBData
#else
 : Test_I_CBData
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
{
  Test_I_UI_CBData ()
#if defined (GTK_USE)
   : Test_I_GTK_CBData ()
#elif defined (QT_USE)
   : Test_I_Qt_CBData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_CBData ()
#else
   : Test_I_CBData ()
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
  struct Test_I_UI_ProgressData   progressData;
};

struct Test_I_UI_ThreadData
#if defined (GTK_USE)
 : Test_I_GTK_ThreadData
#elif defined (QT_USE)
 : Test_I_Qt_ThreadData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_ThreadData
#else
 : Test_I_ThreadData
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
{
  Test_I_UI_ThreadData ()
#if defined (GTK_USE)
   : Test_I_GTK_ThreadData ()
#elif defined (QT_USE)
   : Test_I_Qt_ThreadData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_ThreadData ()
#else
   : Test_I_ThreadData ()
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
   , CBData (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
  {}

  struct Test_I_UI_CBData*        CBData;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
};
#endif // GUI_SUPPORT

#endif
