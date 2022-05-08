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
#else
#include "libv4l2.h"
#include "linux/videodev2.h"
#endif

#include "stream_control_message.h"
#include "stream_macros.h"

template <typename DataType,
          typename SessionDataType>
Stream_CamSave_Message_T<DataType,
                         SessionDataType>::Stream_CamSave_Message_T (Stream_SessionId_t sessionId_in,
                                                                     unsigned int size_in)
 : inherited (sessionId_in,
              size_in)
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
                         SessionDataType>::clone (ACE_Message_Block::Message_Flags flags_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message_T::clone"));

  ACE_UNUSED_ARG (flags_in);

  int result = -1;
  size_t current_size = 0;

  // sanity check(s)
  ACE_ASSERT (inherited::data_block_);

  // step1: "deep"-copy the fragment chain
  OWN_TYPE_T* result_p = NULL;

  current_size = inherited::data_block_->size ();
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
  result = data_block_p->size (current_size);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Data_Block::size(%u): \"%m\", aborting\n"),
                current_size));

    // clean up
    data_block_p->release ();

    return NULL;
  } // end IF

  // allocate a new Stream_CamSave_Message_T that contains unique copies of the message
  // block fields, and "deep" copy(s) of the data block(s)

  // *NOTE*: if there is no allocator, use the standard new/delete calls

  if (inherited::message_block_allocator_)
  {
    // *NOTE*: the argument to calloc() doesn't matter (as long as it is not 0),
    //         the returned memory is always sizeof(Stream_CamSave_Message_T)
    ACE_NEW_MALLOC_NORETURN (result_p,
                             static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->calloc (sizeof (OWN_TYPE_T),
                                                                                                    '\0')),
                             OWN_TYPE_T (inherited::sessionId_,
                                         data_block_p,
                                         inherited::message_block_allocator_,
                                         true));
  } // end IF
  else
    ACE_NEW_NORETURN (result_p,
                      OWN_TYPE_T (inherited::sessionId_,
                                  data_block_p,
                                  NULL,
                                  true));
  if (!result_p)
  {
    Stream_IAllocator* allocator_p =
        dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    if (allocator_p && allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Stream_CamSave_Message_T: \"%m\", aborting\n")));
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
      result_p->cont_ = inherited::cont_->clone ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in ACE_Message_Block::clone(): \"%m\", continuing\n")));
    }
    if (!result_p->cont_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to ACE_Message_Block::clone(): \"%m\", aborting\n")));

      // clean up
      result_p->release ();

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

template <typename DataType,
          typename SessionDataType>
ACE_Message_Block*
Stream_CamSave_Message_T<DataType,
                         SessionDataType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // if there is no allocator, use the standard new and delete calls.
  ACE_NEW_NORETURN (message_p,
                    OWN_TYPE_T (this->sessionId (),
                                this->length ()));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Stream_CamSave_Message_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF
  int result = message_p->copy (this->rd_ptr (),
                                this->length ());
  ACE_ASSERT (result == 0);

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
template <typename DataType,
          typename SessionDataType>
Stream_CamSave_LibCamera_Message_T<DataType,
                                   SessionDataType>::Stream_CamSave_LibCamera_Message_T (Stream_SessionId_t sessionId_in,
                                                                                         unsigned int size_in)
 : inherited (sessionId_in,
              size_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_LibCamera_Message_T::Stream_CamSave_LibCamera_Message_T"));

}

template <typename DataType,
          typename SessionDataType>
Stream_CamSave_LibCamera_Message_T<DataType,
                                   SessionDataType>::Stream_CamSave_LibCamera_Message_T (const OWN_TYPE_T& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_LibCamera_Message_T::Stream_CamSave_LibCamera_Message_T"));

}

template <typename DataType,
          typename SessionDataType>
Stream_CamSave_LibCamera_Message_T<DataType,
                                   SessionDataType>::Stream_CamSave_LibCamera_Message_T (Stream_SessionId_t sessionId_in,
                                                                                         ACE_Data_Block* dataBlock_in,
                                                                                         ACE_Allocator* messageAllocator_in,
                                                                                         bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_LibCamera_Message_T::Stream_CamSave_LibCamera_Message_T"));

}

template <typename DataType,
          typename SessionDataType>
