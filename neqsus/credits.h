/////////////////////////////////////////////////////////////////////////////
// Name:        credits.h
// Purpose:     
// Author:      Leon Tony Atkinson
// Modified by: 
// Created:     03/27/05 16:49:09
// RCS-ID:      
// Copyright:   Copyright 2005, Leon Tony Atkinson
// Licence:     GPL (General Public License)
/////////////////////////////////////////////////////////////////////////////

#ifndef _CREDITS_H_
#define _CREDITS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "credits.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG_CREDITS 10028
#define SYMBOL_DIALOGCREDITS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL
#define SYMBOL_DIALOGCREDITS_TITLE _("About Neqsus")
#define SYMBOL_DIALOGCREDITS_IDNAME ID_DIALOG_CREDITS
#define SYMBOL_DIALOGCREDITS_SIZE wxDefaultSize
#define SYMBOL_DIALOGCREDITS_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * DialogCredits class declaration
 */

class DialogCredits: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( DialogCredits )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    DialogCredits( );
    DialogCredits( wxWindow* parent, wxWindowID id = SYMBOL_DIALOGCREDITS_IDNAME, const wxString& caption = SYMBOL_DIALOGCREDITS_TITLE, const wxPoint& pos = SYMBOL_DIALOGCREDITS_POSITION, const wxSize& size = SYMBOL_DIALOGCREDITS_SIZE, long style = SYMBOL_DIALOGCREDITS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DIALOGCREDITS_IDNAME, const wxString& caption = SYMBOL_DIALOGCREDITS_TITLE, const wxPoint& pos = SYMBOL_DIALOGCREDITS_POSITION, const wxSize& size = SYMBOL_DIALOGCREDITS_SIZE, long style = SYMBOL_DIALOGCREDITS_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin DialogCredits event handler declarations

////@end DialogCredits event handler declarations

////@begin DialogCredits member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end DialogCredits member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin DialogCredits member variables
////@end DialogCredits member variables
};

#endif
    // _CREDITS_H_
