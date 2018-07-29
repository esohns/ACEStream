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

#ifndef TEST_I_TARGET_EVENTHANDLER_H
#define TEST_I_TARGET_EVENTHANDLER_H

#include "ace/config-lite.h"
#include "ace/Global_Macros.h"

#include "stream_common.h"

#include "stream_isessionnotify.h"

#include "test_i_target_common.h"

 // forward declarations
struct Test_I_GTK_CBData;

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
class Test_I_Target_EventHandler_T
 : public Stream_ISessionDataNotify_T<SessionIdType,
                                      SessionDataType,
                                      SessionEventType,
                                      MessageType,
                                      SessionMessageType>
{
 public:
  Test_I_Target_EventHandler_T (CallbackDataType*); // GTK state
  inline virtual ~Test_I_Target_EventHandler_T () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (SessionIdType,           // session id
                      const SessionDataType&); // session data
  virtual void notify (SessionIdType,            // session id
                       const SessionEventType&); // event (state/status change, ...)
  virtual void end (SessionIdType); // session id
  virtual void notify (SessionIdType,       // session id
                       const MessageType&); // (protocol) data
  virtual void notify (SessionIdType,              // session id
                       const SessionMessageType&); // session message

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_EventHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_EventHandler_T (const Test_I_Target_EventHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_EventHandler_T& operator= (const Test_I_Target_EventHandler_T&))

  CallbackDataType* CBData_;
  SessionDataType*  sessionData_;
};

// include template definition
#include "test_i_target_eventhandler.inl"

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_Target_EventHandler_T<Stream_SessionId_t,
                                     struct Test_I_Target_DirectShow_SessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_Target_DirectShow_Stream_Message,
                                     Test_I_Target_DirectShow_Stream_SessionMessage,
                                     struct Test_I_Target_DirectShow_GTK_CBData> Test_I_Target_DirectShow_EventHandler_t;

typedef Test_I_Target_EventHandler_T<Stream_SessionId_t,
                                     struct Test_I_Target_MediaFoundation_SessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_Target_MediaFoundation_Stream_Message,
                                     Test_I_Target_MediaFoundation_Stream_SessionMessage,
                                     struct Test_I_Target_MediaFoundation_GTK_CBData> Test_I_Target_MediaFoundation_EventHandler_t;
#else
typedef Test_I_Target_EventHandler_T<Stream_SessionId_t,
                                     Test_I_Target_SessionData,
                                     Stream_SessionMessageType,
                                     Test_I_Target_Stream_Message,
                                     Test_I_Target_Stream_SessionMessage,
                                     Test_I_Target_GTK_CBData> Test_I_Target_EventHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
