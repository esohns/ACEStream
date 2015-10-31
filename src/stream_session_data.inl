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
Stream_SessionData_T<SessionDataType>::Stream_SessionData_T (const SessionDataType& data_in)
 : inherited (1,     // initial count
              false) // delete on zero ?
 , data_ (data_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::Stream_SessionData_T"));

}

template <typename SessionDataType>
Stream_SessionData_T<SessionDataType>::~Stream_SessionData_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::~Stream_SessionData_T"));

}

template <typename SessionDataType>
void
Stream_SessionData_T::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::dump_state"));

  // *TODO*: remove type inferences
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("user data: %@, start of session: %s%s\n"),
              data_.userData,
              ACE_TEXT (Stream_Tools::timestamp2LocalString (data_.startOfSession).c_str ()),
              (data_.userAbort_ ? ACE_TEXT(" [user abort !]")
                                : ACE_TEXT(""))));
}

template <typename SessionDataType>
const SessionDataType&
Stream_SessionData_T<SessionDataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::get"));

  return data_;
}
template <typename SessionDataType>
void
Stream_SessionData_T<SessionDataType>::set (const SessionDataType& data_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::set"));

  data_ = data_in;
}
