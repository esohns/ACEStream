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

#ifndef STREAM_EVENTHANDLER_H
#define STREAM_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "common_inotify.h"

#include "test_u_common.h"

class Stream_EventHandler
 : public Common_INotify_T<Stream_SessionData,
                           Stream_Message_t>
{
 public:
  Stream_EventHandler (Stream_GTK_CBData*); // GTK state
  virtual ~Stream_EventHandler ();

  // implement Common_INotify_T
  virtual void start (const Stream_SessionData&);
  virtual void notify (const Stream_Message_t&);
  virtual void end ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Stream_EventHandler (const Stream_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Stream_EventHandler& operator=(const Stream_EventHandler&))

  Stream_GTK_CBData* CBData_;
};

#endif
