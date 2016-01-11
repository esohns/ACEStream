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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "dshow.h"
#include "Dvdmedia.h"
 //#include "streams.h"

#include "gdk/gdkwin32.h"
#endif

#include "glade/glade.h"

#include "common_file_tools.h"
#include "common_timer_manager.h"

#include "common_ui_common.h"
#include "common_ui_defines.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_tools.h"

#include "stream_macros.h"

#include "stream_file_defines.h"

#include "test_i_common.h"
#include "test_i_defines.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

#include "test_i_source_common.h"
#include "test_i_target_listener_common.h"

bool
load_capture_devices (GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_capture_devices"));

  bool result = false;

  // initialize result
  gtk_list_store_clear (listStore_in);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  ICreateDevEnum* enumerator_p = NULL;
  result_2 =
    CoCreateInstance (CLSID_SystemDeviceEnum, NULL,
                      CLSCTX_INPROC_SERVER, IID_PPV_ARGS (&enumerator_p));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_SystemDeviceEnum): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);

  IEnumMoniker* enum_moniker_p = NULL;
  result_2 =
    enumerator_p->CreateClassEnumerator (CLSID_VideoInputDeviceCategory,
                                         &enum_moniker_p,
                                         0);
  if (result_2 != S_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICreateDevEnum::CreateClassEnumerator(CLSID_VideoInputDeviceCategory): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    //result_2 = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
    goto error;
  } // end IF
  ACE_ASSERT (enum_moniker_p);

  IMoniker* moniker_p = NULL;
  IPropertyBag* properties_p = NULL;
  VARIANT variant;
  GtkTreeIter iterator;
  while (enum_moniker_p->Next (1, &moniker_p, NULL) == S_OK)
  {
    ACE_ASSERT (moniker_p);

    properties_p = NULL;
    result = moniker_p->BindToStorage (0, 0, IID_PPV_ARGS (&properties_p));
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMoniker::BindToStorage(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (properties_p);

    VariantInit (&variant);
    result_2 = properties_p->Read (L"FriendlyName", &variant, 0);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPropertyBag::Read(Description/FriendlyName): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      goto error;
    } // end IF
    properties_p->Release ();
    properties_p = NULL;
    ACE_Wide_To_Ascii converter (variant.bstrVal);
    VariantClear (&variant);
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.char_rep (),
                        -1);

    moniker_p->Release ();
    moniker_p = NULL;
  } // end WHILE
  result = true;

error:
  if (properties_p)
    properties_p->Release ();
  if (moniker_p)
    moniker_p->Release ();
  if (enum_moniker_p)
    enum_moniker_p->Release ();
  if (enumerator_p)
    enumerator_p->Release ();
#else
#endif

  return result;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct less_guid
{
  bool operator() (const GUID& lhs_in, const GUID& rhs_in) const
  {
    //ACE_ASSERT (lhs_in.Data2 == rhs_in.Data2);
    //ACE_ASSERT (lhs_in.Data3 == rhs_in.Data3);
    //ACE_ASSERT (*(long long*)lhs_in.Data4 == *(long long*)rhs_in.Data4);

    return (lhs_in.Data1 < rhs_in.Data1);
  }
};

bool
load_formats (IAMStreamConfig* IAMStreamConfig_in,
              GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_formats"));

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result;
  int count = 0, size = 0;
  std::set<GUID, less_guid> GUIDs;
  std::string media_subtype_string;
  std::string GUID_stdstring;
  result = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  AM_MEDIA_TYPE* media_type_p = NULL;
  struct _VIDEO_STREAM_CONFIG_CAPS capabilities;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  for (int i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result = IAMStreamConfig_in->GetStreamCaps (i,
                                                &media_type_p,
                                                (BYTE*)&capabilities);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);
    if ((media_type_p->formattype != FORMAT_VideoInfo) &&
        (media_type_p->formattype != FORMAT_VideoInfo2))
    {
      //DeleteMediaType (media_type_p);
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF

      // *NOTE*: FORMAT_VideoInfo2 types do not work with the Video Renderer
      //         directly --> insert the Overlay Mixer
    GUIDs.insert (media_type_p->subtype);

    //DeleteMediaType (media_type_p);
    Stream_Module_Device_Tools::deleteMediaType (media_type_p);
  } // end FOR

  GtkTreeIter iterator;
  OLECHAR GUID_string[39];
  ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
  for (std::set<GUID, less_guid>::const_iterator iterator_2 = GUIDs.begin ();
  iterator_2 != GUIDs.end ();
    ++iterator_2)
  {
    count = StringFromGUID2 (*iterator_2,
                             GUID_string, sizeof (GUID_string));
    ACE_ASSERT (count == 39);
    ACE_Wide_To_Ascii converter (GUID_string);
    GUID_stdstring = converter.char_rep ();
    gtk_list_store_append (listStore_in, &iterator);
    media_subtype_string =
      Stream_Module_Device_Tools::mediaSubTypeToString (*iterator_2);
    gtk_list_store_set (listStore_in, &iterator,
                        0, media_subtype_string.c_str (),
                        1, GUID_stdstring.c_str (),
                        -1);
  } // end FOR

  return true;
}

bool
load_resolutions (IAMStreamConfig* IAMStreamConfig_in,
                  const GUID& mediaSubType_in,
                  GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_resolutions"));

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result;
  int count = 0, size = 0;
  result = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  AM_MEDIA_TYPE* media_type_p = NULL;
  struct _VIDEO_STREAM_CONFIG_CAPS capabilities;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  std::set<std::pair<unsigned int, unsigned int> > resolutions;
  for (int i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result = IAMStreamConfig_in->GetStreamCaps (i,
                                                &media_type_p,
                                                (BYTE*)&capabilities);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (media_type_p->subtype != mediaSubType_in)
    {
      //DeleteMediaType (media_type_p);
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF
    if (media_type_p->formattype == FORMAT_VideoInfo)
    {
      video_info_header_p = (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
      resolutions.insert (std::make_pair (video_info_header_p->bmiHeader.biWidth,
                                          video_info_header_p->bmiHeader.biHeight));
    } // end IF
    else if (media_type_p->formattype == FORMAT_VideoInfo2)
    {
      // *NOTE*: these media subtypes do not work with the Video Renderer
      //         directly --> insert the Overlay Mixer
      video_info_header2_p = (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
      resolutions.insert (std::make_pair (video_info_header_p->bmiHeader.biWidth,
                                          video_info_header_p->bmiHeader.biHeight));
    } // end ELSE IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid AM_MEDIA_TYPE, aborting\n")));

      // clean up
      //DeleteMediaType (media_type_p);
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);

      return false;
    } // end ELSE
      //DeleteMediaType (media_type_p);
    Stream_Module_Device_Tools::deleteMediaType (media_type_p);
  } // end WHILE
  GtkTreeIter iterator;
  std::ostringstream converter;
  for (std::set<std::pair<unsigned int, unsigned int> >::const_iterator iterator_2 = resolutions.begin ();
  iterator_2 != resolutions.end ();
    ++iterator_2)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (*iterator_2).first;
    converter << 'x';
    converter << (*iterator_2).second;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, (*iterator_2).first,
                        2, (*iterator_2).second,
                        -1);
  } // end FOR

  return true;
}

