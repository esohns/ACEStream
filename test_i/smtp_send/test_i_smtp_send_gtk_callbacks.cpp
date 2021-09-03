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

#include "test_i_smtp_send_gtk_callbacks.h"

#include "ace/config-lite.h"
#include "gdk/gdkkeysyms.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gdk/gdkwin32.h"
#else
#include "gdk/gdkx.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "common_timer_manager.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_tools.h"

#include "stream_macros.h"

#include "net_common_tools.h"

#include "test_i_smtp_send_common.h"
#include "test_i_smtp_send_defines.h"
#include "test_i_smtp_send_stream.h"

// global variables
bool un_toggling_stream = false;

ACE_THR_FUNC_RETURN
stream_processing_function (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_processing_function"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("stream processing thread (id: %t) starting\n")));

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = std::numeric_limits<unsigned long>::max ();
#else
  result = arg_in;
#endif // ACE_WIN32 || ACE_WIN64

  struct Stream_SMTPSend_UI_ThreadData* thread_data_p =
      static_cast<struct Stream_SMTPSend_UI_ThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (thread_data_p);
  ACE_ASSERT (thread_data_p->CBData);

  Common_UI_GTK_BuildersIterator_t iterator;
    //  GtkProgressBar* progress_bar_p = NULL;
  GtkStatusbar* statusbar_p = NULL;
  std::ostringstream converter;
  //ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->CBData->UIState->lock);
  Stream_IStreamControlBase* stream_p = NULL;

  struct Stream_SMTPSend_UI_CBData* cb_data_p =
    static_cast<struct Stream_SMTPSend_UI_CBData*> (thread_data_p->CBData);
  ACE_ASSERT (cb_data_p->configuration);
  ACE_ASSERT (cb_data_p->stream);
  const SMTP_Stream_SessionData_t* session_data_container_p = NULL;
  const struct SMTP_Stream_SessionData* session_data_p = NULL;

  iterator =
    thread_data_p->CBData->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != thread_data_p->CBData->UIState->builders.end ());

  // retrieve progress bar handle
  gdk_threads_enter ();
//    progress_bar_p =
//      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
//                                                ACE_TEXT_ALWAYS_CHAR (TEST_USTREAM_UI_GTK_PROGRESSBAR_NAME)));
//    ACE_ASSERT (progress_bar_p);

  // generate context id
  statusbar_p =
    GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_STATUSBAR_NAME)));
  ACE_ASSERT (statusbar_p);

  gdk_threads_leave ();

  if (!cb_data_p->stream->initialize (cb_data_p->configuration->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_SMTPSend_Stream::initialize(), aborting\n")));
    goto error;
  } // end IF
  stream_p = cb_data_p->stream;
  session_data_container_p = &cb_data_p->stream->getR_2 ();
  session_data_p = &session_data_container_p->getR ();
  thread_data_p->sessionId = session_data_p->sessionId;
  converter << session_data_p->sessionId;

  // generate context id
  gdk_threads_enter ();
  thread_data_p->CBData->UIState->contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_INFORMATION,
                                                                     gtk_statusbar_get_context_id (statusbar_p,
                                                                                                   converter.str ().c_str ())));
  gdk_threads_leave ();

  ACE_ASSERT (stream_p);
  stream_p->start ();
  ACE_ASSERT (stream_p->isRunning ());
  stream_p->wait (true, false, false);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif // ACE_WIN32 || ACE_WIN64

error:
  //guint event_source_id = g_idle_add (idle_session_end_cb,
  //                                    data_p->CBData);
  //if (event_source_id == 0)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", continuing\n")));
  //else
  //  data_p->CBData->eventSourceIds.insert (event_source_id);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, thread_data_p->CBData->UIState->lock, -1);
#else
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, thread_data_p->CBData->UIState->lock, std::numeric_limits<void*>::max ());
#endif // ACE_WIN32 || ACE_WIN64
    thread_data_p->CBData->progressData.completedActions.insert (thread_data_p->eventSourceId);
  } // end lock scope

  // clean up
  delete thread_data_p; thread_data_p = NULL;

  return result;
}

