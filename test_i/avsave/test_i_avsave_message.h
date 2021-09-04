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

#ifndef TEST_I_AVSAVE_MESSAGE_H
#define TEST_I_AVSAVE_MESSAGE_H

#include "ace/Global_Macros.h"
#include "ace/Malloc_Base.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_data_message_base.h"

#include "test_i_common.h"

// forward declaration(s)
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T;
template <typename DataMessageType,
          typename SessionDataType>
class Stream_AVSave_SessionMessage_T;

template <typename DataType>
class Stream_AVSave_Message_T
 : public Stream_DataMessageBase_T<DataType,
                                   enum Stream_MessageType,
                                   enum Stream_MediaType_Type>
{
  typedef Stream_DataMessageBase_T<DataType,
                                   enum Stream_MessageType,
                                   enum Stream_MediaType_Type> inherited;

  // grant access to specific private ctors
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Stream_AVSave_Message_T<DataType>,
                                                 Stream_AVSave_SessionMessage_T<Stream_AVSave_Message_T<DataType>,
                                                                                Stream_AVSave_DirectShow_SessionData_t> >;
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Stream_AVSave_Message_T<DataType>,
                                                 Stream_AVSave_SessionMessage_T<Stream_AVSave_Message_T<DataType>,
                                                                                Stream_AVSave_MediaFoundation_SessionData_t> >;
#else
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Stream_AVSave_Message_T<DataType>,
                                                 Stream_AVSave_SessionMessage_T<Stream_AVSave_Message_T<DataType>,
                                                                                Stream_AVSave_ALSA_SessionData_t> >;
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Stream_AVSave_Message_T<DataType>,
                                                 Stream_AVSave_SessionMessage_T<Stream_AVSave_Message_T<DataType>,
                                                                                Stream_AVSave_V4L_SessionData_t> >;
#endif // ACE_WIN32 || ACE_WIN64

 public:
  Stream_AVSave_Message_T (unsigned int); // size
  virtual ~Stream_AVSave_Message_T ();

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy that references the same packet
  // *NOTE*: uses the allocator (if any)
  virtual ACE_Message_Block* duplicate (void) const;
  // insert this buffer back into the device incoming queue
  virtual ACE_Message_Block* release (void);

  // implement Stream_MessageBase_T
  inline virtual enum Stream_MediaType_Type command () const { return mediaType_; }
  static std::string CommandTypeToString (enum Stream_MediaType_Type);

  inline void setMediaType (enum Stream_MediaType_Type mediaType_in) { mediaType_ = mediaType_in; }
  inline enum Stream_MediaType_Type getMediaType () const { return mediaType_; }

 protected:
  // convenient types
  typedef Stream_AVSave_Message_T<DataType> OWN_TYPE_T;

  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Stream_AVSave_Message_T (const OWN_TYPE_T&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_Message_T ())
  // *NOTE*: to be used by message allocators
  Stream_AVSave_Message_T (Stream_SessionId_t,
                           ACE_Data_Block*, // data block to use
                           ACE_Allocator*,  // message allocator
                           bool = true);    // increment running message counter ?
  Stream_AVSave_Message_T (Stream_SessionId_t,
                           ACE_Allocator*); // message allocator
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_Message_T& operator= (const Stream_AVSave_Message_T&))

  enum Stream_MediaType_Type mediaType_;
};

// include template definition
#include "test_i_avsave_message.inl"

#endif