bool
load_rates (IAMStreamConfig* IAMStreamConfig_in,
            const GUID& mediaSubType_in,
            unsigned int width_in,
            GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_rates"));

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result;
  int count = 0, size = 0;
  result = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  AM_MEDIA_TYPE* media_type_p = NULL;
  struct _VIDEO_STREAM_CONFIG_CAPS capabilities;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  unsigned int frame_duration;
  std::set<std::pair<unsigned int, unsigned int> > frame_rates;
  for (int i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result = IAMStreamConfig_in->GetStreamCaps (i,
                                                &media_type_p,
                                                (BYTE*)&capabilities);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (media_type_p->subtype != mediaSubType_in)
    {
      //DeleteMediaType (media_type_p);
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF
    if (media_type_p->formattype == FORMAT_VideoInfo)
    {
      video_info_header_p = (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
      if (video_info_header_p->bmiHeader.biWidth != width_in)
      {
        //DeleteMediaType (media_type_p);
        Stream_Module_Device_Tools::deleteMediaType (media_type_p);
        continue;
      } // end IF
      else
        frame_duration =
        static_cast<unsigned int> (video_info_header_p->AvgTimePerFrame);
    } // end IF
    else if (media_type_p->formattype == FORMAT_VideoInfo2)
    {
      video_info_header2_p = (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
      if (video_info_header2_p->bmiHeader.biWidth != width_in)
      {
        //DeleteMediaType (media_type_p);
        Stream_Module_Device_Tools::deleteMediaType (media_type_p);
        continue;
      } // end IF
      else
        frame_duration =
        static_cast<unsigned int> (video_info_header_p->AvgTimePerFrame);
    } // end ELSEIF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid AM_MEDIA_TYPE, aborting\n")));

      // clean up
      //DeleteMediaType (media_type_p);
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);

      return false;
    } // end IF
    frame_rates.insert (std::make_pair (10000000 / static_cast<unsigned int> (capabilities.MinFrameInterval),
                                        frame_duration));
    //DeleteMediaType (media_type_p);
    Stream_Module_Device_Tools::deleteMediaType (media_type_p);
  } // end WHILE
  GtkTreeIter iterator;
  for (std::set<std::pair<unsigned int, unsigned int> >::const_iterator iterator_2 = frame_rates.begin ();
  iterator_2 != frame_rates.end ();
    ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, (*iterator_2).first,
                        1, (*iterator_2).second,
                        -1);
  } // end FOR

  return true;
}
#endif

/////////////////////////////////////////

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

  Test_I_Source_ThreadData* data_p =
    static_cast<Test_I_Source_ThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->CBData);
  ACE_ASSERT (data_p->CBData->configuration);
  ACE_ASSERT (data_p->CBData->stream);

  GtkSpinButton* spin_button_p = NULL;
  GtkStatusbar* statusbar_p = NULL;
  Test_I_Source_StreamBase_t* stream_p = NULL;
  std::ostringstream converter;
  const Test_I_Source_Stream_SessionData_t* session_data_container_p = NULL;
  const Test_I_Source_Stream_SessionData* session_data_p = NULL;

  gdk_threads_enter ();
  bool leave_gdk = true;

  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->CBData->lock);

    Common_UI_GTKBuildersIterator_t iterator =
        data_p->CBData->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
    // sanity check(s)
    ACE_ASSERT (iterator != data_p->CBData->builders.end ());

    // retrieve stream handle
    switch (data_p->CBData->configuration->protocol)
    {
      case NET_TRANSPORTLAYER_TCP:
        stream_p = data_p->CBData->stream;
        data_p->CBData->configuration->moduleHandlerConfiguration.stream =
          stream_p;
        break;
      case NET_TRANSPORTLAYER_UDP:
        stream_p = data_p->CBData->UDPStream;
        data_p->CBData->configuration->moduleHandlerConfiguration.stream =
          stream_p;
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown protocol (was: %d), returning\n"),
                    data_p->CBData->configuration->protocol));
        goto done;
      }
    } // end SWITCH
    ACE_ASSERT (stream_p);

    // retrieve status bar handle
    statusbar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_STATUSBAR_NAME)));
    ACE_ASSERT (statusbar_p);
  } // end lock scope

  gdk_threads_leave ();
  leave_gdk = false;

  if (!stream_p->initialize (data_p->CBData->configuration->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream: \"%m\", aborting\n")));
    goto done;
  } // end IF
  session_data_container_p = stream_p->get ();
  ACE_ASSERT (session_data_container_p);
  session_data_p = &session_data_container_p->get ();
  ACE_ASSERT (session_data_p);
  converter.clear ();
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter << session_data_p->sessionID;

  // generate context ID
  gdk_threads_enter ();
  data_p->CBData->configuration->moduleHandlerConfiguration.contextID =
    gtk_statusbar_get_context_id (statusbar_p,
                                  converter.str ().c_str ());
  gdk_threads_leave ();

  // *NOTE*: processing currently happens 'inline' (borrows calling thread)
  stream_p->start ();
  //    if (!stream_p->isRunning ())
  //    {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to start stream, aborting\n")));
  //      return;
  //    } // end IF
  stream_p->waitForCompletion (true,
                               false);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif

done:
  if (leave_gdk)
    gdk_threads_leave ();

  { // synch access
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->CBData->lock);
    data_p->CBData->progressData.completedActions.insert (data_p->eventSourceID);
  } // end lock scope

  // clean up
  delete data_p;

  return result;
}

/////////////////////////////////////////

gboolean
idle_initialize_source_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_source_UI_cb"));

  Test_I_Source_GTK_CBData* data_p =
    static_cast<Test_I_Source_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  //// sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT hresult = CoInitializeEx (NULL, COINIT_MULTITHREADED);
  if (FAILED (hresult))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (hresult).c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
#endif

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
                             std::numeric_limits<double>::max ());
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
  if (!load_capture_devices (list_store_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_capture_devices(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
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
                                  //"cell-background", 0,
                                  //"text", 1,
                                  "text", 0,
                                  NULL);
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
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
                                  //"cell-background", 0,
                                  //"text", 1,
                                  "text", 0,
                                  NULL);
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_ASCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
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
                                  //"cell-background", 0,
                                  //"text", 1,
                                  "text", 0,
                                  NULL);
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_RATE_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_RATE_NAME)));
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
                                  //"cell-background", 0,
                                  //"text", 1,
                                  "text", 0,
                                  NULL);

  GtkEntry* entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ENTRY_DESTINATION_NAME)));
  ACE_ASSERT (entry_p);
