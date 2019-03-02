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

#include <sstream>

#include "ace/Synch.h"
#include "test_u_imagescreen_gtk_callbacks.h"

#include "ace/config-lite.h"

#include "gdk/gdkkeysyms.h"

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch_Traits.h"

#include "common_timer_manager.h"

#include "common_ui_defines.h"
#include "common_ui_ifullscreen.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_tools.h"

#include "test_u_imagescreen_defines.h"
#include "test_u_imagescreen_stream.h"

static bool untoggling_stream_b = false;

void
load_entries (GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_entries"));

  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  std::ostringstream converter;
  for (unsigned int i = 0;
       i < 10;
       ++i)
  {
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << i;

    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, i,
                        -1);
  } // end FOR
}

/////////////////////////////////////////

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  // sanity check(s)
  struct Stream_ImageScreen_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_ImageScreen_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());
  Stream_ImageScreen_StreamConfiguration_t::ITERATOR_T stream_configuration_iterator =
      ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  //  GtkWidget* image_icon_p = gtk_image_new_from_file (path.c_str ());
  //  ACE_ASSERT (image_icon_p);
  //  gtk_window_set_icon (GTK_WINDOW (dialog_p),
  //                       gtk_image_get_pixbuf (GTK_IMAGE (image_icon_p)));
  //GdkWindow* dialog_window_p = gtk_widget_get_window (dialog_p);
  //gtk_window4096_set_title (,
  //                      caption.c_str ());

  GtkWidget* about_dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (about_dialog_p);

  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);

  // step2: (auto-)connect signals/slots
  gtk_builder_connect_signals ((*iterator).second.second,
                               ui_cb_data_p);

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

  // set defaults
  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_FILECHOOSERBUTTON_DIRECTORY_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  std::string default_folder_uri = ACE_TEXT_ALWAYS_CHAR ("file://");
  default_folder_uri +=
      (*stream_configuration_iterator).second.second.fileIdentifier.identifier;
  gboolean result =
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p),
                                         ACE_TEXT_ALWAYS_CHAR ((*stream_configuration_iterator).second.second.fileIdentifier.identifier.c_str ()));
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT ((*stream_configuration_iterator).second.second.fileIdentifier.identifier.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF

  // step9: draw main dialog
  gtk_widget_show_all (dialog_p);

  // step10: retrieve window handle
  (*stream_configuration_iterator).second.second.window =
      gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT ((*stream_configuration_iterator).second.second.window);
  GtkAllocation allocation_s;
  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
                             &allocation_s);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  (*stream_configuration_iterator).second.second.outputFormat.resolution.cx =
#else
  (*stream_configuration_iterator).second.second.outputFormat.resolution.width =
#endif // ACE_WIN32 || ACE_WIN64
    allocation_s.width;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  (*stream_configuration_iterator).second.second.outputFormat.resolution.cy =
#else
  (*stream_configuration_iterator).second.second.outputFormat.resolution.height =
#endif // ACE_WIN32 || ACE_WIN64
      allocation_s.height;
  ui_cb_data_p->configuration->streamConfiguration.configuration_.format.resolution =
      (*stream_configuration_iterator).second.second.outputFormat.resolution;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("initial window size: %ux%u\n"),
              allocation_s.width, allocation_s.height));

//  (*stream_configuration_iterator).second.second.pixelBuffer =
//#if GTK_CHECK_VERSION (3,0,0)
//          gdk_pixbuf_get_from_window ((*stream_configuration_iterator).second.second.window,
//                                      0, 0,
//                                      allocation_s.width, allocation_s.height);
//#else
//          gdk_pixbuf_get_from_drawable (NULL,
//                                        GDK_DRAWABLE ((*stream_configuration_iterator).second.second.window),
//                                        NULL,
//                                        0, 0,
//                                        0, 0, allocation_s.width, allocation_s.height);
//#endif
//  if (!(*stream_configuration_iterator).second.second.pixelBuffer)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gdk_pixbuf_get_from_window(), aborting\n")));
//    return FALSE;
//  } // end IF

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  ACE_UNUSED_ARG (userData_in);

  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

