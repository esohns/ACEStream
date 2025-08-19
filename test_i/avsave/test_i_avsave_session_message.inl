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

#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <typename DataMessageType,
          typename SessionDataType>
Stream_AVSave_SessionMessage_T<DataMessageType,
                               SessionDataType>::Stream_AVSave_SessionMessage_T (Stream_SessionId_t sessionId_in,
                                                                                 enum Stream_SessionMessageType messageType_in,
                                                                                 SessionDataType*& sessionData_in,
                                                                                 struct Stream_UserData* userData_in,
                                                                                 bool expedited_in)
 : inherited (sessionId_in,
              messageType_in,
              sessionData_in,
              userData_in,
              expedited_in) // expedited ?
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AVSave_SessionMessage_T::Stream_AVSave_SessionMessage_T"));

}

template <typename DataMessageType,
          typename SessionDataType>
Stream_AVSave_SessionMessage_T<DataMessageType,
                               SessionDataType>::Stream_AVSave_SessionMessage_T (const Stream_AVSave_SessionMessage_T<DataMessageType,
                                                                                                                      SessionDataType>& message_in)
 : inherited (message_in)
 , mediaType_ (message_in.mediaType_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AVSave_SessionMessage_T::Stream_AVSave_SessionMessage_T"));

}

template <typename DataMessageType,
          typename SessionDataType>
Stream_AVSave_SessionMessage_T<DataMessageType,
                               SessionDataType>::Stream_AVSave_SessionMessage_T (Stream_SessionId_t sessionId_in,
                                                                                 ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AVSave_SessionMessage_T::Stream_AVSave_SessionMessage_T"));

}

template <typename DataMessageType,
          typename SessionDataType>
Stream_AVSave_SessionMessage_T<DataMessageType,
                               SessionDataType>::Stream_AVSave_SessionMessage_T (Stream_SessionId_t sessionId_in,
                                                                                 ACE_Data_Block* dataBlock_in,
                                                                                 ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              dataBlock_in,        // use (don't own (!) memory of-) this data block
              messageAllocator_in) // message block allocator
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AVSave_SessionMessage_T::Stream_AVSave_SessionMessage_T"));

}

template <typename DataMessageType,
          typename SessionDataType>
ACE_Message_Block*
Stream_AVSave_SessionMessage_T<DataMessageType,
                               SessionDataType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_AVSave_SessionMessage_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new <Stream_AVSave_SessionMessage_T> that contains unique copies of
  // the message block fields, but a reference counted duplicate of
  // the <ACE_Data_Block>

  // if there is no allocator, use the standard new and delete calls.
  if (unlikely (!inherited::message_block_allocator_))
    ACE_NEW_RETURN (message_p,
                    OWN_TYPE_T (*this),
                    NULL);

  // *WARNING*: the allocator returns a Stream_AVSave_SessionMessage_Base_T<ConfigurationType>
  // when passing 0 as argument to malloc()...
  ACE_NEW_MALLOC_RETURN (message_p,
                         static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->malloc (0)),
                         OWN_TYPE_T (*this),
                         NULL);



  // increment the reference counts of all the continuation messages
  if (unlikely (inherited::cont_))
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (unlikely (!message_p->cont_))
    {
      message_p->release (); message_p = NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

