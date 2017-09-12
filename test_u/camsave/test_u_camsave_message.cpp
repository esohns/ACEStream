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

#include "ace/Synch.h"
#include "test_u_camsave_message.h"

#include "ace/Malloc_Base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <dshow.h>
#else
#include <libv4l2.h>
#include <linux/videodev2.h>
#endif

#include "stream_control_message.h"
#include "stream_macros.h"

Stream_CamSave_Message::Stream_CamSave_Message (unsigned int size_in)
 : inherited (size_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message::Stream_CamSave_Message"));

}

Stream_CamSave_Message::Stream_CamSave_Message (const Stream_CamSave_Message& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message::Stream_CamSave_Message"));

}

Stream_CamSave_Message::Stream_CamSave_Message (Stream_SessionId_t sessionId_in,
                                                ACE_Data_Block* dataBlock_in,
                                                ACE_Allocator* messageAllocator_in,
                                                bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message::Stream_CamSave_Message"));

}

Stream_CamSave_Message::Stream_CamSave_Message (Stream_SessionId_t sessionId_in,
                                                ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message::Stream_CamSave_Message"));

}

Stream_CamSave_Message::~Stream_CamSave_Message ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message::~Stream_CamSave_Message"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // release media sample ?
  if (inherited::data_.sample)
  {
    inherited::data_.sample->Release ();
    inherited::data_.sample = NULL;
  } // end IF
#endif
}

ACE_Message_Block*
Stream_CamSave_Message::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message::duplicate"));

  Stream_CamSave_Message* message_p = NULL;

  // create a new Stream_CamSave_Message that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      Stream_CamSave_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to alloc() does not really matter, as this creates
    //         a shallow copy of the existing data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<Stream_CamSave_Message*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
                                                                                                                 '\0')),
                             Stream_CamSave_Message (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Stream_CamSave_Message: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_CamSave_Message::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
ACE_Message_Block*
Stream_CamSave_Message::release (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message::release"));

  // release any continuations first
  if (inherited::cont_)
  {
    inherited::cont_->release ();
    inherited::cont_ = NULL;
  } // end IF

  int reference_count = inherited::reference_count ();
  if ((reference_count > 1)           || // not the last reference
      (inherited::data_.device == -1) || // not a device data buffer
      inherited::data_.release)          // clean up (device data)
    return inherited::release ();

  // sanity check(s)
  ACE_ASSERT (inherited::data_block_);

  // *IMPORTANT NOTE*: handle race condition here
  {
    ACE_GUARD_RETURN (ACE_Lock, ace_mon, *inherited::data_block_->locking_strategy (), NULL);

    if (inherited::size ()) // is device-data ?
      goto continue_;

    return NULL; // nothing to do
  } // end lock scope

continue_:
  struct v4l2_buffer buffer;
  ACE_OS::memset (&buffer, 0, sizeof (struct v4l2_buffer));
  buffer.index = inherited::data_.index;
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer.memory = inherited::data_.method;
  switch (inherited::data_.method)
  {
    case V4L2_MEMORY_USERPTR:
    {
      buffer.m.userptr =
          reinterpret_cast<unsigned long> (inherited::rd_ptr ());
      buffer.length = inherited::size ();
      break;
    }
    case V4L2_MEMORY_MMAP:
    {
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown method (was: %d), returning\n"),
                  inherited::data_.method));
      return NULL;
    }
  } // end SWITCH
  // *NOTE*: in oder to retrieve the buffer instance handle from the device
  //         buffer when it has written the frame data, the address of the
  //         ACE_Message_Block (!) could be embedded in the 'reserved' field(s).
  //         Unfortunately, this does not work, the fields seem to be zeroed by
  //         the driver
  //         --> maintain a mapping: buffer index <--> buffer handle
//        buffer.reserved = reinterpret_cast<unsigned long> (message_block_p);
  int result = v4l2_ioctl (inherited::data_.device,
                           VIDIOC_QBUF,
                           &buffer);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                inherited::data_.device, ACE_TEXT ("VIDIOC_QBUF")));

  return NULL;
}
#endif

//std::string
//Stream_CamSave_Message::CommandTypeToString (Stream_CommandType_t command_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message::CommandTypeToString"));
//
//  ACE_UNUSED_ARG (command_in);
//
//  return ACE_TEXT_ALWAYS_CHAR ("MB_DATA");
//}
