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

#include "stream_session_data_base.h"

template <typename SessionDataType>
class Stream_SessionData_T
 : public Stream_SessionDataBase_T<SessionDataType>
 , public Common_IGet_T<SessionDataType>
{
 public:
  Stream_SessionData_T (const SessionDataType&);

  // implement Common_IDumpState
  virtual void dump_state () const;

  // implement Common_IGet_T
  virtual const SessionDataType& get () const;

 private:
  typedef Stream_SessionDataBase_T<SessionDataType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_SessionData_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionData_T (const Stream_SessionData_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionData_T& operator= (const Stream_SessionData_T&))
  virtual ~Stream_SessionData_T ();

  SessionDataType sessionData_;
};

#endif
