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

#ifndef STREAM_SESSION_BASE_T_H
#define STREAM_SESSION_BASE_T_H

#include "ace/Condition_Thread_Mutex.h"
#include "ace/Global_Macros.h"

#include "stream_isessionnotify.h"

// forward declarations
class ACE_Thread_Mutex;

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType>
class Stream_SessionBase_T
 : public Stream_ISession
 , public Stream_ISessionDataNotify_T<SessionDataType,
                                      SessionEventType,
                                      MessageType,
                                      SessionMessageType>
{
 public:
  inline virtual ~Stream_SessionBase_T () {}

  // convenient types
  typedef Stream_ISessionNotify_T<SessionDataType,
                                  SessionEventType> INOTIFY_T;

  // implement Stream_ISession
  // *TODO*: the current implementation is broken; there is a race condition
  //         between testing inSession_ and waiting for condition_ to be
  //         signalled
  //         --> do NOT pass a NULL timeout (i.e. do not use the blocking
  //             behaviour), as this may hang the thread (until the next session
  //             ends/starts)
  virtual void wait (bool = true,                   // wait for end ? : start
                     const ACE_Time_Value* = NULL); // timeout (absolute) {NULL: block}

 protected:
  Stream_SessionBase_T (ACE_Thread_Mutex*);

  // implement Stream_ISession
  virtual void startCB ();
  virtual void endCB ();

  bool                       inSession_;
  ACE_Thread_Mutex*          lock_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionBase_T (const Stream_SessionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionBase_T& operator= (const Stream_SessionBase_T&))

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,      // session id
                      const SessionDataType&); // session data
  inline virtual void notify (Stream_SessionId_t,
                              const SessionEventType&) {}
  virtual void end (Stream_SessionId_t); // session id
  inline virtual void notify (Stream_SessionId_t,
                              const MessageType&) {}
  inline virtual void notify (Stream_SessionId_t,
                              const SessionMessageType&) {}

  ACE_Condition_Thread_Mutex condition_;
};

// include template definition
#include "stream_session_base.inl"

#endif
