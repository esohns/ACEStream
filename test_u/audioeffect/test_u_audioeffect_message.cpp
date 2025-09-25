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

#include "test_u_audioeffect_message.h"

#include "ace/Malloc_Base.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_AudioEffect_DirectShow_Message::Test_U_AudioEffect_DirectShow_Message (Stream_SessionId_t sessionId_in,
                                                                              unsigned int size_in)
 : inherited (sessionId_in,
              size_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Message::Test_U_AudioEffect_DirectShow_Message"));

}

Test_U_AudioEffect_DirectShow_Message::Test_U_AudioEffect_DirectShow_Message (const Test_U_AudioEffect_DirectShow_Message& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Message::Test_U_AudioEffect_DirectShow_Message"));

}

Test_U_AudioEffect_DirectShow_Message::Test_U_AudioEffect_DirectShow_Message (Stream_SessionId_t sessionId_in,
                                                                              ACE_Data_Block* dataBlock_in,
                                                                              ACE_Allocator* messageAllocator_in,
                                                                              bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Message::Test_U_AudioEffect_DirectShow_Message"));

}

Test_U_AudioEffect_DirectShow_Message::Test_U_AudioEffect_DirectShow_Message (Stream_SessionId_t sessionId_in,
                                                                              ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Message::Test_U_AudioEffect_DirectShow_Message"));

}

Test_U_AudioEffect_DirectShow_Message::~Test_U_AudioEffect_DirectShow_Message ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Message::~Test_U_AudioEffect_DirectShow_Message"));

  // release media sample ?
  //if (inherited::data_.sample)
  //{
  //  inherited::data_.sample->Release (); inherited::data_.sample = NULL;
  //} // end IF
}

ACE_Message_Block*
Test_U_AudioEffect_DirectShow_Message::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Message::duplicate"));

  Test_U_AudioEffect_DirectShow_Message* message_p = NULL;

  // create a new Stream_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      Test_U_AudioEffect_DirectShow_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<Test_U_AudioEffect_DirectShow_Message*> (inherited::message_block_allocator_->calloc (sizeof (Test_U_AudioEffect_DirectShow_Message),
                                                                                                                               '\0')),
                             Test_U_AudioEffect_DirectShow_Message (*this));
  } // end ELSE
  if (unlikely (!message_p))
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    if ((allocator_p && allocator_p->block ()) ||
        !allocator_p)
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Test_U_AudioEffect_DirectShow_Message: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (unlikely (!message_p->cont_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Test_U_AudioEffect_DirectShow_Message::duplicate(): \"%m\", aborting\n")));
      message_p->release ();
      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  // increase reference count
  if (message_p->data_.sample)
    message_p->data_.sample->AddRef ();
  // reset data index
  message_p->data_.index = -1;

  return message_p;
}

ACE_Message_Block*
Test_U_AudioEffect_DirectShow_Message::release (void)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Message::release"));

  // release any continuations first
  if (inherited::cont_)
  {
    inherited::cont_->release (); inherited::cont_ = NULL;
  } // end IF

  // DirectShow buffer ?
  if (inherited::data_.sample)
  {
    inherited::data_.sample->Release (); inherited::data_.sample = NULL;
  } // end IF

  return inherited::release ();
}

std::string
Test_U_AudioEffect_DirectShow_Message::CommandTypeToString (Stream_CommandType_t command_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Message::CommandTypeToString"));

  ACE_UNUSED_ARG (command_in);

  return ACE_TEXT_ALWAYS_CHAR ("MB_DATA");
}

//////////////////////////////////////////

Test_U_AudioEffect_MediaFoundation_Message::Test_U_AudioEffect_MediaFoundation_Message (Stream_SessionId_t sessionId_in,
                                                                                        unsigned int size_in)
 : inherited (sessionId_in,
              size_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Message::Test_U_AudioEffect_MediaFoundation_Message"));

}

Test_U_AudioEffect_MediaFoundation_Message::Test_U_AudioEffect_MediaFoundation_Message (const Test_U_AudioEffect_MediaFoundation_Message& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Message::Test_U_AudioEffect_MediaFoundation_Message"));

}

