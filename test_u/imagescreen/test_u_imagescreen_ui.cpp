#include "ace/Synch.h"
#include "test_u_imagescreen_ui.h"

#include "test_u_imagescreen_common.h"

//////////////////////////////////////////

void
process_stream_events (struct Stream_ImageScreen_UI_CBData* CBData_in,
                       bool& finished_out)
{
  STREAM_TRACE (ACE_TEXT ("::process_stream_events"));

  // initialize return value(s)
  finished_out = false;

  // sanity check(s)
  ACE_ASSERT (CBData_in);
  ACE_ASSERT (CBData_in->UIState);

  enum Stream_Visualization_VideoRenderer renderer_e =
    STREAM_VISUALIZATION_VIDEORENDERER_INVALID;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_ImageScreen_DirectShow_UI_CBData* directshow_cb_data_p = NULL;
  struct Stream_ImageScreen_MediaFoundation_UI_CBData* mediafoundation_cb_data_p =
    NULL;
  struct Stream_MediaFramework_Direct3D_Configuration* direct3DConfiguration_p =
    NULL;
  switch (CBData_in->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_ImageScreen_DirectShow_UI_CBData*> (CBData_in);
      ACE_ASSERT (directshow_cb_data_p->configuration);
      renderer_e =
        directshow_cb_data_p->configuration->streamConfiguration.configuration_.renderer;
      direct3DConfiguration_p =
        &directshow_cb_data_p->configuration->direct3DConfiguration;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_ImageScreen_MediaFoundation_UI_CBData*> (CBData_in);
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      renderer_e =
        mediafoundation_cb_data_p->configuration->streamConfiguration.configuration_.renderer;
      direct3DConfiguration_p =
        &mediafoundation_cb_data_p->configuration->direct3DConfiguration;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  CBData_in->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Stream_ImageScreen_V4L_UI_CBData* cb_data_p =
    static_cast<struct Stream_ImageScreen_V4L_UI_CBData*> (CBData_in);
  ACE_ASSERT (cb_data_p->configuration);
  renderer_e =
    cb_data_p->configuration->streamConfiguration.configuration_.renderer;
#endif // ACE_WIN32 || ACE_WIN64

  enum Common_UI_EventType* event_p = NULL;
  enum Common_UI_EventType event_2;
  Common_UI_wxWidgets_XmlResourcesIterator_t iterator;
  wxDialog* dialog_p = NULL;
  wxSpinCtrl* spin_control_p = NULL;
  int result = -1;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_in->UIState->lock);
    iterator =
      CBData_in->UIState->resources.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
    ACE_ASSERT (iterator != CBData_in->UIState->resources.end ());
    dialog_p = dynamic_cast<wxDialog*> ((*iterator).second.second);
    ACE_ASSERT (dialog_p);

    for (Common_UI_Events_t::ITERATOR iterator_2 (CBData_in->UIState->eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_STARTED:
        {
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spinctrl_messages_session"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (1);
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spinctrl_messages_data"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (0);
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spinctrl_data"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (0);
          break;
        }
        case COMMON_UI_EVENT_DATA:
        {
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spinctrl_messages_data"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (spin_control_p->GetValue () + 1);
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spinctrl_data"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (CBData_in->progressData.statistic.bytes);
          break;
        }
        case COMMON_UI_EVENT_FINISHED:
        {
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spinctrl_messages_session"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (spin_control_p->GetValue () + 1);

          wxToggleButton* toggle_button_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("togglebutton_record"),
                     wxToggleButton);
          ACE_ASSERT (toggle_button_p);
          toggle_button_p->Enable (true);
          wxButton* button_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("button_snapshot"),
                     wxButton);
          ACE_ASSERT (button_p);
          button_p->Enable (false);
          button_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("button_cut"),
                     wxButton);
          ACE_ASSERT (button_p);
          button_p->Enable (false);
#if defined (_DEBUG)
          button_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("button_report"),
                     wxButton);
          ACE_ASSERT (button_p);
          button_p->Enable (false);
#endif // _DEBUG

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
                     wxButton);
          ACE_ASSERT (button_p);
          button_p->Enable (true);

          toggle_button_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("togglebutton_save"),
                     wxToggleButton);
          ACE_ASSERT (toggle_button_p);
          toggle_button_p->Enable (true);
          wxTextCtrl* text_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("textctrl_filename"),
                     wxTextCtrl);
          ACE_ASSERT (text_p);
          text_p->Enable (true);
//          wxDirPickerCtrl* picker_p =
//            XRCCTRL (*dialog_p,
//                     ACE_TEXT_ALWAYS_CHAR ("directorypicker_save"),
//                     wxDirPickerCtrl);
//          ACE_ASSERT (picker_p);
//          picker_p->Enable (true);

          toggle_button_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("togglebutton_display"),
                     wxToggleButton);
          ACE_ASSERT (toggle_button_p);
          toggle_button_p->Enable (true);
          // *NOTE*: the stream will reset the device to 'desktop' mode, if that
          //         was the original setting; reset the control accordingly
          toggle_button_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("togglebutton_fullscreen"),
                     wxToggleButton);
          ACE_ASSERT (toggle_button_p);
          switch (renderer_e)
          {
            case STREAM_VISUALIZATION_VIDEORENDERER_NULL:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_2D:
              ACE_ASSERT (false); // *TODO*
              ACE_NOTSUP;
              ACE_NOTREACHED (break;)
            case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D:
              ACE_ASSERT (direct3DConfiguration_p);
              toggle_button_p->SetValue (!direct3DConfiguration_p->presentationParameters.Windowed);
              break;
            case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTSHOW:
            case STREAM_VISUALIZATION_VIDEORENDERER_GDI:
            case STREAM_VISUALIZATION_VIDEORENDERER_MEDIAFOUNDATION:
              ACE_ASSERT (false); // *TODO*
              ACE_NOTSUP;
              ACE_NOTREACHED (break;)
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GTK_SUPPORT)
            case STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO:
            case STREAM_VISUALIZATION_VIDEORENDERER_GTK_PIXBUF:
              ACE_ASSERT (false); // *TODO*
              ACE_NOTSUP;
              ACE_NOTREACHED (break;)
#endif // GTK_SUPPORT
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("invalid/unknown video renderer (was: %d), returning\n"),
                          renderer_e));
              return;
            }
          } // end SWITCH
          toggle_button_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("togglebutton_display"),
                     wxToggleButton);
          ACE_ASSERT (toggle_button_p);
          choice_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("choice_screen"),
                     wxChoice);
          ACE_ASSERT (choice_p);
          choice_p->Enable (toggle_button_p->GetValue ());

          wxGauge* gauge_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("gauge_progress"),
                     wxGauge);
          ACE_ASSERT (gauge_p);
          gauge_p->SetValue (0);
          gauge_p->Enable (false);

          finished_out = true;

          break;
        }
        case COMMON_UI_EVENT_RESET:
        {
          enum Stream_Visualization_VideoRenderer renderer_e =
            STREAM_VISUALIZATION_VIDEORENDERER_INVALID;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          switch (CBData_in->mediaFramework)
          {
            case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
            {
              ACE_ASSERT (directshow_cb_data_p);
              ACE_ASSERT (directshow_cb_data_p->configuration);
              renderer_e =
                directshow_cb_data_p->configuration->streamConfiguration.configuration_.renderer;
              direct3DConfiguration_p =
                &directshow_cb_data_p->configuration->direct3DConfiguration;
              break;
            }
            case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
            {
              ACE_ASSERT (mediafoundation_cb_data_p);
              ACE_ASSERT (mediafoundation_cb_data_p->configuration);
              renderer_e =
                mediafoundation_cb_data_p->configuration->streamConfiguration.configuration_.renderer;
              break;
            }
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                          CBData_in->mediaFramework));
              return;
            }
          } // end SWITCH
#else
          ACE_ASSERT (cb_data_p);
          ACE_ASSERT (cb_data_p->configuration);
          renderer_e =
            cb_data_p->configuration->streamConfiguration.configuration_.renderer;
#endif // ACE_WIN32 || ACE_WIN64
          switch (renderer_e)
          {
            case STREAM_VISUALIZATION_VIDEORENDERER_NULL:
              break;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_2D:
              break;
            case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D:
            {
              ACE_ASSERT (direct3DConfiguration_p);
              ACE_ASSERT (direct3DConfiguration_p->handle);
              if (!Stream_MediaFramework_DirectDraw_Tools::reset (direct3DConfiguration_p->handle,
                                                                  *direct3DConfiguration_p))
              {
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("failed to Stream_MediaFramework_DirectDraw_Tools::reset(), returning\n")));
                return;
              } // end IF
              break;
            }
            case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTSHOW:
            case STREAM_VISUALIZATION_VIDEORENDERER_GDI:
            case STREAM_VISUALIZATION_VIDEORENDERER_MEDIAFOUNDATION:
              break;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GTK_SUPPORT)
            case STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO:
            case STREAM_VISUALIZATION_VIDEORENDERER_GTK_PIXBUF:
              break;