/////////////////////////////////////////

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct Stream_SMTPSend_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_SMTPSend_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->UIState);

  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);

  GtkWidget* about_dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (about_dialog_p);

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

  GtkEntry* entry_p = 
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_SERVER_NAME)));
  ACE_ASSERT (entry_p);
  std::string hostname_string =
    Net_Common_Tools::IPAddressToString (ui_cb_data_p->configuration->address, true, true);
  if (hostname_string.empty ()) // failed to resolve --> use dotted-decimal
    hostname_string =
      Net_Common_Tools::IPAddressToString (ui_cb_data_p->configuration->address, true, false);
  gtk_entry_set_text (entry_p,
                      hostname_string.c_str ());
  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<gdouble> (ui_cb_data_p->configuration->address.get_port_number ()));
  entry_p = 
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_USERNAME_NAME)));
  ACE_ASSERT (entry_p);
  gtk_entry_set_text (entry_p,
                      ui_cb_data_p->configuration->username.c_str ());
  entry_p = 
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_PASSWORD_NAME)));
  ACE_ASSERT (entry_p);
  gtk_entry_set_text (entry_p,
                      ui_cb_data_p->configuration->password.c_str ());
  entry_p = 
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_FROM_NAME)));
  ACE_ASSERT (entry_p);
  gtk_entry_set_text (entry_p,
                      ui_cb_data_p->configuration->from.c_str ());
  entry_p = 
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_TO_NAME)));
  ACE_ASSERT (entry_p);
  gtk_entry_set_text (entry_p,
                      ui_cb_data_p->configuration->to.c_str ());
  GtkTextBuffer* text_buffer_p = 
    GTK_TEXT_BUFFER (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTBUFFER_NAME)));
  ACE_ASSERT (text_buffer_p);
  gtk_text_buffer_set_text (text_buffer_p,
                            ui_cb_data_p->configuration->message.c_str (),
                            ui_cb_data_p->configuration->message.size ());

  //Stream_SMTPSend_StreamConfiguration_t::ITERATOR_T iterator_2 =
  //  ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  //ACE_ASSERT (iterator_2 != ui_cb_data_p->configuration->streamConfiguration.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
                                   1.0 / static_cast<double> (width));
  gtk_progress_bar_set_text (progress_bar_p,
                             ACE_TEXT_ALWAYS_CHAR (""));

  // step4: initialize text view, setup auto-scrolling
  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);

  PangoFontDescription* font_description_p =
    pango_font_description_from_string (ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PANGO_LOG_FONT_DESCRIPTION));
  if (!font_description_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pango_font_description_from_string(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_I_UI_GTK_PANGO_LOG_FONT_DESCRIPTION)));
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
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PANGO_LOG_COLOR_BASE),
                   &base_colour);
  rc_style_p->base[GTK_STATE_NORMAL] = base_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PANGO_LOG_COLOR_TEXT),
                   &text_colour);
  rc_style_p->text[GTK_STATE_NORMAL] = text_colour;
  rc_style_p->color_flags[GTK_STATE_NORMAL] =
    static_cast<GtkRcFlags> (GTK_RC_BASE |
                             GTK_RC_TEXT);
  gtk_widget_modify_style (GTK_WIDGET (view_p),
                           rc_style_p);
  //gtk_rc_style_unref (rc_style_p);
  g_object_unref (rc_style_p);

  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);
  GtkTextIter iterator_3;
  gtk_text_buffer_get_end_iter (buffer_p,
                                &iterator_3);
  gtk_text_buffer_create_mark (buffer_p,
                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCROLLMARK_NAME),
                               &iterator_3,
                               TRUE);
  g_object_unref (buffer_p);

  // step5: initialize updates
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_p->UIState->lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the log views
    guint event_source_id =
