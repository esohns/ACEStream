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

#ifndef STREAM_RESETCOUNTERHANDLER_H
#define STREAM_RESETCOUNTERHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Event_Handler.h"
#include "ace/Time_Value.h"

#include "stream_exports.h"

// forward declaration(s)
class Common_ICounter;

class Stream_Export Stream_ResetCounterHandler
 : public ACE_Event_Handler
{
 public:
  Stream_ResetCounterHandler (Common_ICounter*); // interface handle
  virtual ~Stream_ResetCounterHandler ();

  // implement specific behaviour
  virtual int handle_timeout (const ACE_Time_Value&, // current time
                              const void*);          // asynchronous completion token

 private:
  typedef ACE_Event_Handler inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_ResetCounterHandler ())
  ACE_UNIMPLEMENTED_FUNC (Stream_ResetCounterHandler (const Stream_ResetCounterHandler&))
  ACE_UNIMPLEMENTED_FUNC (Stream_ResetCounterHandler& operator= (const Stream_ResetCounterHandler&))

  Common_ICounter* counter_;
};

#endif
