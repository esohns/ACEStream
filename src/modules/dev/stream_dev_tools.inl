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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/Message_Block.h"

#include "libv4l2.h"
#include "linux/videodev2.h"

#include "stream_iallocator.h"

template <typename MessageType>
bool
Stream_Module_Device_Tools::initializeBuffers (int fd_in,
                                               v4l2_memory method_in,
                                               __u32 numberOfBuffers_in,
                                               INDEX2BUFFER_MAP_T& bufferMap_out,
                                               Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::initializeBuffers"));

  // sanity check(s)
  ACE_ASSERT (bufferMap_out.empty ());

  int result = -1;
  struct v4l2_buffer buffer;
  ACE_OS::memset (&buffer, 0, sizeof (struct v4l2_buffer));
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ACE_Message_Block* message_block_p = NULL;
  MessageType* message_p = NULL;

  // step1: retrieve required buffer size (format-dependent)
  struct v4l2_format format;
  ACE_OS::memset (&format, 0, sizeof (struct v4l2_format));
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  result = v4l2_ioctl (fd_in,
                       VIDIOC_G_FMT,
                       &format);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_G_FMT")));
    return false;
  } // end IF
  ACE_ASSERT (format.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);

  switch (method_in)
  {
    case V4L2_MEMORY_USERPTR:
    {
      buffer.memory = V4L2_MEMORY_USERPTR;

      for (unsigned int i = 0;
           i < numberOfBuffers_in;
           ++i)
      {
        buffer.index = i;
        if (allocator_in)
        {
          try
          {
            message_block_p =
                static_cast<ACE_Message_Block*> (allocator_in->malloc (format.fmt.pix.sizeimage));
          }
          catch (...)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                        format.fmt.pix.sizeimage));
            message_block_p = NULL;
          }
        } // end IF
        else
          ACE_NEW_NORETURN (message_block_p,
                            ACE_Message_Block (format.fmt.pix.sizeimage));
        if (!message_block_p)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("failed to allocate memory, aborting\n")));
          goto error;
        } // end IF
        message_p = dynamic_cast<MessageType*> (message_block_p);
        if (!message_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<MessageType*>(0x%@), aborting\n"),
                      message_block_p));

          // clean up
          message_block_p->release ();

          goto error;
        } // end IF
        message_p->wr_ptr (format.fmt.pix.sizeimage);
        // *TODO*: remove type inference
        typename MessageType::DATA_T& data_r =
            const_cast<typename MessageType::DATA_T&> (message_p->get ());
        data_r.device = fd_in;
        data_r.index = i;
        data_r.method = method_in;

        buffer.m.userptr =
            reinterpret_cast<unsigned long> (message_block_p->rd_ptr ());
        buffer.length = format.fmt.pix.sizeimage;
        // *NOTE*: in oder to retrieve the buffer instance handle from the
        //         device buffer when it has written the frame data, the address
        //         of the ACE_Message_Block (!) could be embedded in the
        //         'reserved' field(s). Unfortunately, this does not work, the
        //         fields seem to be zeroed by the driver
        //         --> maintain a mapping: buffer index <--> buffer handle
//        buffer.reserved = reinterpret_cast<unsigned long> (message_block_p);
        result = v4l2_ioctl (fd_in,
                             VIDIOC_QBUF,
                             &buffer);
        if (result == -1)
        {
          int error = ACE_OS::last_error ();
          if (error != EINVAL) // 22
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                        fd_in, ACE_TEXT ("VIDIOC_QBUF")));
            goto error;
          } // end IF
          goto no_support;
        } //IF end
        bufferMap_out.insert (std::make_pair (buffer.index, message_block_p));
      } // end FOR
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("allocated %d (user-pointer) device buffers (%u byte(s))...\n"),
                  numberOfBuffers_in,
                  numberOfBuffers_in * format.fmt.pix.sizeimage));
      break;
    }
    case V4L2_MEMORY_MMAP:
    {
      buffer.memory = V4L2_MEMORY_MMAP;

      void* mmap_p = NULL;
      for (unsigned int i = 0;
           i < numberOfBuffers_in;
           ++i)
      {
        buffer.index = i;
        result = v4l2_ioctl (fd_in,
                             VIDIOC_QUERYBUF,
                             &buffer);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                      fd_in, ACE_TEXT ("VIDIOC_QUERYBUF")));
          goto error;
        } // end IF

        if (allocator_in)
        {
          try
          {
            message_block_p =
                static_cast<ACE_Message_Block*> (allocator_in->malloc (buffer.length));
          }
          catch (...)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                        buffer.length));
            message_block_p = NULL;
          }
        } // end IF
        else
          ACE_NEW_NORETURN (message_block_p,
                            ACE_Message_Block (buffer.length));
        if (!message_block_p)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("failed to allocate memory, aborting\n")));
          goto error;
        } // end IF
        message_p = dynamic_cast<MessageType*> (message_block_p);
        if (!message_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<MessageType*>(0x%@), aborting\n"),
                      message_block_p));

          // clean up
          message_block_p->release ();

          goto error;
        } // end IF
        // *TODO*: remove type inference
        typename MessageType::DATA_T& data_r =
            const_cast<typename MessageType::DATA_T&> (message_p->get ());
        data_r.device = fd_in;
        data_r.index = i;
        data_r.method = method_in;

        mmap_p = v4l2_mmap (NULL,
                            buffer.length,
                            PROT_READ | PROT_WRITE,
                            //PROT_READ,
                            MAP_SHARED,
                            fd_in,
                            buffer.m.offset);
        if (mmap_p == MAP_FAILED)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to v4l2_mmap(): \"%m\", aborting\n")));

          // clean up
          message_block_p->release ();

          goto error;
        } // end IF
        message_block_p->base (static_cast<char*> (mmap_p),
                               buffer.length,
                               ACE_Message_Block::DONT_DELETE);
        message_block_p->wr_ptr (format.fmt.pix.sizeimage);

        // *NOTE*: in oder to retrieve the buffer instance handle from the
        //         device buffer when it has written the frame data, the address
        //         of the ACE_Message_Block (!) could be embedded in the
        //         'reserved' field(s). Unfortunately, this does not work, the
        //         fields seem to be zeroed by the driver
        //         --> maintain a mapping: buffer index <--> buffer handle
