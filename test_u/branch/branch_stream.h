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

#ifndef BRANCH_STREAM_H
#define BRANCH_STREAM_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "branch_message.h"
#include "branch_session_message.h"
#include "branch_stream_common.h"

// forward declarations
class Stream_IAllocator;

extern const char stream_name_string_[];

class Branch_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Branch_StreamState,
                        struct Branch_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Branch_ModuleHandlerConfiguration,
                        Test_U_SessionManager_t,
                        Stream_ControlMessage_t,
                        Branch_Message,
                        Branch_SessionMessage>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Branch_StreamState,
                        struct Branch_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Branch_ModuleHandlerConfiguration,
                        Test_U_SessionManager_t,
                        Stream_ControlMessage_t,
                        Branch_Message,
                        Branch_SessionMessage> inherited;

 public:
  Branch_Stream ();
  inline virtual ~Branch_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // layout handle
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Branch_Stream (const Branch_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Branch_Stream& operator= (const Branch_Stream&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

//////////////////////////////////////////

class Branch_Stream_2
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Branch_StreamState,
                        struct Branch_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Branch_ModuleHandlerConfiguration,
                        Test_U_SessionManager_t,
                        Stream_ControlMessage_t,
                        Branch_Message,
                        Branch_SessionMessage>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Branch_StreamState,
                        struct Branch_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Branch_ModuleHandlerConfiguration,
                        Test_U_SessionManager_t,
                        Stream_ControlMessage_t,
                        Branch_Message,
                        Branch_SessionMessage> inherited;

 public:
  Branch_Stream_2 ();
  inline virtual ~Branch_Stream_2 () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // layout handle
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

  // implement (part of) Stream_ISessionCB
  virtual void onSessionEnd (Stream_SessionId_t);

 private:
  ACE_UNIMPLEMENTED_FUNC (Branch_Stream_2 (const Branch_Stream_2&))
  ACE_UNIMPLEMENTED_FUNC (Branch_Stream_2& operator= (const Branch_Stream_2&))

  // *TODO*: re-consider this API
  //inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

#endif