#endif // GTK_SUPPORT
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("invalid/unknown video renderer (was: %d), returning\n"),
                          renderer_e));
              return;
            }
          } // end SWITCH
          break;
        }
        case COMMON_UI_EVENT_STATISTIC:
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spinctrl_frames_captured"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (CBData_in->progressData.statistic.capturedFrames);
#endif // ACE_WIN32 || ACE_WIN64
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spinctrl_frames_dropped"),
                     wxSpinCtrl);
          ACE_ASSERT (spin_control_p);
          spin_control_p->SetValue (CBData_in->progressData.statistic.droppedFrames);
          spin_control_p =
            XRCCTRL (*dialog_p,
                     ACE_TEXT_ALWAYS_CHAR ("spinctrl_messages_session"),
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
    while (!CBData_in->UIState->eventStack.is_empty ())
    {
      result = CBData_in->UIState->eventStack.pop (event_2);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Unbounded_Stack::pop(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope
}

ACE_THR_FUNC_RETURN
stream_processing_thread (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_processing_thread"));

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("stream processing thread (id: %t) starting\n")));
#endif // _DEBUG

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = std::numeric_limits<unsigned long>::max ();
#else
  result = arg_in;
#endif // ACE_WIN32 || ACE_WIN64

  struct Stream_ImageScreen_UI_ThreadData* thread_data_p =
      static_cast<struct Stream_ImageScreen_UI_ThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (thread_data_p);
  ACE_ASSERT (thread_data_p->CBData);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  const Stream_ImageScreen_DirectShow_SessionData_t* directshow_session_data_container_p =
    NULL;
  const Stream_ImageScreen_MediaFoundation_SessionData_t* mediafoundation_session_data_container_p =
    NULL;
  const Stream_ImageScreen_DirectShow_SessionData* directshow_session_data_p =
    NULL;
  const Stream_ImageScreen_MediaFoundation_SessionData* mediafoundation_session_data_p =
    NULL;
#else
  const Stream_ImageScreen_V4L_SessionData_t* session_data_container_p = NULL;
  const Stream_ImageScreen_V4L_SessionData* session_data_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64
  Stream_IStreamControlBase* stream_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_ImageScreen_DirectShow_UI_CBData* directshow_cb_data_p = NULL;
  struct Stream_ImageScreen_MediaFoundation_UI_CBData* mediafoundation_cb_data_p =
    NULL;
  switch (thread_data_p->CBData->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_ImageScreen_DirectShow_UI_CBData*> (thread_data_p->CBData);
      ACE_ASSERT (directshow_cb_data_p->configuration);
      ACE_ASSERT (directshow_cb_data_p->stream);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_ImageScreen_MediaFoundation_UI_CBData*> (thread_data_p->CBData);
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
  struct Stream_ImageScreen_V4L_UI_CBData* cb_data_p =
    static_cast<struct Stream_ImageScreen_V4L_UI_CBData*> (thread_data_p->CBData);
  ACE_ASSERT (cb_data_p->configuration);
  ACE_ASSERT (cb_data_p->stream);
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (thread_data_p->CBData->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      Stream_ImageScreen_DirectShow_StreamConfiguration_t::ITERATOR_T iterator =
        const_cast<Stream_ImageScreen_DirectShow_StreamConfiguration_t::ITERATOR_T&> (directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR ("")));
      ACE_ASSERT (iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());

      if (!directshow_cb_data_p->stream->initialize (directshow_cb_data_p->configuration->streamConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize stream, aborting\n")));
        goto error;
      } // end IF
      stream_p = directshow_cb_data_p->stream;
      directshow_session_data_container_p =
        &directshow_cb_data_p->stream->getR ();
      ACE_ASSERT (directshow_session_data_container_p);
      directshow_session_data_p = &directshow_session_data_container_p->getR ();
      ACE_ASSERT (directshow_session_data_p);
      thread_data_p->sessionId = directshow_session_data_p->sessionId;
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
      mediafoundation_session_data_container_p =
        &mediafoundation_cb_data_p->stream->getR ();
      ACE_ASSERT (mediafoundation_session_data_container_p);
      mediafoundation_session_data_p =
        &mediafoundation_session_data_container_p->getR ();
      ACE_ASSERT (mediafoundation_session_data_p);
      thread_data_p->sessionId = mediafoundation_session_data_p->sessionId;
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
                ACE_TEXT ("failed to Stream_ImageScreen_V4L_Stream::initialize(), aborting\n")));
    goto error;
  } // end IF
  stream_p = cb_data_p->stream;
  session_data_container_p = &cb_data_p->stream->getR ();
  ACE_ASSERT (session_data_container_p);
  session_data_p = &session_data_container_p->getR ();
  ACE_ASSERT (session_data_p);
  thread_data_p->sessionId = session_data_p->sessionId;
#endif // ACE_WIN32 || ACE_WIN64

  // *NOTE*: blocks until 'finished'
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
//wxIMPLEMENT_DYNAMIC_CLASS (Stream_ImageScreen_DirectShow_WxWidgetsDialog_t, dialog_main)
wxClassInfo
Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::ms_classInfo (L"Stream_ImageScreen_DirectShow_WxWidgetsDialog_t",
                                                           &dialog_main::ms_classInfo,
                                                           NULL,
                                                           (int) sizeof (Stream_ImageScreen_DirectShow_WxWidgetsDialog_t),
                                                           Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::wxCreateObject);
wxClassInfo*
Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::GetClassInfo () const
{
  return &Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::ms_classInfo;
}

wxObject*
Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::wxCreateObject ()
{
  return new Stream_ImageScreen_DirectShow_WxWidgetsDialog_t;
}

//wxIMPLEMENT_DYNAMIC_CLASS (Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t, dialog_main)
wxClassInfo
Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::ms_classInfo (L"Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t",
                                                                &dialog_main::ms_classInfo,
                                                                NULL,
                                                                (int) sizeof (Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t),
                                                                Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::wxCreateObject);
wxClassInfo*
Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::GetClassInfo () const
{
  return &Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::ms_classInfo;
}

wxObject*
Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::wxCreateObject ()
{
  return new Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t;
}

