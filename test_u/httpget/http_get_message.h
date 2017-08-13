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

#ifndef HTTP_GET_MESSAGE_H
#define HTTP_GET_MESSAGE_H

#include <string>

#include "ace/Global_Macros.h"

#include "common_iget.h"

#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_data_message_base.h"
//#include "stream_messageallocatorheap_base.h"

#include "http_codes.h"
#include "http_common.h"
#include "http_defines.h"
#include "http_tools.h"

// forward declaration(s)
struct HTTP_AllocatorConfiguration;
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class HTTPGet_SessionMessage;
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct HTTPGet_AllocatorConfiguration> HTTPGet_ControlMessage_t;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

//////////////////////////////////////////

class HTTPGet_MessageDataContainer
 : public Stream_DataBase_T<struct HTTPGet_MessageData>
 , public Common_ISetPP_T<struct HTTP_Record>
{
 public:
  HTTPGet_MessageDataContainer ();
  // *IMPORTANT NOTE*: fire-and-forget API
  HTTPGet_MessageDataContainer (struct HTTPGet_MessageData*&, // data handle
                                bool = true);                 // delete in dtor ?
  inline virtual ~HTTPGet_MessageDataContainer () {};

  // implement Common_ISetPP_T
  virtual void set (struct HTTP_Record*&);

 private:
  typedef Stream_DataBase_T<struct HTTPGet_MessageData> inherited;

  ACE_UNIMPLEMENTED_FUNC (HTTPGet_MessageDataContainer (const HTTPGet_MessageDataContainer&))
  ACE_UNIMPLEMENTED_FUNC (HTTPGet_MessageDataContainer& operator= (const HTTPGet_MessageDataContainer&))
};

//////////////////////////////////////////

class HTTPGet_Message
 : public Stream_DataMessageBase_2<struct HTTPGet_AllocatorConfiguration,
                                   enum Stream_MessageType,
                                   HTTPGet_MessageDataContainer,
                                   HTTP_Method_t>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct HTTP_AllocatorConfiguration,
                                                 HTTPGet_ControlMessage_t,
                                                 HTTPGet_Message,
                                                 HTTP_SessionMessage>;

 public:
  HTTPGet_Message (unsigned int); // size
  // *NOTE*: to be used by message allocators
  // *TODO*: --> make this private
  HTTPGet_Message (ACE_Data_Block*, // data block
                   ACE_Allocator*,  // message allocator
                   bool = true);    // increment running message counter ?
  inline virtual ~HTTPGet_Message () {};

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_MessageBase_T
  virtual HTTP_Method_t command () const; // return value: message type

  inline static std::string CommandType2String (HTTP_Method_t method_in) { return (method_in == HTTP_Codes::HTTP_METHOD_INVALID ? ACE_TEXT_ALWAYS_CHAR (HTTP_COMMAND_STRING_RESPONSE)
                                                                                                                                : HTTP_Tools::MethodToString (method_in)); };

 protected:
  // copy ctor to be used by duplicate() and derived classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  HTTPGet_Message (const HTTPGet_Message&);

 private:
  typedef Stream_DataMessageBase_2<struct HTTPGet_AllocatorConfiguration,
                                   enum Stream_MessageType,
                                   HTTPGet_MessageDataContainer,
                                   HTTP_Method_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (HTTPGet_Message ())
  HTTPGet_Message (ACE_Allocator*); // message allocator
  ACE_UNIMPLEMENTED_FUNC (HTTPGet_Message& operator= (const HTTPGet_Message&))
};

#endif
