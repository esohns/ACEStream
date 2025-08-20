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

#include "test_i_gtk_callbacks.h"

#ifdef __cplusplus
extern "C"
{
#include "libavformat/avformat.h"
}
#endif /* __cplusplus */

#include "gdk/gdkkeysyms.h"

#include <limits>
#include <map>
#include <set>
#include <sstream>

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"

#include "common_timer_manager.h"

#include "common_ui_ifullscreen.h"
#include "common_ui_tools.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_tools.h"

#include "stream_macros.h"

#include "stream_lib_tools.h"

#include "test_i_stream.h"

#include "test_i_extract_stream_common.h"
#include "test_i_extract_stream_defines.h"

// global variables
bool un_toggling_stream = false;

bool
load_display_adapters (GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_display_adapters"));

  // sanity check(s)
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  Common_UI_DisplayAdapters_t display_adapters_a =
      Common_UI_Tools::getAdapters ();
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
                        0, (*iterator_2).description.c_str (),
//                        0, (*iterator_2).device.c_str (),
                        1, (*iterator_2).device.c_str (),
                        -1);
  } // end FOR

  return true;
}

void
load_media_streams (const std::string& filename_in,
                    GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_media_streams"));

  // sanity check(s)
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  struct AVFormatContext* context_p = avformat_alloc_context ();
  if (unlikely (!context_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to avformat_alloc_context(): \"%m\", returning\n")));
    return;
  } // end IF
  int result =
    avformat_open_input (&context_p,
                         filename_in.c_str (),
                         NULL,
                         NULL);
  if (unlikely (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to avformat_open_input(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (filename_in.c_str ())));
    avformat_free_context (context_p);
    return;
  } // end IF
  result = avformat_find_stream_info (context_p,
                                      NULL);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to avformat_find_stream_info(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (filename_in.c_str ())));
    avformat_free_context (context_p);
    return;
  } // end IF
  for (unsigned int i = 0;
       i < context_p->nb_streams;
       ++i)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, avcodec_get_name (context_p->streams[i]->codecpar->codec_id),
                        1, i,
                        2, context_p->streams[i]->codecpar->codec_id,
                        -1);
  } // end FOR
  avformat_free_context (context_p); context_p = NULL;
}

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

  struct Test_I_ExtractStream_UI_ThreadData* thread_data_p =
      static_cast<struct Test_I_ExtractStream_UI_ThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (thread_data_p);
  ACE_ASSERT (thread_data_p->CBData);

  Common_UI_GTK_BuildersIterator_t iterator;
  GtkProgressBar* progress_bar_p = NULL;
  GtkStatusbar* statusbar_p = NULL;
  std::ostringstream converter;
  Stream_IStreamControlBase* stream_p = NULL;
//  Stream_Module_t* module_p = NULL;
//  bool result_2 = false;

  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (thread_data_p->CBData);
  ACE_ASSERT (cb_data_p->configuration);
  ACE_ASSERT (cb_data_p->stream);

  const Test_I_ExtractStream_SessionData_t* session_data_container_p = NULL;
  const Test_I_ExtractStream_SessionData* session_data_p = NULL;

  iterator =
    thread_data_p->CBData->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != thread_data_p->CBData->UIState->builders.end ());

  progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  statusbar_p =
    GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_STATUSBAR_NAME)));
  ACE_ASSERT (statusbar_p);

  converter.clear ();
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  if (!cb_data_p->stream->initialize (cb_data_p->configuration->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Test_I_Stream::initialize(), aborting\n")));
    goto error;
  } // end IF

  stream_p = cb_data_p->stream;
  session_data_container_p = &cb_data_p->stream->getR_2 ();
  session_data_p = &session_data_container_p->getR ();
  cb_data_p->progressData.sessionId = session_data_p->sessionId;
  converter << session_data_p->sessionId;

//  module_p =
//    const_cast<Stream_Module_t*> (cb_data_p->audioStream->find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_ENCODER_DEFAULT_NAME_STRING),
//                                                                false,  // do not sanitize module names
//                                                                false)); // do not recurse upstream
//  module_2 =
//    const_cast<Stream_Module_t*> (cb_data_p->audioStream->find (ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING),
//                                                                false,  // do not sanitize module names
//                                                                false)); // do not recurse upstream

  // generate context id
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
  thread_data_p->CBData->UIState->contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_INFORMATION,
                                                                     gtk_statusbar_get_context_id (statusbar_p,
                                                                                                   converter.str ().c_str ())));
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)

  ACE_ASSERT (stream_p);
  stream_p->start ();

  //module_p =
  //  const_cast<Stream_Module_t*> (cb_data_p->stream->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING)));
  //ACE_ASSERT (module_p);
  //cb_data_p->dispatch = dynamic_cast<Common_IDispatch*> (module_p->writer ());
  //ACE_ASSERT (cb_data_p->dispatch);

  stream_p->wait (true, false, false);

  //module_p =
  //  const_cast<Stream_Module_t*> (cb_data_p->stream->find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_DECODER_DEFAULT_NAME_STRING)));
  //ACE_ASSERT (module_p);
  //result_2 = cb_data_p->stream->remove (module_p,
  //                                      true,
  //                                      true);
  //ACE_ASSERT (result_2);

  result = NULL;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, thread_data_p->CBData->UIState->lock, -1);
#else
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, thread_data_p->CBData->UIState->lock, std::numeric_limits<void*>::max ());
#endif // ACE_WIN32 || ACE_WIN64
    thread_data_p->CBData->progressData.completedActions.insert (thread_data_p->eventSourceId);
  } // end lock scope

  //thread_data_p->CBData->dispatch = NULL;

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

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());
  ACE_ASSERT (cb_data_p->configuration);

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
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_AUDIO_MESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<ACE_UINT32>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_VIDEO_MESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<ACE_UINT32>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<ACE_UINT32>::max ());

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT64>::max ()));

  std::string filename_string;
  Test_I_ExtractStream_StreamConfiguration_t::ITERATOR_T stream_iterator =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());
  ACE_ASSERT ((*stream_iterator).second.second->fileIdentifier.identifierDiscriminator == Common_File_Identifier::FILE);
  filename_string = (*stream_iterator).second.second->fileIdentifier.identifier;
  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SOURCE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  std::string file_uri =
    ACE_TEXT_ALWAYS_CHAR ("file://") + (filename_string.empty () ? Common_File_Tools::getTempDirectory ()
                                                                 : filename_string);
  if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
                                                file_uri.c_str ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_uri (\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (file_uri.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF

  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_STREAM_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_ASCENDING);
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_STREAM_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkCellRenderer* cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p),
                              cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p),
                                  cell_renderer_p,
                                  "text", 0,
                                  NULL);

  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_SAVE_NAME)));
  ACE_ASSERT (entry_p);
  file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  //GtkFileChooserDialog* file_chooser_dialog_p =
  //  GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME)));
  //ACE_ASSERT (file_chooser_dialog_p);
  GtkFileFilter* file_filter_p =
    GTK_FILE_FILTER (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILEFILTER_WAV_NAME)));
  ACE_ASSERT (file_filter_p);
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("application/x-troff-msaudio"));
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("audio/wav"));
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("audio/msaudio"));
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("audio/x-msaudio"));
  gtk_file_filter_add_pattern (file_filter_p,
                               ACE_TEXT ("*.wav"));
  gtk_file_filter_set_name (file_filter_p,
                            ACE_TEXT ("WAV files"));
  //GError* error_p = NULL;
  //GFile* file_p = NULL;
  //gchar* filename_p = NULL;
