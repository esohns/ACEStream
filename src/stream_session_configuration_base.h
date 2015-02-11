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

#ifndef STREAM_SESSION_CONFIGURATION_BASE_H
#define STREAM_SESSION_CONFIGURATION_BASE_H

#include "ace/Global_Macros.h"
#include "ace/Time_Value.h"

#include "common_referencecounter_base.h"

template <typename DataType>
class Stream_SessionConfigurationBase_T
 : public Common_ReferenceCounterBase
{
 public:
  Stream_SessionConfigurationBase_T (const DataType&,                              // user data
                                     const ACE_Time_Value& = ACE_Time_Value::zero, // "official" start of session
                                     bool = false);                                // session ended because of user abort ?
  virtual ~Stream_SessionConfigurationBase_T ();

  // info
  DataType getUserData () const;
  ACE_Time_Value getStartOfSession () const;
  bool getUserAbort () const;

  // implement Common_IDumpState
  virtual void dump_state () const;

 private:
  typedef Common_ReferenceCounterBase inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_SessionConfigurationBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionConfigurationBase_T (const Stream_SessionConfigurationBase_T<DataType>&));
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionConfigurationBase_T<DataType>& operator=(const Stream_SessionConfigurationBase_T<DataType>&));

  DataType       userData_;
  ACE_Time_Value startOfSession_;
  bool           userAbort_;
};

// include template implementation
#include "stream_session_configuration_base.inl"

#endif
