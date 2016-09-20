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
 , manageSoX_ (true)
 , signalInfo_ ()
 , SoXBuffer_ (NULL)
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
  if (SoXBuffer_)
  {
    result = sox_close (SoXBuffer_);
    if (result != SOX_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_close(), continuing\n")));
  } // end IF

  if (manageSoX_)
  {
    result = sox_quit ();
    if (result != SOX_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_quit(): \"%m\", continuing\n")));
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
    if (SoXBuffer_)
    {
      result = sox_close (SoXBuffer_);
      if (result != SOX_SUCCESS)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_close(), continuing\n")));
      SoXBuffer_ = NULL;
    } // end IF

    if (manageSoX_)
    {
      result = sox_quit ();
      if (result != SOX_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_quit(): \"%m\", aborting\n")));
        return false;
      } // end IF
    } // end IF

    inherited::isInitialized_ = false;
  } // end IF

  if (manageSoX_)
  {
    result = sox_init ();
    if (result != SOX_SUCCESS)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_init(): \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end IF

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

  // *IMPORTANT NOTE*: sox_write() expects signed 32-bit samples
  //                   --> convert the data in memory first
  size_t samples_read, samples_written = 0;
  /* Temporary store whilst copying. */
  sox_sample_t samples[STREAM_DECODER_SOX_SAMPLE_BUFFERS];
  sox_format_t* memory_buffer_p =
      sox_open_mem_read (message_inout->rd_ptr (),
                         message_inout->length (),
                         &signalInfo_,
                         &encodingInfo_,
                         ACE_TEXT_ALWAYS_CHAR ("raw"));
  if (!memory_buffer_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_open_mem_read(): \"%m\", returning\n"),
                inherited::mod_->name ()));
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
  if (!SoXBuffer_)
  {
    SoXBuffer_ =
        sox_open_mem_write (buffer_->wr_ptr (),
                            buffer_->size (),
                            &signalInfo_,
                            &encodingInfo_,
                            //ACE_TEXT_ALWAYS_CHAR (STREAM_DECODER_SOX_WAV_FORMATTYPE_STRING),
                            NULL,
                            NULL);
    if (!SoXBuffer_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_open_mem_write(\"%s\"): \"%m\", aborting\n")));
      goto error;
    } // end IF
  } // end IF

  do
  {
    samples_read = sox_read (memory_buffer_p,
                             samples,
                             STREAM_DECODER_SOX_SAMPLE_BUFFERS);
    if (!samples_read) break;
    samples_written = sox_write (SoXBuffer_,
                                 samples,
                                 samples_read);
    if (samples_written != samples_read)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_write(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
  } while (true);

  result = sox_close (memory_buffer_p);
  if (result != SOX_SUCCESS)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sox_close(), continuing\n")));

  return;

error:
  if (memory_buffer_p)
  {
    result = sox_close (memory_buffer_p);
    if (result != SOX_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_close(), continuing\n")));
  } // end IF

  if (buffer_)
  {
    buffer_->release ();
    buffer_ = NULL;
  } // end IF
  if (SoXBuffer_)
  {
    result = sox_close (SoXBuffer_);
    if (result != SOX_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_close(), continuing\n")));
    SoXBuffer_ = NULL;
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
      int number_of_effect_options = 10;
      char* effect_options[number_of_effect_options];
      ACE_OS::memset (effect_options, 0, sizeof (effect_options));

      ACE_ASSERT (!chain_);
      chain_ = sox_create_effects_chain (&encodingInfo_, &encodingInfo_);
      if (!chain_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_create_effects_chain(), aborting\n")));
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
      result = sox_effect_options (effect_p,
                                   number_of_effect_options,
                                   effect_options);
      if (result != SOX_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_effect_options(\"%s\"), aborting\n"),
                    ACE_TEXT (inherited::configuration_->effect.c_str ())));
        goto error;
      } // end IF
      result = sox_add_effect (chain_,
                               effect_p,
                               &signalInfo_,
                               &signalInfo_);
      if (result != SOX_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_add_effect(\"%s\"), aborting\n"),
                    ACE_TEXT (inherited::configuration_->effect.c_str ())));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("added SoX effect \"%s\"...\n"),
                  ACE_TEXT (inherited::configuration_->effect.c_str ())));

      goto continue_;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (chain_)
      {
        sox_delete_effects_chain (chain_);
        chain_ = NULL;
      } // end IF

      if (SoXBuffer_)
      {
        result = sox_close (SoXBuffer_);
        if (result != SOX_SUCCESS)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to sox_close(), continuing\n")));
        SoXBuffer_ = NULL;
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
