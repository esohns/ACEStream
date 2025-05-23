/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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

#include "http_get_callbacks.h"

#include "gmodule.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gdk/gdkwin32.h"
#endif // ACE_WIN32 || ACE_WIN64

#include <sstream>

#include "ace/Log_Msg.h"
#include "ace/Time_Value.h"

#include "common_time_common.h"
#include "common_timer_manager.h"

#include "common_ui_defines.h"
#include "common_ui_gtk_common.h"
#include "common_ui_tools.h"

#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#include "stream_macros.h"

#include "stream_dec_tools.h"

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "net_configuration.h"

#include "http_tools.h"

#include "http_get_common.h"
#include "http_get_defines.h"
#include "http_get_session_message.h"
#include "http_get_stream_common.h"

// -----------------------------------------------------------------------------

ACE_THR_FUNC_RETURN
stream_processing_function (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_processing_function"));

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = std::numeric_limits<unsigned long>::max ();
#else
  result = arg_in;
#endif

  Common_UI_GTK_BuildersConstIterator_t iterator;
  struct HTTPGet_UI_ThreadData* thread_data_p =
    static_cast<struct HTTPGet_UI_ThreadData*> (arg_in);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // sanity check(s)
  ACE_ASSERT (thread_data_p->CBData);
  ACE_ASSERT (thread_data_p->CBData->configuration);
  ACE_ASSERT (thread_data_p->CBData->stream);

  iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // *IMPORTANT NOTE*: cl.exe (*TODO*: gcc ?) fails to 'dynamic cast'
  //                   Stream_IStream_T to Stream_IStreamControlBase
  //                   --> upcast to ACE_Stream first
  Stream_Base_t* stream_base_p =
    dynamic_cast<Stream_Base_t*> (thread_data_p->CBData->stream);
  ACE_ASSERT (stream_base_p);
  Stream_IStreamControlBase* istream_control_p =
    dynamic_cast<Stream_IStreamControlBase*> (stream_base_p);
  ACE_ASSERT (istream_control_p);
  Common_IInitialize_T<HTTPGet_StreamConfiguration_t>* iinitialize_p =
    dynamic_cast<Common_IInitialize_T<HTTPGet_StreamConfiguration_t>*> (thread_data_p->CBData->stream);
  ACE_ASSERT (iinitialize_p);
  Common_IGetR_2_T<HTTPGet_SessionData_t>* iget_p =
    dynamic_cast<Common_IGetR_2_T<HTTPGet_SessionData_t>*> (thread_data_p->CBData->stream);
  ACE_ASSERT (iget_p);
  //  GtkStatusbar* statusbar_p = NULL;
//  const HTTPGet_SessionData_t* session_data_container_p = NULL;
//  const struct HTTPGet_SessionData* session_ui_cb_data_p = NULL;
//  std::ostringstream converter;
  bool result_2 = false;
//  guint context_id = 0;

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_p->CBData->UIState.lock, -1);
//#else
//  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_p->CBData->UIState.lock, std::numeric_limits<void*>::max ());
//#endif
    // configure streams and retrieve stream handles
    //ACE_Time_Value session_start_timeout =
    //    COMMON_TIME_NOW + ACE_Time_Value (5, 0);
    result_2 =
      iinitialize_p->initialize (thread_data_p->CBData->configuration->streamConfiguration);
    if (!result_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize stream \"%s\", aborting\n"),
                  ACE_TEXT (thread_data_p->CBData->stream->name ().c_str ())));
      goto done;
    } // end IF

//    session_data_container_p = &iget_p->getR_2 ();
//    ACE_ASSERT (session_data_container_p);
//    session_ui_cb_data_p =
//      &const_cast<struct HTTPGet_SessionData&> (session_data_container_p->getR ());
//    ACE_ASSERT (session_ui_cb_data_p);
//    converter.clear ();
//    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
//    converter << session_ui_cb_data_p->sessionId;
    // set context id
    //    gdk_threads_enter ();
    //    statusbar_p =
    //      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
    //                                             ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_STATUSBAR)));
    //    ACE_ASSERT (statusbar_p);
    //    ui_cb_data_p->CBData->configuration->moduleHandlerConfiguration.contextID =
    //        gtk_statusbar_get_context_id (statusbar_p,
    //                                      converter.str ().c_str ());
    //    gdk_threads_leave ();
    //  } // end lock scope

    istream_control_p->start ();
  //    if (!stream_p->isRunning ())
  //    {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to start stream, aborting\n")));
  //      return;
  //    } // end IF
    istream_control_p->wait (true,   // wait for threads ?
                             false,  // wait for upstream ?
                             false); // wait for downstream ?

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif

done:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, -1);
#else
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, std::numeric_limits<void*>::max ());
#endif
    thread_data_p->CBData->progressData.completedActions.insert (thread_data_p->eventSourceId);
  } // end lock scope

  // clean up
  delete thread_data_p; thread_data_p = NULL;

  return result;
}

// -----------------------------------------------------------------------------