//        buffer.reserved = reinterpret_cast<unsigned long> (message_block_p);
        result = v4l2_ioctl (fd_in,
                             VIDIOC_QBUF,
                             &buffer);
        if (result == -1)
        {
          int error = ACE_OS::last_error ();
          if (error != EINVAL) // 22
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                        fd_in, ACE_TEXT ("VIDIOC_QBUF")));

            // clean up
            result = v4l2_munmap (message_block_p->base (),
                                  message_block_p->length ());
            if (result == -1)
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to v4l2_munmap(): \"%m\", continuing\n")));
            message_block_p->release ();

            goto error;
          } // end IF

          // clean up
          result = v4l2_munmap (message_block_p->base (),
                                message_block_p->length ());
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to v4l2_munmap(): \"%m\", continuing\n")));
          message_block_p->release ();

          goto no_support;
        } //IF end
        bufferMap_out.insert (std::make_pair (buffer.index, message_block_p));
      } // end FOR
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("allocated %d (memory-mapped) device buffers (%u byte(s))...\n"),
                  numberOfBuffers_in,
                  numberOfBuffers_in * buffer.length));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown streaming method (was: %d), aborting\n"),
                  method_in));
      return false;
    }
  } // end SWITCH

  return true;

no_support:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("device (was: %d) does not support streaming method (was: %d), aborting\n"),
              fd_in, method_in));
error:
  return false;
}

template <typename MessageType>
void
Stream_Module_Device_Tools::finalizeBuffers (int fd_in,
                                             v4l2_memory method_in,
                                             INDEX2BUFFER_MAP_T& bufferMap_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::finalizeBuffers"));

  int result = -1;

  // step1: dequeue all buffers and unmap/release associated memory
  struct v4l2_buffer buffer;
  ACE_OS::memset (&buffer, 0, sizeof (struct v4l2_buffer));
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer.memory = method_in;
  ACE_Message_Block* message_block_p = NULL;
  MessageType* message_p = NULL;
  unsigned int counter = 0;
  INDEX2BUFFER_MAP_ITERATOR_T iterator;
  do
  {
    result = v4l2_ioctl (fd_in,
                         VIDIOC_DQBUF,
                         &buffer);
    if (result == -1)
    {
      // *NOTE*: most probable reason: VIDIOC_STREAMOFF has been called
      //         previously (EINVAL), dequeueing all buffers; the ioctl fails to
      //         retrieve buffer information in this case
      //         --> set the index manually
      int error = ACE_OS::last_error ();
      if (error != EINVAL)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                    fd_in, ACE_TEXT ("VIDIOC_DQBUF")));
      buffer.index = counter;
    } // end IF

//    message_block_p = reinterpret_cast<ACE_Message_Block*> (buffer.reserved);
    iterator = bufferMap_inout.find (buffer.index);
    ACE_ASSERT (iterator != bufferMap_inout.end ());
    message_block_p = (*iterator).second;
    ACE_ASSERT (message_block_p);

    if (buffer.type == V4L2_MEMORY_MMAP)
    {
      result = v4l2_munmap (message_block_p->base (),
                            message_block_p->size ());
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to v4l2_munmap(): \"%m\", continuing\n")));
    } // end IF
    message_p = dynamic_cast<MessageType*> (message_block_p);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<MessageType*>(0x%@), returning\n"),
                  message_block_p));

      // clean up
      message_block_p->release ();

      return;
    } // end IF
    // *TODO*: remove type inference
    typename MessageType::DATA_T& data_r =
        const_cast<typename MessageType::DATA_T&> (message_p->get ());
    data_r.release = true;
    message_block_p->release ();

    // *NOTE*: unfortunately, V4L2_BUF_FLAG_LAST is not set consistently
    if (//(buffer.flags & V4L2_BUF_FLAG_LAST) ||
        (buffer.index == (bufferMap_inout.size () - 1)))
      break; // done

    ++counter;
  } while (true);
  ++counter;
  ACE_ASSERT (counter == bufferMap_inout.size ());
  bufferMap_inout.clear ();
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("de-allocated %d device buffers...\n"),
              counter));
}
#endif