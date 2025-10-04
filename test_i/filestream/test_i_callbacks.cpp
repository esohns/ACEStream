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

#include "test_i_callbacks.h"

#include <limits>
#include <sstream>

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_file_tools.h"

#include "common_log_common.h"

#include "common_timer_manager.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"

#include "stream_macros.h"

#include "stream_file_defines.h"

#include "net_connection_configuration.h"

#include "test_i_common.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

#include "test_i_connection_common.h"
#include "test_i_filestream_defines.h"
#include "test_i_source_common.h"
#include "test_i_target_listener_common.h"

// initialize statics
static bool un_toggling_play_pause = false;

ACE_THR_FUNC_RETURN
stream_processing_function (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_processing_function"));

  // sanity check(s)
  ACE_ASSERT (arg_in);

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = std::numeric_limits<unsigned long>::max ();
#else
  result = arg_in;
#endif

  struct Test_I_Source_UI_ThreadData* thread_data_p =
    static_cast<struct Test_I_Source_UI_ThreadData*> (arg_in);
  // sanity check(s)
  ACE_ASSERT (thread_data_p->CBData);
  ACE_ASSERT (thread_data_p->CBData->configuration);
  ACE_ASSERT (thread_data_p->CBData->stream);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  GtkSpinButton* spin_button_p = NULL;
  GtkStatusbar* statusbar_p = NULL;
  Stream_IStream_t* istream_p = NULL;
  Stream_IStreamControlBase* istream_control_p = NULL;
  Common_IInitialize_T<Test_I_Source_StreamConfiguration_t>* iinitialize_p =
    NULL;
  Common_IGetR_2_T<struct Test_I_Source_SessionData>* iget_p = NULL;
  std::ostringstream converter;
  const struct Test_I_Source_SessionData* session_data_p = NULL;
  unsigned int counter = 0;
  bool loop = thread_data_p->CBData->loop;
  Test_I_Source_StreamConfiguration_t::ITERATOR_T iterator_2;
  guint context_id = 0;
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
  bool leave_gdk = true;
#endif // GTK_CHECK_VERSION (3,6,0)

  //{ ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, -1);
    Common_UI_GTK_BuildersIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
    // sanity check(s)
    ACE_ASSERT (iterator != state_r.builders.end ());

    iterator_2 =
      thread_data_p->CBData->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (iterator_2 != thread_data_p->CBData->configuration->streamConfiguration.end ());

    // retrieve stream handle
    switch (thread_data_p->CBData->configuration->protocol)
    {
      case NET_TRANSPORTLAYER_TCP:
      {
        istream_p = thread_data_p->CBData->stream;
        istream_control_p = thread_data_p->CBData->stream;
        //(*iterator_2).second.second->stream = istream_p;
        iinitialize_p = thread_data_p->CBData->stream;
        iget_p = thread_data_p->CBData->stream;
        break;
      }
      case NET_TRANSPORTLAYER_UDP:
      {
        istream_p = thread_data_p->CBData->UDPStream;
        istream_control_p = thread_data_p->CBData->UDPStream;
        //(*iterator_2).second.second->stream = istream_p;
        iinitialize_p = thread_data_p->CBData->UDPStream;
        iget_p = thread_data_p->CBData->UDPStream;
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown protocol (was: %d), returning\n"),
                    thread_data_p->CBData->configuration->protocol));
        goto done;
      }
    } // end SWITCH
    ACE_ASSERT (istream_p);
    ACE_ASSERT (istream_control_p);
    ACE_ASSERT (iinitialize_p);
    ACE_ASSERT (iget_p);

    // retrieve spin button handle
    spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_LOOP_NAME)));
    ACE_ASSERT (spin_button_p);

    // retrieve status bar handle
    statusbar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_STATUSBAR_NAME)));
    ACE_ASSERT (statusbar_p);
  //} // end lock scope

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
  leave_gdk = false;
#endif // GTK_CHECK_VERSION (3,6,0)

loop:
  if (!iinitialize_p->initialize (thread_data_p->CBData->configuration->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream (name was: \"%s\"), aborting\n"),
                ACE_TEXT (istream_p->name ().c_str ())));
    goto done;
  } // end IF

  // *TODO*: this is too early; the session id is generated/incremented in Stream_Base::start() !
  session_data_p = &iget_p->getR_2 ();
  ACE_ASSERT (session_data_p);
  converter.clear ();
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter << session_data_p->sessionId;

  // generate context id
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
  context_id =
    gtk_statusbar_get_context_id (statusbar_p,
                                  converter.str ().c_str ());
  state_r.contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_DATA,
                                             context_id));
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)

  // *NOTE*: processing currently happens 'inline' (borrows calling thread)
  istream_control_p->start ();
  //    if (!stream_p->isRunning ())
  //    {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to start stream, aborting\n")));
  //      return;
  //    } // end IF
  istream_control_p->wait (true,
                           false,
                           false);

  ++counter;
  if (loop)
  {
    if (static_cast<int> (thread_data_p->CBData->loop) != -1)
    {
#if GTK_CHECK_VERSION (3,6,0)
#else
      gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
      gtk_spin_button_spin (spin_button_p,
                            GTK_SPIN_STEP_BACKWARD,
                            1.0);
#if GTK_CHECK_VERSION (3,6,0)
#else
      gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
    }  // end IF

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("iteration #%u complete\n"),
                counter));
    if (thread_data_p->CBData->loop)
      goto loop;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif

done:
#if GTK_CHECK_VERSION (3,6,0)
#else
  if (leave_gdk)
    gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)

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

//////////////////////////////////////////

gboolean
idle_initialize_source_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_source_UI_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);

  struct Test_I_Source_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Source_UI_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  //Common_UI_GladeXMLsIterator_t iterator =
  //  thread_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //// sanity check(s)
  //ACE_ASSERT (iterator != thread_data_p->gladeXML.end ());
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
  //  GTK_WIDGET (glade_xml_get_widget ((*iterator).second.second,
  //                                    ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_MAIN_NAME)));
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  //  GtkWidget* image_icon_p = gtk_image_new_from_file (path.c_str ());
  //  ACE_ASSERT (image_icon_p);
  //  gtk_window_set_icon (GTK_WINDOW (dialog_p),
  //                       gtk_image_get_pixbuf (GTK_IMAGE (image_icon_p)));
  //GdkWindow* dialog_window_p = gtk_widget_get_window (dialog_p);
  //gtk_window4096_set_title (,
  //                      caption.c_str ());

//  GtkWidget* about_dialog_p =
//    //GTK_WIDGET (glade_xml_get_widget ((*iterator).second.second,
//    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
//    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
//                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
//  ACE_ASSERT (about_dialog_p);

  GtkSpinButton* spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT64>::max ()));

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FILECHOOSERBUTTON_OPEN_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  Test_I_Source_StreamConfiguration_t::ITERATOR_T iterator_2 =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != ui_cb_data_p->configuration->streamConfiguration.end ());
  std::string path =
      ((*iterator_2).second.second->fileIdentifier.identifier.empty () ? Common_File_Tools::getHomeDirectory (ACE_TEXT_ALWAYS_CHAR (""))
                                                                       : (*iterator_2).second.second->fileIdentifier.identifier);
  GFile* file_p = g_file_new_for_path (path.c_str ());
  ACE_ASSERT (file_p);
  GError* error_p = NULL;
  if (!gtk_file_chooser_set_file (GTK_FILE_CHOOSER (file_chooser_button_p),
                                  file_p,
                                  &error_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_file(): \"%s\", aborting\n"),
                ACE_TEXT (error_p->message)));

    // clean up
    g_error_free (error_p);
    g_object_unref (file_p);

    return G_SOURCE_REMOVE;
  } // end IF
  g_object_unref (file_p);

  GtkEntry* entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ENTRY_DESTINATION_NAME)));
  ACE_ASSERT (entry_p);
  Net_ConnectionConfigurationsIterator_t iterator_3 =
    ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != ui_cb_data_p->configuration->connectionConfigurations.end ());
  ACE_INET_Addr address_s =
    NET_CONFIGURATION_TCP_CAST ((*iterator_3).second)->socketConfiguration.address;
  gtk_entry_set_text (entry_p,
                      address_s.get_host_name ());

  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                              static_cast<double> (address_s.get_port_number ()));

  GtkRadioButton* radio_button_p = NULL;
  if (ui_cb_data_p->configuration->protocol == NET_TRANSPORTLAYER_UDP)
  {
    radio_button_p =
      GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_UDP_NAME)));
    ACE_ASSERT (radio_button_p);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_button_p), TRUE);
  } // end IF
  GtkCheckButton* check_button_p =
      GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_CHECKBUTTON_ASYNCH_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                (ui_cb_data_p->configuration->dispatchConfiguration.numberOfProactorThreads > 0));
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_CHECKBUTTON_LOOPBACK_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                NET_CONFIGURATION_TCP_CAST ((*iterator_3).second)->socketConfiguration.useLoopBackDevice);

  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_LOOP_NAME)));
  ACE_ASSERT(spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             -1.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<double> (ui_cb_data_p->loop));

  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<double> (ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize));

  // step4: initialize text view, setup auto-scrolling
  GtkTextView* view_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
