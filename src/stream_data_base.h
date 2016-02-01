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

#ifndef STREAM_DATA_BASE_H
#define STREAM_DATA_BASE_H

#include "ace/Refcountable_T.h"
#include "ace/Synch_Traits.h"

#include "common_idumpstate.h"
#include "common_iget.h"
#include "common_ireferencecount.h"

template <typename DataType>
class Stream_DataBase_T
 : public ACE_Refcountable_T<ACE_SYNCH_MUTEX>
 , public Common_IGetSet_T<DataType>
 , public Common_IReferenceCount
 , public Common_IDumpState
{
// friend class Stream_DataMessageBase_T<AllocatorConfigurationType,
//                                       Stream_DataBase_T<DataType>,
//                                       CommandType>;

 public:
  Stream_DataBase_T ();
  Stream_DataBase_T (const Stream_DataBase_T&);
  virtual ~Stream_DataBase_T ();

  // *IMPORTANT NOTE*: fire-and-forget API
  Stream_DataBase_T (DataType*&,   // data handle
                     bool = true); // delete in dtor ?

  // override assignment (support merge semantics)
  // *TODO*: enforce merge semantics
  Stream_DataBase_T& operator= (const Stream_DataBase_T&);

  // implement Common_IDumpState
  virtual void dump_state () const;

  // implement Common_IGetSet_T
  virtual const DataType& get () const;
  virtual void set (const DataType&);

  // exposed interface
  virtual unsigned int increase ();
  virtual unsigned int decrease ();
  virtual unsigned int count () const;
  // *NOTE*: should block iff the count is > 0, and wait until the count reaches
  //         x the next time
  virtual void wait (unsigned int = 0);

  // convenience types
  typedef DataType DATA_T;

 private:
  typedef ACE_Refcountable_T<ACE_SYNCH_MUTEX> inherited;

  DataType* data_;
  bool      delete_;
};

// include template implementation
#include "stream_data_base.inl"

#endif
