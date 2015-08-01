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

#include "stream_macros.h"
#include "stream_tools.h"

template <typename SessionDataType>
Stream_SessionData_T<SessionDataType>::Stream_SessionData_T (const SessionDataType& sessionData_in)
 : sessionData_ (sessionData_in)
 ,  inherited (&sessionData_, // handle
               false)         // delete on zero ?
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::Stream_SessionData_T"));

}

template <typename SessionDataType>
Stream_SessionData_T<SessionDataType>::~Stream_SessionData_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::~Stream_SessionData_T"));

}

template <typename SessionDataType>
const SessionDataType&
Stream_SessionData_T<SessionDataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::get"));

  return sessionData_;
}

template <typename SessionDataType>
void
Stream_SessionData_T::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::dump_state"));

  // *TODO*: remove type inference
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("user data: %@, start of session: %s%s\n"),
              userData_,
              ACE_TEXT (Stream_Tools::timestamp2LocalString (startOfSession_).c_str()),
              (userAbort_ ? ACE_TEXT(" [user abort !]")
                          : ACE_TEXT(""))));
}