//  GtkTextBuffer* buffer_p =
////    gtk_text_buffer_new (NULL); // text tag table --> create new
//      gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);
////  gtk_text_view_set_buffer (view_p, buffer_p);

  struct _PangoFontDescription* font_description_p =
    pango_font_description_from_string (ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION));
  if (!font_description_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pango_font_description_from_string(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_I_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION)));
    return G_SOURCE_REMOVE;
  } // end IF
  // apply font
  struct _GtkRcStyle* rc_style_p = gtk_rc_style_new ();
  if (!rc_style_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_rc_style_new(): \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  rc_style_p->font_desc = font_description_p;
  struct _GdkColor base_colour, text_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PANGO_LOG_COLOR_BASE),
                   &base_colour);
  rc_style_p->base[GTK_STATE_NORMAL] = base_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PANGO_LOG_COLOR_TEXT),
                   &text_colour);
  rc_style_p->text[GTK_STATE_NORMAL] = text_colour;
  rc_style_p->color_flags[GTK_STATE_NORMAL] =
    static_cast<GtkRcFlags> (GTK_RC_BASE |
                             GTK_RC_TEXT);
  gtk_widget_modify_style (GTK_WIDGET (view_p),
                           rc_style_p);
  //gtk_rc_style_unref (rc_style_p);
  g_object_unref (rc_style_p);

  //  GtkTextIter iterator;
  //  gtk_text_buffer_get_end_iter (buffer_p,
  //                                &iterator);
  //  gtk_text_buffer_create_mark (buffer_p,
  //                               ACE_TEXT_ALWAYS_CHAR (NET_UI_SCROLLMARK_NAME),
  //                               &iterator,
  //                               TRUE);
  //  g_object_unref (buffer_p);

  // step5: initialize updates
  struct Test_I_GTK_CBData* cb_ui_cb_data_p = ui_cb_data_p;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the log view
    guint event_source_id = g_timeout_add_seconds (1,
                                                   idle_update_log_display_cb,
                                                   cb_ui_cb_data_p);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
    // schedule asynchronous updates of the info view
    event_source_id =
      g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET_MS,
                     idle_update_info_display_source_cb,
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

  // step6: disable some functions ?
  struct _GtkAction* action_p =
    //GTK_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_START_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p,
                            Common_File_Tools::isReadable ((*iterator_2).second.second->fileIdentifier.identifier));
  action_p =
    //GTK_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_CLOSEALL_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_STOP_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, FALSE);

  // step7: (auto-)connect signals/slots
  // *NOTE*: glade_xml_signal_autoconnect does not work reliably
  //glade_xml_signal_autoconnect(userData_out.xml);

  // step6a: connect default signals
  gulong result_2 =
      g_signal_connect (dialog_p,
                        ACE_TEXT_ALWAYS_CHAR ("destroy"),
                        G_CALLBACK (gtk_widget_destroyed),
                        NULL);
  ACE_ASSERT (result_2);
  result_2 =
    g_signal_connect (file_chooser_button_p,
                      ACE_TEXT_ALWAYS_CHAR ("file-set"),
                      G_CALLBACK (filechooserbutton_source_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  GtkFileChooserDialog* file_chooser_dialog_p =
    GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FILECHOOSERDIALOG_OPEN_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  result_2 =
    g_signal_connect (file_chooser_dialog_p,
                      ACE_TEXT_ALWAYS_CHAR ("file-activated"),
                      G_CALLBACK (filechooserdialog_cb),
                      NULL);
  ACE_ASSERT (result_2);

  // step6b: connect custom signals
  //gtk_builder_connect_signals ((*iterator).second.second,
  //                             userData_in);

  GObject* object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("value-changed"),
  //                             G_CALLBACK (spinbutton_port_value_changed_cb),
  //                             cb_ui_cb_data_p);
  //ACE_ASSERT (result_2);
  //
  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_TCP_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
  //                             G_CALLBACK (togglebutton_protocol_toggled_cb),
  //                             cb_ui_cb_data_p);
  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_UDP_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
  //                             G_CALLBACK (togglebutton_protocol_toggled_cb),
  //                             cb_ui_cb_data_p);
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_CHECKBUTTON_LOOP_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("toggled"),
                               G_CALLBACK (checkbutton_loop_toggled_cb),
                               userData_in);

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_START_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("toggled"),
                               G_CALLBACK (toggle_action_start_toggled_cb),
                               userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_STOP_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("activate"),
                      G_CALLBACK (action_stop_activate_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                      G_CALLBACK (textview_size_allocate_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  //-------------------------------------

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_CLEAR_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("clicked"),
                      G_CALLBACK (button_clear_clicked_cb),
                      cb_ui_cb_data_p);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_ABOUT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_about_clicked_cb),
                        cb_ui_cb_data_p);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_QUIT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_quit_clicked_cb),
                        cb_ui_cb_data_p);
  ACE_ASSERT (result_2);
  ACE_UNUSED_ARG (result_2);

  // set defaults
  //file_chooser_button_p =
  //  GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  std::string default_folder_uri = ACE_TEXT_ALWAYS_CHAR ("file://");
  default_folder_uri += (*iterator_2).second.second->fileIdentifier.identifier;
  gboolean result =
    gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
                                             default_folder_uri.c_str ());
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_uri(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (default_folder_uri.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  gtk_widget_show_all (dialog_p);

  return G_SOURCE_REMOVE;
}

gboolean
idle_end_source_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_end_source_UI_cb"));

  // sanity check(s)
  struct Test_I_Source_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Source_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  if (ui_cb_data_p->loop > 0)
    --ui_cb_data_p->loop;

  // sanity check(s)
  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != CBData_->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkTable* table_p =
    GTK_TABLE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TABLE_OPTIONS_NAME)));
  ACE_ASSERT (table_p);
  gtk_widget_set_sensitive (GTK_WIDGET (table_p), TRUE);

  GtkAction* action_p = NULL;
  if (ui_cb_data_p->loop == 0)
  {
    action_p =
      GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_START_NAME)));
    ACE_ASSERT (action_p);
    gtk_action_set_stock_id (action_p, GTK_STOCK_MEDIA_PLAY);

    action_p =
        GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_STOP_NAME)));
    ACE_ASSERT (action_p);
    gtk_action_set_sensitive (action_p, FALSE);
  } // end IF

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_progress_source_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_progress_source_cb"));

  // sanity check(s)
  struct Test_I_Source_ProgressData* progress_data_p =
      static_cast<struct Test_I_Source_ProgressData*> (userData_in);
  ACE_ASSERT (progress_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  int result = -1;
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkProgressBar* progressbar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progressbar_p);

  ACE_THR_FUNC_RETURN exit_status;
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  Common_UI_GTK_PendingActionsIterator_t iterator_2;
  for (Common_UI_GTK_CompletedActionsIterator_t iterator_3 = progress_data_p->completedActions.begin ();
       iterator_3 != progress_data_p->completedActions.end ();
       ++iterator_3)
  {
    iterator_2 = progress_data_p->pendingActions.find (*iterator_3);
    ACE_ASSERT (iterator_2 != progress_data_p->pendingActions.end ());
    ACE_thread_t thread_id = (*iterator_2).second.id ();
    result = thread_manager_p->join (thread_id, &exit_status);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::join(%u): \"%m\", continuing\n"),
                  thread_id));
    else if (exit_status)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("thread %u has joined (status was: %d)...\n"),
                  thread_id,
                  exit_status));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("thread %u has joined (status was: %@)...\n"),
                  thread_id,
                  exit_status));