gboolean
idle_initialize_ui_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_ui_cb"));

  // sanity check(s)
  struct HTTPGet_UI_CBData* ui_cb_data_p =
    static_cast<struct HTTPGet_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_DIALOG_MAIN)));
  ACE_ASSERT (dialog_p);

  GtkAboutDialog* about_dialog_p =
    GTK_ABOUT_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_DIALOG_ABOUT)));
  ACE_ASSERT (about_dialog_p);

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_CONNECTIONS)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<unsigned int>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_DATAMESSAGES)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<unsigned int>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_SESSIONMESSAGES)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<unsigned int>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_DATA)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<unsigned int>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_BUFFER)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<unsigned int>::max ());
  gtk_spin_button_set_value (spin_button_p,
                             ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize);

  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_ENTRY_URL)));
  ACE_ASSERT (entry_p);
  HTTPGet_StreamConfiguration_t::ITERATOR_T iterator_2 =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != ui_cb_data_p->configuration->streamConfiguration.end ());
  gtk_entry_set_text (entry_p,
                      (*iterator_2).second.second->URL.c_str ());

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_FILECHOOSERBUTTON_SAVE)));
  ACE_ASSERT (file_chooser_button_p);
  //struct _GValue property_s = G_VALUE_INIT;
  //g_value_init (&property_s,
  //              G_TYPE_POINTER);
  //g_object_get_property (G_OBJECT (file_chooser_button_p),
  //                       ACE_TEXT_ALWAYS_CHAR ("dialog"),
  //                       &property_s);
  //G_VALUE_HOLDS_POINTER (&property_s);
  //GtkFileChooser* file_chooser_p = NULL;
    //reinterpret_cast<GtkFileChooser*> (g_value_get_pointer (&property_s));
  //g_object_get (G_OBJECT (file_chooser_button_p),
  //              ACE_TEXT_ALWAYS_CHAR ("dialog"),
  //              &file_chooser_p, NULL);
  //ACE_ASSERT (file_chooser_p);
  //ACE_ASSERT (GTK_IS_FILE_CHOOSER_DIALOG (file_chooser_p));
  //GtkFileChooserDialog* file_chooser_dialog_p =
  //  GTK_FILE_CHOOSER_DIALOG (file_chooser_p);
  //ACE_ASSERT (file_chooser_dialog_p);
  //GtkPlacesSidebar* places_sidebar_p = NULL;
  //Common_UI_Tools::dump (GTK_WIDGET (file_chooser_dialog_p));
  //[0].get_children ()[0].get_children ([0].get_children ()[0]
  //  vbox.get_children ()[0].hide ()

  //GError* error_p = NULL;
  //GFile* file_p = NULL;
  struct _GString* string_p = NULL;
  gchar* filename_p = NULL;
  if (!(*iterator_2).second.second->targetFileName.empty ())
  {
    // *NOTE*: gtk does not complain if the file doesn't exist, but the button
    //         will display "(None)" --> create empty file
    if (!Common_File_Tools::isReadable ((*iterator_2).second.second->targetFileName))
      if (!Common_File_Tools::create ((*iterator_2).second.second->targetFileName))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_File_Tools::create(\"%s\"): \"%m\", aborting\n"),
                    ACE_TEXT ((*iterator_2).second.second->targetFileName.c_str ())));
        return G_SOURCE_REMOVE;
      } // end IF
    //file_p =
    //  g_file_new_for_path (ui_cb_data_p->configuration->moduleHandlerConfiguration.targetFileName.c_str ());
    //ACE_ASSERT (file_p);
    //ACE_ASSERT (g_file_query_exists (file_p, NULL));

    //std::string file_uri =
    //  ACE_TEXT_ALWAYS_CHAR ("file://") +
    //  ui_cb_data_p->configuration->moduleHandlerConfiguration.targetFileName;
    //if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
    //                                              file_uri.c_str ()))
    string_p =
      g_string_new ((*iterator_2).second.second->targetFileName.c_str ());
    filename_p = string_p->str;
      //Common_UI_Tools::Locale2UTF8 (ui_cb_data_p->configuration->moduleHandlerConfiguration.targetFileName);
    if (!gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
                                        filename_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_filename(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT ((*iterator_2).second.second->targetFileName.c_str ())));

      // clean up
      g_string_free (string_p, FALSE);
      g_free (filename_p);

      return G_SOURCE_REMOVE;
    } // end IF
    g_string_free (string_p, FALSE);
    g_free (filename_p);

    //if (!gtk_file_chooser_select_file (GTK_FILE_CHOOSER (file_chooser_dialog_p),
    //                                   file_p,
    //                                   &error_p))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to gtk_file_chooser_select_file(\"%s\"): \"%s\", aborting\n"),
    //              ACE_TEXT (ui_cb_data_p->configuration->moduleHandlerConfiguration.targetFileName.c_str ()),
    //              ACE_TEXT (error_p->message)));

    //  // clean up
    //  g_error_free (error_p);
    //  g_object_unref (file_p);

    //  return G_SOURCE_REMOVE;
    //} // end IF
    //g_object_unref (file_p);
  } // end IF
  else
  {
    //file_p =
    //  g_file_new_for_path (Common_File_Tools::getTempDirectory ().c_str ());
    //ACE_ASSERT (file_p);

    string_p = g_string_new (Common_File_Tools::getTempDirectory ().c_str ());
    filename_p = string_p->str;
      //Common_UI_Tools::Locale2UTF8 (Common_File_Tools::getTempDirectory ());
    if (!gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
                                        filename_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_filename(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (Common_File_Tools::getTempDirectory ().c_str ())));

      // clean up
      g_string_free (string_p, FALSE);
      g_free (filename_p);

      return G_SOURCE_REMOVE;
    } // end IF
    g_string_free (string_p, FALSE);
    g_free (filename_p);
    //g_object_unref (file_p);
  } // end ELSE

  std::string default_folder_uri = ACE_TEXT_ALWAYS_CHAR ("file://");
  default_folder_uri += (*iterator_2).second.second->targetFileName;
  if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
                                                default_folder_uri.c_str ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_uri(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (default_folder_uri.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF

  GtkCheckButton* check_button_p =
//    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                              ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_CHECKBUTTON_ASYNCH)));
//  ACE_ASSERT (check_button_p);
//  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
//                                (ui_cb_data_p->configuration->signalHandlerConfiguration.dispatchState->proactorGroupId != -1));
//  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_CHECKBUTTON_SAVE)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                !(*iterator_2).second.second->targetFileName.empty ());

  // step4: initialize text view, setup auto-scrolling
  //GtkTextView* text_view_p =
  //  GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
  //                                         ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_TEXTVIEW_LOG)));
  //ACE_ASSERT (text_view_p);
//  GtkTextBuffer* buffer_p =
////    gtk_text_buffer_new (NULL); // text tag table --> create new
//      gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);
////  gtk_text_view_set_buffer (view_p, buffer_p);

  //PangoFontDescription* font_description_p =
  //  pango_font_description_from_string (ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_GTK_PANGO_LOG_FONT_DESCRIPTION));
  //if (!font_description_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to pango_font_description_from_string(\"%s\"): \"%m\", aborting\n"),
  //              ACE_TEXT (HTTPGET_UI_GTK_PANGO_LOG_FONT_DESCRIPTION)));
  //  return G_SOURCE_REMOVE;
  //} // end IF
  //// apply font
  //GtkRcStyle* rc_style_p = gtk_rc_style_new ();
  //if (!rc_style_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to gtk_rc_style_new(): \"%m\", aborting\n")));
  //  return G_SOURCE_REMOVE;
  //} // end IF
  //rc_style_p->font_desc = font_description_p;
  //GdkColor base_colour, text_colour;
  //gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_GTK_PANGO_LOG_COLOR_BASE),
  //                 &base_colour);
  //rc_style_p->base[GTK_STATE_NORMAL] = base_colour;
  //gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_GTK_PANGO_LOG_COLOR_TEXT),
  //                 &text_colour);
  //rc_style_p->text[GTK_STATE_NORMAL] = text_colour;
  //rc_style_p->color_flags[GTK_STATE_NORMAL] =
  //  static_cast<GtkRcFlags> (GTK_RC_BASE |
  //                           GTK_RC_TEXT);
  //gtk_widget_modify_style (GTK_WIDGET (text_view_p),
  //                         rc_style_p);
  ////gtk_rc_style_unref (rc_style_p);
  //g_object_unref (rc_style_p);

  //  GtkTextIter iterator;
  //  gtk_text_buffer_get_end_iter (buffer_p,
  //                                &iterator);
  //  gtk_text_buffer_create_mark (buffer_p,
  //                               ACE_TEXT_ALWAYS_CHAR (NET_UI_SCROLLMARK_NAME),
  //                               &iterator,
  //                               TRUE);
  //  g_object_unref (buffer_p);

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_PROGRESSBAR)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
                                   1.0 / static_cast<double> (width));

  GtkStatusbar* statusbar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_STATUSBAR)));
  ACE_ASSERT (statusbar_p);
  guint context_id =
      gtk_statusbar_get_context_id (statusbar_p,
                                    ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_STATUSBAR_CONTEXT_DATA));
  state_r.contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_DATA,
                                             context_id));
  context_id =
    gtk_statusbar_get_context_id (statusbar_p,
                                  ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_STATUSBAR_CONTEXT_INFORMATION));
  state_r.contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_INFORMATION,
                                             context_id));

  // step2: (auto-)connect signals/slots
  gtk_builder_connect_signals ((*iterator).second.second,
                               ui_cb_data_p);

  gulong result =
    g_signal_connect (G_OBJECT (dialog_p),
                      ACE_TEXT_ALWAYS_CHAR ("destroy"),
                      G_CALLBACK (button_quit_clicked_cb),
                      ui_cb_data_p);
