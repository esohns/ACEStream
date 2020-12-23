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

//#include "ace/Synch.h"
#include "test_i_imagesave_gtk_callbacks.h"

#include <math.h>

#include <limits>
#include <map>
#include <set>
#include <sstream>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <strmif.h>
#include <reftime.h>
#include <dvdmedia.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <uuids.h>
#include <wmcodecdsp.h>

#include "gdk/gdkwin32.h"
#else
#include "linux/videodev2.h"
#include "libv4l2.h"

#include "gdk/gdkx.h"

#include "ace/Dirent_Selector.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "gdk/gdkkeysyms.h"

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch_Traits.h"

#include "common_timer_manager.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_tools.h"
#include "common_ui_ifullscreen.h"
#include "common_ui_tools.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_vis_common.h"
#include "stream_vis_defines.h"
#include "stream_vis_tools.h"

#include "test_i_imagesave_common.h"
#include "test_i_imagesave_defines.h"
#include "test_i_imagesave_stream.h"

// global variables
bool un_toggling_stream = false;

//////////////////////////////////////////

ACE_THR_FUNC_RETURN
stream_processing_function (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_processing_function"));

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("processing thread (id: %t) starting\n")));
#endif // _DEBUG

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = std::numeric_limits<unsigned long>::max ();
#else
  result = arg_in;
#endif // ACE_WIN32 || ACE_WIN64

  struct Test_I_ImageSave_UI_ThreadData* thread_data_p =
      static_cast<struct Test_I_ImageSave_UI_ThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (thread_data_p);
  ACE_ASSERT (thread_data_p->CBData);

  Common_UI_GTK_BuildersIterator_t iterator;
    //  GtkProgressBar* progress_bar_p = NULL;
  GtkStatusbar* statusbar_p = NULL;
  std::ostringstream converter;
  Stream_IStreamControlBase* stream_p = NULL;

  struct Test_I_ImageSave_UI_CBData* cb_data_p = NULL;
  const Test_I_ImageSave_SessionData_t* session_data_container_p =
    NULL;
  const Test_I_ImageSave_SessionData* session_data_p = NULL;
  cb_data_p =
    static_cast<struct Test_I_ImageSave_UI_CBData*> (thread_data_p->CBData);
  ACE_ASSERT (cb_data_p->configuration);
  ACE_ASSERT (cb_data_p->stream);

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

  converter.clear ();
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  Test_I_StreamConfiguration_t::ITERATOR_T iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());

  // *NOTE*: let the display output module handle the Direct3D device
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if ((*iterator_2).second.second.direct3DConfiguration->handle)
  {
    (*iterator_2).second.second.direct3DConfiguration->handle->Release (); (*iterator_2).second.second.direct3DConfiguration->handle = NULL;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  ACE_ASSERT (cb_data_p->stream);
  if (!cb_data_p->stream->initialize (cb_data_p->configuration->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream, aborting\n")));
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

  // *NOTE*: blocks until 'finished'
  ACE_ASSERT (stream_p);
  stream_p->start ();
  //ACE_ASSERT (!stream_p->isRunning ());
  stream_p->wait (true,
                  false,
                  false);

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

  struct Test_I_ImageSave_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_MAIN_NAME)));
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

  //spin_button_p =
  //  GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  //ACE_ASSERT (spin_button_p);
  //gtk_spin_button_set_range (spin_button_p,
  //                           0.0,
  //                           std::numeric_limits<double>::max ());

  GtkListStore* list_store_p = NULL;
  GtkComboBox* combo_box_p = NULL;
  //gtk_combo_box_set_model (combo_box_p,
  //                         GTK_TREE_MODEL (list_store_p));

  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_SOURCE_NAME)));
  ACE_ASSERT (entry_p);
  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SOURCE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  //GtkFileChooserDialog* file_chooser_dialog_p =
  //  GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME)));
  //ACE_ASSERT (file_chooser_dialog_p);
  GtkFileFilter* file_filter_p =
    GTK_FILE_FILTER (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILEFILTER_MP4_NAME)));
  ACE_ASSERT (file_filter_p);
