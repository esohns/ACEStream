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
#include "stdafx.h"

#include "stream_resetcounterhandler.h"

#include "ace/Log_Msg.h"

#include "common_icounter.h"

#include "stream_macros.h"

Stream_ResetCounterHandler::Stream_ResetCounterHandler (Common_ICounter* counter_in)
 : inherited (NULL,                           // no reactor
              ACE_Event_Handler::LO_PRIORITY) // priority
 , counter_ (counter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ResetCounterHandler::Stream_ResetCounterHandler"));

}

Stream_ResetCounterHandler::~Stream_ResetCounterHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_ResetCounterHandler::~Stream_ResetCounterHandler"));

}

int
Stream_ResetCounterHandler::handle_timeout (const ACE_Time_Value& tv_in,
                                            const void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ResetCounterHandler::handle_timeout"));

  ACE_UNUSED_ARG (tv_in);
  ACE_UNUSED_ARG (arg_in);

  try
  {
    counter_->reset ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught an exception in Common_ICounter::reset() --> check implementation !, continuing\n")));

    // *TODO*: what else can we do ?
  }

  return 0;
}