//    ACE_TCHAR buffer[BUFSIZ];
//    ACE_OS::memset (buffer, 0, sizeof (buffer));
//    int result =
//      data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.peerAddress.addr_to_string (buffer,
//                                                                                                          sizeof (buffer),
//                                                                                                          0);
//    if (result == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", aborting\n")));
//      return G_SOURCE_REMOVE;
//    } // end IF
//    gtk_entry_set_text (entry_p, buffer);
  gtk_entry_set_text (entry_p,
                      data_p->configuration->socketConfiguration.peerAddress.get_host_name ());

  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                              static_cast<double> (data_p->configuration->socketConfiguration.peerAddress.get_port_number ()));

  GtkRadioButton* radio_button_p = NULL;
  if (data_p->configuration->protocol == NET_TRANSPORTLAYER_UDP)
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
                                !data_p->configuration->useReactor);
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_CHECKBUTTON_LOOPBACK_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                data_p->configuration->socketConfiguration.useLoopBackDevice);

  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<double> (data_p->configuration->streamConfiguration.bufferSize));

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
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
  Test_I_GTK_CBData* cb_data_p = data_p;
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

    // schedule asynchronous updates of the log view
    guint event_source_id = g_timeout_add_seconds (1,
                                                   idle_update_log_display_cb,
                                                   cb_data_p);
    if (event_source_id > 0)
      data_p->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
    // schedule asynchronous updates of the info view
    event_source_id = g_timeout_add (TEST_I_STREAM_UI_GTKEVENT_RESOLUTION,
                                     idle_update_info_display_cb,
                                     cb_data_p);
    if (event_source_id > 0)
      data_p->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step6: disable some functions ?

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

  // step6b: connect custom signals
  //gtk_builder_connect_signals ((*iterator).second.second,
  //                             userData_in);

  GObject* object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_STREAM_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("toggled"),
                               G_CALLBACK (toggleaction_stream_toggled_cb),
                               userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_SETTINGS_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("activate"),
                      G_CALLBACK (action_settings_activate_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_RESET_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("activate"),
                      G_CALLBACK (action_reset_activate_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("changed"),
                      G_CALLBACK (combobox_source_changed_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("changed"),
                      G_CALLBACK (combobox_format_changed_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("changed"),
                      G_CALLBACK (combobox_resolution_changed_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_RATE_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("changed"),
                      G_CALLBACK (combobox_rate_changed_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("value-changed"),
  //                             G_CALLBACK (spinbutton_port_value_changed_cb),
  //                             cb_data_p);
  //ACE_ASSERT (result_2);
  //
  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_TCP_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
  //                             G_CALLBACK (togglebutton_protocol_toggled_cb),
  //                             cb_data_p);
  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_UDP_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
  //                             G_CALLBACK (togglebutton_protocol_toggled_cb),
  //                             cb_data_p);

  //-------------------------------------

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
                      cb_data_p);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_ABOUT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_about_clicked_cb),
                        cb_data_p);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_QUIT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_quit_clicked_cb),
                        cb_data_p);
  ACE_ASSERT (result_2);
  ACE_UNUSED_ARG (result_2);

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  gtk_widget_show_all (dialog_p);

//  // step10a: retrieve window handle
//  GdkWindow* window_p = gtk_widget_get_window (GTK_WIDGET (dialog_p));
//  ACE_ASSERT (window_p);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  data_p->configuration->moduleHandlerConfiguration.window =
//    gdk_win32_window_get_impl_hwnd (window_p);
//#else
//#endif

  // step10: select default capture source (if any)
  //         --> populate the option-comboboxes
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF
  else
  {
    GtkToggleAction* toggle_action_p =
      GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_STREAM_NAME)));
    ACE_ASSERT (toggle_action_p);
    gtk_action_set_sensitive (GTK_ACTION (toggle_action_p), false);
  } // end IF

  return G_SOURCE_REMOVE;
}

gboolean
idle_end_source_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_end_source_UI_cb"));

  Test_I_Source_GTK_CBData* data_p =
    static_cast<Test_I_Source_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  // synch access
  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  //ACE_ASSERT (iterator != CBData_->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

  GtkToggleAction* toggle_action_p =
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_STREAM_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_action_set_stock_id (GTK_ACTION (toggle_action_p), GTK_STOCK_MEDIA_PLAY);

  GtkAction* action_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_SETTINGS_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, true);
  action_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_RESET_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, true);

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_progress_source_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_progress_source_cb"));

  Stream_GTK_ProgressData* data_p =
      static_cast<Stream_GTK_ProgressData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->GTKState);

  // synch access
  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->GTKState->lock);

  int result = -1;
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

  // step1: join completed thread(s)
  ACE_THR_FUNC_RETURN exit_status;
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  Stream_PendingActionsIterator_t iterator_2;
  for (Stream_CompletedActionsIterator_t iterator_3 = data_p->completedActions.begin ();
       iterator_3 != data_p->completedActions.end ();
       ++iterator_3)
  {
    iterator_2 = data_p->pendingActions.find (*iterator_3);
    ACE_ASSERT (iterator_2 != data_p->pendingActions.end ());
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

    data_p->GTKState->eventSourceIds.erase (*iterator_3);
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
    //    return FALSE; // G_SOURCE_REMOVE
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

  // step2: update progress bar text
  std::ostringstream converter;
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->GTKState->lock);

    converter << data_p->statistic.messagesPerSecond;
  } // end lock scope
  converter << ACE_TEXT_ALWAYS_CHAR (" fps");
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_text (progress_bar_p,
                             (done ? ACE_TEXT_ALWAYS_CHAR ("")
                                   : ACE_TEXT_ALWAYS_CHAR (converter.str ().c_str ())));
  if (done)
  {
    gtk_progress_bar_set_fraction (progress_bar_p, 0.0);
    gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), false);
  } // end IF
  else
    gtk_progress_bar_pulse (progress_bar_p);

  // --> reschedule
  return (done ? G_SOURCE_REMOVE : G_SOURCE_CONTINUE);
}

/////////////////////////////////////////

gboolean
idle_initialize_target_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_target_UI_cb"));

  Test_I_Target_GTK_CBData* data_p =
    static_cast<Test_I_Target_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  //// sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT hresult = CoInitializeEx (NULL, COINIT_MULTITHREADED);
  if (FAILED (hresult))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (hresult).c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