//  gtk_file_filter_add_mime_type (file_filter_p,
//                                 ACE_TEXT ("application/x-troff-msvideo"));
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("video/mp4"));
//  gtk_file_filter_add_mime_type (file_filter_p,
//                                 ACE_TEXT ("video/msvideo"));
//  gtk_file_filter_add_mime_type (file_filter_p,
//                                 ACE_TEXT ("video/x-msvideo"));
  gtk_file_filter_add_pattern (file_filter_p,
                               ACE_TEXT ("*.mp4"));
  gtk_file_filter_set_name (file_filter_p,
                            ACE_TEXT ("MP4 files"));
  //GError* error_p = NULL;
  //GFile* file_p = NULL;
  //gchar* filename_p = NULL;
  Common_Image_Resolution_t resolution_s;
  unsigned int framerate_i = 0;
  std::string filename_string;
  //bool is_display_b = false, is_fullscreen_b = false;
  unsigned int buffer_size_i = 0;
  Test_I_StreamConfiguration_t::ITERATOR_T stream_iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_StreamConfiguration_t::ITERATOR_T stream_iterator_2;
#endif // ACE_WIN32 || ACE_WIN64
  cb_data_p =
    static_cast<struct Test_I_ImageSave_UI_CBData*> (cb_data_p);
  ACE_ASSERT (cb_data_p->configuration);
  stream_iterator =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  stream_iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING));
  ACE_ASSERT (stream_iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  //format_s =
  //  cb_data_p->configuration->streamConfiguration.configuration->format.format;
  resolution_s = cb_data_p->configuration->streamConfiguration.configuration->format.resolution;
  filename_string = (*stream_iterator).second.second.targetFileName;
  gtk_entry_set_text (entry_p,
                      (filename_string.empty () ? ACE_TEXT_ALWAYS_CHAR ("")
                                                : ACE_TEXT_ALWAYS_CHAR (ACE::basename (filename_string.c_str (), ACE_DIRECTORY_SEPARATOR_CHAR))));
  std::string file_uri =
    ACE_TEXT_ALWAYS_CHAR ("file://") +
    (filename_string.empty () ? Common_File_Tools::getTempDirectory () : filename_string);
  if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
                                                file_uri.c_str ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_uri (\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (file_uri.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF

  file_chooser_button_p =
      GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_TARGET_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
                                                file_uri.c_str ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_uri (\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (file_uri.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  //gtk_combo_box_set_model (combo_box_p,
  //                         GTK_TREE_MODEL (list_store_p));
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
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_ASCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
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

  //GtkToggleButton* toggle_button_p =
  //  GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PROCESS_NAME)));
  //ACE_ASSERT (toggle_button_p);
  //gtk_toggle_button_set_active (toggle_button_p,
  //                              !filename_string.empty ());

  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->configuration);
  buffer_size_i =
    cb_data_p->configuration->streamConfiguration.configuration->allocatorConfiguration->defaultBufferSize;

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

  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);

  // step5: initialize updates
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, cb_data_p->UIState->lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the log views
    guint event_source_id =
//        g_timeout_add_seconds (1,
//                               idle_update_log_display_cb,
//                               userData_in);
//    if (event_source_id > 0)
//      cb_data_p->UIState->eventSourceIds.insert (event_source_id);
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
      cb_data_p->UIState->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step6: disable some functions ?

  // step2: (auto-)connect signals/slots
  gtk_builder_connect_signals ((*iterator).second.second,
                               cb_data_p);

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

  //result_2 =
  //  g_signal_connect (file_chooser_button_p,
  //                    ACE_TEXT_ALWAYS_CHAR ("file-set"),
  //                    G_CALLBACK (filechooserbutton_cb),
  //                    userData_in);
  //ACE_ASSERT (result_2);
  //result_2 =
  //  g_signal_connect (file_chooser_dialog_p,
  //                    ACE_TEXT_ALWAYS_CHAR ("file-activated"),
  //                    G_CALLBACK (filechooserdialog_cb),
  //                    NULL);
  //ACE_ASSERT (result_2);

  // set defaults
  //file_chooser_button_p =
  //  GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
//  std::string default_folder_uri = ACE_TEXT_ALWAYS_CHAR ("file://");
//  default_folder_uri += filename_string;
  gboolean result =
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p),
                                         ACE_TEXT_ALWAYS_CHAR (ACE::dirname (filename_string.c_str ())));
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (ACE::dirname (filename_string.c_str ()))));
    return G_SOURCE_REMOVE;
  } // end IF

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  gtk_widget_show_all (dialog_p);

  // step10: retrieve canvas coordinates, window handle and pixel buffer
  GtkAllocation allocation;
  ACE_OS::memset (&allocation, 0, sizeof (GtkAllocation));
  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
                             &allocation);
  GdkWindow* window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT (window_p);
  //ACE_ASSERT (!(*stream_iterator).second.second.window);
  //ACE_ASSERT (!cb_data_p->configuration->direct3DConfiguration.presentationParameters.hDeviceWindow);
  //ACE_ASSERT (!cb_data_p->configuration->direct3DConfiguration.focusWindow);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (gdk_win32_window_is_win32 (window_p));
  //(*stream_iterator).second.second.window =
  //  gdk_win32_window_get_impl_hwnd (window_p);
  cb_data_p->configuration->direct3DConfiguration.focusWindow =
    NULL;
  cb_data_p->configuration->direct3DConfiguration.presentationParameters.hDeviceWindow =
    gdk_win32_window_get_impl_hwnd (window_p);
