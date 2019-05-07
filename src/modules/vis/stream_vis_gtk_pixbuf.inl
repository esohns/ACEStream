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
 , buffer_ (NULL)
#if GTK_CHECK_VERSION (3,0,0)
 , context_ (NULL)
#endif // GTK_CHECK_VERSION (3,0,0)
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

#if GTK_CHECK_VERSION (3,0,0)
 if (context_)
   cairo_destroy (context_);
#endif // GTK_CHECK_VERSION (3,0,0)
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

  bool leave_gdk = false;

  gdk_threads_enter ();

  ACE_OS::memcpy (gdk_pixbuf_get_pixels (buffer_),
                  message_inout->rd_ptr (),
                  message_inout->length ());

  // *IMPORTANT NOTE*: this potentially involves transfer of image data to an X
  //                   server running on a different host. Also, X servers don't
  //                   react kindly to multithreaded access
  //                   --> move this into the gtk context and simply schedule a
  //                       refresh, which takes care of that
#if GTK_CHECK_VERSION (3,0,0)
  ACE_ASSERT (context_);
//  gdk_cairo_set_source_pixbuf (context_, buffer_,
//                               0, 0);
  cairo_paint (context_);
  cairo_fill (context_);
#else
  //    gint width_i, height_i;
  //    gdk_drawable_get_size (GDK_DRAWABLE (inherited::configuration_->window),
  //                           &width_i, &height_i);
  gdk_draw_pixbuf (GDK_DRAWABLE (inherited::configuration_->window),
                   NULL,
                   buffer_,
                   0, 0, 0, 0, -1, -1,
                   GDK_RGB_DITHER_NONE, 0, 0);
#endif // GTK_CHECK_VERSION (3,0,0)

  if (likely (leave_gdk))
  {
    gdk_threads_leave ();
    leave_gdk = false;
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
      // *TODO*: remove type inference
      ACE_ASSERT (inherited::configuration_->window);
      ACE_ASSERT (!buffer_);

      gint width_i, height_i;

      gdk_threads_enter ();

#if GTK_CHECK_VERSION (3,0,0)
      width_i = gdk_window_get_width (inherited::configuration_->window);
      height_i = gdk_window_get_height (inherited::configuration_->window);
#else
      gdk_drawable_get_size (GDK_DRAWABLE (inherited::configuration_->window),
                             &width_i, &height_i);
#endif // GTK_CHECK_VERSION (3,0,0)
      buffer_ =
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
      if (!buffer_)
      { // *NOTE*: most probable reason: window is not mapped
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to gdk_pixbuf_get_from_window(), aborting\n"),
                    inherited::mod_->name ()));
        gdk_threads_leave ();
        goto error;
      } // end IF

      // sanity check(s)
      ACE_ASSERT (GDK_IS_PIXBUF (buffer_));
      ACE_ASSERT (gdk_pixbuf_get_colorspace (buffer_) == GDK_COLORSPACE_RGB);
      ACE_ASSERT (gdk_pixbuf_get_bits_per_sample (buffer_) == 8);
//      ACE_ASSERT (gdk_pixbuf_get_n_channels (buffer_) == 3);
      ACE_ASSERT (gdk_pixbuf_get_n_channels (buffer_) == 4);
      ACE_ASSERT (gdk_pixbuf_get_has_alpha (buffer_));

#if GTK_CHECK_VERSION (3,0,0)
      ACE_ASSERT (context_);
      gdk_cairo_set_source_pixbuf (context_, buffer_,
                                   0, 0);
#endif // GTK_CHECK_VERSION (3,0,0)

      gdk_threads_leave ();

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      // sanity check(s)
      ACE_ASSERT (false); // *TODO*
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
    case STREAM_SESSION_MESSAGE_END:
    {
      if (buffer_)
      {
        g_object_unref (buffer_); buffer_ = NULL;
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
      g_object_unref (buffer_);
      buffer_ = NULL;
    } // end IF

#if GTK_CHECK_VERSION (3,0,0)
    if (context_)
    {
      cairo_destroy (context_);
      context_ = NULL;
    } // end IF
#endif // GTK_CHECK_VERSION (3,0,0)
  } // end IF

#if GTK_CHECK_VERSION (3,0,0)
  ACE_ASSERT (configuration_in.window);
  context_ = gdk_cairo_create (configuration_in.window);
  ACE_ASSERT (context_);
#endif // GTK_CHECK_VERSION (3,0,0)

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
