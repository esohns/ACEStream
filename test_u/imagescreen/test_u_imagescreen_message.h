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

#ifndef TEST_U_IMAGESCREEN_MESSAGE_H
#define TEST_U_IMAGESCREEN_MESSAGE_H

#include "ace/Global_Macros.h"
#include "ace/Malloc_Base.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_data_message_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

// forward declaration(s)
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T;
template <typename DataMessageType,
          typename SessionDataType>
class Stream_ImageScreen_SessionMessage_T;

struct Stream_ImageScreen_MessageData
{
  Stream_ImageScreen_MessageData ()
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

template <typename SessionDataType> // derives off Stream_SessionData_T
class Stream_ImageScreen_Message_T
 : public Stream_DataMessageBase_T<struct Stream_ImageScreen_MessageData,
//                                   struct Stream_AllocatorConfiguration,
                                   enum Stream_MessageType,
                                   int>
{
  typedef Stream_DataMessageBase_T<struct Stream_ImageScreen_MessageData,
//                                   struct Stream_AllocatorConfiguration,
                                   enum Stream_MessageType,
                                   int> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Stream_ImageScreen_Message_T<SessionDataType>,
                                                 Stream_ImageScreen_SessionMessage_T<Stream_ImageScreen_Message_T<SessionDataType>,
                                                                                     SessionDataType> >;

 public:
  Stream_ImageScreen_Message_T (unsigned int); // size
  virtual ~Stream_ImageScreen_Message_T ();

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy that references the same packet
  // *NOTE*: uses the allocator (if any)
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_MessageBase_T
  inline virtual int command () const { return ACE_Message_Block::MB_DATA; }

 protected:
  // convenient types
  typedef Stream_ImageScreen_Message_T<SessionDataType> OWN_TYPE_T;

  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Stream_ImageScreen_Message_T (const OWN_TYPE_T&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_ImageScreen_Message_T ())
  // *NOTE*: to be used by message allocators
  Stream_ImageScreen_Message_T (Stream_SessionId_t,
                                ACE_Data_Block*, // data block to use
                                ACE_Allocator*,  // message allocator
                                bool = true);    // increment running message counter ?
  Stream_ImageScreen_Message_T (Stream_SessionId_t,
                                ACE_Allocator*); // message allocator
  ACE_UNIMPLEMENTED_FUNC (Stream_ImageScreen_Message_T& operator= (const Stream_ImageScreen_Message_T&))
};

// include template definition
#include "test_u_imagescreen_message.inl"

#endif
