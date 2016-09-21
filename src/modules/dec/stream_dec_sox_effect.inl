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

#include "common_file_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_SoXEffect_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType>::Stream_Decoder_SoXEffect_T ()
 : inherited ()
 , buffer_ (NULL)
 , chain_ (NULL)
 , encodingInfo_ ()
 , input_ (NULL)
 , manageSoX_ (true)
 , output_ (NULL)
 , signalInfo_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SoXEffect_T::Stream_Decoder_SoXEffect_T"));

  ACE_OS::memset (&encodingInfo_, 0, sizeof (struct sox_encodinginfo_t));
  ACE_OS::memset (&signalInfo_, 0, sizeof (struct sox_signalinfo_t));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_SoXEffect_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType>::~Stream_Decoder_SoXEffect_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SoXEffect_T::~Stream_Decoder_SoXEffect_T"));

  int result = -1;

  if (buffer_)
    buffer_->release ();

  if (chain_)
    sox_delete_effects_chain (chain_);

  if (manageSoX_)
  {
    result = sox_quit ();
    if (result != SOX_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_quit(): \"%s\", continuing\n"),
                  ACE_TEXT (sox_strerror (result))));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
bool
Stream_Decoder_SoXEffect_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SoXEffect_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    if (buffer_)
    {
      buffer_->release ();
      buffer_ = NULL;
    } // end IF

    if (chain_)
    {
      sox_delete_effects_chain (chain_);
      chain_ = NULL;
    } // end IF
    input_ = NULL;
    output_ = NULL;

    if (manageSoX_)
    {
      result = sox_quit ();
      if (result != SOX_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_quit(): \"%s\", aborting\n"),
                    ACE_TEXT (sox_strerror (result))));
        return false;
      } // end IF
    } // end IF
    manageSoX_ = true;

    inherited::isInitialized_ = false;
  } // end IF

  manageSoX_ = configuration_in.manageSoX;
  if (manageSoX_)
  {
    result = sox_init ();
    if (result != SOX_SUCCESS)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_init(): \"%s\", aborting\n"),
                  ACE_TEXT (sox_strerror (result))));
      return false;
    } // end IF
  } // end IF

  // set sane SoX global default values
  struct sox_globals_t* sox_globals_p = sox_get_globals ();
  ACE_ASSERT (sox_globals_p);
  sox_globals_p->bufsiz = sox_globals_p->input_bufsiz =
      STREAM_DECODER_SOX_BUFFER_SIZE;

  return inherited::initialize (configuration_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
void
Stream_Decoder_SoXEffect_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SoXEffect_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Message_Block* message_block_2 = NULL;
  char* effect_options[1];
  struct sox_format_t* input_buffer_p, *output_buffer_p;

  input_buffer_p =
      sox_open_mem_read (message_inout->rd_ptr (),
                         message_inout->length (),
                         &signalInfo_,
                         &encodingInfo_,
                         ACE_TEXT_ALWAYS_CHAR (STREAM_DECODER_SOX_FORMAT_RAW_STRING));
  if (!input_buffer_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_open_mem_read(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  effect_options[0] = reinterpret_cast<char*> (input_buffer_p);
  result = sox_effect_options (input_, 1, effect_options);
  if (result != SOX_SUCCESS)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_effect_options(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (sox_strerror (result))));
    goto error;
  } // end IF

  if (!buffer_)
  {
    buffer_ =
        allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
    if (!buffer_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%d) failed: \"%m\", returning\n"),
                  inherited::configuration_->streamConfiguration->bufferSize));
      goto error;
    } // end IF
  } // end IF
  message_block_p = buffer_;
  output_buffer_p =
      sox_open_mem_write (message_block_p->wr_ptr (),
                          inherited::configuration_->streamConfiguration->bufferSize,
                          &signalInfo_,
                          &encodingInfo_,
                          ACE_TEXT_ALWAYS_CHAR (STREAM_DECODER_SOX_FORMAT_RAW_STRING),
                          NULL);
  if (!output_buffer_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sox_open_mem_write(\"%s\"): \"%m\", aborting\n")));
    goto error;
  } // end IF
  effect_options[0] = reinterpret_cast<char*> (output_buffer_p);
  result = sox_effect_options (output_, 1, effect_options);
  if (result != SOX_SUCCESS)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_effect_options(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (sox_strerror (result))));
    goto error;
  } // end IF

  do
  {
    result = sox_flow_effects (chain_,
                               NULL,
                               NULL);
    if (result == SOX_SUCCESS)
      break; // done

    if (result != SOX_EOF)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_flow_effects(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
      goto error;
    } // end IF

    // output buffer is full --> (dispatch and- ?) allocate another one
//    ACE_ASSERT (output_buffer_p->tell_off <= inherited::configuration_->streamConfiguration->bufferSize);
    message_block_p->wr_ptr ((output_buffer_p->tell_off <= inherited::configuration_->streamConfiguration->bufferSize) ? output_buffer_p->tell_off
                                                                                                                       : inherited::configuration_->streamConfiguration->bufferSize);

    message_block_2 = NULL;
    message_block_2 =
        allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
    if (!message_block_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%d) failed: \"%m\", returning\n"),
                  inherited::configuration_->streamConfiguration->bufferSize));
      goto error;
    } // end IF
    message_block_p->cont (message_block_2);
    message_block_p = message_block_2;

    result = sox_close (output_buffer_p);
    if (result != SOX_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_close(): \"%s\", continuing\n"),
                  ACE_TEXT (sox_strerror (result))));
    output_buffer_p = NULL;
    output_buffer_p =
        sox_open_mem_write (message_block_p->wr_ptr (),
                            inherited::configuration_->streamConfiguration->bufferSize,
                            &signalInfo_,
                            &encodingInfo_,
                            ACE_TEXT_ALWAYS_CHAR (STREAM_DECODER_SOX_FORMAT_RAW_STRING),
                            NULL);
    if (!output_buffer_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_open_mem_write(\"%s\"): \"%m\", aborting\n")));
      goto error;
    } // end IF
    effect_options[0] = reinterpret_cast<char*> (output_buffer_p);
    result = sox_effect_options (output_, 1, effect_options);
    if (result != SOX_SUCCESS)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_effect_options(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
      goto error;
    } // end IF
  } while (true);
