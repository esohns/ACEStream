///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Aug  8 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "test_u_camsave_ui_base.h"

///////////////////////////////////////////////////////////////////////////

dialog_main::dialog_main( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxVERTICAL );
	
	togglebutton_record = new wxToggleButton( this, wxID_ANY, wxT("record"), wxDefaultPosition, wxDefaultSize, 0 );
	togglebutton_record->Enable( false );
	togglebutton_record->SetToolTip( wxT("record") );
	
	bSizer21->Add( togglebutton_record, 0, wxEXPAND, 0 );
	
	button_snapshot = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_snapshot->SetBitmap( wxArtProvider::GetBitmap( wxART_NORMAL_FILE, wxART_BUTTON ) );
	button_snapshot->Enable( false );
	button_snapshot->SetToolTip( wxT("save snapshot") );
	
	bSizer21->Add( button_snapshot, 0, wxEXPAND, 0 );
	
	button_cut = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_cut->SetBitmap( wxArtProvider::GetBitmap( wxART_CUT, wxART_BUTTON ) );
	button_cut->Enable( false );
	button_cut->SetToolTip( wxT("cut save file") );
	
	bSizer21->Add( button_cut, 0, wxEXPAND, 0 );
	
	button_report = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_report->SetBitmap( wxArtProvider::GetBitmap( wxART_COPY, wxART_BUTTON ) );
	button_report->Enable( false );
	button_report->SetToolTip( wxT("report statistic information") );
	
	bSizer21->Add( button_report, 0, wxEXPAND, 0 );
	
	
	bSizer20->Add( bSizer21, 0, wxEXPAND, 0 );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	label_frames = new wxStaticText( this, wxID_ANY, wxT("frames"), wxDefaultPosition, wxDefaultSize, 0 );
	label_frames->Wrap( -1 );
	label_frames->Enable( false );
	
	fgSizer1->Add( label_frames, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer221;
	bSizer221 = new wxBoxSizer( wxVERTICAL );
	
	spincontrol_frames_captured = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxSP_ARROW_KEYS, 0, 10, 0 );
	spincontrol_frames_captured->Enable( false );
	spincontrol_frames_captured->SetToolTip( wxT("captured frames") );
	
	bSizer221->Add( spincontrol_frames_captured, 1, wxEXPAND, 0 );
	
	spincontrol_frames_dropped = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxSP_ARROW_KEYS, 0, 10, 0 );
	spincontrol_frames_dropped->Enable( false );
	spincontrol_frames_dropped->SetToolTip( wxT("dropped frames") );
	
	bSizer221->Add( spincontrol_frames_dropped, 1, wxEXPAND, 0 );
	
	
	fgSizer1->Add( bSizer221, 1, wxEXPAND, 0 );
	
	label_messages = new wxStaticText( this, wxID_ANY, wxT("messages"), wxDefaultPosition, wxDefaultSize, 0 );
	label_messages->Wrap( -1 );
	label_messages->Enable( false );
	
	fgSizer1->Add( label_messages, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer231;
	bSizer231 = new wxBoxSizer( wxVERTICAL );
	
	spincontrol_messages_session = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxSP_ARROW_KEYS, 0, 10, 0 );
	spincontrol_messages_session->Enable( false );
	spincontrol_messages_session->SetToolTip( wxT("session messages") );
	
	bSizer231->Add( spincontrol_messages_session, 1, wxEXPAND, 0 );
	
	spincontrol_messages_data = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxSP_ARROW_KEYS, 0, 10, 0 );
	spincontrol_messages_data->Enable( false );
	spincontrol_messages_data->SetToolTip( wxT("data messages") );
	
	bSizer231->Add( spincontrol_messages_data, 1, wxEXPAND, 0 );
	
	
	fgSizer1->Add( bSizer231, 1, wxEXPAND, 0 );
	
	label_data = new wxStaticText( this, wxID_ANY, wxT("data"), wxDefaultPosition, wxDefaultSize, 0 );
	label_data->Wrap( -1 );
	label_data->Enable( false );
	
	fgSizer1->Add( label_data, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	spincontrol_data = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxSP_ARROW_KEYS, 0, 10, 0 );
	spincontrol_data->Enable( false );
	spincontrol_data->SetToolTip( wxT("data (bytes)") );
	
	fgSizer1->Add( spincontrol_data, 0, 0, 0 );
	
	label_buffer = new wxStaticText( this, wxID_ANY, wxT("buffer"), wxDefaultPosition, wxDefaultSize, 0 );
	label_buffer->Wrap( -1 );
	label_buffer->Enable( false );
	
	fgSizer1->Add( label_buffer, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	spincontrol_buffer = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxSP_ARROW_KEYS, 0, 10, 0 );
	spincontrol_buffer->Enable( false );
	spincontrol_buffer->SetToolTip( wxT("buffer (bytes)") );
	
	fgSizer1->Add( spincontrol_buffer, 0, 0, 0 );
	
	
	bSizer20->Add( fgSizer1, 1, wxEXPAND, 0 );
	
	
	bSizer19->Add( bSizer20, 1, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxHORIZONTAL );
	
	wxArrayString choice_sourceChoices;
	choice_source = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_sourceChoices, wxCB_SORT );
	choice_source->SetSelection( 0 );
	choice_source->Enable( false );
	choice_source->SetToolTip( wxT("capture device") );
	
	bSizer25->Add( choice_source, 1, wxEXPAND, 0 );
	
	button_hardware_settings = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_hardware_settings->SetBitmap( wxArtProvider::GetBitmap( wxART_LIST_VIEW, wxART_BUTTON ) );
	button_hardware_settings->Enable( false );
	button_hardware_settings->SetToolTip( wxT("hardware settings") );
	
	bSizer25->Add( button_hardware_settings, 0, 0, 0 );
	
	
	bSizer24->Add( bSizer25, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer26;
	bSizer26 = new wxBoxSizer( wxVERTICAL );
	
	wxArrayString choice_formatChoices;
	choice_format = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_formatChoices, wxCB_SORT );
	choice_format->SetSelection( 0 );
	choice_format->Enable( false );
	choice_format->SetToolTip( wxT("capture format") );
	
	bSizer26->Add( choice_format, 0, wxEXPAND, 0 );
	
	wxArrayString choice_resolutionChoices;
	choice_resolution = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_resolutionChoices, wxCB_SORT );
	choice_resolution->SetSelection( 0 );
	choice_resolution->Enable( false );
	choice_resolution->SetToolTip( wxT("capture resolution") );
	
	bSizer26->Add( choice_resolution, 0, wxEXPAND, 0 );
	
	wxArrayString choice_framerateChoices;
	choice_framerate = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_framerateChoices, wxCB_SORT );
	choice_framerate->SetSelection( 0 );
	choice_framerate->Enable( false );
	choice_framerate->SetToolTip( wxT("capture framerate") );
	
	bSizer26->Add( choice_framerate, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer( wxHORIZONTAL );
	
	button_reset = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_reset->SetBitmap( wxArtProvider::GetBitmap( wxART_UNDO, wxART_BUTTON ) );
	button_reset->Enable( false );
	button_reset->SetToolTip( wxT("reset format") );
	
	bSizer27->Add( button_reset, 0, 0, 0 );
	
	
	bSizer26->Add( bSizer27, 0, wxEXPAND, 5 );
	
	
	bSizer24->Add( bSizer26, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer( wxVERTICAL );
	
	togglebutton_save = new wxToggleButton( this, wxID_ANY, wxT("save"), wxDefaultPosition, wxDefaultSize, 0 );
	togglebutton_save->Enable( false );
	togglebutton_save->SetToolTip( wxT("save to file ?") );
	
	bSizer28->Add( togglebutton_save, 0, 0, 0 );
	
	wxBoxSizer* bSizer29;
	bSizer29 = new wxBoxSizer( wxHORIZONTAL );
	
	textcontrol_filename = new wxTextCtrl( this, wxID_ANY, wxT("output.avi"), wxDefaultPosition, wxDefaultSize, 0 );
	textcontrol_filename->Enable( false );
	textcontrol_filename->SetToolTip( wxT("target file") );
	
	bSizer29->Add( textcontrol_filename, 1, 0, 0 );
	
	directorypicker_save = new wxDirPickerCtrl( this, wxID_ANY, wxT("C:\\Windows\\Temp"), wxT("Select a folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE|wxDIRP_DIR_MUST_EXIST|wxDIRP_SMALL );
	directorypicker_save->Enable( false );
	directorypicker_save->SetToolTip( wxT("target directory") );
	
	bSizer29->Add( directorypicker_save, 0, 0, 0 );
	
	
	bSizer28->Add( bSizer29, 0, wxEXPAND, 0 );
	
	
	bSizer24->Add( bSizer28, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxHORIZONTAL );
	
	togglebutton_fullscreen = new wxToggleButton( this, wxID_ANY, wxT("fullscreen"), wxDefaultPosition, wxDefaultSize, 0 );
	togglebutton_fullscreen->Enable( false );
	togglebutton_fullscreen->SetToolTip( wxT("fullscreen ?") );
	
	bSizer30->Add( togglebutton_fullscreen, 0, 0, 0 );
	
	button_display_settings = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_display_settings->SetBitmap( wxArtProvider::GetBitmap( wxART_LIST_VIEW, wxART_BUTTON ) );
	button_display_settings->Enable( false );
	button_display_settings->SetToolTip( wxT("display settings") );
	
	bSizer30->Add( button_display_settings, 0, 0, 5 );
	
	
	bSizer24->Add( bSizer30, 0, wxEXPAND, 0 );
	
	
	bSizer19->Add( bSizer24, 0, wxEXPAND, 0 );
	
	
	bSizer18->Add( bSizer19, 0, wxEXPAND, 0 );
	
	panel_video = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( 320,240 ), wxTAB_TRAVERSAL );
	bSizer18->Add( panel_video, 1, wxEXPAND, 0 );
	
	
	bSizer17->Add( bSizer18, 0, wxEXPAND, 0 );
	
	staticline_bottom = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer17->Add( staticline_bottom, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxHORIZONTAL );
	
	button_about = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_about->SetBitmap( wxArtProvider::GetBitmap( wxART_TIP, wxART_BUTTON ) );
	button_about->SetToolTip( wxT("about") );
	
	bSizer31->Add( button_about, 0, wxEXPAND, 0 );
	
	button_quit = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_quit->SetBitmap( wxArtProvider::GetBitmap( wxART_QUIT, wxART_BUTTON ) );
	button_quit->SetToolTip( wxT("quit") );
	
	bSizer31->Add( button_quit, 0, wxEXPAND, 0 );
	
	
	bSizer17->Add( bSizer31, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxVERTICAL );
	
	gauge_progress = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL|wxGA_SMOOTH );
	gauge_progress->SetValue( 0 ); 
	gauge_progress->Enable( false );
	
	bSizer32->Add( gauge_progress, 1, 0, 0 );
	
	
	bSizer17->Add( bSizer32, 0, wxEXPAND, 0 );
	
	
	this->SetSizer( bSizer17 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	togglebutton_record->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_record_toggled_cb ), NULL, this );
	button_snapshot->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_snapshot_click_cb ), NULL, this );
	button_cut->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_cut_click_cb ), NULL, this );
	button_report->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_report_click_cb ), NULL, this );
	choice_source->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_source_selected_cb ), NULL, this );
	button_hardware_settings->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_hardware_settings_click_cb ), NULL, this );
	choice_format->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_format_selected_cb ), NULL, this );
	choice_resolution->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_resolution_selected_cb ), NULL, this );
	choice_framerate->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_framerate_selected_cb ), NULL, this );
	button_reset->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_reset_format_click_cb ), NULL, this );
	togglebutton_save->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_save_toggled_cb ), NULL, this );
	directorypicker_save->Connect( wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler( dialog_main::picker_directory_save_changed_cb ), NULL, this );
	togglebutton_fullscreen->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_fullscreen_toggled_cb ), NULL, this );
	button_display_settings->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_display_settings_click_cb ), NULL, this );
	button_about->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_about_click_cb ), NULL, this );
	button_quit->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_quit_click_cb ), NULL, this );
}

dialog_main::~dialog_main()
{
	// Disconnect Events
	togglebutton_record->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_record_toggled_cb ), NULL, this );
	button_snapshot->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_snapshot_click_cb ), NULL, this );
	button_cut->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_cut_click_cb ), NULL, this );
	button_report->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_report_click_cb ), NULL, this );
	choice_source->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_source_selected_cb ), NULL, this );
	button_hardware_settings->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_hardware_settings_click_cb ), NULL, this );
	choice_format->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_format_selected_cb ), NULL, this );
	choice_resolution->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_resolution_selected_cb ), NULL, this );
	choice_framerate->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_framerate_selected_cb ), NULL, this );
	button_reset->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_reset_format_click_cb ), NULL, this );
	togglebutton_save->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_save_toggled_cb ), NULL, this );
	directorypicker_save->Disconnect( wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler( dialog_main::picker_directory_save_changed_cb ), NULL, this );
	togglebutton_fullscreen->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_fullscreen_toggled_cb ), NULL, this );
	button_display_settings->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_display_settings_click_cb ), NULL, this );
	button_about->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_about_click_cb ), NULL, this );
	button_quit->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_quit_click_cb ), NULL, this );
	
}
