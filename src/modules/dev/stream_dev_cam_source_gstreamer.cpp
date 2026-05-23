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

#include "stream_dev_cam_source_gstreamer.h"

#include "gst/app/gstappsink.h"

#include "stream_dev_defines.h"

const char libacestream_default_dev_cam_source_gstreamer_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_GSTREAMER_DEFAULT_NAME_STRING);

//////////////////////////////////////////

gboolean
acestream_dev_cam_source_gstreamer_bus_cb (GstBus* bus_in,
                                           GstMessage* message_in,
                                           gpointer data_in)
{
  STREAM_TRACE (ACE_TEXT ("acestream_dev_cam_source_gstreamer_bus_cb"));

  // sanity check(s)
  struct ACEStream_Device_Cam_Source_GStreamer_CBData* cb_data_p =
    static_cast<struct ACEStream_Device_Cam_Source_GStreamer_CBData*> (data_in);
  ACE_ASSERT (cb_data_p);

  switch (GST_MESSAGE_TYPE (message_in))
  {
    case GST_MESSAGE_EOS:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("end of stream\n")));

      g_main_loop_quit (cb_data_p->loop);
     
      break;
    }
    case GST_MESSAGE_ERROR:
    {
      gchar*  debug_p = NULL;
      GError* error_p = NULL;
      gst_message_parse_error (message_in, &error_p, &debug_p);
      g_free (debug_p);

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("error: \"%s\", aborting\n"),
                  ACE_TEXT (error_p->message)));
      g_error_free (error_p);

      g_main_loop_quit (cb_data_p->loop);

      break;
    }
    default:
      break;
  } // end SWITCH

  return TRUE;
}

GstFlowReturn
acestream_dev_cam_source_gstreamer_new_sample_cb (GstElement* sink_in,
                                                  gpointer userData_in)
{
  // sanity check(s)
  struct ACEStream_Device_Cam_Source_GStreamer_CBData* cb_data_p =
    static_cast<struct ACEStream_Device_Cam_Source_GStreamer_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->allocator);
  ACE_ASSERT (cb_data_p->queue);
  // ACE_ASSERT (cb_data_p->sessionId);

  GstSample* sample_p = gst_app_sink_pull_sample (GST_APP_SINK (sink_in));
  if (unlikely (!sample_p))
    return GST_FLOW_ERROR;
  GstBuffer* buffer_p = gst_sample_get_buffer (sample_p);
  if (unlikely (!sample_p))
  {
    gst_sample_unref (sample_p);
    return GST_FLOW_ERROR;
  } // end IF
  GstMapInfo map;
  if (!gst_buffer_map (buffer_p, &map, GST_MAP_READ))
  {
    gst_sample_unref (sample_p);
    return GST_FLOW_ERROR;
  } // end IF

  ACE_Message_Block* message_block_p =
    static_cast<ACE_Message_Block*> (cb_data_p->allocator->malloc (map.size));
  ACE_ASSERT (message_block_p);

  int result =
    message_block_p->copy (reinterpret_cast<char*> (map.data),
                           static_cast<size_t> (map.size));
  ACE_ASSERT (result == 0);

  result = cb_data_p->queue->enqueue (message_block_p,
                                      NULL);
  ACE_ASSERT (result != -1);

  // clean up
  gst_buffer_unmap (buffer_p, &map);
  gst_sample_unref (sample_p);

  return GST_FLOW_OK;
}
