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

#ifndef TEST_U_AUDIOEFFECT_SESSION_MESSAGE_H
#define TEST_U_AUDIOEFFECT_SESSION_MESSAGE_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "stream_common.h"
#include "stream_session_message_base.h"

#include "test_u_audioeffect_common.h"

// forward declaratation(s)
class ACE_Allocator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_U_AudioEffect_MediaFoundation_Message;
class Test_U_AudioEffect_DirectShow_Message;
#else
class Test_U_AudioEffect_Message;
#endif
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_U_AudioEffect_DirectShow_SessionMessage
 : public Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      Stream_SessionMessageType,
                                      Test_U_AudioEffect_DirectShow_SessionData_t,
                                      Stream_UserData,
                                      Test_U_AudioEffect_DirectShow_ControlMessage_t,
                                      Test_U_AudioEffect_DirectShow_Message>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                                 Test_U_AudioEffect_DirectShow_ControlMessage_t,
                                                 Test_U_AudioEffect_DirectShow_Message,
                                                 Test_U_AudioEffect_DirectShow_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  // *TODO*: (using gcc) cannot pass reference to pointer for some reason
  Test_U_AudioEffect_DirectShow_SessionMessage (Stream_SessionMessageType,                     // session message type
                                                Test_U_AudioEffect_DirectShow_SessionData_t*&, // session data container handle
                                                Stream_UserData*);                             // user data handle
  virtual ~Test_U_AudioEffect_DirectShow_SessionMessage ();

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  typedef Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      Stream_SessionMessageType,
                                      Test_U_AudioEffect_DirectShow_SessionData_t,
                                      Stream_UserData,
                                      Test_U_AudioEffect_DirectShow_ControlMessage_t,
                                      Test_U_AudioEffect_DirectShow_Message> inherited;

  // copy ctor to be used by duplicate()
  Test_U_AudioEffect_DirectShow_SessionMessage (const Test_U_AudioEffect_DirectShow_SessionMessage&);

  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Test_U_AudioEffect_DirectShow_SessionMessage (ACE_Allocator*); // message allocator
  Test_U_AudioEffect_DirectShow_SessionMessage (ACE_Data_Block*, // data block
                                                ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_DirectShow_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_DirectShow_SessionMessage& operator= (const Test_U_AudioEffect_DirectShow_SessionMessage&))
};

//////////////////////////////////////////

class Test_U_AudioEffect_MediaFoundation_SessionMessage
 : public Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      Stream_SessionMessageType,
                                      Test_U_AudioEffect_MediaFoundation_SessionData_t,
                                      Stream_UserData,
                                      Test_U_AudioEffect_MediaFoundation_ControlMessage_t,
                                      Test_U_AudioEffect_MediaFoundation_Message>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                                 Test_U_AudioEffect_MediaFoundation_ControlMessage_t,
                                                 Test_U_AudioEffect_MediaFoundation_Message,
                                                 Test_U_AudioEffect_MediaFoundation_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  // *TODO*: (using gcc) cannot pass reference to pointer for some reason
  Test_U_AudioEffect_MediaFoundation_SessionMessage (Stream_SessionMessageType,                          // session message type
                                                     Test_U_AudioEffect_MediaFoundation_SessionData_t*&, // session data container handle
                                                     Stream_UserData*);                                  // user data handle
  virtual ~Test_U_AudioEffect_MediaFoundation_SessionMessage ();

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  typedef Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      Stream_SessionMessageType,
                                      Test_U_AudioEffect_MediaFoundation_SessionData_t,
                                      Stream_UserData,
                                      Test_U_AudioEffect_MediaFoundation_ControlMessage_t,
                                      Test_U_AudioEffect_MediaFoundation_Message> inherited;

  // copy ctor to be used by duplicate()
  Test_U_AudioEffect_MediaFoundation_SessionMessage (const Test_U_AudioEffect_MediaFoundation_SessionMessage&);

  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Test_U_AudioEffect_MediaFoundation_SessionMessage (ACE_Allocator*); // message allocator
  Test_U_AudioEffect_MediaFoundation_SessionMessage (ACE_Data_Block*, // data block
                                                     ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_MediaFoundation_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_MediaFoundation_SessionMessage& operator= (const Test_U_AudioEffect_MediaFoundation_SessionMessage&))
};
#else
class Test_U_AudioEffect_SessionMessage
 : public Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      Stream_SessionMessageType,
                                      Test_U_AudioEffect_SessionData_t,
                                      Stream_UserData,
                                      Test_U_AudioEffect_ControlMessage_t,
                                      Test_U_AudioEffect_Message>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                                 Test_U_AudioEffect_ControlMessage_t,
                                                 Test_U_AudioEffect_Message,
                                                 Test_U_AudioEffect_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  // *TODO*: (using gcc) cannot pass reference to pointer for some reason
  Test_U_AudioEffect_SessionMessage (Stream_SessionMessageType,          // session message type
                                     Test_U_AudioEffect_SessionData_t*&, // session data container handle
                                     Stream_UserData*);                  // user data handle
  virtual ~Test_U_AudioEffect_SessionMessage ();

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  typedef Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      Stream_SessionMessageType,
                                      Test_U_AudioEffect_SessionData_t,
                                      Stream_UserData,
                                      Test_U_AudioEffect_ControlMessage_t,
                                      Test_U_AudioEffect_Message> inherited;

  // copy ctor to be used by duplicate()
  Test_U_AudioEffect_SessionMessage (const Test_U_AudioEffect_SessionMessage&);

  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Test_U_AudioEffect_SessionMessage (ACE_Allocator*); // message allocator
  Test_U_AudioEffect_SessionMessage (ACE_Data_Block*, // data block
                                     ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_SessionMessage& operator= (const Test_U_AudioEffect_SessionMessage&))
};
#endif

#endif
