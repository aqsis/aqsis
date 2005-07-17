/////////////////////////////////////////////////////////////////////////////
// Name:        about.h
// Purpose:     
// Author:      Leon Tony Atkinson
// Modified by: 
// Created:     03/12/05 10:14:00
// RCS-ID:      
// Copyright:   Copyright 2005, Leon Tony Atkinson
// Licence:     GPL (General Public License)
/////////////////////////////////////////////////////////////////////////////

#ifndef _ABOUT_H_
#define _ABOUT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "about.cpp"
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
#define ID_DIALOG_ABOUT 10028
#define SYMBOL_DIALOGABOUT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL
#define SYMBOL_DIALOGABOUT_TITLE _("About Neqsus")
#define SYMBOL_DIALOGABOUT_IDNAME ID_DIALOG_ABOUT
#define SYMBOL_DIALOGABOUT_SIZE wxDefaultSize
#define SYMBOL_DIALOGABOUT_POSITION wxDefaultPosition
#define ID_CREDITS 10010
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
 * DialogAbout class declaration
 */

class DialogAbout: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( DialogAbout )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    DialogAbout( );
    DialogAbout( wxWindow* parent, wxWindowID id = SYMBOL_DIALOGABOUT_IDNAME, const wxString& caption = SYMBOL_DIALOGABOUT_TITLE, const wxPoint& pos = SYMBOL_DIALOGABOUT_POSITION, const wxSize& size = SYMBOL_DIALOGABOUT_SIZE, long style = SYMBOL_DIALOGABOUT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DIALOGABOUT_IDNAME, const wxString& caption = SYMBOL_DIALOGABOUT_TITLE, const wxPoint& pos = SYMBOL_DIALOGABOUT_POSITION, const wxSize& size = SYMBOL_DIALOGABOUT_SIZE, long style = SYMBOL_DIALOGABOUT_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin DialogAbout event handler declarations

////@end DialogAbout event handler declarations

////@begin DialogAbout member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end DialogAbout member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin DialogAbout member variables
////@end DialogAbout member variables
};

#endif
    // _ABOUT_H_
