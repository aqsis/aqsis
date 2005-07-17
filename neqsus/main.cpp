/////////////////////////////////////////////////////////////////////////////
// Name:        main.cpp
// Purpose:     
// Author:      Leon Tony Atkinson
// Modified by: 
// Created:     03/11/05 18:25:31
// RCS-ID:      
// Copyright:   Copyright 2005, Leon Tony Atkinson
// Licence:     GPL (General Public License)
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "main.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "options.h"
#include "about.h"
////@end includes

#include "main.h"

////@begin XPM images
////@end XPM images

/*!
 * FrameMain type definition
 */

IMPLEMENT_CLASS( FrameMain, wxFrame )

/*!
 * FrameMain event table definition
 */

BEGIN_EVENT_TABLE( FrameMain, wxFrame )

////@begin FrameMain event table entries
#if defined(__WXMSW__)
    EVT_MENU( wxID_EXIT, FrameMain::OnExitClick )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( wxID_UNDO, FrameMain::OnUndoUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( wxID_REDO, FrameMain::OnRedoUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( wxID_CUT, FrameMain::OnCutUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( wxID_COPY, FrameMain::OnCopyUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( wxID_PASTE, FrameMain::OnPasteUpdate )
#endif

#if defined(__WXGTK__)
    EVT_MENU( wxID_PREFERENCES, FrameMain::OnPreferencesClick )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_SHOWSTATUSBAR, FrameMain::OnShowstatusbarUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXMAC__)
    EVT_MENU( wxID_PREFERENCES, FrameMain::OnPreferencesClick )
#endif

#if defined(__WXMAC__)
    EVT_UPDATE_UI( ID_SHOWSTATUSBAR, FrameMain::OnShowstatusbarUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_SHOWGENERAL, FrameMain::OnShowgeneralUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_SHOWPREVIEW, FrameMain::OnShowpreviewUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_SHOWLOG, FrameMain::OnShowlogUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_MENU( wxID_ABOUT, FrameMain::OnAboutClick )
#endif

#if defined(__WXMAC__)
    EVT_MENU( wxID_ABOUT, FrameMain::OnAboutClick )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_CONFIGANIMATION, FrameMain::OnConfiganimationUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_PARAMANIMATION, FrameMain::OnParamanimationUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_CONFIGSHUTTER, FrameMain::OnConfigshutterUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_PARAMSHUTTER, FrameMain::OnParamshutterUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_CONFIGQUALITY, FrameMain::OnConfigqualityUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_PARAMQUALITY, FrameMain::OnParamqualityUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_CONFIGEXPOSURE, FrameMain::OnConfigexposureUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_PARAMEXPOSURE, FrameMain::OnParamexposureUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_CONFIGCLIPPING, FrameMain::OnConfigclippingUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_PARAMCLIPPING, FrameMain::OnParamclippingUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_CONFIGDOF, FrameMain::OnConfigdofUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_PARAMDOF, FrameMain::OnParamdofUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_CONFIGSHADOWMAP, FrameMain::OnConfigshadowmapUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_PARAMSHADOWMAP, FrameMain::OnParamshadowmapUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_CONFIGTWEAK, FrameMain::OnConfigtweakUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_PARAMTWEAK, FrameMain::OnParamtweakUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_CONFIGTWEAKX, FrameMain::OnConfigtweakxUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_PARAMTWEAKX, FrameMain::OnParamtweakxUpdate )
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    EVT_UPDATE_UI( ID_STATIC_PREVIEW, FrameMain::OnStaticPreviewUpdate )
#endif

////@end FrameMain event table entries

END_EVENT_TABLE()

/*!
 * FrameMain constructors
 */

FrameMain::FrameMain( )
{
}

FrameMain::FrameMain( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create( parent, id, caption, pos, size, style );
}

/*!
 * MyFrame creator
 */

bool FrameMain::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin FrameMain member initialisation
////@end FrameMain member initialisation

////@begin FrameMain creation
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end FrameMain creation
    return TRUE;
}

/*!
 * Control creation for MyFrame
 */

void FrameMain::CreateControls()
{    
////@begin FrameMain content construction
    FrameMain* itemFrame1 = this;

    wxMenuBar* menuBar = new wxMenuBar;
#if defined(__WXMSW__) || defined(__WXGTK__)
    wxMenu* itemMenu3 = new wxMenu;
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu3->Append(wxID_OPEN, _("Open"), _("Opens a file"), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    wxMenu* itemMenu5 = new wxMenu;
#if defined(__WXMAC__)
    itemMenu5->Append(ID_MRU1, _T(""), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    itemMenu5->Append(ID_MRU2, _T(""), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    itemMenu5->Append(ID_MRU3, _T(""), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    itemMenu5->Append(ID_MRU4, _T(""), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    itemMenu5->Append(ID_MRU5, _T(""), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    itemMenu5->Append(ID_MRU6, _T(""), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    itemMenu5->Append(ID_MRU7, _T(""), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    itemMenu5->Append(ID_MRU8, _T(""), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    itemMenu5->Append(ID_MRU9, _T(""), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    itemMenu5->Append(ID_MRU10, _T(""), _T(""), wxITEM_NORMAL);
#endif
    itemMenu3->Append(ID_MRU, _("Open Recent"), itemMenu5);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu3->AppendSeparator();
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu3->Append(ID_MRU1, _("1 vase.rib"), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu3->Append(ID_MRU2, _("2"), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu3->Append(ID_MRU3, _("3"), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu3->Append(ID_MRU4, _("4"), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    wxMenu* itemMenu21 = new wxMenu;
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu21->Append(ID_MRU5, _("5"), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu21->Append(ID_MRU6, _("6"), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu21->Append(ID_MRU7, _("7"), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu21->Append(ID_MRU8, _("8"), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu21->Append(ID_MRU9, _("9"), _T(""), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu21->Append(ID_MRU10, _("10"), _T(""), wxITEM_NORMAL);
#endif
    itemMenu3->Append(ID_MRU, _("Recent Files"), itemMenu21);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu3->AppendSeparator();
#endif
#if defined(__WXMSW__)
    itemMenu3->Append(wxID_EXIT, _("Exit"), _("Closes the application"), wxITEM_NORMAL);
#endif
#if defined(__WXGTK__)
    itemMenu3->Append(wxID_EXIT, _("Quit"), _("Closes the application"), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    itemMenu3->Append(wxID_EXIT, _("Quit Neqsus"), _("Closes the application"), wxITEM_NORMAL);
#endif
    menuBar->Append(itemMenu3, _("File"));
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    wxMenu* itemMenu32 = new wxMenu;
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu32->Append(wxID_UNDO, _("Undo"), _("Undoes last command"), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu32->Append(wxID_REDO, _("Redo"), _("Redoes last command"), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu32->AppendSeparator();
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu32->Append(wxID_CUT, _("Cut"), _("Deletes data"), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu32->Append(wxID_COPY, _("Copy"), _("Copies data to the clipboard"), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu32->Append(wxID_PASTE, _("Paste"), _("Pastes data from the clipboard"), wxITEM_NORMAL);
#endif
#if defined(__WXGTK__)
    itemMenu32->AppendSeparator();
#endif
#if defined(__WXGTK__)
    itemMenu32->Append(wxID_PREFERENCES, _("Preferences"), _("Configures the application"), wxITEM_NORMAL);
#endif
    menuBar->Append(itemMenu32, _("Edit"));
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    wxMenu* itemMenu41 = new wxMenu;
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu41->Append(ID_SHOWSTATUSBAR, _("Status Bar"), _("If enabled, will display the status bar"), wxITEM_CHECK);
    itemMenu41->Check(ID_SHOWSTATUSBAR, TRUE);
#endif
    menuBar->Append(itemMenu41, _("View"));
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    wxMenu* itemMenu43 = new wxMenu;
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu43->Append(ID_ENABLEAQSL, _("Compile Shaders"), _("If enabled, will automatically compile your shaders"), wxITEM_CHECK);
    itemMenu43->Check(ID_ENABLEAQSL, TRUE);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu43->Append(ID_ENABLETEQSER, _("Optimise Textures"), _("If enabled, will automatically optimise your textures"), wxITEM_CHECK);
    itemMenu43->Check(ID_ENABLETEQSER, TRUE);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu43->Append(ID_ENABLEBEEP, _("Enable Sound"), _("If enabled, will beep at the end of each render"), wxITEM_CHECK);
    itemMenu43->Check(ID_ENABLEBEEP, TRUE);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu43->Append(ID_ENABLECGKIT, _("Enable CGKit"), _("If enabled, will enhance the functionality of this application"), wxITEM_CHECK);
    itemMenu43->Check(ID_ENABLECGKIT, TRUE);
#endif
#if defined(__WXMSW__)
    itemMenu43->AppendSeparator();
#endif
#if defined(__WXMSW__) || defined(__WXMAC__)
    itemMenu43->Append(wxID_PREFERENCES, _("Options"), _("Configures the application"), wxITEM_NORMAL);
#endif
    menuBar->Append(itemMenu43, _("Tools"));
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    wxMenu* itemMenu50 = new wxMenu;
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu50->Append(wxMINIMIZE, _("Minimise"), _("Minimises this window"), wxITEM_NORMAL);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu50->Append(wxMAXIMIZE, _("Maximise"), _("Maximises this window"), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    itemMenu50->Append(wxMAXIMIZE, _("Zoom"), _("Maximises this window"), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    itemMenu50->AppendSeparator();
#endif
#if defined(__WXMAC__)
    itemMenu50->Append(ID_SHOWSTATUSBAR, _("Show Status Bar"), _("If enabled, will display the status bar"), wxITEM_CHECK);
    itemMenu50->Check(ID_SHOWSTATUSBAR, TRUE);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu50->AppendSeparator();
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu50->Append(ID_SHOWGENERAL, _("General Window"), _("If enabled, will display the general window"), wxITEM_RADIO);
    itemMenu50->Check(ID_SHOWGENERAL, TRUE);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu50->Append(ID_SHOWPREVIEW, _("Preview Window"), _("If enabled, will display the preview window"), wxITEM_RADIO);
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu50->Append(ID_SHOWLOG, _("Log Window"), _("If enabled, will display the log window"), wxITEM_RADIO);
#endif
    menuBar->Append(itemMenu50, _("Window"));
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    wxMenu* itemMenu60 = new wxMenu;
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu60->Append(wxID_HELP, _("Contents"), _("If enabled, will display the help system"), wxITEM_CHECK);
#endif
#if defined(__WXMAC__)
    itemMenu60->Append(wxID_HELP, _("Neqsus Help"), _("If enabled, will display the help system"), wxITEM_CHECK);
#endif
#if defined(__WXMSW__)
    itemMenu60->AppendSeparator();
#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    itemMenu60->Append(wxID_ABOUT, _("About"), _("About this application"), wxITEM_NORMAL);
#endif
#if defined(__WXMAC__)
    itemMenu60->Append(wxID_ABOUT, _("About Neqsus"), _("About this application"), wxITEM_NORMAL);
#endif
    menuBar->Append(itemMenu60, _("Help"));
#endif
    itemFrame1->SetMenuBar(menuBar);

    wxBoxSizer* itemBoxSizer66 = new wxBoxSizer(wxHORIZONTAL);
    itemFrame1->SetSizer(itemBoxSizer66);

    wxBoxSizer* itemBoxSizer67 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer66->Add(itemBoxSizer67, 0, wxGROW|wxALL, 0);

    wxNotebook* itemNotebook68 = new wxNotebook( itemFrame1, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_TOP );

    wxScrolledWindow* itemScrolledWindow69 = new wxScrolledWindow( itemNotebook68, ID_NOTEBOOK_GENERAL, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
    itemScrolledWindow69->SetScrollbars(1, 1, 0, 0);
    wxBoxSizer* itemBoxSizer70 = new wxBoxSizer(wxVERTICAL);
    itemScrolledWindow69->SetSizer(itemBoxSizer70);

    wxStaticBox* itemStaticBoxSizer71Static = new wxStaticBox(itemScrolledWindow69, ID_PARAMIMAGE, _("Image Settings"));
    wxStaticBoxSizer* itemStaticBoxSizer71 = new wxStaticBoxSizer(itemStaticBoxSizer71Static, wxVERTICAL);
    itemBoxSizer70->Add(itemStaticBoxSizer71, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer72 = new wxFlexGridSizer(3, 3, 0, 0);
    itemStaticBoxSizer71->Add(itemFlexGridSizer72, 0, wxGROW|wxALL, 0);
    wxStaticText* itemStaticText73 = new wxStaticText( itemScrolledWindow69, wxID_STATIC, _("Width:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer72->Add(itemStaticText73, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl74 = new wxSpinCtrl( itemScrolledWindow69, ID_WIDTH, _("640"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 1280, 640 );
    itemFlexGridSizer72->Add(itemSpinCtrl74, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText75 = new wxStaticText( itemScrolledWindow69, wxID_STATIC, _("pixels"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer72->Add(itemStaticText75, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText76 = new wxStaticText( itemScrolledWindow69, wxID_STATIC, _("Height:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer72->Add(itemStaticText76, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl77 = new wxSpinCtrl( itemScrolledWindow69, ID_HEIGHT, _("480"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 1280, 480 );
    itemFlexGridSizer72->Add(itemSpinCtrl77, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText78 = new wxStaticText( itemScrolledWindow69, wxID_STATIC, _("pixels"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer72->Add(itemStaticText78, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText79 = new wxStaticText( itemScrolledWindow69, wxID_STATIC, _("PAR:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText79->SetForegroundColour(wxColour(255, 0, 0));
    itemFlexGridSizer72->Add(itemStaticText79, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxString itemComboBox80Strings[] = {
        _("1"),
        _("0.9"),
        _("0.8"),
        _("0.7"),
        _("0.6"),
        _("0.5"),
        _("0.4"),
        _("0.3"),
        _("0.2"),
        _("0.1")
    };
    wxComboBox* itemComboBox80 = new wxComboBox( itemScrolledWindow69, ID_PIXELASPECTRATIO, _("1"), wxDefaultPosition, wxDefaultSize, 10, itemComboBox80Strings, wxCB_DROPDOWN );
    itemComboBox80->SetStringSelection(_("1"));
    itemFlexGridSizer72->Add(itemComboBox80, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText81 = new wxStaticText( itemScrolledWindow69, wxID_STATIC, _("float"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText81->SetForegroundColour(wxColour(255, 0, 0));
    itemFlexGridSizer72->Add(itemStaticText81, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxCheckBox* itemCheckBox82 = new wxCheckBox( itemScrolledWindow69, ID_ASPECTRATIO, _("Maintain Aspect Ratio"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBox82->SetValue(TRUE);
    itemStaticBoxSizer71->Add(itemCheckBox82, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer83 = new wxFlexGridSizer(1, 2, 0, 0);
    itemBoxSizer70->Add(itemFlexGridSizer83, 0, wxGROW|wxALL, 0);
#if defined(__WXOS2__)
    wxBitmap itemBitmapButton84Bitmap(itemFrame1->GetBitmapResource(wxT(".neqsus/ui/expand.png")));
    wxBitmapButton* itemBitmapButton84 = new wxBitmapButton( itemScrolledWindow69, ID_CONFIGANIMATION, itemBitmapButton84Bitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemFlexGridSizer83->Add(itemBitmapButton84, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);
#endif

    wxToggleButton* itemToggleButton85 = new wxToggleButton( itemScrolledWindow69, ID_CONFIGANIMATION, _("+"), wxDefaultPosition, wxSize(14, 14), 0 );
    itemToggleButton85->SetValue(FALSE);
    itemFlexGridSizer83->Add(itemToggleButton85, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxCheckBox* itemCheckBox86 = new wxCheckBox( itemScrolledWindow69, ID_ENABLEANIMATION, _("Enable Animation"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBox86->SetValue(FALSE);
    itemFlexGridSizer83->Add(itemCheckBox86, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxPanel* itemPanel87 = new wxPanel( itemScrolledWindow69, ID_PARAMANIMATION, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer70->Add(itemPanel87, 0, wxGROW|wxALL, 5);
    wxStaticBox* itemStaticBoxSizer88Static = new wxStaticBox(itemPanel87, wxID_ANY, _("Animation Settings"));
    wxStaticBoxSizer* itemStaticBoxSizer88 = new wxStaticBoxSizer(itemStaticBoxSizer88Static, wxVERTICAL);
    itemPanel87->SetSizer(itemStaticBoxSizer88);

    wxFlexGridSizer* itemFlexGridSizer89 = new wxFlexGridSizer(3, 2, 0, 0);
    itemStaticBoxSizer88->Add(itemFlexGridSizer89, 0, wxGROW|wxALL, 0);
    wxStaticText* itemStaticText90 = new wxStaticText( itemPanel87, wxID_STATIC, _("Start Frame:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer89->Add(itemStaticText90, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl91 = new wxSpinCtrl( itemPanel87, ID_STARTFRAME, _("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 999, 1 );
    itemFlexGridSizer89->Add(itemSpinCtrl91, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText92 = new wxStaticText( itemPanel87, wxID_STATIC, _("End Frame:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer89->Add(itemStaticText92, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl93 = new wxSpinCtrl( itemPanel87, ID_ENDFRAME, _("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 999, 1 );
    itemFlexGridSizer89->Add(itemSpinCtrl93, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

#if defined(__WXOS2__)
    wxStaticText* itemStaticText94 = new wxStaticText( itemPanel87, wxID_STATIC, _("Exclusions:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer89->Add(itemStaticText94, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);
#endif

#if defined(__WXOS2__)
    wxTextCtrl* itemTextCtrl95 = new wxTextCtrl( itemPanel87, ID_EXCLUDEFRAME, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer89->Add(itemTextCtrl95, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif

    wxFlexGridSizer* itemFlexGridSizer96 = new wxFlexGridSizer(1, 2, 0, 0);
    itemStaticBoxSizer88->Add(itemFlexGridSizer96, 0, wxGROW|wxALL, 0);
#if defined(__WXOS2__)
    wxBitmap itemBitmapButton97Bitmap(itemFrame1->GetBitmapResource(wxT(".neqsus/ui/expand.png")));
    wxBitmapButton* itemBitmapButton97 = new wxBitmapButton( itemPanel87, ID_CONFIGSHUTTER, itemBitmapButton97Bitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemFlexGridSizer96->Add(itemBitmapButton97, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);
#endif

    wxToggleButton* itemToggleButton98 = new wxToggleButton( itemPanel87, ID_CONFIGSHUTTER, _("+"), wxDefaultPosition, wxSize(14, 14), 0 );
    itemToggleButton98->SetValue(FALSE);
    itemFlexGridSizer96->Add(itemToggleButton98, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxCheckBox* itemCheckBox99 = new wxCheckBox( itemPanel87, ID_ENABLESHUTTER, _("Use Motion Blur"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBox99->SetValue(TRUE);
    itemFlexGridSizer96->Add(itemCheckBox99, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxPanel* itemPanel100 = new wxPanel( itemPanel87, ID_PARAMSHUTTER, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer88->Add(itemPanel100, 0, wxGROW|wxALL, 5);
    wxStaticBox* itemStaticBoxSizer101Static = new wxStaticBox(itemPanel100, wxID_ANY, _("Motion Blur Settings"));
    wxStaticBoxSizer* itemStaticBoxSizer101 = new wxStaticBoxSizer(itemStaticBoxSizer101Static, wxVERTICAL);
    itemPanel100->SetSizer(itemStaticBoxSizer101);

    wxFlexGridSizer* itemFlexGridSizer102 = new wxFlexGridSizer(2, 3, 0, 0);
    itemStaticBoxSizer101->Add(itemFlexGridSizer102, 0, wxGROW|wxALL, 0);
    wxStaticText* itemStaticText103 = new wxStaticText( itemPanel100, wxID_STATIC, _("Shutter Open:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer102->Add(itemStaticText103, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl104 = new wxSpinCtrl( itemPanel100, ID_SHUTTEROPEN, _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, 0 );
    itemFlexGridSizer102->Add(itemSpinCtrl104, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText105 = new wxStaticText( itemPanel100, wxID_STATIC, _("???"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText105->SetForegroundColour(wxColour(255, 0, 0));
    itemFlexGridSizer102->Add(itemStaticText105, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText106 = new wxStaticText( itemPanel100, wxID_STATIC, _("Shutter Close:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer102->Add(itemStaticText106, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl107 = new wxSpinCtrl( itemPanel100, ID_SHUTTERCLOSE, _("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, 1 );
    itemFlexGridSizer102->Add(itemSpinCtrl107, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText108 = new wxStaticText( itemPanel100, wxID_STATIC, _("???"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText108->SetForegroundColour(wxColour(255, 0, 0));
    itemFlexGridSizer102->Add(itemStaticText108, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticLine* itemStaticLine109 = new wxStaticLine( itemScrolledWindow69, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer70->Add(itemStaticLine109, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer110 = new wxFlexGridSizer(1, 2, 0, 0);
    itemBoxSizer70->Add(itemFlexGridSizer110, 0, wxGROW|wxALL, 0);
#if defined(__WXOS2__)
    wxBitmap itemBitmapButton111Bitmap(itemFrame1->GetBitmapResource(wxT(".neqsus/ui/expand.png")));
    wxBitmapButton* itemBitmapButton111 = new wxBitmapButton( itemScrolledWindow69, ID_CONFIGQUALITY, itemBitmapButton111Bitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemFlexGridSizer110->Add(itemBitmapButton111, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);
#endif

    wxToggleButton* itemToggleButton112 = new wxToggleButton( itemScrolledWindow69, ID_CONFIGQUALITY, _("+"), wxDefaultPosition, wxSize(14, 14), 0 );
    itemToggleButton112->SetValue(FALSE);
    itemFlexGridSizer110->Add(itemToggleButton112, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxCheckBox* itemCheckBox113 = new wxCheckBox( itemScrolledWindow69, ID_ENABLEQUALITY, _("Configure Quality"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBox113->SetValue(FALSE);
    itemFlexGridSizer110->Add(itemCheckBox113, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxPanel* itemPanel114 = new wxPanel( itemScrolledWindow69, ID_PARAMQUALITY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer70->Add(itemPanel114, 0, wxGROW|wxALL, 5);
    wxStaticBox* itemStaticBoxSizer115Static = new wxStaticBox(itemPanel114, wxID_ANY, _("Quality Settings"));
    wxStaticBoxSizer* itemStaticBoxSizer115 = new wxStaticBoxSizer(itemStaticBoxSizer115Static, wxVERTICAL);
    itemPanel114->SetSizer(itemStaticBoxSizer115);

    wxFlexGridSizer* itemFlexGridSizer116 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer115->Add(itemFlexGridSizer116, 0, wxGROW|wxALL, 0);
    wxStaticText* itemStaticText117 = new wxStaticText( itemPanel114, wxID_STATIC, _("Anti-Aliasing:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer116->Add(itemStaticText117, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxString itemComboBox118Strings[] = {
        _("16x16"),
        _("8x8"),
        _("4x4"),
        _("2x2"),
        _("1x1")
    };
    wxComboBox* itemComboBox118 = new wxComboBox( itemPanel114, ID_PIXELSAMPLE, _("4x4"), wxDefaultPosition, wxDefaultSize, 5, itemComboBox118Strings, wxCB_DROPDOWN );
    itemComboBox118->SetStringSelection(_("4x4"));
    itemFlexGridSizer116->Add(itemComboBox118, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText119 = new wxStaticText( itemPanel114, wxID_STATIC, _("Shading:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer116->Add(itemStaticText119, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSlider* itemSlider120 = new wxSlider( itemPanel114, ID_SHADINGRATE, 1, 0, 10, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_AUTOTICKS|wxSL_LABELS );
    itemFlexGridSizer116->Add(itemSlider120, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer121 = new wxFlexGridSizer(1, 2, 0, 0);
    itemBoxSizer70->Add(itemFlexGridSizer121, 0, wxGROW|wxALL, 0);
#if defined(__WXOS2__)
    wxBitmap itemBitmapButton122Bitmap(itemFrame1->GetBitmapResource(wxT(".neqsus/ui/expand.png")));
    wxBitmapButton* itemBitmapButton122 = new wxBitmapButton( itemScrolledWindow69, ID_CONFIGEXPOSURE, itemBitmapButton122Bitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemFlexGridSizer121->Add(itemBitmapButton122, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);
#endif

    wxToggleButton* itemToggleButton123 = new wxToggleButton( itemScrolledWindow69, ID_CONFIGEXPOSURE, _("+"), wxDefaultPosition, wxSize(14, 14), 0 );
    itemToggleButton123->SetValue(FALSE);
    itemFlexGridSizer121->Add(itemToggleButton123, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxCheckBox* itemCheckBox124 = new wxCheckBox( itemScrolledWindow69, ID_ENABLEEXPOSURE, _("Configure Exposure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBox124->SetValue(FALSE);
    itemFlexGridSizer121->Add(itemCheckBox124, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxPanel* itemPanel125 = new wxPanel( itemScrolledWindow69, ID_PARAMEXPOSURE, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer70->Add(itemPanel125, 0, wxGROW|wxALL, 5);
    wxStaticBox* itemStaticBoxSizer126Static = new wxStaticBox(itemPanel125, wxID_ANY, _("Exposure Settings"));
    wxStaticBoxSizer* itemStaticBoxSizer126 = new wxStaticBoxSizer(itemStaticBoxSizer126Static, wxVERTICAL);
    itemPanel125->SetSizer(itemStaticBoxSizer126);

    wxFlexGridSizer* itemFlexGridSizer127 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer126->Add(itemFlexGridSizer127, 0, wxGROW|wxALL, 0);
    wxStaticText* itemStaticText128 = new wxStaticText( itemPanel125, wxID_STATIC, _("Gain:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer127->Add(itemStaticText128, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSlider* itemSlider129 = new wxSlider( itemPanel125, ID_GAIN, 1, 0, 10, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_AUTOTICKS|wxSL_LABELS );
    itemFlexGridSizer127->Add(itemSlider129, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText130 = new wxStaticText( itemPanel125, wxID_STATIC, _("Gamma:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer127->Add(itemStaticText130, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSlider* itemSlider131 = new wxSlider( itemPanel125, ID_GAMMA, 2, 0, 10, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_AUTOTICKS|wxSL_LABELS );
    itemFlexGridSizer127->Add(itemSlider131, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer132 = new wxFlexGridSizer(1, 2, 0, 0);
    itemBoxSizer70->Add(itemFlexGridSizer132, 0, wxGROW|wxALL, 0);
#if defined(__WXOS2__)
    wxBitmap itemBitmapButton133Bitmap(itemFrame1->GetBitmapResource(wxT(".neqsus/ui/expand.png")));
    wxBitmapButton* itemBitmapButton133 = new wxBitmapButton( itemScrolledWindow69, ID_CONFIGCLIPPING, itemBitmapButton133Bitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemFlexGridSizer132->Add(itemBitmapButton133, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);
#endif

    wxToggleButton* itemToggleButton134 = new wxToggleButton( itemScrolledWindow69, ID_CONFIGCLIPPING, _("+"), wxDefaultPosition, wxSize(14, 14), 0 );
    itemToggleButton134->SetValue(FALSE);
    itemFlexGridSizer132->Add(itemToggleButton134, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxCheckBox* itemCheckBox135 = new wxCheckBox( itemScrolledWindow69, ID_ENABLECLIPPING, _("Configure Clipping"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBox135->SetValue(FALSE);
    itemFlexGridSizer132->Add(itemCheckBox135, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxPanel* itemPanel136 = new wxPanel( itemScrolledWindow69, ID_PARAMCLIPPING, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer70->Add(itemPanel136, 0, wxGROW|wxALL, 5);
    wxStaticBox* itemStaticBoxSizer137Static = new wxStaticBox(itemPanel136, wxID_ANY, _("Clipping Settings"));
    wxStaticBoxSizer* itemStaticBoxSizer137 = new wxStaticBoxSizer(itemStaticBoxSizer137Static, wxVERTICAL);
    itemPanel136->SetSizer(itemStaticBoxSizer137);

    wxFlexGridSizer* itemFlexGridSizer138 = new wxFlexGridSizer(2, 3, 0, 0);
    itemStaticBoxSizer137->Add(itemFlexGridSizer138, 0, wxGROW|wxALL, 0);
    wxStaticText* itemStaticText139 = new wxStaticText( itemPanel136, wxID_STATIC, _("Near Plane:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(itemStaticText139, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl140 = new wxSpinCtrl( itemPanel136, ID_HITHER, _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, 0 );
    itemFlexGridSizer138->Add(itemSpinCtrl140, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText141 = new wxStaticText( itemPanel136, wxID_STATIC, _("???"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText141->SetForegroundColour(wxColour(255, 0, 0));
    itemFlexGridSizer138->Add(itemStaticText141, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText142 = new wxStaticText( itemPanel136, wxID_STATIC, _("Far Plane:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(itemStaticText142, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl143 = new wxSpinCtrl( itemPanel136, ID_YON, _("999"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, 999 );
    itemFlexGridSizer138->Add(itemSpinCtrl143, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText144 = new wxStaticText( itemPanel136, wxID_STATIC, _("???"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText144->SetForegroundColour(wxColour(255, 0, 0));
    itemFlexGridSizer138->Add(itemStaticText144, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticLine* itemStaticLine145 = new wxStaticLine( itemScrolledWindow69, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer70->Add(itemStaticLine145, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer146 = new wxFlexGridSizer(1, 2, 0, 0);
    itemBoxSizer70->Add(itemFlexGridSizer146, 0, wxGROW|wxALL, 0);
#if defined(__WXOS2__)
    wxBitmap itemBitmapButton147Bitmap(itemFrame1->GetBitmapResource(wxT(".neqsus/ui/expand.png")));
    wxBitmapButton* itemBitmapButton147 = new wxBitmapButton( itemScrolledWindow69, ID_CONFIGDOF, itemBitmapButton147Bitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemFlexGridSizer146->Add(itemBitmapButton147, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);
#endif

    wxToggleButton* itemToggleButton148 = new wxToggleButton( itemScrolledWindow69, ID_CONFIGDOF, _("+"), wxDefaultPosition, wxSize(14, 14), 0 );
    itemToggleButton148->SetValue(FALSE);
    itemFlexGridSizer146->Add(itemToggleButton148, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxCheckBox* itemCheckBox149 = new wxCheckBox( itemScrolledWindow69, ID_ENABLEDOF, _("Use Depth of Field"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBox149->SetValue(TRUE);
    itemFlexGridSizer146->Add(itemCheckBox149, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxPanel* itemPanel150 = new wxPanel( itemScrolledWindow69, ID_PARAMDOF, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer70->Add(itemPanel150, 0, wxGROW|wxALL, 5);
    wxStaticBox* itemStaticBoxSizer151Static = new wxStaticBox(itemPanel150, wxID_ANY, _("Depth of Field Settings"));
    wxStaticBoxSizer* itemStaticBoxSizer151 = new wxStaticBoxSizer(itemStaticBoxSizer151Static, wxVERTICAL);
    itemPanel150->SetSizer(itemStaticBoxSizer151);

    wxFlexGridSizer* itemFlexGridSizer152 = new wxFlexGridSizer(3, 3, 0, 0);
    itemStaticBoxSizer151->Add(itemFlexGridSizer152, 0, wxGROW|wxALL, 0);
    wxStaticText* itemStaticText153 = new wxStaticText( itemPanel150, wxID_STATIC, _("F-Stop:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer152->Add(itemStaticText153, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl154 = new wxSpinCtrl( itemPanel150, ID_FSTOP, _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, 0 );
    itemFlexGridSizer152->Add(itemSpinCtrl154, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText155 = new wxStaticText( itemPanel150, wxID_STATIC, _("???"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText155->SetForegroundColour(wxColour(255, 0, 0));
    itemFlexGridSizer152->Add(itemStaticText155, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText156 = new wxStaticText( itemPanel150, wxID_STATIC, _("Focal Length:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer152->Add(itemStaticText156, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl157 = new wxSpinCtrl( itemPanel150, ID_FOCALLENGTH, _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, 0 );
    itemFlexGridSizer152->Add(itemSpinCtrl157, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText158 = new wxStaticText( itemPanel150, wxID_STATIC, _("millimeters"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText158->SetForegroundColour(wxColour(255, 0, 0));
    itemFlexGridSizer152->Add(itemStaticText158, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText159 = new wxStaticText( itemPanel150, wxID_STATIC, _("Focal Distance:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer152->Add(itemStaticText159, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl160 = new wxSpinCtrl( itemPanel150, ID_FOCALDISTANCE, _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, 0 );
    itemFlexGridSizer152->Add(itemSpinCtrl160, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText161 = new wxStaticText( itemPanel150, wxID_STATIC, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText161->SetForegroundColour(wxColour(255, 0, 0));
    itemFlexGridSizer152->Add(itemStaticText161, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxFlexGridSizer* itemFlexGridSizer162 = new wxFlexGridSizer(1, 2, 0, 0);
    itemBoxSizer70->Add(itemFlexGridSizer162, 0, wxGROW|wxALL, 0);
#if defined(__WXOS2__)
    wxBitmap itemBitmapButton163Bitmap(itemFrame1->GetBitmapResource(wxT(".neqsus/ui/expand.png")));
    wxBitmapButton* itemBitmapButton163 = new wxBitmapButton( itemScrolledWindow69, ID_CONFIGSHADOWMAP, itemBitmapButton163Bitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemFlexGridSizer162->Add(itemBitmapButton163, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);
#endif

    wxToggleButton* itemToggleButton164 = new wxToggleButton( itemScrolledWindow69, ID_CONFIGSHADOWMAP, _("+"), wxDefaultPosition, wxSize(14, 14), 0 );
    itemToggleButton164->SetValue(FALSE);
    itemFlexGridSizer162->Add(itemToggleButton164, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxCheckBox* itemCheckBox165 = new wxCheckBox( itemScrolledWindow69, ID_ENABLESHADOWMAP, _("Use Shadows"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBox165->SetValue(TRUE);
    itemFlexGridSizer162->Add(itemCheckBox165, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxPanel* itemPanel166 = new wxPanel( itemScrolledWindow69, ID_PARAMSHADOWMAP, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer70->Add(itemPanel166, 0, wxGROW|wxALL, 5);
    wxStaticBox* itemStaticBoxSizer167Static = new wxStaticBox(itemPanel166, wxID_ANY, _("Quality Settings"));
    wxStaticBoxSizer* itemStaticBoxSizer167 = new wxStaticBoxSizer(itemStaticBoxSizer167Static, wxVERTICAL);
    itemPanel166->SetSizer(itemStaticBoxSizer167);

    wxFlexGridSizer* itemFlexGridSizer168 = new wxFlexGridSizer(1, 2, 0, 0);
    itemStaticBoxSizer167->Add(itemFlexGridSizer168, 0, wxGROW|wxALL, 0);
    wxStaticText* itemStaticText169 = new wxStaticText( itemPanel166, wxID_STATIC, _("Samples:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer168->Add(itemStaticText169, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxString itemComboBox170Strings[] = {
        _("16x16"),
        _("8x8"),
        _("4x4"),
        _("2x2"),
        _("1x1")
    };
    wxComboBox* itemComboBox170 = new wxComboBox( itemPanel166, ID_SHADOWMAPSAMPLES, _("4x4"), wxDefaultPosition, wxDefaultSize, 5, itemComboBox170Strings, wxCB_DROPDOWN );
    itemComboBox170->SetStringSelection(_("4x4"));
    itemFlexGridSizer168->Add(itemComboBox170, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine171 = new wxStaticLine( itemScrolledWindow69, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer70->Add(itemStaticLine171, 0, wxGROW|wxALL, 5);

    wxRadioButton* itemRadioButton172 = new wxRadioButton( itemScrolledWindow69, ID_ENABLEFILE, _("Render to a File"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    itemRadioButton172->SetValue(TRUE);
    itemBoxSizer70->Add(itemRadioButton172, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer173 = new wxFlexGridSizer(1, 1, 0, 0);
    itemBoxSizer70->Add(itemFlexGridSizer173, 0, wxGROW|wxLEFT, 15);
    wxCheckBox* itemCheckBox174 = new wxCheckBox( itemScrolledWindow69, ID_ENABLEFRAMEBUFFER, _("Display Preview"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBox174->SetValue(FALSE);
    itemFlexGridSizer173->Add(itemCheckBox174, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxRadioButton* itemRadioButton175 = new wxRadioButton( itemScrolledWindow69, ID_ENABLEFRAMEBUFFER, _("Render to the Screen"), wxDefaultPosition, wxDefaultSize, 0 );
    itemRadioButton175->SetValue(FALSE);
    itemBoxSizer70->Add(itemRadioButton175, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer70->Add(5, 5, 0, wxGROW|wxALL, 5);

    wxStaticLine* itemStaticLine177 = new wxStaticLine( itemScrolledWindow69, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer70->Add(itemStaticLine177, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer178 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer70->Add(itemBoxSizer178, 0, wxALIGN_RIGHT|wxALL, 0);
#if defined(__WXMSW__) || defined(__WXGTK__)
    wxButton* itemButton179 = new wxButton( itemScrolledWindow69, ID_RENDER, _("&Render"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton179->SetDefault();
    itemBoxSizer178->Add(itemButton179, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);
#endif

    wxButton* itemButton180 = new wxButton( itemScrolledWindow69, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer178->Add(itemButton180, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

#if defined(__WXMAC__)
    wxButton* itemButton181 = new wxButton( itemScrolledWindow69, ID_RENDER, _("&Render"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton181->SetDefault();
    itemBoxSizer178->Add(itemButton181, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);
#endif

    itemNotebook68->AddPage(itemScrolledWindow69, _("General"));

    wxScrolledWindow* itemScrolledWindow182 = new wxScrolledWindow( itemNotebook68, ID_NOTEBOOK_PREVIEW, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
    itemScrolledWindow182->SetScrollbars(1, 1, 0, 0);
    wxBoxSizer* itemBoxSizer183 = new wxBoxSizer(wxVERTICAL);
    itemScrolledWindow182->SetSizer(itemBoxSizer183);

    wxStaticBox* itemStaticBoxSizer184Static = new wxStaticBox(itemScrolledWindow182, ID_PARAMLIBRARY, _("Library"));
    wxStaticBoxSizer* itemStaticBoxSizer184 = new wxStaticBoxSizer(itemStaticBoxSizer184Static, wxVERTICAL);
    itemBoxSizer183->Add(itemStaticBoxSizer184, 0, wxGROW|wxALL, 5);
    wxTreeCtrl* itemTreeCtrl185 = new wxTreeCtrl( itemScrolledWindow182, ID_LIBRARY, wxDefaultPosition, wxDefaultSize, wxTR_SINGLE );
    itemStaticBoxSizer184->Add(itemTreeCtrl185, 0, wxGROW|wxALL, 5);

    wxStaticLine* itemStaticLine186 = new wxStaticLine( itemScrolledWindow182, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer183->Add(itemStaticLine186, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer187 = new wxFlexGridSizer(1, 2, 0, 0);
    itemBoxSizer183->Add(itemFlexGridSizer187, 0, wxGROW|wxALL, 0);
#if defined(__WXOS2__)
    wxBitmap itemBitmapButton188Bitmap(itemFrame1->GetBitmapResource(wxT(".neqsus/ui/expand.png")));
    wxBitmapButton* itemBitmapButton188 = new wxBitmapButton( itemScrolledWindow182, ID_CONFIGTWEAK, itemBitmapButton188Bitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemFlexGridSizer187->Add(itemBitmapButton188, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);
#endif

    wxToggleButton* itemToggleButton189 = new wxToggleButton( itemScrolledWindow182, ID_CONFIGTWEAK, _("+"), wxDefaultPosition, wxSize(14, 14), 0 );
    itemToggleButton189->SetValue(FALSE);
    itemFlexGridSizer187->Add(itemToggleButton189, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxCheckBox* itemCheckBox190 = new wxCheckBox( itemScrolledWindow182, ID_ENABLETWEAK, _("Tweak Selection"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBox190->SetValue(FALSE);
    itemFlexGridSizer187->Add(itemCheckBox190, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxPanel* itemPanel191 = new wxPanel( itemScrolledWindow182, ID_PARAMTWEAK, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer183->Add(itemPanel191, 0, wxGROW|wxALL, 5);
    wxStaticBox* itemStaticBoxSizer192Static = new wxStaticBox(itemPanel191, wxID_ANY, _("Tweak Settings"));
    wxStaticBoxSizer* itemStaticBoxSizer192 = new wxStaticBoxSizer(itemStaticBoxSizer192Static, wxVERTICAL);
    itemPanel191->SetSizer(itemStaticBoxSizer192);

    wxFlexGridSizer* itemFlexGridSizer193 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer192->Add(itemFlexGridSizer193, 0, wxGROW|wxALL, 0);
    wxStaticText* itemStaticText194 = new wxStaticText( itemPanel191, wxID_STATIC, _("Opacity:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer193->Add(itemStaticText194, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSlider* itemSlider195 = new wxSlider( itemPanel191, ID_OPACITY, 10, 0, 10, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_AUTOTICKS|wxSL_LABELS );
    itemFlexGridSizer193->Add(itemSlider195, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText196 = new wxStaticText( itemPanel191, wxID_STATIC, _("Focus:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer193->Add(itemStaticText196, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSlider* itemSlider197 = new wxSlider( itemPanel191, ID_FOCUS, 10, 0, 20, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_AUTOTICKS|wxSL_LABELS );
    itemFlexGridSizer193->Add(itemSlider197, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer198 = new wxFlexGridSizer(1, 2, 0, 0);
    itemBoxSizer183->Add(itemFlexGridSizer198, 0, wxGROW|wxALL, 0);
#if defined(__WXOS2__)
    wxBitmap itemBitmapButton199Bitmap(itemFrame1->GetBitmapResource(wxT(".neqsus/ui/expand.png")));
    wxBitmapButton* itemBitmapButton199 = new wxBitmapButton( itemScrolledWindow182, ID_CONFIGTWEAKX, itemBitmapButton199Bitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemFlexGridSizer198->Add(itemBitmapButton199, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);
#endif

    wxToggleButton* itemToggleButton200 = new wxToggleButton( itemScrolledWindow182, ID_CONFIGTWEAKX, _("+"), wxDefaultPosition, wxSize(14, 14), 0 );
    itemToggleButton200->SetValue(FALSE);
    itemFlexGridSizer198->Add(itemToggleButton200, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxCheckBox* itemCheckBox201 = new wxCheckBox( itemScrolledWindow182, ID_ENABLETWEAKX, _("Additional Tweaking"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBox201->SetValue(FALSE);
    itemFlexGridSizer198->Add(itemCheckBox201, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxPanel* itemPanel202 = new wxPanel( itemScrolledWindow182, ID_PARAMTWEAKX, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer183->Add(itemPanel202, 0, wxGROW|wxALL, 5);
    wxStaticBox* itemStaticBoxSizer203Static = new wxStaticBox(itemPanel202, wxID_ANY, _("Additional Tweak Settings"));
    wxStaticBoxSizer* itemStaticBoxSizer203 = new wxStaticBoxSizer(itemStaticBoxSizer203Static, wxVERTICAL);
    itemPanel202->SetSizer(itemStaticBoxSizer203);

    wxFlexGridSizer* itemFlexGridSizer204 = new wxFlexGridSizer(3, 2, 0, 0);
    itemStaticBoxSizer203->Add(itemFlexGridSizer204, 0, wxGROW|wxALL, 0);
    wxStaticText* itemStaticText205 = new wxStaticText( itemPanel202, wxID_STATIC, _("Dummy 1:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer204->Add(itemStaticText205, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl206 = new wxSpinCtrl( itemPanel202, ID_DUMMY1, _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
    itemFlexGridSizer204->Add(itemSpinCtrl206, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText207 = new wxStaticText( itemPanel202, wxID_STATIC, _("Dummy 2:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer204->Add(itemStaticText207, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl208 = new wxSpinCtrl( itemPanel202, ID_DUMMY2, _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
    itemFlexGridSizer204->Add(itemSpinCtrl208, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText209 = new wxStaticText( itemPanel202, wxID_STATIC, _("Dummy 3:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer204->Add(itemStaticText209, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSpinCtrl* itemSpinCtrl210 = new wxSpinCtrl( itemPanel202, ID_DUMMY3, _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
    itemFlexGridSizer204->Add(itemSpinCtrl210, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer183->Add(5, 5, 0, wxGROW|wxALL, 5);

    wxStaticLine* itemStaticLine212 = new wxStaticLine( itemScrolledWindow182, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer183->Add(itemStaticLine212, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer213 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer183->Add(itemBoxSizer213, 0, wxALIGN_RIGHT|wxALL, 0);
#if defined(__WXMSW__) || defined(__WXGTK__)
    wxButton* itemButton214 = new wxButton( itemScrolledWindow182, ID_PREVIEW, _("&Preview"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton214->SetDefault();
    itemBoxSizer213->Add(itemButton214, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);
#endif

    wxButton* itemButton215 = new wxButton( itemScrolledWindow182, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer213->Add(itemButton215, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

#if defined(__WXMAC__)
    wxButton* itemButton216 = new wxButton( itemScrolledWindow182, ID_PREVIEW, _("&Preview"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton216->SetDefault();
    itemBoxSizer213->Add(itemButton216, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);
#endif

    itemNotebook68->AddPage(itemScrolledWindow182, _("Preview"));

    wxScrolledWindow* itemScrolledWindow217 = new wxScrolledWindow( itemNotebook68, ID_NOTEBOOK_LOG, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
    itemScrolledWindow217->SetScrollbars(1, 1, 0, 0);
    wxBoxSizer* itemBoxSizer218 = new wxBoxSizer(wxVERTICAL);
    itemScrolledWindow217->SetSizer(itemBoxSizer218);

    wxBoxSizer* itemBoxSizer219 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer218->Add(itemBoxSizer219, 0, wxGROW|wxALL, 0);
    wxTextCtrl* itemTextCtrl220 = new wxTextCtrl( itemScrolledWindow217, ID_LOG, _("c:\\program files\\aqsis\\examples\\..\\bin\\aqsis -fb -progress vase.rib"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
    itemBoxSizer219->Add(itemTextCtrl220, 0, wxGROW|wxALL, 5);

    itemNotebook68->AddPage(itemScrolledWindow217, _("Log"));

    itemBoxSizer67->Add(itemNotebook68, 0, wxGROW|wxALL, 0);

    wxBitmap itemStaticBitmap221Bitmap(itemFrame1->GetBitmapResource(wxT(".neqsus/temp/static_preview.png")));
    wxStaticBitmap* itemStaticBitmap221 = new wxStaticBitmap( itemFrame1, ID_STATIC_PREVIEW, itemStaticBitmap221Bitmap, wxDefaultPosition, wxSize(300, 300), 0 );
    itemBoxSizer67->Add(itemStaticBitmap221, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStatusBar* itemStatusBar222 = new wxStatusBar( itemFrame1, ID_STATUSBAR, wxST_SIZEGRIP|wxNO_BORDER );
    itemStatusBar222->SetFieldsCount(2);
    itemStatusBar222->SetStatusText(_("http://www.aqsis.org"), 0);
    itemStatusBar222->SetStatusText(_("Time Remaining: 00:00:00 (Progress: 0%)"), 1);
    itemFrame1->SetStatusBar(itemStatusBar222);

////@end FrameMain content construction
}

/*!
 * Should we show tooltips?
 */

bool FrameMain::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap FrameMain::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin FrameMain bitmap retrieval
    if (name == wxT(".neqsus/ui/expand.png"))
    {
        wxBitmap bitmap(_T(".neqsus/ui/expand.png"), wxBITMAP_TYPE_PNG);
        return bitmap;
    }
    else if (name == wxT(".neqsus/temp/static_preview.png"))
    {
        wxBitmap bitmap(_T(".neqsus/temp/static_preview.png"), wxBITMAP_TYPE_PNG);
        return bitmap;
    }
    return wxNullBitmap;
////@end FrameMain bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon FrameMain::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin FrameMain icon retrieval
    return wxNullIcon;
////@end FrameMain icon retrieval
}
/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT
 */

void FrameMain::OnExitClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT in Frame_Main.
    // Before editing this code, remove the block markers.
    Destroy();
////@end wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT in Frame_Main. 
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_ABOUT
 */

void FrameMain::OnAboutClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_MENU_SELECTED event handler for wxID_ABOUT in FrameMain.
    // Before editing this code, remove the block markers.
    DialogAbout* window = new DialogAbout(NULL, ID_DIALOG_ABOUT, _("About Neqsus"));
    int returnValue = window->ShowModal();
    window->Destroy();
////@end wxEVT_COMMAND_MENU_SELECTED event handler for wxID_ABOUT in FrameMain. 
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_PREFERENCES
 */

void FrameMain::OnPreferencesClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_MENU_SELECTED event handler for wxID_PREFERENCES in FrameMain.
    // Before editing this code, remove the block markers.
    DialogOptions* window = new DialogOptions(NULL, ID_DIALOG_OPTIONS, _("Options"));
    int returnValue = window->ShowModal();
    window->Destroy();
////@end wxEVT_COMMAND_MENU_SELECTED event handler for wxID_PREFERENCES in FrameMain. 
}


/*!
 * wxEVT_UPDATE_UI event handler for ID_CONFIGANIMATION
 */

void FrameMain::OnConfiganimationUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_CONFIGANIMATION in FrameMain.
    // Before editing this code, remove the block markers.
    wxCheckBox* checkBox = (wxCheckBox*) FindWindow(ID_ENABLEANIMATION);
    event.Enable(checkBox->GetValue());
//@end wxEVT_UPDATE_UI event handler for ID_CONFIGANIMATION in FrameMain. 
}


/*!
 * wxEVT_UPDATE_UI event handler for ID_PARAMANIMATION
 */

void FrameMain::OnParamanimationUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_PARAMANIMATION in FrameMain.
    // Before editing this code, remove the block markers.
    wxToggleButton* toggleButton = (wxToggleButton*) FindWindow(ID_CONFIGANIMATION);
    FindWindow(ID_PARAMANIMATION)->SetSize(-1, 600);
    FindWindow(ID_PARAMANIMATION)->Show(toggleButton->GetValue());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_PARAMANIMATION in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_CONFIGSHUTTER
 */

void FrameMain::OnConfigshutterUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_CONFIGSHUTTER in FrameMain.
    // Before editing this code, remove the block markers.
    wxCheckBox* checkBox = (wxCheckBox*) FindWindow(ID_ENABLESHUTTER);
    event.Enable(checkBox->GetValue());
//@end wxEVT_UPDATE_UI event handler for ID_CONFIGSHUTTER in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_PARAMSHUTTER
 */

void FrameMain::OnParamshutterUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_PARAMSHUTTER in FrameMain.
    // Before editing this code, remove the block markers.
    wxToggleButton* toggleButton = (wxToggleButton*) FindWindow(ID_CONFIGSHUTTER);
    FindWindow(ID_PARAMSHUTTER)->Show(toggleButton->GetValue());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_PARAMSHUTTER in FrameMain. 
}


/*!
 * wxEVT_UPDATE_UI event handler for ID_CONFIGQUALITY
 */

void FrameMain::OnConfigqualityUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_CONFIGQUALITY in FrameMain.
    // Before editing this code, remove the block markers.
    wxCheckBox* checkBox = (wxCheckBox*) FindWindow(ID_ENABLEQUALITY);
    event.Enable(checkBox->GetValue());
//@end wxEVT_UPDATE_UI event handler for ID_CONFIGQUALITY in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_PARAMQUALITY
 */

void FrameMain::OnParamqualityUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_PARAMQUALITY in FrameMain.
    // Before editing this code, remove the block markers.
    wxToggleButton* toggleButton = (wxToggleButton*) FindWindow(ID_CONFIGQUALITY);
    FindWindow(ID_PARAMQUALITY)->Show(toggleButton->GetValue());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_PARAMQUALITY in FrameMain. 
}


/*!
 * wxEVT_UPDATE_UI event handler for ID_CONFIGEXPOSURE
 */

void FrameMain::OnConfigexposureUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_CONFIGEXPOSURE in FrameMain.
    // Before editing this code, remove the block markers.
    wxCheckBox* checkBox = (wxCheckBox*) FindWindow(ID_ENABLEEXPOSURE);
    event.Enable(checkBox->GetValue());
//@end wxEVT_UPDATE_UI event handler for ID_CONFIGEXPOSURE in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_PARAMEXPOSURE
 */

void FrameMain::OnParamexposureUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_PARAMEXPOSURE in FrameMain.
    // Before editing this code, remove the block markers.
    wxToggleButton* toggleButton = (wxToggleButton*) FindWindow(ID_CONFIGEXPOSURE);
    FindWindow(ID_PARAMEXPOSURE)->Show(toggleButton->GetValue());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_PARAMEXPOSURE in FrameMain. 
}


/*!
 * wxEVT_UPDATE_UI event handler for ID_CONFIGCLIPPING
 */

void FrameMain::OnConfigclippingUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_CONFIGCLIPPING in FrameMain.
    // Before editing this code, remove the block markers.
    wxCheckBox* checkBox = (wxCheckBox*) FindWindow(ID_ENABLECLIPPING);
    event.Enable(checkBox->GetValue());
//@end wxEVT_UPDATE_UI event handler for ID_CONFIGCLIPPING in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_PARAMCLIPPING
 */

void FrameMain::OnParamclippingUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_PARAMCLIPPING in FrameMain.
    // Before editing this code, remove the block markers.
    wxToggleButton* toggleButton = (wxToggleButton*) FindWindow(ID_CONFIGCLIPPING);
    FindWindow(ID_PARAMCLIPPING)->Show(toggleButton->GetValue());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_PARAMCLIPPING in FrameMain. 
}


/*!
 * wxEVT_UPDATE_UI event handler for ID_CONFIGDOF
 */

void FrameMain::OnConfigdofUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_CONFIGDOF in FrameMain.
    // Before editing this code, remove the block markers.
    wxCheckBox* checkBox = (wxCheckBox*) FindWindow(ID_ENABLEDOF);
    event.Enable(checkBox->GetValue());
//@end wxEVT_UPDATE_UI event handler for ID_CONFIGDOF in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_PARAMDOF
 */

void FrameMain::OnParamdofUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_PARAMDOF in FrameMain.
    // Before editing this code, remove the block markers.
    wxToggleButton* toggleButton = (wxToggleButton*) FindWindow(ID_CONFIGDOF);
    FindWindow(ID_PARAMDOF)->Show(toggleButton->GetValue());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_PARAMDOF in FrameMain. 
}


/*!
 * wxEVT_UPDATE_UI event handler for ID_CONFIGSHADOWMAP
 */

void FrameMain::OnConfigshadowmapUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_CONFIGSHADOWMAP in FrameMain.
    // Before editing this code, remove the block markers.
    wxCheckBox* checkBox = (wxCheckBox*) FindWindow(ID_ENABLESHADOWMAP);
    event.Enable(checkBox->GetValue());
//@end wxEVT_UPDATE_UI event handler for ID_CONFIGSHADOWMAP in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_PARAMSHADOWMAP
 */

void FrameMain::OnParamshadowmapUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_PARAMSHADOWMAP in FrameMain.
    // Before editing this code, remove the block markers.
    wxToggleButton* toggleButton = (wxToggleButton*) FindWindow(ID_CONFIGSHADOWMAP);
    FindWindow(ID_PARAMSHADOWMAP)->Show(toggleButton->GetValue());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_PARAMSHADOWMAP in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_CONFIGTWEAK
 */

void FrameMain::OnConfigtweakUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_CONFIGTWEAK in FrameMain.
    // Before editing this code, remove the block markers.
    wxCheckBox* checkBox = (wxCheckBox*) FindWindow(ID_ENABLETWEAK);
    event.Enable(checkBox->GetValue());
//@end wxEVT_UPDATE_UI event handler for ID_CONFIGTWEAK in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_PARAMTWEAK
 */

void FrameMain::OnParamtweakUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_PARAMTWEAK in FrameMain.
    // Before editing this code, remove the block markers.
    wxToggleButton* toggleButton = (wxToggleButton*) FindWindow(ID_CONFIGTWEAK);
    FindWindow(ID_PARAMTWEAK)->Show(toggleButton->GetValue());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_PARAMTWEAK in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_CONFIGTWEAKX
 */

void FrameMain::OnConfigtweakxUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_CONFIGTWEAKX in FrameMain.
    // Before editing this code, remove the block markers.
    wxCheckBox* checkBox = (wxCheckBox*) FindWindow(ID_ENABLETWEAKX);
    event.Enable(checkBox->GetValue());
//@end wxEVT_UPDATE_UI event handler for ID_CONFIGTWEAKX in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_PARAMTWEAKX
 */

void FrameMain::OnParamtweakxUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_PARAMTWEAKX in FrameMain.
    // Before editing this code, remove the block markers.
    wxToggleButton* toggleButton = (wxToggleButton*) FindWindow(ID_CONFIGTWEAKX);
    FindWindow(ID_PARAMTWEAKX)->Show(toggleButton->GetValue());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_PARAMTWEAKX in FrameMain. 
}


/*!
 * wxEVT_UPDATE_UI event handler for wxID_UNDO
 */

void FrameMain::OnUndoUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for wxID_UNDO in FrameMain.
    // Before editing this code, remove the block markers.
    event.Enable(false);
//@end wxEVT_UPDATE_UI event handler for wxID_UNDO in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for wxID_REDO
 */

void FrameMain::OnRedoUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for wxID_REDO in FrameMain.
    // Before editing this code, remove the block markers.
    event.Enable(false);
//@end wxEVT_UPDATE_UI event handler for wxID_REDO in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for wxID_CUT
 */

void FrameMain::OnCutUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for wxID_CUT in FrameMain.
    // Before editing this code, remove the block markers.
    event.Enable(false);
//@end wxEVT_UPDATE_UI event handler for wxID_CUT in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for wxID_COPY
 */

void FrameMain::OnCopyUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for wxID_COPY in FrameMain.
    // Before editing this code, remove the block markers.
    event.Enable(false);
//@end wxEVT_UPDATE_UI event handler for wxID_COPY in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for wxID_PASTE
 */

void FrameMain::OnPasteUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for wxID_PASTE in FrameMain.
    // Before editing this code, remove the block markers.
    event.Enable(false);
//@end wxEVT_UPDATE_UI event handler for wxID_PASTE in FrameMain. 
}


#if defined(__WXMSW__) || defined(__WXGTK__)
/*!
 * wxEVT_UPDATE_UI event handler for ID_SHOWSTATUSBAR
 */

void FrameMain::OnShowstatusbarUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_SHOWSTATUSBAR in FrameMain.
    // Before editing this code, remove the block markers.
    wxMenuBar* mbar = GetMenuBar();
    mbar->Check(ID_SHOWSTATUSBAR, GetStatusBar() && GetStatusBar()->IsShown());
    //wxMenuItem* menuItem = (wxMenuItem*) FindWindow(ID_SHOWSTATUSBAR);
    //FindWindow(ID_STATUSBAR)->Show(menuItem->IsChecked());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_SHOWSTATUSBAR in FrameMain. 
}
#if defined(__WXMAC__)
    //wxMenuBar* mbar = GetMenuBar();
    //mbar->Check(ID_SHOWSTATUSBAR, GetStatusBar() && GetStatusBar()->IsShown());
    //event.Skip();
#endif
#endif

/*!
 * wxEVT_UPDATE_UI event handler for ID_SHOWGENERAL
 */

void FrameMain::OnShowgeneralUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_SHOWGENERAL in FrameMain.
    // Before editing this code, remove the block markers.
    wxMenuItem* menuItem = (wxMenuItem*) FindWindow(ID_SHOWGENERAL);
    FindWindow(ID_NOTEBOOK_GENERAL)->Show(menuItem->IsChecked());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_SHOWGENERAL in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_SHOWPREVIEW
 */

void FrameMain::OnShowpreviewUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_SHOWPREVIEW in FrameMain.
    // Before editing this code, remove the block markers.
    wxMenuItem* menuItem = (wxMenuItem*) FindWindow(ID_SHOWPREVIEW);
    FindWindow(ID_NOTEBOOK_PREVIEW)->Show(menuItem->IsChecked());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_SHOWPREVIEW in FrameMain. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_SHOWLOG
 */

void FrameMain::OnShowlogUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_SHOWLOG in FrameMain.
    // Before editing this code, remove the block markers.
    wxMenuItem* menuItem = (wxMenuItem*) FindWindow(ID_SHOWLOG);
    FindWindow(ID_NOTEBOOK_LOG)->Show(menuItem->IsChecked());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_SHOWLOG in FrameMain. 
}


/*!
 * wxEVT_UPDATE_UI event handler for ID_STATIC_PREVIEW
 */

void FrameMain::OnStaticPreviewUpdate( wxUpdateUIEvent& event )
{
//@begin wxEVT_UPDATE_UI event handler for ID_STATIC_PREVIEW in FrameMain.
    // Before editing this code, remove the block markers.
    wxScrolledWindow* scrolledWindow = (wxScrolledWindow*) FindWindow(ID_NOTEBOOK_PREVIEW);
    FindWindow(ID_STATIC_PREVIEW)->Show(scrolledWindow->IsShown());
    event.Skip();
//@end wxEVT_UPDATE_UI event handler for ID_STATIC_PREVIEW in FrameMain. 
}

