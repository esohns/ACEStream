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

#ifndef TEST_I_MESSAGE_H
#define TEST_I_MESSAGE_H

#include "ace/Global_Macros.h"

#include "stream_control_message.h"
#include "stream_data_message_base.h"

#include "stream_dec_common.h"

#include "test_i_common.h"

#include "test_i_stream_common.h"

// forward declaration(s)
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_SpeechCommand_DirectShow_MessageData
 : Test_I_DirectShow_MessageData
{
  Test_I_SpeechCommand_DirectShow_MessageData ()
   : Test_I_DirectShow_MessageData ()
   , index (-1)
   , words ()
  {}

  // WaveIn
  unsigned int                index;

  Stream_Decoder_STT_Result_t words;
};
//typedef Stream_DataBase_T<struct Test_I_SpeechCommand_DirectShow_MessageData> Test_I_SpeechCommand_DirectShow_MessageData_t;

struct Test_I_SpeechCommand_MediaFoundation_MessageData
 : Test_I_MediaFoundation_MessageData
{
  Test_I_SpeechCommand_MediaFoundation_MessageData ()
   : Test_I_MediaFoundation_MessageData ()
   , index (-1)
   , words ()
  {}

  // WaveIn
  unsigned int                index;

  Stream_Decoder_STT_Result_t words;
};
//typedef Stream_DataBase_T<struct Test_I_SpeechCommand_MediaFoundation_MessageData> Test_I_SpeechCommand_MediaFoundation_MessageData_t;
#else
struct Test_I_SpeechCommand_ALSA_MessageData
 : Test_I_ALSA_MessageData
{
  Test_I_SpeechCommand_ALSA_MessageData ()
   : Test_I_ALSA_MessageData ()
   , words ()
  {}

  Stream_Decoder_STT_Result_t words;
};
//typedef Stream_DataBase_T<struct Test_I_SpeechCommand_ALSA_MessageData> Test_I_SpeechCommand_ALSA_MessageData_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_DirectShow_Message
 : public Stream_DataMessageBase_T<struct Test_I_SpeechCommand_DirectShow_MessageData,
                                   enum Stream_MessageType,
                                   int>
{
  typedef Stream_DataMessageBase_T<struct Test_I_SpeechCommand_DirectShow_MessageData,
                                   enum Stream_MessageType,
                                   int> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Stream_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_DirectShow_Message,
                                                 Test_I_DirectShow_SessionMessage_t>;

 public:
  Test_I_DirectShow_Message (Stream_SessionId_t, // session id
                             unsigned int);      // size
  inline virtual ~Test_I_DirectShow_Message () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_MessageBase_T
  //virtual int command () const; // return value: message type
  //inline static std::string CommandTypeToString (HTTP_Method_t method_in) { return (method_in == HTTP_Codes::HTTP_METHOD_INVALID ? ACE_TEXT_ALWAYS_CHAR (HTTP_COMMAND_STRING_RESPONSE) : HTTP_Tools::MethodToString (method_in)); }

  // implement (part of) Stream_IMediaType
  inline virtual enum Stream_MediaType_Type getMediaType () const { return STREAM_MEDIATYPE_AUDIO; }

 protected:
  // copy ctor to be used by duplicate() and derived classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_DirectShow_Message (const Test_I_DirectShow_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_DirectShow_Message ())
  // *NOTE*: to be used by message allocators
  Test_I_DirectShow_Message (Stream_SessionId_t, // session id
                             ACE_Data_Block*,    // data block to use
                             ACE_Allocator*,     // message allocator
                             bool = true);       // increment running message counter ?
  Test_I_DirectShow_Message (Stream_SessionId_t, // session id
                             ACE_Allocator*);    // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_I_DirectShow_Message& operator= (const Test_I_DirectShow_Message&))
};

class Test_I_MediaFoundation_Message
 : public Stream_DataMessageBase_T<struct Test_I_SpeechCommand_MediaFoundation_MessageData,
                                   enum Stream_MessageType,
                                   int>
{
  typedef Stream_DataMessageBase_T<struct Test_I_SpeechCommand_MediaFoundation_MessageData,
                                   enum Stream_MessageType,
                                   int> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Stream_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_MediaFoundation_Message,
                                                 Test_I_MediaFoundation_SessionMessage_t>;

 public:
  Test_I_MediaFoundation_Message (Stream_SessionId_t, // session id
                                  unsigned int);      // size
  inline virtual ~Test_I_MediaFoundation_Message () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_MessageBase_T
  //virtual int command () const; // return value: message type
  //inline static std::string CommandTypeToString (HTTP_Method_t method_in) { return (method_in == HTTP_Codes::HTTP_METHOD_INVALID ? ACE_TEXT_ALWAYS_CHAR (HTTP_COMMAND_STRING_RESPONSE) : HTTP_Tools::MethodToString (method_in)); }

  // implement (part of) Stream_IMediaType
  inline virtual enum Stream_MediaType_Type getMediaType () const { return STREAM_MEDIATYPE_AUDIO; }

 protected:
  // copy ctor to be used by duplicate() and derived classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_MediaFoundation_Message (const Test_I_MediaFoundation_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_MediaFoundation_Message ())
  // *NOTE*: to be used by message allocators
  Test_I_MediaFoundation_Message (Stream_SessionId_t, // session id
                                  ACE_Data_Block*,    // data block to use
                                  ACE_Allocator*,     // message allocator
                                  bool = true);       // increment running message counter ?
  Test_I_MediaFoundation_Message (Stream_SessionId_t, // session id
                                  ACE_Allocator*);    // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_I_MediaFoundation_Message& operator= (const Test_I_MediaFoundation_Message&))
};
#else
class Test_I_Message
 : public Stream_DataMessageBase_T<struct Test_I_SpeechCommand_ALSA_MessageData,
                                   enum Stream_MessageType,
                                   int>
{
  typedef Stream_DataMessageBase_T<struct Test_I_SpeechCommand_ALSA_MessageData,
                                   enum Stream_MessageType,
                                   int> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Stream_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_Message,
                                                 Test_I_ALSA_SessionMessage_t>;

 public:
  Test_I_Message (Stream_SessionId_t, // session id
                  unsigned int);      // size
  inline virtual ~Test_I_Message () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_MessageBase_T
  //virtual int command () const; // return value: message type
  //inline static std::string CommandTypeToString (HTTP_Method_t method_in) { return (method_in == HTTP_Codes::HTTP_METHOD_INVALID ? ACE_TEXT_ALWAYS_CHAR (HTTP_COMMAND_STRING_RESPONSE) : HTTP_Tools::MethodToString (method_in)); }

  // implement (part of) Stream_IMediaType
  inline virtual enum Stream_MediaType_Type getMediaType () const { return STREAM_MEDIATYPE_AUDIO; }

 protected:
  // copy ctor to be used by duplicate() and derived classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_Message (const Test_I_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Message ())
  // *NOTE*: to be used by message allocators
  Test_I_Message (Stream_SessionId_t, // session id
                  ACE_Data_Block*,    // data block to use
                  ACE_Allocator*,     // message allocator
                  bool = true);       // increment running message counter ?
  Test_I_Message (Stream_SessionId_t, // session id
                  ACE_Allocator*);    // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_I_Message& operator= (const Test_I_Message&))
};
#endif // ACE_WIN32 || ACE_WIN64

#endif
