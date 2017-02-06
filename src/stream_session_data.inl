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

#include <ace/Log_Msg.h>

#include "stream_macros.h"
#include "stream_tools.h"

template <typename DataType>
Stream_SessionData_T<DataType>::Stream_SessionData_T ()
 : inherited (1,    // initial count
              true) // delete 'this' on zero ?
 , data_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::Stream_SessionData_T"));

}
template <typename DataType>
Stream_SessionData_T<DataType>::Stream_SessionData_T (DataType*& data_inout)
 : inherited (1,    // initial count
              true) // delete 'this' on zero ?
 , data_ (data_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::Stream_SessionData_T"));

  data_inout = NULL;
}

template <typename DataType>
Stream_SessionData_T<DataType>::~Stream_SessionData_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::~Stream_SessionData_T"));

  if (data_)
    delete data_;
}

template <typename DataType>
unsigned int
Stream_SessionData_T<DataType>::increase ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::increase"));

  return static_cast<unsigned int> (inherited::increment ());
}
template <typename DataType>
unsigned int
Stream_SessionData_T<DataType>::decrease ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::decrease"));

  return static_cast<unsigned int> (inherited::decrement ());
}

template <typename DataType>
const DataType&
Stream_SessionData_T<DataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::get"));

  if (data_)
    return *data_;

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (DataType ());

  ACE_NOTREACHED (return DataType ();)
}
template <typename DataType>
void
Stream_SessionData_T<DataType>::set (const DataType& data_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::set"));

  // sanity check(s)
  ACE_ASSERT (data_);

  *data_ += data_in;
}
//template <typename DataType>
//void
//Stream_SessionData_T<DataType>::set (DataType*& data_inout)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::set"));
//
//  // clean up
//  if (data_)
//    delete data_;
//
//  data_ = data_inout;
//  data_inout = NULL;
//}

template <typename DataType>
void
Stream_SessionData_T<DataType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::dump_state"));

  // sanity check(s)
  ACE_ASSERT (data_);

  // *TODO*: remove type inferences
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("user data: %@, start of session: %s%s\n"),
              data_->userData,
              ACE_TEXT (Stream_Tools::timeStamp2LocalString (data_->startOfSession).c_str ()),
              (data_->aborted ? ACE_TEXT(" [aborted]") : ACE_TEXT(""))));
}
