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

#include "stream_callbacks.h"

#include "ace/Synch_Traits.h"

#include "glade/glade.h"

#include "common_timer_manager.h"

#include "common_ui_common.h"
#include "common_ui_defines.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_tools.h"

#include "stream_macros.h"

#include "test_u_common.h"

#include "test_u_filecopy_defines.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
G_MODULE_EXPORT gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  Stream_GTK_CBData* data_p = static_cast<Stream_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  //// sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
  //  GTK_WIDGET (glade_xml_get_widget ((*iterator).second.second,
  //                                    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_MAIN_NAME)));
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  //  GtkWidget* image_icon_p = gtk_image_new_from_file (path.c_str ());
  //  ACE_ASSERT (image_icon_p);
  //  gtk_window_set_icon (GTK_WINDOW (dialog_p),
  //                       gtk_image_get_pixbuf (GTK_IMAGE (image_icon_p)));
  //GdkWindow* dialog_window_p = gtk_widget_get_window (dialog_p);
  //gtk_window_set_title (,
  //                      caption.c_str ());

//  GtkWidget* about_dialog_p =
//    //GTK_WIDGET (glade_xml_get_widget ((*iterator).second.second,
//    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
//    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
//                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
//  ACE_ASSERT (about_dialog_p);

  // step4: initialize text view, setup auto-scrolling
  GtkTextView* view_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
//  GtkTextBuffer* buffer_p =
////    gtk_text_buffer_new (NULL); // text tag table --> create new
//      gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);
////  gtk_text_view_set_buffer (view_p, buffer_p);

  PangoFontDescription* font_description_p =
    pango_font_description_from_string (ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION));
  if (!font_description_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pango_font_description_from_string(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_U_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION)));
    return FALSE; // G_SOURCE_REMOVE
  } // end IF
  // apply font
  GtkRcStyle* rc_style_p = gtk_rc_style_new ();
  if (!rc_style_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_rc_style_new(): \"%m\", aborting\n")));
    return FALSE; // G_SOURCE_REMOVE
  } // end IF
  rc_style_p->font_desc = font_description_p;
  GdkColor base_colour, text_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PANGO_LOG_COLOR_BASE),
                   &base_colour);
  rc_style_p->base[GTK_STATE_NORMAL] = base_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PANGO_LOG_COLOR_TEXT),
                   &text_colour);
  rc_style_p->text[GTK_STATE_NORMAL] = text_colour;
  rc_style_p->color_flags[GTK_STATE_NORMAL] =
    static_cast<GtkRcFlags> (GTK_RC_BASE |
                             GTK_RC_TEXT);
  gtk_widget_modify_style (GTK_WIDGET (view_p),
                           rc_style_p);
  gtk_rc_style_unref (rc_style_p);

  //  GtkTextIter iterator;
  //  gtk_text_buffer_get_end_iter (buffer_p,
  //                                &iterator);
  //  gtk_text_buffer_create_mark (buffer_p,
  //                               ACE_TEXT_ALWAYS_CHAR (NET_UI_SCROLLMARK_NAME),
  //                               &iterator,
  //                               TRUE);
  //  g_object_unref (buffer_p);

  // step5: initialize updates
  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (data_p->lock);

    // schedule asynchronous updates of the log view
    guint event_source_id = g_timeout_add_seconds (1,
                                                   idle_update_log_display_cb,
                                                   userData_in);
    if (event_source_id > 0)
      data_p->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
      return FALSE; // G_SOURCE_REMOVE
    } // end ELSE
    // schedule asynchronous updates of the info view
    event_source_id = g_timeout_add (TEST_U_STREAM_UI_GTKEVENT_RESOLUTION,
                                     idle_update_info_display_cb,
                                     userData_in);
    if (event_source_id > 0)
      data_p->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return FALSE; // G_SOURCE_REMOVE
    } // end ELSE
  } // end lock scope

  // step6: disable some functions ?
  GtkButton* button_p =
    //GTK_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_START_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);
  button_p =
      //GTK_BUTTON (glade_xml_get_widget ((*iterator).second.second,
      //                                  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CLOSEALL_NAME)));
      GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_STOP_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);

  // step7: (auto-)connect signals/slots
  // *NOTE*: glade_xml_signal_autoconnect does not work reliably
  //glade_xml_signal_autoconnect(userData_out.xml);

  // step6a: connect default signals
  gulong result =
      g_signal_connect (dialog_p,
                        ACE_TEXT_ALWAYS_CHAR ("destroy"),
                        G_CALLBACK (gtk_widget_destroyed),
                        NULL);
  ACE_ASSERT (result);

  // step6b: connect custom signals
  //gtk_builder_connect_signals ((*iterator).second.second,
  //                             userData_in);
  GObject* object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_START_NAME));
  ACE_ASSERT (object_p);
  result = g_signal_connect (object_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (button_start_clicked_cb),
                             userData_in);
  ACE_ASSERT (result);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_STOP_NAME));
  ACE_ASSERT (object_p);
  result =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_stop_clicked_cb),
                        userData_in);
  ACE_ASSERT (result);
  //-------------------------------------

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CLEAR_NAME));
  ACE_ASSERT (object_p);
  result =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("clicked"),
                      G_CALLBACK (button_clear_clicked_cb),
                      userData_in);
  ACE_ASSERT (result);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_ABOUT_NAME));
  ACE_ASSERT (object_p);
  result =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_about_clicked_cb),
                        userData_in);
  ACE_ASSERT (result);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_QUIT_NAME));
  ACE_ASSERT (object_p);
  result =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_quit_clicked_cb),
                        userData_in);
  ACE_ASSERT (result);
  ACE_UNUSED_ARG (result);

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  gtk_widget_show_all (dialog_p);

  return FALSE; // G_SOURCE_REMOVE
}

