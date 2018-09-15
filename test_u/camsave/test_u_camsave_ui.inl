#if defined (ACE_WIN64) || defined (ACE_WIN32)
// *TODO*: find a way to include uuids.h here
#if defined (UUIDS_H)
#else
#ifndef OUR_GUID_ENTRY
    #define OUR_GUID_ENTRY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8);
#endif
OUR_GUID_ENTRY(CLSID_VideoInputDeviceCategory,
0x860BB310,0x5D01,0x11d0,0xBD,0x3B,0x00,0xA0,0xC9,0x11,0xCE,0x86)
#define UUIDS_H
//#include <uuids.h>
#endif // UUIDS_H
#endif // ACE_WIN64 || ACE_WIN32

#include "wx/aboutdlg.h"

#include "ace/Log_Msg.h"
#include "ace/OS_NS_Thread.h"

#include "stream_macros.h"

#if defined (ACE_WIN64) || defined (ACE_WIN32)
#include "stream_dev_directshow_tools.h"
#endif // ACE_WIN64 || ACE_WIN32

#include "test_u_tools.h"

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
                                 StreamType>::OnInit_2 (Common_UI_wxWidgets_ITopLevel_T<typename InterfaceType::STATE_T,
                                                                                        typename InterfaceType::CONFIGURATION_T>::IAPPLICATION_T* iapplication_in)
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
  button_reset = XRCCTRL (*this, "button_reset", wxBitmapButton);
  togglebutton_save = XRCCTRL (*this, "togglebutton_save", wxToggleButton);
  textcontrol_filename = XRCCTRL (*this, "textcontrol_filename", wxTextCtrl);
  directorypicker_save = XRCCTRL (*this, "directorypicker_save", wxDirPickerCtrl);
  togglebutton_fullscreen = XRCCTRL (*this, "togglebutton_fullscreen", wxToggleButton);
  button_display_settings = XRCCTRL (*this, "button_display_settings", wxBitmapButton);
  panel_video = XRCCTRL (*this, "panel_video", wxPanel);
  button_about = XRCCTRL (*this, "button_about", wxBitmapButton);
  button_quit = XRCCTRL (*this, "button_quit", wxBitmapButton);
  gauge_progress = XRCCTRL (*this, "gauge_progress", wxGauge);

  // populate controls
  bool activate_source = true;
  unsigned int active_source = 0;
  InterfaceType::CONFIGURATION_T& configuration_r =
    const_cast<InterfaceType::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  StreamType::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  Stream_Module_Device_List_t devices_a;
