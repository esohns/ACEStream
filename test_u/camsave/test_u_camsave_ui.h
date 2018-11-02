#ifndef __test_u_camsave_ui__
#define __test_u_camsave_ui__

#include "wx/wx.h"

#include "ace/config-macros.h"

#include "common_ui_wxwidgets_itoplevel.h"

#include "test_u_camsave_common.h"
#include "test_u_camsave_stream.h"
#include "test_u_camsave_ui_base.h"

// helper functions
void process_stream_events (struct Stream_CamSave_UI_CBData*, bool&);

// thread functions
//ACE_THR_FUNC_RETURN event_processing_thread (void*);
ACE_THR_FUNC_RETURN stream_processing_thread (void*);

template <typename InterfaceType, // implements Common_UI_wxWidgets_IApplication_T
          typename StreamType>
class Stream_CamSave_WxWidgetsDialog_T
 : public dialog_main
 , public Common_UI_wxWidgets_ITopLevel_T<typename InterfaceType::STATE_T,
                                          typename InterfaceType::CONFIGURATION_T>
{
  typedef dialog_main inherited;
  //typedef Common_UI_wxWidgets_ITopLevel_T<typename InterfaceType::STATE_T,
  //                                        typename InterfaceType::CONFIGURATION_T> inherited2;

 public:
  // convenient types
  typedef Common_UI_wxWidgets_IApplication_T<typename InterfaceType::STATE_T,
                                             typename InterfaceType::CONFIGURATION_T> IAPPLICATION_T;

  Stream_CamSave_WxWidgetsDialog_T (wxWindow* = NULL); // parent window (if any)
  inline virtual ~Stream_CamSave_WxWidgetsDialog_T () {}

  // implement Common_UI_wxWidgets_ITopLevel
  inline virtual const IAPPLICATION_T* const getP () const { ACE_ASSERT (application_); return application_; }

 private:
  // convenient types
  typedef Stream_CamSave_WxWidgetsDialog_T<InterfaceType,
                                           StreamType> OWN_TYPE_T;

  // implement Common_UI_wxWidgets_ITopLevel
  virtual bool OnInit_2 (IAPPLICATION_T*);
  virtual void OnExit_2 ();

  // event handlers
  virtual void dialog_main_idle_cb (wxIdleEvent&);
  virtual void dialog_main_keydown_cb (wxKeyEvent&);

  // control handlers
  virtual void togglebutton_record_toggled_cb (wxCommandEvent&);
  virtual void button_snapshot_click_cb (wxCommandEvent&);
  virtual void button_cut_click_cb (wxCommandEvent&);
#if defined (_DEBUG)
  virtual void button_report_click_cb (wxCommandEvent&);
#endif // _DEBUG
  virtual void choice_source_selected_cb (wxCommandEvent&);
  virtual void button_hardware_settings_click_cb (wxCommandEvent&);
  virtual void choice_format_selected_cb (wxCommandEvent&);
  virtual void choice_resolution_selected_cb (wxCommandEvent&);
  virtual void choice_framerate_selected_cb (wxCommandEvent&);
  virtual void button_reset_format_click_cb (wxCommandEvent&);
  virtual void togglebutton_save_toggled_cb (wxCommandEvent&);
  virtual void picker_directory_save_changed_cb (wxFileDirPickerEvent&);
  virtual void togglebutton_display_toggled_cb (wxCommandEvent&);
  virtual void togglebutton_fullscreen_toggled_cb (wxCommandEvent&);
  virtual void choice_adapter_selected_cb (wxCommandEvent&);
  virtual void choice_display_selected_cb (wxCommandEvent&);
  virtual void button_display_settings_click_cb (wxCommandEvent&);
  virtual void choice_resolution_2_selected_cb (wxCommandEvent&);
  virtual void button_about_click_cb (wxCommandEvent&);
  virtual void button_quit_click_cb (wxCommandEvent&);

  wxDECLARE_DYNAMIC_CLASS (OWN_TYPE_T);
  wxDECLARE_EVENT_TABLE ();

  InterfaceType* application_;
  bool           initializing_;
  bool           untoggling_;
};

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// specializations (for Win32)
template <>
class Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                       Stream_CamSave_DirectShow_Stream>
 : public dialog_main
 , public Common_UI_wxWidgets_ITopLevel_T<struct Common_UI_wxWidgets_State,
                                          struct Stream_CamSave_DirectShow_UI_CBData>
{
  typedef dialog_main inherited;
  //typedef Common_UI_wxWidgets_ITopLevel_T<struct Common_UI_wxWidgets_State,
  //                                        struct Stream_CamSave_DirectShow_UI_CBData> inherited2;

 public:
  // convenient types
  typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                             struct Stream_CamSave_DirectShow_UI_CBData> IAPPLICATION_T;

  Stream_CamSave_WxWidgetsDialog_T (wxWindow* = NULL); // parent window (if any)
  inline virtual ~Stream_CamSave_WxWidgetsDialog_T () {}

  // implement Common_UI_wxWidgets_ITopLevel
  inline virtual const IAPPLICATION_T* const getP () const { ACE_ASSERT (application_); return application_; }

 private:
  // convenient types
  typedef Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                           Stream_CamSave_DirectShow_Stream> OWN_TYPE_T;

  // implement Common_UI_wxWidgets_ITopLevel
  virtual bool OnInit_2 (IAPPLICATION_T*);
  virtual void OnExit_2 ();