#endif
    } // end IF

    state_r.eventSourceIds.erase (*iterator_3);
    progress_data_p->pendingActions.erase (iterator_2);
  } // end FOR
  progress_data_p->completedActions.clear ();

  bool done = false;
  if (progress_data_p->pendingActions.empty ())
  {
    //if (progress_data_p->cursorType != GDK_LAST_CURSOR)
    //{
    //  GdkCursor* cursor_p = gdk_cursor_new (progress_data_p->cursorType);
    //  if (!cursor_p)
    //  {
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("failed to gdk_cursor_new(%d): \"%m\", continuing\n"),
    //                progress_data_p->cursorType));
    //    return FALSE; // G_SOURCE_REMOVE
    //  } // end IF
    //  GtkWindow* window_p =
    //    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
    //                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_MAIN)));
    //  ACE_ASSERT (window_p);
    //  GdkWindow* window_2 = gtk_widget_get_window (GTK_WIDGET (window_p));
    //  ACE_ASSERT (window_2);
    //  gdk_window_set_cursor (window_2, cursor_p);
    //  progress_data_p->cursorType = GDK_LAST_CURSOR;
    //} // end IF
    int result = progress_data_p->state->condition.broadcast ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Condition::broadcast(): \"%m\", continuing\n")));

    done = true;
  } // end IF

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string magnitude_string = ACE_TEXT_ALWAYS_CHAR ("byte(s)/s");
  float speed = progress_data_p->statistic.bytesPerSecond;
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
    result = ACE_OS::sprintf (buffer, ACE_TEXT ("%.2f %s"),
                              speed, magnitude_string.c_str ());
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", continuing\n")));

    gtk_progress_bar_set_text (progressbar_p,
                               ACE_TEXT_ALWAYS_CHAR (buffer));
  } // end IF

  gdouble fraction_d = 0.0;
  if (progress_data_p->size)
  {
    ACE_ASSERT (progress_data_p->transferred <= progress_data_p->size);
    fraction_d =
      static_cast<double> (progress_data_p->transferred) / static_cast<double> (progress_data_p->size);
  } // end IF
  gtk_progress_bar_set_fraction (progressbar_p, fraction_d);

  // --> reschedule
  return (done ? G_SOURCE_REMOVE : G_SOURCE_CONTINUE);
}

/////////////////////////////////////////

gboolean
idle_initialize_target_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_target_UI_cb"));

  // sanity check(s)
  struct Test_I_Target_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Target_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //// sanity check(s)
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
  //  GTK_WIDGET (glade_xml_get_widget ((*iterator).second.second,
  //                                    ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_MAIN_NAME)));
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  //  GtkWidget* image_icon_p = gtk_image_new_from_file (path.c_str ());
  //  ACE_ASSERT (image_icon_p);
  //  gtk_window_set_icon (GTK_WINDOW (dialog_p),
  //                       gtk_image_get_pixbuf (GTK_IMAGE (image_icon_p)));
  //GdkWindow* dialog_window_p = gtk_widget_get_window (dialog_p);
  //gtk_window4096_set_title (,
  //                      caption.c_str ());

//  GtkWidget* about_dialog_p =
//    //GTK_WIDGET (glade_xml_get_widget ((*iterator).second.second,
//    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
//    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
//                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
//  ACE_ASSERT (about_dialog_p);

  GtkSpinButton* spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT64>::max ()));
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  GFile* file_p = NULL;
  std::string directory, file_name;
  Test_I_Target_StreamConfiguration_t::ITERATOR_T iterator_2 =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != ui_cb_data_p->configuration->streamConfiguration.end ());
  directory = (*iterator_2).second.second->fileIdentifier.identifier;
  file_name = (*iterator_2).second.second->fileIdentifier.identifier;
  // sanity check(s)
  if (!Common_File_Tools::isDirectory (directory))
  {
    directory =
      ACE_TEXT_ALWAYS_CHAR (ACE::dirname (ACE_TEXT (directory.c_str ())));
    if (Common_File_Tools::isValidPath (directory))
    {
      if (!Common_File_Tools::isDirectory (directory))
        if (!Common_File_Tools::createDirectory (directory))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to create directory \"%s\": \"%m\", aborting\n"),
                      ACE_TEXT (directory.c_str ())));
          return G_SOURCE_REMOVE;
        } // end IF
    } // end IF
    else if (Common_File_Tools::isValidFilename (directory))
    {
      directory =
        ACE_TEXT_ALWAYS_CHAR (ACE::dirname (ACE_TEXT (directory.c_str ())));
      if (!Common_File_Tools::isDirectory (directory))
        if (!Common_File_Tools::createDirectory (directory))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to create directory \"%s\": \"%m\", aborting\n"),
                      ACE_TEXT (directory.c_str ())));
          return G_SOURCE_REMOVE;
        } // end IF
    } // end IF
    else
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("invalid target directory (was: \"%s\"), falling back\n"),
                  ACE_TEXT (directory.c_str ())));
      directory = Common_File_Tools::getTempDirectory ();
    } // end ELSE
  } // end IF
  if (Common_File_Tools::isDirectory (file_name))
    file_name = ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_DEFAULT_OUTPUT_FILENAME);
  else if (Common_File_Tools::isValidFilename (file_name))
    file_name = ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (file_name.c_str ())));
  file_name = directory +
              ACE_DIRECTORY_SEPARATOR_STR_A +
              file_name;
  ACE_ASSERT (Common_File_Tools::isValidFilename (file_name));

  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  Net_ConnectionConfigurationsIterator_t iterator_3 =
    ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != ui_cb_data_p->configuration->connectionConfigurations.end ());
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<double> (NET_CONFIGURATION_TCP_CAST ((*iterator_3).second)->socketConfiguration.address.get_port_number ()));

  GtkRadioButton* radio_button_p = NULL;
  if (ui_cb_data_p->configuration->protocol == NET_TRANSPORTLAYER_UDP)
  {
    radio_button_p =
      GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_UDP_NAME)));
    ACE_ASSERT (radio_button_p);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_button_p), TRUE);
  } // end IF
  GtkCheckButton* check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_CHECKBUTTON_ASYNCH_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                (ui_cb_data_p->configuration->dispatchConfiguration.numberOfProactorThreads > 0));
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_CHECKBUTTON_LOOPBACK_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                NET_CONFIGURATION_TCP_CAST ((*iterator_3).second)->socketConfiguration.useLoopBackDevice);

  spin_button_p =
      //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
      //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));
  gtk_spin_button_set_value (spin_button_p,
                              static_cast<double> (ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize));

  GtkProgressBar* progressbar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progressbar_p);
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progressbar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progressbar_p,
                                   1.0 / static_cast<double> (width));

  // step4: initialize text view, setup auto-scrolling
  GtkTextView* view_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
