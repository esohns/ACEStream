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

#ifdef __cplusplus
extern "C"
{
#include "libavutil/imgutils.h"
}
#endif /* __cplusplus */

#include "gtk/gtk.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               MediaType>::Stream_Module_Vis_GTK_Pixbuf_T (ISTREAM_T* stream_in)
#else
                               MediaType>::Stream_Module_Vis_GTK_Pixbuf_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , buffer_ (NULL)
 , isFirst_ (true)
 , lock_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::Stream_Module_Vis_GTK_Pixbuf_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::~Stream_Module_Vis_GTK_Pixbuf_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::~Stream_Module_Vis_GTK_Pixbuf_T"));

  if (buffer_)
    g_object_unref (buffer_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (!inherited::configuration_->window)
    return; // done
  ACE_ASSERT (buffer_);

  // *NOTE*: 'crunching' the message data simplifies the data transformation
  //         algorithms, at the cost of (several) memory copies. This is a
  //         tradeoff that may warrant further optimization efforts
  try {
    message_inout->defragment ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_IDataMessage_T::defragment(), returning\n"),
                inherited::mod_->name ()));
    return;
  }

  int result = -1;
  bool leave_gdk = false;
  bool release_lock = false;

  if (likely (lock_))
  {
    result = lock_->acquire ();
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::acquire(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      return;
    } // end IF
    release_lock = true;
  } // end IF

  gdk_threads_enter ();
  leave_gdk = true;

  ACE_OS::memcpy (gdk_pixbuf_get_pixels (buffer_),
                  message_inout->rd_ptr (),
                  message_inout->length ());

  //  gint width, height;
  //  gdk_drawable_get_size (GDK_DRAWABLE (configuration_->window),
  //                         &width, &height);
  // *IMPORTANT NOTE*: potentially, this involves tranfer of image data to an X
  //                   server running on a different host. Also, X servers don't
  //                   react kindly to multithreaded access
  //                   --> move this into the gtk context and simply schedule a
  //                       refresh, which takes care of that
  //  gdk_draw_pixbuf (GDK_DRAWABLE (configuration_->window), NULL,
  //                   buffer_,
  //                   0, 0, 0, 0, width, height,
  //                   GDK_RGB_DITHER_NONE, 0, 0);

  // step5: schedule an 'expose' event
  // *NOTE*: gdk_window_invalidate_rect() is not thread-safe. It will race with
  //         the UI refresh and eventually crash (even though gdk_threads_enter/
  //         gdk_threads_leave is in effect)
  //         --> schedule a refresh with gtk_widget_queue_draw_area() instead
  // *NOTE*: this does not work either... :-(
  //         --> make the (downstream) UI event handler queue an 'idle refresh'
//  gdk_window_invalidate_rect (inherited::configuration_->window,
//                              NULL,
//                              false);
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

  if (likely (leave_gdk))
  {
    gdk_threads_leave ();
    leave_gdk = false;
  } // end IF

  if (likely (release_lock))
  { ACE_ASSERT (lock_);
    result = lock_->release ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
    release_lock = false;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const typename SessionDataContainerType::DATA_T& session_data_r =
          inherited::sessionData_->getR ();
      const MediaType& media_type_r = session_data_r.formats.front ();
      struct Stream_MediaFramework_FFMPEG_MediaType media_type_s;
      inherited2::getMediaType (media_type_r,
                                media_type_s);
      unsigned int frame_size_i =
          av_image_get_buffer_size (media_type_s.format,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                    media_type_s.resolution.cx,
                                    media_type_s.resolution.cy,
#else
                                    media_type_s.resolution.width,
                                    media_type_s.resolution.height,
#endif // ACE_WIN32 || ACE_WIN64
                                    1); // *TODO*: linesize alignment
      ACE_UNUSED_ARG (frame_size_i);
      unsigned int row_stride_i =
          av_image_get_linesize (media_type_s.format,
    #if defined (ACE_WIN32) || defined (ACE_WIN64)
                                 media_type_s.resolution.cx,
    #else
                                 media_type_s.resolution.width,
    #endif // ACE_WIN32 || ACE_WIN64
                                 0);
//      ACE_UNUSED_ARG (row_stride_i);
      ACE_ASSERT (buffer_);
      int pixbuf_height = gdk_pixbuf_get_height (buffer_);
      int pixbuf_width = gdk_pixbuf_get_width (buffer_);
      int pixbuf_row_stride = gdk_pixbuf_get_rowstride (buffer_);

      ACE_ASSERT ((static_cast<unsigned int> (pixbuf_width) == media_type_s.resolution.width) && (static_cast<unsigned int> (pixbuf_height) == media_type_s.resolution.height));
      ACE_ASSERT (static_cast<unsigned int> (pixbuf_row_stride) == row_stride_i);

      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      // sanity check(s)
      // *TODO*: remove type inferences
      ACE_ASSERT (inherited::configuration_->area.width && inherited::configuration_->area.height);
      ACE_ASSERT (inherited::configuration_->window);

      if (buffer_)
      {
        g_object_unref (buffer_); buffer_ = NULL;
      } // end IF

      gdk_threads_enter ();

      buffer_ =
    #if GTK_CHECK_VERSION (3,0,0)
          gdk_pixbuf_get_from_window (inherited::configuration_->window,
                                      0, 0,
                                      inherited::configuration_->area.width, inherited::configuration_->area.height);
    #else
          gdk_pixbuf_get_from_drawable (NULL,
                                        GDK_DRAWABLE (inherited::configuration_->window),
                                        NULL,
                                        0, 0,
                                        0, 0, inherited::configuration_->area.width, inherited::configuration_->area.height);
    #endif
      if (!buffer_)
      { // *NOTE*: most probable reason: window is not mapped
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to gdk_pixbuf_get_from_window(), aborting\n"),
                    inherited::mod_->name ()));
        gdk_threads_leave ();
        goto error;
      } // end IF
      gdk_threads_leave ();

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
//    case STREAM_SESSION_MESSAGE_END:
//      break;
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
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
      g_object_unref (buffer_); buffer_ = NULL;
    } // end IF
    isFirst_ = true;
    lock_ = NULL;
  } // end IF

  // sanity check(s)
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.area.width && configuration_in.area.height);
  ACE_ASSERT (configuration_in.window);

  lock_ = configuration_in.lock;

  gdk_threads_enter ();

  buffer_ =
#if GTK_CHECK_VERSION (3,0,0)
      gdk_pixbuf_get_from_window (configuration_in.window,
                                  0, 0,
                                  configuration_in.area.width, configuration_in.area.height);
#else
      gdk_pixbuf_get_from_drawable (NULL,
                                    GDK_DRAWABLE (configuration_in.window),
                                    NULL,
                                    0, 0,
                                    0, 0, configuration_in.area.width, configuration_in.area.height);
#endif
  if (!buffer_)
  { // *NOTE*: most probable reason: window is not mapped
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to gdk_pixbuf_get_from_window(), aborting\n"),
                inherited::mod_->name ()));
    gdk_threads_leave ();
    return false;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (GDK_IS_PIXBUF (buffer_));
  ACE_ASSERT (gdk_pixbuf_get_colorspace (buffer_) == GDK_COLORSPACE_RGB);
  ACE_ASSERT (gdk_pixbuf_get_bits_per_sample (buffer_) == 8);
  ACE_ASSERT (gdk_pixbuf_get_n_channels (buffer_) == 4);
  ACE_ASSERT (gdk_pixbuf_get_has_alpha (buffer_));

  gdk_threads_leave ();

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::toggle"));

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}