wxBEGIN_EVENT_TABLE (Stream_ImageScreen_DirectShow_WxWidgetsDialog_t, dialog_main)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_record"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::togglebutton_record_toggled_cb)
 EVT_BUTTON (XRCID ("button_snapshot"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::button_snapshot_clicked_cb)
 EVT_BUTTON (XRCID ("button_cut"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::button_cut_clicked_cb)
#if defined (_DEBUG)
 EVT_BUTTON (XRCID ("button_report"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::button_report_clicked_cb)
#endif // _DEBUG
 EVT_CHOICE (XRCID ("choice_source"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::choice_source_changed_cb)
 EVT_BUTTON (XRCID ("button_camera_properties"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::button_camera_properties_clicked_cb)
 EVT_CHOICE (XRCID ("choice_format"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::choice_format_changed_cb)
 EVT_CHOICE (XRCID ("choice_resolution"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::choice_resolution_changed_cb)
 EVT_CHOICE (XRCID ("choice_framerate"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::choice_framerate_changed_cb)
 EVT_BUTTON (XRCID ("button_reset"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::button_reset_clicked_cb)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_save"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::togglebutton_save_toggled_cb)
 EVT_DIRPICKER_CHANGED (XRCID ("directorypicker_save"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::picker_directory_save_changed_cb)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_display"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::togglebutton_display_toggled_cb)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_fullscreen"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::togglebutton_fullscreen_toggled_cb)
 EVT_CHOICE (XRCID ("choice_displayadapter"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::choice_displayadapter_changed_cb)
 EVT_CHOICE (XRCID ("choice_screen"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::choice_screen_changed_cb)
 EVT_BUTTON (XRCID ("button_display_settings"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::button_display_settings_clicked_cb)
 EVT_CHOICE (XRCID ("choice_resolution_2"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::choice_resolution_2_changed_cb)
 EVT_BUTTON (XRCID ("button_about"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::button_about_clicked_cb)
 EVT_BUTTON (XRCID ("button_quit"), Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::button_quit_clicked_cb)
 EVT_IDLE (Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::dialog_main_idle_cb)
 EVT_CHAR_HOOK (Stream_ImageScreen_DirectShow_WxWidgetsDialog_t::dialog_main_keydown_cb)
wxEND_EVENT_TABLE ()

wxBEGIN_EVENT_TABLE (Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t, dialog_main)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_record"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::togglebutton_record_toggled_cb)
 EVT_BUTTON (XRCID ("button_snapshot"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::button_snapshot_clicked_cb)
 EVT_BUTTON (XRCID ("button_cut"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::button_cut_clicked_cb)
#if defined (_DEBUG)
 EVT_BUTTON (XRCID ("button_report"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::button_report_clicked_cb)
#endif // _DEBUG
 EVT_CHOICE (XRCID ("choice_source"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::choice_source_changed_cb)
 EVT_BUTTON (XRCID ("button_camera_properties"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::button_camera_properties_clicked_cb)
 EVT_CHOICE (XRCID ("choice_format"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::choice_format_changed_cb)
 EVT_CHOICE (XRCID ("choice_resolution"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::choice_resolution_changed_cb)
 EVT_CHOICE (XRCID ("choice_framerate"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::choice_framerate_changed_cb)
 EVT_BUTTON (XRCID ("button_reset"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::button_reset_clicked_cb)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_save"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::togglebutton_save_toggled_cb)
 EVT_DIRPICKER_CHANGED (XRCID ("directorypicker_save"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::picker_directory_save_changed_cb)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_display"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::togglebutton_display_toggled_cb)
 EVT_TOGGLEBUTTON (XRCID ("togglebutton_fullscreen"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::togglebutton_fullscreen_toggled_cb)
 EVT_CHOICE (XRCID ("choice_displayadapter"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::choice_displayadapter_changed_cb)
 EVT_CHOICE (XRCID ("choice_screen"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::choice_screen_changed_cb)
 EVT_BUTTON (XRCID ("button_display_settings"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::button_display_settings_clicked_cb)
 EVT_CHOICE (XRCID ("choice_resolution_2"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::choice_resolution_2_changed_cb)
 EVT_BUTTON (XRCID ("button_about"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::button_about_clicked_cb)
 EVT_BUTTON (XRCID ("button_quit"), Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::button_quit_clicked_cb)
 EVT_IDLE (Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::dialog_main_idle_cb)
 EVT_CHAR_HOOK (Stream_ImageScreen_MediaFoundation_WxWidgetsDialog_t::dialog_main_keydown_cb)
wxEND_EVENT_TABLE ()
#else
//wxIMPLEMENT_DYNAMIC_CLASS (Stream_ImageScreen_V4L_WxWidgetsDialog_t, wxDialog_main)
wxClassInfo
Stream_ImageScreen_V4L_WxWidgetsDialog_t::ms_classInfo (ACE_TEXT_ALWAYS_WCHAR ("Stream_ImageScreen_V4L_WxWidgetsDialog_t"),
                                                    &wxDialog_main::ms_classInfo,
                                                    NULL,
                                                    (int) sizeof (Stream_ImageScreen_V4L_WxWidgetsDialog_t),
                                                    Stream_ImageScreen_V4L_WxWidgetsDialog_t::wxCreateObject);
wxClassInfo*
Stream_ImageScreen_V4L_WxWidgetsDialog_t::GetClassInfo () const
{
  return &Stream_ImageScreen_V4L_WxWidgetsDialog_t::ms_classInfo;
}

wxObject*
Stream_ImageScreen_V4L_WxWidgetsDialog_t::wxCreateObject ()
{
  return new Stream_ImageScreen_V4L_WxWidgetsDialog_t;
}

//wxBEGIN_EVENT_TABLE (Stream_ImageScreen_V4L_WxWidgetsDialog_t, wxDialog_main)
// EVT_TOGGLEBUTTON (XRCID ("togglebutton_record"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::togglebutton_record_toggled_cb)
// EVT_BUTTON (XRCID ("button_snapshot"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::button_snapshot_clicked_cb)
// EVT_BUTTON (XRCID ("button_cut"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::button_cut_clicked_cb)
//#if defined (_DEBUG)
// EVT_BUTTON (XRCID ("button_report"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::button_report_clicked_cb)
//#endif // _DEBUG
// EVT_CHOICE (XRCID ("choice_source"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::choice_source_changed_cb)
// EVT_BUTTON (XRCID ("button_camera_properties"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::button_camera_properties_clicked_cb)
// EVT_CHOICE (XRCID ("choice_format"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::choice_format_changed_cb)
// EVT_CHOICE (XRCID ("choice_resolution"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::choice_resolution_changed_cb)
// EVT_CHOICE (XRCID ("choice_framerate"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::choice_framerate_changed_cb)
// EVT_BUTTON (XRCID ("button_reset"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::button_reset_clicked_cb)
// EVT_TOGGLEBUTTON (XRCID ("togglebutton_save"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::togglebutton_save_toggled_cb)
// EVT_DIRPICKER_CHANGED (XRCID ("directorypicker_save"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::picker_directory_save_changed_cb)
// EVT_TOGGLEBUTTON (XRCID ("togglebutton_display"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::togglebutton_display_toggled_cb)
// EVT_TOGGLEBUTTON (XRCID ("togglebutton_fullscreen"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::togglebutton_fullscreen_toggled_cb)
// EVT_CHOICE (XRCID ("choice_displayadapter"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::choice_displayadapter_changed_cb)
// EVT_CHOICE (XRCID ("choice_screen"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::choice_screen_changed_cb)
// EVT_BUTTON (XRCID ("button_display_settings"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::button_display_settings_clicked_cb)
// EVT_CHOICE (XRCID ("choice_resolution_2"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::choice_resolution_2_changed_cb)
// EVT_BUTTON (XRCID ("button_about"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::button_about_clicked_cb)
// EVT_BUTTON (XRCID ("button_quit"), Stream_ImageScreen_V4L_WxWidgetsDialog_t::button_quit_clicked_cb)
// EVT_IDLE (Stream_ImageScreen_V4L_WxWidgetsDialog_t::dialog_main_idle_cb)
// EVT_CHAR_HOOK (Stream_ImageScreen_V4L_WxWidgetsDialog_t::dialog_main_keydown_cb)
//wxEND_EVENT_TABLE ()
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//////////////////////////////////////////

Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::Stream_ImageScreen_WxWidgetsDialog_T (wxWindow* parent_in)
 : inherited (parent_in, wxID_ANY, wxEmptyString)
 , application_ (NULL)
 , initializing_ (true)
 , untoggling_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::Stream_ImageScreen_WxWidgetsDialog_T"));

}

bool
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::OnInit_2 (IAPPLICATION_T* iapplication_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::OnInit_2"));

  // sanity check(s)
  ACE_ASSERT (!application_);

  application_ =
    dynamic_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t*> (iapplication_in);
  ACE_ASSERT (application_);

  togglebutton_record = XRCCTRL (*this, "togglebutton_record", wxToggleButton);
  button_snapshot = XRCCTRL (*this, "button_snapshot", wxButton);
  button_cut = XRCCTRL (*this, "button_cut", wxButton);
  button_report = XRCCTRL (*this, "button_report", wxButton);
  spinctrl_control = XRCCTRL (*this, "spinctrl_control", wxSpinCtrl);
  spinctrl_session = XRCCTRL (*this, "spinctrl_session", wxSpinCtrl);
  spinctrl_data = XRCCTRL (*this, "spinctrl_data", wxSpinCtrl);
  spinctrl_payload = XRCCTRL (*this, "spinctrl_payload", wxSpinCtrl);
  spinctrl_framesize = XRCCTRL (*this, "spinctrl_framesize", wxSpinCtrl);
  button_reset_camera = XRCCTRL (*this, "button_reset_camera", wxButton);
  choice_source = XRCCTRL (*this, "choice_source", wxChoice);
  button_camera_properties = XRCCTRL (*this, "button_camera_properties", wxButton);
  choice_format = XRCCTRL (*this, "choice_format", wxChoice);
  choice_resolution = XRCCTRL (*this, "choice_resolution", wxChoice);
  choice_framerate = XRCCTRL (*this, "choice_framerate", wxChoice);
  togglebutton_display = XRCCTRL (*this, "togglebutton_display", wxToggleButton);
  togglebutton_fullscreen = XRCCTRL (*this, "togglebutton_fullscreen", wxToggleButton);
  choice_displayadapter = XRCCTRL (*this, "choice_displayadapter", wxChoice);
  choice_screen = XRCCTRL (*this, "choice_screen", wxChoice);
//  button_display_settings = XRCCTRL (*this, "button_display_settings", wxButton);
  choice_resolution_2 = XRCCTRL (*this, "choice_resolution_2", wxChoice);
  togglebutton_save = XRCCTRL (*this, "togglebutton_save", wxToggleButton);
  textctrl_filename = XRCCTRL (*this, "textctrl_filename", wxTextCtrl);
//  directorypicker_save = XRCCTRL (*this, "directorypicker_save", wxDirPickerCtrl);
  panel_video = XRCCTRL (*this, "panel_video", wxPanel);
  gauge_progress = XRCCTRL (*this, "gauge_progress", wxGauge);
  button_about = XRCCTRL (*this, "button_about", wxButton);
  button_quit = XRCCTRL (*this, "button_quit", wxButton);

  this->SetDefaultItem (togglebutton_record);

  // populate controls
#if defined (_DEBUG)
#else
  button_report->Show (false);
#endif // _DEBUG
  spinctrl_control->SetRange (0,
                              std::numeric_limits<int>::max ());
  spinctrl_session->SetRange (0,
                              std::numeric_limits<int>::max ());
  spinctrl_data->SetRange (0,
                           std::numeric_limits<int>::max ());
  spinctrl_payload->SetRange (0,
                              std::numeric_limits<int>::max ());
  spinctrl_framesize->SetRange (0,
                                std::numeric_limits<int>::max ());

  bool activate_source_b = true, activate_display_b = true;
  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator_2 =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (Stream_Visualization_Tools::rendererToModuleName (configuration_r.configuration->streamConfiguration.configuration_.renderer).c_str ()));
  ACE_ASSERT (stream_iterator_2 != configuration_r.configuration->streamConfiguration.end ());

  Stream_Device_List_t devices_a =
      Stream_Device_Tools::getVideoCaptureDevices ();
  wxStringClientData* client_data_p = NULL;
  int index_i = -1;
  for (Stream_Device_ListIterator_t iterator = devices_a.begin ();
       iterator != devices_a.end ();
       ++iterator)
  {
    client_data_p = NULL;
    ACE_NEW_NORETURN (client_data_p,
                      wxStringClientData ());
    ACE_ASSERT (client_data_p);
    client_data_p->SetData ((*iterator).identifier);
    index_i =
      choice_source->Append ((*iterator).description.c_str (),
                             client_data_p);
  } // end FOR
  if (unlikely (devices_a.empty ()))
    activate_source_b = false;
  else
    choice_source->Enable (true);

  togglebutton_save->Enable (!(*stream_iterator).second.second.targetFileName.empty ());
  togglebutton_save->SetValue (!(*stream_iterator).second.second.targetFileName.empty ());
  textctrl_filename->Enable (togglebutton_save->GetValue ());
  textctrl_filename->SetValue (ACE_TEXT_ALWAYS_CHAR (ACE::basename ((*stream_iterator).second.second.targetFileName.c_str (),
                                                                    ACE_DIRECTORY_SEPARATOR_CHAR)));
//  directorypicker_save->Enable (togglebutton_save->GetValue ());
//  directorypicker_save->SetPath (ACE_TEXT_ALWAYS_CHAR (ACE::dirname ((*stream_iterator).second.second.targetFileName.c_str (),
//                                                                     ACE_DIRECTORY_SEPARATOR_CHAR)));

  Common_UI_DisplayAdapters_t display_adapters_a =
    Common_UI_Tools::getAdapters ();
  for (Common_UI_DisplayAdaptersIterator_t iterator = display_adapters_a.begin ();
       iterator != display_adapters_a.end ();
       ++iterator)
  {
    client_data_p = NULL;
    ACE_NEW_NORETURN (client_data_p,
                      wxStringClientData ());
    ACE_ASSERT (client_data_p);
    client_data_p->SetData ((*iterator).device);

    index_i =
      choice_displayadapter->Append ((*iterator).description.c_str (),
                                     client_data_p);
  } // end FOR

  Common_UI_DisplayDevices_t display_devices_a =
    Common_UI_Tools::getDisplays ();
  for (Common_UI_DisplayDevicesIterator_t iterator = display_devices_a.begin ();
       iterator != display_devices_a.end ();
       ++iterator)
  {
    client_data_p = NULL;
    ACE_NEW_NORETURN (client_data_p,
                      wxStringClientData ());
    ACE_ASSERT (client_data_p);
    client_data_p->SetData ((*iterator).device);

    index_i =
      choice_screen->Append ((*iterator).description.c_str (),
                              client_data_p);
  } // end FOR
  if (likely (!display_devices_a.empty ()))
  {
    togglebutton_display->Enable (!(*stream_iterator_2).second.second.deviceIdentifier.identifier.empty ());
    togglebutton_display->SetValue (!(*stream_iterator_2).second.second.deviceIdentifier.identifier.empty ());
    togglebutton_fullscreen->Enable (togglebutton_display->GetValue ());
    togglebutton_fullscreen->SetValue ((*stream_iterator_2).second.second.fullScreen);
    panel_video->Show (togglebutton_display->GetValue () &&
                       !togglebutton_fullscreen->GetValue ());
  } // end IF
  if (unlikely (devices_a.empty ()))
    activate_display_b = false;
  else
    choice_screen->Enable (true);

  wxCommandEvent* event_p = NULL;
  if (likely (activate_source_b))
  { ACE_ASSERT (!(*stream_iterator).second.second.deviceIdentifier.identifier.empty ());
    index_i =
      Common_UI_WxWidgets_Tools::clientDataToIndex (choice_source,
                                                    (*stream_iterator).second.second.deviceIdentifier.identifier);
    ACE_ASSERT (index_i != wxNOT_FOUND);
    choice_source->Select (index_i);

    ACE_NEW_NORETURN (event_p,
                      wxCommandEvent (wxEVT_COMMAND_CHOICE_SELECTED,
                                      XRCID ("choice_source")));
    ACE_ASSERT (event_p);
    event_p->SetInt (index_i);
    this->QueueEvent (event_p);
    event_p = NULL;
  } // end IF
  application_->wait ();

  if (likely (activate_display_b))
  { ACE_ASSERT (!(*stream_iterator_2).second.second.display.device.empty ());
    index_i =
      Common_UI_WxWidgets_Tools::clientDataToIndex (choice_screen,
                                                    (*stream_iterator_2).second.second.display.device);
    ACE_ASSERT (index_i != wxNOT_FOUND);
    choice_screen->Select (index_i);

    ACE_NEW_NORETURN (event_p,
                      wxCommandEvent (wxEVT_COMMAND_CHOICE_SELECTED,
                                      XRCID ("choice_screen")));
    ACE_ASSERT (event_p);
    event_p->SetInt (index_i);
    this->QueueEvent (event_p);
    event_p = NULL;

    ACE_NEW_NORETURN (event_p,
                      wxCommandEvent (wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
                                      XRCID ("togglebutton_display")));
    ACE_ASSERT (event_p);
    event_p->SetInt ((*stream_iterator_2).second.second.display.device.empty () ? 0
                                                                                : 1);
    this->QueueEvent (event_p);
    event_p = NULL;

    ACE_NEW_NORETURN (event_p,
                      wxCommandEvent (wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
                                      XRCID ("togglebutton_fullscreen")));
    ACE_ASSERT (event_p);
    event_p->SetInt ((*stream_iterator_2).second.second.fullScreen ? 1
                                                                   : 0);
    this->QueueEvent (event_p);
    event_p = NULL;
  } // end IF

  return true;
}
void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::OnExit_2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::OnExit_2"));

  // sanity check(s)
  ACE_ASSERT (application_);
}

//////////////////////////////////////////

void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::dialog_main_idle_cb (wxIdleEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::dialog_main_idle_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);
  bool finished_b = false;

  process_stream_events (&configuration_r,
                         finished_b);
  if (!finished_b &&
      configuration_r.stream->isRunning ())
    gauge_progress->Pulse ();
}

void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::dialog_main_keydown_cb (wxKeyEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::dialog_main_keydown_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);

  wxCommandEvent* event_p = NULL;
  switch (event_in.GetUnicodeKey ())
  {
    // It's a "normal" character. Notice that this includes control characters
    // in 1..31 range, e.g. WXK_RETURN or WXK_BACK, so check for them explicitly
    case 'c':
    case 'C':
    {
      // sanity check(s)
      if (!configuration_r.stream->isRunning ())
        return; // nothing to do

      ACE_NEW_NORETURN (event_p,
                        wxCommandEvent (wxEVT_COMMAND_BUTTON_CLICKED,
                                        XRCID ("button_cut")));
      ACE_ASSERT (event_p);
      event_p->SetInt (1);
      this->QueueEvent (event_p);
      event_p = NULL;
      break;
    }
    case 'f':
    case 'F':
    case WXK_ESCAPE:
    {
      // *NOTE*: escape does nothing when not fullscreen
      if ((event_in.GetUnicodeKey () == WXK_ESCAPE) &&
          !togglebutton_fullscreen->GetValue ())
        break;
      bool is_checked_b = togglebutton_fullscreen->GetValue ();
      togglebutton_fullscreen->SetValue (!is_checked_b);

      ACE_NEW_NORETURN (event_p,
                        wxCommandEvent (wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
                                        XRCID ("togglebutton_fullscreen")));
      ACE_ASSERT (event_p);
      event_p->SetInt (is_checked_b ? 0
                                    : 1);
      this->QueueEvent (event_p);
      event_p = NULL;
      break;
    }
    case 'r':
    case 'R':
    {
      bool is_checked_b = togglebutton_record->GetValue ();
      togglebutton_record->SetValue (!is_checked_b);

      ACE_NEW_NORETURN (event_p,
                        wxCommandEvent (wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
                                        XRCID ("togglebutton_record")));
      ACE_ASSERT (event_p);
      event_p->SetInt (is_checked_b ? 0
                                    : 1);
      this->QueueEvent (event_p);
      event_p = NULL;
      break;
    }
    case 's':
    case 'S':
    {
      // sanity check(s)
      if (!configuration_r.stream->isRunning ())
        return; // nothing to do

      ACE_NEW_NORETURN (event_p,
                        wxCommandEvent (wxEVT_COMMAND_BUTTON_CLICKED,
                                        XRCID ("button_snapshot")));
      ACE_ASSERT (event_p);
      event_p->SetInt (1);
      this->QueueEvent (event_p);
      event_p = NULL;
      break;
    }
    //////////////////////////////////////
    case WXK_NONE: // no character value (i.e. control characters)
    {
      switch (event_in.GetKeyCode ())
      {
        //case :
        //{
        //  break;
        //}
        default:
          break;
      } // end SWITCH
      break;
    }
    default:
      break;
  } // end SWITCH
}

//////////////////////////////////////////

void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::togglebutton_record_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::togglebutton_record_toggled_cb"));

  // handle untoggle --> PLAY
  if (untoggling_)
  {
    untoggling_ = false;
    return; // done
  } // end IF

  // sanity check(s)
  ACE_ASSERT (application_);

  // --> user pressed play/pause/stop
  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator_2 =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (Stream_Visualization_Tools::rendererToModuleName (configuration_r.configuration->streamConfiguration.configuration_.renderer).c_str ()));
  ACE_ASSERT (stream_iterator_2 != configuration_r.configuration->streamConfiguration.end ());

  Stream_IStreamControlBase* stream_p = configuration_r.stream;
  ACE_ASSERT (stream_p);

  // toggle ?
  if (!event_in.IsChecked ())
  { // --> user pressed pause/stop
    ACE_ASSERT (stream_p->isRunning ());
    stream_p->stop (false, // wait ?
                    true,  // recurse upstream ?
                    true); // locked access ?

    // modify controls
    togglebutton_record->Enable (false);

    return;
  } // end IF

  // --> user pressed record

  // step1: reset progress reporting
  ACE_OS::memset (&configuration_r.progressData.statistic,
                  0,
                  sizeof (struct Stream_ImageScreen_StatisticData));

  // step2: update configuration
  // step2a: update capture device configuration
  configuration_r.configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize =
    spinctrl_framesize->GetValue ();

  // *NOTE*: the capture format has been updated already (see:
  //         choice_framerate_changed_cb())
  //wxStringClientData* client_data_p =
  //  dynamic_cast<wxStringClientData*> (choice_source->GetClientObject (choice_source->GetSelection ()));
  //ACE_ASSERT (client_data_p);
  //(*stream_iterator).second.second.deviceIdentifier =
  //  client_data_p->GetData ().ToStdString ();
  //client_data_p =
  //  dynamic_cast<wxStringClientData*> (choice_format->GetClientObject (choice_format->GetSelection ()));
  //ACE_ASSERT (client_data_p);
  //struct _GUID format_s =
  //  Common_Tools::StringToGUID (client_data_p->GetData ().ToStdString ());
  //ACE_ASSERT (!InlineIsEqualGUID (format_s, GUID_NULL));
  //client_data_p =
  //  dynamic_cast<wxStringClientData*> (choice_resolution->GetClientObject (choice_resolution->GetSelection ()));
  //ACE_ASSERT (client_data_p);
  //std::istringstream converter;
  //converter.str (client_data_p->GetData ().ToStdString ());
  //Common_UI_Resolution_t resolution_s;
  //converter >> resolution_s.cx;
  //converter >> resolution_s.cy;
  //converter.clear ();
  //converter.str (choice_framerate->GetString (choice_framerate->GetSelection ()).ToStdString ());
  //unsigned int framerate_i = 0;
  //converter >> framerate_i;

//  Common_UI_Resolution_t resolution_s;
//  resolution_s.width =
//      configuration_r.configuration->streamConfiguration.configuration_.format.width;
//  resolution_s.height =
//      configuration_r.configuration->streamConfiguration.configuration_.format.height;

  // step2b: update save configuration
  std::string filename_string;
  if (!togglebutton_save->GetValue ())
    goto continue_;
  filename_string =
//      directorypicker_save->GetPath ();
      Common_File_Tools::getTempDirectory ();
  ACE_ASSERT (Common_File_Tools::isDirectory (filename_string));
  ACE_ASSERT (Common_File_Tools::isWriteable (filename_string));
  filename_string += ACE_DIRECTORY_SEPARATOR_STR;
  filename_string += textctrl_filename->GetValue ();
  ACE_ASSERT (Common_File_Tools::isValidPath (filename_string));
continue_:
  (*stream_iterator).second.second.targetFileName = filename_string;

  // step2c: update display configuration
  if (togglebutton_display->GetValue ())
  {
  //  wxRect rectangle_s = panel_video->GetClientRect ();
  //  (*stream_iterator).second.second.area.left = rectangle_s.GetX ();
  //  (*stream_iterator).second.second.area.right =
  //    (*stream_iterator).second.second.area.left + rectangle_s.GetWidth ();
  //  (*stream_iterator).second.second.area.top = rectangle_s.GetY ();
  //  (*stream_iterator).second.second.area.bottom =
  //    (*stream_iterator).second.second.area.top + rectangle_s.GetHeight ();
  //  (*stream_iterator_2).second.second.area =
  //    (*stream_iterator).second.second.area;
  //  client_data_p =
  //    dynamic_cast<wxStringClientData*> (choice_screen->GetClientObject (choice_screen->GetSelection ()));
  //  ACE_ASSERT (client_data_p);
  //  (*stream_iterator_2).second.second.deviceIdentifier =
  //    client_data_p->GetData ().ToStdString ();
  } // end IF
  else
  {
  //  ACE_OS::memset (&(*stream_iterator_2).second.second.area, 0, sizeof (struct tagRECT));
  //  (*stream_iterator).second.second.area =
  //    (*stream_iterator_2).second.second.area;
  //  (*stream_iterator_2).second.second.deviceIdentifier.clear ();
  //  //ACE_ASSERT ((*stream_iterator_2).second.second.direct3DConfiguration);
  } // end ELSE
  if (togglebutton_fullscreen->GetValue ())
  {
  //  struct Common_UI_DisplayDevice display_device_s =
  //    Common_UI_Tools::getDisplayDevice ((*stream_iterator).second.second.deviceIdentifier);
  //  (*stream_iterator_2).second.second.area = display_device_s.clippingArea;
  //  (*stream_iterator_2).second.second.fullScreen = true;
  } // end IF
  else
  {
  //  wxRect rectangle_s = panel_video->GetClientRect ();
  //  (*stream_iterator_2).second.second.area.left = rectangle_s.GetX ();
  //  (*stream_iterator_2).second.second.area.right =
  //    (*stream_iterator_2).second.second.area.left + rectangle_s.GetWidth ();
  //  (*stream_iterator_2).second.second.area.top = rectangle_s.GetY ();
  //  (*stream_iterator_2).second.second.area.bottom =
  //    (*stream_iterator_2).second.second.area.top + rectangle_s.GetHeight ();
  //  (*stream_iterator_2).second.second.fullScreen = false;
  } // end ELSE

  // step3: set up device ?
  switch (configuration_r.configuration->streamConfiguration.configuration_.renderer)
  {
    case STREAM_VISUALIZATION_VIDEORENDERER_NULL:
      break;
    case STREAM_VISUALIZATION_VIDEORENDERER_X11:
    {
      break;
    }
#if defined (GTK_USE)
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO:
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_PIXBUF:
      break;
#endif // GTK_USE
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown video renderer (was: %d), aborting\n"),
                  configuration_r.configuration->streamConfiguration.configuration_.renderer));
      return;
    }
  } // end SWITCH

  // step4: start processing thread(s)
  ACE_Thread_ID thread_id_2;
  bool result =
    Common_Test_U_Tools::spawn<struct Stream_ImageScreen_UI_ThreadData,
                               Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T> (ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_THREAD_NAME),
                                                                                             ::stream_processing_thread,
                                                                                             COMMON_EVENT_REACTOR_THREAD_GROUP_ID + 1,
                                                                                             configuration_r,
                                                                                             thread_id_2);
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Test_U_Tools::spawn(): \"%m\", returning\n")));
    return;
  } // end IF

  // step5: modify controls
  button_snapshot->Enable (true);
  button_cut->Enable (togglebutton_save->GetValue ());
#if defined (_DEBUG)
  button_report->Enable (true);
#endif // _DEBUG
  choice_source->Enable (false);
  choice_format->Enable (false);
  choice_resolution->Enable (false);
  choice_framerate->Enable (false);
  button_reset_camera->Enable (false);
  togglebutton_save->Enable (false);
  textctrl_filename->Enable (false);
//  directorypicker_save->Enable (false);
  togglebutton_display->Enable (false);
  choice_screen->Enable (false);
  gauge_progress->Enable (true);
}
void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::button_snapshot_clicked_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::button_snapshot_clicked_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);

  configuration_r.stream->control (STREAM_CONTROL_STEP_2);
}
void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::button_cut_clicked_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::button_cut_clicked_cb"));

}
void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::button_report_clicked_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::button_report_clicked_cb"));

}
void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::choice_source_changed_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::choice_source_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  std::string device_identifier;
  wxStringClientData* client_data_p =
    dynamic_cast<wxStringClientData*> (choice_source->GetClientObject (event_in.GetSelection ()));
  ACE_ASSERT (client_data_p);
  device_identifier = client_data_p->GetData ().ToStdString ();
  ACE_ASSERT (!device_identifier.empty ());

  if ((*stream_iterator).second.second.deviceIdentifier.fileDescriptor != -1)
  {
    int result = v4l2_close ((*stream_iterator).second.second.deviceIdentifier.fileDescriptor);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", returning\n"),
                  (*stream_iterator).second.second.deviceIdentifier.fileDescriptor));
      return;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("closed video device (fd was: %d)\n"),
                (*stream_iterator).second.second.deviceIdentifier.fileDescriptor));
#endif // _DEBUG
    (*stream_iterator).second.second.deviceIdentifier.fileDescriptor = -1;
  } // end IF
  ACE_ASSERT ((*stream_iterator).second.second.deviceIdentifier.fileDescriptor == -1);
  int open_mode_i = O_RDONLY;
  (*stream_iterator).second.second.deviceIdentifier.fileDescriptor =
      v4l2_open (ACE_TEXT_ALWAYS_CHAR (device_identifier.c_str ()),
                 open_mode_i);
  if (unlikely ((*stream_iterator).second.second.deviceIdentifier.fileDescriptor == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_open(\"%s\",%d): \"%m\", returning\n"),
                ACE_TEXT (device_identifier.c_str ()),
                open_mode_i));
    return;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("opened video device \"%s\" (fd: %d)\n"),
              ACE_TEXT (device_identifier.c_str ()),
              (*stream_iterator).second.second.deviceIdentifier.fileDescriptor));
#endif // _DEBUG

  choice_format->SetSelection (wxNOT_FOUND);
  choice_format->Clear ();
  Stream_MediaFramework_V4L_CaptureFormats_t subformats_a =
    Stream_Device_Tools::getCaptureSubFormats ((*stream_iterator).second.second.deviceIdentifier.fileDescriptor);
  ACE_ASSERT (!subformats_a.empty ());
  std::stringstream converter;
  int index_i = -1;
  for (Stream_MediaFramework_V4L_CaptureFormatsIterator_t iterator = subformats_a.begin ();
       iterator != subformats_a.end ();
       ++iterator)
  {
    client_data_p = NULL;
    ACE_NEW_NORETURN (client_data_p,
                      wxStringClientData ());
    ACE_ASSERT (client_data_p);
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << (*iterator).first;
    client_data_p->SetData (converter.str ());

    index_i =
      choice_format->Append ((*iterator).second.c_str (),
                             client_data_p);
  } // end FOR

  choice_format->Enable (!subformats_a.empty ());
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << configuration_r.configuration->streamConfiguration.configuration_.format.format.pixelformat;
  index_i =
    (initializing_ ? Common_UI_WxWidgets_Tools::clientDataToIndex (choice_format,
                                                                   converter.str ())
                   : 0);
  ACE_ASSERT (index_i != wxNOT_FOUND);
  choice_format->Select (index_i);

  wxCommandEvent* event_p = NULL;
  ACE_NEW_NORETURN (event_p,
                    wxCommandEvent (wxEVT_COMMAND_CHOICE_SELECTED,
                                    XRCID ("choice_format")));
  ACE_ASSERT (event_p);
  event_p->SetInt (index_i);
  this->QueueEvent (event_p);
  event_p = NULL;

  button_camera_properties->Enable (true);
}
void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::button_camera_properties_clicked_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::button_camera_properties_clicked_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
}
void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::choice_format_changed_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::choice_format_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  int index_i = event_in.GetSelection ();
  ACE_ASSERT (index_i != wxNOT_FOUND);
  wxStringClientData* client_data_p =
    dynamic_cast<wxStringClientData*> (choice_format->GetClientObject (index_i));
  ACE_ASSERT (client_data_p);
  std::stringstream converter;
  converter.str (client_data_p->GetData ().ToStdString ());
  __u32 format_i = 0;
  converter >> format_i;
  index_i = wxNOT_FOUND;
  Common_UI_Resolutions_t resolutions_a =
    Stream_Device_Tools::getCaptureResolutions ((*stream_iterator).second.second.deviceIdentifier.fileDescriptor,
                                                format_i);
  ACE_ASSERT (!resolutions_a.empty ());

  choice_resolution->SetSelection (wxNOT_FOUND);
  choice_resolution->Clear ();
  std::string resolution_string;
  for (Common_UI_ResolutionsIterator_t iterator = resolutions_a.begin ();
       iterator != resolutions_a.end ();
       ++iterator)
  {
    client_data_p = NULL;
    ACE_NEW_NORETURN (client_data_p,
                      wxStringClientData ());
    ACE_ASSERT (client_data_p);
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (*iterator).width;
    converter << ACE_TEXT_ALWAYS_CHAR (" ");
    converter << (*iterator).height;
    client_data_p->SetData (converter.str ());

    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (*iterator).width;
    converter << ACE_TEXT_ALWAYS_CHAR (" x ");
    converter << (*iterator).height;

    index_i = choice_resolution->Append (converter.str (),
                                         client_data_p);
    if ((configuration_r.configuration->streamConfiguration.configuration_.format.format.width == (*iterator).width) &&
        (configuration_r.configuration->streamConfiguration.configuration_.format.format.height == (*iterator).height))
      resolution_string = converter.str ();
  } // end FOR
  ACE_ASSERT (!resolution_string.empty ());
  choice_resolution->Enable (!resolutions_a.empty ());
  index_i =
    (initializing_ ? choice_resolution->FindString (resolution_string)
                   : 0);
  choice_resolution->Select (index_i);

  wxCommandEvent* event_p = NULL;
  ACE_NEW_NORETURN (event_p,
                    wxCommandEvent (wxEVT_COMMAND_CHOICE_SELECTED,
                                    XRCID ("choice_resolution")));
  ACE_ASSERT (event_p);
  event_p->SetInt (index_i);
  this->QueueEvent (event_p);
  event_p = NULL;
}
void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::choice_resolution_changed_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::choice_resolution_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  wxStringClientData* client_data_p =
    dynamic_cast<wxStringClientData*> (choice_format->GetClientObject (choice_format->GetSelection ()));
  ACE_ASSERT (client_data_p);
  std::stringstream converter;
  converter.str (client_data_p->GetData ().ToStdString ());
  __u32 format_i = 0;
  converter >> format_i;
  client_data_p =
    dynamic_cast<wxStringClientData*> (choice_resolution->GetClientObject (event_in.GetSelection ()));
  ACE_ASSERT (client_data_p);
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter.str (client_data_p->GetData ().ToStdString ());
  Common_UI_Resolution_t resolution_s;
  converter >> resolution_s.width;
  converter >> resolution_s.height;
  Common_UI_Framerates_t framerates_a =
    Stream_Device_Tools::getCaptureFramerates ((*stream_iterator).second.second.deviceIdentifier.fileDescriptor,
                                               format_i,
                                               resolution_s);
  ACE_ASSERT (!framerates_a.empty ());

  choice_framerate->Clear ();
  int index_i = wxNOT_FOUND;
  std::string framerate_string;
  for (Common_UI_FrameratesConstIterator_t iterator = framerates_a.begin ();
       iterator != framerates_a.end ();
       ++iterator)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (*iterator);

    index_i = choice_framerate->Append (converter.str ());
    if (configuration_r.configuration->streamConfiguration.configuration_.format.frameRate.numerator == *iterator)
      framerate_string = converter.str ();
  } // end FOR
  ACE_ASSERT (!framerate_string.empty ());
  choice_framerate->Enable (!framerates_a.empty ());
  index_i =
    (initializing_ ? choice_framerate->FindString (framerate_string)
                   : 0);
  choice_framerate->Select (index_i);

  wxCommandEvent* event_p = NULL;
  ACE_NEW_NORETURN (event_p,
                    wxCommandEvent (wxEVT_COMMAND_CHOICE_SELECTED,
                                    XRCID ("choice_framerate")));
  ACE_ASSERT (event_p);
  event_p->SetInt (index_i);
  this->QueueEvent (event_p);
  event_p = NULL;
}
void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::choice_framerate_changed_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::choice_framerate_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator_2 =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (Stream_Visualization_Tools::rendererToModuleName (configuration_r.configuration->streamConfiguration.configuration_.renderer).c_str ()));
  ACE_ASSERT (stream_iterator_2 != configuration_r.configuration->streamConfiguration.end ());
  wxStringClientData* client_data_p =
    dynamic_cast<wxStringClientData*> (choice_source->GetClientObject (choice_source->GetSelection ()));
  ACE_ASSERT (client_data_p);
  (*stream_iterator).second.second.deviceIdentifier.identifier =
      client_data_p->GetData ().ToStdString ();
  client_data_p =
    dynamic_cast<wxStringClientData*> (choice_format->GetClientObject (choice_format->GetSelection ()));
  ACE_ASSERT (client_data_p);
  std::stringstream converter;
  converter.str (client_data_p->GetData ().ToStdString ());
  converter >> configuration_r.configuration->streamConfiguration.configuration_.format.format.pixelformat;
  client_data_p =
    dynamic_cast<wxStringClientData*> (choice_resolution->GetClientObject (choice_resolution->GetSelection ()));
  ACE_ASSERT (client_data_p);
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter.str (client_data_p->GetData ().ToStdString ());
  converter >> configuration_r.configuration->streamConfiguration.configuration_.format.format.width;
  converter >> configuration_r.configuration->streamConfiguration.configuration_.format.format.height;
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter.str (choice_framerate->GetString (choice_framerate->GetSelection ()).ToStdString ());
  converter >> configuration_r.configuration->streamConfiguration.configuration_.format.frameRate.numerator;

  // update controls
  if (initializing_)
    togglebutton_record->Enable (true);
  spinctrl_framesize->SetValue (Stream_MediaFramework_Tools::toFrameSize (configuration_r.configuration->streamConfiguration.configuration_.format));
  struct Stream_MediaFramework_V4L_MediaType media_type_s =
      Stream_Device_Tools::defaultCaptureFormat ((*stream_iterator).second.second.deviceIdentifier.identifier);
  button_reset_camera->Enable (!(configuration_r.configuration->streamConfiguration.configuration_.format == media_type_s));
}
void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::button_reset_camera_clicked_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::button_reset_camera_clicked_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());

  struct Stream_MediaFramework_V4L_MediaType media_type_s =
    Stream_Device_Tools::defaultCaptureFormat ((*stream_iterator).second.second.deviceIdentifier.identifier);
  std::stringstream converter;
  converter <<  media_type_s.format.pixelformat;
  int index_i =
      Common_UI_WxWidgets_Tools::clientDataToIndex (choice_format,
                                                    converter.str ());
  ACE_ASSERT (index_i != wxNOT_FOUND);
  choice_format->Select (index_i);

  wxCommandEvent* event_p = NULL;
  ACE_NEW_NORETURN (event_p,
                    wxCommandEvent (wxEVT_COMMAND_CHOICE_SELECTED,
                                    XRCID ("choice_format")));
  ACE_ASSERT (event_p);
  event_p->SetInt (index_i);
  this->QueueEvent (event_p);
  event_p = NULL;

  application_->wait ();

  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << media_type_s.format.width;
  converter << ACE_TEXT_ALWAYS_CHAR (" ");
  converter << media_type_s.format.height;
  index_i = Common_UI_WxWidgets_Tools::clientDataToIndex (choice_resolution,
                                                          converter.str ());
  ACE_ASSERT (index_i != wxNOT_FOUND);
  choice_resolution->Select (index_i);

  ACE_NEW_NORETURN (event_p,
                    wxCommandEvent (wxEVT_COMMAND_CHOICE_SELECTED,
                                    XRCID ("choice_resolution")));
  ACE_ASSERT (event_p);
  event_p->SetInt (index_i);
  this->QueueEvent (event_p);
  event_p = NULL;

  application_->wait ();

  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << media_type_s.frameRate.numerator;
  index_i = choice_framerate->FindString (converter.str ());
  ACE_ASSERT (index_i != wxNOT_FOUND);
  choice_framerate->Select (index_i);

  ACE_NEW_NORETURN (event_p,
                    wxCommandEvent (wxEVT_COMMAND_CHOICE_SELECTED,
                                    XRCID ("choice_framerate")));
  ACE_ASSERT (event_p);
  event_p->SetInt (index_i);
  this->QueueEvent (event_p);
  event_p = NULL;

  button_reset_camera->Enable (false);
}

