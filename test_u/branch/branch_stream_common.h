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

#ifndef BRANCH_STREAM_COMMON_H
#define BRANCH_STREAM_COMMON_H

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

// forward declarations
struct Branch_AllocatorConfiguration;
class Branch_Message;
class Branch_SessionMessage;

struct Branch_MessageData
{
  Branch_MessageData ()
  {}
  virtual ~Branch_MessageData ()
  {}
  inline void operator+= (struct Branch_MessageData rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};
typedef Stream_DataBase_T<struct Branch_MessageData> Branch_MessageData_t;

struct Branch_SessionData
 : Stream_SessionData
{
  Branch_SessionData ()
   : Stream_SessionData ()
   , stream (NULL)
  {}

  struct Branch_SessionData& operator+= (const struct Branch_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    stream = (stream ? stream : rhs_in.stream);

    return *this;
  }

  Stream_Base_t* stream; // used by aggregator modules
};
typedef Stream_SessionData_T<struct Branch_SessionData> Branch_SessionData_t;

typedef Stream_ISessionDataNotify_T<struct Branch_SessionData,
                                    enum Stream_SessionMessageType,
                                    Branch_Message,
                                    Branch_SessionMessage> Branch_Notification_t;
typedef std::list<Branch_Notification_t*> Branch_Subscribers_t;
typedef Branch_Subscribers_t::iterator Branch_SubscribersIterator_t;

// forward declarations
//extern const char stream_name_string_[];
struct Branch_StreamConfiguration
 : Stream_Configuration
{
  Branch_StreamConfiguration ()
   : Stream_Configuration ()
   , module_2 (NULL)
  {}

  Stream_Module_t* module_2;
};
struct Branch_ModuleHandlerConfiguration;
typedef Stream_Configuration_T< // stream_name_string_,
                               struct Branch_StreamConfiguration,
                               struct Branch_ModuleHandlerConfiguration> Branch_StreamConfiguration_t;
struct Branch_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  Branch_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , printProgressDot (false)
   , pushStatisticMessages (true)
   , queue (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {}

  bool                          printProgressDot;         // file writer module
  bool                          pushStatisticMessages;
  ACE_Message_Queue_Base*       queue;
  Branch_StreamConfiguration_t* streamConfiguration;      // net source module
  Branch_Notification_t*        subscriber;
  Branch_Subscribers_t*         subscribers;
};

struct Branch_StreamState
 : Stream_State
{
  Branch_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   //, userData (NULL)
  {}

  struct Branch_SessionData* sessionData;

  //struct Branch_UserData*    userData;
};

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Branch_Message,
                                          Branch_SessionMessage> Branch_MessageAllocator_t;

#endif
