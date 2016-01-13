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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "linux/videodev2.h"
#endif

#include "stream_macros.h"
#include "stream_session_message_base.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_DrawingArea_T<SessionMessageType,
                                    MessageType,
                                    ConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType>::Stream_Module_Vis_GTK_DrawingArea_T ()
 : inherited ()
 , configuration_ (NULL)
 , sessionData_ (NULL)
 , cairoContext_ (NULL)
 , pixelBuffer_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_DrawingArea_T::Stream_Module_Vis_GTK_DrawingArea_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_DrawingArea_T<SessionMessageType,
                                    MessageType,
                                    ConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType>::~Stream_Module_Vis_GTK_DrawingArea_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_DrawingArea_T::~Stream_Module_Vis_GTK_DrawingArea_T"));

  if (cairoContext_)
    cairo_destroy (cairoContext_);
  if (pixelBuffer_)
    g_object_unref (pixelBuffer_);
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_DrawingArea_T<SessionMessageType,
                                    MessageType,
                                    ConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType>::handleDataMessage (MessageType*& message_inout,
                                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_DrawingArea_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (sessionData_);
  ACE_ASSERT (sessionData_->format.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);

  gdk_threads_enter ();

  if (pixelBuffer_)
  {
    g_object_unref (pixelBuffer_);
    pixelBuffer_ = NULL;
  } // end IF

  // step1: allocate a pixel buffer
  GdkColorspace colorspace = GDK_COLORSPACE_RGB;
  bool has_alpha = false;
  int bits_per_sample = -1;
  switch (sessionData_->format.fmt.pix.pixelformat)
  {
    case V4L2_PIX_FMT_BGR24:
      bits_per_sample = 8; break;
    case V4L2_PIX_FMT_RGB24:
      bits_per_sample = 8; break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown pixel format (was: %d), returning\n"),
                  sessionData_->format.fmt.pix.pixelformat));

      // clean up
      gdk_threads_leave ();

      return;
    }
  } // end SWITCH

  pixelBuffer_ =
      gdk_pixbuf_new_from_data (reinterpret_cast<guchar*> (message_inout->rd_ptr ()), // data
                                colorspace,                                           // color space
                                has_alpha,                                            // has alpha channel ?
                                bits_per_sample,                                      // bits per sample
                                sessionData_->format.fmt.pix.width,                   // width
                                sessionData_->format.fmt.pix.height,                  // height
                                sessionData_->format.fmt.pix.bytesperline,            // row stride
                                NULL,                                                 // destroy function
                                NULL);                                                // destroy function act
  if (!pixelBuffer_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_pixbuf_new_from_data(), returning\n")));

    // clean up
    gdk_threads_leave ();

    return;
  } // end IF

  // step2: paste the pixel buffer into the specified window area
  gdk_cairo_set_source_pixbuf (cairoContext_,
                               pixelBuffer_,
                               0.0, 0.0);
  cairo_paint (cairoContext_);

  // step3: schedule an 'expose' event
//  gtk_widget_queue_draw_area ();
  gdk_window_invalidate_rect (configuration_->gdkWindow,
//                              &configuration_->area,
                              NULL,
                              false);

  gdk_threads_leave ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_DrawingArea_T<SessionMessageType,
                                    MessageType,
                                    ConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_DrawingArea_T::handleSessionMessage"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      const SessionDataContainerType& session_data_container_r =
          message_inout->get ();
      sessionData_ = &session_data_container_r.get ();
      break;
    }
    case STREAM_SESSION_END:
    {
      sessionData_ = NULL;
      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Module_Vis_GTK_DrawingArea_T<SessionMessageType,
                                    MessageType,
                                    ConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_DrawingArea_T::initialize"));

  configuration_ = &configuration_in;

  // sanity check(s)
  ACE_ASSERT (configuration_->gdkWindow);

  if (isInitialized_)
  {
    if (cairoContext_)
    {
      cairo_destroy (cairoContext_);
      cairoContext_ = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF
  ACE_ASSERT (!cairoContext_);

  cairoContext_ = gdk_cairo_create (configuration_->gdkWindow);
  if (!cairoContext_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
    return false;
  } // end IF
//  gdk_cairo_set_source_window (cairoContext_,
//                               configuration_->gdkWindow,
//                               0.0, 0.0);

  isInitialized_ = true;

  return isInitialized_;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
const ConfigurationType&
Stream_Module_Vis_GTK_DrawingArea_T<SessionMessageType,
                                    MessageType,
                                    ConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_DrawingArea_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}
