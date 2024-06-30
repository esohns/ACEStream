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

#include "common_ui_gtk_tools.h"

#include "stream_macros.h"

#include "stream_vis_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               MediaType>::Stream_Module_Vis_GTK_Window_T (ISTREAM_T* stream_in)
#else
                               MediaType>::Stream_Module_Vis_GTK_Window_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
// , mainLoop_ (NULL)
 , window_ (NULL)
#if GTK_CHECK_VERSION (3,0,0)
 , context_ (NULL)
#endif // GTK_CHECK_VERSION (3,0,0)
 , pixbuf_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::Stream_Module_Vis_GTK_Window_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::~Stream_Module_Vis_GTK_Window_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::~Stream_Module_Vis_GTK_Window_T"));

#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_ENTER ();
#endif // GTK_CHECK_VERSION (3,6,0)
  if (window_)
  {
#if GTK_CHECK_VERSION (3,10,0)
    gtk_window_close (window_); window_ = NULL;
#else
    gtk_widget_destroy (GTK_WIDGET (window_)); window_ = NULL;
#endif // GTK_CHECK_VERSION (3,10,0)
  } // end IF

//  if (mainLoop_)
//    g_main_loop_unref (mainLoop_);

#if GTK_CHECK_VERSION (3,0,0)
  if (context_)
    cairo_destroy (context_);
#endif // GTK_CHECK_VERSION (3,0,0)
  if (pixbuf_)
    g_object_unref (pixbuf_);

#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_LEAVE ();
#endif // GTK_CHECK_VERSION (3,6,0)
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (window_);
  ACE_ASSERT (pixbuf_);
#if GTK_CHECK_VERSION (3,0,0)
  ACE_ASSERT (context_);
#endif // GTK_CHECK_VERSION (3,0,0)

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

#if GTK_CHECK_VERSION (3,6,0)
#else
  bool leave_gdk = false;
  GDK_THREADS_ENTER ();
  leave_gdk = true;
#endif // GTK_CHECK_VERSION (3,6,0)

  ACE_OS::memcpy (gdk_pixbuf_get_pixels (pixbuf_),
                  message_inout->rd_ptr (),
                  message_inout->length ());

#if GTK_CHECK_VERSION (3,0,0)
  gdk_cairo_set_source_pixbuf (context_, pixbuf_, 0.0, 0.0);
  cairo_paint (context_);
#else
  gdk_draw_pixbuf (GDK_DRAWABLE (window_),
                   NULL,
                   pixbuf_,
                   0, 0, 0, 0, -1, -1,
                   GDK_RGB_DITHER_NONE, 0, 0);
#endif // GTK_CHECK_VERSION (3,0,0)

#if GTK_CHECK_VERSION (3,6,0)
#else
  if (likely (leave_gdk))
  {
    GDK_THREADS_LEAVE ();
    leave_gdk = false;
  } // end IF
#endif // GTK_CHECK_VERSION (3,6,0)
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      // *TODO*: remove type inference
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (!window_);// && !mainLoop_);
      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
      struct Stream_MediaFramework_V4L_MediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      Common_Image_Resolution_t resolution_s =
        inherited2::getResolution (media_type_s);

#if GTK_CHECK_VERSION (3,6,0)
#else
      bool leave_gdk = false;
      GDK_THREADS_ENTER ();
      leave_gdk = true;