//gboolean
//idle_update_log_display_cb (gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));

//  struct Stream_ImageScreen_UI_CBData* ui_cb_data_p =
//    static_cast<struct Stream_ImageScreen_UI_CBData*> (userData_in);

//  // sanity check(s)
//  ACE_ASSERT (ui_cb_data_p);
//  Common_UI_GTK_BuildersIterator_t iterator =
//    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

//  GtkTextView* view_p =
//      GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
//                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TEXTVIEW_NAME)));
//  ACE_ASSERT (view_p);
//  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);

//  GtkTextIter text_iterator;
//  gtk_text_buffer_get_end_iter (buffer_p,
//                                &text_iterator);

//  gchar* converted_text = NULL;
//  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_p->UIState->lock, G_SOURCE_REMOVE);
//    if (ui_cb_data_p->UIState->logStack.empty ())
//      return G_SOURCE_CONTINUE;

//    // step1: convert text
//    for (Common_MessageStackConstIterator_t iterator_2 = ui_cb_data_p->UIState->logStack.begin ();
//         iterator_2 != ui_cb_data_p->UIState->logStack.end ();
//         ++iterator_2)
//    {
//      converted_text = Common_UI_GTK_Tools::localeToUTF8 (*iterator_2);
//      if (!converted_text)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to Common_UI_GTK_Tools::localeToUTF8(\"%s\"), aborting\n"),
//                    ACE_TEXT ((*iterator_2).c_str ())));
//        return G_SOURCE_REMOVE;
//      } // end IF

//      // step2: display text
//      gtk_text_buffer_insert (buffer_p,
//                              &text_iterator,
//                              converted_text,
//                              -1);

//      g_free (converted_text); converted_text = NULL;
//    } // end FOR
//    ui_cb_data_p->UIState->logStack.clear ();
//  } // end lock scope

//  // step3: scroll the view accordingly
////  // move the iterator to the beginning of line, so it doesn't scroll
////  // in horizontal direction
////  gtk_text_iter_set_line_offset (&text_iterator, 0);

////  // ...and place the mark at iter. The mark will stay there after insertion
////  // because it has "right" gravity
////  GtkTextMark* text_mark_p =
////      gtk_text_buffer_get_mark (buffer_p,
////                                ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SCROLLMARK_NAME));
//////  gtk_text_buffer_move_mark (buffer_p,
//////                             text_mark_p,
//////                             &text_iterator);

////  // scroll the mark onscreen
////  gtk_text_view_scroll_mark_onscreen (view_p,
////                                      text_mark_p);
//  GtkAdjustment* adjustment_p =
//      GTK_ADJUSTMENT (gtk_builder_get_object ((*iterator).second.second,
//                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ADJUSTMENT_NAME)));
//  ACE_ASSERT (adjustment_p);
//  gtk_adjustment_set_value (adjustment_p,
//                            gtk_adjustment_get_upper (adjustment_p));

//  return G_SOURCE_CONTINUE;
//}

//gboolean
//idle_update_info_display_cb (gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

//  // sanity check(s)
//  ACE_ASSERT (userData_in);
//  struct Stream_ImageScreen_UI_CBData* ui_cb_data_p =
//      static_cast<struct Stream_ImageScreen_UI_CBData*> (userData_in);
//  Common_UI_GTK_BuildersIterator_t iterator =
//    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

//  GtkSpinButton* spin_button_p = NULL;
//  bool is_session_message = false;
//  enum Common_UI_EventType* event_p = NULL;
//  int result = -1;
//  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
//  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_p->UIState->lock, G_SOURCE_REMOVE);
//    for (Common_UI_Events_t::ITERATOR iterator_2 (ui_cb_data_p->UIState->eventStack);
//         iterator_2.next (event_p);
//         iterator_2.advance ())
//    { ACE_ASSERT (event_p);
//      switch (*event_p)
//      {
//        case COMMON_UI_EVENT_STARTED:
//        {
//          spin_button_p =
//            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
//          ACE_ASSERT (spin_button_p);
//          gtk_spin_button_set_value (spin_button_p, 0.0);
//          spin_button_p =
//            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
//          ACE_ASSERT (spin_button_p);
//          gtk_spin_button_set_value (spin_button_p, 0.0);
//          is_session_message = true;
//          break;
//        }
//        case COMMON_UI_EVENT_DATA:
//        {
//          spin_button_p =
//            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATA_NAME)));
//          ACE_ASSERT (spin_button_p);
//          gtk_spin_button_set_value (spin_button_p,
//                                     static_cast<gdouble> (ui_cb_data_p->progressData.statistic.bytes));

