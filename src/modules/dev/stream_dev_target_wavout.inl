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
#include "ace/Message_Queue.h"

#include "common_file_tools.h"
#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

//#include "stream_dec_defines.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

static void CALLBACK
stream_dev_target_wavout_async_callback (HWAVEOUT  hwo,
                                         UINT      uMsg,
                                         DWORD_PTR dwInstance,
                                         DWORD_PTR dwParam1,
                                         DWORD_PTR dwParam2)
{
  switch (uMsg)
  {
    case WOM_CLOSE:
    case WOM_OPEN:
      break;
    case WOM_DONE:
    {
      // sanity check(s)
      ACE_ASSERT (dwInstance);
      struct Stream_Device_WavOut_Playback_AsynchCBData* cb_data_p =
        reinterpret_cast<struct Stream_Device_WavOut_Playback_AsynchCBData*> (dwInstance);
      ACE_ASSERT (dwParam1);
      struct wavehdr_tag* wave_hdr_p = reinterpret_cast<struct wavehdr_tag*> (dwParam1);
      //ACE_ASSERT (!dwParam2);      
      MMRESULT result = waveOutUnprepareHeader (hwo,
                                                wave_hdr_p,
                                                sizeof (struct wavehdr_tag));
      ACE_ASSERT (result == MMSYSERR_NOERROR);
      ACE_ASSERT (wave_hdr_p->dwUser);
      ACE_Message_Block* message_block_p =
        reinterpret_cast<ACE_Message_Block*> (wave_hdr_p->dwUser);
      message_block_p->release ();
      delete wave_hdr_p;
      --cb_data_p->inFlightBuffers;

      ACE_ASSERT (cb_data_p->queue);
      cb_data_p->done =
        (cb_data_p->queue->is_empty () && !cb_data_p->inFlightBuffers);
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
          typename SessionIdType,
          typename SessionDataType>
Stream_Dev_Target_WavOut_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionIdType,
                           SessionDataType>::Stream_Dev_Target_WavOut_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , CBData_ ()
 , handle_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WavOut_T::Stream_Dev_Target_WavOut_T"));

  CBData_.done = false;
  CBData_.inFlightBuffers = 0;
  CBData_.queue = inherited::msg_queue ();
  ACE_ASSERT (CBData_.queue);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
Stream_Dev_Target_WavOut_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionIdType,
                         SessionDataType>::~Stream_Dev_Target_WavOut_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WavOut_T::~Stream_Dev_Target_WavOut_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
bool
Stream_Dev_Target_WavOut_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionIdType,
                         SessionDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WavOut_T::initialize"));

  bool result_2 = inherited::initialize (configuration_in,
                                         allocator_in);
  if (!result_2)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));

  return result_2;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
void
Stream_Dev_Target_WavOut_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionIdType,
                         SessionDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WavOut_T::handleDataMessage"));

  // sanity check(s)
  if (unlikely (!handle_))
    return;

  ACE_Message_Block* message_block_p = message_inout;
  message_block_p = message_inout->duplicate ();
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to DataMessageType::duplicate(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF
  ACE_ASSERT (!message_block_p->cont ());

  struct wavehdr_tag* wave_hdr_p = NULL;
  ACE_NEW_NORETURN (wave_hdr_p,
                    struct wavehdr_tag ());
  if (unlikely (!wave_hdr_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to allocate wave header: \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_block_p->release ();
    return;
  } // end IF
  ACE_OS::memset (wave_hdr_p, 0, sizeof (struct wavehdr_tag));
  wave_hdr_p->lpData = message_block_p->rd_ptr ();
  wave_hdr_p->dwBufferLength = message_block_p->length ();
  //wave_hdr_p->dwBytesRecorded = 0;
  wave_hdr_p->dwUser = reinterpret_cast<DWORD_PTR> (message_block_p);
  //wave_hdr_p->dwFlags = 0;
  MMRESULT result = waveOutPrepareHeader (handle_,
                                          wave_hdr_p,
                                          sizeof (struct wavehdr_tag));
  if (unlikely (result != MMSYSERR_NOERROR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to waveOutPrepareHeader(): \"%s\", returning\n"),
                inherited::mod_->name (),
                Common_Error_Tools::errorToString (result, true, false)));
    message_block_p->release ();
    delete wave_hdr_p;
    return;
  } // end IF
  result = waveOutWrite (handle_,
                         wave_hdr_p,
                         sizeof (struct wavehdr_tag));
  if (unlikely (result != MMSYSERR_NOERROR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to waveOutWrite(): \"%s\", returning\n"),
                inherited::mod_->name (),
                Common_Error_Tools::errorToString (result, true, false)));
    message_block_p->release ();
    delete wave_hdr_p;
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
          typename SessionIdType,
          typename SessionDataType>
void
Stream_Dev_Target_WavOut_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionIdType,
                         SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WavOut_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      ACE_ASSERT (inherited::sessionData_);
      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->getR ());
      ACE_ASSERT (!session_data_r.formats.empty ());
      struct _AMMediaType& media_type_r =
          session_data_r.formats.front ();
      ACE_ASSERT (InlineIsEqualGUID (media_type_r.formattype, FORMAT_WaveFormatEx));
      ACE_ASSERT (media_type_r.pbFormat);
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_r.pbFormat);
      ACE_ASSERT (waveformatex_p);
      DWORD flags_u = CALLBACK_FUNCTION                        |
                      WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE |
                      WAVE_FORMAT_DIRECT;
      MMRESULT result =
        waveOutOpen (&handle_,
                     WAVE_MAPPER,
                     waveformatex_p,
                     reinterpret_cast<DWORD_PTR> (stream_dev_target_wavout_async_callback),
                     reinterpret_cast<DWORD_PTR> (&CBData_),
                     flags_u);
      if (result != MMSYSERR_NOERROR)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to waveOutOpen(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    Common_Error_Tools::errorToString (result, true, false)));
        goto error;
      } // end IF

//continue_:
      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      while (!CBData_.done)
        ACE_OS::sleep (ACE_Time_Value (1, 0));

      if (inherited::thr_count_)
        inherited::stop (false); // wait ?

      MMRESULT result = waveOutClose (handle_);
      if (result != MMSYSERR_NOERROR)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to waveOutClose(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    Common_Error_Tools::errorToString (result, true, false)));

      break;
    }
    default:
      break;
  } // end SWITCH
}
