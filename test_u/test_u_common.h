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

#include <deque>
#include <string>

#include "gtk/gtk.h"

#include "common.h"

#include "ace/Synch_Traits.h"

#include "common_ui_common.h"

#include "stream_common.h"

// forward declarations
class ACE_Message_Queue_Base;
struct Stream_Test_U_Configuration;

enum Stream_GTK_Event
{
  STREAM_GKTEVENT_INVALID = -1,
  // ------------------------------------
  STREAM_GTKEVENT_START = 0,
  STREAM_GTKEVENT_DATA,
  STREAM_GTKEVENT_END,
  STREAM_GTKEVENT_STATISTIC,
  // ------------------------------------
  STREAM_GTKEVENT_MAX
};
typedef std::deque<Stream_GTK_Event> Stream_GTK_Events_t;
typedef Stream_GTK_Events_t::const_iterator Stream_GTK_EventsIterator_t;

//struct Stream_Test_U_UserData
// : Stream_UserData
//{
//  inline Stream_Test_U_UserData ()
//   : Stream_UserData ()
//   //, configuration (NULL)
//  {};
//
//  //Stream_Test_U_Configuration* configuration;
//};

//struct Stream_Test_U_StreamState
//{
//  inline Stream_Test_U_StreamState ()
//   : currentSessionData (NULL)
//   , userData (NULL)
//  {};
//
//  Stream_Test_U_SessionData* currentSessionData;
//  Stream_Test_U_UserData*    userData;
//};

typedef int Stream_HeaderType_t;
typedef int Stream_CommandType_t;

//typedef Stream_SessionDataBase_T<Stream_Test_U_SessionData> Stream_Test_U_SessionData_t;

struct Stream_Test_U_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  inline Stream_Test_U_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , active (false)
   , contextID (0)
   , queue (NULL)
   , fileName ()
   , printProgressDot (false)
   , targetFileName ()
  {};

  bool                    active;
  guint                   contextID;
  ACE_Message_Queue_Base* queue;
  std::string             fileName;
  bool                    printProgressDot;
  std::string             targetFileName;
};

struct Stream_Test_U_StreamConfiguration
 : Stream_Configuration
{
  inline Stream_Test_U_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration_2 ()
   , moduleHandlerConfiguration_2 ()
  {};

  Stream_ModuleConfiguration               moduleConfiguration_2;
  Stream_Test_U_ModuleHandlerConfiguration moduleHandlerConfiguration_2;
};

struct Stream_Test_U_Configuration
{
  inline Stream_Test_U_Configuration ()
   : streamConfiguration ()
   , streamUserData ()
  {};

  Stream_Test_U_StreamConfiguration streamConfiguration;
  Stream_UserData                   streamUserData;
};

struct Stream_Test_U_GTK_CBData
 : Common_UI_GTKState
{
  inline Stream_Test_U_GTK_CBData ()
   : Common_UI_GTKState ()
   , allowUserRuntimeStatistic (true)
   , configuration (NULL)
   , eventStack ()
   , logStack ()
  {};

  bool                         allowUserRuntimeStatistic;
  Stream_Test_U_Configuration* configuration;
  Stream_GTK_Events_t          eventStack;
  Common_MessageStack_t        logStack;
};

#endif
