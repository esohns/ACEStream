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
#include "spa/param/props.h"
#include "spa/pod/builder.h"

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
