#if defined (ACE_WIN64) || defined (ACE_WIN32)
#include <OleCtl.h>
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H
#endif // ACE_WIN64 || ACE_WIN32

#include "wx/aboutdlg.h"

#include "ace/Date_Time.h"
#include "ace/Log_Msg.h"
#include "ace/OS_NS_Thread.h"

#include "common_file_tools.h"

#include "common_ui_ifullscreen.h"
#include "common_ui_tools.h"

#include "common_ui_wxwidgets_tools.h"

#include "common_test_u_tools.h"

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_macros.h"

#if defined (ACE_WIN64) || defined (ACE_WIN32)
#include "stream_dev_directshow_tools.h"
#else
#include "stream_lib_tools.h"
#endif // ACE_WIN64 || ACE_WIN32

#include "stream_vis_tools.h"

template <typename InterfaceType,
          typename StreamType>
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::Stream_CamSave_WxWidgetsDialog_T (wxWindow* parent_in)
 : inherited (parent_in)
 , application_ (NULL)
 , initializing_ (true)
 , untoggling_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::Stream_CamSave_WxWidgetsDialog_T"));

#if defined (ACE_WIN64) || defined (ACE_WIN32)
  inherited::MSWSetOldWndProc (NULL);
#endif // ACE_WIN64 || ACE_WIN32
}

template <typename InterfaceType,
          typename StreamType>
bool
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::OnInit_2 (IAPPLICATION_T* iapplication_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::OnInit_2"));

  // sanity check(s)
  ACE_ASSERT (!application_);

  application_ = dynamic_cast<InterfaceType*> (iapplication_in);
  ACE_ASSERT (application_);

  togglebutton_record = XRCCTRL (*this, "togglebutton_record", wxToggleButton);
  button_snapshot = XRCCTRL (*this, "button_snapshot", wxBitmapButton);
  button_cut = XRCCTRL (*this, "button_cut", wxBitmapButton);
  button_report = XRCCTRL (*this, "button_report", wxBitmapButton);
  button_snapshot = XRCCTRL (*this, "button_snapshot", wxBitmapButton);
  spincontrol_frames_captured = XRCCTRL (*this, "spincontrol_frames_captured", wxSpinCtrl);
  spincontrol_frames_dropped = XRCCTRL (*this, "spincontrol_frames_dropped", wxSpinCtrl);
  spincontrol_messages_session = XRCCTRL (*this, "spincontrol_messages_session", wxSpinCtrl);
  spincontrol_messages_data = XRCCTRL (*this, "spincontrol_messages_data", wxSpinCtrl);
  spincontrol_data = XRCCTRL (*this, "spincontrol_data", wxSpinCtrl);
  spincontrol_buffer = XRCCTRL (*this, "spincontrol_buffer", wxSpinCtrl);
  choice_source = XRCCTRL (*this, "choice_source", wxChoice);
  button_hardware_settings = XRCCTRL (*this, "button_hardware_settings", wxBitmapButton);
  choice_format = XRCCTRL (*this, "choice_format", wxChoice);
  choice_resolution = XRCCTRL (*this, "choice_resolution", wxChoice);
  choice_framerate = XRCCTRL (*this, "choice_framerate", wxChoice);
  button_reset_format = XRCCTRL (*this, "button_reset", wxBitmapButton);
  togglebutton_save = XRCCTRL (*this, "togglebutton_save", wxToggleButton);
  textcontrol_filename = XRCCTRL (*this, "textcontrol_filename", wxTextCtrl);
  directorypicker_save = XRCCTRL (*this, "directorypicker_save", wxDirPickerCtrl);
  togglebutton_display = XRCCTRL (*this, "togglebutton_display", wxToggleButton);
  togglebutton_fullscreen = XRCCTRL (*this, "togglebutton_fullscreen", wxToggleButton);
  choice_adapter = XRCCTRL (*this, "choice_adapter", wxChoice);
  choice_display = XRCCTRL (*this, "choice_display", wxChoice);
  button_display_settings = XRCCTRL (*this, "button_display_settings", wxBitmapButton);
  choice_resolution_2 = XRCCTRL (*this, "choice_resolution_2", wxChoice);
  panel_video = XRCCTRL (*this, "panel_video", wxPanel);
  button_about = XRCCTRL (*this, "button_about", wxBitmapButton);
  button_quit = XRCCTRL (*this, "button_quit", wxBitmapButton);
  gauge_progress = XRCCTRL (*this, "gauge_progress", wxGauge);

  // populate controls
#if defined (_DEBUG)
#else
  button_report->Show (false);
#endif // _DEBUG
  bool activate_source = true;
  typename InterfaceType::CONFIGURATION_T& configuration_r =
    const_cast<typename InterfaceType::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  typename StreamType::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());

  Stream_Device_List_t devices_a;
#if defined (ACE_WIN64) || defined (ACE_WIN32)
  switch (configuration_r.mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      devices_a =
        Stream_Device_DirectShow_Tools::getCaptureDevices (CLSID_VideoInputDeviceCategory);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (false);
      ACE_NOTREACHED (return false;)
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  configuration_r.mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
#endif // ACE_WIN64 || ACE_WIN32

  int index_i = wxNOT_FOUND;
  for (Stream_Device_ListIterator_t iterator = devices_a.begin ();
       iterator != devices_a.end ();
       ++iterator)
  {
#if defined (ACE_WIN64) || defined (ACE_WIN32)
    switch (configuration_r.mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        index_i =
          choice_source->Append (Stream_Device_DirectShow_Tools::devicePathToString (*iterator).c_str ());
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        ACE_ASSERT (false);
        ACE_NOTSUP_RETURN (false);
        ACE_NOTREACHED (return false;)
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                    configuration_r.mediaFramework));
        return false;
      }
    } // end SWITCH
#else
    ACE_ASSERT (false);
    ACE_NOTSUP_RETURN (false);
    ACE_NOTREACHED (return false;)
#endif // ACE_WIN64 || ACE_WIN32
  } // end FOR
  if (unlikely (devices_a.empty ()))
    activate_source = false;
  else
    choice_source->Enable (true);

  if (likely (activate_source))
  {
    index_i =
      (initializing_ ? choice_source->FindString ((*stream_iterator).second.second.deviceIdentifier.identifier.c_str ())
                     : 0);
    choice_source->Select (index_i);
    wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                            XRCID ("choice_source"));
    event_s.SetInt (index_i);
    //choice_source->GetEventHandler ()->ProcessEvent (event_s);
    this->AddPendingEvent (event_s);
  } // end IF

  return true;
}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::OnExit_2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::OnExit_2"));

  // sanity check(s)
  ACE_ASSERT (application_);
}

//////////////////////////////////////////

template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::dialog_main_idle_cb (wxIdleEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::dialog_main_idle_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  typename InterfaceType::CONFIGURATION_T& configuration_r =
    const_cast<typename InterfaceType::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);

  if (configuration_r.stream->isRunning ())
    gauge_progress->Pulse ();
}

template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::dialog_main_keydown_cb (wxKeyEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::dialog_main_keydown_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  typename InterfaceType::CONFIGURATION_T& configuration_r =
    const_cast<typename InterfaceType::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);
}

//////////////////////////////////////////

template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::togglebutton_record_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_record_toggled_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::button_snapshot_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_snapshot_click_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  typename InterfaceType::CONFIGURATION_T& configuration_r =
    const_cast<typename InterfaceType::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);

  configuration_r.stream->control (STREAM_CONTROL_STEP_2);
}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::button_cut_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_cut_click_cb"));

}
#if defined (_DEBUG)
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::button_report_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_report_click_cb"));

}
#endif // _DEBUG
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::choice_source_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_source_selected_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  typename InterfaceType::CONFIGURATION_T& configuration_r =
    const_cast<typename InterfaceType::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
#if defined (ACE_WIN64) || defined (ACE_WIN32)
  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
#else
  typename StreamType::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
#endif // ACE_WIN64 || ACE_WIN32

  std::string device_identifier;
//  int index_i = -1;
#if defined (ACE_WIN64) || defined (ACE_WIN32)
  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
#else
  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
