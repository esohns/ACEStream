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

#ifndef TEST_U_MESSAGE_H
#define TEST_U_MESSAGE_H

#include "ace/Global_Macros.h"

#include "stream_data_message_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "test_u_animated_gif_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class Test_U_SessionMessage;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T;

struct Test_U_MessageData
{
  Test_U_MessageData ()
   : format ()
#if defined (IMAGEMAGICK_SUPPORT)
   , relinquishMemory (NULL)
#endif // IMAGEMAGICK_SUPPORT
  {}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType                                format;
#else
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_VideoMediaType format;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
#if defined (IMAGEMAGICK_SUPPORT)
  // free memory with MagickRelinquishMemory() ?
  void*                                              relinquishMemory;
#endif // IMAGEMAGICK_SUPPORT
};

class Test_U_Message
 : public Stream_DataMessageBase_T<struct Test_U_MessageData,
//                                   struct Stream_AllocatorConfiguration,
                                   enum Stream_MessageType,
                                   Stream_CommandType_t>
{
  typedef Stream_DataMessageBase_T<struct Test_U_MessageData,
//                                   struct Stream_AllocatorConfiguration,
                                   enum Stream_MessageType,
                                   Stream_CommandType_t> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_U_Message,
                                                 Test_U_SessionMessage>;

 public:
  Test_U_Message (Stream_SessionId_t, // session id
                  size_t);            // size
  virtual ~Test_U_Message ();

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_MessageBase_T
  inline virtual Stream_CommandType_t command () const { return ACE_Message_Block::MB_DATA; }
  static std::string CommandTypeToString (Stream_CommandType_t);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_U_Message (const Test_U_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Message ())
  // *NOTE*: to be used by message allocators
  Test_U_Message (Stream_SessionId_t, // session id
                  ACE_Data_Block*,    // data block
                  ACE_Allocator*,     // message allocator
                  bool = true);       // increment running message counter ?
  Test_U_Message (Stream_SessionId_t, // session id
                  ACE_Allocator*);    // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_U_Message& operator= (const Test_U_Message&))
};

#endif // TEST_U_MESSAGE_H