#endif // GTK_CHECK_VERSION (3,6,0)
      if (unlikely (!initialize_GTK (resolution_s)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Module_Vis_GTK_Window_T::initialize_GTK(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (window_);

      gtk_widget_show_all (GTK_WIDGET (window_));

      ACE_ASSERT (!pixbuf_);
      gint width_i, height_i;
      GtkAllocation allocation_s;
      gtk_widget_get_allocation (GTK_WIDGET (window_),
                                 &allocation_s);
      width_i = allocation_s.width;
      height_i = allocation_s.height;

      pixbuf_ =
#if GTK_CHECK_VERSION (3,0,0)
        gdk_pixbuf_get_from_window (gtk_widget_get_window (GTK_WIDGET (window_)),
                                    0, 0,
                                    width_i, height_i);
#else
        gdk_pixbuf_get_from_drawable (NULL,
                                      GDK_DRAWABLE (gtk_widget_get_window (GTK_WIDGET (window_))),
                                      NULL,
                                      0, 0,
                                      0, 0, width_i, height_i);
#endif // GTK_CHECK_VERSION (3,0,0)
      if (unlikely (!pixbuf_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to gdk_pixbuf_get_from_window(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (gdk_pixbuf_get_colorspace (pixbuf_) == GDK_COLORSPACE_RGB);
      ACE_ASSERT (gdk_pixbuf_get_bits_per_sample (pixbuf_) == 8);
      ACE_ASSERT (gdk_pixbuf_get_n_channels (pixbuf_) == 3);

#if GTK_CHECK_VERSION (3,0,0)
      ACE_ASSERT (!context_);
      context_ = gdk_cairo_create (gtk_widget_get_window (GTK_WIDGET (window_)));
      if (unlikely (!context_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to gdk_cairo_create(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      gdk_cairo_set_source_pixbuf (context_, pixbuf_, 0.0, 0.0);
#endif // GTK_CHECK_VERSION (3,0,0)

      // *TODO*: subscribe to signals (realize, configure, expose, ...)

#if GTK_CHECK_VERSION (3,6,0)
#else
      GDK_THREADS_LEAVE ();
      leave_gdk = false;
#endif // GTK_CHECK_VERSION (3,6,0)

      inherited::threadCount_ = 1;
      inherited::start (NULL);
      inherited::threadCount_ = 0;

      break;

error:
#if GTK_CHECK_VERSION (3,6,0)
#else
      if (likely (leave_gdk))
      {
        GDK_THREADS_LEAVE ();
        leave_gdk = false;
      } // end IF
#endif // GTK_CHECK_VERSION (3,6,0)

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
#if GTK_CHECK_VERSION (3,6,0)
#else
      GDK_THREADS_ENTER ();
#endif // GTK_CHECK_VERSION (3,6,0)

      if (likely (window_))
      {
#if GTK_CHECK_VERSION (3,10,0)
        gtk_window_close (window_); window_ = NULL;
#else
        gtk_widget_destroy (GTK_WIDGET (window_)); window_ = NULL;
#endif // GTK_CHECK_VERSION (3,10,0)
      } // end IF

//      if (likely (mainLoop_ &&
//                  g_main_loop_is_running (mainLoop_)))
//        g_main_loop_quit (mainLoop_);
      if (inherited::thr_count_ > 0)
        gtk_main_quit ();

//      if (likely (mainLoop_))
//      {
//        g_main_loop_unref (mainLoop_); mainLoop_ = NULL;
//      } // end IF

#if GTK_CHECK_VERSION (3,0,0)
      if (context_)
      {
        cairo_destroy (context_); context_ = NULL;
      } // end IF
#endif // GTK_CHECK_VERSION (3,0,0)
      if (pixbuf_)
      {
        g_object_unref (pixbuf_); pixbuf_ = NULL;
      }

#if GTK_CHECK_VERSION (3,6,0)
#else
      GDK_THREADS_LEAVE ();
#endif // GTK_CHECK_VERSION (3,6,0)

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
          typename MediaType>
bool
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::initialize"));

  if (inherited::isInitialized_)
  {
#if GTK_CHECK_VERSION (3,6,0)
#else
    GDK_THREADS_ENTER ();
#endif // GTK_CHECK_VERSION (3,6,0)

//    if (mainLoop_)
//    {
//      g_main_loop_unref (mainLoop_); mainLoop_ = NULL;
//    } // end IF
    if (window_)
    {
#if GTK_CHECK_VERSION (3,10,0)
      gtk_window_close (window_); window_ = NULL;
#else
      gtk_widget_destroy (GTK_WIDGET (window_)); window_ = NULL;
#endif // GTK_CHECK_VERSION (3,10,0)
    } // end IF

#if GTK_CHECK_VERSION (3,0,0)
    if (context_)
    {
      cairo_destroy (context_); context_ = NULL;
    } // end IF
#endif // GTK_CHECK_VERSION (3,0,0)
    if (pixbuf_)
    {
      g_object_unref (pixbuf_); pixbuf_ = NULL;
    }

#if GTK_CHECK_VERSION (3,6,0)
#else
    GDK_THREADS_LEAVE ();
#endif // GTK_CHECK_VERSION (3,6,0)
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::toggle"));

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::initialize_GTK (const Common_Image_Resolution_t& resolution_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::initialize_GTK"));

  // sanity check(s)
  ACE_ASSERT (Common_UI_GTK_Tools::GTKInitialized);
//  ACE_ASSERT (!mainLoop_);
  ACE_ASSERT (!window_);

//  mainLoop_ = g_main_loop_new (NULL,
//                               FALSE); // is running ?
//  if (unlikely (!mainLoop_))
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("%s: failed to g_main_loop_new(NULL,FALSE), aborting\n"),
//                inherited::mod_->name ()));
//    return false;
//  } // end IF

  gint width_i, height_i;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  width_i = resolution_in.cx;
  height_i = resolution_in.cy;
#else
  width_i = resolution_in.width;
  height_i = resolution_in.height;
#endif // ACE_WIN32 || ACE_WIN64
  window_ = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
  if (unlikely (!window_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to gtk_window_new(), aborting\n"),
                inherited::mod_->name ()));
//    g_main_loop_unref (mainLoop_); mainLoop_ = NULL;
    return false;
  } // end IF
  gtk_window_set_title (window_,
                        ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DEFAULT_WINDOW_TITLE));
  //  gtk_widget_set_size_request (GTK_WIDGET (window_),
  gtk_window_resize (window_,
                     width_i, height_i);

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
int
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::svc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::svc"));

  // sanity check(s)
//  ACE_ASSERT (mainLoop_);

#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_ENTER ();
#endif // GTK_CHECK_VERSION (3,6,0)

//  g_main_loop_run (mainLoop_);
  gtk_main ();

#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_LEAVE ();
#endif // GTK_CHECK_VERSION (3,6,0)

  return 0;
}
