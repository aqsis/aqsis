/////////////////////////////////////////////////////////////////////////////
// Name:        options.cpp
// Purpose:     
// Author:      Leon Tony Atkinson
// Modified by: 
// Created:     03/27/05 16:54:55
// RCS-ID:      
// Copyright:   Copyright 2005, Leon Tony Atkinson
// Licence:     GPL (General Public License)
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "options.h"
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
////@end includes

#include "options.h"

////@begin XPM images
////@end XPM images

/*!
 * DialogOptions type definition
 */

IMPLEMENT_DYNAMIC_CLASS( DialogOptions, wxDialog )

/*!
 * DialogOptions event table definition
 */

BEGIN_EVENT_TABLE( DialogOptions, wxDialog )

////@begin DialogOptions event table entries
////@end DialogOptions event table entries

END_EVENT_TABLE()

/*!
 * DialogOptions constructors
 */

DialogOptions::DialogOptions( )
{
}

DialogOptions::DialogOptions( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * DialogOptions creator
 */

bool DialogOptions::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin DialogOptions member initialisation
////@end DialogOptions member initialisation

////@begin DialogOptions creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end DialogOptions creation
    return TRUE;
}

/*!
 * Control creation for DialogOptions
 */

void DialogOptions::CreateControls()
{    
////@begin DialogOptions content construction

    DialogOptions* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Static"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(5, 3, 0, 0);
    itemStaticBoxSizer3->Add(itemFlexGridSizer4, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("AQSIS Directory:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl6 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, _("c:\\program files\\aqsis\\bin"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl6, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_BUTTON, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemButton7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("RIB Directory:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, _("c:\\program files\\aqsis\\examples"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl9, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemDialog1, ID_BUTTON1, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemButton10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("RSL Directory:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl12 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL2, _("c:\\program files\\aqsis\\shaders"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl12, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, ID_BUTTON2, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemButton13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine14 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemStaticBoxSizer3->Add(itemStaticLine14, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer15 = new wxFlexGridSizer(1, 3, 0, 0);
    itemStaticBoxSizer3->Add(itemFlexGridSizer15, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText16 = new wxStaticText( itemDialog1, wxID_STATIC, _("CGKit Directory:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer15->Add(itemStaticText16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl17 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL3, _("c:\\program files\\cgkit\\bin"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer15->Add(itemTextCtrl17, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton18 = new wxButton( itemDialog1, ID_BUTTON3, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer15->Add(itemButton18, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemStaticBoxSizer3->Add(5, 5, 0, wxGROW|wxALL, 5);

    wxStaticLine* itemStaticLine20 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemStaticBoxSizer3->Add(itemStaticLine20, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer21, 0, wxALIGN_RIGHT|wxALL, 0);

#if defined(__WXMSW__) || defined(__WXGTK__)
    wxButton* itemButton22 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton22->SetDefault();
    itemBoxSizer21->Add(itemButton22, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

#endif

    wxButton* itemButton23 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer21->Add(itemButton23, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

#if defined(__WXMAC__)
    wxButton* itemButton24 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton24->SetDefault();
    itemBoxSizer21->Add(itemButton24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

#endif

////@end DialogOptions content construction
}

/*!
 * Should we show tooltips?
 */

bool DialogOptions::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap DialogOptions::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin DialogOptions bitmap retrieval
    return wxNullBitmap;
////@end DialogOptions bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon DialogOptions::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin DialogOptions icon retrieval
    return wxNullIcon;
////@end DialogOptions icon retrieval
}