//  GtkTextBuffer* buffer_p =
////    gtk_text_buffer_new (NULL); // text tag table --> create new
//      gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);
////  gtk_text_view_set_buffer (view_p, buffer_p);

  PangoFontDescription* font_description_p =
    pango_font_description_from_string (ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION));
  if (!font_description_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pango_font_description_from_string(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_I_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION)));
    return G_SOURCE_REMOVE;
  } // end IF
  // apply font
  GtkRcStyle* rc_style_p = gtk_rc_style_new ();
  if (!rc_style_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_rc_style_new(): \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  rc_style_p->font_desc = font_description_p;
  GdkColor base_colour, text_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PANGO_LOG_COLOR_BASE),
                   &base_colour);
  rc_style_p->base[GTK_STATE_NORMAL] = base_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PANGO_LOG_COLOR_TEXT),
                   &text_colour);
  rc_style_p->text[GTK_STATE_NORMAL] = text_colour;
  rc_style_p->color_flags[GTK_STATE_NORMAL] =
    static_cast<GtkRcFlags> (GTK_RC_BASE |
                             GTK_RC_TEXT);
  gtk_widget_modify_style (GTK_WIDGET (view_p),
                           rc_style_p);
  //gtk_rc_style_unref (rc_style_p);
  g_object_unref (rc_style_p);

  //  GtkTextIter iterator;
  //  gtk_text_buffer_get_end_iter (buffer_p,
  //                                &iterator);
  //  gtk_text_buffer_create_mark (buffer_p,
  //                               ACE_TEXT_ALWAYS_CHAR (NET_UI_SCROLLMARK_NAME),
  //                               &iterator,
  //                               TRUE);
  //  g_object_unref (buffer_p);

  // step5: initialize updates
  guint event_source_id = 0;
  struct Test_I_GTK_CBData* ui_cb_data_2 = ui_cb_data_p;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the log view
    event_source_id = g_timeout_add_seconds (1,
                                             idle_update_log_display_cb,
                                             ui_cb_data_2);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE

    // schedule asynchronous updates of the info view
    event_source_id =
      g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET_MS,
                     idle_update_info_display_target_cb,
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

  // step6: (auto-)connect signals/slots
  // step6a: connect default signals
  // *NOTE*: glade_xml_signal_autoconnect does not work reliably
  // glade_xml_signal_autoconnect (userData_in.xml);
  gtk_builder_connect_signals ((*iterator).second.second,
                               userData_in);

  // step6b: connect custom signals
  gulong result_2 =
    g_signal_connect (dialog_p,
                      ACE_TEXT_ALWAYS_CHAR ("destroy"),
                      G_CALLBACK (gtk_widget_destroyed),
                      NULL);
  ACE_ASSERT (result_2);

  // step7: set defaults
  gchar* string_p = Common_UI_GTK_Tools::localeToUTF8 (directory);
  if (!gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p),
                                            string_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder(\"%s\"), aborting\n"),
                ACE_TEXT (directory.c_str ())));
    g_free (string_p);
    return G_SOURCE_REMOVE;
  } // end IF
  g_free (string_p); string_p = NULL;

  struct _GtkAction* action_p =
    //GTK_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_LISTEN_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_activate (action_p);

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  gtk_widget_show_all (dialog_p);

  return G_SOURCE_REMOVE;
}

gboolean
idle_start_target_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_start_target_UI_cb"));

  // sanity check(s)
  struct Test_I_Target_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Target_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // sanity check(s)
  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != CBData_->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkAction* action_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_CLOSE_ALL_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, TRUE);

  return G_SOURCE_REMOVE;
}

gboolean
idle_end_target_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_end_target_UI_cb"));

  // sanity check(s)
  struct Test_I_Target_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Target_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // sanity check(s)
  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != CBData_->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkAction* action_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_CLOSE_ALL_NAME)));
  ACE_ASSERT (action_p);
  Test_I_Target_TCPConnectionManager_t* connection_manager_p =
    TEST_I_TARGET_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  gtk_action_set_sensitive (action_p,
                            (connection_manager_p->count () != 0));

  return G_SOURCE_REMOVE;
}

gboolean
idle_reset_target_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_reset_target_UI_cb"));

  // sanity check(s)
  struct Test_I_Target_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Target_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // sanity check(s)
  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != CBData_->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_progress_target_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_progress_target_cb"));

  // sanity check(s)
  struct Test_I_GTK_ProgressData* progress_data_p =
    static_cast<struct Test_I_GTK_ProgressData*> (userData_in);
  ACE_ASSERT (progress_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result = -1;
  float speed = 0.0F;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
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
    result = ACE_OS::sprintf (buffer, ACE_TEXT ("%.2f %s"),
                              speed, magnitude_string.c_str ());
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

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  ACE_UNUSED_ARG (userData_in);

  // leave GTK
  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_info_display_source_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_info_display_source_cb"));

  // sanity check(s)
  struct Test_I_Source_UI_CBData* ui_cb_data_p =
      static_cast<struct Test_I_Source_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

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

  // sanity check(s)
  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (state_r.eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_STARTED:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
          if (spin_button_p)
          {
            gint number_of_connections =
              gtk_spin_button_get_value_as_int (spin_button_p);
            gtk_spin_button_set_value (spin_button_p,
                                       static_cast<gdouble> (++number_of_connections));
          } // end IF

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (ui_cb_data_p->progressData.transferred));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          break;
        }
        case COMMON_UI_EVENT_STOPPED:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
          if (spin_button_p)
          {
            gint number_of_connections =
              gtk_spin_button_get_value_as_int (spin_button_p);
            gtk_spin_button_set_value (spin_button_p,
                                       static_cast<gdouble> (--number_of_connections));
          } // end IF

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STATISTIC:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (ui_cb_data_p->progressData.statistic.bytes));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
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
idle_update_info_display_target_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_info_display_target_cb"));

  // sanity check(s)
  struct Test_I_Target_UI_CBData* ui_cb_data_p =
      static_cast<struct Test_I_Target_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

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

  // sanity check(s)
  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (state_r.eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_STARTED:
        {
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);

          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
          if (spin_button_p)
          {
            gint number_of_connections =
                gtk_spin_button_get_value_as_int (spin_button_p);
            gtk_spin_button_set_value (spin_button_p,
                                       static_cast<gdouble> (++number_of_connections));
          } // end IF

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (ui_cb_data_p->progressData.statistic.bytes));

          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          break;
        }
        case COMMON_UI_EVENT_STOPPED:
        {
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
          if (spin_button_p)
          {
            gint number_of_connections =
                gtk_spin_button_get_value_as_int (spin_button_p);
            gtk_spin_button_set_value (spin_button_p,
                                       static_cast<gdouble> (--number_of_connections));
          } // end IF

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STATISTIC:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (ui_cb_data_p->progressData.statistic.bytes));

          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
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
idle_update_log_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));

  // sanity check(s)
  struct Test_I_GTK_CBData* ui_cb_data_p =
      static_cast<struct Test_I_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  // sanity check(s)
  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkTextView* view_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);

  GtkTextIter text_iterator;
  gtk_text_buffer_get_end_iter (buffer_p,
                                &text_iterator);

  gchar* string_p = NULL;
  // sanity check
  if (state_r.logQueue.empty ())
    return G_SOURCE_CONTINUE;

  // step1: convert text
  for (Common_Log_MessageQueueConstIterator_t iterator_2 = state_r.logQueue.begin ();
       iterator_2 != state_r.logQueue.end ();
       iterator_2++)
  {
    string_p = Common_UI_GTK_Tools::localeToUTF8 (*iterator_2);
    if (!string_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Tools::localeToUTF8(\"%s\"), aborting\n"),
                  ACE_TEXT ((*iterator_2).c_str ())));
      return G_SOURCE_REMOVE;
    } // end IF

    // step2: display text
    gtk_text_buffer_insert (buffer_p,
                            &text_iterator,
                            string_p,
                            -1);

    // clean up
    g_free (string_p);
  } // end FOR

  state_r.logQueue.clear ();

  // step3: scroll the view accordingly
//  // move the iterator to the beginning of line, so it doesn't scroll
//  // in horizontal direction
//  gtk_text_iter_set_line_offset (&text_iterator, 0);

//  // ...and place the mark at iter. The mark will stay there after insertion
//  // because it has "right" gravity
//  GtkTextMark* text_mark_p =
//      gtk_text_buffer_get_mark (buffer_p,
//                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SCROLLMARK_NAME));
////  gtk_text_buffer_move_mark (buffer_p,
////                             text_mark_p,
////                             &text_iterator);