void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::togglebutton_save_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::togglebutton_save_toggled_cb"));

  bool is_checked_b = event_in.IsChecked ();
  textctrl_filename->Enable (is_checked_b);
//  directorypicker_save->Enable (is_checked_b);
}
//void
//Stream_ImageScreen_WxWidgetsDialog_T<Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
//                                 Stream_ImageScreen_V4L_Stream>::picker_directory_save_changed_cb (wxFileDirPickerEvent& event_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::picker_directory_save_changed_cb"));

//}

void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::togglebutton_display_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::togglebutton_display_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (Stream_Visualization_Tools::rendererToModuleName (configuration_r.configuration->streamConfiguration.configuration_.renderer).c_str ()));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());

  bool is_checked_b = event_in.IsChecked ();
  togglebutton_fullscreen->Enable (is_checked_b);
  choice_screen->Enable (is_checked_b);
//  button_display_settings->Enable (is_checked_b);
  wxRect area_s;
  if (is_checked_b)
  {
    area_s = panel_video->GetClientRect ();
    (*stream_iterator).second.second.area.left = area_s.GetLeft ();
    (*stream_iterator).second.second.area.top = area_s.GetTop ();
    (*stream_iterator).second.second.area.width =
        static_cast<__u32> (area_s.GetWidth ());
    (*stream_iterator).second.second.area.height =
        static_cast<__u32> (area_s.GetHeight ());
//    GtkWidget* widget_p = panel_video->GetHandle ();
//    ACE_ASSERT (widget_p);
//    GdkWindow* window_p = gtk_widget_get_window (widget_p);
//    ACE_ASSERT (window_p);
//    (*stream_iterator).second.second.window = GDK_WINDOW_XID (window_p);
//    ACE_ASSERT ((*stream_iterator).second.second.window);
    wxStringClientData* client_data_p =
      dynamic_cast<wxStringClientData*> (choice_screen->GetClientObject (choice_screen->GetSelection ()));
    ACE_ASSERT (client_data_p);
    (*stream_iterator).second.second.deviceIdentifier.identifier =
        client_data_p->GetData ().ToStdString ();
  } // end IF
  else
  {
    ACE_OS::memset (&(*stream_iterator).second.second.area, 0, sizeof (struct v4l2_rect));
    (*stream_iterator).second.second.window = 0;
    (*stream_iterator).second.second.deviceIdentifier.identifier.clear ();
  } // end ELSE

  switch (configuration_r.configuration->streamConfiguration.configuration_.renderer)
  {
    case STREAM_VISUALIZATION_VIDEORENDERER_NULL:
      break;
    case STREAM_VISUALIZATION_VIDEORENDERER_X11:
    {
      break;
    }
//#if defined (GTK_USE)
//    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO:
//    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_PIXBUF:
//      break;
//#endif // GTK_USE
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown video renderer (was: %d), aborting\n"),
                  configuration_r.configuration->streamConfiguration.configuration_.renderer));
      return;
    }
  } // end SWITCH
}

