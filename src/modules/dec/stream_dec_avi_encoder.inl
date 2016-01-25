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

#include "stream_macros.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_ReaderTask_T<TaskSynchType,
                                       TimePolicyType,
                                       SessionDataType>::Stream_Decoder_AVIEncoder_ReaderTask_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::Stream_Decoder_AVIEncoder_ReaderTask_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_ReaderTask_T<TaskSynchType,
                                       TimePolicyType,
                                       SessionDataType>::~Stream_Decoder_AVIEncoder_ReaderTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::~Stream_Decoder_AVIEncoder_ReaderTask_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType>
int
Stream_Decoder_AVIEncoder_ReaderTask_T<TaskSynchType,
                                       TimePolicyType,
                                       SessionDataType>::put (ACE_Message_Block* messageBlock_in,
                                                              ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::put"));

  switch (messageBlock_in->msg_type ())
  {
    case ACE_Message_Block::MB_IOCTL:
    {
      SessionDataType* session_data_p =
          static_cast<SessionDataType*> (messageBlock_in->base ());
      ACE_ASSERT (session_data_p);

      // *TODO*: remove type inference
      if (!postProcessHeader (session_data_p->targetFilename))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Decoder_AVIEncoder_ReaderTask_T::postProcessHeader(\"%s\"), aborting\n"),
                    ACE_TEXT (session_data_p->targetFilename.c_str ())));
        return -1;
      } // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type (was: %d), aborting\n"),
                  messageBlock_in->msg_type ()));
      return -1;
    }
  } // end SWITCH

  return 0;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType>
bool
Stream_Decoder_AVIEncoder_ReaderTask_T<TaskSynchType,
                                       TimePolicyType,
                                       SessionDataType>::postProcessHeader (const std::string& filename_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::postProcessHeader"));

}

////////////////////////////////////////////////////////////////////////////////

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataType>::Stream_Decoder_AVIEncoder_WriterTask_T ()
 : inherited ()
 , configuration_ (NULL)
 , sessionData_ (NULL)
 , isFirst_ (true)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::Stream_Decoder_AVIEncoder_WriterTask_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataType>::~Stream_Decoder_AVIEncoder_WriterTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::~Stream_Decoder_AVIEncoder_WriterTask_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
bool
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::initialize"));

  if (isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    configuration_ = NULL;
    sessionData_ = NULL;
    isFirst_ = true;

    isInitialized_ = false;
  } // end IF

  configuration_ = &configuration_in;
  isInitialized_ = true;

  return isInitialized_;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
const ConfigurationType&
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::get"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (ConfigurationType ());
  ACE_NOTREACHED (return ConfigurationType ();)
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
void
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataType>::handleDataMessage (MessageType*& message_inout,
                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::handleDataMessage"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  // initialize driver ?
  if (isFirst_)
  {
    // *TODO*: remove type inference


    isFirst_ = false;
  } // end IF

  // OK: parse this message

  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
  //              message_p->getID (),
  //              message_p->length ()));
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
void
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  const typename SessionMessageType::SESSION_DATA_T& session_data_container_r =
    message_inout->get ();
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (session_data_container_r.get ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      sessionData_ = &session_data_r;
      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
MessageType*
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::allocateMessage"));

  // initialize return value(s)
  MessageType* message_p = NULL;

  // sanity check(s)
  ACE_ASSERT (configuration_.streamConfiguration);

  if (configuration_.streamConfiguration.messageAllocator)
  {
allocate:
    try
    {
      message_p =
        static_cast<MessageType*> (configuration_.streamConfiguration.messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_p && !configuration_.streamConfiguration.messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      MessageType (requestedSize_in));
  if (!message_p)
  {
    if (configuration_.streamConfiguration.messageAllocator)
    {
      if (configuration_.streamConfiguration.messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate MessageType: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate MessageType: \"%m\", aborting\n")));
  } // end IF

  return message_p;
}
