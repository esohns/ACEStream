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

  pw_proxy_destroy (cb_data_p->proxy);

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

  int result = pw_stream_disconnect (cb_data_p->stream);
  pw_stream_destroy (cb_data_p->stream);

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

  // int result = pw_node_set_param ((struct pw_node*)cb_data_p->proxy,
  //                                 SPA_PARAM_Route, // parameter type we are setting
  //                                 0,               // flags
  //                                 cb_data_p->pod);

  float volumes_a[SPA_AUDIO_MAX_CHANNELS];
  for (unsigned int i = 0; i < SPA_AUDIO_MAX_CHANNELS; i++)
    volumes_a[i] = cb_data_p->volume;
  int result = pw_stream_set_control (cb_data_p->stream,
                                      SPA_PROP_channelVolumes,
                                      2,
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
                                                      float level_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Pipewire_Tools::setVolumeLevel"));

  // static uint8_t PODBuffer_a[BUFSIZ];
  // struct spa_pod_builder builder_s =
  //   SPA_POD_BUILDER_INIT (PODBuffer_a, sizeof (uint8_t[BUFSIZ]));

  // pipewire typically uses cubic scaling for perception
  float actual_level_f = level_in * level_in * level_in;
  // float volumes_a[2] = {actual_level_f, actual_level_f};

  // struct spa_pod* pod_p =
  //   (struct spa_pod*)spa_pod_builder_add_object (&builder_s,
  //                                                SPA_TYPE_OBJECT_Props,
  //                                                SPA_PARAM_Props,
  //                                                SPA_PROP_channelVolumes, SPA_POD_Array (sizeof (float), SPA_TYPE_Float, 2, volumes_a));
  // ACE_ASSERT (pod_p);

  // uint32_t version_i = 0;
  // ACE_ASSERT (ACE_OS::strcmp (pw_proxy_get_type (proxy_in, &version_i), PW_TYPE_INTERFACE_Node) == 0);

  static struct acestream_lib_pipewire_cbdata cb_data_s;
  // cb_data_s.proxy = proxy_in;
  // cb_data_s.pod = pod_p;
  cb_data_s.stream = stream_in;
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
