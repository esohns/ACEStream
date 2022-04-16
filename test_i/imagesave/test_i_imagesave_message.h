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

#ifndef TEST_I_MESSAGE_H
#define TEST_I_MESSAGE_H

#include "ace/Global_Macros.h"

#include "common_configuration.h"

#include "stream_common.h"
#include "stream_message_base.h"

#include "stream_lib_imediatype.h"

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
class Test_I_SessionMessage_T;
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;

class Test_I_Message
 : public Stream_MessageBase_T<enum Stream_MessageType,
                               int>
 , public Stream_IMediaType
{
  typedef Stream_MessageBase_T<enum Stream_MessageType,
                               int> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
#if defined (FFMPEG_SUPPORT)
                                                 struct Stream_MediaFramework_FFMPEG_AllocatorConfiguration,
#else
                                                 struct Stream_AllocatorConfiguration,
#endif // FFMPEG_SUPPORT
                                                 Stream_ControlMessage_t,
                                                 Test_I_Message,
                                                 Test_I_SessionMessage_T<Test_I_Message,
                                                                         Test_I_ImageSave_SessionData_t> >;

 public:
  Test_I_Message (Stream_SessionId_t, // session id
                  unsigned int);      // size
  inline virtual ~Test_I_Message () {}

  // overrides from ACE_Message_Block
  // create a "deep" copy
  // *NOTE*: uses the allocator (if any)
  virtual ACE_Message_Block* clone (ACE_Message_Block::Message_Flags = 0) const;
  // --> create a "shallow" copy that references the same packet
  // *NOTE*: uses the allocator (if any)
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_MessageBase_T
  inline virtual int command () const { return ACE_Message_Block::MB_DATA; }

  // implement Stream_IMediaType
  inline virtual enum Stream_MediaType_Type getMediaType () const { return mediaType_; }
  inline virtual void setMediaType (enum Stream_MediaType_Type mediaType_in) { mediaType_ = mediaType_in; }

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  //Test_I_Message (const Test_I_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Message ())
  // *NOTE*: to be used by message allocators
  Test_I_Message (Stream_SessionId_t, // session id
                  ACE_Data_Block*,    // data block to use
                  ACE_Allocator*,     // message allocator
                  bool = true);       // increment running message counter ?
  Test_I_Message (Stream_SessionId_t, // session id
                  ACE_Allocator*);    // message allocator
  Test_I_Message (const Test_I_Message&);
  ACE_UNIMPLEMENTED_FUNC (Test_I_Message& operator= (const Test_I_Message&))

  enum Stream_MediaType_Type mediaType_;
};

#endif