#endif

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
                             std::numeric_limits<double>::max ());
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  GFile* file_p = NULL;
  std::string directory, file_name;
  directory = data_p->configuration->moduleHandlerConfiguration.targetFileName;
  file_name = data_p->configuration->moduleHandlerConfiguration.targetFileName;
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
    file_name =
      ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_DEFAULT_OUTPUT_FILENAME);
  else if (Common_File_Tools::isValidFilename (file_name))
    file_name =
      ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (file_name.c_str ())));
  file_name = directory +
              ACE_DIRECTORY_SEPARATOR_CHAR_A +
              file_name;
  ACE_ASSERT (Common_File_Tools::isValidFilename (file_name));
  file_p =
    g_file_new_for_path (file_name.c_str ());
  ACE_ASSERT (file_p);
  //GFile* file_2 = g_file_get_parent (file_p);
  //if (!file_2)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to g_file_get_parent(), aborting\n")));

  //  // clean up
  //  g_object_unref (file_p);

  //  return G_SOURCE_REMOVE;
  //} // end IF
  //g_object_unref (file_p);
  //char* string_p = g_file_get_path (file_2);
  char* string_p = g_file_get_path (file_p);
  if (!string_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_file_get_path(), aborting\n")));

    // clean up
    //g_object_unref (file_2);
    g_object_unref (file_p);

    return G_SOURCE_REMOVE;
  } // end IF
  //g_object_unref (file_2);
  g_object_unref (file_p);
  //if (!gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
  if (!gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
                                         string_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_filename(), aborting\n")));

    // clean up
    g_free (string_p);

    return G_SOURCE_REMOVE;
  } // end IF
  g_free (string_p);

  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<double> (data_p->configuration->socketConfiguration.peerAddress.get_port_number ()));

  GtkRadioButton* radio_button_p = NULL;
  if (data_p->configuration->protocol == NET_TRANSPORTLAYER_UDP)
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
                                !data_p->configuration->useReactor);
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_CHECKBUTTON_LOOPBACK_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                data_p->configuration->socketConfiguration.useLoopBackDevice);

  spin_button_p =
      //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
      //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  gtk_spin_button_set_value (spin_button_p,
                              static_cast<double> (data_p->configuration->streamConfiguration.bufferSize));

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
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
  gtk_rc_style_unref (rc_style_p);

  //  GtkTextIter iterator;
  //  gtk_text_buffer_get_end_iter (buffer_p,
  //                                &iterator);
  //  gtk_text_buffer_create_mark (buffer_p,
  //                               ACE_TEXT_ALWAYS_CHAR (NET_UI_SCROLLMARK_NAME),
  //                               &iterator,
  //                               TRUE);
  //  g_object_unref (buffer_p);

  GtkDrawingArea* drawing_area_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);

  // step5: initialize updates
  guint event_source_id = 0;
  Test_I_GTK_CBData* cb_data_p = data_p;
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

    // schedule asynchronous updates of the log view
    event_source_id = g_timeout_add_seconds (1,
                                             idle_update_log_display_cb,
                                             cb_data_p);
    if (event_source_id > 0)
      data_p->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE

    // schedule asynchronous updates of the info view
    event_source_id = g_timeout_add (TEST_I_STREAM_UI_GTKEVENT_RESOLUTION,
                                     idle_update_info_display_cb,
                                     cb_data_p);
    if (event_source_id > 0)
      data_p->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step6: (auto-)connect signals/slots
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
                      G_CALLBACK (filechooserbutton_target_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  result_2 =
    g_signal_connect (GTK_FILE_CHOOSER (file_chooser_button_p),
                      //ACE_TEXT_ALWAYS_CHAR ("current-folder-changed"),
                      ACE_TEXT_ALWAYS_CHAR ("selection-changed"),
                      G_CALLBACK (filechooser_target_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  GtkFileChooserDialog* file_chooser_dialog_p =
    GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME)));
  ACE_ASSERT (file_chooser_dialog_p);
  result_2 =
    g_signal_connect (file_chooser_dialog_p,
                      ACE_TEXT_ALWAYS_CHAR ("file-activated"),
                      G_CALLBACK (filechooserdialog_target_cb),
                      NULL);
  ACE_ASSERT (result_2);

  // step6b: connect custom signals
  //gtk_builder_connect_signals ((*iterator).second.second,
  //                             userData_in);

  GObject* object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_TCP_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
  //                             G_CALLBACK (togglebutton_protocol_toggled_cb),
  //                             cb_data_p);
  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_UDP_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
  //                             G_CALLBACK (togglebutton_protocol_toggled_cb),
  //                             cb_data_p);

  //object_p =
  //    gtk_builder_get_object ((*iterator).second.second,
  //                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                           ACE_TEXT_ALWAYS_CHAR ("value-changed"),
  //                           G_CALLBACK (spinbutton_port_value_changed_cb),
  //                           cb_data_p);
  //ACE_ASSERT (result_2);
  //object_p =
  //    gtk_builder_get_object ((*iterator).second.second,
  //                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_STOP_NAME));
  //ACE_ASSERT (object_p);
  //result_2 =
  //    g_signal_connect (object_p,
  //                      ACE_TEXT_ALWAYS_CHAR ("clicked"),
  //                      G_CALLBACK (button_stop_clicked_cb),
  //                      userData_in);
  //ACE_ASSERT (result_2);
  //object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_LISTEN_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("toggled"),
                      G_CALLBACK (toggleaction_listen_activate_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_CLOSE_ALL_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("activate"),
                               G_CALLBACK (action_close_all_activate_cb),
                               userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_REPORT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("activate"),
                      G_CALLBACK (action_report_activate_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR(TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                      G_CALLBACK (textview_size_allocate_cb),
                      cb_data_p);
  ACE_ASSERT (result_2);

  //-------------------------------------

  result_2 =
    g_signal_connect (G_OBJECT (drawing_area_p),
                      ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                      G_CALLBACK (drawingarea_configure_event_cb),
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
                      cb_data_p);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_ABOUT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_about_clicked_cb),
                        cb_data_p);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_QUIT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_quit_clicked_cb),
                        cb_data_p);
  ACE_ASSERT (result_2);
  ACE_UNUSED_ARG (result_2);

  // step7: set defaults
  GtkToggleAction* toggle_action_p =
    //GTK_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_LISTEN_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_action_activate (GTK_ACTION (toggle_action_p));

  //file_chooser_button_p =
  //  GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  std::string default_folder_uri = ACE_TEXT_ALWAYS_CHAR ("file://");
  default_folder_uri +=
    data_p->configuration->moduleHandlerConfiguration.targetFileName;
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

  // step10: retrieve window handle (canvas coordinates)
  GdkWindow* window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  data_p->configuration->moduleHandlerConfiguration.window =
    gdk_win32_window_get_impl_hwnd (window_p);
#else
#endif
  GtkAllocation allocation;
  ACE_OS::memset (&allocation, 0, sizeof (allocation));
  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
                             &allocation);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  data_p->configuration->moduleHandlerConfiguration.area.bottom =
    allocation.height;
  data_p->configuration->moduleHandlerConfiguration.area.left =
    allocation.x;
  data_p->configuration->moduleHandlerConfiguration.area.right =
    allocation.width;
  data_p->configuration->moduleHandlerConfiguration.area.top =
    allocation.y;
#else
  data_p->configuration->moduleHandlerConfiguration.area =
    allocation;
#endif

  return G_SOURCE_REMOVE;
}

gboolean
idle_start_target_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_start_target_UI_cb"));

  Test_I_Target_GTK_CBData* data_p =
    static_cast<Test_I_Target_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  //ACE_ASSERT (iterator != CBData_->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

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

  Test_I_Target_GTK_CBData* data_p =
    static_cast<Test_I_Target_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  //ACE_ASSERT (iterator != CBData_->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkAction* action_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_CLOSE_ALL_NAME)));
  ACE_ASSERT (action_p);
  Test_I_Target_InetConnectionManager_t* connection_manager_p =
    TEST_I_TARGET_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  gtk_action_set_sensitive (action_p,
                            (connection_manager_p->count () != 0));

  return G_SOURCE_REMOVE;
}