void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::togglebutton_fullscreen_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::togglebutton_fullscreen_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (Stream_Visualization_Tools::rendererToModuleName (configuration_r.configuration->streamConfiguration.configuration_.renderer).c_str ()));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  ACE_ASSERT (configuration_r.stream);

  choice_resolution_2->Enable (event_in.IsChecked ());
  if (event_in.IsChecked ())
  { // --> toggle to fullscreen
    struct Common_UI_DisplayDevice display_device_s =
      Common_UI_Tools::getDisplay ((*stream_iterator).second.second.deviceIdentifier.identifier);
    (*stream_iterator).second.second.area.left = display_device_s.clippingArea.x;
    (*stream_iterator).second.second.area.top = display_device_s.clippingArea.y;
    (*stream_iterator).second.second.area.width =
        display_device_s.clippingArea.width;
    (*stream_iterator).second.second.area.height =
        display_device_s.clippingArea.height;
    (*stream_iterator).second.second.fullScreen = true;
  } // end IF
  else
  { // toggle to windowed
    wxRect area_s = panel_video->GetClientRect ();
    (*stream_iterator).second.second.area.left = area_s.GetLeft ();
    (*stream_iterator).second.second.area.top = area_s.GetTop ();
    (*stream_iterator).second.second.area.width = area_s.GetWidth ();
    (*stream_iterator).second.second.area.height = area_s.GetHeight ();
    (*stream_iterator).second.second.fullScreen = false;
  } // end ELSE

  std::string module_name_string =
    Stream_Visualization_Tools::rendererToModuleName (configuration_r.configuration->streamConfiguration.configuration_.renderer);
  switch (configuration_r.configuration->streamConfiguration.configuration_.renderer)
  {
    case STREAM_VISUALIZATION_VIDEORENDERER_NULL:
      break;
    case STREAM_VISUALIZATION_VIDEORENDERER_X11:
    {
      break;
    }
#if defined (GTK_USE)
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO:
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_PIXBUF:
      break;
#endif // GTK_USE
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown video renderer (was: %d), aborting\n"),
                  configuration_r.configuration->streamConfiguration.configuration_.renderer));
      return;
    }
  } // end SWITCH

  if (!configuration_r.stream->isRunning ())
    return;
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (configuration_r.stream->find (module_name_string.c_str ()));
  ACE_ASSERT (module_p);
  Common_UI_IFullscreen* ifullscreen_p =
    dynamic_cast<Common_UI_IFullscreen*> (module_p->writer ());
  if (!ifullscreen_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s:%s: dynamic_cast<Common_UI_IFullscreen> failed, aborting\n"),
                ACE_TEXT (configuration_r.stream->name ().c_str ()),
                ACE_TEXT (module_name_string.c_str ())));
    return;
  } // end IF
  ifullscreen_p->toggle ();
}