//                   G_CALLBACK(gtk_widget_destroyed),
//                   &main_dialog_p,
//  g_signal_connect (G_OBJECT (dialog_p),
//                    ACE_TEXT_ALWAYS_CHAR ("delete-event"),
//                    G_CALLBACK (delete_event_cb),
//                    NULL);
  ACE_ASSERT (result);

  result = g_signal_connect_swapped (G_OBJECT (about_dialog_p),
                                     ACE_TEXT_ALWAYS_CHAR ("response"),
                                     G_CALLBACK (gtk_widget_hide),
                                     about_dialog_p);
  ACE_ASSERT (result);

  // step4: connect custom signals
  //--------------------------------------

  //g_signal_connect (drawing_area_p,
  //                  ACE_TEXT_ALWAYS_CHAR ("key-press-event"),
  //                  G_CALLBACK (key_cb),
  //                  ui_cb_data_p);
  //g_signal_connect (drawing_area_p,
  //                  ACE_TEXT_ALWAYS_CHAR ("motion-notify-event"),
  //                  G_CALLBACK (motion_cb),
  //                  ui_cb_data_p);

//  // step5: use correct screen
//  if (parentWidget_in)
//    gtk_window_set_screen (GTK_WINDOW (dialog),
//                           gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step6: draw main window
  gtk_widget_show_all (GTK_WIDGET (dialog_p));

  // step7: initialize updates
  guint event_source_id = 0;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the log view
    //event_source_id = g_timeout_add_seconds (1,
    //                                         idle_update_log_display_cb,
    //                                         ui_cb_data_p);
    //if (event_source_id > 0)
    //  ui_cb_data_p->UIState.eventSourceIds.insert (event_source_id);
    //else
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
    //  return G_SOURCE_REMOVE;
    //} // end ELSE
    // schedule asynchronous updates of the info view
    event_source_id =
        g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET_MS,
                       idle_update_info_display_cb,
                       ui_cb_data_p);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step7: initialize fps, schedule refresh
  //ui_cb_data_p->timeStamp = COMMON_TIME_POLICY ();
  //guint opengl_refresh_rate =
  //    static_cast<guint> (HTTPGET_UI_WIDGET_GL_REFRESH_INTERVAL);
  //ui_cb_data_p->openGLRefreshId = g_timeout_add (opengl_refresh_rate,
  //                                            process_cb,
  //                                            ui_cb_data_p);
  //if (!ui_cb_data_p->openGLRefreshId)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
  //  return G_SOURCE_REMOVE;
  //} // end IF
  //ui_cb_data_p->openGLRefreshId = g_idle_add_full (10000,
  //                                              process_cb,
  //                                              ui_cb_data_p,
  //                                              NULL);
  //if (ui_cb_data_p->openGLRefreshId == 0)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to g_idle_add_full(): \"%m\", aborting\n")));
  //  return G_SOURCE_REMOVE;
  //} // end IF
  //else
  //  ui_cb_data_p->UIState.eventSourceIds.insert (ui_cb_data_p->openGLRefreshId);

  // step9: activate some widgets
  if ((*iterator_2).second.second->targetFileName.empty ())
  {
    GtkFrame* frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_FRAME_SAVE)));
    ACE_ASSERT (frame_p);
    gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
                              false);
  } // end IF

  ///* Get Icons shown on buttons */
  //GtkSettings* settings_p = gtk_settings_get_default ();
  //ACE_ASSERT (settings_p);
  //gtk_settings_set_long_property (settings_p,
  //                                ACE_TEXT_ALWAYS_CHAR ("gtk-button-images"),
  //                                TRUE,
  //                                ACE_TEXT_ALWAYS_CHAR ("main"));

  // one-shot action
  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_ui_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_finalize_ui_cb"));

  ACE_UNUSED_ARG (userData_in);

  //// sanity check(s)
  //struct HTTPGet_UI_CBData* ui_cb_data_p =
  //    static_cast<struct HTTPGet_UI_CBData*> (userData_in);
  //ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  state_r.eventSourceIds.clear ();

  gtk_main_quit ();

  // one-shot action
  return G_SOURCE_REMOVE;
}

