/////////////////////////////////////////////////////////////////////////////
// Name:        about.cpp
// Purpose:     
// Author:      Leon Tony Atkinson
// Modified by: 
// Created:     03/12/05 10:14:00
// RCS-ID:      
// Copyright:   Copyright 2005, Leon Tony Atkinson
// Licence:     GPL (General Public License)
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "about.h"
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

#include "about.h"

////@begin XPM images
////@end XPM images

/*!
 * DialogAbout type definition
 */

IMPLEMENT_DYNAMIC_CLASS( DialogAbout, wxDialog )

/*!
 * DialogAbout event table definition
 */

BEGIN_EVENT_TABLE( DialogAbout, wxDialog )

////@begin DialogAbout event table entries
////@end DialogAbout event table entries

END_EVENT_TABLE()

/*!
 * DialogAbout constructors
 */

DialogAbout::DialogAbout( )
{
}

DialogAbout::DialogAbout( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * Dialog_About creator
 */

bool DialogAbout::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin DialogAbout member initialisation
////@end DialogAbout member initialisation

////@begin DialogAbout creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end DialogAbout creation
    return TRUE;
}

/*!
 * Control creation for Dialog_About
 */

void DialogAbout::CreateControls()
{    
////@begin DialogAbout content construction

    DialogAbout* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxBitmap itemStaticBitmap4Bitmap(wxNullBitmap);
    wxStaticBitmap* itemStaticBitmap4 = new wxStaticBitmap( itemDialog1, wxID_STATIC, itemStaticBitmap4Bitmap, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticBitmap4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Neqsus"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

#if defined(__WXMSW__) || defined(__WXGTK__)
    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("1.0 (alpha)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

#endif

#if defined(__WXMAC__)
    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("1.0 (alpha)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

#endif

    wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Easy-to-use interface for the RenderMan-compliant AQSIS rendering system."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemDialog1, wxID_STATIC, _("Copyright 2005, The AQSIS Team."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

#if defined(__WXMSW__) || defined(__WXGTK__)
    itemBoxSizer3->Add(5, 5, 0, wxGROW|wxALL, 0);

#endif

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

#if defined(__WXGTK__)
    wxButton* itemButton13 = new wxButton( itemDialog1, ID_CREDITS, _("C&redits"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

#endif

#if defined(__WXGTK__)
    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton14->SetDefault();
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

#endif

#if defined(__WXMSW__)
    wxButton* itemButton15 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton15->SetDefault();
    itemBoxSizer12->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

#endif

////@end DialogAbout content construction
}

/*!
 * Should we show tooltips?
 */

bool DialogAbout::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap DialogAbout::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin DialogAbout bitmap retrieval
    return wxNullBitmap;
////@end DialogAbout bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon DialogAbout::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin DialogAbout icon retrieval
    return wxNullIcon;
////@end DialogAbout icon retrieval
}
