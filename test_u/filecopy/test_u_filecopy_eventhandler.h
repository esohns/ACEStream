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

#ifndef TEST_U_FILECOPY_EVENTHANDLER_H
#define TEST_U_FILECOPY_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "stream_common.h"

#include "test_u_filecopy_message.h"
#include "test_u_filecopy_session_message.h"

// forward declarations
struct Stream_Filecopy_UI_CBData;

class Stream_Filecopy_EventHandler
 : public Stream_Filecopy_ISessionNotify_t
{
 public:
  Stream_Filecopy_EventHandler (struct Stream_Filecopy_UI_CBData*); // GTK state
  inline virtual ~Stream_Filecopy_EventHandler () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const struct Stream_Filecopy_SessionData&);
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const Stream_Filecopy_Message&);
  virtual void notify (Stream_SessionId_t,
                       const Stream_Filecopy_SessionMessage&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_EventHandler (const Stream_Filecopy_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_EventHandler& operator= (const Stream_Filecopy_EventHandler&))

  struct Stream_Filecopy_UI_CBData*   CBData_;
  struct Stream_Filecopy_SessionData* sessionData_;
};

#endif
