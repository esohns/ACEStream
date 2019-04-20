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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#include <DShow.h>
#else
#include "libv4l2.h"
#include "linux/videodev2.h"
#endif

#include "stream_control_message.h"
#include "stream_macros.h"

template <typename DataType,
          typename SessionDataType>
Stream_CamSave_Message_T<DataType,
                         SessionDataType>::Stream_CamSave_Message_T (unsigned int size_in)
 : inherited (size_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message_T::Stream_CamSave_Message_T"));

}

template <typename DataType,
          typename SessionDataType>
Stream_CamSave_Message_T<DataType,
                         SessionDataType>::Stream_CamSave_Message_T (const OWN_TYPE_T& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message_T::Stream_CamSave_Message_T"));

}

template <typename DataType,
          typename SessionDataType>
Stream_CamSave_Message_T<DataType,
                         SessionDataType>::Stream_CamSave_Message_T (Stream_SessionId_t sessionId_in,
                                                                     ACE_Data_Block* dataBlock_in,
                                                                     ACE_Allocator* messageAllocator_in,
                                                                     bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message_T::Stream_CamSave_Message_T"));

}

template <typename DataType,
          typename SessionDataType>
Stream_CamSave_Message_T<DataType,
                         SessionDataType>::Stream_CamSave_Message_T (Stream_SessionId_t sessionId_in,
                                                                     ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message_T::Stream_CamSave_Message_T"));

}

template <typename DataType,
          typename SessionDataType>
Stream_CamSave_Message_T<DataType,
                         SessionDataType>::~Stream_CamSave_Message_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message_T::~Stream_CamSave_Message_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // release media sample ?
  if (inherited::data_.sample)
  {
    inherited::data_.sample->Release (); inherited::data_.sample = NULL;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
}

template <typename DataType,
          typename SessionDataType>
ACE_Message_Block*
Stream_CamSave_Message_T<DataType,
                         SessionDataType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new Stream_CamSave_Message_T that contains unique copies of
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
                  ACE_TEXT ("failed to allocate Stream_CamSave_Message_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (unlikely (!message_p->cont_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_CamSave_Message_T::duplicate(): \"%m\", aborting\n")));
      message_p->release (); message_p = NULL;
      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  // *NOTE*: duplicates may reuse the device buffer memory, but only the
  //         original message will requeue it (see release() below)
  return message_p;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
template <typename DataType,
          typename SessionDataType>
ACE_Message_Block*
Stream_CamSave_Message_T<DataType,
                         SessionDataType>::release (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message_T::release"));

  // release any continuations first
  if (inherited::cont_)
  {
    inherited::cont_->release (); inherited::cont_ = NULL;
  } // end IF

  int reference_count = inherited::reference_count ();
  if ((reference_count > 1)           || // not the last reference
      (inherited::data_.device == -1) || // not a device data buffer (-clone)
      inherited::data_.release)          // clean up (device data)
    return inherited::release ();

  // sanity check(s)
  ACE_ASSERT (inherited::data_block_);

//  // *IMPORTANT NOTE*: handle race condition here
//  { ACE_GUARD_RETURN (ACE_Lock, aGuard, *inherited::data_block_->locking_strategy (), NULL);
//    if (inherited::size ()) // is device-data ?
//      goto requeue;

//    return inherited::release ();
//  } // end lock scope

//requeue:
  struct v4l2_buffer buffer_s;
  ACE_OS::memset (&buffer_s, 0, sizeof (struct v4l2_buffer));
  buffer_s.index = inherited::data_.index;
  buffer_s.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer_s.memory = inherited::data_.method;
  switch (inherited::data_.method)
  {
    case V4L2_MEMORY_USERPTR:
    {
      buffer_s.m.userptr =
          reinterpret_cast<unsigned long> (inherited::rd_ptr ());
      buffer_s.length = inherited::size ();
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
                           &buffer_s);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                inherited::data_.device, ACE_TEXT ("VIDIOC_QBUF")));

  return NULL;
}
#endif