//  bool is_display_b = false;
  bool is_fullscreen_b = false;
  filename_string = (*stream_iterator).second.second->targetFileName;
  GtkEntryBuffer* entry_buffer_p = gtk_entry_get_buffer (entry_p);
  ACE_ASSERT (entry_buffer_p);
  gtk_entry_buffer_set_text (entry_buffer_p,
                             (filename_string.empty () ? ACE_TEXT_ALWAYS_CHAR ("")
                                                       : ACE_TEXT_ALWAYS_CHAR (ACE::basename (filename_string.c_str (), ACE_DIRECTORY_SEPARATOR_CHAR))),
                             -1);
  file_uri =
    ACE_TEXT_ALWAYS_CHAR ("file://") + (filename_string.empty () ? Common_File_Tools::getTempDirectory ()
                                                                 : filename_string);
  if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
                                                file_uri.c_str ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_uri (\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (file_uri.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_SAVE_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                !filename_string.empty ());

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  switch (cb_data_p->mediaFramework)
//  {
//    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//    {
//      is_fullscreen_b = (*stream_iterator).second.second->fullScreen;
//      break;
//    }
//    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//    {
//      is_fullscreen_b =
//        (*mediafoundation_stream_iterator).second.second->fullScreen;
//      break;
//    }
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
//                  cb_data_p->mediaFramework));
//      return G_SOURCE_REMOVE;
//    }
//  } // end SWITCH
//#else
////  is_display_b =
////      !(*iterator_3).second.second->deviceIdentifier.identifier.empty ();
////  is_fullscreen_b = (*iterator_2).second.second->fullScreen;
//#endif // ACE_WIN32 || ACE_WIN64
//  toggle_button_p =
//    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_DISPLAY_NAME)));
//  ACE_ASSERT (toggle_button_p);
//  gtk_toggle_button_set_active (toggle_button_p,
//                                is_display_b);

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_ADAPTER_NAME)));
  ACE_ASSERT (list_store_p);
  if (!load_display_adapters (list_store_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_display_adapters(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_ADAPTER_NAME)));
  ACE_ASSERT (combo_box_p);
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

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_DISPLAY_NAME)));
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
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME)));
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

  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_FULLSCREEN_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                is_fullscreen_b);

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

  //GtkDrawingArea* drawing_area_p =
  //  GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
  //                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_VIDEO_NAME)));
  //ACE_ASSERT (drawing_area_p);

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
      g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET_MS,
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
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_CUT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                            FALSE);

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

  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_VIDEO_NAME)));
  ACE_ASSERT (drawing_area_p);

  (*stream_iterator).second.second->window.gdk_window =
    gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  (*stream_iterator).second.second->window.type = Common_UI_Window::TYPE_GTK;

  // step10: retrieve canvas coordinates, window handle and pixel buffer
  GtkAllocation allocation;
  ACE_OS::memset (&allocation, 0, sizeof (GtkAllocation));
  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
                             &allocation);
  //GdkWindow* window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  //ACE_ASSERT (window_p);
  //ACE_ASSERT (gdk_win32_window_is_win32 (window_p));
  //(*stream_iterator_3).second.second->window = window_p;
//        gdk_win32_window_get_impl_hwnd (window_p);
  //cb_data_p->configuration->direct3DConfiguration.focusWindow =
  //  NULL;
  //cb_data_p->configuration->direct3DConfiguration.presentationParameters.hDeviceWindow =
  //  gdk_win32_window_get_impl_hwnd (window_p);

  Common_Image_Resolution_t resolution_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  resolution_s.cx = allocation.width;
  resolution_s.cy = allocation.height;
#else
  resolution_s.width = allocation.width;
  resolution_s.height = allocation.height;
#endif // ACE_WIN32 || ACE_WIN64
  (*stream_iterator).second.second->outputFormat.video.resolution = resolution_s;

  //(*stream_iterator).second.second->area.bottom =
  //  allocation.y + allocation.height;
  //(*stream_iterator).second.second->area.left = allocation.x;
  //(*stream_iterator).second.second->area.right =
  //  allocation.x + allocation.width;
  //(*stream_iterator).second.second->area.top = allocation.y;

  //(*stream_iterator).second.second->pixelBuffer =
  //  cb_data_p->pixelBuffer;

  //ACE_ASSERT (IsWindow (cb_data_p->configuration->direct3DConfiguration.presentationParameters.hDeviceWindow));
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("drawing area window handle: 0x%@; size: %dx%d\n"),
  //            (*stream_iterator).second.second->window,
  //            allocation.width, allocation.height));

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

//  // sanity check(s)
//  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
//    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
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

  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, cb_data_p->UIState->lock, G_SOURCE_REMOVE);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  // *IMPORTANT NOTE*: there are two major reasons for being here that are not
  //                   mutually exclusive, so there could be a race:
  //                   - user pressed stop
  //                   - there was an asynchronous error on the stream
  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_button_set_label (GTK_BUTTON (toggle_button_p),
                        GTK_STOCK_MEDIA_PLAY);
  if (gtk_toggle_button_get_active (toggle_button_p))
  {
    un_toggling_stream = true;
    gtk_toggle_button_set_active (toggle_button_p,
                                  FALSE);
  } // end IF
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_CUT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_SNAPSHOT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_SOURCE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);
  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_OPTIONS_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);
  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_SAVE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);
  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_DISPLAY_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);

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

//  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
//    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);

//  // sanity check(s)
//  ACE_ASSERT (cb_data_p);
//  Common_UI_GTK_BuildersIterator_t iterator =
//    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

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
//  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, cb_data_p->UIState->lock, G_SOURCE_REMOVE);
//    if (cb_data_p->UIState->logStack.empty ())
//      return G_SOURCE_CONTINUE;

