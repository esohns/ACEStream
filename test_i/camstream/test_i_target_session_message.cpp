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
#include "stdafx.h"

//#include "ace/Synch.h"
#include "test_i_target_session_message.h"

#include "ace/Malloc_Base.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_I_Target_DirectShow_SessionMessage::Test_I_Target_DirectShow_SessionMessage (Stream_SessionId_t sessionId_in,
                                                                                  enum Stream_SessionMessageType messageType_in,
                                                                                  Test_I_Target_DirectShow_SessionData_t*& sessionData_in,
                                                                                  struct Stream_UserData* userData_in)
 : inherited (sessionId_in,
              messageType_in,
              sessionData_in,
              userData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_DirectShow_SessionMessage::Test_I_Target_DirectShow_SessionMessage"));

}

Test_I_Target_DirectShow_SessionMessage::Test_I_Target_DirectShow_SessionMessage (const Test_I_Target_DirectShow_SessionMessage& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_DirectShow_SessionMessage::Test_I_Target_DirectShow_SessionMessage"));

}

Test_I_Target_DirectShow_SessionMessage::Test_I_Target_DirectShow_SessionMessage (Stream_SessionId_t sessionId_in,
                                                                                  ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_DirectShow_SessionMessage::Test_I_Target_DirectShow_SessionMessage"));

}

Test_I_Target_DirectShow_SessionMessage::Test_I_Target_DirectShow_SessionMessage (Stream_SessionId_t sessionId_in,
                                                                                  ACE_Data_Block* dataBlock_in,
                                                                                  ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              dataBlock_in,        // use (don't own (!) memory of-) this data block
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_DirectShow_SessionMessage::Test_I_Target_DirectShow_SessionMessage"));

}

ACE_Message_Block*
Test_I_Target_DirectShow_SessionMessage::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_DirectShow_SessionMessage::duplicate"));

  Test_I_Target_DirectShow_SessionMessage* message_p = NULL;

  // create a new <Test_I_Target_DirectShow_SessionMessage> that contains unique copies of
  // the message block fields, but a reference counted duplicate of
  // the <ACE_Data_Block>.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_RETURN (message_p,
                    Test_I_Target_DirectShow_SessionMessage (*this),
                    NULL);

  // *WARNING*: the allocator returns a Test_I_Target_DirectShow_SessionMessageBase<ConfigurationType>
  //            when passing 0 as argument to malloc()...
  ACE_NEW_MALLOC_RETURN (message_p,
                         static_cast<Test_I_Target_DirectShow_SessionMessage*> (inherited::message_block_allocator_->malloc (0)),
                         Test_I_Target_DirectShow_SessionMessage (*this),
                         NULL);

  // increment the reference counts of all the continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();

    // when things go wrong, release all resources and return
    if (message_p->cont_ == 0)
    {
      message_p->release (); message_p = NULL;
    } // end IF
  } // end IF

    // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

Test_I_Target_MediaFoundation_SessionMessage::Test_I_Target_MediaFoundation_SessionMessage (Stream_SessionId_t sessionId_in,
                                                                                            enum Stream_SessionMessageType messageType_in,
                                                                                            Test_I_Target_MediaFoundation_SessionData_t*& sessionData_in,
                                                                                            struct Stream_UserData* userData_in)
 : inherited (sessionId_in,
              messageType_in,
              sessionData_in,
              userData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_MediaFoundation_SessionMessage::Test_I_Target_MediaFoundation_SessionMessage"));

}

Test_I_Target_MediaFoundation_SessionMessage::Test_I_Target_MediaFoundation_SessionMessage (const Test_I_Target_MediaFoundation_SessionMessage& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_MediaFoundation_SessionMessage::Test_I_Target_MediaFoundation_SessionMessage"));

}

Test_I_Target_MediaFoundation_SessionMessage::Test_I_Target_MediaFoundation_SessionMessage (Stream_SessionId_t sessionId_in,
                                                                                            ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_MediaFoundation_SessionMessage::Test_I_Target_MediaFoundation_SessionMessage"));

}

