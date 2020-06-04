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

#include "common_macros.h"

#include "stream_macros.h"
#include "stream_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_defines.h"
#endif // ACE_WIN32 || ACE_WIN64

template <typename BaseType,
          typename MediaFormatType,
          typename StreamStateType,
          typename StatisticType,
          typename UserDataType>
Stream_SessionDataMediaBase_T<BaseType,
                              MediaFormatType,
                              StreamStateType,
                              StatisticType,
                              UserDataType>::Stream_SessionDataMediaBase_T ()
 : inherited ()
 , formats ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
 , state (NULL)
 , statistic ()
 , userData (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataMediaBase_T::Stream_SessionDataMediaBase_T"));

}

template <typename BaseType,
          typename MediaFormatType,
          typename StreamStateType,
          typename StatisticType,
          typename UserDataType>
Stream_SessionDataMediaBase_T<BaseType,
                              MediaFormatType,
                              StreamStateType,
                              StatisticType,
                              UserDataType>::Stream_SessionDataMediaBase_T (const OWN_TYPE_T& data_in)
 : inherited (data_in)
 , formats (data_in.formats)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , mediaFramework (data_in.mediaFramework)
#endif // ACE_WIN32 || ACE_WIN64
 , state (data_in.state)
 , statistic (data_in.statistic)
 , userData (data_in.userData)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataMediaBase_T::Stream_SessionDataMediaBase_T"));

}

template <typename BaseType,
          typename MediaFormatType,
          typename StreamStateType,
          typename StatisticType,
          typename UserDataType>
Stream_SessionDataMediaBase_T<BaseType,
                              MediaFormatType,
                              StreamStateType,
                              StatisticType,
                              UserDataType>&
Stream_SessionDataMediaBase_T<BaseType,
                              MediaFormatType,
                              StreamStateType,
                              StatisticType,
                              UserDataType>::operator+= (const Stream_SessionDataMediaBase_T<BaseType,
                                                                                             MediaFormatType,
                                                                                             StreamStateType,
                                                                                             StatisticType,
                                                                                             UserDataType>& rhs_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataMediaBase_T::operator+="));

  // *NOTE*: the idea is to 'merge' the data
  inherited::operator+= (rhs_in);
//  ACE_ASSERT (formats.size () == rhs_in.formats.size ());
//  state = (state ? state : rhs_in.state);
  statistic =
      ((statistic.timeStamp >= rhs_in.statistic.timeStamp) ? statistic
                                                           : rhs_in.statistic);

  userData = (userData ? userData : rhs_in.userData);

  return *this;
}

//template <typename BaseType,
//          typename MediaFormatType,
//          typename StreamStateType,
//          typename StatisticType,
//          typename UserDataType>
//Stream_SessionDataMediaBase_T<BaseType,
//                              MediaFormatType,
//                              StreamStateType,
//                              StatisticType,
//                              UserDataType>&
//Stream_SessionDataMediaBase_T<BaseType,
//                              MediaFormatType,
//                              StreamStateType,
//                              StatisticType,
//                              UserDataType>::operator= (const Stream_SessionDataMediaBase_T<BaseType,
//                                                                                            MediaFormatType,
//                                                                                            StreamStateType,
//                                                                                            StatisticType,
//                                                                                            UserDataType>& rhs_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataMediaBase_T::operator="));

//  // *NOTE*: the idea is to 'merge' the data
//  inherited::operator= (rhs_in);
////  ACE_ASSERT (formats.size () == rhs_in.formats.size ());
////  state = (state ? state : rhs_in.state);
//  statistic =
//      ((statistic.timeStamp >= rhs_in.statistic.timeStamp) ? statistic
//                                                           : rhs_in.statistic);

//  userData = (userData ? userData : rhs_in.userData);

//  return *this;
//}

//////////////////////////////////////////

//template <typename DataType>
//Stream_SessionData_T<DataType>::Stream_SessionData_T ()
// : inherited (1,    // initial count
//              true) // delete 'this' on zero ?
// , data_ (NULL)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::Stream_SessionData_T"));

//}
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
const DataType&
Stream_SessionData_T<DataType>::getR () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::getR"));

  if (likely (data_))
    return *data_;

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (DataType ());

  ACE_NOTREACHED (return DataType ();)
}
template <typename DataType>
void
Stream_SessionData_T<DataType>::setR (const DataType& data_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::setR"));

  // sanity check(s)
  ACE_ASSERT (data_);

  *data_ += data_in;
}

//template <typename DataType>
//void
//Stream_SessionData_T<DataType>::setP (DataType* data_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::set"));

//  // sanity check(s)
//  ACE_ASSERT (data_in);

//  // clean up
//#if defined (_DEBUG)
//  if (unlikely (data_ && (inherited::refcount_ != 1)))
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("resetting session (id: %d) data on-the-fly\n"),
//                data_->sessionId));
//#endif // _DEBUG
//  if (likely (data_))
//    delete data_;

//  data_ = data_in;
//}

template <typename DataType>
void
Stream_SessionData_T<DataType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::dump_state"));

  // sanity check(s)
  ACE_ASSERT (data_);

  // *TODO*: remove type inferences
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("user data: %@, start of session: %s%s\n"),
              data_->userData,
              ACE_TEXT (Stream_Tools::timeStampToLocalString (data_->startOfSession).c_str ()),
              (data_->aborted ? ACE_TEXT(" [aborted]") : ACE_TEXT(""))));
}
