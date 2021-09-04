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
#include "common_ui_tools.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_tools.h"

#include "test_u_imagescreen_defines.h"
#include "test_u_imagescreen_stream.h"

static bool untoggling_stream_b = false;

bool
load_display_adapters (GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_display_adapters"));

  // sanity check(s)
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  Common_UI_DisplayAdapters_t display_adapters_a =
      Common_UI_Tools::getAdapters();
  GtkTreeIter iterator;
  for (Common_UI_DisplayAdaptersIterator_t iterator_2 = display_adapters_a.begin ();
       iterator_2 != display_adapters_a.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, (*iterator_2).description.c_str (),
                        1, (*iterator_2).device.c_str (),
                        -1);
  } // end FOR

  return true;
}

bool
load_display_devices (GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_display_devices"));

  // sanity check(s)
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  Common_UI_DisplayDevices_t display_devices_a =
      Common_UI_Tools::getDisplays ();
  GtkTreeIter iterator;
  for (Common_UI_DisplayDevicesIterator_t iterator_2 = display_devices_a.begin ();
       iterator_2 != display_devices_a.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
//                        0, (*iterator_2).description.c_str (),
                        0, (*iterator_2).device.c_str (),
                        1, (*iterator_2).device.c_str (),
                        -1);
  } // end FOR

  return true;
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

  GtkWidget* about_dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (about_dialog_p);

  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_LISTSTORE_ADAPTER_NAME)));
  ACE_ASSERT (list_store_p);
  if (!load_display_adapters (list_store_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_display_adapters(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_COMBOBOX_ADAPTER_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkCellRenderer* cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  "text", 0,
                                  NULL);

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_LISTSTORE_DISPLAY_NAME)));
  ACE_ASSERT (list_store_p);
  if (!load_display_devices (list_store_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_display_devices(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_COMBOBOX_DISPLAY_NAME)));
  ACE_ASSERT (combo_box_p);
  //gtk_combo_box_set_model (combo_box_p,
  //                         GTK_TREE_MODEL (list_store_p));
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  "text", 0,
                                  NULL);

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
  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEBUTTON_FULLSCREEN_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                (*stream_configuration_iterator).second.second.fullScreen);

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_FILECHOOSERBUTTON_DIRECTORY_NAME)));
  ACE_ASSERT (file_chooser_button_p);
//  std::string default_folder_uri = ACE_TEXT_ALWAYS_CHAR ("file://");
//  default_folder_uri +=
//      (*stream_configuration_iterator).second.second.fileIdentifier.identifier;
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

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
                                   1.0 / static_cast<double> (width));
  //gtk_progress_bar_set_show_text (progress_bar_p,
  //                                TRUE);

  // step9: draw main dialog
  gtk_widget_show_all (dialog_p);

  // step10: retrieve window handle
//  (*stream_configuration_iterator).second.second.window =
//      gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
//  ACE_ASSERT ((*stream_configuration_iterator).second.second.window);
//  GtkAllocation allocation_s;
//  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
//                             &allocation_s);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  (*stream_configuration_iterator).second.second.outputFormat.resolution.cx =
#else
  (*stream_configuration_iterator).second.second.outputFormat.resolution.width =
#endif // ACE_WIN32 || ACE_WIN64
//    allocation_s.width;
      640;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  (*stream_configuration_iterator).second.second.outputFormat.resolution.cy =
#else
  (*stream_configuration_iterator).second.second.outputFormat.resolution.height =
#endif // ACE_WIN32 || ACE_WIN64
//      allocation_s.height;
      480;
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("initial window size: %ux%u\n"),
//              allocation_s.width, allocation_s.height));

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_COMBOBOX_DISPLAY_NAME)));
  ACE_ASSERT (combo_box_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value,
                      (*stream_configuration_iterator).second.second.display.device.c_str ());
  Common_UI_GTK_Tools::selectValue (combo_box_p,
                                    value,
                                    1);

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

  struct Stream_ImageScreen_ProgressData* data_p =
      static_cast<struct Stream_ImageScreen_ProgressData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->state);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->state->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->state->builders.end ());

  Common_UI_GTK_PendingActionsIterator_t iterator_2;
  bool done = false;
  std::ostringstream converter;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->state->lock, G_SOURCE_REMOVE);
    for (Common_UI_GTK_CompletedActionsIterator_t iterator_3 = data_p->completedActions.begin ();
         iterator_3 != data_p->completedActions.end ();
         ++iterator_3)
    {
      iterator_2 = data_p->pendingActions.find (*iterator_3);
      ACE_ASSERT (iterator_2 != data_p->pendingActions.end ());

      data_p->state->eventSourceIds.erase (*iterator_3);
      data_p->pendingActions.erase (iterator_2);
    } // end FOR
    data_p->completedActions.clear ();

    if (data_p->pendingActions.empty ())
    {
      data_p->eventSourceId = 0;
      done = true;
    } // end IF
  } // end lock scope

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
//  ACE_ASSERT (data_p->total);
  converter  << data_p->current;
  converter  << ACE_TEXT_ALWAYS_CHAR (" / ");
  converter  << data_p->total;
  gtk_progress_bar_set_text (progress_bar_p,
                             (done ? ACE_TEXT_ALWAYS_CHAR ("")
                                   : converter.str ().c_str ()));
  gtk_progress_bar_set_fraction (progress_bar_p,
                                 (gfloat)data_p->current / (gfloat)data_p->total);