//        g_timeout_add_seconds (1,
//                               idle_update_log_display_cb,
//                               userData_in);
//    if (event_source_id > 0)
//      ui_cb_data_p->UIState->eventSourceIds.insert (event_source_id);
//    else
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
//      return G_SOURCE_REMOVE;
//    } // end ELSE
//    // schedule asynchronous updates of the info view
//    event_source_id =
      g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET,
                     idle_update_info_display_cb,
                     userData_in);
    if (event_source_id > 0)
      ui_cb_data_p->UIState->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step2: (auto-)connect signals/slots
  gtk_builder_connect_signals ((*iterator).second.second,
                               userData_in);

  // step6a: connect default signals
  gulong result_2 =
      g_signal_connect (dialog_p,
                        ACE_TEXT_ALWAYS_CHAR ("destroy"),
                        G_CALLBACK (gtk_widget_destroyed),
                        NULL);
  ACE_ASSERT (result_2);

  result_2 = g_signal_connect_swapped (G_OBJECT (about_dialog_p),
                                       ACE_TEXT_ALWAYS_CHAR ("response"),
                                       G_CALLBACK (gtk_widget_hide),
                                       about_dialog_p);
  ACE_ASSERT (result_2);

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  gtk_widget_show_all (dialog_p);

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  // sanity check(s)
  struct Stream_SMTPSend_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_SMTPSend_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  // leave GTK
  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

gboolean
idle_session_end_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_session_end_cb"));

  // sanity check(s)
  struct Stream_SMTPSend_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_SMTPSend_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  //// synch access
  //ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_p->UIState->lock, G_SOURCE_REMOVE);

  GtkAction* action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ACTION_SEND_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, TRUE);
  //GtkFrame* frame_p =
  //  GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
  //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_CONFIGURATION_NAME)));
  //ACE_ASSERT (frame_p);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  // sanity check(s)
  struct Stream_SMTPSend_UI_CBData* ui_cb_data_p =
      static_cast<struct Stream_SMTPSend_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Common_UI_EventType* event_p = NULL;
  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_p->UIState->lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (ui_cb_data_p->UIState->eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_STARTED:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (ui_cb_data_p->progressData.statistic.bytes));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          break;
        }
        case COMMON_UI_EVENT_FINISHED:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STATISTIC:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          is_session_message = true;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown event type (was: %d), continuing\n"),
                      event_e));
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
    while (!ui_cb_data_p->UIState->eventStack.is_empty ())
    {
      result = ui_cb_data_p->UIState->eventStack.pop (event_e);
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
  struct Stream_SMTPSend_ProgressData* progress_data_p =
      static_cast<struct Stream_SMTPSend_ProgressData*> (userData_in);
  ACE_ASSERT (progress_data_p);
  ACE_ASSERT (progress_data_p->state);
  Common_UI_GTK_BuildersIterator_t iterator =
    progress_data_p->state->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != progress_data_p->state->builders.end ());

  // synch access

  int result = -1;

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_THR_FUNC_RETURN exit_status;
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  Common_UI_GTK_PendingActionsIterator_t iterator_2;
  bool done = false;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, progress_data_p->state->lock, G_SOURCE_REMOVE);
    for (Common_UI_GTK_CompletedActionsIterator_t iterator_3 = progress_data_p->completedActions.begin ();
         iterator_3 != progress_data_p->completedActions.end ();
         ++iterator_3)
    {
      iterator_2 = progress_data_p->pendingActions.find (*iterator_3);
      ACE_ASSERT (iterator_2 != progress_data_p->pendingActions.end ());
      result = thread_manager_p->join ((*iterator_2).second.id (), &exit_status);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Thread_Manager::join(%d): \"%m\", continuing\n"),
                    (*iterator_2).second.id ()));
      else
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("thread %u has joined (status was: %u)\n"),
                    (*iterator_2).second.id (),
                    exit_status));
#else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("thread %u has joined (status was: %@)\n"),
                    (*iterator_2).second.id (),
                    exit_status));