#if defined (ACE_WIN64) || defined (ACE_WIN32)
  switch (configuration_r.mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      devices_a =
        Stream_Module_Device_DirectShow_Tools::getCaptureDevices (CLSID_VideoInputDeviceCategory);
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
  for (Stream_Module_Device_ListIterator_t iterator = devices_a.begin ();
       iterator != devices_a.end ();
       ++iterator)
  {
#if defined (ACE_WIN64) || defined (ACE_WIN32)
    switch (configuration_r.mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        index_i =
          choice_source->Append (Stream_Module_Device_DirectShow_Tools::devicePathToString (*iterator).c_str ());
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
  {
    choice_source->Enable (true);
    button_hardware_settings->Enable (true);
  } // end ELSE

  if (likely (activate_source))
  {
    index_i =
      (initializing_ ? choice_source->FindString ((*stream_iterator).second.second.deviceIdentifier.c_str ())
                     : 0);
    choice_source->Select (index_i);
    wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                            XRCID ("choice_source"));
    event_s.SetInt (index_i);
    //choice_source->GetEventHandler ()->ProcessEvent (event_s);
    this->AddPendingEvent (&event_s);
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
                                 StreamType>::OnIdle (wxIdleEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::OnIdle"));

  // sanity check(s)
  ACE_ASSERT (application_);

  InterfaceType::CONFIGURATION_T& configuration_r =
    const_cast<InterfaceType::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);

  if (configuration_r.stream->IsRunning ())
    gauge_progress->Pulse ();
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

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::button_cut_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_cut_click_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::button_report_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_report_click_cb"));

}
template <typename InterfaceType,
          typename StreamType>
void
Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                 StreamType>::choice_source_selected_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::choice_source_selected_cb"));

  // sanity check(s)
  ACE_ASSERT (application_);

  InterfaceType::CONFIGURATION_T& configuration_r =
    const_cast<InterfaceType::CONFIGURATION_T&> (application_->getR_2 ());
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
  int index_i = -1;
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
                                 StreamType>::togglebutton_fullscreen_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_fullscreen_toggled_cb"));

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
  InterfaceType::CONFIGURATION_T& configuration_r =
    const_cast<InterfaceType::CONFIGURATION_T&> (application_->getR_2 ());
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

Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::Stream_CamSave_WxWidgetsDialog_T (wxWindow* parent_in)
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
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::OnInit_2 (Common_UI_wxWidgets_ITopLevel_T<struct Common_UI_wxWidgets_State,
                                                                                                              struct Stream_CamSave_DirectShow_UI_CBData>::IAPPLICATION_T* iapplication_in)
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
  button_reset = XRCCTRL (*this, "button_reset", wxBitmapButton);
  togglebutton_save = XRCCTRL (*this, "togglebutton_save", wxToggleButton);
  textcontrol_filename = XRCCTRL (*this, "textcontrol_filename", wxTextCtrl);
  directorypicker_save = XRCCTRL (*this, "directorypicker_save", wxDirPickerCtrl);
  togglebutton_fullscreen = XRCCTRL (*this, "togglebutton_fullscreen", wxToggleButton);
  button_display_settings = XRCCTRL (*this, "button_display_settings", wxBitmapButton);
  panel_video = XRCCTRL (*this, "panel_video", wxPanel);
  button_about = XRCCTRL (*this, "button_about", wxBitmapButton);
  button_quit = XRCCTRL (*this, "button_quit", wxBitmapButton);
  gauge_progress = XRCCTRL (*this, "gauge_progress", wxGauge);

  // populate controls
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

  bool activate_source = true;
  unsigned int active_source = 0;
  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  Stream_Module_Device_List_t devices_a =
    Stream_Module_Device_DirectShow_Tools::getCaptureDevices (CLSID_VideoInputDeviceCategory);

  int index_i = -1;
  wxStringClientData* client_data_p = NULL;
  for (Stream_Module_Device_ListIterator_t iterator = devices_a.begin ();
       iterator != devices_a.end ();
       ++iterator)
  {
    client_data_p = NULL;
    ACE_NEW_NORETURN (client_data_p,
                      wxStringClientData ());
    ACE_ASSERT (client_data_p);
    client_data_p->SetData (*iterator);

    index_i =
      choice_source->Append (Stream_Module_Device_DirectShow_Tools::devicePathToString (*iterator).c_str (),
                             client_data_p);
  } // end FOR
  if (unlikely (devices_a.empty ()))
    activate_source = false;
  else
  {
    choice_source->Enable (true);
    button_hardware_settings->Enable (true);
  } // end ELSE

  togglebutton_save->SetValue (!(*stream_iterator).second.second.targetFileName.empty ());
  directorypicker_save->Enable (togglebutton_save->GetValue ());
  textcontrol_filename->SetValue (ACE_TEXT_ALWAYS_CHAR (ACE::basename ((*stream_iterator).second.second.targetFileName.c_str (),
                                                                       ACE_DIRECTORY_SEPARATOR_CHAR)));
  directorypicker_save->Enable (togglebutton_save->GetValue ());
  directorypicker_save->SetPath (ACE_TEXT_ALWAYS_CHAR (ACE::dirname ((*stream_iterator).second.second.targetFileName.c_str (),
                                                                     ACE_DIRECTORY_SEPARATOR_CHAR)));
  togglebutton_fullscreen->Enable ((*stream_iterator).second.second.fullScreen);
  //gauge_progress->Pulse ();

  if (likely (activate_source))
  {
    index_i =
      (initializing_ ? choice_source->FindString (Stream_Module_Device_DirectShow_Tools::devicePathToString ((*stream_iterator).second.second.deviceIdentifier),
                                                  false)
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
                                 Stream_CamSave_DirectShow_Stream>::OnIdle (wxIdleEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::OnIdle"));

  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);

  if (configuration_r.stream->isRunning ())
    gauge_progress->Pulse ();
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

  Stream_IStreamControlBase* stream_p = configuration_r.stream;
  ACE_ASSERT (stream_p);

  // toggle ?
  if (!event_in.IsChecked ())
  { // --> user pressed pause/stop
    stream_p->stop (false, // wait ?
                    true,  // recurse upstream ?
                    true); // locked access ?
    return;
  } // end IF

  // --> user pressed record

  struct Stream_CamSave_UI_ThreadData* thread_data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_thread_t thread_id = std::numeric_limits<unsigned long>::max ();
#else
  ACE_thread_t thread_id = -1;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;

  // step1: set up progress reporting
  ACE_OS::memset (&configuration_r.progressData.statistic,
                  0,
                  sizeof (struct Stream_CamSave_StatisticData));
  //gauge_progress->Pulse ();

  // step2: update capture configuration
  configuration_r.configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize =
    spincontrol_buffer->GetValue ();
  ACE_ASSERT ((*stream_iterator).second.second.sourceFormat);
  wxStringClientData* client_data_p =
    dynamic_cast<wxStringClientData*> (choice_source->GetClientObject (choice_source->GetSelection ()));
  ACE_ASSERT (client_data_p);
  (*stream_iterator).second.second.deviceIdentifier =
    client_data_p->GetData ().ToStdString ();
  // *NOTE*: the format is synchronized automatically
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
  wxRect rectangle_s = panel_video->GetClientRect ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  (*stream_iterator).second.second.area.left = rectangle_s.GetX ();
  (*stream_iterator).second.second.area.right =
    (*stream_iterator).second.second.area.left + rectangle_s.GetWidth ();
  (*stream_iterator).second.second.area.top = rectangle_s.GetY ();
  (*stream_iterator).second.second.area.bottom =
    (*stream_iterator).second.second.area.top + rectangle_s.GetHeight ();
#endif // ACE_WIN32 || ACE_WIN64
  (*stream_iterator).second.second.targetFileName = filename_string;
  (*stream_iterator).second.second.window =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    (HWND)panel_video->GetHandle ();
  ACE_ASSERT ((*stream_iterator).second.second.direct3DConfiguration);
  (*stream_iterator).second.second.direct3DConfiguration->focusWindow =
    (HWND)panel_video->GetHandle ();
  //panel_video->GetClientSize (reinterpret_cast<int*> (&(*stream_iterator).second.second.direct3DConfiguration->presentationParameters.BackBufferWidth),
  //                            reinterpret_cast<int*> (&(*stream_iterator).second.second.direct3DConfiguration->presentationParameters.BackBufferHeight));
#else
    (xid)panel_video->GetHandle ();
#endif // ACE_WIN32 || ACE_WIN64

  // step3: start processing thread(s)
  ACE_Thread_ID thread_id_2;
  bool result =
    Test_U_Tools::spawn<struct Stream_CamSave_UI_ThreadData,
                        Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T> (ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_THREAD_NAME),
                                                                                             ::stream_processing_thread,
                                                                                             COMMON_EVENT_REACTOR_THREAD_GROUP_ID + 1,
                                                                                             configuration_r,
                                                                                             thread_id_2);
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Test_U_Tools::spawn(): \"%m\", returning\n")));
    return;
  } // end IF
  result =
    Test_U_Tools::spawn<struct Stream_CamSave_UI_ThreadData,
                        Stream_CamSave_DirectShow_WxWidgetsIApplication_t::CONFIGURATION_T> (ACE_TEXT_ALWAYS_CHAR (TEST_U_EVENT_THREAD_NAME),
                                                                                             ::event_processing_thread,
                                                                                             COMMON_EVENT_REACTOR_THREAD_GROUP_ID + 1,
                                                                                             configuration_r,
                                                                                             thread_id_2);
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Test_U_Tools::spawn(): \"%m\", returning\n")));
    return;
  } // end IF

  // step0: modify widgets
  button_snapshot->Enable (true);
  button_cut->Enable (true);
  button_report->Enable (true);
  gauge_progress->Enable (true);

  choice_source->Enable (false);
  choice_format->Enable (false);
  choice_resolution->Enable (false);
  choice_framerate->Enable (false);
  button_reset->Enable (false);
  togglebutton_save->Enable (false);
  textcontrol_filename->Enable (false);
  directorypicker_save->Enable (false);
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_snapshot_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_snapshot_click_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_cut_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_cut_click_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_report_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_report_click_cb"));

}
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
  if (!Stream_Module_Device_DirectShow_Tools::loadDeviceGraph (device_identifier,
                                                               CLSID_VideoInputDeviceCategory,
                                                               (*stream_iterator).second.second.builder,
                                                               buffer_negotiation_p,
                                                               configuration_r.streamConfiguration,
                                                               graph_layout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), returning\n"),
                ACE_TEXT (device_identifier.c_str ())));
    return;
  } // end IF
  ACE_ASSERT ((*stream_iterator).second.second.builder);
  ACE_ASSERT ((*stream_iterator).second.second.sourceFormat);
  ACE_ASSERT (buffer_negotiation_p);
  ACE_ASSERT (configuration_r.streamConfiguration);

  buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;

  choice_format->Clear ();
  Common_Identifiers_t subformats_a =
    Stream_Module_Device_DirectShow_Tools::getCaptureSubFormats (configuration_r.streamConfiguration);
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
//#if defined (_DEBUG)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("\"%s\": supports: %s %s: %d...\n"),
//                ACE_TEXT (choice_source->GetString (event_in.GetSelection ()).ToStdString ().c_str ()),
//                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (*iterator, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()),
//                ACE_TEXT (Common_Tools::GUIDToString (*iterator).c_str ()),
//                index_i));
//#endif // _DEBUG
  } // end FOR
  if (!choice_format->IsEnabled ()) // initial setup ?
  {
    choice_format->Enable (true);
  } // end IF

  index_i =
    (initializing_ ? choice_format->FindString (Stream_MediaFramework_Tools::mediaSubTypeToString ((*stream_iterator).second.second.sourceFormat->subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
                   : 0);
  choice_format->Select (index_i);
  wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                          XRCID ("choice_format"));
  event_s.SetInt (index_i);
  //choice_format->GetEventHandler ()->ProcessEvent (event_s);
  this->AddPendingEvent (event_s);
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_hardware_settings_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_hardware_settings_click_cb"));

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
  ACE_ASSERT (configuration_r.streamConfiguration);
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
    Stream_Module_Device_DirectShow_Tools::getCaptureResolutions (configuration_r.streamConfiguration,
                                                                  format_s);
  ACE_ASSERT (!resolutions_a.empty ());
  ACE_ASSERT ((*stream_iterator).second.second.sourceFormat);
  Common_UI_Resolution_t resolution_s =
    Stream_MediaFramework_DirectShow_Tools::mediaTypeToResolution (*(*stream_iterator).second.second.sourceFormat);

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
//#if defined (_DEBUG)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("\"%s\": format %s supports: %dx%d...\n"),
//                ACE_TEXT (choice_source->GetString (choice_source->GetSelection ()).ToStdString ().c_str ()),
//                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (format_s, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()),
//                (*iterator).cx, (*iterator).cy));
//#endif // _DEBUG
    if ((resolution_s.cx == (*iterator).cx) &&
        (resolution_s.cy == (*iterator).cy))
      resolution_string = converter.str ();
  } // end FOR
  ACE_ASSERT (!resolution_string.empty ());
  choice_resolution->Enable (true);
  index_i =
    (initializing_ ? choice_resolution->FindString (resolution_string)
                   : 0);
  choice_resolution->Select (index_i);
  wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                          XRCID ("choice_resolution"));
  event_s.SetInt (index_i);
  //choice_source->GetEventHandler ()->ProcessEvent (event_s);
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
  ACE_ASSERT (configuration_r.streamConfiguration);
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
    Stream_Module_Device_DirectShow_Tools::getCaptureFramerates (configuration_r.streamConfiguration,
                                                                 format_s,
                                                                 resolution_s);
  ACE_ASSERT (!framerates_a.empty ());
  ACE_ASSERT ((*stream_iterator).second.second.sourceFormat);
  unsigned int framerate_i =
    Stream_MediaFramework_DirectShow_Tools::mediaTypeToFramerate (*(*stream_iterator).second.second.sourceFormat);

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
//#if defined (_DEBUG)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("\"%s\": format %s, resolution %dx%d supports: %d...\n"),
//                ACE_TEXT (choice_source->GetString (choice_source->GetSelection ()).ToStdString ().c_str ()),
//                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (format_s, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()),
//                resolution_s.cx, resolution_s.cy,
//                *iterator));
//#endif // _DEBUG
    if (framerate_i == *iterator)
      framerate_string = converter.str ();
  } // end FOR
  //ACE_ASSERT (!framerate_string.empty ());
  choice_framerate->Enable (true);
  index_i =
    (initializing_ ? choice_framerate->FindString (framerate_string)
                   : 0);
  choice_framerate->Select (index_i);
  wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                          XRCID ("choice_framerate"));
  event_s.SetInt (index_i);
  //choice_source->GetEventHandler ()->ProcessEvent (event_s);
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
  ACE_ASSERT (configuration_r.streamConfiguration);
  Stream_CamSave_DirectShow_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  ACE_ASSERT ((*stream_iterator).second.second.builder);
  wxStringClientData* client_data_p =
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

  if (!Stream_Module_Device_DirectShow_Tools::getVideoCaptureFormat ((*stream_iterator).second.second.builder,
                                                                     format_s,
                                                                     resolution_s.cx, resolution_s.cy,
                                                                     framerate_i,
                                                                     (*stream_iterator).second.second.sourceFormat))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::getVideoCaptureFormat(\"%s\",%s,%dx%d,%d), returning\n"),
                ACE_TEXT (choice_source->GetString (choice_source->GetSelection ()).ToStdString ().c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (format_s, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()),
                resolution_s.cx, resolution_s.cy,
                framerate_i));
    return;
  } // end IF
  ACE_ASSERT ((*stream_iterator).second.second.sourceFormat);
  ACE_ASSERT ((*stream_iterator).second.second.direct3DConfiguration);
  (*stream_iterator).second.second.direct3DConfiguration->presentationParameters.BackBufferWidth =
    resolution_s.cx;
  (*stream_iterator).second.second.direct3DConfiguration->presentationParameters.BackBufferHeight =
    resolution_s.cy;
  togglebutton_record->Enable (true);
  spincontrol_buffer->SetValue (Stream_MediaFramework_DirectShow_Tools::mediaTypeToFramesize (*(*stream_iterator).second.second.sourceFormat));
  button_reset->Enable (true);
  togglebutton_save->Enable (true);
  togglebutton_fullscreen->Enable (true);
  button_display_settings->Enable (true);

  if (initializing_)
    initializing_ = false;
}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_reset_format_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_reset_format_click_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::togglebutton_save_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_save_toggled_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::picker_directory_save_changed_cb (wxFileDirPickerEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::picker_directory_save_changed_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::togglebutton_fullscreen_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_fullscreen_toggled_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_display_settings_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_display_settings_click_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                 Stream_CamSave_DirectShow_Stream>::button_about_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_about_click_cb"));

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
                                 Stream_CamSave_MediaFoundation_Stream>::OnInit_2 (Common_UI_wxWidgets_ITopLevel_T<struct Common_UI_wxWidgets_State,
                                                                                                                   struct Stream_CamSave_MediaFoundation_UI_CBData>::IAPPLICATION_T* iapplication_in)
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
  button_reset = XRCCTRL (*this, "button_reset", wxBitmapButton);
  togglebutton_save = XRCCTRL (*this, "togglebutton_save", wxToggleButton);
  textcontrol_filename = XRCCTRL (*this, "textcontrol_filename", wxTextCtrl);
  directorypicker_save = XRCCTRL (*this, "directorypicker_save", wxDirPickerCtrl);
  togglebutton_fullscreen = XRCCTRL (*this, "togglebutton_fullscreen", wxToggleButton);
  button_display_settings = XRCCTRL (*this, "button_display_settings", wxBitmapButton);
  panel_video = XRCCTRL (*this, "panel_video", wxPanel);
  button_about = XRCCTRL (*this, "button_about", wxBitmapButton);
  button_quit = XRCCTRL (*this, "button_quit", wxBitmapButton);
  gauge_progress = XRCCTRL (*this, "gauge_progress", wxGauge);

  // populate controls
  bool activate_source = true;
  unsigned int active_source = 0;
  Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.configuration);
  Stream_CamSave_MediaFoundation_Stream::CONFIGURATION_T::ITERATOR_T stream_iterator =
    configuration_r.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != configuration_r.configuration->streamConfiguration.end ());
  Stream_Module_Device_List_t devices_a =
    Stream_Module_Device_DirectShow_Tools::getCaptureDevices (CLSID_VideoInputDeviceCategory);

  int index_i = 0, index_source_i = -1;
  for (Stream_Module_Device_ListIterator_t iterator = devices_a.begin ();
       iterator != devices_a.end ();
       ++iterator)
  {
    index_i =
      choice_source->Append (Stream_Module_Device_DirectShow_Tools::devicePathToString (*iterator).c_str ());
    if (!ACE_OS::strcmp ((*iterator).c_str (),
                         (*stream_iterator).second.second.deviceIdentifier.c_str ()))
      index_source_i = index_i;
  } // end FOR
  if (unlikely (devices_a.empty ()))
    activate_source = false;
  else
  {
    choice_source->Enable (true);
    button_hardware_settings->Enable (true);
  } // end ELSE

  if (likely (activate_source))
  {
    choice_source->Select (index_source_i);
    if (index_source_i == 0)
    { // already selected; trigger event manually
      wxCommandEvent event_s (wxEVT_COMMAND_CHOICE_SELECTED,
                              XRCID ("choice_source"));
      //choice_source->GetEventHandler ()->ProcessEvent (event_s);
      this->QueueEvent (&event_s);
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
                                 Stream_CamSave_MediaFoundation_Stream>::OnIdle (wxIdleEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::OnIdle"));

  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  ACE_ASSERT (application_);

  Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t::CONFIGURATION_T&> (application_->getR_2 ());
  ACE_ASSERT (configuration_r.stream);

  if (configuration_r.stream->isRunning ())
    gauge_progress->Pulse ();
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
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::button_report_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_report_click_cb"));

}
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
  if (!Stream_Module_Device_MediaFoundation_Tools::getMediaSource (device_identifier,
                                                                   MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                                   media_source_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getMediaSource(\"%s\"), returning\n"),
                ACE_TEXT (device_identifier.c_str ())));
    return;
  } // end IF
  ACE_ASSERT (media_source_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (configuration_r.stream->find (ACE_TEXT_ALWAYS_CHAR (MODULE_VIS_RENDERER_NULL_MODULE_NAME)));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::find(\"%s\"), returning\n"),
                ACE_TEXT (MODULE_VIS_RENDERER_NULL_MODULE_NAME)));
    media_source_p->Release (); media_source_p = NULL;
    return;
  } // end IF
  Stream_CamSave_MediaFoundation_MediaFoundationDisplayNull* display_impl_p =
    dynamic_cast<Stream_CamSave_MediaFoundation_MediaFoundationDisplayNull*> (module_p->writer ());
  ACE_ASSERT (display_impl_p);

  IMFTopology* topology_p = NULL;
  struct _MFRatio pixel_aspect_ratio = { 1, 1 };
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  if (!Stream_Module_Device_MediaFoundation_Tools::loadDeviceTopology (device_identifier,
                                                                        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                                        media_source_p,
                                                                        display_impl_p,
                                                                        topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::loadDeviceTopology(), returning\n")));
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

  if ((*stream_iterator).second.second.sourceFormat)
  {
    (*stream_iterator).second.second.sourceFormat->Release (); (*stream_iterator).second.second.sourceFormat = NULL;
  } // end IF
  HRESULT result_2 =
    MFCreateMediaType (&(*stream_iterator).second.second.sourceFormat);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", returning\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return;
  } // end IF
  ACE_ASSERT ((*stream_iterator).second.second.sourceFormat);
  result_2 =
    (*stream_iterator).second.second.sourceFormat->SetGUID (MF_MT_MAJOR_TYPE,
                                                            MFMediaType_Video);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 =
    (*stream_iterator).second.second.sourceFormat->SetUINT32 (MF_MT_INTERLACE_MODE,
                                                              MFVideoInterlace_Unknown);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 =
    MFSetAttributeRatio ((*stream_iterator).second.second.sourceFormat,
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
  //  log_file_name += MODULE_DEV_DIRECTSHOW_LOGFILE_NAME;
  //  Stream_Module_Device_Tools::debug (data_p->configuration->moduleHandlerConfiguration.builder,
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
                                 Stream_CamSave_MediaFoundation_Stream>::togglebutton_fullscreen_toggled_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::togglebutton_fullscreen_toggled_cb"));

}
void
Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                 Stream_CamSave_MediaFoundation_Stream>::button_display_settings_click_cb (wxCommandEvent& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_WxWidgetsDialog_T::button_display_settings_click_cb"));

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