//          spin_button_p =
//            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
//          ACE_ASSERT (spin_button_p);
//          break;
//        }
//        case COMMON_UI_EVENT_FINISHED:
//        {
//          spin_button_p =
//            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
//          ACE_ASSERT (spin_button_p);
//          is_session_message = true;
//          break;
//        }
//        case COMMON_UI_EVENT_STATISTIC:
//        {
////#if defined (ACE_WIN32) || defined (ACE_WIN64)
////          spin_button_p =
////            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
////                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_CAPTUREDFRAMES_NAME)));
////          ACE_ASSERT (spin_button_p);
////          gtk_spin_button_set_value (spin_button_p,
////                                     static_cast<gdouble> (ui_cb_data_p->progressData.statistic.capturedFrames));
////#endif
////
////          spin_button_p =
////            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
////                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DROPPEDFRAMES_NAME)));
////          ACE_ASSERT (spin_button_p);
////          gtk_spin_button_set_value (spin_button_p,
////                                     static_cast<gdouble> (ui_cb_data_p->progressData.statistic.droppedFrames));

//          spin_button_p =
//            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
//          ACE_ASSERT (spin_button_p);

//          is_session_message = true;
//          break;
//        }
//        default:
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("invalid/unknown event type (was: %d), continuing\n"),
//                      event_e));
//          break;
//        }
//      } // end SWITCH
//      ACE_UNUSED_ARG (is_session_message);
//      gtk_spin_button_spin (spin_button_p,
//                            GTK_SPIN_STEP_FORWARD,
//                            1.0);
//      event_p = NULL;
//    } // end FOR

//    // clean up
//    while (!ui_cb_data_p->UIState->eventStack.is_empty ())
//    {
//      result = ui_cb_data_p->UIState->eventStack.pop (event_e);
//      if (result == -1)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_Unbounded_Stack::pop(): \"%m\", continuing\n")));
//    } // end WHILE
//  } // end lock scope

//  return G_SOURCE_CONTINUE;
//}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  struct Common_UI_GTK_ProgressData* data_p =
      static_cast<struct Common_UI_GTK_ProgressData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->state);

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->state->lock, G_SOURCE_REMOVE);

//  int result = -1;
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->state->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->state->builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

//  ACE_THR_FUNC_RETURN exit_status;
//  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
//  ACE_ASSERT (thread_manager_p);
  Common_UI_GTK_PendingActionsIterator_t iterator_2;
  for (Common_UI_GTK_CompletedActionsIterator_t iterator_3 = data_p->completedActions.begin ();
       iterator_3 != data_p->completedActions.end ();
       ++iterator_3)
  {
    iterator_2 = data_p->pendingActions.find (*iterator_3);
    ACE_ASSERT (iterator_2 != data_p->pendingActions.end ());
//    ACE_thread_t thread_id = (*iterator_2).second.id ();
//    result = thread_manager_p->join (thread_id, &exit_status);
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Thread_Manager::join(%d): \"%m\", continuing\n"),
//                  thread_id));
//    else
//    {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("thread %u has joined (status was: %u)\n"),
//                  thread_id,
//                  exit_status));
//#else
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("thread %u has joined (status was: 0x%@)\n"),
//                  thread_id,
//                  exit_status));
//#endif
//    } // end ELSE

    data_p->state->eventSourceIds.erase (*iterator_3);
    data_p->pendingActions.erase (iterator_2);
  } // end FOR
  data_p->completedActions.clear ();

  bool done = false;
  if (data_p->pendingActions.empty ())
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

    data_p->eventSourceId = 0;

    done = true;
  } // end IF

  // synch access
  std::ostringstream converter;
  converter << data_p->eventSourceId;
  converter << ACE_TEXT_ALWAYS_CHAR (" fps");
  gtk_progress_bar_set_text (progress_bar_p,
                             (done ? ACE_TEXT_ALWAYS_CHAR ("")
                                   : converter.str ().c_str ()));
  gtk_progress_bar_pulse (progress_bar_p);

  // reschedule ?
  return (done ? G_SOURCE_REMOVE : G_SOURCE_CONTINUE);
}