gboolean
idle_session_end_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_session_end_cb"));

  // sanity check(s)
  struct HTTPGet_UI_CBData* ui_cb_data_p =
    static_cast<struct HTTPGet_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // *IMPORTANT NOTE*: there are two major reasons for being here that are not
  //                   mutually exclusive, so there could be a race:
  //                   - user pressed stop
  //                   - there was an asynchronous error on the stream
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_BUTTON_EXECUTE)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), true);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_BUTTON_CANCEL)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), false);

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_FRAME_URL)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);
  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_FRAME_SAVE)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);
  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_FRAME_OPTIONS)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

  // stop progress reporting ?
  if (ui_cb_data_p->progressData.eventSourceId)
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    if (!g_source_remove (ui_cb_data_p->progressData.eventSourceId))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                  ui_cb_data_p->progressData.eventSourceId));
    state_r.eventSourceIds.erase (ui_cb_data_p->progressData.eventSourceId);
    ui_cb_data_p->progressData.eventSourceId = 0;
  } // end lock scope

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_PROGRESSBAR)));
  ACE_ASSERT (progress_bar_p);
  // *NOTE*: this disables "activity mode" (in Gtk2)
  gtk_progress_bar_set_fraction (progress_bar_p, 0.0);
  //gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));
  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), false);

  return G_SOURCE_REMOVE;
}
gboolean
idle_session_start_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_session_start_cb"));

  // sanity check(s)
  struct HTTPGet_UI_CBData* ui_cb_data_p =
    static_cast<struct HTTPGet_UI_CBData*> (userData_in);
  ACE_UNUSED_ARG (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // update widgets
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_BUTTON_CANCEL)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), true);

  // reset counters
  GtkSpinButton* spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_SESSIONMESSAGES)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_DATAMESSAGES)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);

  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_DATA)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  // sanity check(s)
  struct HTTPGet_UI_CBData* ui_cb_data_p =
    static_cast<struct HTTPGet_UI_CBData*> (userData_in);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Common_UI_EventType* event_p = NULL;
  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (state_r.eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
//    for (Common_UI_Events_t::ITERATOR iterator_2 (state_r.eventStack);
//         !iterator_2.done ();
//         iterator_2.next (event_p))
    { ACE_ASSERT (event_p);
      spin_button_p = NULL;
      switch (*event_p)
      {
        case COMMON_UI_EVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_DATA)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (ui_cb_data_p->progressData.statistic.bytes));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_DATAMESSAGES)));
          ACE_ASSERT (spin_button_p);

          break;
        }
        case COMMON_UI_EVENT_SESSION:
        case COMMON_UI_EVENT_FINISHED:
        case COMMON_UI_EVENT_STARTED:
        case COMMON_UI_EVENT_ABORT:
        case COMMON_UI_EVENT_STEP:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_SESSIONMESSAGES)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_CONNECT:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_CONNECTIONS)));
          if (spin_button_p) // target ?
            gtk_spin_button_spin (spin_button_p,
                                  GTK_SPIN_STEP_FORWARD,
                                  1.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_SESSIONMESSAGES)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_DISCONNECT:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_CONNECTIONS)));
          gtk_spin_button_spin (spin_button_p,
                                GTK_SPIN_STEP_BACKWARD,
                                1.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_SESSIONMESSAGES)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown event type (was: %d), continuing\n"),
                      *event_p));
          break;
        }
      } // end SWITCH
      ACE_UNUSED_ARG (is_session_message);
      gtk_spin_button_spin (spin_button_p,
                            GTK_SPIN_STEP_FORWARD,
                            1.0);
      event_p = NULL;
    } // end FOR

    // clean up
    while (!state_r.eventStack.is_empty ())
    {
      result = state_r.eventStack.pop (event_e);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Unbounded_Stack::pop(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);

  // sanity check(s)
  struct HTTPGet_ProgressData* progress_data_p =
    static_cast<struct HTTPGet_ProgressData*> (userData_in);
  ACE_ASSERT (progress_data_p->state);
  Common_UI_GTK_BuildersConstIterator_t iterator =
    progress_data_p->state->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != progress_data_p->state->builders.end ());
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_PROGRESSBAR)));
  ACE_ASSERT (progress_bar_p);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result = -1;
  float fps, speed = 0.0F;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, progress_data_p->state->lock, G_SOURCE_REMOVE);
    fps   = progress_data_p->statistic.messagesPerSecond;
    speed = progress_data_p->statistic.bytesPerSecond;
  } // end lock scope
  std::string magnitude_string = ACE_TEXT_ALWAYS_CHAR ("byte(s)/s");
  if (speed)
  {
    if (speed >= 1024.0F)
    {
      speed /= 1024.0F;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("kbyte(s)/s");
    } // end IF
    if (speed >= 1024.0F)
    {
      speed /= 1024.0F;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("mbyte(s)/s");
    } // end IF
    result = ACE_OS::sprintf (buffer, ACE_TEXT ("%.0f fps | %.2f %s"),
                              fps, speed, magnitude_string.c_str ());
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", continuing\n")));
  } // end IF
  gtk_progress_bar_set_text (progress_bar_p,
                             ACE_TEXT_ALWAYS_CHAR (buffer));
  gtk_progress_bar_pulse (progress_bar_p);

  // --> reschedule
  return G_SOURCE_CONTINUE;
}

