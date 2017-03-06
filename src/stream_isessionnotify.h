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

#ifndef STREAM_ISESSIONNOTIFY_H
#define STREAM_ISESSIONNOTIFY_H

// forward declarations
class ACE_Time_Value;

class Stream_ISession
{
 public:
  inline virtual ~Stream_ISession () {};

  virtual void wait (bool = true,                       // wait for end ? : start
                     const ACE_Time_Value* = NULL) = 0; // timeout (absolute) {NULL: block}
};

class Stream_ISessionCB
{
 public:
  inline virtual ~Stream_ISessionCB () {};

  virtual void startCB () = 0;
  virtual void endCB () = 0;
};

//////////////////////////////////////////

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType>
class Stream_ISessionNotify_T
{
 public:
  inline virtual ~Stream_ISessionNotify_T () {};

  virtual void start (SessionIdType,                 // session id
                      const SessionDataType&) = 0;   // session data
  virtual void notify (SessionIdType,                // session id
                       const SessionEventType&) = 0; // event (state/status change, ...)
  virtual void end (SessionIdType) = 0;              // session id
};

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType>
class Stream_ISessionDataNotify_T
 : public Stream_ISessionNotify_T<SessionIdType,
                                  SessionDataType,
                                  SessionEventType>
{
 public:
  inline virtual ~Stream_ISessionDataNotify_T () {};

  virtual void notify (SessionIdType,                  // session id
                       const MessageType&) = 0;        // (protocol) data
  virtual void notify (SessionIdType,                  // session id
                       const SessionMessageType&) = 0; // session message
};

#endif
