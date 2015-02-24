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

#include "stream_common.h"
#include "stream_macros.h"
#include "stream_tools.h"

template <typename DataType>
Stream_SessionDataBase_T<DataType>::Stream_SessionDataBase_T (DataType* data_in,
                                                              bool deleteData_in,
                                                              Stream_State_t* state_in,
                                                              const ACE_Time_Value& startOfSession_in,
                                                              bool userAbort_in)
 : inherited (1,    // initial count
              true) // delete on zero ?
 , deleteData_ (deleteData_in)
 , startOfSession_ (startOfSession_in)
 , state_ (state_in)
 , userAbort_ (userAbort_in)
 , data_ (data_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataBase_T::Stream_SessionDataBase_T"));

}

template <typename DataType>
Stream_SessionDataBase_T<DataType>::~Stream_SessionDataBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataBase_T::~Stream_SessionDataBase_T"));

  if (deleteData_)
    delete data_;
}

template <typename DataType>
DataType*
Stream_SessionDataBase_T<DataType>::getData () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataBase_T::getData"));

  return data_;
}

template <typename DataType>
Stream_State_t*
Stream_SessionDataBase_T<DataType>::getState () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataBase_T::getState"));

  return state_;
}

template <typename DataType>
ACE_Time_Value
Stream_SessionDataBase_T<DataType>::getStartOfSession () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataBase_T::getStartOfSession"));

  return startOfSession_;
}

template <typename DataType>
bool
Stream_SessionDataBase_T<DataType>::getUserAbort () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataBase_T::getUserAbort"));

  return userAbort_;
}

template <typename DataType>
void
Stream_SessionDataBase_T<DataType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataBase_T::dump_state"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("start of session: %s%s\n"),
              ACE_TEXT (Stream_Tools::timestamp2LocalString (startOfSession_).c_str ()),
              (userAbort_ ? ACE_TEXT(", [user abort !]")
                          : ACE_TEXT (""))));
}