#endif // ACE_WIN64 || ACE_WIN32
  ACE_ASSERT (!device_identifier.empty ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
#else
  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
#endif // ACE_WIN32 || ACE_WIN64
}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::button_hardware_settings_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_hardware_settings_click_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::choice_format_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_format_selected_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::choice_resolution_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_resolution_selected_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::choice_framerate_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_framerate_selected_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::button_reset_format_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_reset_format_click_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::togglebutton_save_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_save_toggled_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::picker_directory_save_changed_cb (wxFileDirPickerEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::picker_directory_save_changed_cb"));

}

template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::togglebutton_display_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_display_toggled_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::togglebutton_fullscreen_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_fullscreen_toggled_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::choice_adapter_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_adapter_selected_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::choice_display_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_display_selected_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::button_display_settings_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_display_settings_click_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::choice_resolution_2_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_resolution_2_selected_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  typename InterfaceType::CONFIGURATION_T& configuration_r =
    const_cast<typename InterfaceType::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
#if defined (ACE_WIN64) || defined (ACE_WIN32)
  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
#else
  typename StreamType::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
#endif // ACE_WIN64 || ACE_WIN32
}

template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::button_about_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_about_click_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::button_quit_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_quit_click_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  // step1: make sure the stream has stopped
  StreamType* stream_p = NULL;
  typename InterfaceType::CONFIGURATION_T& configuration_r =
    const_cast<typename InterfaceType::CONFIGURATION_T&> (application_->getR_2 ());
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

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::Stream_CamSave_WxWidgetsDialog_T (wxWindow* parent_in)
 : inherited (parent_in)
 , application_ (NULL)
 , initializing_ (true)
 , reset_ (false)
 , untoggling_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::Stream_CamSave_WxWidgetsDialog_T"));

#if defined (ACE_WIN64) || defined (ACE_WIN32)
  inherited::MSWSetOldWndProc (NULL);
#endif // ACE_WIN64 || ACE_WIN32
}

bool
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::OnInit_2 (IAPPLICATION_T* iapplication_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::OnInit_2"));

  // sanity check(s)
  ACE_ASSERT (!application_);

  application_ =
    dynamic_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t*> (iapplication_in);
  ACE_ASSERT (application_);

  togglebutton_record = XRCCTRL (*this, "togglebutton_record", wxToggleButton);
  button_snapshot = XRCCTRL (*this, "button_snapshot", wxBitmapButton);
  button_cut = XRCCTRL (*this, "button_cut", wxBitmapButton);
  button_report = XRCCTRL (*this, "button_report", wxBitmapButton);
  button_snapshot = XRCCTRL (*this, "button_snapshot", wxBitmapButton);
  spincontrol_frames_captured = XRCCTRL (*this, "spincontrol_frames_captured", wxSpinCtrl);
  spincontrol_frames_dropped = XRCCTRL (*this, "spincontrol_frames_dropped", wxSpinCtrl);
  spincontrol_messages_session = XRCCTRL (*this, "spincontrol_messages_session", wxSpinCtrl);
  spincontrol_messages_data = XRCCTRL (*this, "spincontrol_messages_data", wxSpinCtrl);
  spincontrol_data = XRCCTRL (*this, "spincontrol_data", wxSpinCtrl);
  spincontrol_buffer = XRCCTRL (*this, "spincontrol_buffer", wxSpinCtrl);
  choice_source = XRCCTRL (*this, "choice_source", wxChoice);
  button_hardware_settings = XRCCTRL (*this, "button_hardware_settings", wxBitmapButton);
  choice_format = XRCCTRL (*this, "choice_format", wxChoice);
  choice_resolution = XRCCTRL (*this, "choice_resolution", wxChoice);
  choice_framerate = XRCCTRL (*this, "choice_framerate", wxChoice);
  button_reset_format = XRCCTRL (*this, "button_reset_format", wxBitmapButton);
  togglebutton_save = XRCCTRL (*this, "togglebutton_save", wxToggleButton);
  textcontrol_filename = XRCCTRL (*this, "textcontrol_filename", wxTextCtrl);
  directorypicker_save = XRCCTRL (*this, "directorypicker_save", wxDirPickerCtrl);
  togglebutton_display = XRCCTRL (*this, "togglebutton_display", wxToggleButton);
  togglebutton_fullscreen = XRCCTRL (*this, "togglebutton_fullscreen", wxToggleButton);
  choice_adapter = XRCCTRL (*this, "choice_adapter", wxChoice);
  choice_display = XRCCTRL (*this, "choice_display", wxChoice);
  button_display_settings = XRCCTRL (*this, "button_display_settings", wxBitmapButton);
  choice_resolution_2 = XRCCTRL (*this, "choice_resolution_2", wxChoice);
  panel_video = XRCCTRL (*this, "panel_video", wxPanel);
  button_about = XRCCTRL (*this, "button_about", wxBitmapButton);
  button_quit = XRCCTRL (*this, "button_quit", wxBitmapButton);
  gauge_progress = XRCCTRL (*this, "gauge_progress", wxGauge);

  this->SetDefaultItem (togglebutton_record);

  // populate controls
#if defined (_DEBUG)
#else
  button_report->Show (false);
#endif // _DEBUG
  spincontrol_frames_captured->SetRange (0,
                                         std::numeric_limits<int>::max ());
  spincontrol_frames_dropped->SetRange (0,
                                        std::numeric_limits<int>::max ());
  spincontrol_messages_session->SetRange (0,
                                          std::numeric_limits<int>::max ());
  spincontrol_messages_data->SetRange (0,
                                       std::numeric_limits<int>::max ());
  spincontrol_data->SetRange (0,
                              std::numeric_limits<int>::max ());
  spincontrol_buffer->SetRange (0,
                                std::numeric_limits<int>::max ());

  bool activate_source_b = true, activate_display_b = true;
  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator_2 =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (Stream_Visualization_Tools::rendererToModuleName (configuration_r.configuration->streamConfiguration.configuration_.renderer).c_str ()));
  ACE_ASSERT (stream_iterator_2 != configuration_r.configuration->streamConfiguration.end ());

  Stream_Device_List_t devices_a =
    Stream_Device_DirectShow_Tools::getCaptureDevices (CLSID_VideoInputDeviceCategory);

  int index_i = -1;
  wxStringClientData* client_data_p = NULL;
  for (Stream_Device_ListIterator_t iterator = devices_a.begin ();
       iterator != devices_a.end ();
       ++iterator)
  { ACE_ASSERT ((*iterator).identifierDiscriminator == Stream_Device_Identifier::STRING);
    client_data_p = NULL;
    ACE_NEW_NORETURN (client_data_p,
                      wxStringClientData ());
    ACE_ASSERT (client_data_p);
    client_data_p->SetData ((*iterator).identifier._string);

    index_i =
      choice_source->Append (Stream_Device_DirectShow_Tools::devicePathToString ((*iterator).identifier._string).c_str (),
                             client_data_p);
  } // end FOR
  if (unlikely (devices_a.empty ()))
    activate_source_b = false;
  else
    choice_source->Enable (true);

  togglebutton_save->Enable (!(*stream_iterator).second.second.targetFileName.empty ());
  togglebutton_save->SetValue (!(*stream_iterator).second.second.targetFileName.empty ());
  textcontrol_filename->Enable (togglebutton_save->GetValue ());
  textcontrol_filename->SetValue (ACE_TEXT_ALWAYS_CHAR (ACE::basename ((*stream_iterator).second.second.targetFileName.c_str (),
                                                                       ACE_DIRECTORY_SEPARATOR_CHAR)));
  directorypicker_save->Enable (togglebutton_save->GetValue ());
  directorypicker_save->SetPath (ACE_TEXT_ALWAYS_CHAR (ACE::dirname ((*stream_iterator).second.second.targetFileName.c_str (),
                                                                     ACE_DIRECTORY_SEPARATOR_CHAR)));

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
    client_data_p->SetData ((*iterator).id);

    index_i =
      choice_adapter->Append ((*iterator).description.c_str (),
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
      choice_display->Append ((*iterator).description.c_str (),
                              client_data_p);
  } // end FOR
  if (likely (!display_devices_a.empty ()))
  {
    togglebutton_display->Enable ((*stream_iterator_2).second.second.deviceIdentifier.identifierDiscriminator != Stream_Device_Identifier::INVALID);
    togglebutton_display->SetValue ((*stream_iterator_2).second.second.deviceIdentifier.identifierDiscriminator != Stream_Device_Identifier::INVALID);
    togglebutton_fullscreen->Enable (togglebutton_display->GetValue ());
    togglebutton_fullscreen->SetValue ((*stream_iterator_2).second.second.fullScreen);
    panel_video->Show (togglebutton_display->GetValue () &&
                       !togglebutton_fullscreen->GetValue ());
  } // end IF
  if (unlikely (devices_a.empty ()))
    activate_display_b = false;
  else
    choice_display->Enable (true);

  if (likely (activate_source_b))
  { ACE_ASSERT ((*stream_iterator).second.second.deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::STRING);
    index_i =
      (initializing_ ? choice_source->FindString (Stream_Device_DirectShow_Tools::devicePathToString ((*stream_iterator).second.second.deviceIdentifier.identifier._string),
                                                  false)
                     : 0);
    ACE_ASSERT (index_i != wxNOT_FOUND);
    choice_source->Select (index_i);
    wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                            XRCID ("choice_source"));
    event_s.SetInt (index_i);
    //choice_source->GetEventHandler ()->ProcessEvent (event_s);
    this->AddPendingEvent (event_s);
  } // end IF
  application_->wait ();

  ACE_ASSERT ((*stream_iterator_2).second.second.deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::STRING);
  if (likely (activate_display_b))
  {
    index_i =
      (initializing_ ? Common_UI_WxWidgets_Tools::clientDataToIndex (choice_display,
                                                                     ACE_TEXT_ALWAYS_CHAR ((*stream_iterator_2).second.second.deviceIdentifier.identifier._string))
                     : 0);
    ACE_ASSERT (index_i != wxNOT_FOUND);
    choice_display->Select (index_i);
    wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                            XRCID ("choice_display"));
    event_s.SetInt (index_i);
    //choice_source->GetEventHandler ()->ProcessEvent (event_s);
    this->AddPendingEvent (event_s);
    wxCommandEvent event_2 (wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
                            XRCID ("togglebutton_display"));

    event_2.SetInt (!ACE_OS::strlen ((*stream_iterator_2).second.second.deviceIdentifier.identifier._string) ? 1
                                                                                                             : 0);
    //togglebutton_display->GetEventHandler ()->ProcessEvent (event_2);
    this->AddPendingEvent (event_2);
    wxCommandEvent event_3 (wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
                            XRCID ("togglebutton_fullscreen"));
    event_3.SetInt ((*stream_iterator_2).second.second.fullScreen ? 1 : 0);
    //togglebutton_fullscreen->GetEventHandler ()->ProcessEvent (event_3);
    this->AddPendingEvent (event_3);
  } // end IF

  return true;
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::OnExit_2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::OnExit_2"));

  // sanity check(s)
  ACE_ASSERT (application_);
}