#endif // ACE_WIN32 || ACE_WIN64

  //(*stream_iterator).second.second.area.bottom =
  //  allocation.y + allocation.height;
  //(*stream_iterator).second.second.area.left = allocation.x;
  //(*stream_iterator).second.second.area.right =
  //  allocation.x + allocation.width;
  //(*stream_iterator).second.second.area.top = allocation.y;

  //(*stream_iterator).second.second.pixelBuffer =
  //  cb_data_p->pixelBuffer;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (IsWindow (cb_data_p->configuration->direct3DConfiguration.presentationParameters.hDeviceWindow));
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("drawing area window handle: 0x%@; size: %dx%d\n"),
              cb_data_p->configuration->direct3DConfiguration.presentationParameters.hDeviceWindow,
              allocation.width, allocation.height));
#endif // _DEBUG
#endif // ACE_WIN32 || ACE_WIN64

#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  g_value_init (&value, G_TYPE_STRING);
  guint index_i =0;

  // step11: select default capture source (if any)
  //         --> populate the options comboboxes
  // select default capture format
  std::ostringstream converter;
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  g_value_unset (&value);
  g_value_init (&value, G_TYPE_STRING);
  //g_value_set_string (&value,
  //                    Common_Tools::GUIDToString (format_s).c_str ());
  index_i =
      Common_UI_GTK_Tools::valueToIndex (GTK_TREE_MODEL (list_store_p),
                                         value,
                                         1);
  //ACE_ASSERT (index_i != std::numeric_limits<unsigned int>::max ());
  gtk_combo_box_set_active (combo_box_p, static_cast<gint> (index_i));

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
  ACE_ASSERT (combo_box_p);
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  g_value_unset (&value);
  g_value_init (&value, G_TYPE_STRING);
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  converter << resolution_s.cx;
#else
  converter << resolution_s.width;
#endif // ACE_WIN32 || ACE_WIN64
  converter << ACE_TEXT_ALWAYS_CHAR (" x ");
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  converter << resolution_s.cy;
#else
  converter << resolution_s.height;
#endif // ACE_WIN32 || ACE_WIN64
  g_value_set_string (&value,
                      converter.str ().c_str ());
  index_i = Common_UI_GTK_Tools::valueToIndex (GTK_TREE_MODEL (list_store_p),
                                               value,
                                               0);
//  ACE_ASSERT (index_i != std::numeric_limits<unsigned int>::max ());
  gtk_combo_box_set_active (combo_box_p,
                            static_cast<gint> (index_i));

  //combo_box_p =
  //  GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
  //                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME)));
  //ACE_ASSERT (combo_box_p);
  //g_value_unset (&value);
  //g_value_init (&value, G_TYPE_STRING);
  //g_value_set_string (&value,
  //                    (*stream_iterator).second.second.display.device.c_str ());
  //Common_UI_GTK_Tools::selectValue (combo_box_p,
  //                                  value,
  //                                  1);

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

//  struct Test_I_ImageSave_UI_CBData* cb_data_p =
//    static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);

//  // sanity check(s)
//  ACE_ASSERT (cb_data_p);

  // leave GTK
  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

