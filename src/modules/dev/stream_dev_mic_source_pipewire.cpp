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

#include "stream_dev_mic_source_pipewire.h"

#include "common_file_tools.h"

#include "stream_dev_defines.h"

const char libacestream_default_dev_mic_source_pipewire_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_PIPEWIRE_DEFAULT_NAME_STRING);

void
acestream_dev_mic_pw_on_client_event_info_cb (void* userData_in,
                                              const struct pw_client_info* info_in)
{
  STREAM_TRACE (ACE_TEXT ("acestream_dev_mic_pw_on_client_event_info_cb"));

  struct Stream_Device_Pipewire_Capture_CBData* cb_data_p =
    static_cast<struct Stream_Device_Pipewire_Capture_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);

  // ACE_DEBUG ((LM_DEBUG,
  //             ACE_TEXT ("client id: %u\n"),
  //             info_in->id));
  // const struct spa_dict_item* item_p;
  // spa_dict_for_each (item_p, info_in->props)
  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("%s: %s\n"),
  //               ACE_TEXT (item_p->key), ACE_TEXT (item_p->value)));
}

void
acestream_dev_mic_pw_on_device_event_info_cb (void* userData_in,
                                              const struct pw_device_info* info_in)
{
  STREAM_TRACE (ACE_TEXT ("acestream_dev_mic_pw_on_device_event_info_cb"));

  struct Stream_Device_Pipewire_Capture_CBData* cb_data_p =
    static_cast<struct Stream_Device_Pipewire_Capture_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->device);

  if ((info_in->change_mask & PW_DEVICE_CHANGE_MASK_PARAMS) == PW_DEVICE_CHANGE_MASK_PARAMS)
  {
    for (uint32_t i = 0; i != info_in->n_params; i++)
    {
      struct spa_param_info& param_s = info_in->params[i];
      if (param_s.id == SPA_PARAM_Route)
      {
        if ((param_s.flags & SPA_PARAM_INFO_READWRITE) == SPA_PARAM_INFO_READWRITE)
          pw_device_enum_params ((struct pw_device*)cb_data_p->device,
                                 0,
                                 SPA_PARAM_Route,
                                 0,
                                 UINT32_MAX,
                                 NULL);
        else
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("unable to enumerate route param for capture device as the param does not have read+write permissions\n")));
        break;
      } // end IF
    } // end FOR
  } // end IF
}

void
acestream_dev_mic_pw_on_device_event_param_cb (void* userData_in,
                                               int seq_in,
                                               uint32_t id_in,
                                               uint32_t index_in,
                                               uint32_t next_in,
                                               const struct spa_pod* parameters_in)
{
  STREAM_TRACE (ACE_TEXT ("acestream_dev_mic_pw_on_device_event_param_cb"));

  // sanity check(s)
  struct Stream_Device_Pipewire_Capture_CBData* cb_data_p =
    static_cast<struct Stream_Device_Pipewire_Capture_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  if (!parameters_in || id_in != SPA_PARAM_Route)
    return;
  // In many simple setups, the first (index 0) or only route is the one we want.
  // A more robust method would check `active` or metadata.
  static bool acestream_dev_mic_pw_on_device_event_params_found_b = false;
  if (acestream_dev_mic_pw_on_device_event_params_found_b)
    return;

  // struct spa_pod_object* pod_object_p = (struct spa_pod_object*)parameters_in;
  // struct spa_pod_prop* pod_prop_p;

  // check if this route is currently active (a common property of the default route)
  // bool active = false;
  // SPA_POD_OBJECT_FOREACH (pod_object_p, pod_prop_p)
  // { ACE_ASSERT (pod_prop_p);
  //   if (pod_prop_p->key == SPA_PARAM_ROUTE_props)
  //   {
  //     struct spa_pod* props_struct_p = (struct spa_pod*)&pod_prop_p->value;
  //     ACE_ASSERT (props_struct_p);
  //     struct spa_pod_prop* pod_prop_2;
  //     SPA_POD_STRUCT_FOREACH (props_struct_p, pod_prop_2)
  //     { ACE_ASSERT (pod_prop_2);
  //       if (pod_prop_2->key == SPA_PROP_DEVICE_ACTIVE)
  //         spa_pod_get_bool (&pod_prop_2->value, &active);
  //     } // end foreach
  //   } // end IF
  // } // end foreach

  struct spa_pod_parser parser_s;
  ACE_OS::memset (&parser_s, 0, sizeof (struct spa_pod_parser));
  spa_pod_parser_pod (&parser_s, parameters_in);
  uint32_t id = SPA_PARAM_Route;
  spa_pod_parser_get_object (&parser_s,
                             SPA_TYPE_OBJECT_ParamRoute, &id,
                             SPA_PARAM_ROUTE_device, SPA_POD_Int (&cb_data_p->routeDevice),
                             SPA_PARAM_ROUTE_index, SPA_POD_Int (&cb_data_p->routeIndex));

  // if (index_in == 0 /* or check if active */)
  // {
  //   pod_prop_p = spa_pod_prop_first (&pod_object_p->body);
  //   spa_pod_get_int (&pod_prop_p->value, &cb_data_p->routeIndex);
  //   pod_prop_p = spa_pod_prop_next (pod_prop_p);
  //   spa_pod_get_int (&pod_prop_p->value, &cb_data_p->routeDevice);
    acestream_dev_mic_pw_on_device_event_params_found_b = true;

    // ACE_ASSERT (cb_data_p->loop);
    // pw_main_loop_quit (cb_data_p->loop);
  // } // end IF
}