gboolean
idle_update_video_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_video_display_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);

  struct Stream_ImageScreen_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_ImageScreen_UI_CBData*> (userData_in);

  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);

  gdk_window_invalidate_rect (gtk_widget_get_window (GTK_WIDGET (drawing_area_p)),
                              NULL,
                              false);

  return G_SOURCE_REMOVE;
}

gboolean
idle_session_end_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_session_end_cb"));

  // sanity check(s)
  struct Stream_ImageScreen_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_ImageScreen_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
  ACE_ASSERT (toggle_button_p);
  ACE_ASSERT (gtk_toggle_button_get_active (toggle_button_p));

  untoggling_stream_b = true;
  gtk_toggle_button_set_active (toggle_button_p,
                                FALSE);

  ACE_ASSERT (ui_cb_data_p->progressData.eventSourceId);
  ui_cb_data_p->progressData.completedActions.insert (ui_cb_data_p->progressData.eventSourceId);

  return G_SOURCE_REMOVE;
}

//////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void
togglebutton_start_toggled_cb (GtkToggleButton* toggleButton_in,
                               gpointer userData_in)
{

  // sanity check(s)
  struct Stream_ImageScreen_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_ImageScreen_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  bool is_active_b =
      gtk_toggle_button_get_active (toggleButton_in);

  gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                        (is_active_b ? GTK_STOCK_MEDIA_STOP : GTK_STOCK_MEDIA_PLAY));

  if (untoggling_stream_b)
  {
    untoggling_stream_b = false;
    return;
  } // end IF

  // toggle ?
  if (!is_active_b)
  {
    // --> user pressed pause/stop
    ACE_ASSERT (ui_cb_data_p->progressData.eventSourceId);
    ui_cb_data_p->progressData.completedActions.insert (ui_cb_data_p->progressData.eventSourceId);

    // stop stream
    ui_cb_data_p->stream->stop (true,  // wait ?
                                true); // locked access ?

    return;
  } // end IF

  if (!ui_cb_data_p->stream->initialize (ui_cb_data_p->configuration->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream, returning\n")));
    return;
  } // end IF

  // step3: start progress reporting
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
                ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", returning\n")));
    return;
  } // end lock scope
  ui_cb_data_p->progressData.pendingActions[ui_cb_data_p->progressData.eventSourceId] =
      ACE_Thread_ID (0, 0);
  //    ACE_DEBUG ((LM_DEBUG,
  //                ACE_TEXT ("idle_update_progress_cb: %d\n"),
  //                event_source_id));
  ui_cb_data_p->UIState->eventSourceIds.insert (ui_cb_data_p->progressData.eventSourceId);

  ui_cb_data_p->stream->start ();
} // toggleaction_record_toggled_cb

//void
//toggleaction_fullscreen_toggled_cb (GtkToggleAction* toggleAction_in,
//                                    gpointer userData_in)
//{
//  struct Stream_ImageScreen_UI_CBData* ui_cb_data_p =
//    static_cast<struct Stream_ImageScreen_UI_CBData*> (userData_in);

//  // sanity check(s)
//  ACE_ASSERT (ui_cb_data_p);

//  bool is_active_b =
//#if GTK_CHECK_VERSION(3,0,0)
//      gtk_toggle_action_get_active (toggleAction_in);
//#elif GTK_CHECK_VERSION(2,0,0)
//      gtk_toggle_button_get_active (toggleButton_in);
//#endif // GTK_CHECK_VERSION

//  Common_UI_GTK_BuildersIterator_t iterator =
//    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

