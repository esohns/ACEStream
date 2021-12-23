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

static void CALLBACK
stream_dev_target_wavout_async_callback (HWAVEOUT  hwo,
                                         UINT      uMsg,
                                         DWORD_PTR dwInstance,
                                         DWORD_PTR dwParam1,
                                         DWORD_PTR dwParam2)
{
  //STREAM_TRACE (ACE_TEXT ("::stream_dev_target_wavout_async_callback"));

  // sanity check(s)
  struct Stream_Device_WavOut_Playback_AsynchCBData* cb_data_p =
    reinterpret_cast<struct Stream_Device_WavOut_Playback_AsynchCBData*> (dwInstance);
  ACE_ASSERT (cb_data_p);

  switch (uMsg)
  {
    case WOM_CLOSE:
    case WOM_OPEN:
      break;
    case WOM_DONE:
    {
      // sanity check(s)
      struct wavehdr_tag* wave_hdr_p =
        reinterpret_cast<struct wavehdr_tag*> (dwParam1);
      ACE_ASSERT (wave_hdr_p);
      //ACE_ASSERT (!dwParam2);
      ACE_Message_Block* message_block_p =
        reinterpret_cast<ACE_Message_Block*> (wave_hdr_p->dwUser);
      ACE_ASSERT (message_block_p);

      // step1: unprepare header
      MMRESULT result = waveOutUnprepareHeader (hwo,
                                                wave_hdr_p,
                                                sizeof (struct wavehdr_tag));
      if (unlikely (result != MMSYSERR_NOERROR))
      { char error_msg_a[BUFSIZ];
        waveOutGetErrorText (result, error_msg_a, BUFSIZ - 1);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to waveOutUnprepareHeader(): \"%s\", returning\n"),
                    ACE_TEXT (error_msg_a)));
        message_block_p->release ();
        ReleaseSemaphore (cb_data_p->lock, 1, NULL);
        return;
      } // end IF

      // step2: more data ?
      ACE_Message_Block* message_block_2 = message_block_p->cont ();
      if (unlikely (message_block_2))
      {
        // step2a: release data
        // *TODO*: the message block is a shallow copy; manipulating the buffer
        //         chain is not allowed, as it breaks consistency
        //         of the data for all downstream modules !
        message_block_p->cont (NULL);
        message_block_p->release (); message_block_p = NULL;

        // step2b: prepare header
        wave_hdr_p->lpData = message_block_2->rd_ptr ();
        wave_hdr_p->dwBufferLength = message_block_2->length ();
        wave_hdr_p->dwUser = reinterpret_cast<DWORD_PTR> (message_block_2);
        wave_hdr_p->dwFlags = 0;
        result = waveOutPrepareHeader (hwo,
                                       wave_hdr_p,
                                       sizeof (struct wavehdr_tag));
        if (unlikely (result != MMSYSERR_NOERROR))
        { char error_msg_a[BUFSIZ];
          waveOutGetErrorText (result, error_msg_a, BUFSIZ - 1);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to waveOutPrepareHeader(): \"%s\", returning\n"),
                      ACE_TEXT (error_msg_a)));
          message_block_2->release ();
          ReleaseSemaphore (cb_data_p->lock, 1, NULL);
          return;
        } // end IF

        // step2c: write to device
        result = waveOutWrite (hwo,
                               wave_hdr_p,
                               sizeof (struct wavehdr_tag));
        if (unlikely (result != MMSYSERR_NOERROR))
        { char error_msg_a[BUFSIZ];
          waveOutGetErrorText (result, error_msg_a, BUFSIZ - 1);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to waveOutWrite(): \"%s\", returning\n"),
                      ACE_TEXT (error_msg_a)));
          waveOutUnprepareHeader (hwo,
                                  wave_hdr_p,
                                  sizeof (struct wavehdr_tag));
          message_block_2->release ();
          ReleaseSemaphore (cb_data_p->lock, 1, NULL);
          return;
        } // end IF

        return;
      } // end IF

      // step3: release data
      message_block_p->release (); message_block_p = NULL;
      --cb_data_p->inFlightBuffers;
      cb_data_p->done = !cb_data_p->inFlightBuffers;

      // step4: get more data
      if (!ReleaseSemaphore (cb_data_p->lock, 1, NULL))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ReleaseSemaphore(): \"%m\", returning\n")));

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid message type (was: %u), returning\n"),
                  uMsg));
      break;
    }
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
 , header_ ()
 , lock_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WavOut_T::Stream_Dev_Target_WavOut_T"));

  lock_ = CreateSemaphore (NULL, 1, 1, NULL);
  if (unlikely (lock_ == NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CreateSemaphore(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  CBData_.done = false;
  CBData_.inFlightBuffers = 0;
  CBData_.lock = lock_;

  ACE_OS::memset (&header_, 0, sizeof (struct wavehdr_tag));
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
  if (likely (lock_))
    CloseHandle (lock_);
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

  // step0: retain handle to data
  // *TODO*: this ought to be DataMessageType::clone() (see above)
  ACE_Message_Block* message_block_p = message_inout->duplicate ();
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to DataMessageType::duplicate(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  // step1: wait for buffer
  DWORD result = WaitForSingleObject (lock_, INFINITE);
  if (unlikely (result != WAIT_OBJECT_0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to WaitForSingleObject(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  // step2: prepare header
  header_.lpData = message_block_p->rd_ptr ();
  header_.dwBufferLength = message_block_p->length ();
  header_.dwFlags = 0;
  header_.dwUser = reinterpret_cast<DWORD_PTR> (message_block_p);
  MMRESULT result_2 = waveOutPrepareHeader (handle_,
                                            &header_,
                                            sizeof (struct wavehdr_tag));
  if (unlikely (result_2 != MMSYSERR_NOERROR))
  { char error_msg_a[BUFSIZ];
    waveOutGetErrorText (result_2, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to waveOutPrepareHeader(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (error_msg_a)));
    ReleaseSemaphore (lock_, 1, NULL);
    message_block_p->release (); message_block_p = NULL;
    return;
  } // end IF
  result_2 = waveOutWrite (handle_,
                           &header_,
                           sizeof (struct wavehdr_tag));
  if (unlikely (result_2 != MMSYSERR_NOERROR))
  { char error_msg_a[BUFSIZ];
    waveOutGetErrorText (result_2, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to waveOutWrite(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (error_msg_a)));
    waveOutUnprepareHeader (handle_,
                            &header_,
                            sizeof (struct wavehdr_tag));
    ReleaseSemaphore (lock_, 1, NULL);
    message_block_p->release (); message_block_p = NULL;
    return;
  } // end IF
  ++CBData_.inFlightBuffers;
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
                                media_type_s);
      ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
      ACE_ASSERT (media_type_s.pbFormat);
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      ACE_ASSERT (waveformatex_p);

      DWORD flags_u = CALLBACK_FUNCTION;// |
                      //WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE |
                      //WAVE_FORMAT_DIRECT;
      result =
        waveOutOpen (&handle_,
                     inherited::configuration_->deviceIdentifier.identifier._id,
                     waveformatex_p,
                     reinterpret_cast<DWORD_PTR> (stream_dev_target_wavout_async_callback),
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

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // render remaining data
      while (!CBData_.done)
        ACE_OS::sleep (ACE_Time_Value (1, 0));

      result = waveOutClose (handle_);
      if (unlikely (result != MMSYSERR_NOERROR))
      { char error_msg_a[BUFSIZ];
        waveOutGetErrorText (result, error_msg_a, BUFSIZ - 1);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to waveOutClose(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (error_msg_a)));
      } // end IF
      break;
    }
    default:
      break;
  } // end SWITCH
}
