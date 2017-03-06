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

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include "stream_isessionnotify.h"

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType>
class Stream_SessionBase_T
 : public Stream_ISession
 , public Stream_ISessionCB
 , public Stream_ISessionDataNotify_T<SessionIdType,
                                      SessionDataType,
                                      SessionEventType,
                                      MessageType,
                                      SessionMessageType>
{
 public:
  virtual ~Stream_SessionBase_T ();

  // convenient types
  typedef Stream_ISessionNotify_T<SessionIdType,
                                  SessionDataType,
                                  SessionEventType> INOTIFY_T;

  // implement Stream_ISession
  virtual void wait (bool = true,                   // wait for end ? : start
                     const ACE_Time_Value* = NULL); // timeout (absolute) {NULL: block}

 protected:
  Stream_SessionBase_T ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionBase_T (const Stream_SessionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionBase_T& operator= (const Stream_SessionBase_T&))

  // implement Stream_ISessionCB
  virtual void startCB ();
  virtual void endCB ();

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

  ACE_SYNCH_CONDITION condition_;
  ACE_SYNCH_MUTEX     lock_;
};

// include template definition
#include "stream_session_base.inl"

#endif
