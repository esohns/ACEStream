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

#include "ace/Malloc_Base.h"

#include "stream_control_message.h"
#include "stream_macros.h"

template <typename DataType>
Test_I_Message_T<DataType>::Test_I_Message_T (Stream_SessionId_t sessionId_in,
                                              size_t size_in)
 : inherited (sessionId_in,
              size_in)
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Message_T::Test_I_Message_T"));

}

template <typename DataType>
Test_I_Message_T<DataType>::Test_I_Message_T (const OWN_TYPE_T& message_in)
 : inherited (message_in)
 , mediaType_ (message_in.mediaType_)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Message_T::Test_I_Message_T"));

}

template <typename DataType>
Test_I_Message_T<DataType>::Test_I_Message_T (Stream_SessionId_t sessionId_in,
                                              ACE_Data_Block* dataBlock_in,
                                              ACE_Allocator* messageAllocator_in,
                                              bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Message_T::Test_I_Message_T"));

}

template <typename DataType>
Test_I_Message_T<DataType>::Test_I_Message_T (Stream_SessionId_t sessionId_in,
                                              ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Message_T::Test_I_Message_T"));

}

template <typename DataType>
ACE_Message_Block*
Test_I_Message_T<DataType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Message_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // if there is no allocator, use the standard new and delete calls.
  if (unlikely (!inherited::message_block_allocator_))
    ACE_NEW_NORETURN (message_p,
                      OWN_TYPE_T (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to alloc() does not really matter, as this creates
    //         a shallow copy of the existing data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->calloc (sizeof (OWN_TYPE_T),
                                                                                                    '\0')),
                             OWN_TYPE_T (*this));
  } // end ELSE
  if (unlikely (!message_p))
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Stream_CameraAR_Message_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (unlikely (!message_p->cont_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_CameraAR_Message_T::duplicate(): \"%m\", aborting\n")));
      message_p->release (); message_p = NULL;
      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

template <typename DataType>
std::string
Test_I_Message_T<DataType>::CommandTypeToString (enum Stream_MediaType_Type mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Message_T::CommandTypeToString"));

  std::string result;

  switch (mediaType_in)
  {
    case STREAM_MEDIATYPE_AUDIO:
      result = ACE_TEXT_ALWAYS_CHAR ("STREAM_MEDIATYPE_AUDIO"); break;
    case STREAM_MEDIATYPE_VIDEO:
      result = ACE_TEXT_ALWAYS_CHAR ("STREAM_MEDIATYPE_VIDEO"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media type (was: %d), aborting\n"),
                  mediaType_in));
      break;
    }
  } // end SWITCH

  return result;
}