//  ACE_ASSERT (output_buffer_p->tell_off <= inherited::configuration_->streamConfiguration->bufferSize);
  message_block_p->wr_ptr ((output_buffer_p->tell_off <= inherited::configuration_->streamConfiguration->bufferSize) ? output_buffer_p->tell_off
                                                                                                                     : inherited::configuration_->streamConfiguration->bufferSize);

  result = inherited::put_next (buffer_, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  buffer_ = NULL;

  result = sox_close (input_buffer_p);
  if (result != SOX_SUCCESS)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sox_close(): \"%s\", continuing\n"),
                ACE_TEXT (sox_strerror (result))));
  result = sox_close (output_buffer_p);
  if (result != SOX_SUCCESS)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sox_close(): \"%s\", continuing\n"),
                ACE_TEXT (sox_strerror (result))));

  // clean up
  message_inout->release ();
  message_inout = NULL;

  return;

error:
  if (input_buffer_p)
  {
    result = sox_close (input_buffer_p);
    if (result != SOX_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_close(): \"%s\", continuing\n"),
                  ACE_TEXT (sox_strerror (result))));
  } // end IF
  if (output_buffer_p)
  {
    result = sox_close (output_buffer_p);
    if (result != SOX_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_close(): \"%s\", continuing\n"),
                  ACE_TEXT (sox_strerror (result))));
  } // end IF

  if (buffer_)
  {
    buffer_->release ();
    buffer_ = NULL;
  } // end IF

  // clean up
  message_inout->release ();
  message_inout = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
