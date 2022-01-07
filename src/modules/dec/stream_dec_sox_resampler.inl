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

#include <utility>

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
 , encodingInfoOut_ ()
 , input_ (NULL)
 , output_ (NULL)
 , signalInfo_ ()
 , signalInfoOut_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SoXResampler_T::Stream_Decoder_SoXResampler_T"));

  ACE_OS::memset (&encodingInfo_, 0, sizeof (struct sox_encodinginfo_t));
  ACE_OS::memset (&encodingInfoOut_, 0, sizeof (struct sox_encodinginfo_t));
  ACE_OS::memset (&signalInfo_, 0, sizeof (struct sox_signalinfo_t));
  ACE_OS::memset (&signalInfoOut_, 0, sizeof (struct sox_signalinfo_t));
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

    ACE_OS::memset (&encodingInfo_, 0, sizeof (struct sox_encodinginfo_t));
    ACE_OS::memset (&encodingInfoOut_, 0, sizeof (struct sox_encodinginfo_t));
    ACE_OS::memset (&signalInfo_, 0, sizeof (struct sox_signalinfo_t));
    ACE_OS::memset (&signalInfoOut_, 0, sizeof (struct sox_signalinfo_t));

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
  if (unlikely (!chain_))
    return;
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  ACE_Message_Block* message_block_p = NULL, *message_block_2 = NULL;
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
                          &signalInfoOut_,
                          &encodingInfoOut_,
                          ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_SOX_FORMAT_RAW_STRING),
                          NULL);
  if (unlikely (!output_buffer_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_open_mem_write(): \"%m\", returning\n"),
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

    // output buffer is full --> allocate another one
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *IMPORTANT NOTE*: SoX cannot write to the message block directly
    // (Win32/MinGW does not currently support fmemopen())
    // --> copy the data out of the tmpfile() manually
    FILE* file_p = reinterpret_cast<FILE*> (output_buffer_p->fp);
    ACE_OS::rewind (file_p);
    size_t bytes_read_i = ACE_OS::fread (message_block_p->wr_ptr (),
                                         1,
                                         inherited::configuration_->allocatorConfiguration->defaultBufferSize,
                                         file_p);
    ACE_ASSERT (bytes_read_i == inherited::configuration_->allocatorConfiguration->defaultBufferSize);
#endif // ACE_WIN32 || ACE_WIN64
    message_block_p->wr_ptr (std::min (output_buffer_p->tell_off, static_cast<uint64_t> (inherited::configuration_->allocatorConfiguration->defaultBufferSize)));

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
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_close(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
      goto error;
    } // end IF
    output_buffer_p = NULL;
    output_buffer_p =
        sox_open_mem_write (message_block_p->wr_ptr (),
                            inherited::configuration_->allocatorConfiguration->defaultBufferSize,
                            &signalInfoOut_,
                            &encodingInfoOut_,
                            ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_SOX_FORMAT_RAW_STRING),
                            NULL);
    if (unlikely (!output_buffer_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_open_mem_write(): \"%m\", returning\n"),
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
  message_block_p->wr_ptr (std::min (output_buffer_p->tell_off, static_cast<uint64_t> (inherited::configuration_->allocatorConfiguration->defaultBufferSize)));

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
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_close(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (sox_strerror (result))));
    goto error;
  } // end IF
  result = sox_close (output_buffer_p);
  if (unlikely (result != SOX_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_close(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (sox_strerror (result))));
    goto error;
  } // end IF

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
      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->getR ());
      const struct sox_effect_handler_t* effect_handler_p = NULL;
      struct sox_effect_t* effect_p = NULL;
      struct sox_signalinfo_t intermediate_signal_s, target_signal_s;
      MediaType media_type_3;
      ACE_OS::memset (&media_type_3, 0, sizeof (MediaType));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s, media_type_2;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      ACE_OS::memset (&media_type_2, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                media_type_s);
      Stream_MediaFramework_DirectShow_Tools::to (media_type_s,
                                                  encodingInfo_,
                                                  signalInfo_);
      inherited2::getMediaType (inherited::configuration_->outputFormat,
                                media_type_2);
      Stream_MediaFramework_DirectShow_Tools::to (media_type_2,
                                                  encodingInfoOut_,
                                                  signalInfoOut_);
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s, media_type_2;
      inherited2::getMediaType (session_data_r.formats.back (),
                                media_type_s);
      Stream_MediaFramework_ALSA_Tools::to (media_type_s,
                                            encodingInfo_,
                                            signalInfo_);
      inherited2::getMediaType (inherited::configuration_->outputFormat,
                                media_type_2);
      Stream_MediaFramework_ALSA_Tools::to (media_type_2,
                                            encodingInfoOut_,
                                            signalInfoOut_);
