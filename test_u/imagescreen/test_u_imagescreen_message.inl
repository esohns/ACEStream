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

#if defined (IMAGEMAGICK_SUPPORT)
#if defined (IMAGEMAGICK_IS_GRAPHICSMAGICK)
#include "wand/wand_api.h"
#else
#if defined (ACE_LINUX)
#if defined (IS_UBUNTU_LINUX) // *NOTE*: github "*-latest" runners lag behind:
#include "wand/MagickWand.h" //          - Ubuntu 'noble' still is on ImageMagick-6
#else
#include "MagickWand/MagickWand.h"
#endif // IS_UBUNTU_LINUX
#else
#include "MagickWand/MagickWand.h"
#endif // ACE_LINUX
#endif // IMAGEMAGICK_IS_GRAPHICSMAGICK
#endif // IMAGEMAGICK_SUPPORT

#include "ace/Malloc_Base.h"

#include "stream_control_message.h"
#include "stream_macros.h"

template <typename SessionDataType>
Stream_ImageScreen_Message_T<SessionDataType>::Stream_ImageScreen_Message_T (Stream_SessionId_t sessionId_in,
                                                                             size_t size_in)
 : inherited (sessionId_in,
              size_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_Message_T::Stream_ImageScreen_Message_T"));

}

template <typename SessionDataType>
Stream_ImageScreen_Message_T<SessionDataType>::Stream_ImageScreen_Message_T (const OWN_TYPE_T& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_Message_T::Stream_ImageScreen_Message_T"));

}

template <typename SessionDataType>
Stream_ImageScreen_Message_T<SessionDataType>::Stream_ImageScreen_Message_T (Stream_SessionId_t sessionId_in,
                                                                             ACE_Data_Block* dataBlock_in,
                                                                             ACE_Allocator* messageAllocator_in,
                                                                             bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_Message_T::Stream_ImageScreen_Message_T"));

}

template <typename SessionDataType>
Stream_ImageScreen_Message_T<SessionDataType>::Stream_ImageScreen_Message_T (Stream_SessionId_t sessionId_in,
                                                                             ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_Message_T::Stream_ImageScreen_Message_T"));

}

template <typename SessionDataType>
Stream_ImageScreen_Message_T<SessionDataType>::~Stream_ImageScreen_Message_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_Message_T::~Stream_ImageScreen_Message_T"));

#if defined (IMAGEMAGICK_SUPPORT)
  if (inherited::data_.relinquishMemory)
  {
    MagickRelinquishMemory (inherited::data_.relinquishMemory); inherited::data_.relinquishMemory = NULL;
  } // end IF
#endif // IMAGEMAGICK_SUPPORT
}

template <typename SessionDataType>
ACE_Message_Block*
Stream_ImageScreen_Message_T<SessionDataType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_Message_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new Stream_ImageScreen_Message_T that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls.
  if (unlikely (!inherited::message_block_allocator_))
    ACE_NEW_NORETURN (message_p,
                      OWN_TYPE_T (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to alloc() does not really matter, as this creates
    //         a shallow copy of the existing data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
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
                  ACE_TEXT ("failed to allocate Stream_ImageScreen_Message_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (unlikely (!message_p->cont_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_ImageScreen_Message_T::duplicate(): \"%m\", aborting\n")));
      message_p->release (); message_p = NULL;
      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

//  // *NOTE*: duplicates may reuse the device buffer memory, but only the
//  //         original message will requeue it (see release() below)
//  DataType& data_r = const_cast<DataType&> (message_p->getR ());
//  data_r.device = -1;

  return message_p;
}
