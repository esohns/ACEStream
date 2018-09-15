#include "ace/Synch.h"
#include "test_u_camsave_ui.h"

#include "test_u_camsave_common.h"

//////////////////////////////////////////

ACE_THR_FUNC_RETURN
event_processing_thread (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("::event_processing_thread"));

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("processing thread (id: %t) starting...\n")));
#endif // _DEBUG

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = std::numeric_limits<unsigned long>::max ();
#else
  result = arg_in;
#endif // ACE_WIN32 || ACE_WIN64

  struct Stream_CamSave_UI_ThreadData* thread_data_p =
      static_cast<struct Stream_CamSave_UI_ThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (thread_data_p);
  ACE_ASSERT (thread_data_p->CBData);
  ACE_ASSERT (thread_data_p->CBData->UIState);

  enum Common_UI_EventType* event_p = NULL;
  Common_UI_wxWidgets_XmlResourcesIterator_t iterator;
  wxDialog* dialog_p = NULL;
  wxSpinCtrl* spin_control_p = NULL;
  ACE_Time_Value delay (0, 200000);
loop:
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, thread_data_p->CBData->UIState->lock, result);
    iterator =
      thread_data_p->CBData->UIState->resources.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
    ACE_ASSERT (iterator != thread_data_p->CBData->UIState->resources.end ());
    dialog_p = dynamic_cast<wxDialog*> ((*iterator).second.second);
    ACE_ASSERT (dialog_p);

    for (Common_UI_Events_t::ITERATOR iterator_2 (thread_data_p->CBData->UIState->eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_STARTED:
        {
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spincontrol_messages_session"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (1);
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spincontrol_messages_data"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (0);
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spincontrol_data"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (0);
          break;
        }
        case COMMON_UI_EVENT_DATA:
        {
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spincontrol_messages_data"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (spin_control_p->GetValue () + 1);
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spincontrol_data"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (thread_data_p->CBData->progressData.statistic.bytes);
          break;
        }
        case COMMON_UI_EVENT_FINISHED:
        {
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spincontrol_messages_session"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (spin_control_p->GetValue () + 1);

          wxBitmapButton* button_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("button_snapshot"),
                     wxBitmapButton);
          ACE_ASSERT (button_p);
          button_p->Enable (false);
          button_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("button_cut"),
                     wxBitmapButton);
          ACE_ASSERT (button_p);
          button_p->Enable (false);
          button_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("button_report"),
                     wxBitmapButton);
          ACE_ASSERT (button_p);
          button_p->Enable (false);
          wxGauge* gauge_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("gauge_progress"),
                     wxGauge);
          ACE_ASSERT (gauge_p);
          gauge_p->Enable (false);

          wxChoice* choice_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("choice_source"),
                     wxChoice);
          ACE_ASSERT (choice_p);
          choice_p->Enable (true);
          choice_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("choice_format"),
                     wxChoice);
          ACE_ASSERT (choice_p);
          choice_p->Enable (true);
          choice_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("choice_resolution"),
                     wxChoice);
          ACE_ASSERT (choice_p);
          choice_p->Enable (true);
          choice_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("choice_framerate"),
                     wxChoice);
          ACE_ASSERT (choice_p);
          choice_p->Enable (true);
          button_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("button_reset"),
                     wxBitmapButton);
          ACE_ASSERT (button_p);
          button_p->Enable (true);
          wxToggleButton* toggle_button_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("togglebutton_save"),
                     wxToggleButton);
          ACE_ASSERT (toggle_button_p);
          toggle_button_p->Enable (true);
          wxTextCtrl* text_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("textcontrol_filename"),
                     wxTextCtrl);
          ACE_ASSERT (text_p);
          text_p->Enable (true);
          wxDirPickerCtrl* picker_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("directorypicker_save"),
                     wxDirPickerCtrl);
          ACE_ASSERT (picker_p);
          picker_p->Enable (true);

          break;
        }
        case COMMON_UI_EVENT_STATISTIC:
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spincontrol_frames_captured"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (thread_data_p->CBData->progressData.statistic.capturedFrames);
#endif // ACE_WIN32 || ACE_WIN64
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spincontrol_frames_dropped"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (thread_data_p->CBData->progressData.statistic.droppedFrames);
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spincontrol_messages_session"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (spin_control_p->GetValue () + 1);
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
      event_p = NULL;
    } // end FOR

    // clean up
    enum Common_UI_EventType event_2;
    while (!thread_data_p->CBData->UIState->eventStack.is_empty ())
    {
      result = thread_data_p->CBData->UIState->eventStack.pop (event_2);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Unbounded_Stack::pop(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope

  result = ACE_OS::sleep (delay);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                &delay));
  goto loop;

