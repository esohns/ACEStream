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

#include "common_timer_tools.h"

#include "stream_macros.h"

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
 , codecConfigurationData (NULL)
 , codecConfigurationDataSize (0)
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
 , codecConfigurationData (NULL)
 , codecConfigurationDataSize (0)
 , formats (data_in.formats)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , mediaFramework (data_in.mediaFramework)
#endif // ACE_WIN32 || ACE_WIN64
 , state (data_in.state)
 , statistic (data_in.statistic)
 , targetFileName (data_in.targetFileName)
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
                              UserDataType>::~Stream_SessionDataMediaBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataMediaBase_T::~Stream_SessionDataMediaBase_T"));

  if (codecConfigurationData)
    delete [] codecConfigurationData;
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

  // *NOTE*: the idea is to 'merge' the data
  // *TODO*: this is problematic on windows
  formats = rhs_in.formats;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  mediaFramework = rhs_in.mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  state = (state ? state : rhs_in.state);
  statistic =
      ((statistic.timeStamp >= rhs_in.statistic.timeStamp) ? statistic
                                                           : rhs_in.statistic);
  targetFileName = rhs_in.targetFileName;
  userData = (userData ? userData : rhs_in.userData);

  return *this;
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
                              UserDataType>::operator= (const Stream_SessionDataMediaBase_T<BaseType,
                                                                                            MediaFormatType,
                                                                                            StreamStateType,
                                                                                            StatisticType,
                                                                                            UserDataType>& rhs_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataMediaBase_T::operator="));

  inherited::operator= (rhs_in);

  // *TODO*: this is problematic on windows
  formats = rhs_in.formats;
#if defined(ACE_WIN32) || defined(ACE_WIN64)
  mediaFramework = rhs_in.mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  state = (state ? state : rhs_in.state);
  statistic = rhs_in.statistic;
  targetFileName = rhs_in.targetFileName;
  userData = (userData ? userData : rhs_in.userData);

  return *this;
}

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
bool
Stream_SessionData_T<DataType>::lock (bool block_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::lock"));

  int result = -1;

  result = (block_in ? inherited::lock_.acquire ()
                     : inherited::lock_.tryacquire ());
  if (unlikely (result == -1))
  {
    if (block_in)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
    else
    { int error = ACE_OS::last_error ();
      if (error != EBUSY)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_MUTEX::tryacquire(): \"%m\", aborting\n")));
    } // end ELSE
  } // end IF

  return (result == 0);
}

template <typename DataType>
const DataType&
Stream_SessionData_T<DataType>::getR () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::getR"));

  if (likely (data_))
    return *data_;

  static DataType dummy;
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (dummy);
  ACE_NOTREACHED (return dummy;)
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
Stream_SessionData_T<DataType>*
Stream_SessionData_T<DataType>::clone () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::clone"));

  OWN_TYPE_T* result_p = NULL;

  DataType* data_p = NULL;
  if (data_)
  {
    ACE_NEW_NORETURN (data_p,
                      DataType (*data_)); // try to copy-construct
    if (unlikely (!data_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, aborting\n")));
      return NULL;
    } // end IF
  } // end IF

  ACE_NEW_NORETURN (result_p,
                    OWN_TYPE_T (data_p));
  if (unlikely (!result_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    if (data_p)
      delete data_p;
    return NULL;
  } // end IF

  return result_p;
}

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
              ACE_TEXT (Common_Timer_Tools::timeStampToLocalString (data_->startOfSession).c_str ()),
              (data_->aborted ? ACE_TEXT (" [aborted]") : ACE_TEXT (""))));
}