//////////////////////////////////////////

void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::dialog_main_idle_cb (wxIdleEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::dialog_main_idle_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);
  bool finished_b = false;

  process_stream_events (&configuration_r,
                         finished_b);
  if (!finished_b &&
      configuration_r.stream->isRunning ())
    gauge_progress->Pulse ();
}

void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::dialog_main_keydown_cb (wxKeyEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::dialog_main_keydown_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);

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

      wxCommandEvent event_s (wxEVT_COMMAND_BUTTON_CLICKED,
                              XRCID ("button_cut"));
      event_s.SetInt (1);
      //choice_source->GetEventHandler ()->ProcessEvent (event_s);
      this->AddPendingEvent (event_s);
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
      wxCommandEvent event_s (wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
                              XRCID ("togglebutton_fullscreen"));
      event_s.SetInt (is_checked_b ? 0 : 1);
      //choice_source->GetEventHandler ()->ProcessEvent (event_s);
      this->AddPendingEvent (event_s);
      break;
    }
    case 'r':
    case 'R':
    {
      wxCommandEvent event_s (wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
                              XRCID ("togglebutton_record"));
      bool is_checked_b = togglebutton_record->GetValue ();
      togglebutton_record->SetValue (!is_checked_b);
      event_s.SetInt (togglebutton_record->GetValue () ? 0 : 1);
      //choice_source->GetEventHandler ()->ProcessEvent (event_s);
      this->AddPendingEvent (event_s);
      break;
    }
    case 's':
    case 'S':
    {
      // sanity check(s)
      if (!configuration_r.stream->isRunning ())
        return; // nothing to do

      wxCommandEvent event_s (wxEVT_COMMAND_BUTTON_CLICKED,
                              XRCID ("button_snapshot"));
      event_s.SetInt (1);
      //choice_source->GetEventHandler ()->ProcessEvent (event_s);
      this->AddPendingEvent (event_s);
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
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::togglebutton_record_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_record_toggled_cb"));

  // handle untoggle --> PLAY
  if (untoggling_)
  {
    untoggling_ = false;
    return; // done
  } // end IF

  // sanity check(s)
  ACE_ASSERT (application_);

  // --> user pressed play/pause/stop
  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator_2 =
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

  struct Stream_CamSave_UI_ThreadData* thread_data_p = NULL;
  ACE_thread_t thread_id = std::numeric_limits<unsigned long>::max ();
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;
  struct _AMMediaType* media_type_p = NULL;

  // step1: reset progress reporting
  ACE_OS::memset (&configuration_r.progressData.statistic,
                  0,
                  sizeof (struct Stream_CamSave_StatisticData));

  // step2: update configuration
  // step2a: update capture device configuration
  configuration_r.configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize =
    spincontrol_buffer->GetValue ();

  // *NOTE*: the capture format has been updated already (see:
  //         choice_framerate_selected_cb())
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

  Common_UI_Resolution_t resolution_2 =
    Stream_MediaFramework_DirectShow_Tools::toResolution (configuration_r.configuration->streamConfiguration.configuration_.format);
  //bool reset_device_b =
  //  ((resolution_s.cx != resolution_2.cx) || (resolution_s.cy != resolution_2.cy));
  //ACE_ASSERT ((*stream_iterator).second.second.sourceFormat);
  //(*stream_iterator_2).second.second.sourceFormat =
  //  Stream_MediaFramework_DirectShow_Tools::copy (*(*stream_iterator).second.second.sourceFormat);
  //if (reset_device_b)
  //{
    Stream_MediaFramework_DirectShow_Tools::free ((*stream_iterator).second.second.outputFormat);
    (*stream_iterator).second.second.outputFormat =
      Stream_MediaFramework_DirectShow_Tools::toRGB (configuration_r.configuration->streamConfiguration.configuration_.format);
    Stream_MediaFramework_DirectShow_Tools::free ((*stream_iterator_2).second.second.outputFormat);
    media_type_p =
      Stream_MediaFramework_DirectShow_Tools::copy ((*stream_iterator).second.second.outputFormat);
    ACE_ASSERT (media_type_p);
    (*stream_iterator_2).second.second.outputFormat = *media_type_p;
    CoTaskMemFree (media_type_p); media_type_p = NULL;
  //} // end IF

  configuration_r.configuration->direct3DConfiguration.presentationParameters.BackBufferWidth =
    resolution_2.cx;
  configuration_r.configuration->direct3DConfiguration.presentationParameters.BackBufferHeight =
    resolution_2.cy;

  // step2b: update save configuration
  std::string filename_string;
  if (!togglebutton_save->GetValue ())
    goto continue_;
  filename_string = directorypicker_save->GetPath ();
  ACE_ASSERT (Common_File_Tools::isDirectory (filename_string));
  ACE_ASSERT (Common_File_Tools::isWriteable (filename_string));
  filename_string += ACE_DIRECTORY_SEPARATOR_STR;
  filename_string += textcontrol_filename->GetValue ();
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
  //    dynamic_cast<wxStringClientData*> (choice_display->GetClientObject (choice_display->GetSelection ()));
  //  ACE_ASSERT (client_data_p);
  //  (*stream_iterator_2).second.second.deviceIdentifier =
  //    client_data_p->GetData ().ToStdString ();

    //ACE_ASSERT ((*stream_iterator_2).second.second.direct3DConfiguration);
    configuration_r.configuration->direct3DConfiguration.focusWindow =
      (HWND)panel_video->GetHandle ();
    configuration_r.configuration->direct3DConfiguration.presentationParameters.hDeviceWindow =
      (HWND)panel_video->GetHandle ();
  } // end IF
  else
  {
  //  ACE_OS::memset (&(*stream_iterator_2).second.second.area, 0, sizeof (struct tagRECT));
  //  (*stream_iterator).second.second.area =
  //    (*stream_iterator_2).second.second.area;
  //  (*stream_iterator_2).second.second.deviceIdentifier.clear ();
  //  //ACE_ASSERT ((*stream_iterator_2).second.second.direct3DConfiguration);

    configuration_r.configuration->direct3DConfiguration.focusWindow =
      NULL;
    configuration_r.configuration->direct3DConfiguration.presentationParameters.hDeviceWindow =
      NULL;
  } // end ELSE
  if (togglebutton_fullscreen->GetValue ())
  {
  //  struct Common_UI_DisplayDevice display_device_s =
  //    Common_UI_Tools::getDisplayDevice ((*stream_iterator).second.second.deviceIdentifier);
  //  (*stream_iterator_2).second.second.area = display_device_s.clippingArea;
  //  (*stream_iterator_2).second.second.fullScreen = true;

  //  //ACE_ASSERT ((*stream_iterator).second.second.direct3DConfiguration);
    configuration_r.configuration->direct3DConfiguration.focusWindow =
      GetAncestor ((HWND)panel_video->GetHandle (),
                   GA_ROOTOWNER);
    ACE_ASSERT (configuration_r.configuration->direct3DConfiguration.focusWindow == (HWND)this->GetHandle ());
    configuration_r.configuration->direct3DConfiguration.presentationParameters.hDeviceWindow =
      NULL;
    //configuration_r.configuration->direct3DConfiguration.presentationParameters.Windowed =
    //  FALSE;
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

    configuration_r.configuration->direct3DConfiguration.focusWindow =
      (HWND)panel_video->GetHandle ();
    configuration_r.configuration->direct3DConfiguration.presentationParameters.hDeviceWindow =
      (HWND)panel_video->GetHandle ();
  //  configuration_r.configuration->direct3DConfiguration.presentationParameters.Windowed =
  //    TRUE;
  } // end ELSE

  // step3: set up device ?
  switch (configuration_r.configuration->streamConfiguration.configuration_.renderer)
  {
    //case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_2D:
    //  break;
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D:
    {
      if (!reset_)
        break;

      // sanity check(s)
      ACE_ASSERT (configuration_r.configuration->direct3DConfiguration.handle);
      ACE_ASSERT (ACE_OS::thr_equal (ACE_OS::thr_self (), configuration_r.configuration->direct3DConfiguration.threadId));

      // *TODO*: remove ASAP
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, configuration_r.configuration->direct3DConfiguration.lock);

      // *NOTE*: may toggle the device between windowed/fullscreen mode
      if (!Stream_MediaFramework_DirectDraw_Tools::reset (configuration_r.configuration->direct3DConfiguration.handle,
                                                          configuration_r.configuration->direct3DConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_DirectDraw_Tools::reset(), returning\n")));
        return;
      } // end IF
      reset_ = false;
      break;
    }
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTSHOW:
    case STREAM_VISUALIZATION_VIDEORENDERER_GDI:
      break;
    //case STREAM_VISUALIZATION_VIDEORENDERER_MEDIAFOUNDATION:
    //  break;
#if defined (GTK_USE)
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO:
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_PIXBUF:
      break;
#endif // GTK_USE
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown video renderer (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  configuration_r.configuration->streamConfiguration.configuration_.renderer));
      return;
    }
  } // end SWITCH

  // step4: start processing thread(s)
  ACE_Thread_ID thread_id_2;
  bool result =
    Common_Test_U_Tools::spawn<struct Stream_CamSave_UI_ThreadData,
                               Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T> (ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_THREAD_NAME),
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
  button_reset_format->Enable (false);
  togglebutton_save->Enable (false);
  textcontrol_filename->Enable (false);
  directorypicker_save->Enable (false);
  togglebutton_display->Enable (false);
  choice_display->Enable (false);
  gauge_progress->Enable (true);
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_snapshot_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_snapshot_click_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);

  configuration_r.stream->control (STREAM_CONTROL_STEP_2);
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_cut_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_cut_click_cb"));

}
#if defined (_DEBUG)
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_report_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_report_click_cb"));

}
#endif // _DEBUG
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::choice_source_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_source_selected_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  std::string device_identifier;
  wxStringClientData* client_data_p =
    dynamic_cast<wxStringClientData*> (choice_source->GetClientObject (event_in.GetSelection ()));
  ACE_ASSERT (client_data_p);
  device_identifier = client_data_p->GetData ().ToStdString ();
  ACE_ASSERT (!device_identifier.empty ());

  if ((*stream_iterator).second.second.builder)
  {
    (*stream_iterator).second.second.builder->Release (); (*stream_iterator).second.second.builder = NULL;
  } // end IF
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  if (configuration_r.streamConfiguration)
  {
    configuration_r.streamConfiguration->Release (); configuration_r.streamConfiguration = NULL;
  } // end IF
  Stream_MediaFramework_DirectShow_Graph_t graph_layout;
  if (!Stream_Device_DirectShow_Tools::loadDeviceGraph (device_identifier,
                                                        CLSID_VideoInputDeviceCategory,
                                                        (*stream_iterator).second.second.builder,
                                                        buffer_negotiation_p,
                                                        configuration_r.streamConfiguration,
                                                        graph_layout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), returning\n"),
                ACE_TEXT (device_identifier.c_str ())));
    return;
  } // end IF
  ACE_ASSERT ((*stream_iterator).second.second.builder);
  ACE_ASSERT (buffer_negotiation_p);
  ACE_ASSERT (configuration_r.streamConfiguration);

  buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;

  choice_format->SetSelection (wxNOT_FOUND);
  choice_format->Clear ();
  Common_Identifiers_t subformats_a =
    Stream_Device_DirectShow_Tools::getCaptureSubFormats (configuration_r.streamConfiguration);
  ACE_ASSERT (!subformats_a.empty ());
  int index_i = -1;
  for (Common_IdentifiersIterator_t iterator = subformats_a.begin ();
        iterator != subformats_a.end ();
        ++iterator)
  {
    client_data_p = NULL;
    ACE_NEW_NORETURN (client_data_p,
                      wxStringClientData ());
    ACE_ASSERT (client_data_p);
    client_data_p->SetData (Common_Tools::GUIDToString (*iterator));

    index_i =
      choice_format->Append (Stream_MediaFramework_Tools::mediaSubTypeToString (*iterator,
                                                                                STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str (),
                             client_data_p);
  } // end FOR

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    (*stream_iterator).second.second.builder->FindFilterByName (STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO,
                                                                &filter_p);
  ACE_ASSERT (SUCCEEDED (result) && filter_p);
  button_hardware_settings->Enable (Stream_MediaFramework_DirectShow_Tools::hasPropertyPages (filter_p));
  filter_p->Release (); filter_p = NULL;
  choice_format->Enable (!subformats_a.empty ());
  index_i =
    (initializing_ ? choice_format->FindString (Stream_MediaFramework_Tools::mediaSubTypeToString (configuration_r.configuration->streamConfiguration.configuration_.format.subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
                   : 0);
  choice_format->Select (index_i);
  wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                          XRCID ("choice_format"));
  event_s.SetInt (index_i);
  this->AddPendingEvent (event_s);
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_hardware_settings_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_hardware_settings_click_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  ACE_ASSERT ((*stream_iterator).second.second.builder);

  IBaseFilter* filter_p = NULL;
  struct _AMMediaType* media_type_p = NULL;
  HRESULT result =
    (*stream_iterator).second.second.builder->FindFilterByName (STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO,
                                                                &filter_p);
  ACE_ASSERT (SUCCEEDED (result) && filter_p);
  ISpecifyPropertyPages* property_pages_p = NULL;
  result = filter_p->QueryInterface (IID_PPV_ARGS (&property_pages_p));
  ACE_ASSERT (SUCCEEDED (result) && property_pages_p);
  struct tagCAUUID uuids_a;
  ACE_OS::memset (&uuids_a, 0, sizeof (struct tagCAUUID));
  result = property_pages_p->GetPages (&uuids_a);
  ACE_ASSERT (SUCCEEDED (result) && uuids_a.pElems);
  property_pages_p->Release (); property_pages_p = NULL;
  IUnknown* iunknown_p = NULL;
  filter_p->QueryInterface (IID_PPV_ARGS (&iunknown_p));
  ACE_ASSERT (SUCCEEDED (result) && iunknown_p);
  // display modal properties dialog
  // *TODO*: implement modeless support
  //result = OleCreatePropertyFrame (NULL,                     // Parent window {NULL ? modeless : modal}
  result =
    OleCreatePropertyFrame ((HWND)this->GetHandle (), // Parent window {NULL ? modeless : modal}
                            0, 0,                     // Reserved
#if defined (OLE2ANSI)
                            Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str (), // Caption for the dialog box
#else
                            ACE_TEXT_ALWAYS_WCHAR (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()), // Caption for the dialog box
#endif // OLE2ANSI
                            1,                        // Number of objects (just the filter)
                            &iunknown_p,              // Array of object pointers
                            uuids_a.cElems,           // Number of property pages
                            uuids_a.pElems,           // Array of property page CLSIDs
                            0,                        // Locale identifier
                            0, NULL);                 // Reserved

  iunknown_p->Release (); iunknown_p = NULL;
  CoTaskMemFree (uuids_a.pElems); uuids_a.pElems = NULL;
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::choice_format_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_format_selected_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  wxStringClientData* client_data_p =
    dynamic_cast<wxStringClientData*> (choice_format->GetClientObject (event_in.GetSelection ()));
  ACE_ASSERT (client_data_p);
  struct _GUID format_s =
    Common_Tools::StringToGUID (client_data_p->GetData ().ToStdString ());
  ACE_ASSERT (!InlineIsEqualGUID (format_s, GUID_NULL));
  int index_i = wxNOT_FOUND;
  Common_UI_Resolutions_t resolutions_a =
    Stream_Device_DirectShow_Tools::getCaptureResolutions (configuration_r.streamConfiguration,
                                                           format_s);
  ACE_ASSERT (!resolutions_a.empty ());
  Common_UI_Resolution_t resolution_s =
    Stream_MediaFramework_DirectShow_Tools::toResolution (configuration_r.configuration->streamConfiguration.configuration_.format);

  choice_resolution->SetSelection (wxNOT_FOUND);
  choice_resolution->Clear ();
  std::stringstream converter;
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
    converter << (*iterator).cx;
    converter << ACE_TEXT_ALWAYS_CHAR (" ");
    converter << (*iterator).cy;
    client_data_p->SetData (converter.str ());

    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (*iterator).cx;
    converter << ACE_TEXT_ALWAYS_CHAR (" x ");
    converter << (*iterator).cy;

    index_i = choice_resolution->Append (converter.str (),
                                         client_data_p);
    if ((resolution_s.cx == (*iterator).cx) &&
        (resolution_s.cy == (*iterator).cy))
      resolution_string = converter.str ();
  } // end FOR
  ACE_ASSERT (!resolution_string.empty ());
  choice_resolution->Enable (!resolutions_a.empty ());
  index_i =
    (initializing_ ? choice_resolution->FindString (resolution_string)
                   : 0);
  choice_resolution->Select (index_i);
  wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                          XRCID ("choice_resolution"));
  event_s.SetInt (index_i);
  this->AddPendingEvent (event_s);
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::choice_resolution_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_resolution_selected_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  wxStringClientData* client_data_p =
    dynamic_cast<wxStringClientData*> (choice_format->GetClientObject (choice_format->GetSelection ()));
  ACE_ASSERT (client_data_p);
  struct _GUID format_s =
    Common_Tools::StringToGUID (client_data_p->GetData ().ToStdString ());
  ACE_ASSERT (!InlineIsEqualGUID (format_s, GUID_NULL));
  client_data_p =
    dynamic_cast<wxStringClientData*> (choice_resolution->GetClientObject (event_in.GetSelection ()));
  ACE_ASSERT (client_data_p);
  std::stringstream converter (client_data_p->GetData ().ToStdString ());
  Common_UI_Resolution_t resolution_s;
  converter >> resolution_s.cx;
  converter >> resolution_s.cy;
  Common_UI_Framerates_t framerates_a =
    Stream_Device_DirectShow_Tools::getCaptureFramerates (configuration_r.streamConfiguration,
                                                          format_s,
                                                          resolution_s);
  ACE_ASSERT (!framerates_a.empty ());
  unsigned int framerate_i =
    Stream_MediaFramework_DirectShow_Tools::toFramerate (configuration_r.configuration->streamConfiguration.configuration_.format);

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
    if (framerate_i == *iterator)
      framerate_string = converter.str ();
  } // end FOR
  choice_framerate->Enable (!framerates_a.empty ());
  index_i =
    (initializing_ ? choice_framerate->FindString (framerate_string)
                   : 0);
  choice_framerate->Select (index_i);
  wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                          XRCID ("choice_framerate"));
  event_s.SetInt (index_i);
  this->AddPendingEvent (event_s);
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::choice_framerate_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_framerate_selected_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator_2 =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (Stream_Visualization_Tools::rendererToModuleName (configuration_r.configuration->streamConfiguration.configuration_.renderer).c_str ()));
  ACE_ASSERT (stream_iterator_2 != configuration_r.configuration->streamConfiguration.end ());
  wxStringClientData* client_data_p =
    dynamic_cast<wxStringClientData*> (choice_source->GetClientObject (choice_source->GetSelection ()));
  ACE_ASSERT (client_data_p);
  ACE_ASSERT ((*stream_iterator).second.second.deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::STRING);
  if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR ((*stream_iterator).second.second.deviceIdentifier.identifier._string),
                      client_data_p->GetData ().ToStdString ().c_str ()))
    reset_ = true;
  (*stream_iterator).second.second.deviceIdentifier.identifierDiscriminator =
    Stream_Device_Identifier::STRING;
  ACE_OS::strcpy ((*stream_iterator).second.second.deviceIdentifier.identifier._string,
                  client_data_p->GetData ().ToStdString ().c_str ());
  client_data_p =
    dynamic_cast<wxStringClientData*> (choice_format->GetClientObject (choice_format->GetSelection ()));
  ACE_ASSERT (client_data_p);
  struct _GUID format_s =
    Common_Tools::StringToGUID (client_data_p->GetData ().ToStdString ());
  ACE_ASSERT (!InlineIsEqualGUID (format_s, GUID_NULL));
  client_data_p =
    dynamic_cast<wxStringClientData*> (choice_resolution->GetClientObject (choice_resolution->GetSelection ()));
  ACE_ASSERT (client_data_p);
  std::istringstream converter;
  converter.str (client_data_p->GetData ().ToStdString ());
  Common_UI_Resolution_t resolution_s;
  converter >> resolution_s.cx;
  converter >> resolution_s.cy;
  converter.clear ();
  converter.str (choice_framerate->GetString (choice_framerate->GetSelection ()).ToStdString ());
  unsigned int framerate_i = 0;
  converter >> framerate_i;

  struct _AMMediaType* media_type_p = NULL;
  Stream_MediaFramework_DirectShow_Tools::free (configuration_r.configuration->streamConfiguration.configuration_.format);
  if (!Stream_Device_DirectShow_Tools::getVideoCaptureFormat ((*stream_iterator).second.second.builder,
                                                              format_s,
                                                              resolution_s.cx, resolution_s.cy,
                                                              framerate_i,
                                                              configuration_r.configuration->streamConfiguration.configuration_.format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::getVideoCaptureFormat(\"%s\",%s,%dx%d,%d), returning\n"),
                ACE_TEXT (choice_source->GetString (choice_source->GetSelection ()).ToStdString ().c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (format_s, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()),
                resolution_s.cx, resolution_s.cy,
                framerate_i));
    return;
  } // end IF

  // update controls
  if (initializing_)
    togglebutton_record->Enable (true);
  spincontrol_buffer->SetValue (Stream_MediaFramework_DirectShow_Tools::toFramesize (configuration_r.configuration->streamConfiguration.configuration_.format));
  IBaseFilter* filter_p = NULL;
  HRESULT result =
    (*stream_iterator).second.second.builder->FindFilterByName (STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO,
                                                                &filter_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_p =
    Stream_MediaFramework_DirectShow_Tools::defaultCaptureFormat (filter_p);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::defaultCaptureFormat(\"%s\"), returning\n"),
                ACE_TEXT (choice_source->GetString (choice_source->GetSelection ()).ToStdString ().c_str ())));
    filter_p->Release (); filter_p = NULL;
    return;
  } // end IF
  filter_p->Release (); filter_p = NULL;
  button_reset_format->Enable (!Stream_MediaFramework_DirectShow_Tools::match (configuration_r.configuration->streamConfiguration.configuration_.format,
                                                                               *media_type_p));
  Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_reset_format_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_reset_format_click_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  ACE_ASSERT ((*stream_iterator).second.second.builder);

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    (*stream_iterator).second.second.builder->FindFilterByName (STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO,
                                                                &filter_p);
  ACE_ASSERT (SUCCEEDED (result));
  struct _AMMediaType* media_type_p =
    Stream_MediaFramework_DirectShow_Tools::defaultCaptureFormat (filter_p);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::defaultCaptureFormat(\"%s\"), returning\n"),
                ACE_TEXT (choice_source->GetString (choice_source->GetSelection ()).ToStdString ().c_str ())));
    filter_p->Release (); filter_p = NULL;
    return;
  } // end IF
  filter_p->Release (); filter_p = NULL;
  int index_i =
    choice_format->FindString (Stream_MediaFramework_Tools::mediaSubTypeToString (media_type_p->subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW));
  ACE_ASSERT (index_i != wxNOT_FOUND);
  choice_format->Select (index_i);
  wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                          XRCID ("choice_format"));
  event_s.SetInt (index_i);
  //choice_format->GetEventHandler ()->ProcessEvent (event_s);
  this->AddPendingEvent (event_s);
  application_->wait ();

  std::ostringstream converter;
  std::string resolution_string;
  Common_UI_Resolution_t resolution_s =
    Stream_MediaFramework_DirectShow_Tools::toResolution (*media_type_p);
  converter << resolution_s.cx;
  converter << ACE_TEXT_ALWAYS_CHAR (" ");
  converter << resolution_s.cy;
  resolution_string = converter.str ();
  index_i = Common_UI_WxWidgets_Tools::clientDataToIndex (choice_resolution,
                                                          resolution_string);
  ACE_ASSERT (index_i != wxNOT_FOUND);
  choice_resolution->Select (index_i);
  wxCommandEvent event_2 (wxEVT_COMMAND_CHOICE_SELECTED,
                          XRCID ("choice_resolution"));
  event_2.SetInt (index_i);
  //choice_source->GetEventHandler ()->ProcessEvent (event_s);
  this->AddPendingEvent (event_2);
  application_->wait ();

  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  std::string framerate_string;
  unsigned int framerate_i =
    Stream_MediaFramework_DirectShow_Tools::toFramerate (*media_type_p);
  converter << framerate_i;
  framerate_string = converter.str ();
  index_i = choice_framerate->FindString (framerate_string);
  ACE_ASSERT (index_i != wxNOT_FOUND);
  choice_framerate->Select (index_i);
  wxCommandEvent event_3 (wxEVT_COMMAND_CHOICE_SELECTED,
                          XRCID ("choice_framerate"));
  event_3.SetInt (index_i);
  //choice_source->GetEventHandler ()->ProcessEvent (event_s);
  this->AddPendingEvent (event_3);

  Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);

  button_reset_format->Enable (false);
}