//    // step1: convert text
//    for (Common_MessageStackConstIterator_t iterator_2 = cb_data_p->UIState->logStack.begin ();
//         iterator_2 != cb_data_p->UIState->logStack.end ();
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
//    cb_data_p->UIState->logStack.clear ();
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
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
      static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Test_I_ExtractStream_UI_EventType* event_p = NULL;
  int result = -1;
  enum Test_I_ExtractStream_UI_EventType event_e = STREAM_AV_UI_EVENT_INVALID;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, cb_data_p->UIState->lock, G_SOURCE_REMOVE);
    for (Test_I_ExtractStream_UI_EventsIterator_t iterator_2 (cb_data_p->UIState->eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_STARTED:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_AUDIO_MESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_VIDEO_MESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
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
        case COMMON_UI_EVENT_FINISHED:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_ABORT:
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
        case COMMON_UI_EVENT_RESIZE:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          is_session_message = true;
          break;
        }
        case STREAM_AV_UI_EVENT_DATA_AUDIO:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (cb_data_p->progressData.statistic.bytes));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_AUDIO_MESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          break;
        }
        case STREAM_AV_UI_EVENT_DATA_VIDEO:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (cb_data_p->progressData.statistic.bytes));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_VIDEO_MESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
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
idle_update_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_display_cb"));

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (userData_in);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  // trigger refresh of the 2D areas
  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_VIDEO_NAME)));
  ACE_ASSERT (drawing_area_p);
  GtkDrawingArea* drawing_area_2 =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_AUDIO_NAME)));
  ACE_ASSERT (drawing_area_2);
  gdk_window_invalidate_rect (gtk_widget_get_window (GTK_WIDGET (drawing_area_p)),
                              NULL,   // whole window
                              FALSE); // invalidate children ?
  gdk_window_invalidate_rect (gtk_widget_get_window (GTK_WIDGET (drawing_area_2)),
                              NULL,   // whole window
                              FALSE); // invalidate children ?

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  struct Test_I_ExtractStream_ProgressData* data_p =
      static_cast<struct Test_I_ExtractStream_ProgressData*> (userData_in);

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
#endif // ACE_WIN32 || ACE_WIN64
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

  ACE_Time_Value now = ACE_OS::gettimeofday ();
  ACE_Time_Value elapsed_time = now - data_p->timeStamp;
  data_p->timeStamp = now;
  unsigned long milliseconds_i = elapsed_time.msec ();
  unsigned long delta_audio_frames =
    data_p->statistic.capturedFrames - data_p->lastStatistic.capturedFrames;
  unsigned long delta_video_frames =
    data_p->statistic.totalFrames - data_p->lastStatistic.totalFrames;
  data_p->lastStatistic = data_p->statistic;
  unsigned int audio_frames_per_second =
    static_cast<unsigned int> ((1000.0F / (float)milliseconds_i) * (float)delta_audio_frames);
  unsigned int video_frames_per_second =
    static_cast<unsigned int> ((1000.0F / (float)milliseconds_i) * (float)delta_video_frames);

  std::ostringstream converter;
  converter << audio_frames_per_second;
  converter << ACE_TEXT_ALWAYS_CHAR (" / ");
  converter << video_frames_per_second;
  converter << ACE_TEXT_ALWAYS_CHAR (" fps");

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
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

//  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
//    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);

//  Common_UI_GTK_BuildersIterator_t iterator =
//    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  // sanity check(s)
//  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

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
togglebutton_play_toggled_cb (GtkToggleButton* toggleButton_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::toggleaction_play_toggled_cb"));

  // handle untoggle --> PLAY
  if (un_toggling_stream)
  {
    un_toggling_stream = false;
    return; // done
  } // end IF

  bool is_active_b = gtk_toggle_button_get_active (toggleButton_in);

  // --> user pressed play/pause/stop

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  Stream_IStreamControlBase* stream_p = NULL;
  Test_I_ExtractStream_StreamConfiguration_t::ITERATOR_T stream_iterator;
  stream_p = cb_data_p->stream;
  ACE_ASSERT (cb_data_p->configuration);
  stream_iterator =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());
  ACE_ASSERT (stream_p);

  // toggle ?
  if (!is_active_b)
  {
    // --> user pressed pause/stop

    // stop stream
    stream_p->stop (false,  // wait ?
                    false,  // recurse upstream ?
                    false); // high priority ?
    return;
  } // end IF

  GtkButton* button_p = NULL;
  GtkFrame* frame_p = NULL;

  // --> user pressed play

  struct Test_I_ExtractStream_UI_ThreadData* thread_data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_thread_t thread_id = std::numeric_limits<unsigned long>::max ();
#else
  ACE_thread_t thread_id = -1;
#endif
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;
  guint event_source_id_i;

  // step0: modify widgets
  gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                        GTK_STOCK_MEDIA_STOP);

  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_CUT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);
//  button_p =
//    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_REPORT_NAME)));
//  ACE_ASSERT (button_p);
//  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_SNAPSHOT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);

  // step1: set up progress reporting
  ACE_OS::memset (&cb_data_p->progressData.statistic, 0, sizeof (struct Stream_Statistic));
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
  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_SAVE_NAME)));
  ACE_ASSERT (toggle_button_p);
  GtkFileChooserButton* file_chooser_button_p = NULL;
  //  GError* error_p = NULL;
  std::string filename_string;
  file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SOURCE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  gchar* filename_p = 
    gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser_button_p));
  if (filename_p)
  {
    filename_string = filename_p;
    //filename_string = Common_UI_GTK_Tools::UTF8ToLocale (filename_string);
    g_free (filename_p); filename_p = NULL;
  } // end IF
  //ACE_ASSERT (Common_File_Tools::isReadable (filename_string));
  (*stream_iterator).second.second->fileIdentifier.identifier = filename_string;

  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_SLOW_NAME)));
  ACE_ASSERT (toggle_button_p);
  if (gtk_toggle_button_get_active (toggle_button_p))
  {
    GtkSpinButton* spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SLOW_NAME)));
    ACE_ASSERT (spin_button_p);
    cb_data_p->configuration->streamConfiguration.configuration_->slowDown =
      gtk_spin_button_get_value_as_int (spin_button_p);
  } // end IF
  else
    cb_data_p->configuration->streamConfiguration.configuration_->slowDown = -1;

  GtkEntry* entry_p = NULL;
  filename_string.clear ();
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_SAVE_NAME)));
  ACE_ASSERT (toggle_button_p);
  if (!gtk_toggle_button_get_active (toggle_button_p))
    goto continue_;
  file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  filename_string =
    gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p));
  ACE_ASSERT (Common_File_Tools::isDirectory (filename_string));
  ACE_ASSERT (Common_File_Tools::isWriteable (filename_string));
  entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_SAVE_NAME)));
  ACE_ASSERT (entry_p);
  filename_string += ACE_DIRECTORY_SEPARATOR_STR;
  filename_string +=
    ACE_TEXT_ALWAYS_CHAR (gtk_entry_buffer_get_text (gtk_entry_get_buffer (entry_p)));
  ACE_ASSERT (Common_File_Tools::isValidPath (filename_string));