void
Stream_Decoder_SoXEffect_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SoXEffect_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->format);

  int result = -1;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_->format);

      Stream_Module_Decoder_Tools::ALSA2SOX (*inherited::configuration_->format,
                                             encodingInfo_,
                                             signalInfo_);

      const struct sox_effect_handler_t* effect_handler_p = NULL;
      struct sox_effect_t* effect_p = NULL;
      char* effect_options[256]; // *TODO*: should be dynamic
      int index = 0;
      sox_signalinfo_t intermediate_signal;
      std::string effect_options_string;

      ACE_ASSERT (!chain_);
      chain_ = sox_create_effects_chain (&encodingInfo_,
                                         &encodingInfo_);
      if (!chain_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_create_effects_chain(), aborting\n")));
        goto error;
      } // end IF

      // add input and output 'effects'
      effect_handler_p =
          sox_find_effect (ACE_TEXT_ALWAYS_CHAR ("input"));
      if (!effect_handler_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_find_effect(\"input\"), aborting\n")));
        goto error;
      } // end IF
      input_ = sox_create_effect (effect_handler_p);
      if (!input_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_create_effect(\"input\"), aborting\n")));
        goto error;
      } // end IF
      intermediate_signal = signalInfo_;
      result = sox_add_effect (chain_,
                               input_,
                               &intermediate_signal,
                               &signalInfo_);
      if (result != SOX_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_add_effect(\"input\"): \"%s\", aborting\n"),
                    ACE_TEXT (sox_strerror (result))));
        goto error;
      } // end IF

      effect_handler_p =
          sox_find_effect (inherited::configuration_->effect.c_str ());
      if (!effect_handler_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_find_effect(\"%s\"), aborting\n"),
                    ACE_TEXT (inherited::configuration_->effect.c_str ())));
        goto error;
      } // end IF
      effect_p = sox_create_effect (effect_handler_p);
      if (!effect_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_create_effect(\"%s\"), aborting\n"),
                    ACE_TEXT (inherited::configuration_->effect.c_str ())));
        goto error;
      } // end IF
      if (!inherited::configuration_->effectOptions.empty ())
      {
        for (std::vector<std::string>::const_iterator iterator = inherited::configuration_->effectOptions.begin ();
             iterator != inherited::configuration_->effectOptions.end ();
             ++iterator, ++index)
        {
          effect_options[index] = const_cast<char*> ((*iterator).c_str ());
          effect_options_string += *iterator;
          effect_options_string += ACE_TEXT_ALWAYS_CHAR (" ");
        } // end FOR
        result = sox_effect_options (effect_p,
                                     index,
                                     effect_options);
        if (result != SOX_SUCCESS)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to sox_effect_options(\"%s\"): \"%s\", aborting\n"),
                      ACE_TEXT (inherited::configuration_->effect.c_str ()),
                      ACE_TEXT (sox_strerror (result))));
          goto error;
        } // end IF
      } // end IF
      result = sox_add_effect (chain_,
                               effect_p,
                               &intermediate_signal,
                               &signalInfo_);
      if (result != SOX_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_add_effect(\"%s\"): \"%s\", aborting\n"),
                    ACE_TEXT (inherited::configuration_->effect.c_str ()),
                    ACE_TEXT (sox_strerror (result))));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("added SoX effect \"%s\" (options: \"%s\")...\n"),
                  ACE_TEXT (inherited::configuration_->effect.c_str ()),
                  ACE_TEXT (effect_options_string.c_str ())));

      effect_handler_p =
          sox_find_effect (ACE_TEXT_ALWAYS_CHAR ("output"));
      if (!effect_handler_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_find_effect(\"output\"), aborting\n")));
        goto error;
      } // end IF
      output_ = sox_create_effect (effect_handler_p);
      if (!output_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_create_effect(\"output\"), aborting\n")));
        goto error;
      } // end IF
      result = sox_add_effect (chain_,
                               output_,
                               &intermediate_signal,
                               &signalInfo_);
      if (result != SOX_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_add_effect(\"output\"): \"%s\", aborting\n"),
                    ACE_TEXT (sox_strerror (result))));
        goto error;
      } // end IF
      // *NOTE*: SoX effects work on 32-bit integer samples
      //         --> convert back to input format
      output_->out_signal.precision = signalInfo_.precision;

      goto continue_;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (buffer_)
      {
        buffer_->release ();
        buffer_ = NULL;
      } // end IF

      if (chain_)
      {
        sox_delete_effects_chain (chain_);
        chain_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
DataMessageType*
Stream_Decoder_SoXEffect_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SoXEffect_T::allocateMessage"));

  // initialize return value(s)
  DataMessageType* message_block_p = NULL;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  if (inherited::configuration_->messageAllocator)
  {
allocate:
    try {
      message_block_p =
        static_cast<DataMessageType*> (inherited::configuration_->messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited::configuration_->messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_block_p,
                      DataMessageType (requestedSize_in));
  if (!message_block_p)
  {
    if (inherited::configuration_->messageAllocator)
    {
      if (inherited::configuration_->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate data message: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate data message: \"%m\", aborting\n")));
  } // end IF

  return message_block_p;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
struct v4l2_format*
Stream_Decoder_SoXEffect_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType>::getFormat_impl (const Stream_Module_Device_ALSAConfiguration&)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SoXEffect_T::getFormat_impl"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);
  ACE_NOTREACHED (return NULL;)
}
