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

#ifndef STREAM_SESSION_DATA_H
#define STREAM_SESSION_DATA_H

#include <deque>
#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_idumpstate.h"
#include "common_iget.h"
#include "common_referencecounter_base.h"

#include "stream_common.h"

template <typename BaseType,
          typename MediaFormatType,
          typename StreamStateType, // inherits Stream_State
          typename StatisticType, // inherits Stream_Statistic
          typename UserDataType>
class Stream_SessionDataMediaBase_T
 : public BaseType
{
  typedef BaseType inherited;

 public:
  // convenient types
  typedef MediaFormatType MEDIAFORMAT_T;
  typedef std::deque<MediaFormatType> MEDIAFORMATS_T;
  typedef typename MEDIAFORMATS_T::iterator MEDIAFORMATS_ITERATOR_T;
  typedef Stream_SessionDataMediaBase_T<BaseType,
                                        MediaFormatType,
                                        StreamStateType,
                                        StatisticType,
                                        UserDataType> OWN_TYPE_T;

  Stream_SessionDataMediaBase_T ();
  // *NOTE*: the idea is to 'copy' the data
  Stream_SessionDataMediaBase_T (const OWN_TYPE_T&);
  inline virtual ~Stream_SessionDataMediaBase_T () {}

  // *NOTE*: the idea is to 'merge' the data
  //         --> this ought (!) to be overriden by derived classes
  OWN_TYPE_T& operator+= (const OWN_TYPE_T&);

  MEDIAFORMATS_T                  formats;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  StreamStateType*                state;
  StatisticType                   statistic;
  std::string                     targetFileName;

  UserDataType*                   userData;
};

//////////////////////////////////////////

template <typename DataType> // inherits Stream_SessionData
class Stream_SessionData_T
 : public Common_ReferenceCounterBase
 , public Common_IGetSetR_T<DataType>
// , public Common_ISetP_T<DataType>
 , public Common_IDumpState
{
  typedef Common_ReferenceCounterBase inherited;

 public:
  // convenient types
  typedef DataType DATA_T;

  // *IMPORTANT NOTE*: fire-and-forget API (first argument)
  Stream_SessionData_T (DataType*&); // session data handle
  virtual ~Stream_SessionData_T ();

  // override Common_ReferenceCounterBase
  //inline virtual unsigned int increase () { return static_cast<unsigned int> (inherited::increment ()); };
  //inline virtual unsigned int decrease () { return static_cast<unsigned int> (inherited::decrement ()); };

  // implement Common_IGetSet_T
  virtual const DataType& getR () const;
  // *NOTE*: merge-sets the session data (operator+=)
  virtual void setR (const DataType&);

  // implement Common_ISetP_T
  // *NOTE*: re-sets the session data
  // *WARNING*: 'delete's data_; handle with care
  // *IMPORTANT NOTE*: fire-and-forget API (first argument)
//  virtual void setP (DataType*); // session data handle

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  DataType* data_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionData_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionData_T (const Stream_SessionData_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionData_T& operator= (const Stream_SessionData_T&))
};

// include template definition
#include "stream_session_data.inl"

#endif
