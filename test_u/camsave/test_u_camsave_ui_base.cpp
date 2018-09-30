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
	this->SetExtraStyle( this->GetExtraStyle() | wxWS_EX_PROCESS_IDLE );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );
	
	togglebutton_record = new wxToggleButton( this, wxID_ANY, _("record"), wxDefaultPosition, wxDefaultSize, 0 );
	togglebutton_record->Enable( false );
	togglebutton_record->SetToolTip( _("record") );
	
	bSizer5->Add( togglebutton_record, 0, wxEXPAND, 0 );
	
	button_snapshot = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_snapshot->SetBitmap( wxArtProvider::GetBitmap( wxART_NORMAL_FILE, wxART_BUTTON ) );
	button_snapshot->Enable( false );
	button_snapshot->SetToolTip( _("save snapshot") );
	
	bSizer5->Add( button_snapshot, 0, wxEXPAND, 0 );
	
	button_cut = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_cut->SetBitmap( wxArtProvider::GetBitmap( wxART_CUT, wxART_BUTTON ) );
	button_cut->Enable( false );
	button_cut->SetToolTip( _("cut save file") );
	
	bSizer5->Add( button_cut, 0, wxEXPAND, 0 );
	
	button_report = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_report->SetBitmap( wxArtProvider::GetBitmap( wxART_COPY, wxART_BUTTON ) );
	button_report->Enable( false );
	button_report->SetToolTip( _("report statistic information") );
	
	bSizer5->Add( button_report, 0, wxEXPAND, 0 );
	
	
	bSizer4->Add( bSizer5, 0, wxEXPAND, 0 );
	
	m_panel2 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN|wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	label_frames = new wxStaticText( m_panel2, wxID_ANY, _("frames"), wxDefaultPosition, wxDefaultSize, 0 );
	label_frames->Wrap( -1 );
	label_frames->Enable( false );
	
	fgSizer1->Add( label_frames, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );
	
	spincontrol_frames_captured = new wxSpinCtrl( m_panel2, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxSP_ARROW_KEYS, 0, 10, 0 );
	spincontrol_frames_captured->Enable( false );
	spincontrol_frames_captured->SetToolTip( _("captured frames") );
	
	bSizer6->Add( spincontrol_frames_captured, 1, wxEXPAND, 0 );
	
	spincontrol_frames_dropped = new wxSpinCtrl( m_panel2, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxSP_ARROW_KEYS, 0, 10, 0 );
	spincontrol_frames_dropped->Enable( false );
	spincontrol_frames_dropped->SetToolTip( _("dropped frames") );
	
	bSizer6->Add( spincontrol_frames_dropped, 1, wxEXPAND, 0 );
	
	
	fgSizer1->Add( bSizer6, 1, wxEXPAND, 0 );
	
	label_messages = new wxStaticText( m_panel2, wxID_ANY, _("messages"), wxDefaultPosition, wxDefaultSize, 0 );
	label_messages->Wrap( -1 );
	label_messages->Enable( false );
	
	fgSizer1->Add( label_messages, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );
	
	spincontrol_messages_session = new wxSpinCtrl( m_panel2, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxSP_ARROW_KEYS, 0, 10, 0 );
	spincontrol_messages_session->Enable( false );
	spincontrol_messages_session->SetToolTip( _("session messages") );
	
	bSizer7->Add( spincontrol_messages_session, 1, wxEXPAND, 0 );
	
	spincontrol_messages_data = new wxSpinCtrl( m_panel2, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxSP_ARROW_KEYS, 0, 10, 0 );
	spincontrol_messages_data->Enable( false );
	spincontrol_messages_data->SetToolTip( _("data messages") );
	
	bSizer7->Add( spincontrol_messages_data, 1, wxEXPAND, 0 );
	
	
	fgSizer1->Add( bSizer7, 1, wxEXPAND, 0 );
	
	label_data = new wxStaticText( m_panel2, wxID_ANY, _("data"), wxDefaultPosition, wxDefaultSize, 0 );
	label_data->Wrap( -1 );
	label_data->Enable( false );
	
	fgSizer1->Add( label_data, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	spincontrol_data = new wxSpinCtrl( m_panel2, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxSP_ARROW_KEYS, 0, 10, 0 );
	spincontrol_data->Enable( false );
	spincontrol_data->SetToolTip( _("data (bytes)") );
	
	fgSizer1->Add( spincontrol_data, 0, 0, 0 );
	
	label_buffer = new wxStaticText( m_panel2, wxID_ANY, _("buffer"), wxDefaultPosition, wxDefaultSize, 0 );
	label_buffer->Wrap( -1 );
	label_buffer->Enable( false );
	
	fgSizer1->Add( label_buffer, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	spincontrol_buffer = new wxSpinCtrl( m_panel2, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxSP_ARROW_KEYS, 0, 10, 0 );
	spincontrol_buffer->Enable( false );
	spincontrol_buffer->SetToolTip( _("buffer (bytes)") );
	
	fgSizer1->Add( spincontrol_buffer, 0, 0, 0 );
	
	
	m_panel2->SetSizer( fgSizer1 );
	m_panel2->Layout();
	fgSizer1->Fit( m_panel2 );
	bSizer4->Add( m_panel2, 1, wxEXPAND | wxALL, 0 );
	
	
	bSizer3->Add( bSizer4, 0, 0, 0 );
	
	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer3->Add( m_staticline2, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );
	
	m_panel3 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE|wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxHORIZONTAL );
	
	wxArrayString choice_sourceChoices;
	choice_source = new wxChoice( m_panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_sourceChoices, wxCB_SORT );
	choice_source->SetSelection( 0 );
	choice_source->Enable( false );
	choice_source->SetToolTip( _("capture device") );
	
	bSizer9->Add( choice_source, 1, wxEXPAND, 0 );
	
	button_hardware_settings = new wxBitmapButton( m_panel3, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_hardware_settings->SetBitmap( wxArtProvider::GetBitmap( wxART_LIST_VIEW, wxART_BUTTON ) );
	button_hardware_settings->Enable( false );
	button_hardware_settings->SetToolTip( _("hardware settings") );
	
	bSizer9->Add( button_hardware_settings, 0, 0, 0 );
	
	
	bSizer17->Add( bSizer9, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );
	
	wxArrayString choice_formatChoices;
	choice_format = new wxChoice( m_panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_formatChoices, wxCB_SORT );
	choice_format->SetSelection( 0 );
	choice_format->Enable( false );
	choice_format->SetToolTip( _("capture format") );
	
	bSizer10->Add( choice_format, 0, wxEXPAND, 0 );
	
	wxArrayString choice_resolutionChoices;
	choice_resolution = new wxChoice( m_panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_resolutionChoices, wxCB_SORT );
	choice_resolution->SetSelection( 0 );
	choice_resolution->Enable( false );
	choice_resolution->SetToolTip( _("capture resolution") );
	
	bSizer10->Add( choice_resolution, 0, wxEXPAND, 0 );
	
	wxArrayString choice_framerateChoices;
	choice_framerate = new wxChoice( m_panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_framerateChoices, wxCB_SORT );
	choice_framerate->SetSelection( 0 );
	choice_framerate->Enable( false );
	choice_framerate->SetToolTip( _("capture framerate") );
	
	bSizer10->Add( choice_framerate, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxHORIZONTAL );
	
	button_reset_format = new wxBitmapButton( m_panel3, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_reset_format->SetBitmap( wxArtProvider::GetBitmap( wxART_UNDO, wxART_BUTTON ) );
	button_reset_format->Enable( false );
	button_reset_format->SetToolTip( _("reset format") );
	
	bSizer11->Add( button_reset_format, 0, 0, 0 );
	
	
	bSizer10->Add( bSizer11, 0, wxEXPAND, 5 );
	
	
	bSizer17->Add( bSizer10, 0, wxEXPAND, 0 );
	
	
	m_panel3->SetSizer( bSizer17 );
	m_panel3->Layout();
	bSizer17->Fit( m_panel3 );
	bSizer8->Add( m_panel3, 0, wxEXPAND, 0 );
	
	m_panel4 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE|wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );
	
	togglebutton_save = new wxToggleButton( m_panel4, wxID_ANY, _("save"), wxDefaultPosition, wxDefaultSize, 0 );
	togglebutton_save->Enable( false );
	togglebutton_save->SetToolTip( _("save to file ?") );
	
	bSizer12->Add( togglebutton_save, 0, 0, 0 );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxHORIZONTAL );
	
	textcontrol_filename = new wxTextCtrl( m_panel4, wxID_ANY, _("output.avi"), wxDefaultPosition, wxDefaultSize, wxTE_LEFT|wxTE_NOHIDESEL|wxTE_WORDWRAP );
	textcontrol_filename->Enable( false );
	textcontrol_filename->SetToolTip( _("target file") );
	
	bSizer13->Add( textcontrol_filename, 1, wxEXPAND, 0 );
	
	directorypicker_save = new wxDirPickerCtrl( m_panel4, wxID_ANY, wxT("C:\\Windows\\Temp"), _("Select a folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE|wxDIRP_DIR_MUST_EXIST|wxDIRP_SMALL );
	directorypicker_save->Enable( false );
	directorypicker_save->SetToolTip( _("target directory") );
	
	bSizer13->Add( directorypicker_save, 0, wxEXPAND, 0 );
	
	
	bSizer12->Add( bSizer13, 0, 0, 0 );
	
	
	m_panel4->SetSizer( bSizer12 );
	m_panel4->Layout();
	bSizer12->Fit( m_panel4 );
	bSizer8->Add( m_panel4, 0, 0, 0 );
	
	m_panel5 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE|wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxHORIZONTAL );
	
	togglebutton_display = new wxToggleButton( m_panel5, wxID_ANY, _("display"), wxDefaultPosition, wxDefaultSize, 0 );
	togglebutton_display->Enable( false );
	togglebutton_display->SetToolTip( _("display stream ?") );
	
	bSizer20->Add( togglebutton_display, 0, 0, 0 );
	
	togglebutton_fullscreen = new wxToggleButton( m_panel5, wxID_ANY, _("fullscreen"), wxDefaultPosition, wxDefaultSize, 0 );
	togglebutton_fullscreen->Enable( false );
	togglebutton_fullscreen->SetToolTip( _("fullscreen ?") );
	
	bSizer20->Add( togglebutton_fullscreen, 0, 0, 0 );
	
	
	bSizer18->Add( bSizer20, 0, wxEXPAND, 0 );
	
	wxArrayString choice_adapterChoices;
	choice_adapter = new wxChoice( m_panel5, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_adapterChoices, 0 );
	choice_adapter->SetSelection( 0 );
	choice_adapter->Enable( false );
	choice_adapter->SetToolTip( _("adapter") );
	
	bSizer18->Add( choice_adapter, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );
	
	wxArrayString choice_displayChoices;
	choice_display = new wxChoice( m_panel5, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_displayChoices, 0 );
	choice_display->SetSelection( 0 );
	choice_display->Enable( false );
	choice_display->SetToolTip( _("display") );
	
	bSizer14->Add( choice_display, 1, 0, 0 );
	
	button_display_settings = new wxBitmapButton( m_panel5, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_display_settings->SetBitmap( wxArtProvider::GetBitmap( wxART_LIST_VIEW, wxART_BUTTON ) );
	button_display_settings->Enable( false );
	button_display_settings->SetToolTip( _("display settings") );
	
	bSizer14->Add( button_display_settings, 0, 0, 5 );
	
	
	bSizer18->Add( bSizer14, 1, wxEXPAND, 5 );
	
	wxArrayString choice_resolution_2Choices;
	choice_resolution_2 = new wxChoice( m_panel5, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_resolution_2Choices, 0 );
	choice_resolution_2->SetSelection( 0 );
	choice_resolution_2->Enable( false );
	choice_resolution_2->SetToolTip( _("display resolution") );
	
	bSizer18->Add( choice_resolution_2, 0, wxEXPAND, 0 );
	
	
	m_panel5->SetSizer( bSizer18 );
	m_panel5->Layout();
	bSizer18->Fit( m_panel5 );
	bSizer8->Add( m_panel5, 0, wxEXPAND, 0 );
	
	
	bSizer3->Add( bSizer8, 0, wxEXPAND, 0 );
	
	
	bSizer2->Add( bSizer3, 0, wxEXPAND, 0 );
	
	panel_video = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( 320,240 ), wxTAB_TRAVERSAL );
	bSizer2->Add( panel_video, 1, wxEXPAND, 0 );
	
	
	bSizer1->Add( bSizer2, 1, wxEXPAND, 0 );
	
	staticline_bottom = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( staticline_bottom, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxHORIZONTAL );
	
	button_about = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_about->SetBitmap( wxArtProvider::GetBitmap( wxART_TIP, wxART_BUTTON ) );
	button_about->SetToolTip( _("about") );
	
	bSizer15->Add( button_about, 0, wxEXPAND, 0 );
	
	button_quit = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	
	button_quit->SetBitmap( wxArtProvider::GetBitmap( wxART_QUIT, wxART_BUTTON ) );
	button_quit->SetToolTip( _("quit") );
	
	bSizer15->Add( button_quit, 0, wxEXPAND, 0 );
	
	
	bSizer1->Add( bSizer15, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxVERTICAL );
	
	gauge_progress = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL|wxGA_SMOOTH );
	gauge_progress->SetValue( 0 ); 
	gauge_progress->Enable( false );
	
	bSizer16->Add( gauge_progress, 1, 0, 0 );
	
	
	bSizer1->Add( bSizer16, 0, wxEXPAND, 0 );
	
	
	this->SetSizer( bSizer1 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_IDLE, wxIdleEventHandler( dialog_main::dialog_main_idle_cb ) );
	this->Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( dialog_main::dialog_main_keydown_cb ) );
	togglebutton_record->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_record_toggled_cb ), NULL, this );
	button_snapshot->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_snapshot_click_cb ), NULL, this );
	button_cut->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_cut_click_cb ), NULL, this );
	button_report->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_report_click_cb ), NULL, this );
	choice_source->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_source_selected_cb ), NULL, this );
	button_hardware_settings->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_hardware_settings_click_cb ), NULL, this );
	choice_format->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_format_selected_cb ), NULL, this );
	choice_resolution->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_resolution_selected_cb ), NULL, this );
	choice_framerate->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_framerate_selected_cb ), NULL, this );
	button_reset_format->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_reset_format_click_cb ), NULL, this );
	togglebutton_save->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_save_toggled_cb ), NULL, this );
	directorypicker_save->Connect( wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler( dialog_main::picker_directory_save_changed_cb ), NULL, this );
	togglebutton_display->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_display_toggled_cb ), NULL, this );
	togglebutton_fullscreen->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_fullscreen_toggled_cb ), NULL, this );
	choice_adapter->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_adapter_selected_cb ), NULL, this );
	choice_display->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_display_selected_cb ), NULL, this );
	button_display_settings->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_display_settings_click_cb ), NULL, this );
	choice_resolution_2->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_resolution_2_selected_cb ), NULL, this );
	button_about->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_about_click_cb ), NULL, this );
	button_quit->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_quit_click_cb ), NULL, this );
}