//  Stream_IStreamControlBase* stream_base_p = NULL;
//  Stream_IStream_t* stream_p = NULL;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  struct Stream_CamSave_DirectShow_UI_CBData* directshow_cb_data_p = NULL;
//  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
//  struct Stream_CamSave_MediaFoundation_UI_CBData* mediafoundation_cb_data_p =
//    NULL;
//  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
//  switch (ui_cb_data_p->mediaFramework)
//  {
//    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//    {
//      directshow_cb_data_p =
//        static_cast<struct Stream_CamSave_DirectShow_UI_CBData*> (ui_cb_data_p);
//      stream_base_p = directshow_cb_data_p->stream;
//      stream_p = directshow_cb_data_p->stream;
//      ACE_ASSERT (directshow_cb_data_p->configuration);
//      directshow_stream_iterator =
//        directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//      ACE_ASSERT (directshow_stream_iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
//      (*directshow_stream_iterator).second.second.fullScreen = is_active;
//      break;
//    }
//    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//    {
//      mediafoundation_cb_data_p =
//        static_cast<struct Stream_CamSave_MediaFoundation_UI_CBData*> (ui_cb_data_p);
//      stream_base_p = mediafoundation_cb_data_p->stream;
//      stream_p = mediafoundation_cb_data_p->stream;
//      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
//      mediafoundation_stream_iterator =
//        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
//      (*mediafoundation_stream_iterator).second.second.fullScreen = is_active;
//      break;
//    }
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
//                  ui_cb_data_p->mediaFramework));
//      return;
//    }
//  } // end SWITCH
//#else
//  struct Stream_CamSave_V4L_UI_CBData* cb_data_p =
//    static_cast<struct Stream_CamSave_V4L_UI_CBData*> (ui_cb_data_p);
//  stream_base_p = cb_data_p->stream;
//  stream_p = cb_data_p->stream;
//  ACE_ASSERT (cb_data_p->configuration);
//  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
//    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
//  (*iterator_2).second.second.fullScreen = is_active_b;
//#endif
//  ACE_ASSERT (stream_base_p);
//  if (!stream_base_p->isRunning ())
//    return;

//  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());
//  GtkWindow* window_p =
//    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
//                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_WINDOW_FULLSCREEN)));
//  ACE_ASSERT (window_p);

//  if (is_active_b)
//  {
//    gtk_widget_show (GTK_WIDGET (window_p));
////  gtk_window_fullscreen (window_p);
//    gtk_window_maximize (window_p);
//  } // end IF
//  else
//  {
////    gtk_window_minimize (window_p);
////  gtk_window_unfullscreen (window_p);
//    gtk_widget_hide (GTK_WIDGET (window_p));
//  } // end ELSE

//  ACE_ASSERT (stream_p);
//  const Stream_Module_t* module_p = NULL;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  switch (ui_cb_data_p->mediaFramework)
//  {
//    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//      module_p =
//        stream_p->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING));
//      break;
//    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//      module_p =
//        stream_p->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING));
//      break;
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: invalid/unkown media framework (was: %d), returning\n"),
//                  ACE_TEXT (stream_p->name ().c_str ()),
//                  ui_cb_data_p->mediaFramework));
//      return;
//    }
//  } // end SWITCH
//#else
//  module_p =
//      stream_p->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING));
//#endif
//  if (!module_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to Stream_IStream::find(\"Display\"), returning\n"),
//                ACE_TEXT (stream_p->name ().c_str ())));
//    return;
//  } // end IF
//  Common_UI_IFullscreen* ifullscreen_p =
//    dynamic_cast<Common_UI_IFullscreen*> (const_cast<Stream_Module_t*> (module_p)->writer ());
//  if (!ifullscreen_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s:Display: failed to dynamic_cast<Common_UI_IFullscreen*>(0x%@), returning\n"),
//                ACE_TEXT (stream_p->name ().c_str ()),
//                const_cast<Stream_Module_t*> (module_p)->writer ()));
//    return;
//  } // end IF
//  try {
//    ifullscreen_p->toggle ();
//  } catch (...) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Common_UI_IFullscreen::toggle(), returning\n")));
//    return;
//  }
//} // toggleaction_fullscreen_toggled_cb