#endif // ACE_WIN32 || ACE_WIN64
      } // end ELSE

      progress_data_p->state->eventSourceIds.erase (*iterator_3);
      progress_data_p->pendingActions.erase (iterator_2);
    } // end FOR
    progress_data_p->completedActions.clear ();

    if (progress_data_p->pendingActions.empty ())
    {
      //if (data_p->cursorType != GDK_LAST_CURSOR)
      //{
      //  GdkCursor* cursor_p = gdk_cursor_new (data_p->cursorType);
      //  if (!cursor_p)
      //  {
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("failed to gdk_cursor_new(%d): \"%m\", continuing\n"),
      //                data_p->cursorType));
      //    return G_SOURCE_REMOVE;
      //  } // end IF
      //  GtkWindow* window_p =
      //    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
      //                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_MAIN)));
      //  ACE_ASSERT (window_p);
      //  GdkWindow* window_2 = gtk_widget_get_window (GTK_WIDGET (window_p));
      //  ACE_ASSERT (window_2);
      //  gdk_window_set_cursor (window_2, cursor_p);
      //  data_p->cursorType = GDK_LAST_CURSOR;
      //} // end IF

      done = true;
    } // end IF
  } // end lock scope

  std::ostringstream converter;
  converter << progress_data_p->statistic.messagesPerSecond;
  converter << ACE_TEXT_ALWAYS_CHAR (" fps");
  gtk_progress_bar_set_text (progress_bar_p,
                             (done ? ACE_TEXT_ALWAYS_CHAR ("")
                                   : converter.str ().c_str ()));
  gtk_progress_bar_pulse (progress_bar_p);

  // reschedule ?
  return (done ? G_SOURCE_REMOVE : G_SOURCE_CONTINUE);
}

//////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void
action_send_activate_cb (GtkAction* action_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_send_activate_cb"));

  // sanity check(s)
  struct Stream_SMTPSend_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_SMTPSend_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());
  Stream_IStreamControlBase* stream_p = ui_cb_data_p->stream;
  ACE_ASSERT (stream_p);
  Stream_SMTPSend_StreamConfiguration_t::ITERATOR_T iterator_2 =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != ui_cb_data_p->configuration->streamConfiguration.end ());
  ACE_ASSERT ((*iterator_2).second.second.protocolConfiguration);
  ACE_ASSERT ((*iterator_2).second.second.request);
  Net_ConnectionConfigurationsIterator_t iterator_3;

  int result = -1;
  GtkTextIter start, end;
  struct Stream_SMTPSend_UI_ThreadData* thread_data_p = NULL;
  ACE_TCHAR thread_name[BUFSIZ];
  const char* thread_name_2 = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_thread_t thread_id = std::numeric_limits<unsigned long>::max ();