dialog_main::~dialog_main()
{
	// Disconnect Events
	this->Disconnect( wxEVT_IDLE, wxIdleEventHandler( dialog_main::dialog_main_idle_cb ) );
	this->Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler( dialog_main::dialog_main_keydown_cb ) );
	togglebutton_record->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_record_toggled_cb ), NULL, this );
	button_snapshot->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_snapshot_click_cb ), NULL, this );
	button_cut->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_cut_click_cb ), NULL, this );
	button_report->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_report_click_cb ), NULL, this );
	choice_source->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_source_selected_cb ), NULL, this );
	button_hardware_settings->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_hardware_settings_click_cb ), NULL, this );
	choice_format->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_format_selected_cb ), NULL, this );
	choice_resolution->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_resolution_selected_cb ), NULL, this );
	choice_framerate->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_framerate_selected_cb ), NULL, this );
	button_reset_format->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_reset_format_click_cb ), NULL, this );
	togglebutton_save->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_save_toggled_cb ), NULL, this );
	directorypicker_save->Disconnect( wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler( dialog_main::picker_directory_save_changed_cb ), NULL, this );
	togglebutton_display->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_display_toggled_cb ), NULL, this );
	togglebutton_fullscreen->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( dialog_main::togglebutton_fullscreen_toggled_cb ), NULL, this );
	choice_adapter->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_adapter_selected_cb ), NULL, this );
	choice_display->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_display_selected_cb ), NULL, this );
	button_display_settings->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_display_settings_click_cb ), NULL, this );
	choice_resolution_2->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( dialog_main::choice_resolution_2_selected_cb ), NULL, this );
	button_about->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_about_click_cb ), NULL, this );
	button_quit->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dialog_main::button_quit_click_cb ), NULL, this );
	
}
