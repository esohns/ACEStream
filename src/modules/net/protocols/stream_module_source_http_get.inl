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

#include "stream_iallocator.h"
#include "stream_macros.h"

template <typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Module_Net_Source_HTTP_Get_T<SessionMessageType,
                                    ProtocolMessageType>::Stream_Module_Net_Source_HTTP_Get_T ()
 : inherited ()
 , allocator_ (NULL)
 , isInitialized_ (false)
 , URL_ (ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_NET_SOURCE_HTTP_GET_DEFAULT_URL))
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::Stream_Module_Net_Source_HTTP_Get_T"));

}

template <typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Module_Net_Source_HTTP_Get_T<SessionMessageType,
                                    ProtocolMessageType>::~Stream_Module_Net_Source_HTTP_Get_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::~Stream_Module_Net_Source_HTTP_Get_T"));

}

template <typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Module_Net_Source_HTTP_Get_T<SessionMessageType,
                                    ProtocolMessageType>::initialize (Stream_IAllocator* allocator_in,
                                                                      const std::string& URL_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::initialize"));

  allocator_ = allocator_in;
  if (!URL_in.empty ())
    URL_ = URL_in;
  // *TODO*: validate URL

  isInitialized_ = true;

  return true;
}

//template <typename SessionMessageType,
//          typename ProtocolMessageType>
//void
//Stream_Module_Net_Source_HTTP_Get_T<SessionMessageType,
//                                    ProtocolMessageType>::handleDataMessage (ProtocolMessageType*& message_inout,
//                                                             bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::handleDataMessage"));
//
//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);
//}

template <typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Module_Net_Source_HTTP_Get_T<SessionMessageType,
                                    ProtocolMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);
  ACE_ASSERT (inherited::mod_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // step1: allocate message buffer
      // *TODO*: estimate a reasonable buffer size
      ProtocolMessageType* message_p = allocateMessage (BUFSIZ);
      if (!message_p)
      {
        ACE_ASSERT (inherited::mod_);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to allocate message buffer, returning\n"),
                    inherited::mod_->name ()));
        return;
      } // end IF

      // step2: initialize message buffer
      unsigned int offset =
          ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (HTTP_COMMAND_GET_STRING));
      ACE_OS::memcpy (message_p->wr_ptr (),
                      ACE_TEXT_ALWAYS_CHAR (HTTP_COMMAND_GET_STRING),
                      offset);
      ACE_OS::memcpy (message_p->wr_ptr () + offset,
                      ACE_TEXT_ALWAYS_CHAR (" "),
                      1);
      offset += 1;
//      ACE_ASSERT (message_p->space >= URL_.size ());
      ACE_OS::memcpy (message_p->wr_ptr () + offset,
                      URL_.c_str (),
                      URL_.size ());
      offset += URL_.size ();
      ACE_OS::memcpy (message_p->wr_ptr () + offset,
                      ACE_TEXT_ALWAYS_CHAR (" "),
                      1);
      offset += 1;
      unsigned int length =
          ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (HTTP_VERSION_STRING));
      ACE_OS::memcpy (message_p->wr_ptr () + offset,
                      ACE_TEXT_ALWAYS_CHAR (HTTP_VERSION_STRING),
                      length);
      offset += length;
      ACE_OS::memcpy (message_p->wr_ptr () + offset,
                      ACE_TEXT_ALWAYS_CHAR ("\r\n"), // CRLF
                      2);
      offset += 2;
      ACE_OS::memcpy (message_p->wr_ptr () + offset,
                      ACE_TEXT_ALWAYS_CHAR ("\r\n"), // CRLF
                      2);
      offset += 2;
      message_p->wr_ptr (offset);

      // step3: send message
      result = inherited::reply (message_p);
      if (result == -1)
      {
        ACE_ASSERT (inherited::mod_);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::reply(): \"%m\", returning\n"),
                    inherited::mod_->name ()));

        // clean up
        message_p->release ();

        return;
      } // end IF
      ACE_ASSERT (inherited::mod_);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: dispatched HTTP Get (URL was: \"%s\")\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (URL_.c_str ())));

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename ProtocolMessageType>
ProtocolMessageType*
Stream_Module_Net_Source_HTTP_Get_T<SessionMessageType,
                                    ProtocolMessageType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::allocateMessage"));

  // initialize return value(s)
  ProtocolMessageType* message_out = NULL;

  if (allocator_)
  {
    try
    {
      // *TODO*: remove type inference
      message_out =
          static_cast<ProtocolMessageType*> (allocator_->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  requestedSize_in));
      message_out = NULL;
    }
  } // end IF
  else
  {
    ACE_NEW_NORETURN (message_out,
                      ProtocolMessageType (requestedSize_in));
  } // end ELSE
  if (!message_out)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to Stream_IAllocator::malloc(%u), aborting\n"),
                requestedSize_in));
  } // end IF

  return message_out;
}
