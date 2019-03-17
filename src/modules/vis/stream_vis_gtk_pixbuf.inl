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
// , buffer_ (NULL)
// , foreignBuffer_ (true)
// , lock_ (NULL)
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
//  ACE_ASSERT (buffer_);

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
//  bool release_lock = false;
  GdkPixbuf* buffer_p = NULL;
  gint width_i, height_i;

//  if (likely (lock_))
//  {
//    result = lock_->acquire ();
//    if (unlikely (result == -1))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::acquire(): \"%m\", returning\n"),
//                  inherited::mod_->name ()));
//      return;
//    } // end IF
//    release_lock = true;
//  } // end IF

  gdk_threads_enter ();
  leave_gdk = true;

#if GTK_CHECK_VERSION (3,0,0)
  width_i = gdk_window_get_width (inherited::configuration_->window);
  height_i = gdk_window_get_height (inherited::configuration_->window);
#else
  gdk_drawable_get_size (GDK_DRAWABLE (inherited::configuration_->window),
                         &width_i, &height_i);
#endif // GTK_CHECK_VERSION (3,0,0)
  buffer_p =
#if GTK_CHECK_VERSION (3,0,0)
      gdk_pixbuf_get_from_window (inherited::configuration_->window,
                                  0, 0,
                                  width_i, height_i);
#else
      gdk_pixbuf_get_from_drawable (NULL,
                                    GDK_DRAWABLE (inherited::configuration_->window),
                                    NULL,
                                    0, 0,
                                    0, 0, width_i, height_i);
#endif // GTK_CHECK_VERSION (3,0,0)
  if (!buffer_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_pixbuf_get_from_window(), aborting\n")));
    return;
  } // end IF

//  ACE_OS::memcpy (gdk_pixbuf_get_pixels (buffer_),
  ACE_OS::memcpy (gdk_pixbuf_get_pixels (buffer_p),
                  message_inout->rd_ptr (),
                  message_inout->length ());

//    gint width_i, height_i;
//    gdk_drawable_get_size (GDK_DRAWABLE (inherited::configuration_->window),
//                           &width_i, &height_i);
  // *IMPORTANT NOTE*: this potentially involves transfer of image data to an X
  //                   server running on a different host. Also, X servers don't
  //                   react kindly to multithreaded access
  //                   --> move this into the gtk context and simply schedule a
  //                       refresh, which takes care of that
//  if (unlikely (!foreignBuffer_))
//  {
#if GTK_CHECK_VERSION (3,0,0)
  ACE_ASSERT (false); // *TODO*
#else
  gdk_draw_pixbuf (GDK_DRAWABLE (inherited::configuration_->window),
                     NULL,
//                     buffer_,
                     buffer_p,
                     0, 0, 0, 0, -1, -1,
                     GDK_RGB_DITHER_NONE, 0, 0);
#endif // GTK_CHECK_VERSION (3,0,0)
  // step5: schedule an 'expose' event
  // *NOTE*: gdk_window_invalidate_rect() is not thread-safe. It will race with
  //         the UI refresh and eventually crash (even though gdk_threads_enter/
  //         gdk_threads_leave is in effect)
  //         --> schedule a refresh with gtk_widget_queue_draw_area() instead
  // *NOTE*: this does not work either... :-(
  //         --> make the (downstream) UI event handler queue an 'idle refresh'
//    gdk_window_invalidate_rect (inherited::configuration_->window,
//                                NULL,
//                                false);
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
//  } // end IF

    g_object_unref (buffer_p); buffer_p = NULL;

  if (likely (leave_gdk))
  {
    gdk_threads_leave ();
    leave_gdk = false;
  } // end IF

//  if (likely (release_lock))
//  { ACE_ASSERT (lock_);
//    result = lock_->release ();
//    if (unlikely (result == -1))
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n"),
//                  inherited::mod_->name ()));
//    release_lock = false;
//  } // end IF
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
      // *TODO*: remove type inference
      ACE_ASSERT (inherited::configuration_->window);