gboolean
idle_session_end_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_session_end_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);

  struct Test_I_ImageSave_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_p->UIState->lock, G_SOURCE_REMOVE);

  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  // *IMPORTANT NOTE*: there are two major reasons for being here that are not
  //                   mutually exclusive, so there could be a race:
  //                   - user pressed stop
  //                   - there was an asynchronous error on the stream
  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PROCESS_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_button_set_label (GTK_BUTTON (toggle_button_p),
                        GTK_STOCK_EXECUTE);
  if (gtk_toggle_button_get_active (toggle_button_p))
  {
    un_toggling_stream = true;
    gtk_toggle_button_set_active (toggle_button_p,
                                  FALSE);
  } // end IF

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_SOURCE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);
  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_TARGET_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);
  //frame_p =
  //  GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
  //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_DISPLAY_NAME)));
  //ACE_ASSERT (frame_p);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);

  //// stop progress reporting
  //ACE_ASSERT (ui_cb_data_p->progressData.eventSourceId);
  //{
  //  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard_2, ui_cb_data_p->UIState->lock, G_SOURCE_REMOVE);

  //  if (!g_source_remove (ui_cb_data_p->progressData.eventSourceId))
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
  //                ui_cb_data_p->progressData.eventSourceId));
  //  ui_cb_data_p->eventSourceIds.erase (ui_cb_data_p->progressData.eventSourceId);
  //  ui_cb_data_p->progressData.eventSourceId = 0;

  //  ACE_OS::memset (&(ui_cb_data_p->progressData.statistic),
  //                  0,
  //                  sizeof (ui_cb_data_p->progressData.statistic));
  //} // end lock scope
  GtkProgressBar* progressbar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progressbar_p);
  // *NOTE*: this disables "activity mode" (in Gtk2)
  gtk_progress_bar_set_fraction (progressbar_p, 0.0);
  gtk_progress_bar_set_text (progressbar_p, ACE_TEXT_ALWAYS_CHAR (""));
  gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), false);

  return G_SOURCE_REMOVE;
}

//gboolean
//idle_update_log_display_cb (gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));

//  struct Test_I_ImageSave_UI_CBData* ui_cb_data_p =
//    static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);

//  // sanity check(s)
//  ACE_ASSERT (ui_cb_data_p);
//  Common_UI_GTK_BuildersIterator_t iterator =
//    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

//  GtkTextView* view_p =
//      GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
//                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTVIEW_NAME)));
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
////                                ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCROLLMARK_NAME));
//////  gtk_text_buffer_move_mark (buffer_p,
//////                             text_mark_p,
//////                             &text_iterator);

////  // scroll the mark onscreen
////  gtk_text_view_scroll_mark_onscreen (view_p,
////                                      text_mark_p);
//  GtkAdjustment* adjustment_p =
//      GTK_ADJUSTMENT (gtk_builder_get_object ((*iterator).second.second,
//                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ADJUSTMENT_NAME)));
//  ACE_ASSERT (adjustment_p);
//  gtk_adjustment_set_value (adjustment_p,
//                            gtk_adjustment_get_upper (adjustment_p));

//  return G_SOURCE_CONTINUE;
//}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct Test_I_ImageSave_UI_CBData* cb_data_p =
      static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Common_UI_EventType* event_p = NULL;
  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, cb_data_p->UIState->lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (cb_data_p->UIState->eventStack);
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
                                     static_cast<gdouble> (cb_data_p->progressData.statistic.bytes));

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
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//          spin_button_p =
//            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_CAPTUREDFRAMES_NAME)));
//          ACE_ASSERT (spin_button_p);
//          gtk_spin_button_set_value (spin_button_p,
//                                     static_cast<gdouble> (cb_data_p->progressData.statistic.capturedFrames));
//#endif
//
//          spin_button_p =
//            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DROPPEDFRAMES_NAME)));
//          ACE_ASSERT (spin_button_p);
//          gtk_spin_button_set_value (spin_button_p,
//                                     static_cast<gdouble> (cb_data_p->progressData.statistic.droppedFrames));

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
    while (!cb_data_p->UIState->eventStack.is_empty ())
    {
      result = cb_data_p->UIState->eventStack.pop (event_e);
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

  struct Test_I_ImageSave_ProgressData* data_p =
      static_cast<struct Test_I_ImageSave_ProgressData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->state);

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->state->lock, G_SOURCE_REMOVE);

  int result = -1;
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->state->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->state->builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_THR_FUNC_RETURN exit_status;
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  Common_UI_GTK_PendingActionsIterator_t iterator_2;
  for (Common_UI_GTK_CompletedActionsIterator_t iterator_3 = data_p->completedActions.begin ();
       iterator_3 != data_p->completedActions.end ();
       ++iterator_3)
  {
    iterator_2 = data_p->pendingActions.find (*iterator_3);
    ACE_ASSERT (iterator_2 != data_p->pendingActions.end ());
    ACE_thread_t thread_id = (*iterator_2).second.id ();
    result = thread_manager_p->join (thread_id, &exit_status);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::join(%d): \"%m\", continuing\n"),
                  thread_id));
    else
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("thread %u has joined (status was: %u)\n"),
                  thread_id,
                  exit_status));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("thread %u has joined (status was: 0x%@)\n"),
                  thread_id,
                  exit_status));
