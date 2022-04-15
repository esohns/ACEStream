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

#include "test_i_imagesave_message.h"

#include "ace/Malloc_Base.h"

#include "stream_control_message.h"
#include "stream_macros.h"

Test_I_Message::Test_I_Message (Stream_SessionId_t sessionId_in,
                                unsigned int size_in)
 : inherited (sessionId_in,
              size_in)
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Message::Test_I_Message"));

}

Test_I_Message::Test_I_Message (const Test_I_Message& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Message::Test_I_Message"));

}

Test_I_Message::Test_I_Message (Stream_SessionId_t sessionId_in,
                                ACE_Data_Block* dataBlock_in,
                                ACE_Allocator* messageAllocator_in,
                                bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Message::Test_I_Message"));

}

Test_I_Message::Test_I_Message (Stream_SessionId_t sessionId_in,
                                ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Message::Test_I_Message"));

}

ACE_Message_Block*
Test_I_Message::clone (ACE_Message_Block::Message_Flags flags_in) const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Message::clone"));

  ACE_UNUSED_ARG (flags_in);

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited::data_block_);

  // step1: "deep"-copy the fragment chain
  Test_I_Message* result_p = NULL;

  // *NOTE*: ACE_Data_Block::clone() does not retain the value of 'cur_size_'
  //         --> reset it
  // *TODO*: resolve ACE bugzilla issue #4219
  ACE_Data_Block* data_block_p = inherited::data_block_->clone (0);
  if (!data_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Data_Block::clone(0): \"%m\", aborting\n")));
    return NULL;
  } // end IF
  result = data_block_p->size (inherited::data_block_->size ());
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Data_Block::size(%u): \"%m\", aborting\n"),
                inherited::data_block_->size ()));
    data_block_p->release (); data_block_p = NULL;
    return NULL;
  } // end IF

  // allocate a new message that contains unique copies of the message
  // block fields, and "deep" copy(s) of the data block(s)

  // *NOTE*: if there is no allocator, use the standard new/delete calls

  if (inherited::message_block_allocator_)
  {
    // *NOTE*: the argument to calloc() doesn't matter (as long as it is not 0),
    //         the returned memory is always sizeof(Test_I_Message)
    ACE_NEW_MALLOC_NORETURN (result_p,
                             static_cast<Test_I_Message*> (inherited::message_block_allocator_->calloc (sizeof (Test_I_Message),
                                                                                                        '\0')),
                             Test_I_Message (inherited::sessionId_,
                                             data_block_p,
                                             inherited::message_block_allocator_,
                                             true));
  } // end IF
  else
    ACE_NEW_NORETURN (result_p,
                      Test_I_Message (inherited::sessionId_,
                                      data_block_p,
                                      NULL,
                                      true));
  if (!result_p)
  {
    Stream_IAllocator* allocator_p =
        dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    if (allocator_p && allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Test_I_Message: \"%m\", aborting\n")));
    data_block_p->release (NULL);
    return NULL;
  } // end IF
  // set read-/write pointers
  result_p->rd_ptr (inherited::rd_ptr_);
  result_p->wr_ptr (inherited::wr_ptr_);

  // set message type
  result_p->set (inherited::type_);

  // clone any continuations
  if (inherited::cont_)
  {
    try {
      result_p->cont (inherited::cont_->clone ());
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in ACE_Message_Block::clone(): \"%m\", continuing\n")));
    }
    if (!result_p->cont ())
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to ACE_Message_Block::clone(): \"%m\", aborting\n")));
      result_p->release (); result_p = NULL;
      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if 'this' is initialized, so is the "clone"

//  // *NOTE*: the new fragment chain is already 'crunch'ed, i.e. aligned to base_
//  // *TODO*: consider defragment()ing the chain before padding
//
//  // step2: 'pad' the fragment(s)
//  unsigned int padding_bytes =
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    AV_INPUT_BUFFER_PADDING_SIZE;
//#else
//    FF_INPUT_BUFFER_PADDING_SIZE;
//#endif
//  for (ACE_Message_Block* message_block_p = result_p;
//       message_block_p;
//       message_block_p = message_block_p->cont ())
//  { ACE_ASSERT ((message_block_p->capacity () - message_block_p->size ()) >= padding_bytes);
//    ACE_OS::memset (message_block_p->wr_ptr (), 0, padding_bytes);
//  } // end FOR

  return result_p;
}

ACE_Message_Block*
Test_I_Message::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Message::duplicate"));

  Test_I_Message* message_p = NULL;

  // create a new message that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      Test_I_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc SHOULDN'T really matter, as this will be
    //         a "shallow" copy which just references the same data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<Test_I_Message*> (inherited::message_block_allocator_->calloc (sizeof (Test_I_Message),
                                                                                                        '\0')),
                             Test_I_Message (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Stream_MessageBase_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF

    // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MessageBase_T::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)

  return message_p;
}
