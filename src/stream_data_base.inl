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

#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <typename DataType>
Stream_DataBase_T<DataType>::Stream_DataBase_T ()
 : inherited (1) // initial count
 , data_ (NULL)
 , delete_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBase_T::Stream_DataBase_T"));

}

template <typename DataType>
Stream_DataBase_T<DataType>::Stream_DataBase_T (const Stream_DataBase_T<DataType>& data_in)
 : inherited (1) // initial count
 , data_ (NULL)
 , delete_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBase_T::Stream_DataBase_T"));

  *this = data_in;
}

template <typename DataType>
Stream_DataBase_T<DataType>::Stream_DataBase_T (DataType*& data_inout,
                                                bool delete_in)
 : inherited (1) // initial count
 , data_ (data_inout)
 , delete_ (delete_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBase_T::Stream_DataBase_T"));

  data_inout = NULL;
}

template <typename DataType>
Stream_DataBase_T<DataType>&
Stream_DataBase_T<DataType>::operator= (const Stream_DataBase_T<DataType>& rhs_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBase_T::operator="));

  // clean up
  if (data_ && delete_)
  {
    delete data_;
    data_ = NULL;
  } // end IF
  delete_ = false;

  if (rhs_in.delete_)
  {
    // sanity check(s)
    ACE_ASSERT (!data_);

    // *WARNING*: this may not be the intended behavior
    ACE_NEW_NORETURN (data_,
                      DataType (*rhs_in.data_));
    if (!data_)
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, continuing\n")));

    delete_ = (data_ != NULL);
  } // end IF
  else
    data_ = rhs_in.data_;

  return *this;
}

template <typename DataType>
void
Stream_DataBase_T<DataType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBase_T::dump_state"));

//  if (data_)
//  {
//    Common_IDumpState* idumpstate_p = static_cast<Common_IDumpState*> (data_);
//    if (idumpstate_p)
//      idumpstate_p->dump_state ();
//  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("reference count: %d, delete: %s\n"),
              inherited::refcount_.value (),
              (delete_ ? ACE_TEXT ("true") : ACE_TEXT ("false"))));
}

template <typename DataType>
void
Stream_DataBase_T<DataType>::setR (const DataType& data_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBase_T::setR"));

  // merge ?
  DataType& data_r = const_cast<DataType&> (data_in);
  if (data_)
    data_r += *data_;

  // clean up ?
  if (data_ && delete_)
  {
    delete data_; data_ = NULL;
    delete_ = false;
  } // end IF

  data_ = &data_r;
  delete_ = false; // never delete
}

template <typename DataType>
void
Stream_DataBase_T<DataType>::setPR (DataType*& data_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBase_T::setPR"));

  // sanity check(s)
  ACE_ASSERT (data_inout);

  // merge ?
  if (data_)
    *data_inout += *data_;

  // clean up ?
  if (data_ && delete_)
  {
    delete data_; data_ = NULL;
  } // end IF

  data_ = data_inout;
  delete_ = true;

  data_inout = NULL;
}

template <typename DataType>
unsigned int
Stream_DataBase_T<DataType>::decrease ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBase_T::decrease"));

  long result = inherited::decrement ();

  if (result == 0)
    delete this;

  return static_cast<unsigned int> (result);
}