#endif
    } // end ELSE

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

    done = true;
  } // end IF

  // synch access
  std::ostringstream converter;
  converter << data_p->statistic.messagesPerSecond;
  converter << ACE_TEXT_ALWAYS_CHAR (" fps");
  gtk_progress_bar_set_text (progress_bar_p,
                             (done ? ACE_TEXT_ALWAYS_CHAR ("")
                                   : converter.str ().c_str ()));
  gtk_progress_bar_pulse (progress_bar_p);

  // reschedule ?
  return (done ? G_SOURCE_REMOVE : G_SOURCE_CONTINUE);
}

//gboolean
//idle_update_video_display_cb (gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::idle_update_video_display_cb"));

//  // sanity check(s)
//  ACE_ASSERT (userData_in);

//  struct Test_I_ImageSave_UI_CBData* ui_cb_data_p =
//    static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);

//  Common_UI_GTK_BuildersIterator_t iterator =
//    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  // sanity check(s)
//  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

//  GtkDrawingArea* drawing_area_p =
//    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
//                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
//  ACE_ASSERT (drawing_area_p);

//  gdk_window_invalidate_rect (gtk_widget_get_window (GTK_WIDGET (drawing_area_p)),
//                              NULL,
//                              false);

//  return G_SOURCE_REMOVE;
//}

//////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void
togglebutton_process_toggled_cb (GtkToggleButton* toggleButton_in,
                                 gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::toggleaction_process_toggled_cb"));

  // handle untoggle --> PLAY
  if (un_toggling_stream)
  {
    un_toggling_stream = false;
    return; // done
  } // end IF

  bool is_active_b = gtk_toggle_button_get_active (toggleButton_in);

  // --> user pressed process/stop

  struct Test_I_ImageSave_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (cb_data_p);

  Stream_IStreamControlBase* stream_p = NULL;
  Test_I_StreamConfiguration_t::ITERATOR_T stream_iterator;
  stream_p = cb_data_p->stream;
  ACE_ASSERT (cb_data_p->configuration);
  stream_iterator =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());
  ACE_ASSERT (stream_p);
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  // toggle ?
  if (!is_active_b)
  {
    // --> user pressed pause/stop

    // stop stream
    stream_p->stop (false, // wait ?
                    true,  // recurse upstream ?
                    true); // locked access ?

    return;
  } // end IF

//  GtkButton* button_p = NULL;
  GtkFrame* frame_p = NULL;

  // --> user pressed process

  struct Test_I_ImageSave_UI_ThreadData* thread_data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_thread_t thread_id = std::numeric_limits<unsigned long>::max ();
#else
  ACE_thread_t thread_id = -1;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;

//  GtkSpinButton* spin_button_p = NULL;
//  unsigned int buffer_size_i = 0;
//  gdouble value_d = 0.0;

  // step0: modify widgets
  gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                        GTK_STOCK_MEDIA_STOP);

  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
  //                          false);

  // step1: set up progress reporting
  ACE_OS::memset (&cb_data_p->progressData.statistic,
                  0,
                  sizeof (struct Test_I_StatisticData));
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  //gint width, height;
  //gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  //gtk_progress_bar_set_pulse_step (progress_bar_p,
  //                                 1.0 / static_cast<double> (width));
  gtk_progress_bar_set_fraction (progress_bar_p, 0.0);
  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), TRUE);

  // step2: update configuration
  // *NOTE*: the source device configuration is kept up-to-date automatically
  GtkFileChooserButton* file_chooser_button_p = NULL;
//  GError* error_p = NULL;
  std::string filename_string;
  GtkEntry* entry_p = NULL;
  file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_TARGET_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  ACE_ASSERT (gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p)));
  filename_string =
    gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p));
  ACE_ASSERT (Common_File_Tools::isDirectory (filename_string));
  ACE_ASSERT (Common_File_Tools::isWriteable (filename_string));
  entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_TARGET_NAME)));
  ACE_ASSERT (entry_p);
  filename_string += ACE_DIRECTORY_SEPARATOR_STR;
  filename_string += ACE_TEXT_ALWAYS_CHAR (gtk_entry_get_text (entry_p));
  ACE_ASSERT (Common_File_Tools::isValidPath (filename_string));
  (*stream_iterator).second.second.targetFileName =
    filename_string;