continue_:
  (*stream_iterator).second.second->targetFileName = filename_string;

  // step3: start processing thread
  ACE_NEW_NORETURN (thread_data_p,
                    struct Test_I_ExtractStream_UI_ThreadData ());
  if (!thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  thread_data_p->CBData = cb_data_p;
  ACE_TCHAR thread_name[BUFSIZ];
  ACE_OS::memset (thread_name, 0, sizeof (thread_name));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_OS::strcpy (thread_name,
                  ACE_TEXT (TEST_I_STREAM_THREAD_NAME));
#else
  ACE_ASSERT (COMMON_THREAD_PTHREAD_NAME_MAX_LENGTH <= BUFSIZ);
  ACE_OS::strncpy (thread_name,
                   ACE_TEXT (TEST_I_STREAM_THREAD_NAME),
                   std::min (static_cast<size_t> (COMMON_THREAD_PTHREAD_NAME_MAX_LENGTH - 1), static_cast<size_t> (ACE_OS::strlen (ACE_TEXT (TEST_I_STREAM_THREAD_NAME)))));
#endif // ACE_WIN32 || ACE_WIN64
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
      g_timeout_add (//G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                     COMMON_UI_REFRESH_DEFAULT_PROGRESS_MS, // ms (?)
                     idle_update_progress_cb,
                     &cb_data_p->progressData);//,
                     //NULL);
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
  // step4: start display updates
  event_source_id_i =
    g_timeout_add (COMMON_UI_REFRESH_DEFAULT_VIDEO_MS, // ms
                   idle_update_display_cb,
                   userData_in);
  ACE_ASSERT (event_source_id_i);
  cb_data_p->UIState->eventSourceIds.insert (event_source_id_i);

  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_SOURCE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);
  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_OPTIONS_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);
  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_SAVE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);
  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_DISPLAY_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);

  return;

error:
  gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                        GTK_STOCK_MEDIA_RECORD);
  //gtk_action_set_sensitive (action_p, false);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
  //                          true);
} // toggleaction_play_toggled_cb

void
togglebutton_slow_toggled_cb (GtkToggleButton* toggleButton_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_slow_toggled_cb"));

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());
  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SLOW_NAME)));
  ACE_ASSERT (spin_button_p);
  
  gtk_widget_set_sensitive (GTK_WIDGET (spin_button_p),
                            gtk_toggle_button_get_active (toggleButton_in));
} // toggleaction_save_toggled_cb

void
togglebutton_save_toggled_cb (GtkToggleButton* toggleButton_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_save_toggled_cb"));

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  Test_I_ExtractStream_StreamConfiguration_t::ITERATOR_T stream_iterator;
  ACE_ASSERT (cb_data_p->configuration);
  stream_iterator =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  GError* error_p = NULL;
  GFile* file_p = NULL;
  std::string filename_string;
  if (gtk_toggle_button_get_active (toggleButton_in))
  {
    file_p =
      gtk_file_chooser_get_current_folder_file (GTK_FILE_CHOOSER (file_chooser_button_p));
    ACE_ASSERT (file_p);
    char* filename_p = g_file_get_path (file_p);
    ACE_ASSERT (filename_p);
    filename_string = filename_p;
    g_free (filename_p); filename_p = NULL;
    g_object_unref (G_OBJECT (file_p)); file_p = NULL;
  } // end IF
  else
  {
    file_p =
      g_file_new_for_path (Common_File_Tools::getTempDirectory ().c_str ());
    ACE_ASSERT (file_p);
    if (!gtk_file_chooser_set_current_folder_file (GTK_FILE_CHOOSER (file_chooser_button_p),
                                                   file_p,
                                                   &error_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_file(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (file_p),
                  ACE_TEXT (error_p->message)));
      g_error_free (error_p); error_p = NULL;
      g_object_unref (G_OBJECT (file_p)); file_p = NULL;
      return;
    } // end IF
    g_object_unref (G_OBJECT (file_p)); file_p = NULL;
  } // end ELSE

  (*stream_iterator).second.second->targetFileName = filename_string;
} // toggleaction_save_toggled_cb

void
togglebutton_display_toggled_cb (GtkToggleButton* toggleButton_in,
                                 gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_display_toggled_cb"));

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  Test_I_ExtractStream_StreamConfiguration_t::ITERATOR_T stream_iterator;
  ACE_ASSERT (cb_data_p->configuration);
  stream_iterator =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());

  if (!gtk_toggle_button_get_active (toggleButton_in))
  {
//(*stream_iterator).second.second->display.device.clear ();
    return;
  } // end IF

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter iterator_3;
  gboolean result = gtk_combo_box_get_active_iter (combo_box_p,
                                                   &iterator_3);
  ACE_ASSERT (result);
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_DISPLAY_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  //(*stream_iterator).second.second->display.device =
  //  g_value_get_string (&value);
  g_value_unset (&value);
}

void
togglebutton_fullscreen_toggled_cb (GtkToggleButton* toggleButton_in,
                                    gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_fullscreen_toggled_cb"));

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);

  bool is_active_b = gtk_toggle_button_get_active (toggleButton_in);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  Stream_IStreamControlBase* stream_base_p = NULL;
  Stream_IStream_t* stream_p = NULL;
  Test_I_ExtractStream_StreamConfiguration_t::ITERATOR_T stream_iterator;
  stream_base_p = cb_data_p->stream;
  stream_p = cb_data_p->stream;
  ACE_ASSERT (cb_data_p->configuration);
  stream_iterator =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());
  //(*stream_iterator).second.second->fullScreen = is_active_b;
  ACE_ASSERT (stream_base_p);
  if (!stream_base_p->isRunning ())
    return;

  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());
  GtkWindow* window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_WINDOW_FULLSCREEN)));
  ACE_ASSERT (window_p);

  if (is_active_b)
  {
    gtk_widget_show (GTK_WIDGET (window_p));
//  gtk_window_fullscreen (window_p);
    gtk_window_maximize (window_p);
  } // end IF
  else
  {
//    gtk_window_minimize (window_p);
//  gtk_window_unfullscreen (window_p);
    gtk_widget_hide (GTK_WIDGET (window_p));
  } // end ELSE

  ACE_ASSERT (stream_p);
  //const Stream_Module_t* module_p = NULL;
  //module_p =
  //  stream_p->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING));
  //if (!module_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to Stream_IStream::find(\"Display\"), returning\n"),
  //              ACE_TEXT (stream_p->name ().c_str ())));
  //  return;
  //} // end IF
  //Common_UI_IFullscreen* ifullscreen_p =
  //  dynamic_cast<Common_UI_IFullscreen*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  //if (!ifullscreen_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s:Display: failed to dynamic_cast<Common_UI_IFullscreen*>(0x%@), returning\n"),
  //              ACE_TEXT (stream_p->name ().c_str ()),
  //              const_cast<Stream_Module_t*> (module_p)->writer ()));
  //  return;
  //} // end IF
  //try {
  //  ifullscreen_p->toggle ();
  //} catch (...) {
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("caught exception in Common_UI_IFullscreen::toggle(), returning\n")));
  //  return;
  //}
} // toggleaction_fullscreen_toggled_cb

void
button_display_reset_clicked_cb (GtkButton* button_in,
                                 gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_display_reset_clicked_cb"));

  ACE_UNUSED_ARG (button_in);
  ACE_UNUSED_ARG (userData_in);
} // button_display_reset_clicked_cb

void
button_cut_clicked_cb (GtkButton* button_in,
                       gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_cut_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);

  cb_data_p->stream->control (STREAM_CONTROL_STEP,
                              false);
} // button_cut_clicked_cb

void
button_report_clicked_cb (GtkButton* button_in,
                          gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_report_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
} // button_report_clicked_cb

void
button_about_clicked_cb (GtkButton* button_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (dialog_p);

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
} // button_about_clicked_cb