/////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
G_MODULE_EXPORT void
button_execute_clicked_cb (GtkButton* button_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_execute_clicked_cb"));

  // sanity check(s)
  struct HTTPGet_UI_CBData* ui_cb_data_p =
    static_cast<struct HTTPGet_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkSpinButton* spin_button_p = NULL;
  GtkFileChooserButton* file_chooser_button_p = NULL;
  gchar* URI_p = NULL;
  GError* error_p = NULL;
  gchar* hostname_p = NULL;
  gchar* directory_p = NULL;
  std::string directory_string;
  GtkCheckButton* check_button_p = NULL;
  GtkFrame* frame_p = NULL;
  GtkProgressBar* progress_bar_p = NULL;

  bool stop_progress_reporting = false;

  struct HTTPGet_UI_ThreadData* thread_data_p = NULL;
  ACE_thread_t thread_id = -1;
  ACE_hthread_t thread_handle;
  char thread_name[BUFSIZ];
  const char* thread_name_p = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;
  int result = -1;
//  Stream_IStreamControlBase* stream_p = NULL;
  HTTPGet_StreamConfiguration_t::ITERATOR_T iterator_2 =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != ui_cb_data_p->configuration->streamConfiguration.end ());
  Net_ConnectionConfigurationsIterator_t iterator_3 =
    ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != ui_cb_data_p->configuration->connectionConfigurations.end ());

  // update configuration

  // retrieve address
  GtkEntry* entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_ENTRY_URL)));
  ACE_ASSERT (entry_p);
  (*iterator_2).second.second->URL = gtk_entry_get_text (entry_p);
  // step1: parse URL
  ACE_INET_Addr host_address;
  std::string hostname_string, URI_string;
  bool use_SSL = false;
  if (!HTTP_Tools::parseURL ((*iterator_2).second.second->URL,
                             host_address,
                             hostname_string,
                             URI_string,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                ACE_TEXT ((*iterator_2).second.second->URL.c_str ())));
    return;
  } // end IF
  std::string hostname_string_2 = hostname_string;
  size_t position =
    hostname_string_2.find_last_of (':', std::string::npos);
  if (position == std::string::npos)
  {
    hostname_string_2 += ':';
    std::ostringstream converter;
    converter << (use_SSL ? HTTPS_DEFAULT_SERVER_PORT
                          : HTTP_DEFAULT_SERVER_PORT);
    hostname_string_2 += converter.str ();
  } // end IF
  result =
    NET_CONFIGURATION_TCP_CAST ((*iterator_3).second)->socketConfiguration.address.set (hostname_string_2.c_str (),
                                                                                        AF_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (hostname_string_2.c_str ())));
    return;
  } // end IF
  NET_CONFIGURATION_TCP_CAST ((*iterator_3).second)->socketConfiguration.hostname =
    hostname_string;
  NET_CONFIGURATION_TCP_CAST ((*iterator_3).second)->socketConfiguration.useLoopBackDevice =
    NET_CONFIGURATION_TCP_CAST ((*iterator_3).second)->socketConfiguration.address.is_loopback ();

  // sanity check(s)
  if ((ui_cb_data_p->dispatchState.configuration->dispatch == COMMON_EVENT_DISPATCH_PROACTOR) &&
      use_SSL)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("URL: \"%s\", requires SSL, but asynchronous SSL support is not implemented yet (--> use reactor), continuing\n"),
                ACE_TEXT ((*iterator_2).second.second->URL.c_str ())));
