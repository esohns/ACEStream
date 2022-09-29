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

#include "stream_defines.h"
#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_Defragment_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                           SessionMessageType>::Stream_Module_Defragment_T (ISTREAM_T* stream_in)
#else
                           SessionMessageType>::Stream_Module_Defragment_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Defragment_T::Stream_Module_Defragment_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Module_Defragment_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::initialize (const ConfigurationType& configuration_in,
                                                            Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Defragment_T::initialize"));

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_Defragment_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Defragment_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (inherited::configuration_->defragmentMode)
  {
    case STREAM_DEFRAGMENT_CLONE:
    {
      DataMessageType* message_p =
        static_cast<DataMessageType*> (message_inout->clone ());
      ACE_ASSERT (message_p);

      message_p->defragment ();

      int result = inherited::put_next (message_p, NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        message_p->release (); message_p = NULL;
        goto error;
      } // end IF

      passMessageDownstream_out = false;
      message_inout->release (); message_inout = NULL;
      break;
    }
    case STREAM_DEFRAGMENT_CONDENSE:
    { // sanity check(s)
      if (!message_inout->cont ())
        return; // nothing to do
      ACE_ASSERT (inherited::configuration_->messageAllocator);
      ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

      DataMessageType* message_p = NULL;
      size_t total_length_i = message_inout->total_length ();
      //total_length_i += total_length_i / 10; // add 10% for good measure
      size_t allocated_bytes_i =
        total_length_i + inherited::configuration_->allocatorConfiguration->paddingBytes;
      try {
        message_p =
          static_cast<DataMessageType*> (inherited::configuration_->messageAllocator->malloc (allocated_bytes_i));
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                    inherited::mod_->name (),
                    allocated_bytes_i));
        message_p = NULL;
      }
      ACE_ASSERT (message_p);
      message_p->size (total_length_i);

      ACE_Message_Block* message_block_p = message_inout;
      int result = -1;
      do
      {
        result = message_p->copy (message_block_p->rd_ptr (),
                                  message_block_p->length ());
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          message_p->release (); message_p = NULL;
          goto error;
        } // end IF
        message_block_p = message_block_p->cont ();
      } while (message_block_p);

      if (message_inout->isInitialized ())
      {
        typename DataMessageType::DATA_T* data_container_p =
          &const_cast<typename DataMessageType::DATA_T&> (message_inout->getR ());
        data_container_p->increase ();
        message_p->initialize (data_container_p,
                               message_inout->sessionId (),
                               NULL);
      } // end IF

      result = inherited::put_next (message_p, NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        message_p->release (); message_p = NULL;
        goto error;
      } // end IF

      passMessageDownstream_out = false;
      message_inout->release (); message_inout = NULL;
      break;
    }
    case STREAM_DEFRAGMENT_DEFRAGMENT:
      message_inout->defragment (); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown mode (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->defragmentMode));
      passMessageDownstream_out = false;
      message_inout->release (); message_inout = NULL;
      goto error;
    }
  } // end SWITCH

  return;

error:
  inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_Defragment_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Defragment_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      break;
    }
    default:
      break;
  } // end SWITCH
}
