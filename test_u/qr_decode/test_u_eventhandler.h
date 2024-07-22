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

#ifndef TEST_U_EVENTHANDLER_H
#define TEST_U_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "stream_common.h"

#include "test_u_message.h"
#include "test_u_session_message.h"

// forward declarations
//struct Test_U_UI_CBData;
struct Test_U_SessionData;

class Test_U_EventHandler
 : public Test_U_Notification_t
{
 public:
  Test_U_EventHandler ();
  inline virtual ~Test_U_EventHandler () {}

  // implement Stream_ISessionDataNotify_T
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void start (Stream_SessionId_t,                      // session id
                      const QRDecode_DirectShow_SessionData&); // session data
#else
  virtual void start (Stream_SessionId_t,           // session id
                      const QRDecode_SessionData&); // session data
#endif // ACE_WIN32 || ACE_WIN64
  virtual void notify (Stream_SessionId_t,                    // session id
                       const enum Stream_SessionMessageType&, // event (state/status change, ...)
                       bool = false);                         // expedite ?
  virtual void notify (Stream_SessionId_t,     // session id
                       const Test_U_Message&); // data
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void notify (Stream_SessionId_t,                       // session id
                       const Test_U_DirectShow_SessionMessage&); // session message
#else
  virtual void notify (Stream_SessionId_t,            // session id
                       const Test_U_SessionMessage&); // session message
#endif // ACE_WIN32 || ACE_WIN64
  virtual void end (Stream_SessionId_t); // session id

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler (const Test_U_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler& operator= (const Test_U_EventHandler&))
};

#endif