void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::choice_displayadapter_changed_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::choice_displayadapter_changed_cb"));

}
void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::choice_screen_changed_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::choice_screen_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_ImageScreen_V4L_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (Stream_Visualization_Tools::rendererToModuleName (configuration_r.configuration->streamConfiguration.configuration_.renderer).c_str ()));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  wxStringClientData* client_data_p =
    dynamic_cast<wxStringClientData*> (choice_screen->GetClientObject (event_in.GetSelection ()));
  ACE_ASSERT (client_data_p);
  (*stream_iterator).second.second.deviceIdentifier.identifier =
      client_data_p->GetData ().ToStdString ();
  ACE_ASSERT (!(*stream_iterator).second.second.deviceIdentifier.identifier.empty ());
  static struct Common_UI_DisplayDevice display_device_s =
    Common_UI_Tools::getDisplay ((*stream_iterator).second.second.deviceIdentifier.identifier);
  ACE_ASSERT (!display_device_s.device.empty ());
  static struct Common_UI_DisplayAdapter display_adapter_s =
    Common_UI_Tools::getAdapter (display_device_s);
  ACE_ASSERT (!display_adapter_s.device.empty ());
  int index_i = choice_displayadapter->FindString (display_adapter_s.description);
  ACE_ASSERT (index_i != wxNOT_FOUND);
  choice_displayadapter->Select (index_i);

  wxCommandEvent* event_p = NULL;
  ACE_NEW_NORETURN (event_p,
                    wxCommandEvent (wxEVT_COMMAND_CHOICE_SELECTED,
                                    XRCID ("choice_displayadapter")));
  ACE_ASSERT (event_p);
  event_p->SetInt (index_i);
  this->QueueEvent (event_p);
  event_p = NULL;