//      ACE_ASSERT (!buffer_);

//      if (inherited::configuration_->pixelBuffer)
//      {
//        g_object_ref (inherited::configuration_->pixelBuffer);
//        buffer_ = inherited::configuration_->pixelBuffer;
//      } // end IF
//      else
//      {
//        gint width_i = 0, height_i = 0;

//        gdk_threads_enter ();

//        gdk_window_get_size (GDK_DRAWABLE (inherited::configuration_->window),
//                             &width_i, &height_i);

//        buffer_ =
//#if GTK_CHECK_VERSION (3,0,0)
//            gdk_pixbuf_get_from_window (inherited::configuration_->window,
//                                        0, 0,
//                                        width_i, height_i);
//#else
//            gdk_pixbuf_get_from_drawable (NULL,
//                                          GDK_DRAWABLE (inherited::configuration_->window),
//                                          NULL,
//                                          0, 0,
//                                          0, 0, width_i, height_i);
//#endif
//        if (!buffer_)
//        { // *NOTE*: most probable reason: window is not mapped
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("%s: failed to gdk_pixbuf_get_from_window(), aborting\n"),
//                      inherited::mod_->name ()));
//          gdk_threads_leave ();
//          goto error;
//        } // end IF

//        gdk_threads_leave ();

//        foreignBuffer_ = false;
//      } // end ELSE

      // sanity check(s)
//      ACE_ASSERT (GDK_IS_PIXBUF (buffer_));
//      ACE_ASSERT (gdk_pixbuf_get_colorspace (buffer_) == GDK_COLORSPACE_RGB);
//      ACE_ASSERT (gdk_pixbuf_get_bits_per_sample (buffer_) == 8);
//      ACE_ASSERT (gdk_pixbuf_get_n_channels (buffer_) == 3);
    //  ACE_ASSERT (gdk_pixbuf_get_n_channels (buffer_) == 4);
    //  ACE_ASSERT (gdk_pixbuf_get_has_alpha (buffer_));

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      // sanity check(s)
      // *TODO*: remove type inferences
      ACE_ASSERT (inherited::configuration_->window);

      gint width_i = 0, height_i = 0;

//      if (buffer_)
//      {
//        g_object_unref (buffer_); buffer_ = NULL;
//      } // end IF

      gdk_threads_enter ();

#if GTK_CHECK_VERSION (3,0,0)
      width_i = gdk_window_get_width (inherited::configuration_->window);
      height_i = gdk_window_get_height (inherited::configuration_->window);
#elif GTK_CHECK_VERSION (2,0,0)
      gdk_drawable_get_size (GDK_DRAWABLE (inherited::configuration_->window),
                             &width_i, &height_i);
#endif // GTK_CHECK_VERSION (3,0,0)
//      buffer_ =
//#if GTK_CHECK_VERSION (3,0,0)
//          gdk_pixbuf_get_from_window (inherited::configuration_->window,
//                                      0, 0,
//                                      width_i, height_i);
//#else
//          gdk_pixbuf_get_from_drawable (NULL,
//                                        GDK_DRAWABLE (inherited::configuration_->window),
//                                        NULL,
//                                        0, 0,
//                                        0, 0, width_i, height_i);
//#endif // GTK_CHECK_VERSION (3,0,0)
//      if (!buffer_)
//      { // *NOTE*: most probable reason: window is not mapped
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to gdk_pixbuf_get_from_window(), aborting\n"),
//                    inherited::mod_->name ()));
//        gdk_threads_leave ();
//        goto error_2;
//      } // end IF
      gdk_threads_leave ();

      break;

error_2:
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
//    if (buffer_)
//    {
//      g_object_unref (buffer_); buffer_ = NULL;
//    } // end IF
//    foreignBuffer_ = true;
//    lock_ = NULL;
  } // end IF

//  lock_ = configuration_in.lock;

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