#else
  ACE_thread_t thread_id = -1;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  ACE_Thread_Manager* thread_manager_p = NULL;
  bool join_b = false;
  GtkTextBuffer* text_buffer_p = NULL;

  // step1: deactivate some widgets
  gtk_action_set_sensitive (action_in, FALSE);
  //GtkFrame* frame_p =
  //  GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
  //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_CONFIGURATION_NAME)));
  //ACE_ASSERT (frame_p);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);

  // step2: set up progress reporting
  ACE_OS::memset (&ui_cb_data_p->progressData.statistic,
                  0,
                  sizeof (SMTP_Statistic_t));
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_fraction (progress_bar_p, 0.0);
  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), TRUE);

  // step3: update configuration
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_SERVER_NAME)));
  ACE_ASSERT (entry_p);
  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  result =
    ui_cb_data_p->configuration->address.set (static_cast<u_short> (gtk_spin_button_get_value_as_int (spin_button_p)),
                                              ACE_TEXT_ALWAYS_CHAR (gtk_entry_get_text (entry_p)),
                                              1, // encode
                                              AF_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(%d,%s): \"%m\", returning\n"),
                gtk_spin_button_get_value_as_int (spin_button_p),
                ACE_TEXT (gtk_entry_get_text (entry_p))));
    goto error;
  } // end IF
  iterator_3 =
    (*iterator_2).second.second.connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != (*iterator_2).second.second.connectionConfigurations->end ());
  NET_CONFIGURATION_TCP_CAST ((*iterator_3).second)->socketConfiguration.address =
    ui_cb_data_p->configuration->address;
  entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_USERNAME_NAME)));
  ACE_ASSERT (entry_p);
  ui_cb_data_p->configuration->username =
    ACE_TEXT_ALWAYS_CHAR (gtk_entry_get_text (entry_p));
  entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_PASSWORD_NAME)));
  ACE_ASSERT (entry_p);
  ui_cb_data_p->configuration->password =
    ACE_TEXT_ALWAYS_CHAR (gtk_entry_get_text (entry_p));
  entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_FROM_NAME)));
  ACE_ASSERT (entry_p);
  ui_cb_data_p->configuration->from =
    ACE_TEXT_ALWAYS_CHAR (gtk_entry_get_text (entry_p));
  entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_TO_NAME)));
  ACE_ASSERT (entry_p);
  ui_cb_data_p->configuration->to =
    ACE_TEXT_ALWAYS_CHAR (gtk_entry_get_text (entry_p));
  text_buffer_p =
    GTK_TEXT_BUFFER (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTBUFFER_NAME)));
  ACE_ASSERT (text_buffer_p);
  gtk_text_buffer_get_start_iter (text_buffer_p, &start);
  gtk_text_buffer_get_end_iter (text_buffer_p, &end);
  ui_cb_data_p->configuration->message =
    ACE_TEXT_ALWAYS_CHAR (gtk_text_buffer_get_text (text_buffer_p,
                                                    &start, &end,
                                                    TRUE));

  (*iterator_2).second.second.protocolConfiguration->username =
    ui_cb_data_p->configuration->username;
  (*iterator_2).second.second.protocolConfiguration->password =
    ui_cb_data_p->configuration->password;
  (*iterator_2).second.second.request->from = ui_cb_data_p->configuration->from;
  (*iterator_2).second.second.request->to.push_back (ui_cb_data_p->configuration->to);
  (*iterator_2).second.second.request->data = ui_cb_data_p->configuration->message;

  // step4: start processing thread
  ACE_NEW_NORETURN (thread_data_p,
                    struct Stream_SMTPSend_UI_ThreadData ());
  if (!thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  thread_data_p->CBData = ui_cb_data_p;
  ACE_OS::memset (thread_name, 0, sizeof (thread_name));
  ACE_OS::strcpy (thread_name,
                  ACE_TEXT (TEST_I_THREAD_NAME));
  thread_name_2 = static_cast<char*> (thread_name);
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);

  // *NOTE*: lock access to the progress report structures to avoid a race
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_p->UIState->lock);
    result =
      thread_manager_p->spawn (::stream_processing_function,     // function
                               thread_data_p,                    // argument
                               (THR_NEW_LWP      |
                                THR_JOINABLE     |
                                THR_INHERIT_SCHED),              // flags
                               &thread_id,                       // thread id
                               &thread_handle,                   // thread handle
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
      goto error;
    } // end IF

    // step5: start progress reporting
    //ACE_ASSERT (!data_p->progressData.eventSourceId);
    ui_cb_data_p->progressData.eventSourceId =
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_progress_cb,
      //                 &data_p->progressData,
      //                 NULL);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                          COMMON_UI_REFRESH_DEFAULT_PROGRESS, // ms (?)
                          idle_update_progress_cb,
                          &ui_cb_data_p->progressData,
                          NULL);
    if (!ui_cb_data_p->progressData.eventSourceId)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(): \"%m\", returning\n")));
      join_b = true;
      goto error;
    } // end IF
    thread_data_p->eventSourceId = ui_cb_data_p->progressData.eventSourceId;
    ui_cb_data_p->progressData.pendingActions[ui_cb_data_p->progressData.eventSourceId] =
      ACE_Thread_ID (thread_id, thread_handle);
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("idle_update_progress_cb: %d\n"),
//                event_source_id));
    ui_cb_data_p->UIState->eventSourceIds.insert (ui_cb_data_p->progressData.eventSourceId);
  } // end lock scope

  return;

error:
  if (join_b)
  {
    ACE_THR_FUNC_RETURN exit_status;
    result = thread_manager_p->join (thread_id, &exit_status);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::join(%d): \"%m\", continuing\n"),
                  thread_id));
  } // end IF

  gtk_action_set_sensitive (action_in, TRUE);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
  //                          TRUE);
} // eaction_send_activated_cb

