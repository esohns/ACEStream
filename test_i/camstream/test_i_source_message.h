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

#ifndef TEST_I_SOURCE_MESSAGE_H
#define TEST_I_SOURCE_MESSAGE_H

#include "ace/Global_Macros.h"

#include "stream_data_message_base.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "common_referencecounter.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_i_common.h"

#include "test_i_source_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_Source_DirectShow_Stream_Message
 : public Stream_DataMessageBase_T<struct Test_I_DirectShow_MessageData,
                                   enum Stream_MessageType,
                                   Test_I_CommandType_t>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_Source_DirectShow_Stream_Message,
                                                 Test_I_Source_DirectShow_SessionMessage>;

  typedef Stream_DataMessageBase_T<struct Test_I_DirectShow_MessageData,
                                   enum Stream_MessageType,
                                   Test_I_CommandType_t> inherited;

 public:
  Test_I_Source_DirectShow_Stream_Message (Stream_SessionId_t, // session id
                                           unsigned int);      // size
  inline virtual ~Test_I_Source_DirectShow_Stream_Message () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;
  virtual ACE_Message_Block* release (void);

  // implement Stream_MessageBase_T
  inline virtual Test_I_CommandType_t command () const { return ACE_Message_Block::MB_DATA; };
  static std::string CommandTypeToString (Test_I_CommandType_t);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_Source_DirectShow_Stream_Message (const Test_I_Source_DirectShow_Stream_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_DirectShow_Stream_Message ())
  // *NOTE*: to be used by message allocators
  Test_I_Source_DirectShow_Stream_Message (Stream_SessionId_t, // session id
                                           ACE_Data_Block*,    // data block to use
                                           ACE_Allocator*,     // message allocator
                                           bool = true);       // increment running message counter ?
  Test_I_Source_DirectShow_Stream_Message (Stream_SessionId_t, // session id
                                           ACE_Allocator*);    // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_DirectShow_Stream_Message& operator= (const Test_I_Source_DirectShow_Stream_Message&))
};

class Test_I_Source_MediaFoundation_Stream_Message
 : public Stream_DataMessageBase_T<struct Test_I_MediaFoundation_MessageData,
                                   enum Stream_MessageType,
                                   Test_I_CommandType_t>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_Source_MediaFoundation_Stream_Message,
                                                 Test_I_Source_MediaFoundation_SessionMessage>;

  typedef Stream_DataMessageBase_T<struct Test_I_MediaFoundation_MessageData,
                                   enum Stream_MessageType,
                                   Test_I_CommandType_t> inherited;
  
 public:
  Test_I_Source_MediaFoundation_Stream_Message (Stream_SessionId_t, // session id
                                                unsigned int);      // size
  virtual ~Test_I_Source_MediaFoundation_Stream_Message ();

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;
  virtual ACE_Message_Block* release (void);

  // implement Stream_MessageBase_T
  inline virtual Test_I_CommandType_t command () const { return ACE_Message_Block::MB_DATA; };
  static std::string CommandTypeToString (Test_I_CommandType_t);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_Source_MediaFoundation_Stream_Message (const Test_I_Source_MediaFoundation_Stream_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_MediaFoundation_Stream_Message ())
  // *NOTE*: to be used by message allocators
  Test_I_Source_MediaFoundation_Stream_Message (Stream_SessionId_t, // session id
                                                ACE_Data_Block*,    // data block to use
                                                ACE_Allocator*,     // message allocator
                                                bool = true);       // increment running message counter ?
  Test_I_Source_MediaFoundation_Stream_Message (Stream_SessionId_t, // session id
                                                ACE_Allocator*);    // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_MediaFoundation_Stream_Message& operator= (const Test_I_Source_MediaFoundation_Stream_Message&))
};
#else
class Test_I_Source_V4L_Stream_Message
 : public Stream_DataMessageBase_T<struct Test_I_V4L_MessageData,
                                   enum Stream_MessageType,
                                   Test_I_CommandType_t>
 , public Common_ReferenceCounter_T<ACE_MT_SYNCH>
{
  typedef Stream_DataMessageBase_T<struct Test_I_V4L_MessageData,
                                   enum Stream_MessageType,
                                   Test_I_CommandType_t> inherited;
  typedef Common_ReferenceCounter_T<ACE_MT_SYNCH> inherited2;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_Source_V4L_Stream_Message,
                                                 Test_I_Source_V4L_SessionMessage>;

 public:
  Test_I_Source_V4L_Stream_Message (Stream_SessionId_t, // session id
                                    unsigned int);      // size
  inline virtual ~Test_I_Source_V4L_Stream_Message () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;
  virtual ACE_Message_Block* release (void);

  // implement Stream_MessageBase_T
  inline virtual Test_I_CommandType_t command () const { return ACE_Message_Block::MB_DATA; }
  static std::string CommandTypeToString (Test_I_CommandType_t);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_Source_V4L_Stream_Message (const Test_I_Source_V4L_Stream_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_V4L_Stream_Message ())
  // *NOTE*: to be used by message allocators
  Test_I_Source_V4L_Stream_Message (Stream_SessionId_t, // session id
                                    ACE_Data_Block*,    // data block
                                    ACE_Allocator*,     // message allocator
                                    bool = true);       // increment running message counter ?
  Test_I_Source_V4L_Stream_Message (Stream_SessionId_t, // session id
                                    ACE_Allocator*);    // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_V4L_Stream_Message& operator= (const Test_I_Source_V4L_Stream_Message&))
};
#endif // ACE_WIN32 || ACE_WIN64

#endif
