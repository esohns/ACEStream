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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "strmif.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_directsound_tools.h"
#else
#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename ValueType>
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType>::Stream_Module_Vis_Console_Audio_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
 , channels_ (0)
 , frameSize_ (0)
 , iterator_ (NULL)
// , minMax_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::Stream_Module_Vis_Console_Audio_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename ValueType>
bool
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType>::initialize (const ConfigurationType& configuration_in,
                                                          Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::initialize"));

  if (inherited::isInitialized_)
  {
    channels_ = 0;
    frameSize_ = 0;
    // minMax_ = {0, 0};
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
          typename MediaType,
          typename ValueType>
void
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType>::handleDataMessage (DataMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // *NOTE*: integral types need normalization --> compute factor once
  static float factor_f =
    std::is_integral<ValueType>::value ? 1.0f / static_cast<float> (std::numeric_limits<ValueType>::max ()) : 1.0f;

  iterator_.buffer_ = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  ACE_UINT32 frames_i = message_inout->length () / frameSize_;

  /* move cursor up */
  // ACE_OS::fprintf (stdout, "%c[%dA", 0x1b, channels_ + 1);
  // ACE_OS::fprintf (stdout, "captured %d frames\n", samples_i / channels_);
  ACE_OS::fprintf (stdout, "%c[%dA", 0x1b, channels_);

  float max_f;
  uint32_t level_i;
  for (ACE_UINT32 c = 0; c < channels_; ++c)
  {
    max_f = 0.0f;
    for (ACE_UINT32 n = 0; n < frames_i; ++n)
      max_f = fmaxf (max_f, fabsf (iterator_.get (n, c) * factor_f));

    level_i = (uint32_t)fminf (fmaxf (max_f * 30.0f, 0.f), 39.f);

    ACE_OS::fprintf (stdout, "channel %d: |%*s%*s| peak:%f\n", c, level_i + 1, "*", 40 - level_i, "", max_f);
  } // end FOR
  ACE_OS::fflush (stdout);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename ValueType>
void
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::handleSessionMessage"));

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
      ACE_ASSERT (media_type_s.majortype == MEDIATYPE_Audio);
      ACE_ASSERT (media_type_s.formattype == FORMAT_WaveFormatEx);
      struct tWAVEFORMATEX* audio_info_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      channels_ = audio_info_p->nChannels;
      frameSize_ = audio_info_p->nChannels * (audio_info_p->wBitsPerSample / 8);
      ACE_ASSERT (frameSize_ == audio_info_p->nBlockAlign);
      iterator_.initialize (frameSize_,
                            audio_info_p->wBitsPerSample / 8,
                            Stream_MediaFramework_DirectSound_Tools::isFloat (*audio_info_p),
                            audio_info_p->wBitsPerSample > 8, // signed if > 8 bit/sample
                            0x0123); // all Win32 sound data is little endian
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      channels_ = media_type_s.channels;
      frameSize_ =
        (snd_pcm_format_width (media_type_s.format) / 8) * media_type_s.channels;
      iterator_.initialize (frameSize_,
                            snd_pcm_format_width (media_type_s.format) / 8,
                            snd_pcm_format_float (media_type_s.format) ? true : false,
                            snd_pcm_format_signed (media_type_s.format) ? true : false,
                            snd_pcm_format_little_endian (media_type_s.format) ? 0x0123 : 0x3210);
#endif // ACE_WIN32 || ACE_WIN64

      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
      break;
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
          typename MediaType,
          typename ValueType>
void
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::toggle"));

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}