//  // scroll the mark onscreen
//  gtk_text_view_scroll_mark_onscreen (view_p,
//                                      text_mark_p);
  //GtkAdjustment* adjustment_p =
  //    GTK_ADJUSTMENT (gtk_builder_get_object ((*iterator).second.second,
  //                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ADJUSTMENT_NAME)));
  //ACE_ASSERT (adjustment_p);
  //gtk_adjustment_set_value (adjustment_p,
  //                          adjustment_p->upper - adjustment_p->page_size));

  return G_SOURCE_CONTINUE;
}

/////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void
toggle_action_start_toggled_cb (GtkToggleAction* action_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::toggle_action_start_toggled_cb"));

  // handle untoggle --> PLAY
  if (un_toggling_play_pause)
  {
    un_toggling_play_pause = false;
    return; // done
  } // end IF

  // sanity check(s)
  struct Test_I_Source_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Source_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->stream);
  ACE_ASSERT (ui_cb_data_p->UDPStream);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  struct Test_I_Source_UI_ThreadData* thread_data_p = NULL;
  ACE_thread_t thread_id;
  ACE_hthread_t thread_handle;
  ACE_TCHAR thread_name[BUFSIZ];
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;
  int result = -1;
  guint event_source_id = 0;

  Test_I_StreamBase_t* stream_p = NULL;
  switch (ui_cb_data_p->configuration->protocol)
  {
    case NET_TRANSPORTLAYER_TCP:
      stream_p = ui_cb_data_p->stream;
      break;
    case NET_TRANSPORTLAYER_UDP:
      stream_p = ui_cb_data_p->UDPStream;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown protocol (was: %d), returning\n"),
                  ui_cb_data_p->configuration->protocol));
      return;
    }
  } // end SWITCH
  ACE_ASSERT (stream_p);

  // toggle play/pause ?
  enum Stream_StateMachine_ControlState status = stream_p->status ();
  if ((status == STREAM_STATE_RUNNING) ||
      (status == STREAM_STATE_PAUSED))
  {
    stream_p->pause (); // pause/unpause
    //if (!ui_cb_data_p->configuration->moduleHandlerConfiguration.active)
    //{
    //  ACE_ASSERT (!ui_cb_data_p->progressData.pendingActions.empty ());
    //  Stream_PendingActionsIterator_t iterator =
    //    ui_cb_data_p->progressData.pendingActions.begin ();
    //  if (status_r == STREAM_STATE_RUNNING)
    //    result = ACE_Thread::suspend ((*iterator).second.handle);
    //  else
    //    result = ACE_Thread::resume ((*iterator).second.handle);
    //  if (result == -1)
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("failed to ACE_Thread::suspend/resume(): \"%m\", continuing\n")));
    //} // end ELSE

    if (status == STREAM_STATE_RUNNING) // <-- image is "pause"
      gtk_action_set_stock_id (GTK_ACTION (action_in), GTK_STOCK_MEDIA_PLAY);
    else // <-- image is "play"
      gtk_action_set_stock_id (GTK_ACTION (action_in), GTK_STOCK_MEDIA_PAUSE);
    return;
  } // end IF

  un_toggling_play_pause = true;
  gtk_toggle_action_set_active (action_in, false); // untoggle
  gtk_action_set_stock_id (GTK_ACTION (action_in), GTK_STOCK_MEDIA_PAUSE);

  // step0: modify widgets
  GtkTable* table_p =
    GTK_TABLE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TABLE_OPTIONS_NAME)));
  ACE_ASSERT (table_p);
  gtk_widget_set_sensitive (GTK_WIDGET (table_p), false);

  GtkAction* action_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_STOP_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, true);

  // step1: set up progress reporting
  ui_cb_data_p->progressData.transferred = 0;
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_fraction (progress_bar_p, 0.0);

  // step2: update configuration
  // retrieve port number
  GtkSpinButton* spin_button_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  unsigned short port_number =
    static_cast<unsigned short> (gtk_spin_button_get_value_as_int (spin_button_p));
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != ui_cb_data_p->configuration->connectionConfigurations.end ());
  NET_CONFIGURATION_TCP_CAST ((*iterator_2).second)->socketConfiguration.address.set_port_number (port_number,
                                                                                                  1);

  // retrieve protocol
  GtkRadioButton* radio_button_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_TCP_NAME)));
  ACE_ASSERT (radio_button_p);
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio_button_p)))
    ui_cb_data_p->configuration->protocol = NET_TRANSPORTLAYER_TCP;
  else
    ui_cb_data_p->configuration->protocol = NET_TRANSPORTLAYER_UDP;

  // retrieve buffer
  spin_button_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize =
    static_cast<unsigned int> (gtk_spin_button_get_value_as_int (spin_button_p));

  // retrieve loop
  spin_button_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_LOOP_NAME)));
  ACE_ASSERT (spin_button_p);
  ui_cb_data_p->loop =
    static_cast<size_t> (gtk_spin_button_get_value_as_int (spin_button_p));

  // step3: start processing thread
  ACE_NEW_NORETURN (thread_data_p,
                    struct Test_I_Source_UI_ThreadData ());
  if (!thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto clean;
  } // end IF
  thread_data_p->CBData = ui_cb_data_p;
  ACE_OS::memset (thread_name, 0, sizeof (thread_name));
  //  char* thread_name_p = NULL;
  //  ACE_NEW_NORETURN (thread_name_p,
  //                    ACE_TCHAR[BUFSIZ]);
  //  if (!thread_name_p)
  //  {
  //    ACE_DEBUG ((LM_CRITICAL,
  //                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
  //    delete thread_data_p; thread_data_p = NULL;
  //    return;
  //  } // end IF
  //  ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
  //  ACE_OS::strcpy (thread_name_p,
  //                  ACE_TEXT (TEST_I_STREAM_FILECOPY_THREAD_NAME));
  //  const char* thread_name_2 = thread_name_p;
  ACE_OS::strcpy (thread_name,
                  ACE_TEXT (TEST_I_STREAM_THREAD_NAME));
  thread_name_2 = thread_name;
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);

  // *NOTE*: lock access to the progress report structures to avoid a race
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    ACE_THR_FUNC function_p = ACE_THR_FUNC (::stream_processing_function);
    result =
      thread_manager_p->spawn (function_p,                       // function
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
                               &thread_name_2);                  // name
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::spawn(): \"%m\", returning\n")));
      delete thread_data_p; thread_data_p = NULL;
      goto clean;
    } // end IF

    // step3: start progress reporting
    event_source_id =
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_progress_cb,
      //                 &ui_cb_data_p->progressData,
      //                 NULL);
      g_timeout_add (//G_PRIORITY_DEFAULT_IDLE,               // _LOW doesn't work (on Win32)
                     COMMON_UI_REFRESH_DEFAULT_PROGRESS_MS, // ms (?)
                     idle_update_progress_source_cb,
                     &ui_cb_data_p->progressData);// ,
                     //NULL);
    if (!event_source_id)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_source_cb): \"%m\", returning\n")));

      // clean up
      ACE_THR_FUNC_RETURN exit_status;
      result = thread_manager_p->join (thread_id, &exit_status);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Thread_Manager::join(%u): \"%m\", continuing\n"),
                    thread_id));

      goto clean;
    } // end IF
    thread_data_p->eventSourceId = event_source_id;
    ui_cb_data_p->progressData.pendingActions[event_source_id] =
      ACE_Thread_ID (thread_id, thread_handle);
    //    ACE_DEBUG ((LM_DEBUG,
    //                ACE_TEXT ("idle_update_progress_cb: %d\n"),
    //                event_source_id));
    state_r.eventSourceIds.insert (event_source_id);
  } // end lock scope

  return;

clean:
  gtk_action_set_stock_id (GTK_ACTION (action_in), GTK_STOCK_MEDIA_PLAY);
  gtk_widget_set_sensitive (GTK_WIDGET (table_p), TRUE);
  gtk_action_set_sensitive (action_p, FALSE);
} // action_start_activate_cb