  // event handlers
  virtual void dialog_main_idle_cb (wxIdleEvent&);
  virtual void dialog_main_keydown_cb (wxKeyEvent&);

  // control handlers
  virtual void togglebutton_record_toggled_cb (wxCommandEvent&);
  virtual void button_snapshot_click_cb (wxCommandEvent&);
  virtual void button_cut_click_cb (wxCommandEvent&);
#if defined (_DEBUG)
  virtual void button_report_click_cb (wxCommandEvent&);
#endif // _DEBUG
  virtual void choice_source_selected_cb (wxCommandEvent&);
  virtual void button_hardware_settings_click_cb (wxCommandEvent&);
  virtual void choice_format_selected_cb (wxCommandEvent&);
  virtual void choice_resolution_selected_cb (wxCommandEvent&);
  virtual void choice_framerate_selected_cb (wxCommandEvent&);
  virtual void button_reset_format_click_cb (wxCommandEvent&);
  virtual void togglebutton_save_toggled_cb (wxCommandEvent&);
  virtual void picker_directory_save_changed_cb (wxFileDirPickerEvent&);
  virtual void togglebutton_display_toggled_cb (wxCommandEvent&);
  virtual void togglebutton_fullscreen_toggled_cb (wxCommandEvent&);
  virtual void choice_adapter_selected_cb (wxCommandEvent&);
  virtual void choice_display_selected_cb (wxCommandEvent&);
  virtual void button_display_settings_click_cb (wxCommandEvent&);
  virtual void choice_resolution_2_selected_cb (wxCommandEvent&);
  virtual void button_about_click_cb (wxCommandEvent&);
  virtual void button_quit_click_cb (wxCommandEvent&);

  wxDECLARE_DYNAMIC_CLASS (OWN_TYPE_T);
  wxDECLARE_EVENT_TABLE ();

  Stream_CamSave_DirectShow_WxWidgetsIApplication_t* application_;
  bool                                               initializing_;
  bool                                               reset_; // direct3d device-
  bool                                               untoggling_;
};

template <>
class Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                       Stream_CamSave_MediaFoundation_Stream>
 : public dialog_main
 , public Common_UI_wxWidgets_ITopLevel_T<struct Common_UI_wxWidgets_State,
                                          struct Stream_CamSave_MediaFoundation_UI_CBData>
{
  typedef dialog_main inherited;
  //typedef Common_UI_wxWidgets_ITopLevel_T<struct Common_UI_wxWidgets_State,
  //                                        struct Stream_CamSave_MediaFoundation_UI_CBData> inherited2;

 public:
  // convenient types
  typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                             struct Stream_CamSave_MediaFoundation_UI_CBData> IAPPLICATION_T;

  Stream_CamSave_WxWidgetsDialog_T (wxWindow* = NULL); // parent window (if any)
  inline virtual ~Stream_CamSave_WxWidgetsDialog_T () {}

   // implement Common_UI_wxWidgets_ITopLevel
  inline virtual const IAPPLICATION_T* const getP () const { ACE_ASSERT (application_); return application_; }

 private:
  // convenient types
  typedef Stream_CamSave_WxWidgetsDialog_T<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                           Stream_CamSave_MediaFoundation_Stream> OWN_TYPE_T;

  // implement Common_UI_wxWidgets_ITopLevel
  virtual bool OnInit_2 (IAPPLICATION_T*);
  virtual void OnExit_2 ();

  // event handlers
  virtual void dialog_main_idle_cb (wxIdleEvent&);
  virtual void dialog_main_keydown_cb (wxKeyEvent&);

  // control handlers
  virtual void togglebutton_record_toggled_cb (wxCommandEvent&);
  virtual void button_snapshot_click_cb (wxCommandEvent&);
  virtual void button_cut_click_cb (wxCommandEvent&);
#if defined (_DEBUG)
  virtual void button_report_click_cb (wxCommandEvent&);
#endif // _DEBUG
  virtual void choice_source_selected_cb (wxCommandEvent&);
  virtual void button_hardware_settings_click_cb (wxCommandEvent&);
  virtual void choice_format_selected_cb (wxCommandEvent&);
  virtual void choice_resolution_selected_cb (wxCommandEvent&);
  virtual void choice_framerate_selected_cb (wxCommandEvent&);
  virtual void button_reset_format_click_cb (wxCommandEvent&);
  virtual void togglebutton_save_toggled_cb (wxCommandEvent&);
  virtual void picker_directory_save_changed_cb (wxFileDirPickerEvent&);
  virtual void togglebutton_display_toggled_cb (wxCommandEvent&);
  virtual void togglebutton_fullscreen_toggled_cb (wxCommandEvent&);
  virtual void choice_adapter_selected_cb (wxCommandEvent&);
  virtual void choice_display_selected_cb (wxCommandEvent&);
  virtual void button_display_settings_click_cb (wxCommandEvent&);
  virtual void choice_resolution_2_selected_cb (wxCommandEvent&);
  virtual void button_about_click_cb (wxCommandEvent&);
  virtual void button_quit_click_cb (wxCommandEvent&);

  wxDECLARE_DYNAMIC_CLASS (OWN_TYPE_T);
  wxDECLARE_EVENT_TABLE ();

  Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t* application_;
  bool                                                    initializing_;
  bool                                                    reset_; // direct3d device-
  bool                                                    untoggling_;
};
#endif // ACE_WIN32 || ACE_WIN64

// include template definition
#include "test_u_camsave_ui.inl"

#endif // __test_u_camsave_ui__
