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

#include "ace/Global_Macros.h"

#include "common_idumpstate.h"
#include "common_iget.h"
#include "common_referencecounter_base.h"

template <typename DataType>
class Stream_SessionData_T
 : public Common_ReferenceCounterBase
 , public Common_IGetSetR_T<DataType>
 , public Common_IDumpState
{
  typedef Common_ReferenceCounterBase inherited;

 public:
  // convenient types
  typedef DataType DATA_T;

  Stream_SessionData_T ();
  // *IMPORTANT NOTE*: fire-and-forget API
  Stream_SessionData_T (DataType*&); // session data handle
  virtual ~Stream_SessionData_T ();

  // override Common_ReferenceCounterBase
  //inline virtual unsigned int increase () { return static_cast<unsigned int> (inherited::increment ()); };
  //inline virtual unsigned int decrease () { return static_cast<unsigned int> (inherited::decrement ()); };

  // implement Common_IGetSet_T
  virtual const DataType& getR () const;
  // *NOTE*: merge-sets the session data (operator+=)
  virtual void setR (const DataType&);

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  DataType* data_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionData_T (const Stream_SessionData_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionData_T& operator= (const Stream_SessionData_T&))
};

// include template definition
#include "stream_session_data.inl"

#endif