void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::togglebutton_save_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_save_toggled_cb"));

  bool is_checked_b = event_in.IsChecked ();
  textcontrol_filename->Enable (is_checked_b);
  directorypicker_save->Enable (is_checked_b);
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::picker_directory_save_changed_cb (wxFileDirPickerEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::picker_directory_save_changed_cb"));

}

void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::togglebutton_display_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_display_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (Stream_Visualization_Tools::rendererToModuleName (configuration_r.configuration->streamConfiguration.configuration_.renderer).c_str ()));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());

  bool is_checked_b = event_in.IsChecked ();
  togglebutton_fullscreen->Enable (is_checked_b);
  choice_display->Enable (is_checked_b);
  button_display_settings->Enable (is_checked_b);

  if (is_checked_b)
  {
    wxRect rectangle_s = panel_video->GetClientRect ();
    (*stream_iterator).second.second.area.left = rectangle_s.GetX ();
    (*stream_iterator).second.second.area.right =
      (*stream_iterator).second.second.area.left + rectangle_s.GetWidth ();
    (*stream_iterator).second.second.area.top = rectangle_s.GetY ();
    (*stream_iterator).second.second.area.bottom =
      (*stream_iterator).second.second.area.top + rectangle_s.GetHeight ();
    wxStringClientData* client_data_p =
      dynamic_cast<wxStringClientData*> (choice_display->GetClientObject (choice_display->GetSelection ()));
    ACE_ASSERT (client_data_p);
    (*stream_iterator).second.second.deviceIdentifier.identifierDiscriminator =
      Stream_Device_Identifier::STRING;
    ACE_OS::strcpy ((*stream_iterator).second.second.deviceIdentifier.identifier._string,
                    client_data_p->GetData ().ToStdString ().c_str ());
  } // end IF
  else
  {
    ACE_OS::memset (&(*stream_iterator).second.second.area, 0, sizeof (struct tagRECT));
    (*stream_iterator).second.second.deviceIdentifier.clear ();
  } // end ELSE

  switch (configuration_r.configuration->streamConfiguration.configuration_.renderer)
  {
    //case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_2D:
    //  break;
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D:
    {
      //ACE_ASSERT ((*stream_iterator).second.second.direct3DConfiguration);
      if (is_checked_b)
      {
        configuration_r.configuration->direct3DConfiguration.focusWindow =
          (HWND)panel_video->GetHandle ();
      } // end IF
      else
      {
        configuration_r.configuration->direct3DConfiguration.focusWindow =
          NULL;
        configuration_r.configuration->direct3DConfiguration.presentationParameters.hDeviceWindow =
          NULL;
        configuration_r.configuration->direct3DConfiguration.presentationParameters.Windowed =
          FALSE;
      } // end ELSE
      break;
    }
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTSHOW:
    case STREAM_VISUALIZATION_VIDEORENDERER_GDI:
      break;
    //case STREAM_VISUALIZATION_VIDEORENDERER_MEDIAFOUNDATION:
    //  break;
#if defined (GTK_USE)
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO:
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_PIXBUF:
      break;
#endif // GTK_USE
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown video renderer (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  configuration_r.configuration->streamConfiguration.configuration_.renderer));
      return;
    }
  } // end SWITCH
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::togglebutton_fullscreen_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_fullscreen_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (Stream_Visualization_Tools::rendererToModuleName (configuration_r.configuration->streamConfiguration.configuration_.renderer).c_str ()));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  ACE_ASSERT (configuration_r.stream);

  choice_resolution_2->Enable (event_in.IsChecked ());
  if (event_in.IsChecked ())
  { // --> toggle to fullscreen
    struct Common_UI_DisplayDevice display_device_s =
      Common_UI_Tools::getDisplay (ACE_TEXT_ALWAYS_CHAR ((*stream_iterator).second.second.deviceIdentifier.identifier._string));
    (*stream_iterator).second.second.area = display_device_s.clippingArea;
    (*stream_iterator).second.second.fullScreen = true;
  } // end IF
  else
  { // toggle to windowed
    wxRect rectangle_s = panel_video->GetClientRect ();
    (*stream_iterator).second.second.area.left = rectangle_s.GetX ();
    (*stream_iterator).second.second.area.right =
      (*stream_iterator).second.second.area.left + rectangle_s.GetWidth ();
    (*stream_iterator).second.second.area.top = rectangle_s.GetY ();
    (*stream_iterator).second.second.area.bottom =
      (*stream_iterator).second.second.area.top + rectangle_s.GetHeight ();
    (*stream_iterator).second.second.fullScreen = false;
  } // end ELSE

  std::string module_name_string =
    Stream_Visualization_Tools::rendererToModuleName (configuration_r.configuration->streamConfiguration.configuration_.renderer);
  switch (configuration_r.configuration->streamConfiguration.configuration_.renderer)
  {
    //case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_2D:
    //  break;
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D:
    {
      //ACE_ASSERT ((*stream_iterator).second.second.direct3DConfiguration);
      if (event_in.IsChecked ())
      { // --> toggle to fullscreen
        configuration_r.configuration->direct3DConfiguration.presentationParameters.hDeviceWindow =
          NULL;
        configuration_r.configuration->direct3DConfiguration.presentationParameters.Windowed =
          FALSE;
      } // end IF
      else
      { // toggle to windowed
        configuration_r.configuration->direct3DConfiguration.presentationParameters.hDeviceWindow =
          (HWND)panel_video->GetHandle ();
        configuration_r.configuration->direct3DConfiguration.presentationParameters.Windowed =
          TRUE;
      } // end ELSE
      break;
    }
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTSHOW:
    case STREAM_VISUALIZATION_VIDEORENDERER_GDI:
      break;
    //case STREAM_VISUALIZATION_VIDEORENDERER_MEDIAFOUNDATION:
    //  break;
#if defined (GTK_USE)
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO:
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_PIXBUF:
      break;
#endif // GTK_USE
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown video renderer (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
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
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::choice_adapter_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_adapter_selected_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::choice_display_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_display_selected_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (Stream_Visualization_Tools::rendererToModuleName (configuration_r.configuration->streamConfiguration.configuration_.renderer).c_str ()));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  wxStringClientData* client_data_p =
    dynamic_cast<wxStringClientData*> (choice_display->GetClientObject (event_in.GetSelection ()));
  ACE_ASSERT (client_data_p);
  (*stream_iterator).second.second.deviceIdentifier.identifierDiscriminator =
    Stream_Device_Identifier::STRING;
  ACE_OS::strcpy ((*stream_iterator).second.second.deviceIdentifier.identifier._string,
                  client_data_p->GetData ().ToStdString ().c_str ());
  ACE_ASSERT (ACE_OS::strlen ((*stream_iterator).second.second.deviceIdentifier.identifier._string));
  static struct Common_UI_DisplayDevice display_device_s =
    Common_UI_Tools::getDisplay (ACE_TEXT_ALWAYS_CHAR ((*stream_iterator).second.second.deviceIdentifier.identifier._string));
  ACE_ASSERT (display_device_s.handle != NULL);
  static struct Common_UI_DisplayAdapter display_adapter_s =
    Common_UI_Tools::getAdapter (display_device_s);
  ACE_ASSERT (!display_adapter_s.id.empty ());
  int index_i = choice_adapter->FindString (display_adapter_s.description);
  ACE_ASSERT (index_i != wxNOT_FOUND);
  choice_adapter->Select (index_i);
  wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                          XRCID ("choice_adapter"));
  event_s.SetInt (index_i);
  //choice_source->GetEventHandler ()->ProcessEvent (event_s);
  this->AddPendingEvent (event_s);
  //application_->wait ();

  button_display_settings->Enable (togglebutton_display->IsEnabled ());

  Common_UI_Resolutions_t resolutions_a =
    Common_UI_Tools::get (ACE_TEXT_ALWAYS_CHAR ((*stream_iterator).second.second.deviceIdentifier.identifier._string));
  ACE_ASSERT (!resolutions_a.empty ());
  Common_UI_Resolution_t resolution_s =
    Stream_MediaFramework_DirectShow_Tools::toResolution ((*stream_iterator).second.second.outputFormat);

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
    converter << (*iterator).cx;
    converter << ACE_TEXT_ALWAYS_CHAR (" ");
    converter << (*iterator).cy;
    client_data_p->SetData (converter.str ());

    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (*iterator).cx;
    converter << ACE_TEXT_ALWAYS_CHAR (" x ");
    converter << (*iterator).cy;

    index_i = choice_resolution_2->Append (converter.str (),
                                           client_data_p);
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": supports: %ux%u\n"),
                ACE_TEXT (choice_display->GetString (choice_display->GetSelection ()).ToStdString ().c_str ()),
                (*iterator).cx, (*iterator).cy));
