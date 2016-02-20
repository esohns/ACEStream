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

#ifndef TEST_I_SOURCE_EVENTHANDLER_H
#define TEST_I_SOURCE_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "common_inotify.h"

#include "stream_common.h"

#include "test_i_common.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

class Test_I_Stream_Source_EventHandler
 : public Stream_IStreamNotify_t
{
 public:
  Test_I_Stream_Source_EventHandler (Stream_GTK_CBData*); // GTK state
  virtual ~Test_I_Stream_Source_EventHandler ();

  // implement Common_INotify_T
  virtual void start (const Test_I_Stream_SessionData_t&);
  virtual void notify (const Test_I_Stream_Message&);
  virtual void notify (const Test_I_Stream_SessionMessage&);
  virtual void end ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Source_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Source_EventHandler (const Test_I_Stream_Source_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Source_EventHandler& operator= (const Test_I_Stream_Source_EventHandler&))

  Stream_GTK_CBData*           CBData_;
  Test_I_Stream_SessionData_t* sessionData_;
};

#endif
