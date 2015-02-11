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
#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_message_base.h"

template <typename ConfigurationType>
Stream_SessionMessageBase_T<ConfigurationType>::Stream_SessionMessageBase_T(unsigned int sessionID_in,
                                                                            Stream_SessionMessageType& messageType_in,
                                                                            ConfigurationType*& configuration_inout)
 : inherited (0,                                     // size
              Stream_MessageBase::MB_STREAM_SESSION, // type
              NULL,                                  // continuation
              NULL,                                  // data
              NULL,                                  // buffer allocator
              NULL,                                  // locking strategy
              ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY,    // priority
              ACE_Time_Value::zero,                  // execution time
              ACE_Time_Value::max_time,              // deadline time
              NULL,                                  // data block allocator
              NULL)                                  // message block allocator
 , configuration_ (configuration_inout)
 , sessionID_ (sessionID_in)
 , messageType_ (messageType_in)
 , isInitialized_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_TStream_SessionMessageBase_T"));

  // set return value
  config_inout = NULL;
}

template <typename ConfigurationType>
Stream_SessionMessageBase_T<ConfigurationType>::Stream_SessionMessageBase_T(ACE_Allocator* messageAllocator_in)
 : inherited(messageAllocator_in), // message block allocator
   myConfig(NULL),
   myID(0),
   myMessageType(RPG_Stream_SessionMessage::MB_BEGIN_STREAM_SESSION_MAP), // == RPG_Stream_MessageBase::MB_STREAM_SESSION
   myIsInitialized(false)
{
  RPG_TRACE(ACE_TEXT("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // set correct message type
  // *WARNING*: this doesn't work, as we're assigned a (different) data block later...
  // --> do it in init()
//   msg_type(RPG_Stream_MessageBase::MB_STREAM_SESSION);

  // reset read/write pointers
  reset();
}

template <typename ConfigurationType>
Stream_SessionMessageBase_T<ConfigurationType>::Stream_SessionMessageBase_T(ACE_Data_Block* dataBlock_in,
                                                                 ACE_Allocator* messageAllocator_in)
 : inherited(dataBlock_in,         // use (don't own (!) memory of-) this data block
             0,                    // flags --> also "free" our data block upon destruction !
             messageAllocator_in), // re-use the same allocator
   myConfig(NULL),
   myID(0),
   myMessageType(RPG_Stream_SessionMessage::MB_BEGIN_STREAM_SESSION_MAP), // == RPG_Stream_MessageBase::MB_STREAM_SESSION
   myIsInitialized(false)
{
  RPG_TRACE(ACE_TEXT("RPG_Stream_MessageBase::RPG_Stream_MessageBase"));

  // set correct message type
  // *WARNING*: need to finalize initialization through init() !
  msg_type(RPG_Stream_MessageBase::MB_STREAM_SESSION);

  // reset read/write pointers
  reset();
}

template <typename ConfigurationType>
Stream_SessionMessageBase_T<ConfigurationType>::Stream_SessionMessageBase_T(const Stream_SessionMessageBase_T<ConfigurationType>& message_in)
 : inherited(message_in.data_block_->duplicate(),  // make a "shallow" copy of the data block
             0,                                    // "own" the duplicate
             message_in.message_block_allocator_), // message allocator
   myConfig(message_in.myConfig),
   myID(message_in.myID),
   myMessageType(message_in.myMessageType),
   myIsInitialized(message_in.myIsInitialized)
{
  RPG_TRACE(ACE_TEXT("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // increment reference counter
  if (myConfig)
  {
    myConfig->increase();
  } // end IF

  // set read/write pointers
  rd_ptr(message_in.rd_ptr());
  wr_ptr(message_in.wr_ptr());
}

template <typename ConfigurationType>
Stream_SessionMessageBase_T<ConfigurationType>::~Stream_SessionMessageBase_T()
{
  RPG_TRACE(ACE_TEXT("Stream_SessionMessageBase_T::~Stream_SessionMessageBase_T"));

  myID = 0;
  myMessageType = RPG_Stream_SessionMessage::MB_BEGIN_STREAM_SESSION_MAP; // == RPG_Stream_MessageBase::MB_STREAM_SESSION
  // clean up
  if (myConfig)
  {
    // decrease reference counter...
    myConfig->decrease();
    myConfig = NULL;
  } // end IF

  myIsInitialized = false;
}

template <typename ConfigurationType>
const unsigned long
Stream_SessionMessageBase_T<ConfigurationType>::getID() const
{
  RPG_TRACE(ACE_TEXT("Stream_SessionMessageBase_T::getID"));

  return myID;
}

template <typename ConfigurationType>
const RPG_Stream_SessionMessageType
Stream_SessionMessageBase_T<ConfigurationType>::getType() const
{
  RPG_TRACE(ACE_TEXT("Stream_SessionMessageBase_T::getType"));

  return myMessageType;
}

template <typename ConfigurationType>
const ConfigurationType* const
Stream_SessionMessageBase_T<ConfigurationType>::getConfig() const
{
  RPG_TRACE(ACE_TEXT("Stream_SessionMessageBase_T::getConfig"));

  return myConfig;
}

template <typename ConfigurationType>
void
Stream_SessionMessageBase_T<ConfigurationType>::dump_state() const
{
  RPG_TRACE(ACE_TEXT("Stream_SessionMessageBase_T::dump_state"));

  std::string type_string;
  RPG_Stream_SessionMessage::SessionMessageType2String(myMessageType,
                                                       type_string);

  ACE_DEBUG((LM_DEBUG,
             ACE_TEXT("session (ID: %u) message type: \"%s\"\n"),
             myID,
             type_string.c_str()));

  if (myConfig)
  {
    try
    {
      myConfig->dump_state();
    }
    catch (...)
    {
      ACE_DEBUG((LM_ERROR,
                 ACE_TEXT("caught exception in RPG_Stream_SessionConfigBase::dump_state(), continuing")));
    }
  } // end IF
}

template <typename ConfigurationType>
ACE_Message_Block*
Stream_SessionMessageBase_T<ConfigurationType>::duplicate(void) const
{
  RPG_TRACE(ACE_TEXT("Stream_SessionMessageBase_T::duplicate"));

  Stream_SessionMessageBase_T<ConfigurationType>* nb = NULL;

  // create a new <Stream_SessionMessageBase_T> that contains unique copies of
  // the message block fields, but a reference counted duplicate of
  // the <ACE_Data_Block>.

  // if there is no allocator, use the standard new and delete calls.
  if (message_block_allocator_ == NULL)
  {
    ACE_NEW_RETURN(nb,
                   Stream_SessionMessageBase_T<ConfigurationType>(*this),
                   NULL);
  } // end IF

  // *WARNING*:we tell the allocator to return a Stream_SessionMessageBase_T<ConfigurationType>
  // by passing a 0 as argument to malloc()...
  ACE_NEW_MALLOC_RETURN(nb,
                        static_cast<Stream_SessionMessageBase_T<ConfigurationType>*>(message_block_allocator_->malloc(0)),
                        Stream_SessionMessageBase_T<ConfigurationType>(*this),
                        NULL);

  // increment the reference counts of all the continuation messages
  if (cont_)
  {
    nb->cont_ = cont_->duplicate();

    // If things go wrong, release all of our resources and return
    if (nb->cont_ == 0)
    {
      nb->release();
      nb = NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return nb;
}

template <typename ConfigurationType>
void
Stream_SessionMessageBase_T<ConfigurationType>::init(const unsigned long& sessionID_in,
                                            const RPG_Stream_SessionMessageType& messageType_in,
                                            ConfigurationType*& config_inout)
{
  RPG_TRACE(ACE_TEXT("Stream_SessionMessageBase_T::init"));

  // sanity checks
  ACE_ASSERT(!myIsInitialized);
  ACE_ASSERT(myID == 0);
  ACE_ASSERT(myMessageType == RPG_Stream_SessionMessage::MB_BEGIN_STREAM_SESSION_MAP), // == RPG_Stream_MessageBase::MB_STREAM_SESSION
  ACE_ASSERT(myConfig == NULL);

  myID = sessionID_in;
  myMessageType = messageType_in;
  myConfig = config_inout;

  // bye bye... we take on the responsibility for config_inout
  config_inout = NULL;

  // OK !
  myIsInitialized = true;
}