#endif // _DEBUG
    if ((resolution_s.cx == (*iterator).cx) &&
        (resolution_s.cy == (*iterator).cy))
      resolution_string = converter.str ();
  } // end FOR
  ACE_ASSERT (!resolution_string.empty ());
  choice_resolution_2->Enable (!resolutions_a.empty () &&
                               togglebutton_fullscreen->GetValue ());
  index_i =
    (initializing_ ? choice_resolution_2->FindString (resolution_string)
                   : 0);
  choice_resolution_2->Select (index_i);
  wxCommandEvent event_2 (wxEVT_COMMAND_CHOICE_SELECTED,
                          XRCID ("choice_resolution_2"));
  event_2.SetInt (index_i);
  //choice_resolution_2->GetEventHandler ()->ProcessEvent (event_2);
  this->AddPendingEvent (event_2);
  //application_->wait ();

  if (initializing_)
    initializing_ = false;
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_display_settings_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_display_settings_click_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::choice_resolution_2_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_resolution_2_selected_cb"));

}

void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_about_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_about_click_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);
  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::STATE_T& state_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::STATE_T&> (application_->getR ());
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
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_quit_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_quit_click_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  // step1: make sure the stream has stopped
  Stream_CamSave_DirectShow_Stream* stream_p = NULL;
  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
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

