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
#include "streams.h"
#else
#include "alsa/asoundlib.h"
#endif

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dev_tools.h"

#include "stream_vis_defines.h"
#include "stream_vis_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T ()
 : inherited ()
// , cairoContext_ (NULL)
// , cairoSurface_ (NULL)
 , lock_ (NULL)
 , pixelBuffer_ (NULL)
 , isFirst_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::~Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::~Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T"));

//  if (cairoSurface_)
//    cairo_surface_destroy (cairoSurface_);
//  if (cairoContext_)
//    cairo_destroy (cairoContext_);

  if (pixelBuffer_)
    g_object_unref (pixelBuffer_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (!inherited::configuration_->gdkWindow)
    return; // done
  ACE_ASSERT (inherited::sessionData_);
  const SessionDataType& session_data_r = inherited::sessionData_->get ();
  //  ACE_ASSERT (cairoSurface_);
  ACE_ASSERT (pixelBuffer_);

  unsigned char* data_p =
      reinterpret_cast<unsigned char*> (message_inout->rd_ptr ());
  ACE_ASSERT (data_p);

  int result = -1;
  bool release_lock = false;
//  bool leave_gdk = false;
  unsigned int height, width = 0;

  if (lock_)
  {
    result = lock_->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_MUTEX::acquire(): \"%m\", returning\n")));
      return;
    } // end IF
    release_lock = true;
  } // end IF

  height =
      static_cast<unsigned int> (gdk_pixbuf_get_height (pixelBuffer_));
  width =
      static_cast<unsigned int> (gdk_pixbuf_get_width (pixelBuffer_));

  //gdk_threads_enter ();
  //leave_gdk = true;

  result = 0;

unlock:
  if (release_lock)
  {
    ACE_ASSERT (lock_);
    result = lock_->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  if (result == -1)
    goto error;

//  // step3: scale image to window area
//  gint x, y;
//  gdk_window_get_origin (configuration_->window,
//                         &x, &y);
//  gint width, height;
//  gdk_drawable_get_size (GDK_DRAWABLE (configuration_->window),
//                         &width, &height);
//  double scale_factor, scale_factor_x, scale_factor_y;
//  scale_factor_x =
//      width / static_cast<double> (sessionData_->format.fmt.pix.width);
//  scale_factor_y =
//      height / static_cast<double> (sessionData_->format.fmt.pix.height);
//  scale_factor = MIN (scale_factor_x, scale_factor_y);
//  cairo_scale (cairoContext_,
//               scale_factor, scale_factor);

  //  cairo_surface_set_mime_data (cairoSurface_,                                               // surface
  //                               ACE_TEXT_ALWAYS_CHAR ("video/x-raw-yuv/i420"),               // MIME type
  //                               reinterpret_cast<unsigned char*> (message_inout->rd_ptr ()), // data
  //                               message_inout->length (),                                    // length
  //                               NULL,                                                        // destroy function
  //                               NULL);                                                       // destroy function act

  //  pixmap = gdk_pixmap_new(window->window, width, height, -1);
  //  gdk_draw_pixbuf((GdkDrawable *) pixmap, NULL,
  //                  pixbuf,
  //                  0, 0, 0, 0, wifth, height,
  //                  GDK_RGB_DITHER_NORMAL, 0, 0);
  //  gtk_image_set_from_pixmap((GtkImage*) image, pixmap, NULL);
//  gdk_cairo_set_source_pixbuf (cairoContext_,
//                               pixelBuffer_,
//                               0.0, 0.0);

  // step4: paste the pixel buffer into the specified window area
//  cairo_paint (cairoContext_);
//  cairo_surface_flush (cairoSurface_);

  // step5: schedule an 'expose' event
  // *NOTE*: gdk_window_invalidate_rect() is not thread-safe. It will race with
  //         the UI refresh and eventually crash (even though gdk_threads_enter/
  //         gdk_threads_leave is in effect)
  //         --> schedule a refresh with gtk_widget_queue_draw_area() instead
  // *NOTE*: this does not work either... :-(
  //         --> let the downstream event handler queue an idle request
  //gdk_window_invalidate_rect (inherited::configuration_->gdkWindow,
  //                            NULL,
  //                            false);
//  GtkWidget* widget_p = NULL;
//  gdk_window_get_user_data (configuration_->window,
//                            reinterpret_cast<gpointer*> (&widget_p));
//  ACE_ASSERT (widget_p);
//  gtk_widget_queue_draw (widget_p);
//  GtkAllocation allocation;
//  gtk_widget_get_allocation (widget_p,
//                             &allocation);
//  gtk_widget_queue_draw_area (widget_p,
//                              allocation.x, allocation.y,
//                              allocation.width, allocation.height);

error:
//  if (leave_gdk)
//    gdk_threads_leave ();
  return;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->get ());

      // sanity check(s)
      if (!inherited::configuration_->gdkWindow)
        break; // done
      ACE_ASSERT (pixelBuffer_);

//      unsigned int width_window =
//          static_cast<unsigned int> (gdk_pixbuf_get_width (pixelBuffer_));
//      unsigned int height_window =
//          static_cast<unsigned int> (gdk_pixbuf_get_height (pixelBuffer_));
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      ACE_ASSERT (session_data_r.format);

//      struct tagVIDEOINFO* video_info_p =
//        reinterpret_cast<struct tagVIDEOINFO*> (session_data_r.format->pbFormat);

//      ACE_ASSERT (static_cast<LONG> (width_window)  >= video_info_p->bmiHeader.biWidth);
//      ACE_ASSERT (static_cast<LONG> (height_window) >= video_info_p->bmiHeader.biHeight);
//#else
//      ACE_ASSERT (width_window  >= session_data_r.area.width);
//      ACE_ASSERT (height_window >= session_data_r.area.height);
//#endif

