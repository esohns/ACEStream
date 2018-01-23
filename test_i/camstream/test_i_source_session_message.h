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

#ifndef TEST_I_SOURCE_SESSION_MESSAGE_H
#define TEST_I_SOURCE_SESSION_MESSAGE_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "stream_common.h"
#include "stream_session_message_base.h"

#include "test_i_source_common.h"

// forward declaration(s)
class ACE_Allocator;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_Source_DirectShow_Stream_SessionMessage
 : public Stream_SessionMessageBase_T<struct Test_I_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_Source_DirectShow_SessionData_t,
                                      struct Test_I_Source_DirectShow_UserData>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Test_I_AllocatorConfiguration,
                                                 Test_I_ControlMessage_t,
                                                 Test_I_Source_DirectShow_Stream_Message,
                                                 Test_I_Source_DirectShow_Stream_SessionMessage>;

  typedef Stream_SessionMessageBase_T<struct Test_I_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_Source_DirectShow_SessionData_t,
                                      struct Test_I_Source_DirectShow_UserData> inherited;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  // *TODO*: (using gcc) cannot pass reference to pointer for some reason
  Test_I_Source_DirectShow_Stream_SessionMessage (Stream_SessionId_t,
                                                  enum Stream_SessionMessageType,
                                                  Test_I_Source_DirectShow_SessionData_t*&,   // session data container handle
                                                  struct Test_I_Source_DirectShow_UserData*);
  inline virtual ~Test_I_Source_DirectShow_Stream_SessionMessage () {}

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  // copy ctor to be used by duplicate()
  Test_I_Source_DirectShow_Stream_SessionMessage (const Test_I_Source_DirectShow_Stream_SessionMessage&);

  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Test_I_Source_DirectShow_Stream_SessionMessage (Stream_SessionId_t,
                                                  ACE_Allocator*); // message allocator
  Test_I_Source_DirectShow_Stream_SessionMessage (Stream_SessionId_t,
                                                  ACE_Data_Block*, // data block to use
                                                  ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_DirectShow_Stream_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_DirectShow_Stream_SessionMessage& operator= (const Test_I_Source_DirectShow_Stream_SessionMessage&))
};

class Test_I_Source_MediaFoundation_Stream_SessionMessage
 : public Stream_SessionMessageBase_T<struct Test_I_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_Source_MediaFoundation_SessionData_t,
                                      struct Test_I_Source_MediaFoundation_UserData>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Test_I_AllocatorConfiguration,
                                                 Test_I_ControlMessage_t,
                                                 Test_I_Source_MediaFoundation_Stream_Message,
                                                 Test_I_Source_MediaFoundation_Stream_SessionMessage>;

  typedef Stream_SessionMessageBase_T<struct Test_I_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_Source_MediaFoundation_SessionData_t,
                                      struct Test_I_Source_MediaFoundation_UserData> inherited;
  
 public:
  // *NOTE*: assumes responsibility for the second argument !
  // *TODO*: (using gcc) cannot pass reference to pointer for some reason
  Test_I_Source_MediaFoundation_Stream_SessionMessage (Stream_SessionId_t,
                                                       enum Stream_SessionMessageType,
                                                       Test_I_Source_MediaFoundation_SessionData_t*&,   // session data container handle
                                                       struct Test_I_Source_MediaFoundation_UserData*);
  inline virtual ~Test_I_Source_MediaFoundation_Stream_SessionMessage () {}

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  // copy ctor to be used by duplicate()
  Test_I_Source_MediaFoundation_Stream_SessionMessage (const Test_I_Source_MediaFoundation_Stream_SessionMessage&);

  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Test_I_Source_MediaFoundation_Stream_SessionMessage (Stream_SessionId_t,
                                                       ACE_Allocator*); // message allocator
  Test_I_Source_MediaFoundation_Stream_SessionMessage (Stream_SessionId_t,
                                                       ACE_Data_Block*, // data block to use
                                                       ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_MediaFoundation_Stream_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_MediaFoundation_Stream_SessionMessage& operator= (const Test_I_Source_MediaFoundation_Stream_SessionMessage&))
};
#else
class Test_I_Source_V4L2_Stream_SessionMessage
 : public Stream_SessionMessageBase_T<struct Test_I_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_Source_V4L2_SessionData_t,
                                      struct Test_I_Source_V4L2_UserData>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Test_I_AllocatorConfiguration,
                                                 Test_I_ControlMessage_t,
                                                 Test_I_Source_V4L2_Stream_Message,
                                                 Test_I_Source_V4L2_Stream_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  // *TODO*: (using gcc) cannot pass reference to pointer for some reason
  Test_I_Source_V4L2_Stream_SessionMessage (Stream_SessionId_t,
                                            enum Stream_SessionMessageType,
                                            Test_I_Source_V4L2_SessionData_t*&,   // session data container handle
                                            struct Test_I_Source_V4L2_UserData*);
  inline virtual ~Test_I_Source_V4L2_Stream_SessionMessage () {}

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  typedef Stream_SessionMessageBase_T<struct Test_I_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_Source_V4L2_SessionData_t,
                                      struct Test_I_Source_V4L2_UserData> inherited;

  // copy ctor to be used by duplicate()
  Test_I_Source_V4L2_Stream_SessionMessage (const Test_I_Source_V4L2_Stream_SessionMessage&);

  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Test_I_Source_V4L2_Stream_SessionMessage (Stream_SessionId_t,
                                            ACE_Allocator*); // message allocator
  Test_I_Source_V4L2_Stream_SessionMessage (Stream_SessionId_t,
                                            ACE_Data_Block*, // data block to use
                                            ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_V4L2_Stream_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_V4L2_Stream_SessionMessage& operator= (const Test_I_Source_V4L2_Stream_SessionMessage&))
};
#endif

#endif
