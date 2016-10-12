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

#ifndef TEST_U_CAMSAVE_EVENTHANDLER_H
#define TEST_U_CAMSAVE_EVENTHANDLER_H

#include <ace/Global_Macros.h>

#include "stream_common.h"

#include "test_u_camsave_common.h"
#include "test_u_camsave_message.h"
#include "test_u_camsave_session_message.h"

class Stream_CamSave_EventHandler
 : public Stream_CamSave_ISessionNotify_t
{
 public:
  Stream_CamSave_EventHandler (Stream_CamSave_GTK_CBData*); // GTK state
  virtual ~Stream_CamSave_EventHandler ();

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const Stream_CamSave_SessionData&);
  virtual void notify (Stream_SessionId_t,
                       const Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const Stream_CamSave_Message&);
  virtual void notify (Stream_SessionId_t,
                       const Stream_CamSave_SessionMessage&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_CamSave_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Stream_CamSave_EventHandler (const Stream_CamSave_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CamSave_EventHandler& operator= (const Stream_CamSave_EventHandler&))

  Stream_CamSave_GTK_CBData*  CBData_;
  Stream_CamSave_SessionData* sessionData_;
};

#endif