void
action_stop_activate_cb (GtkAction* action_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_stop_activate_cb"));

  // sanity check(s)
  struct Test_I_Source_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Source_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->stream);
  ACE_ASSERT (ui_cb_data_p->UDPStream);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  ui_cb_data_p->loop = 1;

  Test_I_StreamBase_t* stream_p = NULL;
  switch (ui_cb_data_p->configuration->protocol)
  {
    case NET_TRANSPORTLAYER_TCP:
      stream_p = ui_cb_data_p->stream;
      break;
    case NET_TRANSPORTLAYER_UDP:
      stream_p = ui_cb_data_p->UDPStream;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown protocol (was: %d), returning\n"),
                  ui_cb_data_p->configuration->protocol));
      return;
    }
  } // end SWITCH
  ACE_ASSERT (stream_p);
  stream_p->stop (false);

  GtkToggleAction* toggle_action_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_START_NAME)));
  ACE_ASSERT (toggle_action_p);
  if (gtk_toggle_action_get_active (toggle_action_p))
  {
    un_toggling_play_pause = true;
    gtk_toggle_action_set_active (toggle_action_p, FALSE);
  } // end IF
  gtk_action_set_stock_id (GTK_ACTION (toggle_action_p), GTK_STOCK_MEDIA_PLAY);
  gtk_action_set_sensitive (action_in, FALSE);
} // action_stop_activate_cb

void
checkbutton_loop_toggled_cb (GtkToggleButton* toggleButton_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::checkbutton_loop_toggled_cb"));

  // sanity check(s)
  struct Test_I_GTK_CBData* ui_cb_data_p =
    static_cast<struct Test_I_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkSpinButton* spin_button_p =
    //GTK_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_LOOP_NAME)));
  ACE_ASSERT (spin_button_p);
  if (gtk_toggle_button_get_active (toggleButton_in))
    gtk_spin_button_set_value (spin_button_p, -1.0);
  else if (gtk_spin_button_get_value_as_int (spin_button_p) == -1)
    gtk_spin_button_set_value (spin_button_p, 0.0);
} // checkbutton_loop_toggled_cb

void
filechooserbutton_source_cb (GtkFileChooserButton* button_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_source_cb"));

  // sanity check(s)
  struct Test_I_Source_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Source_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  //// step1: display chooser dialog
  //GtkFileChooserDialog* file_chooser_dialog_p =
  //  GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_FILECHOOSER_OPEN_NAME)));
  //ACE_ASSERT (file_chooser_dialog_p);

  //// run dialog
  //GFile* file_p = NULL;
  //gint result = gtk_dialog_run (GTK_DIALOG (file_chooser_dialog_p));
  //switch (result)
  //{
  //  case GTK_RESPONSE_OK:
  //    file_p = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (file_chooser_dialog_p));
  //    if (!file_p) return FALSE; // ? *TODO*
  //    break;
  //  case GTK_RESPONSE_DELETE_EVENT: // ESC
  //  case GTK_RESPONSE_CANCEL:
  //  default:
  //    //gtk_widget_hide (GTK_WIDGET (file_chooser_dialog_p));
  //    return FALSE;
  //} // end SWITCH
  //ACE_ASSERT (file_p);
  //gtk_widget_hide (GTK_WIDGET (file_chooser_dialog_p));
  //GtkEntry* entry_p =
  //  GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
  //  ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ENTRY_SOURCE_NAME)));
  //ACE_ASSERT (entry_p);

  GFile* file_p = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (button_in));
  ACE_ASSERT (file_p);
  gchar* string_p = g_file_get_path (file_p);
  if (!string_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_file_get_path(%@): \"%m\", returning\n"),
                file_p));

    // clean up
    g_object_unref (file_p);

    return;
  } // end IF
  g_object_unref (file_p);
  //gtk_entry_set_text (entry_p, string_p);

  // find out which button toggled
  Test_I_Source_StreamConfiguration_t::ITERATOR_T iterator_2 =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != ui_cb_data_p->configuration->streamConfiguration.end ());
  (*iterator_2).second.second->fileIdentifier.identifier =
      Common_UI_GTK_Tools::UTF8ToLocale (string_p, -1);
  if ((*iterator_2).second.second->fileIdentifier.identifier.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF8ToLocale(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (string_p)));

    // clean up
    g_free (string_p);

    return;
  } // end IF
  g_free (string_p);

  // start button
  GtkAction* action_p =
    //GTK_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_START_NAME)));
  ACE_ASSERT (action_p);
  Net_ConnectionConfigurationsIterator_t iterator_3 =
    ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != ui_cb_data_p->configuration->connectionConfigurations.end ());
  bool activate =
      (!(*iterator_2).second.second->fileIdentifier.identifier.empty () &&
       !NET_CONFIGURATION_TCP_CAST ((*iterator_3).second)->socketConfiguration.address.is_any ());
  gtk_action_set_sensitive (action_p, activate);
} // filechooserbutton_source_cb

/////////////////////////////////////////

void
action_close_all_activate_cb (GtkAction* action_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_close_all_activate_cb"));

  // sanity check(s)
  struct Test_I_Target_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Target_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  gtk_action_set_sensitive (action_in, FALSE);

  // sanity check(s)
  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  int result = -1;

  // *PORTABILITY*: on MS Windows systems, user signals SIGUSRx are not defined
  //                --> use SIGBREAK (21) and SIGTERM (15) instead
  int signal = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  signal = SIGTERM;
#else
  signal = SIGUSR2;
#endif
  result = ACE_OS::raise (signal);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));

  // closed the UDP "listener" ? --> toggle listen button
  if (ui_cb_data_p->configuration->protocol == NET_TRANSPORTLAYER_UDP)
  {
    GtkToggleAction* toggle_action_p =
      //GTK_BUTTON (glade_xml_get_widget ((*iterator).second.second,
      //                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
      GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_LISTEN_NAME)));
    ACE_ASSERT (toggle_action_p);
    gtk_action_activate (GTK_ACTION (toggle_action_p));
  } // end IF

  idle_reset_target_UI_cb (userData_in);
} // action_close_all_activate_cb

