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

#include "projectM-4/projectM.h"

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "common_timer_manager_common.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_directsound_tools.h"
#else
#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_vis_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Module_Vis_ProjectM_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             MediaType>::Stream_Module_Vis_ProjectM_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_ProjectM_T::Stream_Module_Vis_ProjectM_T"));

}



template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Module_Vis_ProjectM_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             MediaType>::initialize (const ConfigurationType& configuration_in,
                                                     Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_ProjectM_T::initialize"));

  if (inherited::isInitialized_)
  {
  } // end IF

  // sanity check(s)
  ACE_ASSERT (configuration_in.projectMConfiguration);

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Module_Vis_ProjectM_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_ProjectM_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->projectMConfiguration);
  ACE_ASSERT (inherited::configuration_->projectMConfiguration->handle);

  // projectm_pcm_add_float (inherited::configuration_->projectMConfiguration->handle,
  //                         reinterpret_cast<float*> (message_inout->rd_ptr ()),
  //                         message_inout->length () / sizeof (float),
  //                         PROJECTM_STEREO); // #channels
  projectm_pcm_add_int16 (inherited::configuration_->projectMConfiguration->handle,
                          reinterpret_cast<int16_t*> (message_inout->rd_ptr ()),
                          message_inout->length () / sizeof (int16_t),
                          PROJECTM_STEREO); // #channels
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Module_Vis_ProjectM_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_ProjectM_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      unsigned int bytes_per_sample_i;
      bool is_signed_sample_format_b, is_integral_sample_format_b;
      unsigned int channels_i = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_ASSERT (media_type_s.majortype == MEDIATYPE_Audio);
      ACE_ASSERT (media_type_s.formattype == FORMAT_WaveFormatEx);
      struct tWAVEFORMATEX* audio_info_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      bytes_per_sample_i = audio_info_p->wBitsPerSample / 8;
      is_signed_sample_format_b = audio_info_p->wBitsPerSample > 8; // signed if > 8 bit/sample
      is_integral_sample_format_b =
        !Stream_MediaFramework_DirectSound_Tools::isFloat (*audio_info_p);
      channels_i = audio_info_p->nChannels;
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      bytes_per_sample_i = snd_pcm_format_width (media_type_s.format) / 8;
      is_signed_sample_format_b =
        snd_pcm_format_signed (media_type_s.format) ? true : false;
      is_integral_sample_format_b =
        snd_pcm_format_float (media_type_s.format) ? false : true;
      channels_i = media_type_s.channels;
#endif // ACE_WIN32 || ACE_WIN64
      if (unlikely ((bytes_per_sample_i != 2) ||
                    (channels_i != 2        ) ||
                    !is_integral_sample_format_b))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: unsupported audio format, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
      break;
    default:
      break;
  } // end SWITCH
}