//  spin_button_p =
//    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
//  ACE_ASSERT (spin_button_p);
//  value_d = gtk_spin_button_get_value (spin_button_p);
//  ACE_ASSERT (value_d);

  // sanity check(s)

  // step3: start processing thread
  ACE_NEW_NORETURN (thread_data_p,
                    struct Test_I_ImageSave_UI_ThreadData ());
  if (!thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  thread_data_p->CBData = cb_data_p;
  ACE_TCHAR thread_name[BUFSIZ];
  ACE_OS::memset (thread_name, 0, sizeof (ACE_TCHAR[BUFSIZ]));
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
//                  ACE_TEXT (TEST_I_CamSave_THREAD_NAME));
//  const char* thread_name_2 = thread_name_p;
  ACE_OS::strcpy (thread_name,
                  ACE_TEXT (TEST_I_THREAD_NAME));
  thread_name_2 = thread_name;
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);

  // *NOTE*: lock access to the progress report structures to avoid a race
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, cb_data_p->UIState->lock);
    int result =
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

    // step3: start progress reporting
    //ACE_ASSERT (!data_p->progressData.eventSourceId);
    cb_data_p->progressData.eventSourceId =
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_progress_cb,
      //                 &data_p->progressData,
      //                 NULL);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                          COMMON_UI_REFRESH_DEFAULT_PROGRESS, // ms (?)
                          idle_update_progress_cb,
                          &cb_data_p->progressData,
                          NULL);
    if (!cb_data_p->progressData.eventSourceId)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", returning\n")));
      ACE_THR_FUNC_RETURN exit_status;
      result = thread_manager_p->join (thread_id, &exit_status);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Thread_Manager::join(%d): \"%m\", continuing\n"),
                    thread_id));
      goto error;
    } // end IF
    thread_data_p->eventSourceId = cb_data_p->progressData.eventSourceId;
    cb_data_p->progressData.pendingActions[cb_data_p->progressData.eventSourceId] =
      ACE_Thread_ID (thread_id, thread_handle);
    //    ACE_DEBUG ((LM_DEBUG,
    //                ACE_TEXT ("idle_update_progress_cb: %d\n"),
    //                event_source_id));
    cb_data_p->UIState->eventSourceIds.insert (cb_data_p->progressData.eventSourceId);
  } // end lock scope

  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_SOURCE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);
  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_TARGET_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);
  //frame_p =
  //  GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
  //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_DISPLAY_NAME)));
  //ACE_ASSERT (frame_p);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);

  return;

error:
  gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                        GTK_STOCK_MEDIA_RECORD);
  //gtk_action_set_sensitive (action_p, false);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
  //                          true);
} // toggleaction_process_toggled_cb

//void
//togglebutton_save_toggled_cb (GtkToggleButton* toggleButton_in,
//                              gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::togglebutton_save_toggled_cb"));
//
//  struct Test_I_ImageSave_UI_CBData* cb_data_p =
//    static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);
//
//  // sanity check(s)
//  ACE_ASSERT (cb_data_p);
//
//  Common_UI_GTK_BuildersIterator_t iterator =
//    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());
//
//  Test_I_StreamConfiguration_t::ITERATOR_T stream_iterator;
//  cb_data_p =
//    static_cast<struct Test_I_ImageSave_UI_CBData*> (cb_data_p);
//  ACE_ASSERT (cb_data_p->configuration);
//  stream_iterator =
//    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());
//
//  GtkFileChooserButton* file_chooser_button_p =
//    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SOURCE_NAME)));
//  ACE_ASSERT (file_chooser_button_p);
//  GError* error_p = NULL;
//  GFile* file_p = NULL;
//  std::string filename_string;
//  if (gtk_toggle_button_get_active (toggleButton_in))
//  {
//    file_p =
//      gtk_file_chooser_get_current_folder_file (GTK_FILE_CHOOSER (file_chooser_button_p));
//    ACE_ASSERT (file_p);
//    char* filename_p = g_file_get_path (file_p);
//    ACE_ASSERT (filename_p);
//    filename_string = filename_p;
//    g_free (filename_p); filename_p = NULL;
//    g_object_unref (G_OBJECT (file_p)); file_p = NULL;
//  } // end IF
//  else
//  {
//    file_p =
//      g_file_new_for_path (Common_File_Tools::getTempDirectory ().c_str ());
//    ACE_ASSERT (file_p);
//    if (!gtk_file_chooser_set_current_folder_file (GTK_FILE_CHOOSER (file_chooser_button_p),
//                                                   file_p,
//                                                   &error_p))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_file(\"%s\"): \"%s\", aborting\n"),
//                  ACE_TEXT (file_p),
//                  ACE_TEXT (error_p->message)));
//      g_error_free (error_p); error_p = NULL;
//      g_object_unref (G_OBJECT (file_p)); file_p = NULL;
//      return;
//    } // end IF
//    g_object_unref (G_OBJECT (file_p)); file_p = NULL;
//  } // end ELSE
//
//  (*stream_iterator).second.second.fileIdentifier.identifier =
//    filename_string;
//} // toggleaction_save_toggled_cb