// -----------------------------------------------------------------------------

gint
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  ACE_UNUSED_ARG (widget_in);
  struct Stream_ImageScreen_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_ImageScreen_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  // retrieve about dialog handle
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!dialog_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to glade_xml_get_widget(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_U_UI_GTK_DIALOG_ABOUT_NAME)));
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
  gtk_widget_hide (GTK_WIDGET (dialog_p));

  return FALSE;
} // button_about_clicked_cb

gint
button_quit_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  ACE_UNUSED_ARG (widget_in);
  struct Stream_ImageScreen_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_ImageScreen_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);

  gtk_main_quit ();

  return FALSE;
} // button_quit_clicked_cb

//////////////////////////////////////////

void
filechooserbutton_current_folder_changed_cb (GtkFileChooser* fileChooser_in,
                                             gpointer userData_in)
{
  // sanity check(s)
  struct Stream_ImageScreen_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_ImageScreen_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());
  Stream_ImageScreen_StreamConfiguration_t::ITERATOR_T stream_configuration_iterator =
      ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  (*stream_configuration_iterator).second.second.fileIdentifier.identifier =
      ACE_TEXT_ALWAYS_CHAR (gtk_file_chooser_get_current_folder (fileChooser_in));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("changed directory to \"%s\"\n"),
              ACE_TEXT ((*stream_configuration_iterator).second.second.fileIdentifier.identifier.c_str ())));
} // filechooserbutton_current_folder_changed_cb

gboolean
drawingarea_expose_event_cb (GtkWidget* widget_in,
                             GdkEvent* event_in,
                             gpointer userData_in)
{
  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Stream_ImageScreen_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_ImageScreen_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  Stream_ImageScreen_StreamConfiguration_t::ITERATOR_T stream_configuration_iterator =
      ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  GdkWindow* window_p = gtk_widget_get_window (widget_in);

  // sanity check(s)
  ACE_ASSERT (window_p);
//  if (!(*stream_configuration_iterator).second.second.pixelBuffer)
//    return TRUE;

//  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *ui_cb_data_p->pixelBufferLock, FALSE);
//#if GTK_CHECK_VERSION(3,0,0)
//  cairo_set_source_surface (context_in,
//                            ui_cb_data_p->pixelBuffer,
//                            0.0, 0.0);
//#elif GTK_CHECK_VERSION(2,22,0)
//  cairo_t* cairo_p = gdk_cairo_create (GDK_DRAWABLE (window_p));
//  ACE_ASSERT (cairo_p);
//  gdk_cairo_set_source_pixbuf (cairo_p,
//                               (*stream_configuration_iterator).second.second.pixelBuffer,
//                               0.0, 0.0);
//  cairo_paint (cairo_p);
//  cairo_destroy (cairo_p); cairo_p = NULL;
//#elif GTK_CHECK_VERSION(2,2,0)
//  gdk_draw_pixbuf (GDK_DRAWABLE (window_p),
//                   NULL,
//                   (*stream_configuration_iterator).second.second.pixelBuffer,
//                   0, 0, 0, 0, -1, -1,
//                   GDK_RGB_DITHER_NONE, 0, 0);
//#endif // GTK_CHECK_VERSION
//  } // end lock scope

  return TRUE;
}

gboolean
drawingarea_configure_event_cb (GtkWindow* window_in,
                                GdkEvent* event_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_configure_event_cb"));

  // sanity check(s)
  Stream_ImageScreen_UI_CBData* ui_cb_data_p =
    static_cast<Stream_ImageScreen_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

//  Common_UI_GTK_BuildersIterator_t iterator =
//    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  // sanity check(s)
//  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());
  Stream_ImageScreen_StreamConfiguration_t::ITERATOR_T stream_configuration_iterator =
      ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  GtkAllocation allocation_s;
  gtk_widget_get_allocation (GTK_WIDGET (window_in),
                             &allocation_s);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  (*stream_configuration_iterator).second.second.outputFormat.resolution.cx =
#else
  (*stream_configuration_iterator).second.second.outputFormat.resolution.width =
#endif // ACE_WIN32 || ACE_WIN64
      allocation_s.width;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  (*stream_configuration_iterator).second.second.outputFormat.resolution.cy =
#else
  (*stream_configuration_iterator).second.second.outputFormat.resolution.height =
#endif // ACE_WIN32 || ACE_WIN64
      allocation_s.height;
  ui_cb_data_p->configuration->streamConfiguration.configuration_.format.resolution =
      (*stream_configuration_iterator).second.second.outputFormat.resolution;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("resized window to %ux%u\n"),
              allocation_s.width, allocation_s.height));