// ---------------------------------------

Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::Stream_CamSave_WxWidgetsDialog_T (wxWindow* parent_in)
 : inherited (parent_in)
 , application_ (NULL)
 , initializing_ (true)
 , untoggling_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::Stream_CamSave_WxWidgetsDialog_T"));

#if defined (ACE_WIN64) || defined (ACE_WIN32)
  inherited::MSWSetOldWndProc (NULL);
#endif // ACE_WIN64 || ACE_WIN32
}

bool
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::OnInit_2 (IAPPLICATION_T* iapplication_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::OnInit_2"));

  // sanity check(s)
  ACE_ASSERT (!application_);

  application_ =
    dynamic_cast<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t*> (iapplication_in);
  ACE_ASSERT (application_);

  togglebutton_record = XRCCTRL (*this, "togglebutton_record", wxToggleButton);
  button_snapshot = XRCCTRL (*this, "button_snapshot", wxBitmapButton);
  button_cut = XRCCTRL (*this, "button_cut", wxBitmapButton);
  button_report = XRCCTRL (*this, "button_report", wxBitmapButton);
  button_snapshot = XRCCTRL (*this, "button_snapshot", wxBitmapButton);
  spincontrol_frames_captured = XRCCTRL (*this, "spincontrol_frames_captured", wxSpinCtrl);
  spincontrol_frames_dropped = XRCCTRL (*this, "spincontrol_frames_dropped", wxSpinCtrl);
  spincontrol_messages_session = XRCCTRL (*this, "spincontrol_messages_session", wxSpinCtrl);
  spincontrol_messages_data = XRCCTRL (*this, "spincontrol_messages_data", wxSpinCtrl);
  spincontrol_data = XRCCTRL (*this, "spincontrol_data", wxSpinCtrl);
  spincontrol_buffer = XRCCTRL (*this, "spincontrol_buffer", wxSpinCtrl);
  choice_source = XRCCTRL (*this, "choice_source", wxChoice);
  button_hardware_settings = XRCCTRL (*this, "button_hardware_settings", wxBitmapButton);
  choice_format = XRCCTRL (*this, "choice_format", wxChoice);
  choice_resolution = XRCCTRL (*this, "choice_resolution", wxChoice);
  choice_framerate = XRCCTRL (*this, "choice_framerate", wxChoice);
  button_reset_format = XRCCTRL (*this, "button_reset_format", wxBitmapButton);
  togglebutton_save = XRCCTRL (*this, "togglebutton_save", wxToggleButton);
  textcontrol_filename = XRCCTRL (*this, "textcontrol_filename", wxTextCtrl);
  directorypicker_save = XRCCTRL (*this, "directorypicker_save", wxDirPickerCtrl);
  togglebutton_display = XRCCTRL (*this, "togglebutton_display", wxToggleButton);
  togglebutton_fullscreen = XRCCTRL (*this, "togglebutton_fullscreen", wxToggleButton);
  choice_display = XRCCTRL (*this, "choice_display", wxChoice);
  button_display_settings = XRCCTRL (*this, "button_display_settings", wxBitmapButton);
  choice_resolution_2 = XRCCTRL (*this, "choice_resolution_2", wxChoice);
  panel_video = XRCCTRL (*this, "panel_video", wxPanel);
  button_about = XRCCTRL (*this, "button_about", wxBitmapButton);
  button_quit = XRCCTRL (*this, "button_quit", wxBitmapButton);
  gauge_progress = XRCCTRL (*this, "gauge_progress", wxGauge);

  // populate controls
#if defined (_DEBUG)
#else
  button_report->Show (false);
#endif // _DEBUG
  bool activate_source = true;
  Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_MediaFoundation_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  Stream_Device_List_t devices_a =
    Stream_Device_DirectShow_Tools::getCaptureDevices (CLSID_VideoInputDeviceCategory);

  int index_i = 0, index_source_i = -1;
  ACE_ASSERT ((*stream_iterator).second.second.deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::STRING);
  for (Stream_Device_ListIterator_t iterator = devices_a.begin ();
       iterator != devices_a.end ();
       ++iterator)
  { ACE_ASSERT ((*iterator).identifierDiscriminator == Stream_Device_Identifier::STRING);
    index_i =
      choice_source->Append (Stream_Device_DirectShow_Tools::devicePathToString ((*iterator).identifier._string).c_str ());
    if (!ACE_OS::strcmp ((*iterator).identifier._string,
                         ACE_TEXT_ALWAYS_CHAR ((*stream_iterator).second.second.deviceIdentifier.identifier._string)))
      index_source_i = index_i;
  } // end FOR
  if (unlikely (devices_a.empty ()))
    activate_source = false;
  else
    choice_source->Enable (true);

  if (likely (activate_source))
  {
    choice_source->Select (index_source_i);
    if (index_source_i == 0)
    { // already selected; trigger event manually
      wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                              XRCID ("choice_source"));
      //choice_source->GetEventHandler ()->ProcessEvent (event_s);
      this->AddPendingEvent (event_s);
    } // end IF
  } // end IF

  return true;
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::OnExit_2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::OnExit_2"));

  // sanity check(s)
  ACE_ASSERT (application_);
}