gboolean
idle_reset_target_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_reset_target_UI_cb"));

  Test_I_Target_GTK_CBData* data_p =
    static_cast<Test_I_Target_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  //ACE_ASSERT (iterator != CBData_->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

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

  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

    data_p->progressData.transferred = 0;
  } // end lock scope

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_progress_target_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_progress_target_cb"));

  Stream_GTK_ProgressData* data_p =
    static_cast<Stream_GTK_ProgressData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->GTKState);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result = -1;
  float speed = 0.0F;

  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->GTKState->lock);

    speed = data_p->statistic.bytesPerSecond;
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
idle_update_info_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  Test_I_GTK_CBData* data_p =
      static_cast<Test_I_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  if (data_p->eventStack.empty ())
    return G_SOURCE_CONTINUE;

  for (Stream_GTK_EventsIterator_t iterator_2 = data_p->eventStack.begin ();
       iterator_2 != data_p->eventStack.end ();
       iterator_2++)
  {
    switch (*iterator_2)
    {
      case STREAM_GTKEVENT_START:
      {
        //spin_button_p =
        //    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
        //                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
        //ACE_ASSERT (spin_button_p);
        //gtk_spin_button_set_value (spin_button_p, 0.0);
        //spin_button_p =
        //    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
        //                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
        //ACE_ASSERT (spin_button_p);
        //gtk_spin_button_set_value (spin_button_p, 0.0);
        //spin_button_p =
        //    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
        //                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
        //ACE_ASSERT (spin_button_p);
        //gtk_spin_button_set_value (spin_button_p, 0.0);

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
          //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
          //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
        ACE_ASSERT (spin_button_p);

        is_session_message = true;
        break;
      }
      case STREAM_GTKEVENT_DATA:
      {
        spin_button_p =
          //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
          //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
        ACE_ASSERT (spin_button_p);
        gtk_spin_button_set_value (spin_button_p,
                                   static_cast<gdouble> (data_p->progressData.transferred));

        spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
        ACE_ASSERT (spin_button_p);

        break;
      }
      case STREAM_GTKEVENT_END:
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
          //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
          //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
        ACE_ASSERT (spin_button_p);

        is_session_message = true;
        break;
      }
      case STREAM_GTKEVENT_STATISTIC:
      {
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
                    *iterator_2));
        break;
      }
    } // end SWITCH
    ACE_UNUSED_ARG (is_session_message);
    gtk_spin_button_spin (spin_button_p,
                          GTK_SPIN_STEP_FORWARD,
                          1.0);
  } // end FOR

  data_p->eventStack.clear ();

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_log_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));

  Test_I_GTK_CBData* data_p =
      static_cast<Test_I_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
      data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

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
  if (data_p->logStack.empty ())
    return G_SOURCE_CONTINUE;

  // step1: convert text
  for (Common_MessageStackConstIterator_t iterator_2 = data_p->logStack.begin ();
       iterator_2 != data_p->logStack.end ();
       iterator_2++)
  {
    string_p = Common_UI_Tools::Locale2UTF8 (*iterator_2);
    if (!string_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to convert message text (was: \"%s\"), aborting\n"),
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

  data_p->logStack.clear ();

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
toggleaction_stream_toggled_cb (GtkToggleAction* toggleAction_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::toggleaction_stream_toggled_cb"));

  Test_I_Source_GTK_CBData* data_p =
    static_cast<Test_I_Source_GTK_CBData*> (userData_in);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->stream);
  ACE_ASSERT (data_p->UDPStream);
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  Test_I_Source_ThreadData* thread_data_p = NULL;
  ACE_thread_t thread_id;
  ACE_hthread_t thread_handle;
  ACE_TCHAR thread_name[BUFSIZ];
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;
  int result = -1;
  guint event_source_id = 0;

  Test_I_Source_StreamBase_t* stream_p = NULL;
  switch (data_p->configuration->protocol)
  {
    case NET_TRANSPORTLAYER_TCP:
      stream_p = data_p->stream;
      break;
    case NET_TRANSPORTLAYER_UDP:
      stream_p = data_p->UDPStream;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown protocol (was: %d), returning\n"),
                  data_p->configuration->protocol));
      return;
    }
  } // end SWITCH
  ACE_ASSERT (stream_p);

  // toggle play/pause ?
  Stream_StateMachine_ControlState status = stream_p->status ();
  if ((status == STREAM_STATE_RUNNING) ||
      (status == STREAM_STATE_PAUSED))
  {
    stream_p->stop (false, true);
    //if (!data_p->configuration->moduleHandlerConfiguration.active)
    //{
    //  ACE_ASSERT (!data_p->progressData.pendingActions.empty ());
    //  Stream_PendingActionsIterator_t iterator =
    //    data_p->progressData.pendingActions.begin ();
    //  if (status_r == STREAM_STATE_RUNNING)
    //    result = ACE_Thread::suspend ((*iterator).second.handle);
    //  else
    //    result = ACE_Thread::resume ((*iterator).second.handle);
    //  if (result == -1)
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("failed to ACE_Thread::suspend/resume(): \"%m\", continuing\n")));
    //} // end ELSE

    // step0: modify widgets
    if (status == STREAM_STATE_RUNNING) // <-- image is "stop"
      gtk_action_set_stock_id (GTK_ACTION (toggleAction_in), GTK_STOCK_MEDIA_PLAY);
    else // <-- image is "play"
      gtk_action_set_stock_id (GTK_ACTION (toggleAction_in), GTK_STOCK_MEDIA_STOP);

    return;
  } // end IF
  gtk_action_set_stock_id (GTK_ACTION (toggleAction_in), GTK_STOCK_MEDIA_STOP);

  // step0: modify widgets
  GtkSpinButton* spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                             0.0);
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                             0.0);
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                             0.0);

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), false);

  GtkAction* action_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_SETTINGS_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, false);
  action_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_RESET_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, false);

  // step1: set up progress reporting
  data_p->progressData.transferred = 0;
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_fraction (progress_bar_p, 0.0);

  // step2: update configuration
  // retrieve port number
  spin_button_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  unsigned short port_number =
    static_cast<unsigned short> (gtk_spin_button_get_value_as_int (spin_button_p));
  data_p->configuration->socketConfiguration.peerAddress.set_port_number (port_number,
                                                                          1);

  // retrieve protocol
  GtkRadioButton* radio_button_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_TCP_NAME)));
  ACE_ASSERT (radio_button_p);
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio_button_p)))
    data_p->configuration->protocol = NET_TRANSPORTLAYER_TCP;
  else
    data_p->configuration->protocol = NET_TRANSPORTLAYER_UDP;

  // retrieve buffer
  spin_button_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  data_p->configuration->streamConfiguration.bufferSize =
    static_cast<unsigned int> (gtk_spin_button_get_value_as_int (spin_button_p));

  // step3: start processing thread
  ACE_NEW_NORETURN (thread_data_p,
                    Test_I_Source_ThreadData ());
  if (!thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto clean;
  } // end IF
  thread_data_p->CBData = data_p;
  ACE_OS::memset (thread_name, 0, sizeof (thread_name));
  //  char* thread_name_p = NULL;
  //  ACE_NEW_NORETURN (thread_name_p,
  //                    ACE_TCHAR[BUFSIZ]);
  //  if (!thread_name_p)
  //  {
  //    ACE_DEBUG ((LM_CRITICAL,
  //                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

  //    // clean up
  //    delete thread_data_p;

  //    return;
  //  } // end IF
  //  ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
  //  ACE_OS::strcpy (thread_name_p,
  //                  ACE_TEXT (TEST_I_STREAM_FILECOPY_THREAD_NAME));
  //  const char* thread_name_2 = thread_name_p;
  ACE_OS::strcpy (thread_name,
                  ACE_TEXT (TEST_I_THREAD_NAME));
  thread_name_2 = thread_name;
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);

  // *NOTE*: lock access to the progress report structures to avoid a race
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

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
                               COMMON_EVENT_THREAD_GROUP_ID + 2, // *TODO*: group id
                               NULL,                             // stack
                               0,                                // stack size
                               &thread_name_2);                  // name
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::spawn(): \"%m\", returning\n")));

      // clean up
      delete thread_data_p;

      goto clean;
    } // end IF

    // step3: start progress reporting
    event_source_id =
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_progress_cb,
      //                 &data_p->progressData,
      //                 NULL);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,                          // _LOW doesn't work (on Win32)
                          TEST_I_STREAM_UI_GTK_PROGRESSBAR_UPDATE_INTERVAL, // ms (?)
                          idle_update_progress_source_cb,
                          &data_p->progressData,
                          NULL);
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
    thread_data_p->eventSourceID = event_source_id;
    data_p->progressData.pendingActions[event_source_id] =
      ACE_Thread_ID (thread_id, thread_handle);
    //    ACE_DEBUG ((LM_DEBUG,
    //                ACE_TEXT ("idle_update_progress_cb: %d\n"),
    //                event_source_id));
    data_p->eventSourceIds.insert (event_source_id);
  } // end lock scope

  return;

clean:
  gtk_action_set_stock_id (GTK_ACTION (toggleAction_in), GTK_STOCK_MEDIA_PLAY);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);
  gtk_action_set_sensitive (action_p, true);
  action_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_SETTINGS_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, true);
} // toggleaction_stream_toggled_cb

void
action_reset_activate_cb (GtkAction* action_in,
                          gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_reset_activate_cb"));

  Test_I_Source_GTK_CBData* data_p =
    static_cast<Test_I_Source_GTK_CBData*> (userData_in);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());
} // action_reset_activate_cb