//  gtk_progress_bar_pulse (progress_bar_p);

  // reschedule ?
  return (done ? G_SOURCE_REMOVE : G_SOURCE_CONTINUE);
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

  if (gtk_toggle_button_get_active (toggle_button_p))
  {
    untoggling_stream_b = true;
    gtk_toggle_button_set_active (toggle_button_p,
                                  FALSE);
  } // end IF
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p),
                            TRUE);

  ACE_ASSERT (ui_cb_data_p->progressData.eventSourceId);
  ui_cb_data_p->progressData.completedActions.insert (ui_cb_data_p->progressData.eventSourceId);

  ui_cb_data_p->progressData.current = 0;
  ui_cb_data_p->progressData.total = 0;

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
  ACE_ASSERT (ui_cb_data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());
  ACE_ASSERT (ui_cb_data_p->configuration);
  Stream_ImageScreen_StreamConfiguration_t::ITERATOR_T stream_configuration_iterator =
      ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

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
    gtk_widget_set_sensitive (GTK_WIDGET (toggleButton_in),
                              FALSE);

//    ACE_ASSERT (ui_cb_data_p->progressData.eventSourceId);
//    ui_cb_data_p->progressData.completedActions.insert (ui_cb_data_p->progressData.eventSourceId);

    // stop stream
    ui_cb_data_p->stream->stop (false,  // wait ?
                                true); // locked access ?

    return;
  } // end IF

  // update configuration
  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEBUTTON_FULLSCREEN_NAME)));
  ACE_ASSERT (toggle_button_p);
  (*stream_configuration_iterator).second.second.fullScreen =
      gtk_toggle_button_get_active (toggle_button_p);

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_FILECHOOSERBUTTON_DIRECTORY_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  (*stream_configuration_iterator).second.second.fileIdentifier.identifier =
    gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p));

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_COMBOBOX_DISPLAY_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter iterator_4;
  gboolean result = gtk_combo_box_get_active_iter (combo_box_p,
                                                   &iterator_4);
  ACE_ASSERT (result);
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_LISTSTORE_DISPLAY_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_4,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  (*stream_configuration_iterator).second.second.display =
      Common_UI_Tools::getDisplay (g_value_get_string (&value));

  if (!ui_cb_data_p->stream->initialize (ui_cb_data_p->configuration->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream, returning\n")));
    return;
  } // end IF

  if ((*stream_configuration_iterator).second.second.fullScreen)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    (*stream_configuration_iterator).second.second.outputFormat.resolution.cx =
		((*stream_configuration_iterator).second.second.display.clippingArea.right -
		 (*stream_configuration_iterator).second.second.display.clippingArea.left);
#else
    (*stream_configuration_iterator).second.second.outputFormat.resolution.width =
		(*stream_configuration_iterator).second.second.display.clippingArea.width;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    (*stream_configuration_iterator).second.second.outputFormat.resolution.cy =
		((*stream_configuration_iterator).second.second.display.clippingArea.bottom -
		 (*stream_configuration_iterator).second.second.display.clippingArea.top);
#else
    (*stream_configuration_iterator).second.second.outputFormat.resolution.height =
		(*stream_configuration_iterator).second.second.display.clippingArea.height;
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
  else
  {
//    GtkDrawingArea* drawing_area_p =
//      GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
//                                                ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DRAWINGAREA_NAME)));
//    ACE_ASSERT (drawing_area_p);

//    GtkAllocation allocation_s;
//    gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
//                               &allocation_s);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    (*stream_configuration_iterator).second.second.outputFormat.resolution.cx =
#else
    (*stream_configuration_iterator).second.second.outputFormat.resolution.width =
#endif // ACE_WIN32 || ACE_WIN64
//        allocation_s.width;
        640;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    (*stream_configuration_iterator).second.second.outputFormat.resolution.cy =
#else
    (*stream_configuration_iterator).second.second.outputFormat.resolution.height =
#endif // ACE_WIN32 || ACE_WIN64
//        allocation_s.height;
        480;
  } // end ELSE

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

void
combobox_display_changed_cb (GtkWidget* widget_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_display_changed_cb"));

  struct Stream_ImageScreen_UI_CBData* ui_cb_data_p =
    static_cast<struct Stream_ImageScreen_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  ACE_ASSERT (ui_cb_data_p->configuration);
  Stream_ImageScreen_StreamConfiguration_t::ITERATOR_T iterator_3 =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != ui_cb_data_p->configuration->streamConfiguration.end ());
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  GtkTreeIter iterator_4;
  gboolean result = gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                                   &iterator_4);
  ACE_ASSERT (result);
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_LISTSTORE_DISPLAY_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_4,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (false); // *TODO*
#else
  (*iterator_3).second.second.display.device = g_value_get_string (&value);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);

  // select corresponding adapter
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_COMBOBOX_ADAPTER_NAME)));
  ACE_ASSERT (combo_box_p);
  struct Common_UI_DisplayAdapter display_adapter_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (false); // *TODO*
#else
  display_adapter_s =
      Common_UI_Tools::getAdapter ((*iterator_3).second.second.display);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value,
                      display_adapter_s.device.c_str ());
  Common_UI_GTK_Tools::selectValue (combo_box_p,
                                    value,
                                    1);
} // combobox_display_changed_cb

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
                ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", aborting\n"),
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

//gboolean
//drawingarea_key_press_event_cb (GtkWidget* widget_in,
//                                GdkEventKey* eventKey_in,
//                                gpointer userData_in)
//{
//  return key_cb (widget_in, eventKey_in, userData_in);
//}

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
