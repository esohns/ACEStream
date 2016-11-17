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

#include <deque>
#include <string>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <linux/videodev2.h>
#endif

#include <ace/Synch_Traits.h>

#include "common.h"

#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_session_data.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_dev_defines.h"
#endif

#include "test_u_defines.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IMediaSample;
struct IMFSample;
#endif
class ACE_Message_Queue_Base;
struct Test_U_Configuration;

typedef Stream_Statistic Test_U_RuntimeStatistic_t;

typedef Common_IStatistic_T<Test_U_RuntimeStatistic_t> Test_U_StatisticReportingHandler_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_DirectShow_MessageData
{
  inline Test_U_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {};

  IMediaSample* sample;
  double        sampleTime;
};
typedef Stream_DataBase_T<Test_U_DirectShow_MessageData> Test_U_DirectShow_MessageData_t;
struct Test_U_MediaFoundation_MessageData
{
  inline Test_U_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {};

  IMFSample* sample;
  LONGLONG   sampleTime;
};
typedef Stream_DataBase_T<Test_U_MediaFoundation_MessageData> Test_U_MediaFoundation_MessageData_t;
#else
struct Test_U_V4L2_MessageData
{
  inline Test_U_V4L2_MessageData ()
   : device (-1)
   , index (0)
   , method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , release (false)
  {};

  int         device; // (capture) device file descriptor
  __u32       index;  // 'index' field of v4l2_buffer
  v4l2_memory method;
  bool        release;
};
typedef Stream_DataBase_T<Test_U_V4L2_MessageData> Test_U_V4L2_MessageData_t;
#endif

struct Test_U_UserData
 : Stream_UserData
{
  inline Test_U_UserData ()
   : Stream_UserData ()
   //, configuration (NULL)
  {};

  //Test_U_Configuration* configuration;
};

struct Test_U_SessionData
 : Stream_SessionData
{
  inline Test_U_SessionData ()
   : Stream_SessionData ()
   //, currentStatistic ()
   , targetFileName ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , useMediaFoundation (TEST_U_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION)
#endif
   , userData (NULL)
  {};
  inline Test_U_SessionData& operator+= (const Test_U_SessionData& rhs_in)
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

  //Test_U_RuntimeStatistic_t currentStatistic;
  std::string               targetFileName;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool                      useMediaFoundation;
#endif
  Test_U_UserData*          userData;
};
typedef Stream_SessionData_T<Test_U_SessionData> Test_U_SessionData_t;

//struct Test_U_StreamState
//{
//  inline Test_U_StreamState ()
//   : currentSessionData (NULL)
//   , userData (NULL)
//  {};
//
//  Test_U_SessionData* currentSessionData;
//  Test_U_UserData*    userData;
//};

//typedef int Stream_HeaderType_t;
typedef int Stream_CommandType_t;

struct Test_U_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  inline Test_U_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , fileName ()
   , inbound (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , manageCOM (false)
#endif
   , printFinalReport (true)
   , printProgressDot (false)
   , pushStatisticMessages (true)
  {};

  std::string fileName;              // file writer module
  bool        inbound;               // statistic/IO module
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool        manageCOM;
#endif
  bool        printFinalReport;      // statistic module
  bool        printProgressDot;      // file writer module
  bool        pushStatisticMessages; // statistic module
};

struct Test_U_StreamConfiguration
 : Stream_Configuration
{
  inline Test_U_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
  {};

  Test_U_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct Test_U_Configuration
{
  inline Test_U_Configuration ()
   : allocatorConfiguration ()
   , moduleConfiguration ()
   , streamConfiguration ()
   , streamUserData ()
  {};

  Stream_AllocatorConfiguration     allocatorConfiguration;
  Stream_ModuleConfiguration        moduleConfiguration;
  Test_U_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Test_U_StreamConfiguration        streamConfiguration;

  Stream_UserData                   streamUserData;
};

#endif
