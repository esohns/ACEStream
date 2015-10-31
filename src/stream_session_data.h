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

#include "common_iget.h"
#include "common_referencecounter_base.h"

template <typename SessionDataType>
class Stream_SessionData_T
 : public Common_ReferenceCounterBase
 , public Common_IGetSet_T<SessionDataType>
{
 public:
  Stream_SessionData_T (const SessionDataType&);
  virtual ~Stream_SessionData_T ();

  // implement Common_IDumpState
  virtual void dump_state () const;

  // implement Common_IGetSet_T
  virtual const SessionDataType& get () const;
  virtual void set (const SessionDataType&);

 private:
  typedef Common_ReferenceCounterBase inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_SessionData_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionData_T (const Stream_SessionData_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionData_T& operator= (const Stream_SessionData_T&))

  SessionDataType data_;
};

#endif