//    return;
  } // end IF

  // save to file ?
  check_button_p =
      GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_CHECKBUTTON_SAVE)));
  ACE_ASSERT (check_button_p);
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_p)))
  {
    (*iterator_2).second.second->targetFileName.clear ();
    goto continue_;
  } // end IF

  // retrieve filename
  file_chooser_button_p =
      GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_FILECHOOSERBUTTON_SAVE)));
  ACE_ASSERT (file_chooser_button_p);
  URI_p =
      gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (file_chooser_button_p));
  if (!URI_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_get_uri(), returning\n")));
    goto error;
  } // end IF
  directory_p = g_filename_from_uri (URI_p,
                                     &hostname_p,
                                     &error_p);
  g_free (URI_p);
  if (!directory_p)
  { ACE_ASSERT (error_p);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_filename_from_uri(): \"%s\", returning\n"),
                ACE_TEXT (error_p->message)));

    // clean up
    g_error_free (error_p);

    goto error;
  } // end IF
  ACE_ASSERT (!hostname_p);
  ACE_ASSERT (!error_p);
  directory_string =
    ACE_TEXT_ALWAYS_CHAR (ACE::dirname (directory_p,
                                        ACE_DIRECTORY_SEPARATOR_CHAR));
  g_free (directory_p);
  ACE_ASSERT (Common_File_Tools::isDirectory (directory_string));
  (*iterator_2).second.second->targetFileName = directory_string;
  (*iterator_2).second.second->targetFileName += ACE_DIRECTORY_SEPARATOR_STR;
  (*iterator_2).second.second->targetFileName +=
    ACE_TEXT_ALWAYS_CHAR (HTTP_GET_DEFAULT_OUTPUT_FILE);

continue_:
  // update widgets
  gtk_widget_set_sensitive (GTK_WIDGET (button_in), false);

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_SESSIONMESSAGES)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                             0.0);
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_DATAMESSAGES)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                             0.0);
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_SPINBUTTON_DATA)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                             0.0);

  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_FRAME_URL)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), false);
  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_FRAME_SAVE)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), false);
  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_FRAME_OPTIONS)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), false);

  progress_bar_p =
      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_PROGRESSBAR)));
  ACE_ASSERT (progress_bar_p);
  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), true);

  // start processing thread
  ACE_NEW_NORETURN (thread_data_p,
                    struct HTTPGet_UI_ThreadData ());
  if (!thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  thread_data_p->CBData = ui_cb_data_p;

  ACE_OS::memset (thread_name, 0, sizeof (char[BUFSIZ]));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_OS::strncpy (thread_name,
                   ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_THREAD_NAME),
                   std::min (static_cast<size_t> (BUFSIZ - 1), static_cast<size_t> (ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_THREAD_NAME)))));
#else
  ACE_ASSERT (COMMON_THREAD_PTHREAD_NAME_MAX_LENGTH <= BUFSIZ);
  ACE_OS::strncpy (thread_name,
                   ACE_TEXT (TEST_U_STREAM_THREAD_NAME),
                   std::min (static_cast<size_t> (COMMON_THREAD_PTHREAD_NAME_MAX_LENGTH - 1), static_cast<size_t> (ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_THREAD_NAME)))));