Test_I_Target_MediaFoundation_SessionMessage::Test_I_Target_MediaFoundation_SessionMessage (Stream_SessionId_t sessionId_in,
                                                                                            ACE_Data_Block* dataBlock_in,
                                                                                            ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              dataBlock_in,        // use (don't own (!) memory of-) this data block
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_MediaFoundation_SessionMessage::Test_I_Target_MediaFoundation_SessionMessage"));

}

ACE_Message_Block*
Test_I_Target_MediaFoundation_SessionMessage::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_MediaFoundation_SessionMessage::duplicate"));

  Test_I_Target_MediaFoundation_SessionMessage* message_p = NULL;

  // create a new <Test_I_Target_MediaFoundation_SessionMessage> that contains unique copies of
  // the message block fields, but a reference counted duplicate of
  // the <ACE_Data_Block>.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_RETURN (message_p,
                    Test_I_Target_MediaFoundation_SessionMessage (*this),
                    NULL);

  // *WARNING*: the allocator returns a Test_I_Target_MediaFoundation_SessionMessageBase<ConfigurationType>
  //            when passing 0 as argument to malloc()...
  ACE_NEW_MALLOC_RETURN (message_p,
                         static_cast<Test_I_Target_MediaFoundation_SessionMessage*> (inherited::message_block_allocator_->malloc (0)),
                         Test_I_Target_MediaFoundation_SessionMessage (*this),
                         NULL);

  // increment the reference counts of all the continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();

    // when things go wrong, release all resources and return
    if (message_p->cont_ == 0)
    {
      message_p->release (); message_p = NULL;
    } // end IF
  } // end IF

    // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}
#else
Test_I_Target_SessionMessage::Test_I_Target_SessionMessage (Stream_SessionId_t sessionId_in,
                                                            enum Stream_SessionMessageType messageType_in,
                                                            Test_I_Target_SessionData_t*& sessionData_in,
                                                            struct Stream_UserData* userData_in)
 : inherited (sessionId_in,
              messageType_in,
              sessionData_in,
              userData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_SessionMessage::Test_I_Target_SessionMessage"));

}

Test_I_Target_SessionMessage::Test_I_Target_SessionMessage (const Test_I_Target_SessionMessage& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_SessionMessage::Test_I_Target_SessionMessage"));

}

Test_I_Target_SessionMessage::Test_I_Target_SessionMessage (Stream_SessionId_t sessionId_in,
                                                            ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_SessionMessage::Test_I_Target_SessionMessage"));

}

Test_I_Target_SessionMessage::Test_I_Target_SessionMessage (Stream_SessionId_t sessionId_in,
                                                            ACE_Data_Block* dataBlock_in,
                                                            ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              dataBlock_in,        // use (don't own (!) memory of-) this data block
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_SessionMessage::Test_I_Target_SessionMessage"));

}

ACE_Message_Block*
Test_I_Target_SessionMessage::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_SessionMessage::duplicate"));

  Test_I_Target_SessionMessage* message_p = NULL;

  // create a new <Test_I_Target_SessionMessage> that contains unique copies of
  // the message block fields, but a reference counted duplicate of
  // the <ACE_Data_Block>.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_RETURN (message_p,
                    Test_I_Target_SessionMessage (*this),
                    NULL);

  // *WARNING*: the allocator returns a Test_I_Target_SessionMessageBase<ConfigurationType>
  //            when passing 0 as argument to malloc()...
  ACE_NEW_MALLOC_RETURN (message_p,
                         static_cast<Test_I_Target_SessionMessage*> (inherited::message_block_allocator_->malloc (0)),
                         Test_I_Target_SessionMessage (*this),
                         NULL);

  // increment the reference counts of all the continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();

    // when things go wrong, release all resources and return
    if (message_p->cont_ == 0)
    {
      message_p->release (); message_p = NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}
#endif // ACE_WIN32 || ACE_WIN64