//  if ((*stream_configuration_iterator).second.second.pixelBuffer)
//  {
//    g_object_unref ((*stream_configuration_iterator).second.second.pixelBuffer);
//    (*stream_configuration_iterator).second.second.pixelBuffer = NULL;
//  } // end IF

//  (*stream_configuration_iterator).second.second.pixelBuffer =
//#if GTK_CHECK_VERSION (3,0,0)
//          gdk_pixbuf_get_from_window (window_in,
//                                      0, 0,
//                                      allocation_s.width, allocation_s.height);
//#else
//          gdk_pixbuf_get_from_drawable (NULL,
//                                        GDK_DRAWABLE (window_in),
//                                        NULL,
//                                        0, 0,
//                                        0, 0, allocation_s.width, allocation_s.height);
//#endif
//  if (!(*stream_configuration_iterator).second.second.pixelBuffer)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gdk_pixbuf_get_from_window(), aborting\n")));
//    return FALSE;
//  } // end IF

  return TRUE;
} // drawingarea_configure_event_cb

//void
//filechooserdialog_cb (GtkFileChooser* fileChooser_in,
//                      gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::filechooserdialog_cb"));
//
//  ACE_UNUSED_ARG (userData_in);
//
//  gtk_dialog_response (GTK_DIALOG (GTK_FILE_CHOOSER_DIALOG (fileChooser_in)),
//                       GTK_RESPONSE_ACCEPT);
//} // filechooserdialog_cb

gboolean
key_cb (GtkWidget* widget_in,
        GdkEventKey* eventKey_in,
        gpointer userData_in)
{
  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  ACE_ASSERT (eventKey_in);

  struct Stream_ImageScreen_UI_CBData* ui_cb_data_p =
      reinterpret_cast<struct Stream_ImageScreen_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  switch (eventKey_in->keyval)
  {
#if GTK_CHECK_VERSION(3,0,0)
    case GDK_KEY_Escape:
    case GDK_KEY_f:
    case GDK_KEY_F:
#else
    case GDK_Escape:
    case GDK_f:
    case GDK_F:
#endif // GTK_CHECK_VERSION(3,0,0)
    {
      bool is_active_b = false;
      GtkToggleButton* toggle_button_p =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
      ACE_ASSERT (toggle_button_p);
      is_active_b = gtk_toggle_button_get_active (toggle_button_p);

      // sanity check(s)
#if GTK_CHECK_VERSION(3,0,0)
      if ((eventKey_in->keyval == GDK_KEY_Escape) &&
#else
      if ((eventKey_in->keyval == GDK_Escape) &&
#endif // GTK_CHECK_VERSION(3,0,0)
          !is_active_b)
        break; // <-- not in fullscreen mode, nothing to do

      gtk_toggle_button_set_active (toggle_button_p,
                                    !is_active_b);

      break;
    }
    default:
      return FALSE; // propagate
  } // end SWITCH

  return TRUE; // done (do not propagate further)
}

gboolean
drawingarea_key_press_event_cb (GtkWidget* widget_in,
                                GdkEventKey* eventKey_in,
                                gpointer userData_in)
{
  return key_cb (widget_in, eventKey_in, userData_in);
}

gboolean
dialog_main_key_press_event_cb (GtkWidget* widget_in,
                                GdkEventKey* eventKey_in,
                                gpointer userData_in)
{
  return key_cb (widget_in, eventKey_in, userData_in);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
