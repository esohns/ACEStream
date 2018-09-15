///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Aug  8 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __TEST_U_CAMSAVE_UI_BASE_H__
#define __TEST_U_CAMSAVE_UI_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/tglbtn.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/bmpbuttn.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/filepicker.h>
#include <wx/panel.h>
#include <wx/statline.h>
#include <wx/gauge.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class dialog_main
///////////////////////////////////////////////////////////////////////////////
class dialog_main : public wxDialog 
{
	private:
	
	protected:
		wxToggleButton* togglebutton_record;
		wxBitmapButton* button_snapshot;
		wxBitmapButton* button_cut;
		wxBitmapButton* button_report;
		wxStaticText* label_frames;
		wxSpinCtrl* spincontrol_frames_captured;
		wxSpinCtrl* spincontrol_frames_dropped;
		wxStaticText* label_messages;
		wxSpinCtrl* spincontrol_messages_session;
		wxSpinCtrl* spincontrol_messages_data;
		wxStaticText* label_data;
		wxSpinCtrl* spincontrol_data;
		wxStaticText* label_buffer;
		wxSpinCtrl* spincontrol_buffer;
		wxChoice* choice_source;
		wxBitmapButton* button_hardware_settings;
		wxChoice* choice_format;
		wxChoice* choice_resolution;
		wxChoice* choice_framerate;
		wxBitmapButton* button_reset;
		wxToggleButton* togglebutton_save;
		wxTextCtrl* textcontrol_filename;
		wxDirPickerCtrl* directorypicker_save;
		wxToggleButton* togglebutton_fullscreen;
		wxBitmapButton* button_display_settings;
		wxPanel* panel_video;
		wxStaticLine* staticline_bottom;
		wxBitmapButton* button_about;
		wxBitmapButton* button_quit;
		wxGauge* gauge_progress;
		
		// Virtual event handlers, overide them in your derived class
		virtual void togglebutton_record_toggled_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void button_snapshot_click_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void button_cut_click_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void button_report_click_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void choice_source_selected_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void button_hardware_settings_click_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void choice_format_selected_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void choice_resolution_selected_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void choice_framerate_selected_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void button_reset_format_click_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void togglebutton_save_toggled_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void picker_directory_save_changed_cb( wxFileDirPickerEvent& event ) { event.Skip(); }
		virtual void togglebutton_fullscreen_toggled_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void button_display_settings_click_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void button_about_click_cb( wxCommandEvent& event ) { event.Skip(); }
		virtual void button_quit_click_cb( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		dialog_main( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("camsave"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 637,467 ), long style = wxCAPTION|wxDIALOG_NO_PARENT|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER ); 
		~dialog_main();
	
};

#endif //__TEST_U_CAMSAVE_UI_BASE_H__