//end:
  return result;
}

ACE_THR_FUNC_RETURN
stream_processing_thread (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_processing_thread"));

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("processing thread (id: %t) starting...\n")));
#endif // _DEBUG

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = std::numeric_limits<unsigned long>::max ();
#else
  result = arg_in;
#endif // ACE_WIN32 || ACE_WIN64

  struct Stream_CamSave_UI_ThreadData* thread_data_p =
      static_cast<struct Stream_CamSave_UI_ThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (thread_data_p);
  ACE_ASSERT (thread_data_p->CBData);

  const Stream_CamSave_SessionData_t* session_data_container_p = NULL;
  const Stream_CamSave_SessionData* session_data_p = NULL;
  Stream_IStreamControlBase* stream_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_UI_CBData* directshow_cb_data_p = NULL;
  struct Stream_CamSave_MediaFoundation_UI_CBData* mediafoundation_cb_data_p =
    NULL;
  switch (thread_data_p->CBData->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_UI_CBData*> (thread_data_p->CBData);
      ACE_ASSERT (directshow_cb_data_p->configuration);
      ACE_ASSERT (directshow_cb_data_p->stream);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_UI_CBData*> (thread_data_p->CBData);
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_cb_data_p->stream);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  thread_data_p->CBData->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (thread_data_p->CBData);
  ACE_ASSERT (cb_data_p->configuration);
  ACE_ASSERT (cb_data_p->stream);
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (thread_data_p->CBData->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T iterator =
        const_cast<Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T&> (directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR ("")));
      ACE_ASSERT (iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      if ((*iterator).second.second.direct3DDevice)
      {
        (*iterator).second.second.direct3DDevice->Release (); (*iterator).second.second.direct3DDevice = NULL;
      } // end IF

      if (!directshow_cb_data_p->stream->initialize (directshow_cb_data_p->configuration->streamConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize stream, aborting\n")));
        goto error;
      } // end IF
      stream_p = directshow_cb_data_p->stream;
      session_data_container_p = &directshow_cb_data_p->stream->getR ();
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (!mediafoundation_cb_data_p->stream->initialize (mediafoundation_cb_data_p->configuration->streamConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize stream, aborting\n")));
        goto error;
      } // end IF
      stream_p = mediafoundation_cb_data_p->stream;
      session_data_container_p = &mediafoundation_cb_data_p->stream->getR ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  thread_data_p->CBData->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  if (!cb_data_p->stream->initialize (cb_data_p->configuration->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_CamSave_Stream::initialize(), aborting\n")));
    goto error;
  } // end IF
  stream_p = cb_data_p->stream;
  session_data_container_p = &cb_data_p->stream->getR ();
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (session_data_container_p);
  session_data_p = &session_data_container_p->getR ();
  thread_data_p->sessionId = session_data_p->sessionId;

  // *NOTE*: blocks until 'finished'
  ACE_ASSERT (stream_p);
  stream_p->start ();
  ACE_ASSERT (!stream_p->isRunning ());
  //stream_p->wait (true, false, false);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif // ACE_WIN32 || ACE_WIN64

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, thread_data_p->CBData->UIState->lock, -1);
#else
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, thread_data_p->CBData->UIState->lock, std::numeric_limits<void*>::max ());
#endif // ACE_WIN32 || ACE_WIN64
    thread_data_p->CBData->progressData.completedActions.insert (thread_data_p->sessionId);
  } // end lock scope

  // clean up
  delete thread_data_p; thread_data_p = NULL;

  return result;
}

/////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//wxIMPLEMENT_DYNAMIC_CLASS (Stream_CamSave_DirectShow_WxWidgetsDialog_t, dialog_main)
wxClassInfo
Stream_CamSave_DirectShow_WxWidgetsDialog_t::ms_classInfo (L"Stream_CamSave_DirectShow_WxWidgetsDialog_t",
                                                           &dialog_main::ms_classInfo,
                                                           NULL,
                                                           (int) sizeof (Stream_CamSave_DirectShow_WxWidgetsDialog_t),
                                                           Stream_CamSave_DirectShow_WxWidgetsDialog_t::wxCreateObject);
