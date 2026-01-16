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

#ifndef STREAM_MISC_COMMON_H
#define STREAM_MISC_COMMON_H

#include <list>

#include "ace/Basic_Types.h"
#include "ace/Time_Value.h"

#include "common_input_common.h"

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_data_base.h"
#include "stream_isessionnotify.h"
#include "stream_message_base.h"
#include "stream_session_data.h"
#include "stream_session_message_base.h"

// forward declarations
class ACE_Message_Queue_Base;

enum Stream_Miscellaneous_DelayModeType
{
  STREAM_MISCELLANEOUS_DELAY_MODE_BYTES = 0,
  STREAM_MISCELLANEOUS_DELAY_MODE_MESSAGES,
  STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER, // support isochronicity (timer-based)
  STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER_BYTES, // support isochronicity (timer-based)
  ////////////////////////////////////////
  STREAM_MISCELLANEOUS_DELAY_MODE_MAX,
  STREAM_MISCELLANEOUS_DELAY_MODE_INVALID = -1
};

struct Stream_Miscellaneous_DelayConfiguration
{
  Stream_Miscellaneous_DelayConfiguration ()
   : averageTokensPerInterval (0)
   , catchUp (false)
   , interval (ACE_Time_Value::zero)
   , isMultimediaTask (false)
   , mode (STREAM_MISCELLANEOUS_DELAY_MODE_INVALID)
   , tokenFactor (1.0f)
  {}

  ACE_UINT64                              averageTokensPerInterval;
  // catch-up mode (accumulate unused available tokens ? : reset after each interval)
  // *NOTE*: set this to false for real isochronicity (i.e. no catch-up)
  bool                                    catchUp;
  ACE_Time_Value                          interval;
  // *TODO*: move this to the module handler configuration ?
  bool                                    isMultimediaTask;
  enum Stream_Miscellaneous_DelayModeType mode;
  // *NOTE*: currently used only for audio streams when 'mode' is invalid (== -1)
  float                                   tokenFactor;
};

//////////////////////////////////////////

typedef Stream_ISessionDataNotify_T<struct Stream_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_MessageBase_T<Stream_DataBase_T<Stream_CommandType_t>,
                                                         enum Stream_MessageType,
                                                         Stream_CommandType_t>,
                                    Stream_SessionMessageBase_T<enum Stream_SessionMessageType,
                                                                Stream_SessionData_T<struct Stream_SessionData>,
                                                                struct Stream_UserData> > Stream_IInputSessionNotify_t;
typedef std::list<Stream_IInputSessionNotify_t*> Stream_InputSessionSubscribers_t;
typedef Stream_InputSessionSubscribers_t::iterator Stream_InputSessionSubscribersIterator_t;

struct Stream_Input_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  Stream_Input_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , queue (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {}

  ACE_Message_Queue_Base*           queue; // input
  Stream_IInputSessionNotify_t*     subscriber;
  Stream_InputSessionSubscribers_t* subscribers;
};

template <typename StreamConfigurationType>
class Stream_Input_Manager_Configuration_T
 : public Common_Input_Manager_Configuration
{
 public:
  Stream_Input_Manager_Configuration_T ()
   : Common_Input_Manager_Configuration ()
   , streamConfiguration (NULL)
  {}

  StreamConfigurationType* streamConfiguration;
};

//////////////////////////////////////////

typedef Stream_Configuration_T<//empty_string_,
                               struct Stream_Configuration,
                               struct Stream_Input_ModuleHandlerConfiguration> Stream_Input_Configuration_t;
typedef Stream_Input_Manager_Configuration_T<Stream_Input_Configuration_t> Stream_Input_Manager_Configuration_t;

#endif