//gint
//button_clear_clicked_cb (GtkWidget* widget_in,
//                         gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

//  ACE_UNUSED_ARG (widget_in);
//  struct Stream_SMTPSend_UI_CBData* ui_cb_data_p =
//    static_cast<struct Stream_SMTPSend_UI_CBData*> (userData_in);

//  // sanity check(s)
//  ACE_ASSERT (ui_cb_data_p);

//  Common_UI_GTK_BuildersIterator_t iterator =
//    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

//  GtkTextView* view_p =
//    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
//                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTVIEW_NAME)));
//  ACE_ASSERT (view_p);
//  GtkTextBuffer* buffer_p =
////    gtk_text_buffer_new (NULL); // text tag table --> create new
//    gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);
//  gtk_text_buffer_set_text (buffer_p,
//                            ACE_TEXT_ALWAYS_CHAR (""), 0);

//  return FALSE;
//}

gint
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Stream_SMTPSend_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_SMTPSend_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  // retrieve about dialog handle
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!dialog_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
    return TRUE; // propagate
  } // end IF

  // run dialog
  gint result = gtk_dialog_run (dialog_p);
  switch (result)
  {
    case GTK_RESPONSE_ACCEPT:
      break;
    default:
      break;
  } // end SWITCH
  //gtk_widget_hide (GTK_WIDGET (dialog_p));

  return FALSE;
} // button_about_clicked_cb

gint
button_quit_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Stream_SMTPSend_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_SMTPSend_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  enum Stream_StateMachine_ControlState status_e = STREAM_STATE_INVALID;
  Stream_IStreamControlBase* stream_p = NULL;
  status_e = ui_cb_data_p->stream->status ();
  stream_p = ui_cb_data_p->stream;
  ACE_ASSERT (stream_p);

  //// step1: remove event sources
  //{ ACE_Guard<ACE_Thread_Mutex> aGuard (data_p->lock);
  //  for (Common_UI_GTKEventSourceIdsIterator_t iterator = data_p->eventSourceIds.begin ();
  //       iterator != data_p->eventSourceIds.end ();
  //       iterator++)
  //    if (!g_source_remove (*iterator))
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
  //                  *iterator));
  //  data_p->eventSourceIds.clear ();
  //} // end lock scope

  // stop stream ?
  if ((status_e == STREAM_STATE_RUNNING) ||
      (status_e == STREAM_STATE_PAUSED))
    stream_p->stop (false, true, true);

  // wait for processing thread(s)
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_p->UIState->lock, FALSE);
    if (!ui_cb_data_p->progressData.pendingActions.empty ())
    {
      // *IMPORTANT NOTE*: cannot wait on the UI condition here, as it is
      //                   signal()ed by the current thread !
      //                   --> emit a signal to come back
      gtk_signal_emit_by_name (GTK_OBJECT (widget_in), "clicked");
    } // end IF
  } // end lock scope

  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                      true); // high priority ?

  // step2: initiate shutdown sequence
  int result = ACE_OS::raise (SIGINT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                SIGINT));

  return FALSE;
} // button_quit_clicked_cb

void
textview_size_allocate_cb (GtkWidget* widget_in,
                           GdkRectangle* rectangle_in,
                           gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::textview_size_allocate_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (rectangle_in);

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct Stream_SMTPSend_UI_CBData* ui_cb_data_p =
      static_cast<struct Stream_SMTPSend_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  GtkScrolledWindow* scrolled_window_p =
    GTK_SCROLLED_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCROLLEDWINDOW_NAME)));
  ACE_ASSERT (scrolled_window_p);
  GtkAdjustment* adjustment_p =
    gtk_scrolled_window_get_vadjustment (scrolled_window_p);
  ACE_ASSERT (adjustment_p);
  gtk_adjustment_set_value (adjustment_p,
                            gtk_adjustment_get_upper (adjustment_p) - gtk_adjustment_get_page_size (adjustment_p));
} // textview_size_allocate_cb
#ifdef __cplusplus
}
#endif /* __cplusplus */