//////////////////////////////////////////

void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::dialog_main_idle_cb (wxIdleEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::dialog_main_idle_cb"));

  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);

  if (configuration_r.stream->isRunning ())
    gauge_progress->Pulse ();
}

void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::dialog_main_keydown_cb (wxKeyEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::dialog_main_keydown_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);
}

//////////////////////////////////////////

void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::togglebutton_record_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_record_toggled_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::button_snapshot_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_snapshot_click_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::button_cut_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_cut_click_cb"));

}
#if defined (_DEBUG)
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::button_report_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_report_click_cb"));

}
#endif // _DEBUG
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::choice_source_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_source_selected_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  ACE_ASSERT (configuration_r.stream);
  Stream_CamSave_MediaFoundation_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  std::string device_identifier;
  int index_i = -1;

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
  ACE_ASSERT (!device_identifier.empty ());

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  if (!Stream_Device_MediaFoundation_Tools::getMediaSource (device_identifier,
                                                            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                            media_source_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::getMediaSource(\"%s\"), returning\n"),
                ACE_TEXT (device_identifier.c_str ())));
    return;
  } // end IF
  ACE_ASSERT (media_source_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (configuration_r.stream->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_NULL_DEFAULT_NAME_STRING)));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::find(\"%s\"), returning\n"),
                ACE_TEXT (STREAM_VIS_NULL_DEFAULT_NAME_STRING)));
    media_source_p->Release (); media_source_p = NULL;
    return;
  } // end IF
  Stream_CamSave_MediaFoundation_MediaFoundationDisplayNull* display_impl_p =
    dynamic_cast<Stream_CamSave_MediaFoundation_MediaFoundationDisplayNull*> (module_p->writer ());
  ACE_ASSERT (display_impl_p);

  IMFTopology* topology_p = NULL;
  struct _MFRatio pixel_aspect_ratio = { 1, 1 };
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  if (!Stream_Device_MediaFoundation_Tools::loadDeviceTopology (device_identifier,
                                                                        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                                        media_source_p,
                                                                        display_impl_p,
                                                                        topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::loadDeviceTopology(), returning\n")));
    media_source_p->Release (); media_source_p = NULL;
    return;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
  ACE_ASSERT (topology_p);
  media_source_p->Release (); media_source_p = NULL;

  // sanity check(s)
  ACE_ASSERT ((*stream_iterator).second.second.session);

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                 (*stream_iterator).second.second.session,
                                                                 true,
                                                                 true))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::setTopology(), returning\n")));
    topology_p->Release (); topology_p = NULL;
    return;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  topology_p->Release (); topology_p = NULL;

  if (configuration_r.configuration->streamConfiguration.configuration_.format)
  {
    configuration_r.configuration->streamConfiguration.configuration_.format->Release (); configuration_r.configuration->streamConfiguration.configuration_.format = NULL;
  } // end IF
  HRESULT result_2 =
    MFCreateMediaType (&configuration_r.configuration->streamConfiguration.configuration_.format);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", returning\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return;
  } // end IF
  ACE_ASSERT (configuration_r.configuration->streamConfiguration.configuration_.format);
  result_2 =
    configuration_r.configuration->streamConfiguration.configuration_.format->SetGUID (MF_MT_MAJOR_TYPE,
                                                                                       MFMediaType_Video);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 =
    configuration_r.configuration->streamConfiguration.configuration_.format->SetUINT32 (MF_MT_INTERLACE_MODE,
                                                                                         MFVideoInterlace_Unknown);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 =
    MFSetAttributeRatio (configuration_r.configuration->streamConfiguration.configuration_.format,
                         MF_MT_PIXEL_ASPECT_RATIO,
                         pixel_aspect_ratio.Numerator,
                         pixel_aspect_ratio.Denominator);
  ACE_ASSERT (SUCCEEDED (result_2));

  //if (_DEBUG)
  //{
  //  std::string log_file_name =
  //    Common_File_Tools::getLogDirectory (std::string (),
  //                                        0);
  //  log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
  //  log_file_name += STREAM_DEV_DIRECTSHOW_LOGFILE_NAME;
  //  Stream_Device_Tools::debug (data_p->configuration->moduleHandlerConfiguration.builder,
  //                                     log_file_name);
  //} // end IF
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::button_hardware_settings_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_hardware_settings_click_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::choice_format_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_format_selected_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::choice_resolution_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_resolution_selected_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::choice_framerate_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_framerate_selected_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::button_reset_format_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_reset_format_click_cb"));

}