Stream_CamSave_LibCamera_Message_T<DataType,
                                   SessionDataType>::Stream_CamSave_LibCamera_Message_T (Stream_SessionId_t sessionId_in,
                                                                                         ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Message_T::Stream_CamSave_Message_T"));

}

template <typename DataType,
          typename SessionDataType>
Stream_CamSave_LibCamera_Message_T<DataType,
                                   SessionDataType>::~Stream_CamSave_LibCamera_Message_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_LibCamera_Message_T::~Stream_CamSave_LibCamera_Message_T"));

}

template <typename DataType,
          typename SessionDataType>
ACE_Message_Block*
Stream_CamSave_LibCamera_Message_T<DataType,
                                   SessionDataType>::clone (ACE_Message_Block::Message_Flags flags_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_LibCamera_Message_T::clone"));

  ACE_UNUSED_ARG (flags_in);

  int result = -1;
  size_t current_size = 0;

  // sanity check(s)
  ACE_ASSERT (inherited::data_block_);

  // step1: "deep"-copy the fragment chain
  OWN_TYPE_T* result_p = NULL;

  current_size = inherited::data_block_->size ();
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
  result = data_block_p->size (current_size);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Data_Block::size(%u): \"%m\", aborting\n"),
                current_size));

    // clean up
    data_block_p->release ();

    return NULL;
  } // end IF

  // allocate a new ARDrone_LiveVideoMessage that contains unique copies of the message
  // block fields, and "deep" copie(s) of the data block(s)

  // *NOTE*: if there is no allocator, use the standard new/delete calls

  if (inherited::message_block_allocator_)
  {
    // *NOTE*: the argument to calloc() doesn't matter (as long as it is not 0),
    //         the returned memory is always sizeof(ARDrone_LiveVideoMessage)
    ACE_NEW_MALLOC_NORETURN (result_p,
                             static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->calloc (sizeof (OWN_TYPE_T),
                                                                                                    '\0')),
                             OWN_TYPE_T (inherited::sessionId_,
                                         data_block_p,
                                         inherited::message_block_allocator_,
                                         true));
  } // end IF
  else
    ACE_NEW_NORETURN (result_p,
                      OWN_TYPE_T (inherited::sessionId_,
                                  data_block_p,
                                  NULL,
                                  true));
  if (!result_p)
  {
    Stream_IAllocator* allocator_p =
        dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate ARDrone_LiveVideoMessage: \"%m\", aborting\n")));

    // clean up
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
      result_p->cont_ = inherited::cont_->clone ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in ACE_Message_Block::clone(): \"%m\", continuing\n")));
    }
    if (!result_p->cont_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to ACE_Message_Block::clone(): \"%m\", aborting\n")));

      // clean up
      result_p->release ();

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

template <typename DataType,
          typename SessionDataType>
ACE_Message_Block*
Stream_CamSave_LibCamera_Message_T<DataType,
                                   SessionDataType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_LibCamera_Message_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // if there is no allocator, use the standard new and delete calls.
  ACE_NEW_NORETURN (message_p,
                    OWN_TYPE_T (this->sessionId (),
                                this->length ()));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Stream_CamSave_Message_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF
  int result = message_p->copy (this->rd_ptr (),
                                this->length ());
  ACE_ASSERT (result == 0);

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

template <typename DataType,
          typename SessionDataType>
ACE_Message_Block*
Stream_CamSave_LibCamera_Message_T<DataType,
                                   SessionDataType>::release (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_LibCamera_Message_T::release"));

  // release any continuations first
  if (inherited::cont_)
  {
    inherited::cont_->release (); inherited::cont_ = NULL;
  } // end IF

  int reference_count = inherited::reference_count ();
  if ((reference_count > 1)    || // not the last reference
      inherited::data_.release || // clean up (device data)
      !inherited::data_.buffer)   // regular message
    return inherited::release ();

  ACE_ASSERT (inherited::data_.buffer);
  ACE_ASSERT (inherited::data_.camera);
  ACE_ASSERT (inherited::data_.request);
  ACE_ASSERT (inherited::data_.stream);
//requeue:
  inherited::data_.request->reuse ();

  inherited::data_.request->addBuffer (inherited::data_.stream,
                                       inherited::data_.buffer);

  inherited::data_.camera->queueRequest (inherited::data_.request);

  return NULL;
}
#endif