void
action_listen_activate_cb (GtkAction* action_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_listen_activate_cb"));

  ACE_UNUSED_ARG (action_in);
  // sanity check(s)
  struct Test_I_Target_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Target_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkToggleButton* toggle_button_p =
    //GTK_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEBUTTON_LISTEN_NAME)));
  ACE_ASSERT (toggle_button_p);
  bool start_listening = gtk_toggle_button_get_active (toggle_button_p);
  gtk_button_set_label (GTK_BUTTON (toggle_button_p),
                        (start_listening ? ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEBUTTON_LABEL_LISTENING_STRING)
                                         : ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEBUTTON_LABEL_LISTEN_STRING)));

  GtkImage* image_p =
    //GTK_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
    GTK_IMAGE (gtk_builder_get_object ((*iterator).second.second,
                                       (start_listening ? ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_IMAGE_CONNECT_NAME)
                                                        : ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_IMAGE_DISCONNECT_NAME))));
  ACE_ASSERT (image_p);
  gtk_button_set_image (GTK_BUTTON (toggle_button_p), GTK_WIDGET (image_p));

  GtkRadioButton* radio_button_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_TCP_NAME)));
  ACE_ASSERT (radio_button_p);
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio_button_p)))
    ui_cb_data_p->configuration->protocol = NET_TRANSPORTLAYER_TCP;
  else
    ui_cb_data_p->configuration->protocol = NET_TRANSPORTLAYER_UDP;

  if (start_listening)
  {
    switch (ui_cb_data_p->configuration->protocol)
    {
      case NET_TRANSPORTLAYER_TCP:
      {
        Test_I_Target_TCPConnectionManager_t* connection_manager_p =
          TEST_I_TARGET_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
        ACE_ASSERT (connection_manager_p);

        // listening on UDP ? --> stop
        if (ui_cb_data_p->configuration->handle != ACE_INVALID_HANDLE)
        {
          Test_I_Target_TCPConnectionManager_t::ICONNECTION_T* connection_p =
              NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          connection_p =
            connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (ui_cb_data_p->configuration->handle));
#else
          connection_p =
            connection_manager_p->get (static_cast<Net_ConnectionId_t> (ui_cb_data_p->configuration->handle));
#endif
          if (connection_p)
          {
            connection_p->abort ();
            connection_p->decrease ();
          } // end ELSE
          ui_cb_data_p->configuration->handle = ACE_INVALID_HANDLE;
        } // end IF

        Net_ConnectionConfigurationsIterator_t iterator_2 =
          ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
        ACE_ASSERT (iterator_2 != ui_cb_data_p->configuration->connectionConfigurations.end ());
        //ui_cb_data_p->configuration->listenerConfiguration.address =
        //  (*iterator_2).second.address;
        ACE_ASSERT (ui_cb_data_p->configuration->signalHandlerConfiguration.listener);
        if (!ui_cb_data_p->configuration->signalHandlerConfiguration.listener->initialize (*static_cast<Test_I_Target_TCPConnectionConfiguration_t*>((*iterator_2).second)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize listener, continuing\n")));
        try {
          ui_cb_data_p->configuration->signalHandlerConfiguration.listener->start (NULL);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_Server_IListener::start(): \"%m\", continuing\n")));
        } // end catch

        break;
      }
      case NET_TRANSPORTLAYER_UDP:
      {
        Test_I_Target_UDPConnectionManager_t* connection_manager_p =
          TEST_I_TARGET_UDP_CONNECTIONMANAGER_SINGLETON::instance ();
        ACE_ASSERT (connection_manager_p);

        // listening on TCP ? --> stop
        ACE_ASSERT (ui_cb_data_p->configuration->signalHandlerConfiguration.listener);
        ui_cb_data_p->configuration->signalHandlerConfiguration.listener->stop ();

        if (ui_cb_data_p->configuration->handle != ACE_INVALID_HANDLE)
        {
          Test_I_Target_UDPConnectionManager_t::ICONNECTION_T* connection_p =
              NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          connection_p =
            connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (ui_cb_data_p->configuration->handle));
#else
          connection_p =
            connection_manager_p->get (static_cast<Net_ConnectionId_t> (ui_cb_data_p->configuration->handle));
#endif
          if (connection_p)
          {
            connection_p->abort ();
            connection_p->decrease (); connection_p = NULL;
          } // end ELSE
          ui_cb_data_p->configuration->handle = ACE_INVALID_HANDLE;
        } // end IF

        Test_I_Target_UDPConnectionManager_t::INTERFACE_T* iconnection_manager_p =
          connection_manager_p;
        ACE_ASSERT (iconnection_manager_p);
        Test_I_Target_IUDPConnector_t* connector_p = NULL;
//        Test_I_Target_StreamConfiguration_t::ITERATOR_T iterator_2 =
//          ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//        ACE_ASSERT (iterator_2 != ui_cb_data_p->configuration->streamConfiguration.end ());
        if (ui_cb_data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0)
          ACE_NEW_NORETURN (connector_p,
                            Test_I_InboundUDPConnector_t (true));
        else
          ACE_NEW_NORETURN (connector_p,
                            Test_I_InboundUDPAsynchConnector_t (true));
        if (!connector_p)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("failed to allocate memory, returning\n")));
          return;
        } // end IF
        //  Stream_IInetConnector_t* iconnector_p = &connector;
        Net_ConnectionConfigurationsIterator_t iterator_3 =
          ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
        ACE_ASSERT (iterator_3 != ui_cb_data_p->configuration->connectionConfigurations.end ());
        if (!connector_p->initialize (*static_cast<Test_I_Target_UDPConnectionConfiguration_t*>((*iterator_3).second)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));
          delete connector_p; connector_p = NULL;
          return;
        } // end IF

        // connect
        ui_cb_data_p->configuration->handle =
          connector_p->connect (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress);
        // *TODO*: support one-thread operation by scheduling a signal and manually
        //         running the dispatch loop for a limited time...
        if (ui_cb_data_p->configuration->dispatchConfiguration.numberOfProactorThreads > 0)
        {
          ACE_Time_Value timeout (NET_CONNECTION_ASYNCH_DEFAULT_ESTABLISHMENT_TIMEOUT_S,
                                  0);
          ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
          Test_I_InboundUDPAsynchConnector_t::ICONNECTION_T* connection_p =
            NULL;
          // *TODO*: avoid tight loop here
          do
          {
            connection_p =
              connection_manager_p->get (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress,
                                         false);
            if (connection_p)
            {
              ui_cb_data_p->configuration->handle =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                  reinterpret_cast<ACE_HANDLE> (connection_p->id ());
#else
                  static_cast<ACE_HANDLE> (connection_p->id ());
#endif // ACE_WIN32 || ACE_WIN64
              connection_p->decrease (); connection_p = NULL;
              break;
            } // end IF
          } while (COMMON_TIME_NOW < deadline);
        } // end IF
        if (ui_cb_data_p->configuration->handle == ACE_INVALID_HANDLE)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to connect to %s, returning\n"),
                      ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress).c_str ())));
          delete connector_p; connector_p = NULL;
          return;
        } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("0x%@: started listening (UDP) (%s)...\n"),
                    ui_cb_data_p->configuration->handle,
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress).c_str ())));
#else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%d: started listening (UDP) (%s)...\n"),
                    ui_cb_data_p->configuration->handle,
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
        delete connector_p; connector_p = NULL;
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown transport layer type (was: %d), returning\n"),
                    ui_cb_data_p->configuration->protocol));
        return;
      } // end catch
    } // end SWITCH

    // start progress reporting
    GtkProgressBar* progressbar_p =
      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
    ACE_ASSERT (progressbar_p);
    gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), TRUE);

    ACE_ASSERT (!ui_cb_data_p->progressData.eventSourceId);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      ui_cb_data_p->progressData.eventSourceId =
        //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
        //                 idle_update_progress_cb,
        //                 &ui_cb_data_p->progressData,
        //                 NULL);
        g_timeout_add (//G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                       COMMON_UI_REFRESH_DEFAULT_PROGRESS_MS, // ms (?)
                       idle_update_progress_target_cb,
                       &ui_cb_data_p->progressData);//,
                       //NULL);
      if (ui_cb_data_p->progressData.eventSourceId > 0)
        state_r.eventSourceIds.insert (ui_cb_data_p->progressData.eventSourceId);
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_timeout_add_full(idle_update_target_progress_cb): \"%m\", returning\n")));
        return;
      } // end IF
    } // end lock scope
  } // end IF
  else
  { ACE_ASSERT (ui_cb_data_p->configuration->signalHandlerConfiguration.listener);
    ui_cb_data_p->configuration->signalHandlerConfiguration.listener->stop ();

    if (ui_cb_data_p->configuration->handle != ACE_INVALID_HANDLE)
    {
      Test_I_Target_UDPConnectionManager_t* connection_manager_p =
        TEST_I_TARGET_UDP_CONNECTIONMANAGER_SINGLETON::instance ();
      ACE_ASSERT (connection_manager_p);

      Test_I_Target_UDPConnectionManager_t::ICONNECTION_T* connection_p =
        NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      connection_p =
        connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (ui_cb_data_p->configuration->handle));
#else
      connection_p =
        connection_manager_p->get (static_cast<Net_ConnectionId_t> (ui_cb_data_p->configuration->handle));
#endif
      if (connection_p)
      {
        connection_p->abort ();
        connection_p->decrease ();
      } // end ELSE
      ui_cb_data_p->configuration->handle = ACE_INVALID_HANDLE;
    } // end IF

    // stop progress reporting
    ACE_ASSERT (ui_cb_data_p->progressData.eventSourceId);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      if (!g_source_remove (ui_cb_data_p->progressData.eventSourceId))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                    ui_cb_data_p->progressData.eventSourceId));
      state_r.eventSourceIds.erase (ui_cb_data_p->progressData.eventSourceId);
      ui_cb_data_p->progressData.eventSourceId = 0;
    } // end lock scope
    GtkProgressBar* progressbar_p =
      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
    ACE_ASSERT (progressbar_p);
    // *NOTE*: this disables "activity mode" (in Gtk2)
    gtk_progress_bar_set_fraction (progressbar_p, 0.0);
    gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), false);
  } // end ELSE
} // action_listen_activate_cb