gint
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  struct Test_I_ImageSave_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (cb_data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  // retrieve about dialog handle
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!dialog_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to glade_xml_get_widget(\"%s\"): \"%m\", aborting\n"),
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
  gtk_widget_hide (GTK_WIDGET (dialog_p));

  return FALSE;
} // button_about_clicked_cb

gint
button_quit_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  struct Test_I_ImageSave_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (cb_data_p);

  enum Stream_StateMachine_ControlState status_e = STREAM_STATE_INVALID;
  Stream_IStreamControlBase* stream_p = NULL;
  status_e = cb_data_p->stream->status ();
  stream_p = cb_data_p->stream;
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

  // step2: initiate shutdown sequence
  int result = ACE_OS::raise (SIGINT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                SIGINT));

  return FALSE;
} // button_quit_clicked_cb

void
combobox_format_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_format_changed_cb"));

  struct Test_I_ImageSave_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (cb_data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  Test_I_StreamConfiguration_t::ITERATOR_T stream_iterator;
  ACE_ASSERT (cb_data_p->configuration);
  stream_iterator =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter iterator_3;
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_3))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p = NULL;
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)

  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_3))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  value = G_VALUE_INIT;
#else
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string format_string = g_value_get_string (&value);
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = Common_Tools::StringToGUID (format_string);
  ACE_ASSERT (!InlineIsEqualGUID (GUID_s, GUID_NULL));
#else
  __u32 format_i = 0;
  std::istringstream converter;
  converter.str (format_string);
  converter >> format_i;
#endif // ACE_WIN32 || ACE_WIN64
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
} // combobox_format_changed_cb

void
combobox_resolution_changed_cb (GtkWidget* widget_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_resolution_changed_cb"));

  struct Test_I_ImageSave_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (cb_data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  Test_I_StreamConfiguration_t::ITERATOR_T stream_iterator;
  ACE_ASSERT (cb_data_p->configuration);
  stream_iterator =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter iterator_3;
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_3))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p = NULL;
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_3))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s =
    Common_Tools::StringToGUID (g_value_get_string (&value));
  ACE_ASSERT (!InlineIsEqualGUID (GUID_s, GUID_NULL));
#else
  __u32 format_i = 0;
  std::istringstream converter;
  converter.str (g_value_get_string (&value));
  converter >> format_i;
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);

  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_3))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value_2;
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
  g_value_init (&value_2, G_TYPE_UINT);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            2, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_UINT);
  unsigned int width = g_value_get_uint (&value);
  g_value_unset (&value);
  unsigned int height = g_value_get_uint (&value_2);
  g_value_unset (&value_2);
  Common_Image_Resolution_t resolution_s;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  resolution_s.cx = width;
  resolution_s.cy = height;
#else
  resolution_s.width = width;
  resolution_s.height = height;
#endif // ACE_WIN32 || ACE_WIN64
} // combobox_resolution_changed_cb

