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

#include "stream_lib_directshow_tools.h"
#include "stream_lib_directsound_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
Stream_Dev_Target_WavOut_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataType,
                           MediaType>::Stream_Dev_Target_WavOut_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , CBData_ ()
 , handle_ (NULL)
 , queue_ (ACE_Message_Queue_Base::DEFAULT_HWM,
           ACE_Message_Queue_Base::DEFAULT_LWM)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WavOut_T::Stream_Dev_Target_WavOut_T"));

  CBData_.inFlightBuffers = 0;
  CBData_.queue = &queue_;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
Stream_Dev_Target_WavOut_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataType,
                           MediaType>::~Stream_Dev_Target_WavOut_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WavOut_T::~Stream_Dev_Target_WavOut_T"));

  if (unlikely (handle_))
    waveOutClose (handle_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
bool
Stream_Dev_Target_WavOut_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataType,
                           MediaType>::initialize (const ConfigurationType& configuration_in,
                                                   Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WavOut_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (unlikely (handle_))
    {
      waveOutClose (handle_); handle_ = NULL;
    } // end IF
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
void
Stream_Dev_Target_WavOut_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataType,
                           MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WavOut_T::handleDataMessage"));

  // sanity check(s)
  if (unlikely (!handle_))
    return;

  passMessageDownstream_out = false;

  ACE_Message_Block* message_block_p = message_inout, *message_block_2 = NULL;
  MMRESULT result = MMSYSERR_ERROR;
  int result_2 = -1;
  struct wavehdr_tag* header_p = NULL;

continue_:
  // step1: get next free header
  result_2 = queue_.dequeue (header_p,
                             NULL);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue_Ex::dequeue(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF
  ACE_ASSERT (header_p);

  // step2: unprepare header
  message_block_2 =
    reinterpret_cast<ACE_Message_Block*> (header_p->dwUser);
  if (likely (message_block_2))
    message_block_2->release ();
  result = waveOutUnprepareHeader (handle_,
                                   header_p,
                                   sizeof (struct wavehdr_tag));
  if (unlikely (result != MMSYSERR_NOERROR))
  { char error_msg_a[BUFSIZ];
    waveOutGetErrorText (result, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to waveOutUnprepareHeader(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (error_msg_a)));
    goto error;
  } // end IF

  // step3: prepare header
  header_p->lpData = message_block_p->rd_ptr ();
  header_p->dwBufferLength = message_block_p->length ();
  header_p->dwUser = reinterpret_cast<DWORD_PTR> (message_block_p);
  header_p->dwFlags = 0;
  result = waveOutPrepareHeader (handle_,
                                 header_p,
                                 sizeof (struct wavehdr_tag));
  if (unlikely (result != MMSYSERR_NOERROR))
  { char error_msg_a[BUFSIZ];
    waveOutGetErrorText (result, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to waveOutPrepareHeader(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (error_msg_a)));
    goto error;
  } // end IF

  // step4: send buffer
  result = waveOutWrite (handle_,
                         header_p,
                         sizeof (struct wavehdr_tag));
  if (unlikely (result != MMSYSERR_NOERROR))
  { char error_msg_a[BUFSIZ];
    waveOutGetErrorText (result, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to waveOutWrite(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (error_msg_a)));
    waveOutUnprepareHeader (handle_,
                            header_p,
                            sizeof (struct wavehdr_tag));
    goto error;
  } // end IF
  header_p = NULL;
  ++CBData_.inFlightBuffers;

  message_block_p = message_block_p->cont ();
  if (unlikely (message_block_p))
    goto continue_;

  return;

error:
  if (header_p)
  {
    result_2 = queue_.enqueue (header_p,
                               NULL);
    if (unlikely (result_2 == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue_Ex::enqueue(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
void
Stream_Dev_Target_WavOut_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataType,
                           MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WavOut_T::handleSessionMessage"));

  MMRESULT result = MMSYSERR_ERROR;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      CBData_.inFlightBuffers = 0; // *TODO*: remove this ASAP
      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::ID);
      ACE_ASSERT (inherited::sessionData_);
      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->getR ());
      ACE_ASSERT (!session_data_r.formats.empty ());
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      ACE_ASSERT (waveformatex_p);

      if (!allocateBuffers ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Dev_Target_WavOut_T::allocateBuffers(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      DWORD flags_u = CALLBACK_FUNCTION |
                      //WAVE_ALLOWSYNC    |
                      //WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE |
                      WAVE_FORMAT_DIRECT;
                      //WAVE_MAPPED;
      result =
        waveOutOpen (&handle_,
                     //WAVE_MAPPER,
                     inherited::configuration_->deviceIdentifier.identifier._id,
                     waveformatex_p,
                     reinterpret_cast<DWORD_PTR> (stream_dev_waveout_data_cb),
                     reinterpret_cast<DWORD_PTR> (&CBData_),
                     flags_u);
      if (unlikely (result != MMSYSERR_NOERROR))
      { char error_msg_a[BUFSIZ];
        waveOutGetErrorText (result, error_msg_a, BUFSIZ - 1);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to waveOutOpen(%u,0x%x): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    inherited::configuration_->deviceIdentifier.identifier._id,
                    flags_u,
                    ACE_TEXT (error_msg_a)));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened device (id: %u, format: %s)...\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->deviceIdentifier.identifier._id,
                  ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::toString (*waveformatex_p, true).c_str ())));
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      break;

error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // render remaining data
      while (CBData_.inFlightBuffers)
        ACE_OS::sleep (ACE_Time_Value (1, 0));

      if (likely (handle_))
      {
        result = waveOutReset (handle_);
        if (unlikely (result != MMSYSERR_NOERROR))
        {
          char error_msg_a[BUFSIZ];
          waveOutGetErrorText (result, error_msg_a, BUFSIZ - 1);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to waveOutReset(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (error_msg_a)));
        } // end IF

        result = waveOutClose (handle_);
        if (unlikely (result != MMSYSERR_NOERROR))
        {
          char error_msg_a[BUFSIZ];
          waveOutGetErrorText (result, error_msg_a, BUFSIZ - 1);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to waveOutClose(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (error_msg_a)));
        } // end IF
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: closed device (id: %u)...\n"),
                      inherited::mod_->name (),
                      inherited::configuration_->deviceIdentifier.identifier._id));
      } // end IF

      int result_2 = -1;
      struct wavehdr_tag* buffer_p = NULL;
      for (unsigned int i = 0;
           i < STREAM_DEV_WAVEOUT_DEFAULT_DEVICE_BUFFERS;
           ++i)
      {
        result_2 = queue_.dequeue (buffer_p,
                                   NULL);
        if (unlikely (result_2 == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue_Ex::dequeue(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
          continue;
        } // end IF
        ACE_ASSERT (buffer_p);
        delete buffer_p; buffer_p = NULL;
      } // end FOR

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
          typename SessionDataType,
          typename MediaType>
bool
Stream_Dev_Target_WavOut_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataType,
                           MediaType>::allocateBuffers ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WavOut_T::allocateBuffers"));

  struct wavehdr_tag* buffer_p = NULL;
  int result = -1;

  for (unsigned int i = 0;
       i < STREAM_DEV_WAVEOUT_DEFAULT_DEVICE_BUFFERS;
       ++i)
  {
    ACE_NEW_NORETURN (buffer_p,
                      struct wavehdr_tag ());
    if (unlikely (!buffer_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF
    ACE_OS::memset (buffer_p, 0, sizeof (struct wavehdr_tag));

    result = queue_.enqueue (buffer_p,
                             NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue_Ex::enqueue(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      delete buffer_p;
      return false;
    } // end IF
  } // end FOR

  return true;
}