void
button_quit_clicked_cb (GtkButton* button_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->UIState);
  ACE_ASSERT (cb_data_p->stream);

  // step1: remove event sources
  { ACE_Guard<ACE_Thread_Mutex> aGuard (cb_data_p->UIState->lock);
    for (Common_UI_GTK_EventSourceIdsIterator_t iterator = cb_data_p->UIState->eventSourceIds.begin ();
        iterator != cb_data_p->UIState->eventSourceIds.end ();
        iterator++)
     if (!g_source_remove (*iterator))
       ACE_DEBUG ((LM_ERROR,
                   ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                   *iterator));
   cb_data_p->UIState->eventSourceIds.clear ();
  } // end lock scope

  // step2: stop stream ?
  enum Stream_StateMachine_ControlState status_e = cb_data_p->stream->status ();
  if ((status_e == STREAM_STATE_RUNNING) ||
      (status_e == STREAM_STATE_PAUSED))
    cb_data_p->stream->stop (false,  // wait for completion ?
                             true,   // recurse upstream ?
                             false); // high priority ?

  // step2: initiate shutdown sequence
  int result = ACE_OS::raise (SIGINT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                SIGINT));
} // button_quit_clicked_cb

void
combobox_stream_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_stream_changed_cb"));

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());
  ACE_ASSERT (cb_data_p->configuration);
  Test_I_ExtractStream_StreamConfiguration_t::ITERATOR_T stream_iterator =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());

  GtkTreeIter iterator_4;
  gboolean result = gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                                   &iterator_4);
  if (!result)
    return; // nothing to do
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_STREAM_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION (2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
#if GTK_CHECK_VERSION (2,30,0)
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value_2;
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_4,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  (*stream_iterator).second.second->streamIndex =
    static_cast<int> (g_value_get_uint (&value));
  g_value_unset (&value);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_4,
                            2, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_UINT);
  (*stream_iterator).second.second->codecConfiguration->codecId =
    static_cast<enum AVCodecID> (g_value_get_uint (&value_2));
  g_value_unset (&value_2);
  cb_data_p->configuration->streamConfiguration.configuration_->mode = 
    (Stream_MediaFramework_Tools::isAudioCodecId ((*stream_iterator).second.second->codecConfiguration->codecId) ? TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_AUDIO_ONLY
                                                                                                                 : TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_VIDEO_ONLY);

  (*stream_iterator).second.second->targetFileName =
    (cb_data_p->configuration->streamConfiguration.configuration_->mode == TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_AUDIO_ONLY ? ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_AUDIO_FILE)
                                                                                                                               : ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_AUDIO_VIDEO_FILE));
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_SAVE_NAME)));
  ACE_ASSERT (entry_p);
  gtk_entry_buffer_set_text (gtk_entry_get_buffer (entry_p),
                             (*stream_iterator).second.second->targetFileName.c_str (),
                             -1);
} // combobox_stream_changed_cb

void
combobox_display_changed_cb (GtkWidget* widget_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_display_changed_cb"));

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());
  ACE_ASSERT (cb_data_p->configuration);
  Test_I_ExtractStream_StreamConfiguration_t::ITERATOR_T stream_iterator =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());

  GtkTreeIter iterator_4;
  gboolean result = gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                                   &iterator_4);
  ACE_ASSERT (result);
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_DISPLAY_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_4,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  //(*stream_iterator).second.second->display.device =
  //  g_value_get_string (&value);
  g_value_unset (&value);

  // select corresponding adapter
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_ADAPTER_NAME)));
  ACE_ASSERT (combo_box_p);
  struct Common_UI_DisplayAdapter display_adapter_s;
  //display_adapter_s =
  //  Common_UI_Tools::getAdapter ((*stream_iterator).second.second->display);
  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value,
                      display_adapter_s.device.c_str ());
  Common_UI_GTK_Tools::selectValue (combo_box_p,
                                    value,
                                    1);
} // combobox_display_changed_cb

//#if GTK_CHECK_VERSION (3,0,0)
//gboolean
//drawingarea_audio_draw_cb (GtkWidget* widget_in,
//                           cairo_t* context_in,
//                           gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::drawingarea_audio_draw_cb"));
//
//  ACE_UNUSED_ARG (widget_in);
//
//     // sanity check(s)
//  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
//    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
//  ACE_ASSERT (cb_data_p);
//  if (!cb_data_p->dispatch_2)
//    return FALSE; // propagate event
//  ACE_ASSERT (!cb_data_p->spectrumAnalyzerCBData.context);
//  ACE_ASSERT (cb_data_p->spectrumAnalyzerCBData.window);
//
//  cb_data_p->spectrumAnalyzerCBData.context = context_in;
//  try {
//    cb_data_p->dispatch_2->dispatch (&cb_data_p->spectrumAnalyzerCBData);
//  } catch (...) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
//    cb_data_p->spectrumAnalyzerCBData.context = NULL;
//    return FALSE; // propagate event
//  }
//  cb_data_p->spectrumAnalyzerCBData.context = NULL;
//
//  return TRUE; // do not propagate
//} // drawingarea_audio_draw_cb
//
//gboolean
//drawingarea_video_draw_cb (GtkWidget* widget_in,
//                           cairo_t* context_in,
//                           gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::drawingarea_video_draw_cb"));
//
//  ACE_UNUSED_ARG (widget_in);
//
//  // sanity check(s)
//  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
//    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
//  ACE_ASSERT (cb_data_p);
//  if (!cb_data_p->dispatch)
//    return FALSE; // propagate event
//
//  try {
//    cb_data_p->dispatch->dispatch (context_in);
//  } catch (...) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
//    return FALSE; // propagate event
//  }
//
//  return TRUE; // do not propagate
//} // drawingarea_video_draw_cb
//#else
//gboolean
//drawingarea_audio_expose_event_cb (GtkWidget* widget_in,
//                                   GdkEvent* event_in,
//                                   gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::drawingarea_audio_expose_event_cb"));
//
//  ACE_UNUSED_ARG (event_in);
//
//  // sanity check(s)
//  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
//    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
//  ACE_ASSERT (cb_data_p);
//  if (!cb_data_p->dispatch_2)
//    return FALSE; // propagate event
//  ACE_ASSERT (!cb_data_p->spectrumAnalyzerCBData.context);
//  ACE_ASSERT (cb_data_p->spectrumAnalyzerCBData.window);
//
//  cb_data_p->spectrumAnalyzerCBData.context =
//    gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (widget_in)));
//  if (unlikely (!cb_data_p->spectrumAnalyzerCBData.context))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
//    return FALSE; // propagate event
//  } // end IF
//
//  try {
//    cb_data_p->dispatch_2->dispatch (&cb_data_p->spectrumAnalyzerCBData);
//  } catch (...) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
//    cairo_destroy (cb_data_p->spectrumAnalyzerCBData.context);
//    cb_data_p->spectrumAnalyzerCBData.context = NULL;
//    return FALSE; // propagate event
//  }
//  cairo_destroy (cb_data_p->spectrumAnalyzerCBData.context);
//  cb_data_p->spectrumAnalyzerCBData.context = NULL;
//
//  return TRUE; // do not propagate
//} // drawingarea_audio_expose_event_cb
//
//gboolean
//drawingarea_video_expose_event_cb (GtkWidget* widget_in,
//                                   GdkEvent* event_in,
//                                   gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::drawingarea_video_expose_event_cb"));
//
//  ACE_UNUSED_ARG (event_in);
//
//  // sanity check(s)
//  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
//    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
//  ACE_ASSERT (cb_data_p);
//  if (!cb_data_p->dispatch)
//    return FALSE; // propagate event
//
//  cairo_t* context_p =
//    gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (widget_in)));
//  if (unlikely (!context_p))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
//    return FALSE; // propagate event
//  } // end IF
//
//  try {
//    cb_data_p->dispatch->dispatch (context_p);
//  } catch (...) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
//    cairo_destroy (context_p);
//    return FALSE; // propagate event
//  }
//  cairo_destroy (context_p);
//
//  return TRUE; // do not propagate
//} // drawingarea_video_expose_event_cb
//#endif // GTK_CHECK_VERSION (3,0,0)

