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

#include "stream_session_data.h"

#include "ace/Guard_T.h"
#include "ace/Synch.h"

#include "stream_macros.h"
#include "stream_tools.h"

Stream_SessionData::Stream_SessionData (const void* data_in,
                                        const ACE_Time_Value& startOfSession_in,
                                        bool userAbort_in)
 : inherited (1,    // initial count
              true) // delete on zero ?
 , startOfSession_ (startOfSession_in)
 , userAbort_ (userAbort_in)
 , userData_ (data_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData::Stream_SessionData"));

}

Stream_SessionData::~Stream_SessionData ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData::~Stream_SessionData"));

}

const void*
Stream_SessionData::getUserData () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData::getUserData"));

  return userData_;
}

ACE_Time_Value
Stream_SessionData::getStartOfSession () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData::getStartOfSession"));

  return startOfSession_;
}

bool
Stream_SessionData::getUserAbort () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData::getUserAbort"));

  return userAbort_;
}

void
Stream_SessionData::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData::dump_state"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("user data: %@, start of session: %s%s\n"),
              userData_,
              ACE_TEXT (Stream_Tools::timestamp2LocalString (startOfSession_).c_str()),
              (userAbort_ ? ACE_TEXT(" [user abort !]")
                          : ACE_TEXT(""))));
}