#endif // ACE_WIN32 || ACE_WIN64
      if (unlikely ((signalInfo_.channels == signalInfoOut_.channels)   &&
                    (signalInfo_.precision == signalInfoOut_.precision) &&
                    (signalInfo_.rate == signalInfoOut_.rate)))
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: output format is input format, continuing\n"),
                    inherited::mod_->name ()));
        goto continue_2;
      } // end IF

      ACE_ASSERT (!chain_);
      chain_ = sox_create_effects_chain (&encodingInfo_,
                                         &encodingInfoOut_);
      if (unlikely (!chain_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_create_effects_chain(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      // add input,output and conversion 'effects'
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
      intermediate_signal_s = signalInfo_;
      result = sox_add_effect (chain_,
                               input_,
                               &intermediate_signal_s,
                               &signalInfo_);
      if (unlikely (result != SOX_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_add_effect(\"input\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (sox_strerror (result))));
        goto error;
      } // end IF

      if (signalInfo_.rate != signalInfoOut_.rate)
      {
        effect_handler_p = sox_find_effect (ACE_TEXT_ALWAYS_CHAR ("rate"));
        if (unlikely (!effect_handler_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_find_effect(\"rate\"), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        effect_p = sox_create_effect (effect_handler_p);
        if (unlikely (!effect_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_create_effect(\"rate\"), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        //const char* args[] = {"-h", "-b", "99,7"};
        result = sox_effect_options (effect_p,
                                     //3, (char**)args);
                                     0, NULL);
        if (unlikely (result != SOX_SUCCESS))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_effect_options(\"rate\"): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (sox_strerror (result))));
          goto error;
        } // end IF
        target_signal_s = intermediate_signal_s;
        target_signal_s.rate = signalInfoOut_.rate;
        result = sox_add_effect (chain_,
                                 effect_p,
                                 &intermediate_signal_s,
                                 &target_signal_s);
        if (unlikely (result != SOX_SUCCESS))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_add_effect(\"rate\"): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (sox_strerror (result))));
          goto error;
        } // end IF
        free (effect_p); effect_p = NULL;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: added SoX effect \"rate\"\n"),
                    inherited::mod_->name ()));
      } // end IF
      if (signalInfo_.channels != signalInfoOut_.channels)
      {
        effect_handler_p = sox_find_effect (ACE_TEXT_ALWAYS_CHAR ("channels"));
        if (unlikely (!effect_handler_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_find_effect(\"channels\"), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        effect_p = sox_create_effect (effect_handler_p);
        if (unlikely (!effect_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_create_effect(\"channels\"), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        result = sox_effect_options (effect_p,
                                     0, NULL);
        if (unlikely (result != SOX_SUCCESS))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_effect_options(\"channels\"): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (sox_strerror (result))));
          goto error;
        } // end IF
        target_signal_s = intermediate_signal_s;
        target_signal_s.channels = signalInfoOut_.channels;
        result = sox_add_effect (chain_,
                                 effect_p,
                                 &intermediate_signal_s,
                                 &target_signal_s);
        if (unlikely (result != SOX_SUCCESS))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_add_effect(\"channels\"): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (sox_strerror (result))));
          goto error;
        } // end IF
        free (effect_p); effect_p = NULL;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: added SoX effect \"channels\"\n"),
                    inherited::mod_->name ()));
      } // end IF
      if (signalInfo_.precision != signalInfoOut_.precision)
      {
        effect_handler_p = sox_find_effect (ACE_TEXT_ALWAYS_CHAR ("precision"));
        if (unlikely (!effect_handler_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_find_effect(\"precision\"), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        effect_p = sox_create_effect (effect_handler_p);
        if (unlikely (!effect_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_create_effect(\"precision\"), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        result = sox_effect_options (effect_p,
                                     0, NULL);
        if (unlikely (result != SOX_SUCCESS))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_effect_options(\"precision\"): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (sox_strerror (result))));
          goto error;
        } // end IF
        target_signal_s = intermediate_signal_s;
        target_signal_s.precision = signalInfoOut_.precision;
        result = sox_add_effect (chain_,
                                 effect_p,
                                 &intermediate_signal_s,
                                 &target_signal_s);
        if (unlikely (result != SOX_SUCCESS))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_add_effect(\"precision\"): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (sox_strerror (result))));
          goto error;
        } // end IF
        free (effect_p); effect_p = NULL;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: added SoX effect \"precision\"\n"),
                    inherited::mod_->name ()));
      } // end IF

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
                               &intermediate_signal_s,
                               &signalInfoOut_);
      if (unlikely (result != SOX_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_add_effect(\"output\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (sox_strerror (result))));
        goto error;
      } // end IF
      // *NOTE*: SoX effects work on 32-bit integer samples
      //         --> convert back to output format
      output_->out_signal.precision = signalInfoOut_.precision;

      inherited2::getMediaType (inherited::configuration_->outputFormat,
                                media_type_3);
      session_data_r.formats.push_back (media_type_3);

continue_2:
#if defined(ACE_WIN32) || defined(ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
      Stream_MediaFramework_DirectShow_Tools::free (media_type_2);
#endif // ACE_WIN32 || ACE_WIN64

      goto continue_;

error:
#if defined(ACE_WIN32) || defined(ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
      Stream_MediaFramework_DirectShow_Tools::free (media_type_2);
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
      free (input_); input_ = NULL;
      free (output_); output_ = NULL;

      break;
    }
    default:
      break;
  } // end SWITCH
}