//void
//drawingarea_configure_event_cb (GtkWindow* window_in,
//                                GdkEvent* event_in,
//                                gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::drawingarea_configure_event_cb"));

//  Test_I_ExtractStream_UI_CBData* data_p =
//    static_cast<Test_I_ExtractStream_UI_CBData*> (userData_in);

//  // sanity check(s)
//  ACE_ASSERT (data_p);
//  ACE_ASSERT (data_p->configuration);

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  if (!data_p->configuration->moduleHandlerConfiguration.window          ||
//      !data_p->configuration->moduleHandlerConfiguration.windowController) // <-- window not realized yet ?
//    return;
//#else
//  if (!data_p->configuration->moduleHandlerConfiguration.window) // <-- window not realized yet ?
//    return;
//#endif

//  Common_UI_GTK_BuildersIterator_t iterator =
//    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  // sanity check(s)
//  ACE_ASSERT (iterator != data_p->builders.end ());

//  GtkDrawingArea* drawing_area_p =
//    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
//                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
//  ACE_ASSERT (drawing_area_p);
//  GtkAllocation allocation;
//  ACE_OS::memset (&allocation, 0, sizeof (GtkAllocation));
//  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
//                             &allocation);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  // sanity check(s)
//  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.windowController);

//  data_p->configuration->moduleHandlerConfiguration.area.bottom =
//    allocation.height;
//  data_p->configuration->moduleHandlerConfiguration.area.left =
//    allocation.x;
//  data_p->configuration->moduleHandlerConfiguration.area.right =
//    allocation.width;
//  data_p->configuration->moduleHandlerConfiguration.area.top =
//    allocation.y;

//  //HRESULT result =
//  //  data_p->configuration->moduleHandlerConfiguration.windowController->SetWindowPosition (data_p->configuration->moduleHandlerConfiguration.area.left,
//  //                                                                                                               data_p->configuration->moduleHandlerConfiguration.area.top,
//  //                                                                                                               data_p->configuration->moduleHandlerConfiguration.area.right,
//  //                                                                                                               data_p->configuration->moduleHandlerConfiguration.area.bottom);
//  //if (FAILED (result))
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IVideoWindow::SetWindowPosition(%d,%d,%d,%d): \"%s\", continuing\n"),
//  //              data_p->configuration->moduleHandlerConfiguration.area.left, data_p->configuration->moduleHandlerConfiguration.area.top,
//  //              data_p->configuration->moduleHandlerConfiguration.area.right, data_p->configuration->moduleHandlerConfiguration.area.bottom,
//  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//#else
//  data_p->configuration->moduleHandlerConfiguration.area =
//    allocation;
//#endif
//} // drawingarea_configure_event_cb