void
filechooser_target_cb (GtkFileChooser* fileChooser_in,
                       gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooser_target_cb"));

  // sanity check(s)
  struct Test_I_Target_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Target_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Test_I_Target_StreamConfiguration_t::ITERATOR_T iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  GFile* file_p = gtk_file_chooser_get_current_folder_file (fileChooser_in);
  ACE_ASSERT (file_p);
  char* string_p = g_file_get_path (file_p);
  if (!string_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_file_get_path(%@): \"%m\", returning\n"),
                file_p));

    // clean up
    g_object_unref (file_p);

    return;
  } // end IF
  g_object_unref (file_p);

  (*iterator).second.second->fileIdentifier.identifier =
    Common_UI_GTK_Tools::UTF8ToLocale (string_p, -1);
  if ((*iterator).second.second->fileIdentifier.identifier.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF8ToLocale(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (string_p)));

    // clean up
    g_free (string_p);

    return;
  } // end IF
  g_free (string_p);
} // filechooser_target_cb

/////////////////////////////////////////

void
action_report_activate_cb (GtkAction* action_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_report_activate_cb"));

  ACE_UNUSED_ARG (action_in);
  ACE_UNUSED_ARG (userData_in);

// *PORTABILITY*: on Windows SIGUSRx are not defined
// --> use SIGBREAK (21) instead...
  int signal = 0;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  signal = SIGUSR1;
#else
  signal = SIGBREAK;
#endif
  if (ACE_OS::raise (signal) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));
} // action_report_activate_cb

//void
//radiobutton_protocol_toggled_cb (GtkToggleButton* toggleButton_in,
//                                 gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::radiobutton_protocol_toggled_cb"));
//
//  // sanity check
//  if (!gtk_toggle_button_get_active (toggleButton_in))
//    return; // nothing to do
//
//  Stream_GTK_CBData* ui_cb_data_p =
//    static_cast<Stream_GTK_CBData*> (userData_in);
//
//  //Common_UI_GladeXMLsIterator_t iterator =
//  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  Common_UI_GTK_BuildersIterator_t iterator =
//    ui_cb_data_p->UIState.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//
//  // sanity check(s)
//  ACE_ASSERT (ui_cb_data_p);
//  ACE_ASSERT (ui_cb_data_p->configuration);
//  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
//  ACE_ASSERT (iterator != ui_cb_data_p->UIState.builders.end ());
//
//  GtkRadioButton* radio_button_p =
//    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
//    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
//    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_TCP_NAME)));
//  ACE_ASSERT (radio_button_p);
//  const gchar* string_p =
//    gtk_buildable_get_name (GTK_BUILDABLE (radio_button_p));
//  ACE_ASSERT (string_p);
//  if (ACE_OS::strcmp (string_p,
//                      gtk_buildable_get_name (GTK_BUILDABLE (toggleButton_in))) == 0)
//    ui_cb_data_p->configuration->protocol = NET_TRANSPORTLAYER_TCP;
//  else
//    ui_cb_data_p->configuration->protocol = NET_TRANSPORTLAYER_UDP;
//}

//void
//spinbutton_port_value_changed_cb (GtkWidget* widget_in,
//                                  gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::spinbutton_port_value_changed_cb"));
//
//  Stream_GTK_CBData* ui_cb_data_p =
//    static_cast<Stream_GTK_CBData*> (userData_in);
//
//  // sanity check(s)
//  ACE_ASSERT (ui_cb_data_p);
//  ACE_ASSERT (ui_cb_data_p->configuration);
//
//  unsigned short port_number =
//    static_cast<unsigned short> (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget_in)));
//  ui_cb_data_p->configuration->socketConfiguration.peerAddress.set_port_number (port_number,
//                                                                          1);
//} // spinbutton_port_value_changed_cb

// -----------------------------------------------------------------------------

gint
button_clear_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  Test_I_GTK_CBData* ui_cb_data_p =
    static_cast<Test_I_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkTextView* view_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p =
//    gtk_text_buffer_new (NULL); // text tag table --> create new
    gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);
  gtk_text_buffer_set_text (buffer_p,
                            ACE_TEXT_ALWAYS_CHAR (""), 0);

  return FALSE;
}

gint
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
//  struct Test_I_GTK_CBData* ui_cb_data_p =
//    static_cast<struct Test_I_GTK_CBData*> (userData_in);
//  ACE_ASSERT (ui_cb_data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  ACE_ASSERT (iterator != state_r.builders.end ());

  // retrieve about dialog handle
  GtkDialog* about_dialog =
    //GTK_DIALOG (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR(TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
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

gint
button_quit_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  int result = -1;

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (userData_in);

  // sanity check(s)
  Test_I_GTK_CBData* ui_cb_data_p =
    static_cast<Test_I_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->UIState);

  // wait for processing thread(s)
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_p->UIState->lock, FALSE);
    while (!ui_cb_data_p->progressData.pendingActions.empty ())
      ui_cb_data_p->UIState->condition.wait (NULL);
  } // end lock scope

  // step1: remove event sources
  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, ui_cb_data_p->UIState->lock, FALSE);
    for (Common_UI_GTK_EventSourceIdsIterator_t iterator = ui_cb_data_p->UIState->eventSourceIds.begin ();
         iterator != ui_cb_data_p->UIState->eventSourceIds.end ();
         iterator++)
      if (!g_source_remove (*iterator))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                    *iterator));
    ui_cb_data_p->UIState->eventSourceIds.clear ();
  } // end lock scope

  // step2: initiate shutdown sequence
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int signal = SIGINT;
#else
  int signal = SIGQUIT;
#endif
  result = ACE_OS::raise (signal);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));

  // step3: stop GTK event processing
  // *NOTE*: triggering UI shutdown here is more consistent, compared to doing
  //         it from the signal handler
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                      true); // high priority ?

  return FALSE;
} // button_quit_clicked_cb

void
textview_size_allocate_cb (GtkWidget* widget_in,
                           GdkRectangle* rectangle_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::textview_size_allocate_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (rectangle_in);

  // sanity check(s)
//  struct Test_I_GTK_CBData* ui_cb_data_p =
//    static_cast<struct Test_I_GTK_CBData*> (userData_in);
//  ACE_ASSERT (ui_cb_data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  ACE_ASSERT(iterator != state_r.builders.end ());

  GtkScrolledWindow* scrolled_window_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_SCROLLED_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SCROLLEDWINDOW_NAME)));
  ACE_ASSERT (scrolled_window_p);
  GtkAdjustment* adjustment_p =
    gtk_scrolled_window_get_vadjustment (scrolled_window_p);
  ACE_ASSERT (adjustment_p);
  gtk_adjustment_set_value (adjustment_p,
                            gtk_adjustment_get_upper (adjustment_p) - gtk_adjustment_get_page_size (adjustment_p));
} // textview_size_allocate_cb

//////////////////////////////////////////

void
filechooserdialog_cb (GtkFileChooser* chooser_in,
                      gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserdialog_cb"));

  ACE_UNUSED_ARG (userData_in);

//  Stream_GTK_CBData* ui_cb_data_p =
//    static_cast<Stream_GTK_CBData*> (userData_in);

//  // sanity check(s)
//  ACE_ASSERT (ui_cb_data_p);

  gtk_dialog_response (GTK_DIALOG (GTK_FILE_CHOOSER_DIALOG (chooser_in)),
                       GTK_RESPONSE_ACCEPT);
} // filechooserdialog_cb

#ifdef __cplusplus
}
#endif /* __cplusplus */
