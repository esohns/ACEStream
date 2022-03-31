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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "linux/videodev2.h"

#include "libv4l2.h"

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

#include "common_configuration.h"

#include "stream_iallocator.h"
#include "stream_macros.h"

template <typename MessageType>
bool
Stream_Device_Tools::initializeBuffers (int fd_in,
                                        v4l2_memory method_in,
                                        __u32 numberOfBuffers_in,
                                        Stream_Device_BufferMap_t& bufferMap_out,
                                        Stream_IAllocator* allocator_in,
                                        struct Common_AllocatorConfiguration* allocatorConfiguration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::initializeBuffers"));

  // initialize return values(s)
  bufferMap_out.clear ();

  // sanity check(s)
  ACE_ASSERT (allocator_in);
  ACE_ASSERT (allocatorConfiguration_in);

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
  unsigned int buffer_size_i =
      format.fmt.pix.sizeimage + allocatorConfiguration_in->paddingBytes;

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
          try {
            message_block_p =
                static_cast<ACE_Message_Block*> (allocator_in->malloc (buffer_size_i));
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                        buffer_size_i));
            message_block_p = NULL;
          }
        } // end IF
        else
          ACE_NEW_NORETURN (message_block_p,
                            MessageType (0,                          // session id
                                         format.fmt.pix.sizeimage)); // size
        if (!message_block_p)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("failed to allocate memory, aborting\n")));
          goto error;
        } // end IF
        message_block_p->size (format.fmt.pix.sizeimage);
        message_p = static_cast<MessageType*> (message_block_p);
        message_p->wr_ptr (format.fmt.pix.sizeimage);
        // *TODO*: remove type inference
        typename MessageType::DATA_T& data_r =
            const_cast<typename MessageType::DATA_T&> (message_p->getR ());
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
        ACE_ASSERT ((buffer.flags & V4L2_BUF_FLAG_QUEUED) && !(buffer.flags & V4L2_BUF_FLAG_MAPPED) && !(buffer.flags & V4L2_BUF_FLAG_DONE));
        bufferMap_out.insert (std::make_pair (buffer.index, message_block_p));
      } // end FOR
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("allocated %d (user-pointer) device buffer(s) (%u byte(s))\n"),
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
        ACE_ASSERT (buffer.length >= format.fmt.pix.sizeimage);

        if (allocator_in)
        {
          try {
            message_block_p =
                static_cast<ACE_Message_Block*> (allocator_in->malloc (buffer_size_i));
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                        buffer_size_i));
            message_block_p = NULL;
          }
        } // end IF
        else
          ACE_NEW_NORETURN (message_block_p,
                            MessageType (0,               // session id
                                         buffer_size_i)); // size
        if (!message_block_p)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("failed to allocate memory, aborting\n")));
          goto error;
        } // end IF
        message_block_p->size (format.fmt.pix.sizeimage);
        message_p = static_cast<MessageType*> (message_block_p);
        // *TODO*: remove type inference
        typename MessageType::DATA_T& data_r =
            const_cast<typename MessageType::DATA_T&> (message_p->getR ());
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
          message_block_p->release (); message_block_p = NULL;
          goto error;
        } // end IF
        message_block_p->base (static_cast<char*> (mmap_p),
                               buffer.length,
                               ACE_Message_Block::DONT_DELETE);
        message_block_p->wr_ptr (format.fmt.pix.sizeimage);

        // *NOTE*: in oder to retrieve the buffer instance handle from the
        //         device buffer when it has written the frame data, the address
        //         of the ACE_Message_Block (!) could be embedded in the
        //         'reserved' field(s). Unfortunately, this does not work, these
        //         fields are 0-ed by the driver
        //         --> maintain a mapping: buffer index <--> buffer handle
        result = v4l2_ioctl (fd_in,
                             VIDIOC_QBUF,
                             &buffer);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                      fd_in, ACE_TEXT ("VIDIOC_QBUF")));
          result = v4l2_munmap (message_block_p->base (),
                                buffer.length);
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to v4l2_munmap(): \"%m\", continuing\n")));
          message_block_p->release (); message_block_p = NULL;
          goto error;
        } // end IF
        ACE_ASSERT ((buffer.flags & V4L2_BUF_FLAG_QUEUED) && (buffer.flags & V4L2_BUF_FLAG_MAPPED) && !(buffer.flags & V4L2_BUF_FLAG_DONE));
        bufferMap_out.insert (std::make_pair (buffer.index, message_block_p));
      } // end FOR
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("allocated %d (memory-mapped) device buffer(s) (%u byte(s))\n"),
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
Stream_Device_Tools::finalizeBuffers (int fd_in,
                                      v4l2_memory method_in,
                                      Stream_Device_BufferMap_t& bufferMap_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::finalizeBuffers"));

  int result = -1;

  // dequeue all buffers and unmap/release associated (device) memory
  struct v4l2_buffer buffer;
  ACE_Message_Block* message_block_p = NULL;
  MessageType* message_p = NULL;
  unsigned int counter = 0;
  Stream_Device_BufferMapIterator_t iterator;
  do
  {
    ACE_OS::memset (&buffer, 0, sizeof (struct v4l2_buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = method_in;
    result = v4l2_ioctl (fd_in,
                         VIDIOC_DQBUF,
                         &buffer);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", returning\n"),
                  fd_in, ACE_TEXT ("VIDIOC_DQBUF")));
      return;
    } // end IF

    iterator = bufferMap_inout.find (buffer.index);
    ACE_ASSERT (iterator != bufferMap_inout.end ());
    message_block_p = (*iterator).second;
    ACE_ASSERT (message_block_p);

    if (method_in == V4L2_MEMORY_MMAP)
    {
      result = v4l2_munmap (message_block_p->base (),
                            buffer.length);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to v4l2_munmap(): \"%m\", continuing\n")));
    } // end IF
    message_p = static_cast<MessageType*> (message_block_p);
    // *TODO*: remove type inference
    typename MessageType::DATA_T& data_r =
        const_cast<typename MessageType::DATA_T&> (message_p->getR ());
    data_r.release = true;
    message_block_p->release (); message_block_p = NULL;

    ++counter;

    // *NOTE*: V4L2_BUF_FLAG_LAST is not set consistently
    if (//(buffer.flags & V4L2_BUF_FLAG_LAST) ||
        counter == bufferMap_inout.size ())
      break; // done
  } while (true);
  bufferMap_inout.clear ();
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("de-allocated %d device buffer(s)\n"),
              counter));
}
#endif // ACE_WIN32 || ACE_WIN64