//gboolean
//drawingarea_audio_resize_end (gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::drawingarea_audio_resize_end"));
//
//  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
//    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
//  ACE_ASSERT (cb_data_p);
//  Common_UI_GTK_BuildersIterator_t iterator =
//    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  // sanity check(s)
//  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());
//  GtkDrawingArea* drawing_area_p =
//    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
//                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_AUDIO_NAME)));
//  ACE_ASSERT (drawing_area_p);
//
//  GtkAllocation allocation_s;
//  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
//                             &allocation_s);
//
//  Stream_IStream_t* stream_p = NULL;
//  const Stream_Module_t* module_p = NULL;
//  std::string module_name;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  struct Test_I_ExtractStream_UI_CBData* cb_data_p = NULL;
//  Test_I_ExtractStream_StreamConfiguration_t::ITERATOR_T stream_iterator;
//  struct Test_I_ExtractStream_MediaFoundation_UI_CBData* mediafoundation_cb_data_p =
//    NULL;
//  Test_I_ExtractStream_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
//  switch (cb_data_p->mediaFramework)
//  {
//    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//    {
//      cb_data_p =
//        static_cast<struct Test_I_ExtractStream_UI_CBData*> (cb_data_p);
//      stream_p = cb_data_p->audioStream;
//      ACE_ASSERT (cb_data_p->configuration);
//      stream_iterator =
//        cb_data_p->configuration->audioStreamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING));
//      ACE_ASSERT (stream_iterator != cb_data_p->configuration->audioStreamConfiguration.end ());
//
//      Common_Image_Resolution_t resolution_s;
//      resolution_s.cx = allocation_s.width;
//      resolution_s.cy = allocation_s.height;
//      Stream_MediaFramework_DirectShow_Tools::setResolution (resolution_s,
//                                                             (*stream_iterator).second.second->outputFormat);
//
//      if (!cb_data_p->audioStream->isRunning ())
//        return G_SOURCE_REMOVE;
//
//      module_name =
//        ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING);
//      break;
//    }
//    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//    {
//      mediafoundation_cb_data_p =
//        static_cast<struct Test_I_ExtractStream_MediaFoundation_UI_CBData*> (cb_data_p);
//      stream_p = mediafoundation_cb_data_p->audioStream;
//      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
//      mediafoundation_stream_iterator =
//        mediafoundation_cb_data_p->configuration->audioStreamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING));
//      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->audioStreamConfiguration.end ());
//
//      HRESULT result_2 =
//        MFSetAttributeSize (const_cast<IMFMediaType*> ((*mediafoundation_stream_iterator).second.second->outputFormat),
//                            MF_MT_FRAME_SIZE,
//                            static_cast<UINT32> (allocation_s.width), static_cast<UINT32> (allocation_s.height));
//      ACE_ASSERT (SUCCEEDED (result_2));
//
//      if (!mediafoundation_cb_data_p->audioStream->isRunning ())
//        return G_SOURCE_REMOVE;
//
//      module_name =
//        ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING);
//
//      break;
//    }
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
//                  cb_data_p->mediaFramework));
//      return G_SOURCE_REMOVE;
//    }
//  } // end SWITCH
//#else
//  struct Test_I_ExtractStream_V4L_UI_CBData* cb_data_p =
//    static_cast<struct Test_I_ExtractStream_V4L_UI_CBData*> (cb_data_p);
//  ACE_ASSERT (cb_data_p->configuration);
//  stream_p = cb_data_p->audioStream;
//  Test_I_ExtractStream_ALSA_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
//    cb_data_p->configuration->audioStreamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//  ACE_ASSERT (iterator_2 != cb_data_p->configuration->audioStreamConfiguration.end ());
//  Test_I_ExtractStream_ALSA_V4L_StreamConfiguration_t::ITERATOR_T iterator_3 =
//#if defined (GTK_USE)
//    cb_data_p->configuration->audioStreamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING));
//#else
//    cb_data_p->configuration->audioStreamConfiguration.find (Stream_Visualization_Tools::rendererToModuleName (STREAM_VISUALIZATION_VIDEORENDERER_X11));
//#endif // GTK_USE
//  ACE_ASSERT (iterator_3 != cb_data_p->configuration->audioStreamConfiguration.end ());
//
//  //  (*iterator_2).second.second->outputFormat.resolution.height =
//  //      allocation_in->height;
//  //  (*iterator_2).second.second->outputFormat.resolution.width =
//  //      allocation_in->width;
//  (*iterator_3).second.second->outputFormat.video.format.height =
//    allocation_s.height;
//  (*iterator_3).second.second->outputFormat.video.format.width =
//    allocation_s.width;
//
//  if (!cb_data_p->audioStream->isRunning ())
//    return G_SOURCE_REMOVE;
//
//  module_name =
//    ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING);
//#endif // ACE_WIN32 || ACE_WIN64
//  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());
//  ACE_ASSERT (stream_p);
//
//  // *NOTE*: update the analyzer
//
//  // step1:
//  module_p = stream_p->find (module_name);
//  if (!module_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to Stream_IStream::find(\"%s\"), returning\n"),
//                ACE_TEXT (stream_p->name ().c_str ()),
//                ACE_TEXT (module_name.c_str ())));
//    return G_SOURCE_REMOVE;
//  } // end IF
//  Common_ISetP_T<GdkWindow>* iset_p =
//    dynamic_cast<Common_ISetP_T<GdkWindow>*> (const_cast<Stream_Module_t*> (module_p)->writer ());
//  if (!iset_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s:%s: failed to dynamic_cast<Common_ISetP_T<GdkWindow>*>(%@), returning\n"),
//                ACE_TEXT (stream_p->name ().c_str ()),
//                ACE_TEXT (module_name.c_str ()),
//                const_cast<Stream_Module_t*> (module_p)->writer ()));
//    return G_SOURCE_REMOVE;
//  } // end IF
//  try {
//    iset_p->setP (gtk_widget_get_window (GTK_WIDGET (drawing_area_p)));
//  } catch (...) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Common_ISetP_T::setP(), returning\n")));
//    return G_SOURCE_REMOVE;
//  }
//
////#if defined (ACE_WIN32) || defined (ACE_WIN64)
////  switch (cb_data_p->mediaFramework)
////  {
////    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
////      cb_data_p->audioStream->control (STREAM_CONTROL_RESIZE, false);
////      break;
////    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
////      mediafoundation_cb_data_p->audioStream->control (STREAM_CONTROL_RESIZE, false);
////      break;
////    default:
////    {
////      ACE_DEBUG ((LM_ERROR,
////                  ACE_TEXT ("%s: invalid/unkown media framework (was: %d), returning\n"),
////                  ACE_TEXT (stream_p->name ().c_str ()),
////                  cb_data_p->mediaFramework));
////      return G_SOURCE_REMOVE;
////    }
////  } // end SWITCH
////#else
////  cb_data_p->audioStream->control (STREAM_CONTROL_RESIZE, false);
////#endif // ACE_WIN32 || ACE_WIN64
//
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("window resized to %dx%d\n"),
//              allocation_s.width, allocation_s.height));
//
//  return G_SOURCE_REMOVE;
//} // drawingarea_audio_resize_end
//
//void
//drawingarea_audio_size_allocate_cb (GtkWidget* widget_in,
//                                    GdkRectangle* allocation_in,
//                                    gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::drawingarea_audio_size_allocate_cb"));
//
//  ACE_UNUSED_ARG (widget_in);
//  ACE_UNUSED_ARG (allocation_in);
//
//  static gint timer_id = 0;
//  if (timer_id == 0)
//  {
//    timer_id = g_timeout_add (300, drawingarea_audio_resize_end, userData_in);
//    return;
//  } // end IF
//  g_source_remove (timer_id);
//  timer_id = g_timeout_add (300, drawingarea_audio_resize_end, userData_in);
//} // drawingarea_audio_size_allocate_cb
//
//gboolean
//drawingarea_video_resize_end (gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::drawingarea_video_resize_end"));
//
//  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
//    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
//  ACE_ASSERT (cb_data_p);
//  Common_UI_GTK_BuildersIterator_t iterator =
//    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  // sanity check(s)
//  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());
//  GtkDrawingArea* drawing_area_p =
//    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
//                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_VIDEO_NAME)));
//  ACE_ASSERT (drawing_area_p);
//
//  GtkAllocation allocation_s;
//  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
//                             &allocation_s);
//
//  Stream_IStream_t* stream_p = NULL;
//  const Stream_Module_t* module_p = NULL;
//  std::string module_name;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  struct Test_I_ExtractStream_UI_CBData* cb_data_p = NULL;
//  Test_I_ExtractStream_StreamConfiguration_t::ITERATOR_T stream_iterator;
//  struct Test_I_ExtractStream_MediaFoundation_UI_CBData* mediafoundation_cb_data_p =
//    NULL;
//  Test_I_ExtractStream_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
//  switch (cb_data_p->mediaFramework)
//  {
//    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//    {
//      cb_data_p =
//        static_cast<struct Test_I_ExtractStream_UI_CBData*> (cb_data_p);
//      stream_p = cb_data_p->videoStream;
//      ACE_ASSERT (cb_data_p->configuration);
//      stream_iterator =
//        cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING));
//      ACE_ASSERT (stream_iterator != cb_data_p->configuration->streamConfiguration.end ());
//
//      Common_Image_Resolution_t resolution_s;
//      resolution_s.cx = allocation_s.width;
//      resolution_s.cy = allocation_s.height;
//      Stream_MediaFramework_DirectShow_Tools::setResolution (resolution_s,
//                                                             (*stream_iterator).second.second->outputFormat);
//
//      if (!cb_data_p->videoStream->isRunning ())
//        return G_SOURCE_REMOVE;
//
//      module_name =
//        ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING);
//      break;
//    }
//    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//    {
//      mediafoundation_cb_data_p =
//        static_cast<struct Test_I_ExtractStream_MediaFoundation_UI_CBData*> (cb_data_p);
//      stream_p = mediafoundation_cb_data_p->videoStream;
//      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
//      mediafoundation_stream_iterator =
//        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING));
//      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
//
//      HRESULT result_2 =
//        MFSetAttributeSize (const_cast<IMFMediaType*> ((*mediafoundation_stream_iterator).second.second->outputFormat),
//                            MF_MT_FRAME_SIZE,
//                            static_cast<UINT32> (allocation_s.width), static_cast<UINT32> (allocation_s.height));
//      ACE_ASSERT (SUCCEEDED (result_2));
//
//      if (!mediafoundation_cb_data_p->videoStream->isRunning ())
//        return G_SOURCE_REMOVE;
//
//      module_name =
//        ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING);
//      break;
//    }
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
//                  cb_data_p->mediaFramework));
//      return G_SOURCE_REMOVE;
//    }
//  } // end SWITCH
//#else
//  struct Test_I_ExtractStream_V4L_UI_CBData* cb_data_p =
//    static_cast<struct Test_I_ExtractStream_V4L_UI_CBData*> (cb_data_p);
//  stream_p = cb_data_p->videoStream;
//  ACE_ASSERT (cb_data_p->configuration);
//  Test_I_ExtractStream_ALSA_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
//    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
//  Test_I_ExtractStream_ALSA_V4L_StreamConfiguration_t::ITERATOR_T iterator_3 =
//#if defined (GTK_USE)
//    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING));
//#else
//    cb_data_p->configuration->streamConfiguration.find (Stream_Visualization_Tools::rendererToModuleName (STREAM_VISUALIZATION_VIDEORENDERER_X11));
//#endif // GTK_USE
//  ACE_ASSERT (iterator_3 != cb_data_p->configuration->streamConfiguration.end ());
//
////  (*iterator_2).second.second->outputFormat.resolution.height =
////      allocation_in->height;
////  (*iterator_2).second.second->outputFormat.resolution.width =
////      allocation_in->width;
//  (*iterator_3).second.second->outputFormat.video.format.height =
//      allocation_s.height;
//  (*iterator_3).second.second->outputFormat.video.format.width =
//      allocation_s.width;
//
//  if (!cb_data_p->videoStream->isRunning ())
//    return G_SOURCE_REMOVE;
//
//  module_name =
//    ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING);
//#endif // ACE_WIN32 || ACE_WIN64
//  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());
//  ACE_ASSERT (stream_p);
//
//  // *NOTE*: two things need doing:
//  //         - drop inbound frames until the 'resize' session message is through
//  //         - enqueue a 'resize' session message
//
//  // step1:
//  module_p = stream_p->find (module_name);
//  if (!module_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to Stream_IStream::find(\"%s\"), returning\n"),
//                ACE_TEXT (stream_p->name ().c_str ()),
//                ACE_TEXT (module_name.c_str ())));
//    return G_SOURCE_REMOVE;
//  } // end IF
//  Stream_Visualization_IResize* iresize_p =
//    dynamic_cast<Stream_Visualization_IResize*> (const_cast<Stream_Module_t*> (module_p)->writer ());
//  if (!iresize_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s:%s: failed to dynamic_cast<Stream_Visualization_IResize*>(%@), returning\n"),
//                ACE_TEXT (stream_p->name ().c_str ()),
//                ACE_TEXT (module_name.c_str ()),
//                const_cast<Stream_Module_t*> (module_p)->writer ()));
//    return G_SOURCE_REMOVE;
//  } // end IF
//  try {
//    iresize_p->resizing ();
//  } catch (...) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Stream_Visualization_IResize::resizing(), returning\n")));
//    return G_SOURCE_REMOVE;
//  }
//
//  // step2
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  switch (cb_data_p->mediaFramework)
//  {
//    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//      cb_data_p->videoStream->control (STREAM_CONTROL_RESIZE, false);
//      break;
//    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//      mediafoundation_cb_data_p->videoStream->control (STREAM_CONTROL_RESIZE,
//                                                       false);
//      break;
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: invalid/unkown media framework (was: %d), returning\n"),
//                  ACE_TEXT (stream_p->name ().c_str ()),
//                  cb_data_p->mediaFramework));
//      return G_SOURCE_REMOVE;
//    }
//  } // end SWITCH
//#else
//  cb_data_p->videoStream->control (STREAM_CONTROL_RESIZE, false);
//#endif // ACE_WIN32 || ACE_WIN64
//
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("window resized to %dx%d\n"),
//              allocation_s.width, allocation_s.height));
//
//  return G_SOURCE_REMOVE;
//} // drawingarea_video_resize_end
//
//void
//drawingarea_video_size_allocate_cb (GtkWidget* widget_in,
//                                    GdkRectangle* allocation_in,
//                                    gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::drawingarea_video_size_allocate_cb"));
//
//  ACE_UNUSED_ARG (widget_in);
//  ACE_UNUSED_ARG (allocation_in);
//
//  static gint timer_id_2 = 0;
//  if (timer_id_2 == 0)
//  {
//    timer_id_2 = g_timeout_add (300, drawingarea_video_resize_end, userData_in);
//    return;
//  } // end IF
//  g_source_remove (timer_id_2);
//  timer_id_2 = g_timeout_add (300, drawingarea_video_resize_end, userData_in);
//} // drawingarea_video_size_allocate_cb

