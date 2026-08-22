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
#include "stdafx.h"

#include "stream_lib_pipewire_tools.h"

#include "spa/param/audio/raw.h"
#include "spa/param/audio/raw-utils.h"
#include "spa/param/props.h"
#include "spa/pod/builder.h"

// #include "stream_dev_defines.h"

#include "stream_lib_alsa_tools.h"
#include "stream_lib_pipewire_defines.h"

//////////////////////////////////////////

struct acestream_lib_pipewire_cbdata
{
  struct spa_pod*   pod;
  struct pw_proxy*  proxy;
  struct pw_stream* stream;

  uint32_t          channels;
  float             volume;
};

int
acestream_lib_pipewire_free_proxy_cb (struct spa_loop* loop_in,
                                      bool async_in,
                                      uint32_t seq_in,
                                      const void* data_in,
                                      size_t size_in,
                                      void* userData_in)
{
  STREAM_TRACE (ACE_TEXT ("acestream_lib_pipewire_free_proxy_cb"));

  struct acestream_lib_pipewire_cbdata* cb_data_p =
    static_cast<struct acestream_lib_pipewire_cbdata*> (userData_in);
  ACE_ASSERT (cb_data_p);

  pw_proxy_destroy (cb_data_p->proxy); cb_data_p->proxy = NULL;

  return 0;
}

int
acestream_lib_pipewire_finalize_stream_cb (struct spa_loop* loop_in,
                                           bool async_in,
                                           uint32_t seq_in,
                                           const void* data_in,
                                           size_t size_in,
                                           void* userData_in)
{
  STREAM_TRACE (ACE_TEXT ("acestream_lib_pipewire_finalize_stream_cb"));

  struct acestream_lib_pipewire_cbdata* cb_data_p =
    static_cast<struct acestream_lib_pipewire_cbdata*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->stream);

  int result = pw_stream_set_active (cb_data_p->stream, false);
  if (unlikely (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pw_stream_set_active (%@, false), continuing\n"),
                cb_data_p->stream));

  result = pw_stream_disconnect (cb_data_p->stream);
  if (unlikely (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pw_stream_disconnect (%@), continuing\n"),
                cb_data_p->stream));

  pw_stream_destroy (cb_data_p->stream); cb_data_p->stream = NULL;

  return result == 0;
}

int
acestream_lib_pipewire_set_volume_cb (struct spa_loop* loop_in,
                                      bool async_in,
                                      uint32_t seq_in,
                                      const void* data_in,
                                      size_t size_in,
                                      void* userData_in)
{
  STREAM_TRACE (ACE_TEXT ("acestream_lib_pipewire_set_volume_cb"));

  struct acestream_lib_pipewire_cbdata* cb_data_p =
    static_cast<struct acestream_lib_pipewire_cbdata*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->channels <= SPA_AUDIO_MAX_CHANNELS);

  float volumes_a[SPA_AUDIO_MAX_CHANNELS];
  for (uint32_t i = 0; i < cb_data_p->channels; i++)
    volumes_a[i] = cb_data_p->volume;
  int result = pw_stream_set_control (cb_data_p->stream,
                                      SPA_PROP_channelVolumes,
                                      cb_data_p->channels,
                                      volumes_a,
                                      0);

  return result > 0;
}

//////////////////////////////////////////

void
Stream_MediaFramework_Pipewire_Tools::freeProxy (struct pw_loop* loop_in,
                                                 struct pw_proxy* proxy_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Pipewire_Tools::freeProxy"));

  static struct acestream_lib_pipewire_cbdata cb_data_s;
  cb_data_s.proxy = proxy_in;
  int result = pw_loop_invoke (loop_in,
                               acestream_lib_pipewire_free_proxy_cb,
                               0,
                               NULL,
                               0,
                               true,
                               &cb_data_s);
  ACE_UNUSED_ARG (result);
}

void
Stream_MediaFramework_Pipewire_Tools::finalizeStream (struct pw_loop* loop_in,
                                                      struct pw_stream* stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Pipewire_Tools::finalizeStream"));

  static struct acestream_lib_pipewire_cbdata cb_data_s;
  cb_data_s.stream = stream_in;

  int result = pw_loop_invoke (loop_in,
                               acestream_lib_pipewire_finalize_stream_cb,
                               0,
                               NULL,
                               0,
                               true,
                               &cb_data_s);
  ACE_UNUSED_ARG (result);
}

