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

#include "ace/Basic_Types.h"
#include "ace/Time_Value.h"

#include "common_input_common.h"

#include "stream_configuration.h"

enum Stream_Miscellaneous_DelayModeType
{
  STREAM_MISCELLANEOUS_DELAY_MODE_BYTES = 0,
  STREAM_MISCELLANEOUS_DELAY_MODE_MESSAGES,
  STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER, // support isochronicity (timer-based)
  STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER_BYTES, // support isochronicity (timer-based)
  ////////////////////////////////////////
  STREAM_MISCELLANEOUS_DELAY_MODE_MAX,
  STREAM_MISCELLANEOUS_DELAY_MODE_INVALID
};

struct Stream_Miscellaneous_DelayConfiguration
{
  Stream_Miscellaneous_DelayConfiguration ()
   : averageTokensPerInterval (0)
   , interval (ACE_Time_Value::zero)
   , mode (STREAM_MISCELLANEOUS_DELAY_MODE_INVALID)
  {}

  ACE_UINT64                              averageTokensPerInterval;
  ACE_Time_Value                          interval;
  enum Stream_Miscellaneous_DelayModeType mode;
};

//////////////////////////////////////////

typedef Stream_ISessionDataNotify_T<struct Stream_SessionData,
                                    enum Stream_SessionMessageType,
                                    ACE_Message_Block,
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
