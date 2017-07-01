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

#include "ace/Global_Macros.h"

#include "common_inotify.h"

#include "stream_common.h"

#include "test_i_common.h"

#include "test_i_session_message.h"
#include "test_i_target_common.h"

class Test_I_Target_EventHandler
 : public Test_I_Target_ISessionNotify_t
{
 public:
  Test_I_Target_EventHandler (struct Test_I_Target_GTK_CBData*); // GTK state
  virtual ~Test_I_Target_EventHandler ();

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const struct Test_I_Target_SessionData&);
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const Test_I_Target_Message_t&);
  virtual void notify (Stream_SessionId_t,
                       const Test_I_Target_SessionMessage&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_EventHandler (const Test_I_Target_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_EventHandler& operator= (const Test_I_Target_EventHandler&))

  struct Test_I_Target_GTK_CBData*  CBData_;
  struct Test_I_Target_SessionData* sessionData_;
};

#endif