G_MODULE_EXPORT gboolean
idle_update_log_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));

  Stream_GTK_CBData* data_p = static_cast<Stream_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTextView* view_p =
      //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
      //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
      GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);

  GtkTextIter text_iterator;
  gtk_text_buffer_get_end_iter (buffer_p,
                                &text_iterator);

  gchar* converted_text = NULL;
  { // synch access
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->stackLock);

    // sanity check
    if (data_p->logStack.empty ())
      return TRUE; // G_SOURCE_CONTINUE

    // step1: convert text
    for (Common_MessageStackConstIterator_t iterator_2 = data_p->logStack.begin ();
         iterator_2 != data_p->logStack.end ();
         iterator_2++)
    {
      converted_text = Common_UI_Tools::Locale2UTF8 (*iterator_2);
      if (!converted_text)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to convert message text (was: \"%s\"), aborting\n"),
                    ACE_TEXT ((*iterator_2).c_str ())));
        return FALSE; // G_SOURCE_REMOVE
      } // end IF

      // step2: display text
      gtk_text_buffer_insert (buffer_p,
                              &text_iterator,
                              converted_text,
                              -1);

      // clean up
      g_free (converted_text);
    } // end FOR

    data_p->logStack.clear ();
  } // end lock scope

  // step3: scroll the view accordingly
//  // move the iterator to the beginning of line, so it doesn't scroll
//  // in horizontal direction
//  gtk_text_iter_set_line_offset (&text_iterator, 0);

//  // ...and place the mark at iter. The mark will stay there after insertion
//  // because it has "right" gravity
//  GtkTextMark* text_mark_p =
//      gtk_text_buffer_get_mark (buffer_p,
//                                ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SCROLLMARK_NAME));
////  gtk_text_buffer_move_mark (buffer_p,
////                             text_mark_p,
////                             &text_iterator);

//  // scroll the mark onscreen
//  gtk_text_view_scroll_mark_onscreen (view_p,
//                                      text_mark_p);
  GtkAdjustment* adjustment_p =
      GTK_ADJUSTMENT (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ADJUSTMENT_NAME)));
  ACE_ASSERT (adjustment_p);
  gtk_adjustment_set_value (adjustment_p,
                            gtk_adjustment_get_upper (adjustment_p));

  return TRUE; // G_SOURCE_CONTINUE
}