#endif // ACE_WIN32 || ACE_WIN64
  thread_name_p = thread_name;
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  result =
      thread_manager_p->spawn (::stream_processing_function,     // function
                               thread_data_p,                    // argument
                               (THR_NEW_LWP      |
                                THR_JOINABLE     |
                                THR_INHERIT_SCHED),              // flags
                               &thread_id,                       // id
                               &thread_handle,                   // handle
                               ACE_DEFAULT_THREAD_PRIORITY,      // priority
                               COMMON_EVENT_REACTOR_THREAD_GROUP_ID + 1, // *TODO*: group id
                               NULL,                             // stack
                               0,                                // stack size
                               &thread_name_p);                  // name
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::spawn(): \"%m\", returning\n")));
    goto error;
  } // end IF

  // start progress reporting
  // *TODO*: there is a race condition here if the processing thread returns
  //         early
  ACE_ASSERT (!ui_cb_data_p->progressData.eventSourceId);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    ui_cb_data_p->progressData.eventSourceId =
        //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
        //                 idle_update_progress_cb,
        //                 &ui_cb_data_p->progressData,
        //                 NULL);
        g_timeout_add (//G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                       COMMON_UI_REFRESH_DEFAULT_PROGRESS_MS, // ms (?)
                       idle_update_progress_cb,
                       &ui_cb_data_p->progressData);//,
                       //NULL);
    if (ui_cb_data_p->progressData.eventSourceId > 0)
    {
      thread_data_p->eventSourceId = ui_cb_data_p->progressData.eventSourceId;
      ui_cb_data_p->progressData.pendingActions[ui_cb_data_p->progressData.eventSourceId] =
          ACE_Thread_ID (thread_id, thread_handle);
      state_r.eventSourceIds.insert (ui_cb_data_p->progressData.eventSourceId);
    } // end IF
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", continuing\n")));
    thread_data_p = NULL;
  } // end lock scope
  stop_progress_reporting = true;

  return;

error:
  // update widgets
  gtk_widget_set_sensitive (GTK_WIDGET (button_in), true);

  frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_FRAME_URL)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);
  frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_FRAME_SAVE)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);
  frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_FRAME_OPTIONS)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

  progress_bar_p =
      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_PROGRESSBAR)));
  ACE_ASSERT (progress_bar_p);
  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), false);

  if (stop_progress_reporting)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    ui_cb_data_p->progressData.completedActions.insert (ui_cb_data_p->progressData.eventSourceId);
  } // end IF

  if (thread_data_p)
    delete thread_data_p;
}

G_MODULE_EXPORT void
button_cancel_clicked_cb (GtkButton* button_in,
                          gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_cancel_clicked_cb"));

  // sanity check(s)
//  struct HTTPGet_UI_CBData* ui_cb_data_p =
//      static_cast<struct HTTPGet_UI_CBData*> (userData_in);
//  ACE_ASSERT (ui_cb_data_p);
}

