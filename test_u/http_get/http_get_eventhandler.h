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

#ifndef HTTP_GET_EVENTHANDLER_H
#define HTTP_GET_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "common_iinitialize.h"

#include "stream_common.h"

#include "http_get_message.h"
#include "http_get_session_message.h"

// forward declarations
struct HTTPGet_UI_CBData;
struct HTTPGet_SessionData;

class HTTPGet_EventHandler
 : public HTTPGet_Notification_t
{
 public:
  HTTPGet_EventHandler (struct HTTPGet_UI_CBData*, // UI state
                        bool = false);             // console mode ?
  inline virtual ~HTTPGet_EventHandler () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,                 // session id
                      const struct HTTPGet_SessionData&); // session data
  virtual void notify (Stream_SessionId_t,                    // session id
                       const enum Stream_SessionMessageType&, // event (state/status change, ...)
                       bool = false);                         // expedite ?
  virtual void notify (Stream_SessionId_t,      // session id
                       const HTTPGet_Message&); // data
  virtual void notify (Stream_SessionId_t,             // session id
                       const HTTPGet_SessionMessage&); // session message
  virtual void end (Stream_SessionId_t); // session id

 private:
  ACE_UNIMPLEMENTED_FUNC (HTTPGet_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (HTTPGet_EventHandler (const HTTPGet_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (HTTPGet_EventHandler& operator= (const HTTPGet_EventHandler&))

  // helper methods
  void endSession ();

  bool                      consoleMode_;
  struct HTTPGet_UI_CBData* CBData_;
};

#endif