Test_U_AudioEffect_MediaFoundation_Message::Test_U_AudioEffect_MediaFoundation_Message (Stream_SessionId_t sessionId_in,
                                                                                        ACE_Data_Block* dataBlock_in,
                                                                                        ACE_Allocator* messageAllocator_in,
                                                                                        bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,        // use (don't own (!) memory of-) this data block
              messageAllocator_in, // re-use the same allocator
              incrementMessageCounter_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Message::Test_U_AudioEffect_MediaFoundation_Message"));

}

Test_U_AudioEffect_MediaFoundation_Message::Test_U_AudioEffect_MediaFoundation_Message (Stream_SessionId_t sessionId_in,
                                                                                        ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Message::Test_U_AudioEffect_MediaFoundation_Message"));

}

Test_U_AudioEffect_MediaFoundation_Message::~Test_U_AudioEffect_MediaFoundation_Message ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Message::~Test_U_AudioEffect_MediaFoundation_Message"));

  // release media sample ?
  if (inherited::data_.sample)
  {
    inherited::data_.sample->Release ();
    inherited::data_.sample = NULL;
  } // end IF
}

ACE_Message_Block*
Test_U_AudioEffect_MediaFoundation_Message::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Message::duplicate"));

  Test_U_AudioEffect_MediaFoundation_Message* message_p = NULL;

  // create a new Stream_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      Test_U_AudioEffect_MediaFoundation_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<Test_U_AudioEffect_MediaFoundation_Message*> (inherited::message_block_allocator_->calloc (sizeof (Test_U_AudioEffect_MediaFoundation_Message),
                                                                                                                                    '\0')),
                             Test_U_AudioEffect_MediaFoundation_Message (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    if ((allocator_p && allocator_p->block ()) ||
        !allocator_p)
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Stream_MessageBase: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MessageBase::duplicate(): \"%m\", aborting\n")));
      message_p->release ();
      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

//const Stream_HeaderType_t&
//Test_U_AudioEffect_MediaFoundation_Message::get () const
//{
//  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Message::get"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (-1);
//
//  ACE_NOTREACHED (return -1;)
//}

std::string
Test_U_AudioEffect_MediaFoundation_Message::CommandTypeToString (Stream_CommandType_t command_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Message::CommandTypeToString"));

  ACE_UNUSED_ARG (command_in);

  return ACE_TEXT_ALWAYS_CHAR ("MB_DATA");
}
#else
Test_U_AudioEffect_Message::Test_U_AudioEffect_Message (Stream_SessionId_t sessionId_in,
                                                        unsigned int size_in)
 : inherited (sessionId_in,
              size_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Message::Test_U_AudioEffect_Message"));

}

Test_U_AudioEffect_Message::Test_U_AudioEffect_Message (const Test_U_AudioEffect_Message& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Message::Test_U_AudioEffect_Message"));

}

Test_U_AudioEffect_Message::Test_U_AudioEffect_Message (Stream_SessionId_t sessionId_in,
                                                        ACE_Data_Block* dataBlock_in,
                                                        ACE_Allocator* messageAllocator_in,
                                                        bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,        // use (don't own (!) memory of-) this data block
              messageAllocator_in, // re-use the same allocator
              incrementMessageCounter_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Message::Test_U_AudioEffect_Message"));

}

Test_U_AudioEffect_Message::Test_U_AudioEffect_Message (Stream_SessionId_t sessionId_in,
                                                        ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Message::Test_U_AudioEffect_Message"));

}

ACE_Message_Block*
Test_U_AudioEffect_Message::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Message::duplicate"));

  Test_U_AudioEffect_Message* message_p = NULL;

  // create a new Stream_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      Test_U_AudioEffect_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to alloc() does not really matter, as this creates
    //         a shallow copy of the existing data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<Test_U_AudioEffect_Message*> (inherited::message_block_allocator_->calloc (sizeof (Test_U_AudioEffect_Message),
                                                                                                                    '\0')),
                             Test_U_AudioEffect_Message (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Stream_MessageBase: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MessageBase::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

std::string
Test_U_AudioEffect_Message::CommandTypeToString (Stream_CommandType_t command_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Message::CommandTypeToString"));

  ACE_UNUSED_ARG (command_in);

  return ACE_TEXT_ALWAYS_CHAR ("MB_DATA");
}
#endif