void
action_settings_activate_cb (GtkAction* action_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_settings_activate_cb"));

  Test_I_Source_GTK_CBData* data_p =
    static_cast<Test_I_Source_GTK_CBData*> (userData_in);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->stream);
  ACE_ASSERT (data_p->UDPStream);
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

} // action_settings_activate_cb

/////////////////////////////////////////

void
action_close_all_activate_cb (GtkAction* action_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_close_all_activate_cb"));

  gtk_action_set_sensitive (action_in, FALSE);
  Test_I_Target_GTK_CBData* data_p =
    static_cast<Test_I_Target_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

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
  if (data_p->configuration->protocol == NET_TRANSPORTLAYER_UDP)
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
toggleaction_listen_activate_cb (GtkToggleAction* toggleAction_in,
                                 gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::toggleaction_listen_activate_cb"));

  ACE_UNUSED_ARG (toggleAction_in);
  Test_I_Target_GTK_CBData* data_p =
    static_cast<Test_I_Target_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

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
    data_p->configuration->protocol = NET_TRANSPORTLAYER_TCP;
  else
    data_p->configuration->protocol = NET_TRANSPORTLAYER_UDP;

  Test_I_Target_InetConnectionManager_t* connection_manager_p =
    TEST_I_TARGET_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  if (start_listening)
  {
    switch (data_p->configuration->protocol)
    {
      case NET_TRANSPORTLAYER_TCP:
      {
        // listening on UDP ? --> stop
        if (data_p->configuration->handle != ACE_INVALID_HANDLE)
        {
          Test_I_Target_InetConnectionManager_t::ICONNECTION_T* connection_p =
            connection_manager_p->get (data_p->configuration->handle);
          if (connection_p)
          {
            connection_p->close ();
            connection_p->decrease ();
          } // end ELSE
          data_p->configuration->handle = ACE_INVALID_HANDLE;
        } // end IF

        data_p->configuration->listenerConfiguration.address =
          data_p->configuration->socketConfiguration.peerAddress;
        ACE_ASSERT (data_p->configuration->signalHandlerConfiguration.listener);
        if (!data_p->configuration->signalHandlerConfiguration.listener->initialize (data_p->configuration->listenerConfiguration))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize listener, continuing\n")));
        try
        {
          data_p->configuration->signalHandlerConfiguration.listener->start ();
        }
        catch (...)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_Server_IListener::start(): \"%m\", continuing\n")));
        } // end catch

        break;
      }
      case NET_TRANSPORTLAYER_UDP:
      {
        // listening on TCP ? --> stop
        ACE_ASSERT (data_p->configuration->signalHandlerConfiguration.listener);
        if (data_p->configuration->signalHandlerConfiguration.listener->isRunning ())
        {
          try
          {
            data_p->configuration->signalHandlerConfiguration.listener->stop ();
          }
          catch (...)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in Net_Server_IListener::stop(): \"%m\", continuing\n")));
          } // end catch
        } // end IF

        if (data_p->configuration->handle != ACE_INVALID_HANDLE)
        {
          Test_I_Target_InetConnectionManager_t::ICONNECTION_T* connection_p =
            connection_manager_p->get (data_p->configuration->handle);
          if (connection_p)
          {
            connection_p->close ();
            connection_p->decrease ();
          } // end ELSE
          data_p->configuration->handle = ACE_INVALID_HANDLE;
        } // end IF

        Test_I_Target_InetConnectionManager_t::INTERFACE_T* iconnection_manager_p =
          connection_manager_p;
        ACE_ASSERT (iconnection_manager_p);
        Test_I_Target_IInetConnector_t* connector_p = NULL;
        if (data_p->configuration->useReactor)
          ACE_NEW_NORETURN (connector_p,
                            Test_I_Target_UDPConnector_t (iconnection_manager_p,
                                                          data_p->configuration->streamConfiguration.statisticReportingInterval));
        else
          ACE_NEW_NORETURN (connector_p,
                            Test_I_Target_UDPAsynchConnector_t (iconnection_manager_p,
                                                                data_p->configuration->streamConfiguration.statisticReportingInterval));
        if (!connector_p)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("failed to allocate memory, returning\n")));
          return;
        } // end IF
        //  Stream_IInetConnector_t* iconnector_p = &connector;
        if (!connector_p->initialize (data_p->configuration->socketHandlerConfiguration))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));

          // clean up
          delete connector_p;

          return;
        } // end IF

        ACE_TCHAR buffer[BUFSIZ];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        int result =
          data_p->configuration->socketConfiguration.peerAddress.addr_to_string (buffer,
                                                                                 sizeof (buffer),
                                                                                 1);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
        // connect
        data_p->configuration->handle =
          connector_p->connect (data_p->configuration->socketConfiguration.peerAddress);
        // *TODO*: support one-thread operation by scheduling a signal and manually
        //         running the dispatch loop for a limited time...
        if (!data_p->configuration->useReactor)
        {
          // *TODO*: avoid tight loop here
          ACE_Time_Value timeout (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0);
          //result = ACE_OS::sleep (timeout);
          //if (result == -1)
          //  ACE_DEBUG ((LM_ERROR,
          //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
          //              &timeout));
          ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
          Test_I_Target_InetConnectionManager_t::ICONNECTION_T* connection_p =
            NULL;
          do
          {
            connection_p =
              connection_manager_p->get (data_p->configuration->socketConfiguration.peerAddress);
            if (connection_p)
            {
              data_p->configuration->handle =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                  reinterpret_cast<ACE_HANDLE> (connection_p->id ());
#else
                  static_cast<ACE_HANDLE> (connection_p->id ());
#endif
              connection_p->decrease ();
              break;
            } // end IF
          } while (COMMON_TIME_NOW < deadline);
        } // end IF
        if (data_p->configuration->handle == ACE_INVALID_HANDLE)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to connect to \"%s\", returning\n"),
                      ACE_TEXT (buffer)));

          // clean up
          connector_p->abort ();
          delete connector_p;

          return;
        } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("0x%@: started listening (UDP) (\"%s\")...\n"),
                    data_p->configuration->handle,
                    buffer));
#else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%d: started listening (UDP) (\"%s\")...\n"),
                    data_p->configuration->handle,
                    buffer));
#endif

        // clean up
        delete connector_p;

        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown transport layer type (was: %d), returning\n"),
                    data_p->configuration->protocol));
        return;
      } // end catch
    } // end SWITCH

    GtkFrame* frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
    ACE_ASSERT (frame_p);
    gtk_widget_set_sensitive (GTK_WIDGET (frame_p), false);

    // start progress reporting
    GtkProgressBar* progress_bar_p =
      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
    ACE_ASSERT (progress_bar_p);
    gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), true);

    ACE_ASSERT (!data_p->progressEventSourceID);
    {
      ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

      data_p->progressEventSourceID =
        //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
        //                 idle_update_progress_cb,
        //                 &data_p->progressData,
        //                 NULL);
        g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,                          // _LOW doesn't work (on Win32)
                            TEST_I_STREAM_UI_GTK_PROGRESSBAR_UPDATE_INTERVAL, // ms (?)
                            idle_update_progress_target_cb,
                            &data_p->progressData,
                            NULL);
      if (data_p->progressEventSourceID > 0)
        data_p->eventSourceIds.insert (data_p->progressEventSourceID);
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_timeout_add_full(idle_update_target_progress_cb): \"%m\", returning\n")));
        return;
      } // end IF
    } // end lock scope
  } // end IF
  else
  {
    ACE_ASSERT (data_p->configuration->signalHandlerConfiguration.listener);
    try
    {
      data_p->configuration->signalHandlerConfiguration.listener->stop ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_Server_IListener::stop(): \"%m\", continuing\n")));
    } // end catch

    if (data_p->configuration->handle != ACE_INVALID_HANDLE)
    {
      Test_I_Target_InetConnectionManager_t::ICONNECTION_T* connection_p =
        connection_manager_p->get (data_p->configuration->handle);
      if (connection_p)
      {
        connection_p->close ();
        connection_p->decrease ();
      } // end ELSE
      data_p->configuration->handle = ACE_INVALID_HANDLE;
    } // end IF

    GtkFrame* frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
    ACE_ASSERT (frame_p);
    gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

    // stop progress reporting
    ACE_ASSERT (data_p->progressEventSourceID);
    {
      ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

      if (!g_source_remove (data_p->progressEventSourceID))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                    data_p->progressEventSourceID));
      data_p->eventSourceIds.erase (data_p->progressEventSourceID);
      data_p->progressEventSourceID = 0;
    } // end lock scope
    GtkProgressBar* progress_bar_p =
      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
    ACE_ASSERT (progress_bar_p);
    // *NOTE*: this disables "activity mode" (in Gtk2)
    gtk_progress_bar_set_fraction (progress_bar_p, 0.0);
    gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), false);
  } // end ELSE
} // action_listen_activate_cb