void
filechooserbutton_source_file_set_cb (GtkFileChooserButton* fileChooserButton_in,
                                      gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_source_file_set_cb"));

  // sanity check(s)
  ACE_ASSERT (fileChooserButton_in);
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
    static_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());
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
  std::string filename_string = string_p;
    //Common_UI_GTK_Tools::UTF8ToLocale (string_p, -1);
  if (filename_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF8ToLocale(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (string_p)));
    g_free (string_p); string_p = NULL;
    return;
  } // end IF
  g_free (string_p); string_p = NULL;

  // populate stream combobox
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_STREAM_NAME)));
  ACE_ASSERT (list_store_p);
  load_media_streams (filename_string,
                      list_store_p);
} // filechooserbutton_source_file_set_cb

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
  STREAM_TRACE (ACE_TEXT ("::key_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  ACE_ASSERT (eventKey_in);

  // sanity check(s)
  struct Test_I_ExtractStream_UI_CBData* cb_data_p =
      reinterpret_cast<struct Test_I_ExtractStream_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  switch (eventKey_in->keyval)
  {
#if GTK_CHECK_VERSION (3,0,0)
    case GDK_KEY_Escape:
    case GDK_KEY_f:
    case GDK_KEY_F:
#else
    case GDK_Escape:
    case GDK_f:
    case GDK_F:
#endif // GTK_CHECK_VERSION (3,0,0)
    {
      bool is_active_b = false;
      GtkToggleButton* toggle_button_p =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_FULLSCREEN_NAME)));
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
} // key_cb

//gboolean
//drawingarea_audio_key_press_event_cb (GtkWidget* widget_in,
//                                      GdkEventKey* eventKey_in,
//                                      gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::drawingarea_audio_key_press_event_cb"));
//
//  return key_cb (widget_in, eventKey_in, userData_in);
//} // drawingarea_audio_key_press_event_cb

gboolean
dialog_main_key_press_event_cb (GtkWidget* widget_in,
                                GdkEventKey* eventKey_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::dialog_main_key_press_event_cb"));

  return key_cb (widget_in, eventKey_in, userData_in);
} // dialog_main_key_press_event_cb

#ifdef __cplusplus
}
#endif /* __cplusplus */