G_MODULE_EXPORT gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  Stream_GTK_CBData* data_p = static_cast<Stream_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  bool update_buttons = false;
  GtkSpinButton* spinbutton_p = NULL;
  { // synch access
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->stackLock);

    if (data_p->eventStack.empty ())
      return TRUE; // G_SOURCE_CONTINUE

    for (Stream_GTK_EventsIterator_t iterator_2 = data_p->eventStack.begin ();
         iterator_2 != data_p->eventStack.end ();
         iterator_2++)
    {
      switch (*iterator_2)
      {
        case STREAM_GTKEVENT_START:
        {
          spinbutton_p =
            //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
            //                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_NUMMESSAGES_NAME)));
          ACE_ASSERT (spinbutton_p);
          gtk_spin_button_set_value (spinbutton_p, 1.0);

          GtkWidget* widget_p =
            //GTK_WIDGET (glade_xml_get_widget ((*iterator).second.second,
            //                                  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
            GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_START_NAME)));
          if (!widget_p)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", continuing\n"),
                        ACE_TEXT (TEST_U_STREAM_UI_GTK_BUTTON_START_NAME)));
          else
            gtk_widget_set_sensitive (widget_p, false);
          widget_p =
            //GTK_WIDGET (glade_xml_get_widget ((*iterator).second.second,
            //                                  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
            GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_STOP_NAME)));
          if (!widget_p)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", continuing\n"),
                        ACE_TEXT (TEST_U_STREAM_UI_GTK_BUTTON_STOP_NAME)));
          else
            gtk_widget_set_sensitive (widget_p, true);

          break;
        }
        case STREAM_GTKEVENT_DATA:
        {
          spinbutton_p =
            //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
            //                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_NUMMESSAGES_NAME)));
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_NUMMESSAGES_NAME)));
          ACE_ASSERT (spinbutton_p);
          gtk_spin_button_spin (spinbutton_p,
                                GTK_SPIN_STEP_FORWARD,
                                1.0);

          break;
        }
        case STREAM_GTKEVENT_END:
        {
          spinbutton_p =
            //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
            //                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_NUMMESSAGES_NAME)));
          ACE_ASSERT (spinbutton_p);
          gtk_spin_button_set_value (spinbutton_p, 0.0);

          GtkWidget* widget_p =
            //GTK_WIDGET (glade_xml_get_widget ((*iterator).second.second,
            //                                  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
            GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_START_NAME)));
          if (!widget_p)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", continuing\n"),
                        ACE_TEXT (TEST_U_STREAM_UI_GTK_BUTTON_START_NAME)));
          else
            gtk_widget_set_sensitive (widget_p, true);
          widget_p =
            //GTK_WIDGET (glade_xml_get_widget ((*iterator).second.second,
            //                                  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
            GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_STOP_NAME)));
          if (!widget_p)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", continuing\n"),
                        ACE_TEXT (TEST_U_STREAM_UI_GTK_BUTTON_STOP_NAME)));
          else
            gtk_widget_set_sensitive (widget_p, false);

          break;
        }
        case STREAM_GTKEVENT_STATISTICS:
        {
          // *TODO*
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown event type (was: %d), continuing\n"),
                      *iterator_2));
          break;
        }
      } // end SWITCH
    } // end FOR

    data_p->eventStack.clear ();
  } // end lock scope

  return TRUE; // G_SOURCE_CONTINUE
}

// -----------------------------------------------------------------------------

G_MODULE_EXPORT gint
button_start_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_start_clicked_cb"));

  ACE_UNUSED_ARG (userData_in);
  Stream_GTK_CBData* data_p = static_cast<Stream_GTK_CBData*> (userData_in);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  gtk_widget_set_sensitive (widget_in, false);
  GtkButton* button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_STOP_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), true);

  data_p->stream.start ();
  if (!data_p->stream.isRunning ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream::start(): \"%m\", continuing\n")));

  return FALSE;
} // button_start_clicked_cb

G_MODULE_EXPORT gint
button_stop_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_stop_clicked_cb"));

  ACE_UNUSED_ARG (userData_in);
  Stream_GTK_CBData* data_p = static_cast<Stream_GTK_CBData*> (userData_in);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  gtk_widget_set_sensitive (widget_in, false);
  GtkButton* button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_START_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), true);

  data_p->stream.stop ();
  if (data_p->stream.isRunning ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream::stop(): \"%m\", continuing\n")));

  return FALSE;
} // button_close_clicked_cb

// -----------------------------------------------------------------------------

G_MODULE_EXPORT gint
button_clear_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  Stream_GTK_CBData* data_p = static_cast<Stream_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTextView* view_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p =
//    gtk_text_buffer_new (NULL); // text tag table --> create new
    gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);
  gtk_text_buffer_set_text (buffer_p,
                            ACE_TEXT_ALWAYS_CHAR (""), 0);

  return FALSE;
}

G_MODULE_EXPORT gint
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  Stream_GTK_CBData* data_p = static_cast<Stream_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  // retrieve about dialog handle
  GtkDialog* about_dialog =
    //GTK_DIALOG (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR(TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to glade_xml_get_widget(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
    return TRUE; // propagate
  } // end IF

  // run dialog
  gint result = gtk_dialog_run (about_dialog);
  switch (result)
  {
    case GTK_RESPONSE_ACCEPT:
      break;
    default:
      break;
  } // end SWITCH
  gtk_widget_hide (GTK_WIDGET (about_dialog));

  return FALSE;
} // button_about_clicked_cb

G_MODULE_EXPORT gint
button_quit_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  int result = -1;

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (userData_in);
  //Stream_GTK_CBData* data_p = static_cast<Stream_GTK_CBData*> (userData_in);
  //// sanity check(s)
  //ACE_ASSERT (data_p);

  //// step1: remove event sources
  //{
  //  ACE_Guard<ACE_Thread_Mutex> aGuard (data_p->lock);

  //  for (Common_UI_GTKEventSourceIdsIterator_t iterator = data_p->eventSourceIds.begin ();
  //       iterator != data_p->eventSourceIds.end ();
  //       iterator++)
  //    if (!g_source_remove (*iterator))
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
  //                  *iterator));
  //  data_p->eventSourceIds.clear ();
  //} // end lock scope

  // step2: initiate shutdown sequence
  result = ACE_OS::raise (SIGINT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                SIGINT));

  // step3: stop GTK event processing
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->close (1);

  return FALSE;
} // button_quit_clicked_cb

#ifdef __cplusplus
}
#endif /* __cplusplus */