void
filechooserbutton_target_cb (GtkFileChooserButton* button_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_target_cb"));

  Test_I_Target_GTK_CBData* data_p =
    static_cast<Test_I_Target_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  // sanity check(s)
  ACE_ASSERT (data_p->configuration);

  GFile* file_p = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (button_in));
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

  data_p->configuration->moduleHandlerConfiguration.targetFileName =
    Common_UI_Tools::UTF82Locale (string_p, -1);
  if (data_p->configuration->moduleHandlerConfiguration.targetFileName.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_Tools::UTF82Locale(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (string_p)));

    // clean up
    g_free (string_p);

    return;
  } // end IF
  g_free (string_p);
} // filechooserbutton_target_cb
void
filechooser_target_cb (GtkFileChooser* fileChooser_in,
                       gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_target_cb"));

  Test_I_Target_GTK_CBData* data_p =
    static_cast<Test_I_Target_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  // sanity check(s)
  ACE_ASSERT (data_p->configuration);

  GFile* file_p = gtk_file_chooser_get_file (fileChooser_in);
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

  data_p->configuration->moduleHandlerConfiguration.targetFileName =
    Common_UI_Tools::UTF82Locale (string_p, -1);
  if (data_p->configuration->moduleHandlerConfiguration.targetFileName.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_Tools::UTF82Locale(\"%s\"): \"%m\", returning\n"),
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
//  Test_I_GTK_CBData* data_p =
//    static_cast<Test_I_GTK_CBData*> (userData_in);
//
//  //Common_UI_GladeXMLsIterator_t iterator =
//  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
//  Common_UI_GTKBuildersIterator_t iterator =
//    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
//
//  // sanity check(s)
//  ACE_ASSERT (data_p);
//  ACE_ASSERT (data_p->configuration);
//  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
//  ACE_ASSERT (iterator != data_p->builders.end ());
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
//    data_p->configuration->protocol = NET_TRANSPORTLAYER_TCP;
//  else
//    data_p->configuration->protocol = NET_TRANSPORTLAYER_UDP;
//}

//void
//spinbutton_port_value_changed_cb (GtkWidget* widget_in,
//                                  gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::spinbutton_port_value_changed_cb"));
//
//  Test_I_GTK_CBData* data_p =
//    static_cast<Test_I_GTK_CBData*> (userData_in);
//
//  // sanity check(s)
//  ACE_ASSERT (data_p);
//  ACE_ASSERT (data_p->configuration);
//
//  unsigned short port_number =
//    static_cast<unsigned short> (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget_in)));
//  data_p->configuration->socketConfiguration.peerAddress.set_port_number (port_number,
//                                                                          1);
//} // spinbutton_port_value_changed_cb

// -----------------------------------------------------------------------------

gint
button_clear_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  Test_I_GTK_CBData* data_p =
    static_cast<Test_I_GTK_CBData*> (userData_in);

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
  Test_I_GTK_CBData* data_p =
    static_cast<Test_I_GTK_CBData*> (userData_in);

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
    //                                  ACE_TEXT_ALWAYS_CHAR(TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to glade_xml_get_widget(\"%s\"): \"%m\", aborting\n"),
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
  //Test_I_GTK_CBData* data_p = static_cast<Test_I_GTK_CBData*> (userData_in);
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
  COMMON_UI_GTK_MANAGER_SINGLETON::instance()->stop (false, true);

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
  Test_I_GTK_CBData* data_p =
    static_cast<Test_I_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT(iterator != data_p->builders.end ());

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
                            adjustment_p->upper - adjustment_p->page_size);
} // textview_size_allocate_cb

void
combobox_source_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_source_changed_cb"));

  Test_I_Source_GTK_CBData* data_p =
    static_cast<Test_I_Source_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
    return; // <-- nothing selected
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  GValue value = {0,};
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            0, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string device_string = g_value_get_string (&value);
  g_value_unset (&value);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->streamConfiguration)
  {
    data_p->streamConfiguration->Release ();
    data_p->streamConfiguration = NULL;
  } // end IF
  if (data_p->configuration->moduleHandlerConfiguration.builder)
  {
    data_p->configuration->moduleHandlerConfiguration.builder->Release ();
    data_p->configuration->moduleHandlerConfiguration.builder = NULL;
  } // end IF
  if (!Stream_Module_Device_Tools::load (device_string,
                                         data_p->configuration->moduleHandlerConfiguration.builder,
                                         data_p->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::load(\"%s\"), returning\n"),
                ACE_TEXT (device_string.c_str ())));
    return;
  } // end IF
  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.builder);
  ACE_ASSERT (data_p->streamConfiguration);

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  if (!load_formats (data_p->streamConfiguration,
                     list_store_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_formats(), returning\n")));
    return;
  } // end IF
#endif

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF

  GtkToggleAction* toggle_action_p =
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_STREAM_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_action_set_sensitive (GTK_ACTION (toggle_action_p), true);
} // combobox_source_changed_cb

void
combobox_format_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_format_changed_cb"));

  Test_I_Source_GTK_CBData* data_p =
    static_cast<Test_I_Source_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->streamConfiguration);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  GValue value = {0,};
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  GUID GUID_i;
  ACE_OS::memset (&GUID_i, 0, sizeof (GUID));
  HRESULT result = E_FAIL;
#if defined (OLE2ANSI)
  result =
    CLSIDFromString (g_value_get_string (&value),
                     &GUID_i);
#else
  result =
    CLSIDFromString (ACE_TEXT_ALWAYS_WCHAR (g_value_get_string (&value)),
                     &GUID_i);
