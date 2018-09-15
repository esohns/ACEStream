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

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "linux/videodev2.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Synch_Traits.h"

#include "common.h"
#include "common_statistic_handler.h"
#include "common_time_common.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_data_base.h"
//#include "stream_inotify.h"
#include "stream_session_data.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#else
#include "stream_dev_defines.h"
#endif // ACE_WIN32 || ACE_WIN64

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IMediaSample;
struct IMFSample;
#endif // ACE_WIN32 || ACE_WIN64
class Stream_IAllocator;
//class Test_I_Stream_Message;
//class Test_I_Stream_SessionMessage;

typedef int Stream_HeaderType_t;
typedef int Stream_CommandType_t;

typedef Stream_Statistic Test_I_Statistic_t;
typedef Common_IStatistic_T<Test_I_Statistic_t> Test_I_IStatisticHandler_t;
typedef Common_StatisticHandler_T<Test_I_Statistic_t> Test_I_StatisticHandler_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_MessageData
{
  Test_I_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {}

  IMediaSample* sample;
  double        sampleTime;
};
typedef Stream_DataBase_T<struct Test_I_DirectShow_MessageData> Test_I_DirectShow_MessageData_t;

struct Test_I_MediaFoundation_MessageData
{
  Test_I_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {}

  IMFSample* sample;
  LONGLONG   sampleTime;
};
typedef Stream_DataBase_T<struct Test_I_MediaFoundation_MessageData> Test_I_MediaFoundation_MessageData_t;
#else
struct Test_I_V4L2_MessageData
{
  Test_I_V4L2_MessageData ()
   : device (-1)
   , index (0)
   , method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , release (false)
  {}

  int         device; // (capture) device file descriptor
  __u32       index;  // 'index' field of v4l2_buffer
  v4l2_memory method;
  bool        release;
};
typedef Stream_DataBase_T<struct Test_I_V4L2_MessageData> Test_I_V4L2_MessageData_t;
#endif // ACE_WIN32 || ACE_WIN64

//struct Test_I_ConnectionConfiguration;
//struct Test_I_StreamConfiguration;
struct Test_I_UserData
 : Stream_UserData
{
  Test_I_UserData ()
   : Stream_UserData ()
  {}
};

struct Test_I_ConnectionState;
struct Test_I_SessionData
 : Stream_SessionData
{
  Test_I_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , userData (NULL)
  {}

  struct Test_I_SessionData& operator+= (const struct Test_I_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    connectionState = (connectionState ? connectionState : rhs_in.connectionState);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  struct Test_I_ConnectionState* connectionState;

  struct Test_I_UserData*        userData;
};
typedef Stream_SessionData_T<struct Test_I_SessionData> Test_I_SessionData_t;

struct Test_I_StreamState
 : Stream_State
{
  Test_I_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   , userData (NULL)
  {}

  struct Test_I_SessionData* sessionData;

  struct Test_I_UserData*    userData;
};

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct Test_I_UI_ProgressData
{
  Test_I_UI_ProgressData ()
   : state (NULL)
   , statistic ()
  {
    ACE_OS::memset (&statistic, 0, sizeof (Test_I_Statistic_t));
  }

  struct Common_UI_State* state;
  Test_I_Statistic_t      statistic;
};

struct Test_I_UI_CBData
{
  Test_I_UI_CBData ()
   : allowUserRuntimeStatistic (true)
   //, configuration (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (MODULE_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , progressData ()
   , UIState ()
  {
    progressData.state = &UIState;
  }

  bool                            allowUserRuntimeStatistic;
  //struct Test_U_Configuration*    configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  struct Test_I_UI_ProgressData   progressData;
  struct Common_UI_State          UIState;
};

struct Test_I_UI_ThreadData
{
  Test_I_UI_ThreadData ()
   : CBData (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (MODULE_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , sessionId (0)
  {}

  struct Test_I_UI_CBData*        CBData;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  size_t                          sessionId;
};
#endif // GUI_SUPPORT

#endif