//      ACE_ASSERT (cairoContext_);
//      ACE_ASSERT (!cairoSurface_);

//      // step1: allocate/retrieve a pixel buffer
//      cairo_status_t result = CAIRO_STATUS_SUCCESS;
//      cairo_format_t format = CAIRO_FORMAT_INVALID;
//      int width, height;
////      cairoSurface_ =
////          cairo_image_surface_create (MODULE_VIS_DEFAULT_CAIRO_FORMAT,       // format
////                                      session_data_r.format->fmt.pix.width,   // width
////                                      session_data_r.format->fmt.pix.height); // height
////      result = cairo_surface_status (cairoSurface_);
////      if (!cairoSurface_ ||
////          (result != CAIRO_STATUS_SUCCESS))
////      {
////        ACE_DEBUG ((LM_ERROR,
////                    ACE_TEXT ("failed to cairo_image_surface_create(%d,%d,%d): \"%s\", returning\n"),
////                    cairoFormat_,
////                    session_data_r.format->fmt.pix.width, session_data_r.format->fmt.pix.height,
////                    ACE_TEXT (cairo_status_to_string (result))));
////        goto error;
////      } // end IF
////      cairo_set_source_surface (cairoContext_, // context
////                                cairoSurface_, // surface
////    //                            (width  - (scale_factor * width))  / (2.0 * scale_factor),
////    //                            (height - (scale_factor * height)) / (2.0 * scale_factor));
////                                0.0, 0.0);     // x,y offset
//      cairo_pattern_t* pattern_p = cairo_get_source (cairoContext_);
//      ACE_ASSERT (pattern_p);
////      cairo_pattern_type_t pattern_type = cairo_pattern_get_type (pattern_p);
////      ACE_ASSERT (pattern_type == CAIRO_PATTERN_TYPE_SURFACE);
//      result = cairo_pattern_get_surface (pattern_p,
//                                          &cairoSurface_);
//      if (!cairoSurface_ ||
//          (result != CAIRO_STATUS_SUCCESS))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to cairo_pattern_get_surface(): \"%s\", aborting\n"),
//                    ACE_TEXT (cairo_status_to_string (result))));
//        goto error;
//      } // end IF
//      format = cairo_image_surface_get_format (cairoSurface_);
//      width = cairo_image_surface_get_width (cairoSurface_);
//      height = cairo_image_surface_get_height (cairoSurface_);
//      ACE_ASSERT (format == MODULE_VIS_DEFAULT_CAIRO_FORMAT);
//      ACE_ASSERT (width  >= static_cast<int> (session_data_r.format->fmt.pix.width));
//      ACE_ASSERT (height >= static_cast<int> (session_data_r.format->fmt.pix.height));

      break;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//error:
#endif
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
//      if (cairoSurface_)
//      {
//        cairo_surface_destroy (cairoSurface_);
//        cairoSurface_ = NULL;
//      } // end IF

      if (pixelBuffer_)
      {
        g_object_unref (pixelBuffer_);
        pixelBuffer_ = NULL;
      } // end IF

      break;
    }
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
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::initialize"));

  if (inherited::isInitialized_)
  {
//    if (cairoSurface_)
//    {
//      cairo_surface_destroy (cairoSurface_);
//      cairoSurface_ = NULL;
//    } // end IF

//    if (cairoContext_)
//    {
//      cairo_destroy (cairoContext_);
//      cairoContext_ = NULL;
//    } // end IF

    if (pixelBuffer_)
    {
      g_object_unref (pixelBuffer_);
      pixelBuffer_ = NULL;
    } // end IF

    isFirst_ = true;
  } // end IF

  lock_ = configuration_in.lock;
  if (!configuration_in.pixelBuffer)
  {
    // *TODO*: remove type inference
    if (configuration_in.gdkWindow)
    {
      gdk_threads_enter ();

      // sanity check(s)
      // *TODO*: remove type inference
      ACE_ASSERT (!configuration_in.pixelBuffer);

      // *TODO*: remove type inference
      pixelBuffer_ =
          //        gdk_pixbuf_get_from_window (configuration_->window,
          //                                    0, 0,
          //                                    configuration_->area.width, configuration_->area.height);
          gdk_pixbuf_get_from_drawable (NULL,
                                        GDK_DRAWABLE (configuration_in.gdkWindow),
                                        NULL,
                                        0, 0,
                                        0, 0, configuration_in.area.width, configuration_in.area.height);
      if (!pixelBuffer_)
      { // *NOTE*: most probable reason: window is not mapped
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to gdk_pixbuf_get_from_drawable(), aborting\n")));

        // clean up
        gdk_threads_leave ();

        return false;
      } // end IF
      gdk_threads_leave ();
    } // end IF
  } // end IF
  else
  {
    g_object_ref (configuration_in.pixelBuffer);
    pixelBuffer_ = configuration_in.pixelBuffer;
  } // end ELSE
  if (!pixelBuffer_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to obtain pixel buffer, aborting\n")));
    return false;
  } // end IF

//  // *TODO*: remove type inference
//  if (configuration_->window)
//  {
//    ACE_ASSERT (!cairoContext_);

//    cairoContext_ = gdk_cairo_create (GDK_DRAWABLE (configuration_->window));
//    if (!cairoContext_)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
//      return false;
//    } // end IF
//    gdk_cairo_set_source_pixbuf (cairoContext_,
//                                 pixelBuffer_,
//                                 0.0, 0.0);
//  } // end IF

  return inherited::initialize (configuration_in);
}