#endif
  g_value_unset (&value);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CLSIDFromString(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.builder);

  AM_MEDIA_TYPE* media_type_p = NULL;
  result = data_p->streamConfiguration->GetFormat (&media_type_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
  ACE_ASSERT (media_type_p);
  media_type_p->subtype = GUID_i;

  // *NOTE*: the graph may (!) be stopped, but is in a "connected" state, i.e.
  //         the filter pins are associated. IGraphConfig::Reconnect fails
  //         unless the graph is "disconnected" first
  if (!Stream_Module_Device_Tools::disconnect (data_p->configuration->moduleHandlerConfiguration.builder))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::disconnect(), returning\n")));
    goto error;
  } // end IF
  if (!Stream_Module_Device_Tools::setFormat (data_p->configuration->moduleHandlerConfiguration.builder,
                                              *media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setFormat(), returning\n")));
    goto error;
  } // end IF
  //DeleteMediaType (media_type_p);
  Stream_Module_Device_Tools::deleteMediaType (media_type_p);

  goto continue_;

error:
  //DeleteMediaType (media_type_p);
  Stream_Module_Device_Tools::deleteMediaType (media_type_p);

  return;

continue_:
  if (!load_resolutions (data_p->streamConfiguration,
                         GUID_i,
                         list_store_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_resolutions(), returning\n")));
    return;
  } // end IF
#endif
  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF
} // combobox_source_changed_cb

void
combobox_resolution_changed_cb (GtkWidget* widget_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_resolution_changed_cb"));

  Test_I_Source_GTK_CBData* data_p =
    static_cast<Test_I_Source_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  GtkTreeIter iterator_2;
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  GValue value = {0,};
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  GUID GUID_i;
  ACE_OS::memset (&GUID_i, 0, sizeof (GUID));
  HRESULT result = E_FAIL;
#if defined (OLE2ANSI)
  result =
    CLSIDFromString (g_value_get_string (&value),
                     &GUID_i);
#else
  result =
    CLSIDFromString (ACE_TEXT_ALWAYS_WCHAR (g_value_get_string (&value)),
                     &GUID_i);
#endif
  g_value_unset (&value);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CLSIDFromString(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF

  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  GValue value_2 = {0,};
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            2, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_UINT);
  unsigned int width = g_value_get_uint (&value);
  g_value_unset (&value);
  unsigned int height = g_value_get_uint (&value_2);
  g_value_unset (&value_2);
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_RATE_NAME)));
  ACE_ASSERT (list_store_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (data_p->streamConfiguration);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.builder);

  AM_MEDIA_TYPE* media_type_p = NULL;
  result = data_p->streamConfiguration->GetFormat (&media_type_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
  ACE_ASSERT (media_type_p);
  if (media_type_p->formattype == FORMAT_VideoInfo)
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
    video_info_header_p->bmiHeader.biWidth = width;
    video_info_header_p->bmiHeader.biHeight = height;
  } // end IF
  else if (media_type_p->formattype == FORMAT_VideoInfo2)
  {
    // *NOTE*: these media subtypes do not work with the Video Renderer
    //         directly --> insert the Overlay Mixer
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
    video_info_header2_p->bmiHeader.biWidth = width;
    video_info_header2_p->bmiHeader.biHeight = height;
  } // end ELSE IF

    // *NOTE*: the graph may (!) be stopped, but is in a "connected" state, i.e.
    //         the filter pins are associated. IGraphConfig::Reconnect fails
    //         unless the graph is "disconnected" first
  if (!Stream_Module_Device_Tools::disconnect (data_p->configuration->moduleHandlerConfiguration.builder))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::disconnect(), returning\n")));
    goto error;
  } // end IF
  if (!Stream_Module_Device_Tools::setFormat (data_p->configuration->moduleHandlerConfiguration.builder,
                                              *media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setFormat(), returning\n")));
    goto error;
  } // end IF
    //DeleteMediaType (media_type_p);
  Stream_Module_Device_Tools::deleteMediaType (media_type_p);

  goto continue_;

error:
  //DeleteMediaType (media_type_p);
  Stream_Module_Device_Tools::deleteMediaType (media_type_p);

  return;

continue_:
  if (!load_rates (data_p->streamConfiguration,
                   GUID_i,
                   width,
                   list_store_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_rates(), returning\n")));
    return;
  } // end IF
#endif
  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_RATE_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF
} // combobox_resolution_changed_cb

void
combobox_rate_changed_cb (GtkWidget* widget_in,
                          gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_rate_changed_cb"));

  Test_I_Source_GTK_CBData* data_p =
    static_cast<Test_I_Source_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_RATE_NAME)));
  ACE_ASSERT (list_store_p);
  GValue value = {0,};
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  unsigned int frame_interval = g_value_get_uint (&value);
  g_value_unset (&value);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (data_p->streamConfiguration);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.builder);

  AM_MEDIA_TYPE* media_type_p = NULL;
  HRESULT result = data_p->streamConfiguration->GetFormat (&media_type_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
  ACE_ASSERT (media_type_p);
  if (media_type_p->formattype == FORMAT_VideoInfo)
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
    video_info_header_p->AvgTimePerFrame = frame_interval;
  } // end IF
  else if (media_type_p->formattype == FORMAT_VideoInfo2)
  {
    // *NOTE*: these media subtypes do not work with the Video Renderer
    //         directly --> insert the Overlay Mixer
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
    video_info_header2_p->AvgTimePerFrame = frame_interval;
  } // end ELSE IF

    // *NOTE*: the graph may (!) be stopped, but is in a "connected" state, i.e.
    //         the filter pins are associated. IGraphConfig::Reconnect fails
    //         unless the graph is "disconnected" first
  if (!Stream_Module_Device_Tools::disconnect (data_p->configuration->moduleHandlerConfiguration.builder))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::disconnect(), returning\n")));
    goto error;
  } // end IF
  if (!Stream_Module_Device_Tools::setFormat (data_p->configuration->moduleHandlerConfiguration.builder,
                                              *media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setFormat(), returning\n")));
    goto error;
  } // end IF
    //DeleteMediaType (media_type_p);
  Stream_Module_Device_Tools::deleteMediaType (media_type_p);

  return;

error:
  //DeleteMediaType (media_type_p);
  Stream_Module_Device_Tools::deleteMediaType (media_type_p);
#endif
} // combobox_rate_changed_cb

void
drawingarea_configure_event_cb (GtkWindow* window_in,
                                GdkEvent* event_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_configure_event_cb"));

  Test_I_Target_GTK_CBData* data_p =
    static_cast<Test_I_Target_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!data_p->configuration->moduleHandlerConfiguration.window ||
      !data_p->configuration->moduleHandlerConfiguration.windowController) // <-- window not realized yet ?
    return;
#else
  if (!data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.window) // <-- window not realized yet ?
    return;
#endif

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkDrawingArea* drawing_area_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  GtkAllocation allocation;
  ACE_OS::memset (&allocation, 0, sizeof (GtkAllocation));
  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
                             &allocation);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.windowController);

  data_p->configuration->moduleHandlerConfiguration.area.bottom =
    allocation.height;
  data_p->configuration->moduleHandlerConfiguration.area.left =
    allocation.x;
  data_p->configuration->moduleHandlerConfiguration.area.right =
    allocation.width;
  data_p->configuration->moduleHandlerConfiguration.area.top =
    allocation.y;

  //HRESULT result =
  //  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.windowController->SetWindowPosition (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.left,
  //                                                                                                               data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.top,
  //                                                                                                               data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.right,
  //                                                                                                               data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.bottom);
  //if (FAILED (result))
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IVideoWindow::SetWindowPosition(%d,%d,%d,%d): \"%s\", continuing\n"),
  //              data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.left, data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.top,
  //              data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.right, data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.bottom,
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
#else
  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area =
    allocation;
#endif
} // drawingarea_configure_event_cb

/////////////////////////////////////////

void
filechooserdialog_target_cb (GtkFileChooser* chooser_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserdialog_target_cb"));

  ACE_UNUSED_ARG (userData_in);

//  Test_I_GTK_CBData* data_p =
//    static_cast<Test_I_GTK_CBData*> (userData_in);

//  // sanity check(s)
//  ACE_ASSERT (data_p);

  gtk_dialog_response (GTK_DIALOG (GTK_FILE_CHOOSER_DIALOG (chooser_in)),
                       GTK_RESPONSE_ACCEPT);
} // filechooserdialog_cb

#ifdef __cplusplus
}
#endif /* __cplusplus */
