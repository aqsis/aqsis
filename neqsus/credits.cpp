/////////////////////////////////////////////////////////////////////////////
// Name:        credits.cpp
// Purpose:     
// Author:      Leon Tony Atkinson
// Modified by: 
// Created:     03/27/05 16:49:09
// RCS-ID:      
// Copyright:   Copyright 2005, Leon Tony Atkinson
// Licence:     GPL (General Public License)
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "credits.h"
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

#include "credits.h"

////@begin XPM images
////@end XPM images

/*!
 * DialogCredits type definition
 */

IMPLEMENT_DYNAMIC_CLASS( DialogCredits, wxDialog )

/*!
 * DialogCredits event table definition
 */

BEGIN_EVENT_TABLE( DialogCredits, wxDialog )

////@begin DialogCredits event table entries
////@end DialogCredits event table entries

END_EVENT_TABLE()

/*!
 * DialogCredits constructors
 */

DialogCredits::DialogCredits( )
{
}

DialogCredits::DialogCredits( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * DialogCredits creator
 */

bool DialogCredits::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin DialogCredits member initialisation
////@end DialogCredits member initialisation

////@begin DialogCredits creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end DialogCredits creation
    return TRUE;
}

/*!
 * Control creation for DialogCredits
 */

void DialogCredits::CreateControls()
{    
////@begin DialogCredits content construction

    DialogCredits* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

////@end DialogCredits content construction
}

/*!
 * Should we show tooltips?
 */

bool DialogCredits::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap DialogCredits::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin DialogCredits bitmap retrieval
    return wxNullBitmap;
////@end DialogCredits bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon DialogCredits::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin DialogCredits icon retrieval
    return wxNullIcon;
////@end DialogCredits icon retrieval
}