//G_MODULE_EXPORT void
//entry_url_delete_text_cb (GtkEditable* editable_in,
//                          gint startPosition_in,
//                          gint endPosition_in,
//                          gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::entry_url_delete_text_cb"));
//
//  ACE_UNUSED_ARG (userData_in);
//
//  gchar* string_p = NULL;
//  std::string entry_string;
//  std::string address_string;
//  guint num_handlers = 0;
//
//  string_p = gtk_editable_get_chars (editable_in, 0, -1);
//  if (!string_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gtk_editable_get_chars(), returning\n")));
//    goto refuse;
//  } // end IF
//  entry_string = string_p;
//  g_free (string_p);
//
//  // validate edited string as a whole
//  address_string = entry_string;
//  if (endPosition_in == -1)
//    address_string.erase (startPosition_in, std::string::npos);
//  else
//    address_string.erase (startPosition_in,
//                          (endPosition_in - startPosition_in));
//
//  if (!Net_Common_Tools::matchIPAddress (address_string))
//    goto refuse;
//
//  // delete
//  num_handlers =
//    g_signal_handlers_block_by_func (editable_in,
//                                     (gpointer)entry_url_delete_text_cb,
//                                     userData_in);
//  ACE_ASSERT (num_handlers == 1);
//  gtk_editable_delete_text (editable_in,
//                            startPosition_in,
//                            endPosition_in);
//  num_handlers =
//    g_signal_handlers_unblock_by_func (editable_in,
//                                       (gpointer)entry_url_delete_text_cb,
//                                       userData_in);
//  ACE_ASSERT (num_handlers == 1);
//
//refuse:
//  g_signal_stop_emission_by_name (editable_in,
//                                  ACE_TEXT_ALWAYS_CHAR ("delete-text"));
//}
//G_MODULE_EXPORT void
//entry_url_insert_text_cb (GtkEditable* editable_in,
//                          gchar* newText_in,
//                          gint newTextLength_in,
//                          gpointer position_inout,
//                          gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::entry_url_insert_text_cb"));
//
//  ACE_UNUSED_ARG (userData_in);
//
//  gchar* string_p = NULL;
//  std::string entry_string;
//  std::stringstream converter;
//  std::string group_string;
//  unsigned int group = 0;
//  std::string::size_type start_position = std::string::npos;
//  std::string::size_type insert_position =
//    *static_cast<gint*> (position_inout);
//  bool is_digit = false;
//  guint number_of_handlers = 0;
//
//  string_p = gtk_editable_get_chars (editable_in, 0, -1);
//  if (!string_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gtk_editable_get_chars(), returning\n")));
//    goto refuse;
//  } // end IF
//  entry_string = string_p;
//  g_free (string_p);
//
//  // sanity check(s)
//  ACE_ASSERT (newTextLength_in);
//  if ((newTextLength_in != 1) ||
//      ((newTextLength_in == -1) &&
//       (ACE_OS::strlen (newText_in) > 1)))
//  {
//    // validate string as a whole
//    // *TODO*: support partial inserts...
//    std::string address_string = entry_string;
//    if (newTextLength_in == -1)
//      address_string.insert (*(gint*)position_inout,
//                             newText_in);
//    else
//      address_string.insert (*(gint*)position_inout,
//                             newText_in,
//                             newTextLength_in);
//
//    if (!Net_Common_Tools::matchIPAddress (address_string))
//      goto refuse;
//
//    goto accept;
//  } // end IF
//
//  // validate single character
//  is_digit = !!::isdigit (*newText_in);
//  if (!is_digit && !(*newText_in == '.'))
//  {
//    //ACE_DEBUG ((LM_DEBUG,
//    //            ACE_TEXT ("invalid input (was: '%c'), returning\n"),
//    //            *newText_in));
//    goto refuse;
//  } // end IF
//
//  if (is_digit)
//  { // rules:
//    // - group must not have leading 0s, unless group IS 0
//    // - group must be <= 255
//    std::string::size_type end_position = std::string::npos;
//
//    // find group
//    start_position = entry_string.rfind ('.', insert_position);
//    if (start_position == std::string::npos) start_position = 0;
//    //else ++start_position;
//    end_position = entry_string.find ('.', insert_position);
//    group_string =
//      entry_string.substr (start_position,
//                           ((end_position == std::string::npos) ? std::string::npos
//                                                                : (end_position - start_position)));
//
//    if (group_string.empty ())
//      goto accept; // group (currently) empty
//    converter << group_string;
//    converter >> group;
//    insert_position = *(gint*)position_inout - start_position;
//    if (*newText_in == '0')
//      if (!group || !insert_position)
//        goto refuse; // refuse leading 0s
//
//    group_string.insert (insert_position, 1, *newText_in);
//    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
//    converter.clear ();
//    converter << group_string;
//    converter >> group;
//    if (group > 255)
//      goto refuse; // refuse groups > 255
//  } // end IF
//  else
//  {
//    // rules:
//    // - any preceding/trailing digits must form a group
//    // - total number of dots ('.') <= 3
//    std::string::size_type position = insert_position;
//    while (position)
//    {
//      if (::isdigit (entry_string[--position]))
//        group_string.insert (0, 1, entry_string[position]);
//      else
//        break;
//    } // end WHILE
//    converter << group_string;
//    converter >> group;
//    if (group > 255)
//      goto refuse; // refuse groups > 255
//    group_string.clear ();
//    position = insert_position;
//    do
//    {
//      if (::isdigit (entry_string[position]))
//        group_string.push_back (entry_string[position]);
//      else
//        break;
//      ++position;
//    } while (true);
//    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
//    converter.clear ();
//    converter << group_string;
//    converter >> group;
//    if (group > 255)
//      goto refuse; // refuse groups > 255
//
//    if (std::count (entry_string.begin (), entry_string.end (), '.') > 3)
//      goto refuse; // refuse more than 3 '.'
//  } // end ELSE
//
//accept:
//  // insert
//  number_of_handlers =
//    g_signal_handlers_block_by_func (editable_in,
//                                     (gpointer)entry_address_insert_text_cb,
//                                     userData_in);
//  ACE_ASSERT (number_of_handlers == 1);
//  gtk_editable_insert_text (editable_in,
//                            newText_in,
//                            newTextLength_in,
//                            (gint*)position_inout);
//  number_of_handlers =
//    g_signal_handlers_unblock_by_func (editable_in,
//                                       (gpointer)entry_address_insert_text_cb,
//                                       userData_in);
//  ACE_ASSERT (number_of_handlers == 1);
//
//refuse:
//  g_signal_stop_emission_by_name (editable_in,
//                                  ACE_TEXT_ALWAYS_CHAR ("insert-text"));
//}

G_MODULE_EXPORT void
checkbutton_save_toggled_cb (GtkToggleButton* toggleButton_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::checkbutton_save_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
//  struct HTTPGet_UI_CBData* ui_cb_data_p =
//      static_cast<struct HTTPGet_UI_CBData*> (userData_in);
//  ACE_ASSERT (ui_cb_data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_FRAME_SAVE)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
                            gtk_toggle_button_get_active (toggleButton_in));
}

G_MODULE_EXPORT void
button_about_clicked_cb (GtkButton* button_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
//  struct HTTPGet_UI_CBData* ui_cb_data_p =
//      static_cast<struct HTTPGet_UI_CBData*> (userData_in);
//  ACE_ASSERT (ui_cb_data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  // retrieve about dialog handle
  GtkWidget* about_dialog =
      GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_WIDGET_NAME_DIALOG_ABOUT)));
  ACE_ASSERT (about_dialog);
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (HTTPGET_UI_WIDGET_NAME_DIALOG_ABOUT)));
    return;
  } // end IF

  // draw it
#if GTK_CHECK_VERSION(3,8,0)
  if (!gtk_widget_is_visible (about_dialog))
#else
  if (!gtk_widget_get_visible (about_dialog))
#endif // GTK_CHECK_VERSION(3,8,0)
    gtk_widget_show_all (about_dialog);
}

G_MODULE_EXPORT void
button_quit_clicked_cb (GtkButton* button_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  ACE_UNUSED_ARG (button_in);
  ACE_UNUSED_ARG (userData_in);

  // this is the "delete-event" / "destroy" handler
  // --> destroy the main dialog widget
  int result = ACE_OS::raise (SIGINT);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(SIGINT): \"%m\", continuing\n")));

  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false,  // wait ?
                                                      false); // N/A
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
