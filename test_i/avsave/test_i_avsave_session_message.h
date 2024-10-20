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

#ifndef TEST_I_AVSAVE_SESSION_MESSAGE_H
#define TEST_I_AVSAVE_SESSION_MESSAGE_H

#include "ace/Global_Macros.h"
#include "ace/Malloc_Base.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"
#include "stream_session_data.h"
#include "stream_session_message_base.h"

#include "stream_lib_common.h"

#include "test_i_common.h"

// forward declaratation(s)
struct Stream_UserData;

template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T;

template <typename DataMessageType,
          typename SessionDataType> // derives off Stream_SessionData_T
class Stream_AVSave_SessionMessage_T
 : public Stream_SessionMessageBase_T<enum Stream_SessionMessageType,
                                      SessionDataType,
                                      struct Stream_UserData>
{
  typedef Stream_SessionMessageBase_T<enum Stream_SessionMessageType,
                                      SessionDataType,
                                      struct Stream_UserData> inherited;

  // grant access to specific private ctors
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //friend class Stream_AllocatorBase_T<ACE_MT_SYNCH,
  //                                    struct Stream_AllocatorConfiguration,
  //                                    Stream_ControlMessage_t,
  //                                    DataMessageType,
  //                                    Stream_AVSave_SessionMessage_T<DataMessageType,
  //                                                                    SessionDataType> >;
#endif // ACE_WIN32 || ACE_WIN64
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 DataMessageType,
                                                 Stream_AVSave_SessionMessage_T<DataMessageType,
                                                                                SessionDataType> >;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  // *TODO*: (using gcc) cannot pass reference to pointer for some reason
  Stream_AVSave_SessionMessage_T (Stream_SessionId_t,
                                  enum Stream_SessionMessageType,
                                  SessionDataType*&,   // session data container handle
                                  struct Stream_UserData*,
                                  bool); // expedited ?
  inline virtual ~Stream_AVSave_SessionMessage_T () {}

  inline void setMediaType (enum Stream_MediaType_Type mediaType_in) { mediaType_ = mediaType_in; }
  inline enum Stream_MediaType_Type getMediaType () const { return mediaType_; }

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  // convenient types
  typedef Stream_AVSave_SessionMessage_T<DataMessageType,
                                         SessionDataType> OWN_TYPE_T;

  // copy ctor to be used by duplicate()
  Stream_AVSave_SessionMessage_T (const Stream_AVSave_SessionMessage_T<DataMessageType,
                                                                       SessionDataType>&);

  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Stream_AVSave_SessionMessage_T (Stream_SessionId_t,
                                  ACE_Allocator*); // message allocator
  Stream_AVSave_SessionMessage_T (Stream_SessionId_t,
                                  ACE_Data_Block*, // data block
                                  ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_SessionMessage_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_SessionMessage_T& operator= (const Stream_AVSave_SessionMessage_T&))

  enum Stream_MediaType_Type mediaType_;
};

// include template definition
#include "test_i_avsave_session_message.inl"

#endif

