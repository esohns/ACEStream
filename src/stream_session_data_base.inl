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

template <typename DataType>
Stream_SessionDataBase_T<DataType>::Stream_SessionDataBase_T ()
 : inherited (1,    // initial count
              true) // delete on zero ?
 , sessionData_ (NULL)
 , deleteSessionData_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataBase_T::Stream_SessionDataBase_T"));

}

template <typename DataType>
Stream_SessionDataBase_T<DataType>::Stream_SessionDataBase_T (DataType* sessionData_in,
                                                              bool deleteSessionData_in)
 : inherited (1,    // initial count
              true) // delete on zero ?
 , sessionData_ (sessionData_in)
 , deleteSessionData_ (deleteSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataBase_T::Stream_SessionDataBase_T"));

}

template <typename DataType>
Stream_SessionDataBase_T<DataType>::~Stream_SessionDataBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataBase_T::~Stream_SessionDataBase_T"));

  // clean up
  if (deleteSessionData_)
    delete sessionData_;
}

template <typename DataType>
DataType*
Stream_SessionDataBase_T<DataType>::getData () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataBase_T::getData"));

  return sessionData_;
}

template <typename DataType>
void
Stream_SessionDataBase_T<DataType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataBase_T::dump_state"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("session data: %@\n"),
              sessionData_));
}