//  application_->wait ();

//  button_display_settings->Enable (togglebutton_display->IsEnabled ());

  Common_UI_Resolutions_t resolutions_a =
    Common_UI_Tools::get ((*stream_iterator).second.second.deviceIdentifier.identifier);
  ACE_ASSERT (!resolutions_a.empty ());
  Common_UI_Resolution_t resolution_s;
  resolution_s.width =
      (*stream_iterator).second.second.outputFormat.resolution.width;
  resolution_s.height =
      (*stream_iterator).second.second.outputFormat.resolution.height;
  choice_resolution_2->SetSelection (wxNOT_FOUND);
  choice_resolution_2->Clear ();
  index_i = wxNOT_FOUND;
  std::stringstream converter;
  std::string resolution_string;
  for (Common_UI_ResolutionsConstIterator_t iterator = resolutions_a.begin ();
       iterator != resolutions_a.end ();
       ++iterator)
  {
    client_data_p = NULL;
    ACE_NEW_NORETURN (client_data_p,
                      wxStringClientData ());
    ACE_ASSERT (client_data_p);
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (*iterator).width;
    converter << ACE_TEXT_ALWAYS_CHAR (" ");
    converter << (*iterator).height;
    client_data_p->SetData (converter.str ());

    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (*iterator).width;
    converter << ACE_TEXT_ALWAYS_CHAR (" x ");
    converter << (*iterator).height;

    index_i = choice_resolution_2->Append (converter.str (),
                                           client_data_p);
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": supports: %ux%u\n"),
                ACE_TEXT ((*stream_iterator).second.second.deviceIdentifier.identifier.c_str ()),
                (*iterator).width, (*iterator).height));
