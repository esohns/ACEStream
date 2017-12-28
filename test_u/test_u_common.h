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
#endif

#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common.h"
#endif
#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_data_base.h"
#include "stream_session_data.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_dev_defines.h"
#endif
#include "stream_lib_common.h"
#include "stream_lib_defines.h"

#include "test_u_defines.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IMediaSample;
struct IMFSample;
#endif
class ACE_Message_Queue_Base;
struct Test_U_Configuration;

typedef Stream_Statistic Test_U_Statistic_t;
typedef Common_IStatistic_T<Test_U_Statistic_t> Test_U_StatisticReportingHandler_t;
typedef Common_StatisticHandler_T<Test_U_Statistic_t> Test_U_StatisticHandler_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_DirectShow_MessageData
{
  Test_U_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {};

  IMediaSample* sample;
  double        sampleTime;
};
typedef Stream_DataBase_T<struct Test_U_DirectShow_MessageData> Test_U_DirectShow_MessageData_t;
struct Test_U_MediaFoundation_MessageData
{
  Test_U_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {};

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
   , method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , release (false)
  {};

  int         fileDescriptor; // (capture) device file descriptor
  __u32       index;  // 'index' field of v4l2_buffer
  v4l2_memory method;
  bool        release;
};
typedef Stream_DataBase_T<struct Test_U_V4L2_MessageData> Test_U_V4L2_MessageData_t;
#endif

struct Test_U_UserData
 : Stream_UserData
{
  Test_U_UserData ()
   : Stream_UserData ()
   //, configuration (NULL)
  {};

  //struct Test_U_Configuration* configuration;
};

struct Test_U_SessionData
 : Stream_SessionData
{
  Test_U_SessionData ()
   : Stream_SessionData ()
   //, currentStatistic ()
   , targetFileName ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , useMediaFoundation (MODULE_LIB_DEFAULT_MEDIAFRAMEWORK == STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION)
#endif
   , userData (NULL)
  {};
  inline Test_U_SessionData& operator+= (const struct Test_U_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    //// *NOTE*: the idea is to 'merge' the data
    //currentStatistic += rhs_in.currentStatistic;
    targetFileName =
      (targetFileName.empty () ? rhs_in.targetFileName : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  //Test_U_Statistic_t currentStatistic;
  std::string               targetFileName;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool                      useMediaFoundation;
#endif
  struct Test_U_UserData*   userData;
};
typedef Stream_SessionData_T<struct Test_U_SessionData> Test_U_SessionData_t;

//struct Test_U_StreamState
//{
//  inline Test_U_StreamState ()
//   : currentSessionData (NULL)
//   , userData (NULL)
//  {};
//
//  struct Test_U_SessionData* currentSessionData;
//  struct Test_U_UserData*    userData;
//};

//typedef int Stream_HeaderType_t;
typedef int Stream_CommandType_t;

struct Test_U_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  Test_U_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , fileName ()
   , inbound (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , manageCOM (false)
   , useMediaFoundation (MODULE_LIB_DEFAULT_MEDIAFRAMEWORK == STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION)
#endif
   , printProgressDot (false)
   , pushStatisticMessages (true)
//   , streamConfiguration (NULL)
  {};

  std::string                  fileName;              // file writer module
  bool                         inbound;               // statistic/IO module
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool                         manageCOM;
  bool                         useMediaFoundation;    // display module
#endif
  bool                         printProgressDot;      // file writer module
  bool                         pushStatisticMessages; // statistic module

//  // *TODO*: remove this ASAP
//  struct Stream_Configuration* streamConfiguration;
};

struct Test_U_StreamConfiguration
 : Stream_Configuration
{
  Test_U_StreamConfiguration ()
   : Stream_Configuration ()
  {};
};

struct Test_U_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_U_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , useMediaFoundation (MODULE_LIB_DEFAULT_MEDIAFRAMEWORK == STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION)
#endif
  {};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool useMediaFoundation;
#endif
};

struct Test_U_Configuration
{
  Test_U_Configuration ()
   : signalHandlerConfiguration ()
//   , streamConfiguration ()
   , userData ()
  {};

  struct Test_U_SignalHandlerConfiguration signalHandlerConfiguration;
//  Stream_Configuration_t                   streamConfiguration;

  struct Test_U_UserData                   userData;
};

#endif
