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

#include <ace/Refcountable_T.h>
#include <ace/Synch_Traits.h>

#include "common_idumpstate.h"
#include "common_iget.h"
#include "common_ireferencecount.h"

template <typename DataType>
class Stream_DataBase_T
 : public ACE_Refcountable_T<ACE_SYNCH_MUTEX>
 , public Common_IGetSetR_T<DataType>
 , public Common_IReferenceCount
 , public Common_IDumpState
{
// friend class Stream_DataMessageBase_T<AllocatorConfigurationType,
//                                       Stream_DataBase_T<DataType>,
//                                       CommandType>;

 public:
  Stream_DataBase_T ();
  // *IMPORTANT NOTE*: fire-and-forget API
  Stream_DataBase_T (DataType*&,   // data handle
                     bool = true); // delete in dtor ?
  Stream_DataBase_T (const Stream_DataBase_T&);
  inline virtual ~Stream_DataBase_T () { if (data_ && delete_) delete data_; };

  // override assignment (support merge semantics)
  // *TODO*: enforce merge semantics
  Stream_DataBase_T& operator= (const Stream_DataBase_T&);

  // implement Common_IDumpState
  virtual void dump_state () const;

  // implement Common_IGetSetR_T
  inline virtual const DataType& get () const { ACE_ASSERT (data_); return *data_; };
  virtual void set (const DataType&);

  // fire-and-forget API
  virtual void set (DataType*&);

  // exposed interface
  inline virtual unsigned int increase () { return static_cast<unsigned int> (inherited::increment ()); };
  virtual unsigned int decrease ();
  inline virtual unsigned int count () const { return static_cast<unsigned int> (inherited::refcount_.value ()); };
  // *NOTE*: should block iff the count is > 0, and wait until the count reaches
  //         x the next time
  inline virtual void wait (unsigned int = 0) const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

  // convenience types
  typedef DataType DATA_T;

 private:
  typedef ACE_Refcountable_T<ACE_SYNCH_MUTEX> inherited;

  DataType* data_;
  bool      delete_;
};

// include template definition
#include "stream_data_base.inl"

#endif