void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::togglebutton_save_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_save_toggled_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::picker_directory_save_changed_cb (wxFileDirPickerEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::picker_directory_save_changed_cb"));

}

void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::togglebutton_display_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_display_toggled_cb"));

  bool is_checked_b = event_in.IsChecked ();
  togglebutton_fullscreen->Enable (is_checked_b);
  choice_display->Enable (is_checked_b);
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::togglebutton_fullscreen_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_fullscreen_toggled_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::choice_adapter_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_adapter_selected_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::choice_display_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_display_selected_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::button_display_settings_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_display_settings_click_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::choice_resolution_2_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_resolution_2_selected_cb"));

}

void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::button_about_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_about_click_cb"));

  wxAboutDialogInfo about_dialog_info;
  about_dialog_info.SetName (_ ("My Program"));
  about_dialog_info.SetVersion (_ ("1.2.3 Beta"));
  about_dialog_info.SetDescription (_ ("This program does something great."));
  about_dialog_info.SetCopyright (wxT ("(C) 2007 Me <my@email.addre.ss>"));
  wxAboutBox (about_dialog_info);
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::button_quit_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_quit_click_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  // step1: make sure the stream has stopped
  Stream_CamSave_MediaFoundation_Stream* stream_p = NULL;
  Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
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
#else
#endif // ACE_WIN32 || ACE_WIN64
