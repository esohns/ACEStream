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
#define STREAM_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "common_inotify.h"

#include "stream_common.h"

#include "test_u_filecopy_common.h"
#include "test_u_filecopy_message.h"
#include "test_u_filecopy_session_message.h"

class Stream_Filecopy_EventHandler
 : public Stream_Filecopy_IStreamNotify_t
{
 public:
  Stream_Filecopy_EventHandler (Stream_Filecopy_GTK_CBData*); // GTK state
  virtual ~Stream_Filecopy_EventHandler ();

  // implement Common_INotify_T
  virtual void start (const Stream_Filecopy_SessionData&);
  virtual void notify (const Stream_Filecopy_Message&);
  virtual void notify (const Stream_Filecopy_SessionMessage&);
  virtual void end ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_EventHandler (const Stream_Filecopy_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_EventHandler& operator= (const Stream_Filecopy_EventHandler&))

  Stream_Filecopy_GTK_CBData*  CBData_;
  Stream_Filecopy_SessionData* sessionData_;
};

#endif
