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
#include "ace/OS.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#else
#include "stream_lib_alsa_common.h"
#include "stream_lib_alsa_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dec_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType>
Stream_Decoder_SoXResampler_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              SessionDataType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              MediaType>::Stream_Decoder_SoXResampler_T (ISTREAM_T* stream_in)
#else
                              MediaType>::Stream_Decoder_SoXResampler_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , buffer_ (NULL)
 , chain_ (NULL)
 , encodingInfo_ ()
 , input_ (NULL)
 , output_ (NULL)
 , signalInfo_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SoXResampler_T::Stream_Decoder_SoXResampler_T"));

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
          typename SessionDataType,
          typename MediaType>
Stream_Decoder_SoXResampler_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              SessionDataType,
                              MediaType>::~Stream_Decoder_SoXResampler_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SoXResampler_T::~Stream_Decoder_SoXResampler_T"));

  int result = -1;

  if (buffer_)
    buffer_->release ();

  if (chain_)
    sox_delete_effects_chain (chain_);

  if (inherited::configuration_ &&
      inherited::configuration_->manageSoX)
  {
    result = sox_quit ();
    if (unlikely (result != SOX_SUCCESS))
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
          typename SessionDataType,
          typename MediaType>
bool
Stream_Decoder_SoXResampler_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              SessionDataType,
                              MediaType>::initialize (const ConfigurationType& configuration_in,
                                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SoXResampler_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
      buffer_->release (); buffer_ = NULL;
    } // end IF

    if (chain_)
    {
      sox_delete_effects_chain (chain_); chain_ = NULL;
    } // end IF
    input_ = NULL; output_ = NULL;

    ACE_ASSERT (inherited::configuration_);
    if (inherited::configuration_->manageSoX)
    {
      result = sox_quit ();
      if (unlikely (result != SOX_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_quit(): \"%s\", aborting\n"),
                    ACE_TEXT (sox_strerror (result))));
        return false;
      } // end IF
    } // end IF
  } // end IF

  if (configuration_in.manageSoX)
  {
    result = sox_init ();
    if (unlikely (result != SOX_SUCCESS))
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
      STREAM_DEC_SOX_BUFFER_SIZE;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType>
void
Stream_Decoder_SoXResampler_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              SessionDataType,
                              MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SoXResampler_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  if (inherited::configuration_->effect.empty ())
    return;

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Message_Block* message_block_2 = NULL;
  char* effect_options[1];
  struct sox_format_t* input_buffer_p = NULL, *output_buffer_p = NULL;

  input_buffer_p =
      sox_open_mem_read (message_inout->rd_ptr (),
                         message_inout->length (),
                         &signalInfo_,
                         &encodingInfo_,
                         ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_SOX_FORMAT_RAW_STRING));
  if (unlikely (!input_buffer_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_open_mem_read(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  effect_options[0] = reinterpret_cast<char*> (input_buffer_p);
  result = sox_effect_options (input_, 1, effect_options);
  if (unlikely (result != SOX_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_effect_options(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (sox_strerror (result))));
    goto error;
  } // end IF

  if (unlikely (!buffer_))
  {
    buffer_ =
        inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (unlikely (!buffer_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: allocateMessage(%d) failed: \"%m\", returning\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));
      goto error;
    } // end IF
  } // end IF
  message_block_p = buffer_;
  output_buffer_p =
      sox_open_mem_write (message_block_p->wr_ptr (),
                          inherited::configuration_->allocatorConfiguration->defaultBufferSize,
                          &signalInfo_,
                          &encodingInfo_,
                          ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_SOX_FORMAT_RAW_STRING),
                          NULL);
  if (unlikely (!output_buffer_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_open_mem_write(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  effect_options[0] = reinterpret_cast<char*> (output_buffer_p);
  result = sox_effect_options (output_, 1, effect_options);
  if (unlikely (result != SOX_SUCCESS))
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
    if (unlikely (result != SOX_EOF))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_flow_effects(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
      goto error;
    } // end IF

    // output buffer is full --> (dispatch and- ?) allocate another one
//    ACE_ASSERT (output_buffer_p->tell_off <= inherited::configuration_->streamConfiguration->bufferSize);
    message_block_p->wr_ptr ((output_buffer_p->tell_off <= inherited::configuration_->allocatorConfiguration->defaultBufferSize) ? output_buffer_p->tell_off
                                                                                                                                 : inherited::configuration_->allocatorConfiguration->defaultBufferSize);

    message_block_2 = NULL;
    message_block_2 =
        inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (unlikely (!message_block_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: allocateMessage(%d) failed: \"%m\", returning\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));
      goto error;
    } // end IF
    message_block_p->cont (message_block_2);
    message_block_p = message_block_2;

    result = sox_close (output_buffer_p);
    if (unlikely (result != SOX_SUCCESS))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_close(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
    output_buffer_p = NULL;
    output_buffer_p =
        sox_open_mem_write (message_block_p->wr_ptr (),
                            inherited::configuration_->allocatorConfiguration->defaultBufferSize,
                            &signalInfo_,
                            &encodingInfo_,
                            ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_SOX_FORMAT_RAW_STRING),
                            NULL);
    if (unlikely (!output_buffer_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_open_mem_write(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
    effect_options[0] = reinterpret_cast<char*> (output_buffer_p);
    result = sox_effect_options (output_, 1, effect_options);
    if (unlikely (result != SOX_SUCCESS))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_effect_options(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
      goto error;
    } // end IF
  } while (true);
//  ACE_ASSERT (output_buffer_p->tell_off <= inherited::configuration_->streamConfiguration->bufferSize);
  message_block_p->wr_ptr ((output_buffer_p->tell_off <= inherited::configuration_->allocatorConfiguration->defaultBufferSize) ? output_buffer_p->tell_off
                                                                                                                               : inherited::configuration_->allocatorConfiguration->defaultBufferSize);

  result = inherited::put_next (buffer_, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  buffer_ = NULL;

  result = sox_close (input_buffer_p);
  if (unlikely (result != SOX_SUCCESS))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_close(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (sox_strerror (result))));
  result = sox_close (output_buffer_p);
  if (unlikely (result != SOX_SUCCESS))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_close(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (sox_strerror (result))));

  // clean up
  message_inout->release (); message_inout = NULL;

  return;

error:
  if (input_buffer_p)
  {
    result = sox_close (input_buffer_p);
    if (unlikely (result != SOX_SUCCESS))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_close(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
  } // end IF
  if (output_buffer_p)
  {
    result = sox_close (output_buffer_p);
    if (unlikely (result != SOX_SUCCESS))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_close(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
  } // end IF

  if (buffer_)
  {
    buffer_->release (); buffer_ = NULL;
  } // end IF

  // clean up
  message_inout->release (); message_inout = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType>
void
Stream_Decoder_SoXResampler_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              SessionDataType,
                              MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SoXResampler_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  int result = -1;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      if (inherited::configuration_->effect.empty ())
        break;
      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->getR ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                media_type_s);
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                media_type_s);
      Stream_MediaFramework_ALSA_Tools::ALSAToSoX (media_type_r.format,
                                                   media_type_r.rate,
                                                   media_type_r.channels,
                                                   encodingInfo_,
                                                   signalInfo_);
#endif // ACE_WIN32 || ACE_WIN64

      const struct sox_effect_handler_t* effect_handler_p = NULL;
      struct sox_effect_t* effect_p = NULL;
      char* effect_options[256]; // *TODO*: should be dynamic
      int index = 0;
      sox_signalinfo_t intermediate_signal;
      std::string effect_options_string;

      ACE_ASSERT (!chain_);
      chain_ = sox_create_effects_chain (&encodingInfo_,
                                         &encodingInfo_);
      if (unlikely (!chain_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_create_effects_chain(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      // add input and output 'effects'
      effect_handler_p =
          sox_find_effect (ACE_TEXT_ALWAYS_CHAR ("input"));
      if (unlikely (!effect_handler_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_find_effect(\"input\"), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      input_ = sox_create_effect (effect_handler_p);
      if (unlikely (!input_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_create_effect(\"input\"), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      intermediate_signal = signalInfo_;
      result = sox_add_effect (chain_,
                               input_,
                               &intermediate_signal,
                               &signalInfo_);
      if (unlikely (result != SOX_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_add_effect(\"input\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (sox_strerror (result))));
        goto error;
      } // end IF

      effect_handler_p =
          sox_find_effect (inherited::configuration_->effect.c_str ());
      if (unlikely (!effect_handler_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_find_effect(\"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_->effect.c_str ())));
        goto error;
      } // end IF
      effect_p = sox_create_effect (effect_handler_p);
      if (unlikely (!effect_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_create_effect(\"%s\"), aborting\n"),
                    inherited::mod_->name (),
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
        if (unlikely (result != SOX_SUCCESS))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_effect_options(\"%s\"): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (inherited::configuration_->effect.c_str ()),
                      ACE_TEXT (sox_strerror (result))));
          goto error;
        } // end IF
      } // end IF
      result = sox_add_effect (chain_,
                               effect_p,
                               &intermediate_signal,
                               &signalInfo_);
      if (unlikely (result != SOX_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_add_effect(\"%s\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_->effect.c_str ()),
                    ACE_TEXT (sox_strerror (result))));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: added SoX effect \"%s\" (options: \"%s\")\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (inherited::configuration_->effect.c_str ()),
                  ACE_TEXT (effect_options_string.c_str ())));
      effect_handler_p =
          sox_find_effect (ACE_TEXT_ALWAYS_CHAR ("output"));
      if (unlikely (!effect_handler_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_find_effect(\"output\"), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      output_ = sox_create_effect (effect_handler_p);
      if (unlikely (!output_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_create_effect(\"output\"), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      result = sox_add_effect (chain_,
                               output_,
                               &intermediate_signal,
                               &signalInfo_);
      if (unlikely (result != SOX_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_add_effect(\"output\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (sox_strerror (result))));
        goto error;
      } // end IF
      // *NOTE*: SoX effects work on 32-bit integer samples
      //         --> convert back to input format
      output_->out_signal.precision = signalInfo_.precision;

#if defined(ACE_WIN32) || defined(ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#endif // ACE_WIN32 || ACE_WIN64

      goto continue_;

error:
#if defined(ACE_WIN32) || defined(ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#endif // ACE_WIN32 || ACE_WIN64

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (buffer_)
      {
        buffer_->release (); buffer_ = NULL;
      } // end IF

      if (chain_)
      {
        sox_delete_effects_chain (chain_); chain_ = NULL;
      } // end IF
      input_ = NULL; output_ = NULL;

      break;
    }
    default:
      break;
  } // end SWITCH
}