void
acestream_dev_mic_pw_on_registry_event_global_cb (void* userData_in,
                                                  uint32_t id_in,
                                                  uint32_t permissions_in,
                                                  const char* type_in,
                                                  uint32_t version_in,
                                                  const struct spa_dict* properties_in)
{
  STREAM_TRACE (ACE_TEXT ("acestream_dev_mic_pw_on_registry_event_global_cb"));

  // sanity check(s)
  struct Stream_Device_Pipewire_Capture_CBData* cb_data_p =
    static_cast<struct Stream_Device_Pipewire_Capture_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (type_in);

  if (ACE_OS::strcmp (type_in, PW_TYPE_INTERFACE_Node) == 0)
  {
    if (cb_data_p->nodeId)
      return;

    const char* media_class_p =
      spa_dict_lookup (properties_in, PW_KEY_MEDIA_CLASS);
    // *NOTE*: this relies on the session manager using standard properties
    if (media_class_p && ACE_OS::strcmp (media_class_p, ACE_TEXT_ALWAYS_CHAR ("Audio/Source")) == 0)
    {
      const char* node_name_p =
        spa_dict_lookup (properties_in, PW_KEY_NODE_NAME);
      if (node_name_p && ACE_OS::strcmp (node_name_p, cb_data_p->nodeName.c_str ()) == 0)
      { ACE_ASSERT (cb_data_p->registry);
        cb_data_p->node =
          (struct pw_proxy*)pw_registry_bind (cb_data_p->registry,
                                              id_in, type_in,
                                              PW_VERSION_NODE,
                                              0);
        ACE_ASSERT (cb_data_p->node);
        cb_data_p->nodeId = id_in;

        const char* node_description_p =
          spa_dict_lookup (properties_in, PW_KEY_NODE_DESCRIPTION);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("found audio source node (\"%s\") as \"%s\" --> %u\n"),
                    ACE_TEXT (node_name_p),
                    ACE_TEXT (node_description_p),
                    id_in));
      } // end IF
    } // end IF
  } // end IF
  else if (ACE_OS::strcmp (type_in, PW_TYPE_INTERFACE_Client) == 0)
  {
    if (cb_data_p->client)
      return;

    const char* application_name_p =
      spa_dict_lookup (properties_in, PW_KEY_APP_NAME);
    if (application_name_p && ACE_OS::strcmp (application_name_p, Common_File_Tools::executable.c_str ()) == 0)
    { ACE_ASSERT (cb_data_p->registry);
      cb_data_p->client =
        (struct pw_proxy*)pw_registry_bind (cb_data_p->registry,
                                            id_in, type_in,
                                            PW_VERSION_CLIENT,
                                            0);
      ACE_ASSERT (cb_data_p->client);
      pw_client_add_listener ((struct pw_client*)cb_data_p->client,
                              &cb_data_p->clientListener,
                              &cb_data_p->clientEvents,
                              userData_in);
    } // end IF
  } // end ELSE IF
  else if (ACE_OS::strcmp (type_in, PW_TYPE_INTERFACE_Device) == 0)
  {
    if (cb_data_p->device)
      return;

    const struct spa_dict_item* item_p;
    spa_dict_for_each (item_p, properties_in)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: %s\n"),
                  ACE_TEXT (item_p->key), ACE_TEXT (item_p->value)));

    const char* device_name_p =
      spa_dict_lookup (properties_in, PW_KEY_DEVICE_NAME);

    // ACE_ASSERT (cb_data_p->registry);
    // cb_data_p->device =
    //   (struct pw_proxy*)pw_registry_bind (cb_data_p->registry,
    //                                       id_in, type_in,
    //                                       PW_VERSION_DEVICE,
    //                                       0);
    // ACE_ASSERT (cb_data_p->device);
    // pw_device_add_listener (cb_data_p->device,
    //                         &cb_data_p->deviceListener,
    //                         &cb_data_p->deviceEvents,
    //                         userData_in);
  } // end ELSE IF
}