void
filechooserbutton_source_cb (GtkFileChooserButton* fileChooserButton_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_source_cb"));

  // sanity check(s)
  ACE_ASSERT (fileChooserButton_in);
  ACE_ASSERT (userData_in);

  struct Test_I_ImageSave_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (cb_data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  Test_I_StreamConfiguration_t::ITERATOR_T stream_iterator;
  ACE_ASSERT (cb_data_p->configuration);
  stream_iterator =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());

  GFile* file_p =
    gtk_file_chooser_get_file (GTK_FILE_CHOOSER (fileChooserButton_in));
  if (!file_p)
    return; // nothing selected (yet)
  char* string_p = g_file_get_path (file_p);
  if (!string_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_file_get_path(%@): \"%m\", returning\n"),
                file_p));
    g_object_unref (G_OBJECT (file_p)); file_p = NULL;
    return;
  } // end IF
  g_object_unref (G_OBJECT (file_p)); file_p = NULL;
  std::string filename_string =
    Common_UI_GTK_Tools::UTF8ToLocale (string_p, -1);
  if (filename_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF8ToLocale(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (string_p)));
    g_free (string_p); string_p = NULL;
    return;
  } // end IF
  g_free (string_p); string_p = NULL;

  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_SOURCE_NAME)));
  ACE_ASSERT (entry_p);
  const gchar* string_2 = gtk_entry_get_text (entry_p);
  filename_string += ACE_DIRECTORY_SEPARATOR_STR;
  filename_string += Common_UI_GTK_Tools::UTF8ToLocale (string_2, -1);

  (*stream_iterator).second.second.fileIdentifier.identifier =
    filename_string;
} // filechooserbutton_source_cb

void
filechooserbutton_target_cb (GtkFileChooserButton* fileChooserButton_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_target_cb"));

  // sanity check(s)
  ACE_ASSERT (fileChooserButton_in);
  ACE_ASSERT (userData_in);

  struct Test_I_ImageSave_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (cb_data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  Test_I_StreamConfiguration_t::ITERATOR_T stream_iterator;
  ACE_ASSERT (cb_data_p->configuration);
  stream_iterator =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());

  GFile* file_p =
    gtk_file_chooser_get_file (GTK_FILE_CHOOSER (fileChooserButton_in));
  if (!file_p)
    return; // nothing selected (yet)
  char* string_p = g_file_get_path (file_p);
  if (!string_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_file_get_path(%@): \"%m\", returning\n"),
                file_p));
    g_object_unref (G_OBJECT (file_p)); file_p = NULL;
    return;
  } // end IF
  g_object_unref (G_OBJECT (file_p)); file_p = NULL;
  std::string filename_string =
    Common_UI_GTK_Tools::UTF8ToLocale (string_p, -1);
  if (filename_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF8ToLocale(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (string_p)));
    g_free (string_p); string_p = NULL;
    return;
  } // end IF
  g_free (string_p); string_p = NULL;

  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_TARGET_NAME)));
  ACE_ASSERT (entry_p);
  const gchar* string_2 = gtk_entry_get_text (entry_p);
  filename_string += ACE_DIRECTORY_SEPARATOR_STR;
  filename_string += Common_UI_GTK_Tools::UTF8ToLocale (string_2, -1);

  (*stream_iterator).second.second.targetFileName =
    filename_string;
} // filechooserbutton_target_cb

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

void
scalebutton_frame_value_changed_cb (GtkScaleButton* button_in,
                                    gdouble value_in,
                                    gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::scalebutton_frame_value_changed_cb"));

  ACE_UNUSED_ARG (userData_in);

} // scalebutton_frame_value_changed_cb

gboolean
key_cb (GtkWidget* widget_in,
        GdkEventKey* eventKey_in,
        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::key_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  ACE_ASSERT (eventKey_in);

  struct Test_I_ImageSave_UI_CBData* cb_data_p =
      reinterpret_cast<struct Test_I_ImageSave_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (cb_data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

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
      //GtkToggleButton* toggle_button_p =
      //  GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
      //                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_FULLSCREEN_NAME)));
      //ACE_ASSERT (toggle_button_p);
      //is_active_b = gtk_toggle_button_get_active (toggle_button_p);

      // sanity check(s)
#if GTK_CHECK_VERSION(3,0,0)
      if ((eventKey_in->keyval == GDK_KEY_Escape) &&
#else
      if ((eventKey_in->keyval == GDK_Escape) &&
#endif // GTK_CHECK_VERSION(3,0,0)
          !is_active_b)
        break; // <-- not in fullscreen mode, nothing to do

      //gtk_toggle_button_set_active (toggle_button_p,
      //                              !is_active_b);

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
  STREAM_TRACE (ACE_TEXT ("::drawingarea_key_press_event_cb"));

  return key_cb (widget_in, eventKey_in, userData_in);
};
gboolean
dialog_main_key_press_event_cb (GtkWidget* widget_in,
                                GdkEventKey* eventKey_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::dialog_main_key_press_event_cb"));

  return key_cb (widget_in, eventKey_in, userData_in);
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