#endif // _DEBUG
    if ((resolution_s.width == (*iterator).width) &&
        (resolution_s.height == (*iterator).height))
      resolution_string = converter.str ();
  } // end FOR
//  ACE_ASSERT (!resolution_string.empty ());
  choice_resolution_2->Enable (!resolutions_a.empty () &&
                               togglebutton_fullscreen->GetValue ());
  index_i =
    (initializing_ ? choice_resolution_2->FindString (resolution_string)
                   : 0);
  choice_resolution_2->Select (index_i);

  ACE_NEW_NORETURN (event_p,
                    wxCommandEvent (wxEVT_COMMAND_CHOICE_SELECTED,
                                    XRCID ("choice_resolution_2")));
  ACE_ASSERT (event_p);
  event_p->SetInt (index_i);
  this->QueueEvent (event_p);
  event_p = NULL;

  //application_->wait ();

  if (initializing_)
    initializing_ = false;
}
//void
//Stream_ImageScreen_WxWidgetsDialog_T<Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
//                                 Stream_ImageScreen_V4L_Stream>::button_display_settings_clicked_cb (wxCommandEvent& event_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::button_display_settings_clicked_cb"));

//}
void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::choice_resolution_2_changed_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::choice_resolution_2_changed_cb"));

}

void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::button_about_clicked_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::button_about_clicked_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);
  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::STATE_T& state_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::STATE_T&> (application_->getR ());
  ACE_ASSERT (state_r.argv);

  std::ostringstream converter;
  wxAboutDialogInfo about_dialog_info;
  about_dialog_info.SetName (_ (Common_File_Tools::basename (wxString (state_r.argv[0]).ToStdString (),
                                                             true).c_str ()));
#if defined (HAVE_CONFIG_H)
  about_dialog_info.SetVersion (_ (ACEStream_PACKAGE_VERSION));
  //about_dialog_info.SetDescription (_ (ACEStream_PACKAGE_DESCRIPTION));
  about_dialog_info.SetDescription (_ ("video capure stream test application"));
#endif // HAVE_CONFIG_H
  std::string copyright_string = ACE_TEXT_ALWAYS_CHAR ("(C) ");
  ACE_Date_Time date_time = COMMON_DATE_NOW;
  converter << date_time.year ();
  copyright_string += converter.str ();
#if defined (HAVE_CONFIG_H)
  copyright_string += ACE_TEXT_ALWAYS_CHAR (" ");
  copyright_string += ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_MAINTAINER);
  copyright_string += ACE_TEXT_ALWAYS_CHAR (" <");
  copyright_string += ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_BUGREPORT);
  copyright_string += ACE_TEXT_ALWAYS_CHAR (">");
#endif // HAVE_CONFIG_H
  about_dialog_info.SetCopyright (_ (copyright_string.c_str ()));
#if defined (HAVE_CONFIG_H)
  about_dialog_info.AddDeveloper (_ (ACEStream_PACKAGE_MAINTAINER));
#endif // HAVE_CONFIG_H
  wxAboutBox (about_dialog_info);
}
void
Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                 Stream_ImageScreen_V4L_WxWidgetsIApplication_t,
                                 Stream_ImageScreen_V4L_Stream>::button_quit_clicked_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_WxWidgetsDialog_T::button_quit_clicked_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  // step1: make sure the stream has stopped
  Stream_ImageScreen_V4L_Stream* stream_p = NULL;
  Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_ImageScreen_V4L_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  stream_p = configuration_r.stream;
  ACE_ASSERT (stream_p);
  const enum Stream_StateMachine_ControlState& status_r =
    stream_p->status ();
  if ((status_r == STREAM_STATE_RUNNING) ||
      (status_r == STREAM_STATE_PAUSED))
    stream_p->stop (false, // wait for completion ?
                    false, // recurse upstream (if any) ?
                    true); // locked access ?

  // step2: close main window
  this->Close (true); // force ?

  // step3: initiate shutdown sequence
  int result = ACE_OS::raise (SIGINT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                SIGINT));
}
#endif // ACE_WIN32 || ACE_WIN64