void
acestream_dev_mic_pw_on_stream_param_changed_cb (void* userData_in,
                                                 uint32_t id_in,
                                                 const struct spa_pod* parameters_in)
{
  STREAM_TRACE (ACE_TEXT ("acestream_dev_mic_pw_on_stream_param_changed_cb"));

  // sanity check(s)
  struct Stream_Device_Pipewire_Capture_CBData* cb_data_p =
    static_cast<struct Stream_Device_Pipewire_Capture_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  /* NULL means to clear the format */
  if (parameters_in == NULL || id_in != SPA_PARAM_Format)
    return;
  int result = spa_format_parse (parameters_in,
                                 &cb_data_p->audioFormat.media_type,
                                 &cb_data_p->audioFormat.media_subtype);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to spa_format_parse(%@), returning\n"),
                parameters_in));
    return;
  } // end IF
  /* only accept raw audio */
  if (unlikely (cb_data_p->audioFormat.media_type != SPA_MEDIA_TYPE_audio ||
                cb_data_p->audioFormat.media_subtype != SPA_MEDIA_SUBTYPE_raw))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid media (sub-)type (was: %u|%u), returning\n"),
                cb_data_p->audioFormat.media_type,
                cb_data_p->audioFormat.media_subtype));
    return;
  } // end IF
  result = spa_format_audio_raw_parse (parameters_in,
                                       &cb_data_p->audioFormat.info.raw);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("capturing %d channel(s) @ %uHz\n"),
              cb_data_p->audioFormat.info.raw.channels,
              cb_data_p->audioFormat.info.raw.rate));
}

void
acestream_dev_mic_pw_on_process_cb (void* userData_in)
{
  // STREAM_TRACE (ACE_TEXT ("acestream_dev_mic_pw_on_process_cb"));

  // sanity check(s)
  struct Stream_Device_Pipewire_Capture_CBData* cb_data_p =
    static_cast<struct Stream_Device_Pipewire_Capture_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->allocator);
  ACE_ASSERT (cb_data_p->allocatorConfiguration);
  ACE_ASSERT (cb_data_p->statistic);
  ACE_ASSERT (cb_data_p->stream);

  struct pw_buffer* pw_buffer_p;
  struct spa_buffer* spa_buffer_p;
  uint8_t* samples_p;
  ACE_Message_Block* message_block_p = NULL;
  uint32_t index_i = 0;
  int result;
  uint32_t available_frames_i, frames_to_copy_i;

  pw_buffer_p = pw_stream_dequeue_buffer (cb_data_p->stream);
  if (unlikely (!pw_buffer_p))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to pw_stream_dequeue_buffer(): %m, returning\n")));
    return;
  } // end IF
  spa_buffer_p = pw_buffer_p->buffer;
  ACE_ASSERT (spa_buffer_p);
  samples_p = static_cast<uint8_t*> (spa_buffer_p->datas[0].data);
  if (unlikely (!samples_p))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("no sample data, returning\n")));
    goto continue_;
  } // end IF

  available_frames_i =
    spa_buffer_p->datas[0].chunk->size / cb_data_p->frameSize;
  while (available_frames_i)
  {
    if (likely (cb_data_p->allocator))
    {
      try {
        message_block_p =
          static_cast<ACE_Message_Block*> (cb_data_p->allocator->malloc (cb_data_p->allocatorConfiguration->defaultBufferSize));
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                    cb_data_p->allocatorConfiguration->defaultBufferSize));
        message_block_p = NULL;
      }
    } // end IF
    else
      ACE_NEW_NORETURN (message_block_p,
                        ACE_Message_Block (cb_data_p->allocatorConfiguration->defaultBufferSize));
    if (unlikely (!message_block_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, aborting\n")));
      goto continue_;
    } // end IF

    frames_to_copy_i = message_block_p->space () / cb_data_p->frameSize;
    frames_to_copy_i = std::min (frames_to_copy_i, available_frames_i);
    result = message_block_p->copy (reinterpret_cast<char*> (&samples_p[index_i]),
                                    cb_data_p->frameSize * frames_to_copy_i);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));
      message_block_p->release ();
      goto continue_;
    } // end IF
    index_i += cb_data_p->frameSize * frames_to_copy_i;
    available_frames_i -= frames_to_copy_i;
    cb_data_p->statistic->capturedFrames += frames_to_copy_i;

    result = cb_data_p->queue->enqueue_tail (message_block_p,
                                             NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue_Base::enqueue_tail(): \"%m\", returning\n")));
      message_block_p->release ();
      goto continue_;
    } // end IF
    message_block_p = NULL;
  } // end WHILE

continue_:
  result = pw_stream_queue_buffer (cb_data_p->stream,
                                   pw_buffer_p);
  ACE_ASSERT (result == 0);
}