bool
Stream_MediaFramework_Pipewire_Tools::getVolumeControl (struct Stream_MediaFramework_ALSA_MediaType& format_in,
                                                        struct pw_thread_loop*& loop_inout,
                                                        struct pw_context*& context_out,
                                                        struct pw_core*& core_out,
                                                        struct pw_stream*& stream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Pipewire_Tools::getVolumeControl"));

  // sanity check(s)
  ACE_ASSERT (/*!loop_out && */!context_out && !core_out && !stream_out);

  struct pw_thread_loop* thread_loop_p = NULL;
  bool our_loop_b = (loop_inout == NULL);
  if (loop_inout)
    goto continue_;

  thread_loop_p = pw_thread_loop_new (NULL,
                                      NULL);
  if (unlikely (!loop_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pw_thread_loop_new(), aborting\n")));
    return false;
  } // end IF
  loop_inout = thread_loop_p;

continue_:
  ACE_ASSERT (loop_inout);
  context_out = pw_context_new (pw_thread_loop_get_loop (loop_inout),
                                NULL,
                                0);
  if (unlikely (!context_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pw_context_new(), aborting\n")));
    if (our_loop_b)
    {
      pw_thread_loop_destroy (thread_loop_p); loop_inout = NULL;
    } // end IF
    return false;
  } // end IF

  core_out = pw_context_connect (context_out,
                                 NULL,
                                 0);
  if (unlikely (!core_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pw_context_connect(), aborting\n")));
    pw_context_destroy (context_out); context_out = NULL;
    if (our_loop_b)
    {
      pw_thread_loop_destroy (thread_loop_p); loop_inout = NULL;
    } // end IF
    return false;
  } // end IF

  struct pw_properties* properties_p =
    pw_properties_new (PW_KEY_MEDIA_TYPE, ACE_TEXT_ALWAYS_CHAR ("Audio"),
                       PW_KEY_MEDIA_CATEGORY, ACE_TEXT_ALWAYS_CHAR ("Playback"),
                       PW_KEY_MEDIA_ROLE, ACE_TEXT_ALWAYS_CHAR ("Music"),
                       ACE_TEXT_ALWAYS_CHAR ("node.passive"), ACE_TEXT_ALWAYS_CHAR ("false"),
                       NULL);
  ACE_ASSERT (properties_p);
  if (our_loop_b)
    pw_thread_loop_lock (loop_inout);
  stream_out =
    pw_stream_new (core_out,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_PIPEWIRE_PLAYBACK_STREAM_NAME_DEFAULT),
                   properties_p);
  if (unlikely (!stream_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pw_stream_new(), aborting\n")));
    if (our_loop_b)
      pw_thread_loop_unlock (loop_inout);
    pw_core_disconnect (core_out); core_out = NULL;
    pw_context_destroy (context_out); context_out = NULL;
    if (our_loop_b)
    {
      pw_thread_loop_destroy (thread_loop_p); loop_inout = NULL;
    } // end IF
    return false;
  } // end IF

  uint8_t buffer_a[4096];
  struct spa_pod_builder POD_builder_s =
    SPA_POD_BUILDER_INIT (buffer_a, sizeof (uint8_t[4096]));
  const struct spa_pod* parameters_a[2];
  struct spa_audio_info_raw audio_info_raw_s;
  ACE_OS::memset (&audio_info_raw_s, 0, sizeof (struct spa_audio_info_raw));
  audio_info_raw_s.channels = format_in.channels;
  audio_info_raw_s.format =
    Stream_MediaFramework_ALSA_Tools::ALSAFormatToPipewireFormat (format_in.format);
  audio_info_raw_s.position[0] = SPA_AUDIO_CHANNEL_FL;
  audio_info_raw_s.position[1] = SPA_AUDIO_CHANNEL_FR;
  audio_info_raw_s.rate = format_in.rate;
  parameters_a[0] = spa_format_audio_raw_build (&POD_builder_s,
                                                SPA_PARAM_EnumFormat,
                                                &audio_info_raw_s);
  ACE_ASSERT (parameters_a[0]);

  uint8_t props_buffer_a[1024];
  struct spa_pod_builder props_builder =
    SPA_POD_BUILDER_INIT (props_buffer_a, sizeof(uint8_t[1024]));
  parameters_a[1] =
    (struct spa_pod *)spa_pod_builder_add_object(&props_builder,
                                                 SPA_TYPE_OBJECT_Props, SPA_PARAM_Props,
                                                 0);
  ACE_ASSERT (parameters_a[1]);

  enum pw_stream_flags stream_flags_e =
    static_cast<enum pw_stream_flags> (PW_STREAM_FLAG_AUTOCONNECT |
                                       PW_STREAM_FLAG_MAP_BUFFERS |
                                       PW_STREAM_FLAG_ASYNC);
  int result = pw_stream_connect (stream_out,
                                  PW_DIRECTION_OUTPUT,
                                  PW_ID_ANY,
                                  stream_flags_e,
                                  parameters_a, 2);
  ACE_ASSERT (result >= 0);
  if (our_loop_b)
    pw_thread_loop_unlock (loop_inout);

  return true;
}

void
Stream_MediaFramework_Pipewire_Tools::freeVolumeControl (struct pw_thread_loop*& loop_inout,
                                                         struct pw_context*& context_inout,
                                                         struct pw_core*& core_inout,
                                                         struct pw_stream*& stream_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Pipewire_Tools::freeVolumeControl"));

  pw_stream_destroy (stream_inout); stream_inout = NULL;
  pw_core_disconnect (core_inout); core_inout = NULL;
  pw_context_destroy (context_inout); context_inout = NULL;
  if (loop_inout)
  {
    pw_thread_loop_destroy (loop_inout); loop_inout = NULL;
  } // end IF
}

bool
Stream_MediaFramework_Pipewire_Tools::setVolumeLevel (struct pw_stream* stream_in,
                                                      uint32_t channels_in,
                                                      float level_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Pipewire_Tools::setVolumeLevel"));

  // sanity check(s)
  ACE_ASSERT (stream_in);

  // pipewire typically uses cubic scaling for perception
  float actual_level_f = level_in * level_in * level_in;

  float volumes_a[SPA_AUDIO_MAX_CHANNELS];
  for (uint32_t i = 0; i < channels_in; i++)
    volumes_a[i] = actual_level_f;
  uint8_t buffer_a[1024];
  struct spa_pod_builder b = SPA_POD_BUILDER_INIT (buffer_a, sizeof (uint8_t[1024]));
  struct spa_pod* parameters_p =
    (struct spa_pod*)spa_pod_builder_add_object (&b,
                                                 SPA_TYPE_OBJECT_Props, SPA_PARAM_Props,
                                                 SPA_PROP_channelVolumes, SPA_POD_Array (sizeof (float), SPA_TYPE_Float, channels_in, volumes_a));
  ACE_ASSERT (parameters_p);
  int result;
  result = pw_stream_set_param (stream_in,
                                SPA_PARAM_Props,
                                parameters_p);
  // result = pw_stream_set_control (stream_in,
  //                                 SPA_PROP_volume,
  //                                 channels_in, volumes_a);
  ACE_ASSERT (result >= 0);

  return result == 0;
}

bool
Stream_MediaFramework_Pipewire_Tools::setVolumeLevel (struct pw_loop* loop_in,
                                                      //struct pw_proxy* proxy_in,
                                                      struct pw_stream* stream_in,
                                                      uint32_t channels_in,
                                                      float level_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Pipewire_Tools::setVolumeLevel"));

  // pipewire typically uses cubic scaling for perception
  float actual_level_f = level_in * level_in * level_in;

  static struct acestream_lib_pipewire_cbdata cb_data_s;
  // cb_data_s.proxy = proxy_in;
  cb_data_s.stream = stream_in;
  cb_data_s.channels = channels_in;
  cb_data_s.volume = actual_level_f;
  int result = pw_loop_invoke (loop_in,
                               acestream_lib_pipewire_set_volume_cb,
                               0,
                               NULL,
                               0,
                               true,
                               &cb_data_s);

  return result == 0;
}