wxClassInfo*
Stream_CamSave_DirectShow_WxWidgetsDialog_t::GetClassInfo () const
{
  return &Stream_CamSave_DirectShow_WxWidgetsDialog_t::ms_classInfo;
}

wxObject*
Stream_CamSave_DirectShow_WxWidgetsDialog_t::wxCreateObject ()
{
  return new Stream_CamSave_DirectShow_WxWidgetsDialog_t;
}

//wxIMPLEMENT_DYNAMIC_CLASS (Stream_CamSave_MediaFoundation_WxWidgetsDialog_t, dialog_main)
wxClassInfo
Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::ms_classInfo (L"Stream_CamSave_MediaFoundation_WxWidgetsDialog_t",
                                                                &dialog_main::ms_classInfo,
                                                                NULL,
                                                                (int) sizeof (Stream_CamSave_MediaFoundation_WxWidgetsDialog_t),
                                                                Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::wxCreateObject);
wxClassInfo*
Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::GetClassInfo () const
{
  return &Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::ms_classInfo;
}

wxObject*
Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::wxCreateObject ()
{
  return new Stream_CamSave_MediaFoundation_WxWidgetsDialog_t;
}

wxBEGIN_EVENT_TABLE (Stream_CamSave_DirectShow_WxWidgetsDialog_t, dialog_main)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_record"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::togglebutton_record_toggled_cb)
 EVT_BUTTON (XRCID ("button_snapshot"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::button_snapshot_click_cb)
 EVT_BUTTON (XRCID ("button_cut"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::button_cut_click_cb)
 EVT_BUTTON (XRCID ("button_report"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::button_report_click_cb)
 EVT_CHOICE (XRCID ("choice_source"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::choice_source_selected_cb)
 EVT_BUTTON (XRCID ("button_hardware_settings"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::button_hardware_settings_click_cb)
 EVT_CHOICE (XRCID ("choice_format"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::choice_format_selected_cb)
 EVT_CHOICE (XRCID ("choice_resolution"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::choice_resolution_selected_cb)
 EVT_CHOICE (XRCID ("choice_framerate"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::choice_framerate_selected_cb)
 EVT_BUTTON (XRCID ("button_reset_format"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::button_reset_format_click_cb)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_save"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::togglebutton_save_toggled_cb)
 EVT_DIRPICKER_CHANGED (XRCID ("directorypicker_save"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::picker_directory_save_changed_cb)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_fullscreen"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::togglebutton_fullscreen_toggled_cb)
 EVT_BUTTON (XRCID ("button_display_settings"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::button_display_settings_click_cb)
 EVT_BUTTON (XRCID ("button_about"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::button_about_click_cb)
 EVT_BUTTON (XRCID ("button_quit"), Stream_CamSave_DirectShow_WxWidgetsDialog_t::button_quit_click_cb)
wxEND_EVENT_TABLE ()
wxBEGIN_EVENT_TABLE (Stream_CamSave_MediaFoundation_WxWidgetsDialog_t, dialog_main)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_record"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::togglebutton_record_toggled_cb)
 EVT_BUTTON (XRCID ("button_snapshot"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::button_snapshot_click_cb)
 EVT_BUTTON (XRCID ("button_cut"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::button_cut_click_cb)
 EVT_BUTTON (XRCID ("button_report"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::button_report_click_cb)
 EVT_CHOICE (XRCID ("choice_source"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::choice_source_selected_cb)
 EVT_BUTTON (XRCID ("button_hardware_settings"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::button_hardware_settings_click_cb)
 EVT_CHOICE (XRCID ("choice_format"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::choice_format_selected_cb)
 EVT_CHOICE (XRCID ("choice_resolution"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::choice_resolution_selected_cb)
 EVT_CHOICE (XRCID ("choice_framerate"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::choice_framerate_selected_cb)
 EVT_BUTTON (XRCID ("button_reset_format"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::button_reset_format_click_cb)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_save"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::togglebutton_save_toggled_cb)
 EVT_DIRPICKER_CHANGED (XRCID ("directorypicker_save"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::picker_directory_save_changed_cb)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_fullscreen"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::togglebutton_fullscreen_toggled_cb)
 EVT_BUTTON (XRCID ("button_display_settings"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::button_display_settings_click_cb)
 EVT_BUTTON (XRCID ("button_about"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::button_about_click_cb)
 EVT_BUTTON (XRCID ("button_quit"), Stream_CamSave_MediaFoundation_WxWidgetsDialog_t::button_quit_click_cb)
wxEND_EVENT_TABLE ()
#else
//wxIMPLEMENT_DYNAMIC_CLASS (Stream_CamSave_V4L_WxWidgetsDialog_t, dialog_main)
wxClassInfo
Stream_CamSave_V4L_WxWidgetsDialog_t::ms_classInfo (L"Stream_CamSave_V4L_WxWidgetsDialog_t",
                                                    &dialog_main::ms_classInfo,
                                                    NULL,
                                                    (int) sizeof (Stream_CamSave_V4L_WxWidgetsDialog_t),
                                                    Stream_CamSave_V4L_WxWidgetsDialog_t::wxCreateObject);
wxClassInfo*
Stream_CamSave_V4L_WxWidgetsDialog_t::GetClassInfo () const
{
  return &Stream_CamSave_V4L_WxWidgetsDialog_t::ms_classInfo;
}

template <>
wxObject*
Stream_CamSave_V4L_WxWidgetsDialog_t::wxCreateObject ()
{
  return new Stream_CamSave_V4L_WxWidgetsDialog_t;
}

wxBEGIN_EVENT_TABLE (Stream_CamSave_V4L_WxWidgetsDialog_t, dialog_main)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_record"), Stream_CamSave_V4L_WxWidgetsDialog_t::togglebutton_record_toggled_cb)
 EVT_BUTTON (XRCID ("button_snapshot"), Stream_CamSave_V4L_WxWidgetsDialog_t::button_snapshot_click_cb)
 EVT_BUTTON (XRCID ("button_cut"), Stream_CamSave_V4L_WxWidgetsDialog_t::button_cut_click_cb)
 EVT_BUTTON (XRCID ("button_report"), Stream_CamSave_V4L_WxWidgetsDialog_t::button_report_click_cb)
 EVT_CHOICE (XRCID ("choice_source"), Stream_CamSave_V4L_WxWidgetsDialog_t::choice_source_selected_cb)
 EVT_BUTTON (XRCID ("button_hardware_settings"), Stream_CamSave_V4L_WxWidgetsDialog_t::button_hardware_settings_click_cb)
 EVT_CHOICE (XRCID ("choice_format"), Stream_CamSave_V4L_WxWidgetsDialog_t::choice_format_selected_cb)
 EVT_CHOICE (XRCID ("choice_resolution"), Stream_CamSave_V4L_WxWidgetsDialog_t::choice_resolution_selected_cb)
 EVT_CHOICE (XRCID ("choice_framerate"), Stream_CamSave_V4L_WxWidgetsDialog_t::choice_framerate_selected_cb)
 EVT_BUTTON (XRCID ("button_reset_format"), Stream_CamSave_V4L_WxWidgetsDialog_t::button_reset_format_click_cb)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_save"), Stream_CamSave_V4L_WxWidgetsDialog_t::togglebutton_save_toggled_cb)
 EVT_DIRPICKER_CHANGED (XRCID ("directorypicker_save"), Stream_CamSave_V4L_WxWidgetsDialog_t::picker_directory_save_changed_cb)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_fullscreen"), Stream_CamSave_V4L_WxWidgetsDialog_t::togglebutton_fullscreen_toggled_cb)
 EVT_BUTTON (XRCID ("button_display_settings"), Stream_CamSave_V4L_WxWidgetsDialog_t::button_display_settings_click_cb)
 EVT_BUTTON (XRCID ("button_about"), Stream_CamSave_V4L_WxWidgetsDialog_t::button_about_click_cb)
 EVT_BUTTON (XRCID ("button_quit"), Stream_CamSave_V4L_WxWidgetsDialog_t::button_quit_click_cb)
wxEND_EVENT_TABLE ()
#endif // ACE_WIN32 || ACE_WIN64
