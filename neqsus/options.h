/////////////////////////////////////////////////////////////////////////////
// Name:        options.h
// Purpose:     
// Author:      Leon Tony Atkinson
// Modified by: 
// Created:     03/27/05 16:54:55
// RCS-ID:      
// Copyright:   Copyright 2005, Leon Tony Atkinson
// Licence:     GPL (General Public License)
/////////////////////////////////////////////////////////////////////////////

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "options.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/statline.h"
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
#define ID_DIALOG_OPTIONS 10029
#define SYMBOL_DIALOGOPTIONS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL
#define SYMBOL_DIALOGOPTIONS_TITLE _("Options")
#define SYMBOL_DIALOGOPTIONS_IDNAME ID_DIALOG_OPTIONS
#define SYMBOL_DIALOGOPTIONS_SIZE wxDefaultSize
#define SYMBOL_DIALOGOPTIONS_POSITION wxDefaultPosition
#define ID_TEXTCTRL 10000
#define ID_BUTTON 10001
#define ID_TEXTCTRL1 10002
#define ID_BUTTON1 10003
#define ID_TEXTCTRL2 10004
#define ID_BUTTON2 10005
#define ID_TEXTCTRL3 10006
#define ID_BUTTON3 10007
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
 * DialogOptions class declaration
 */

class DialogOptions: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( DialogOptions )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    DialogOptions( );
    DialogOptions( wxWindow* parent, wxWindowID id = SYMBOL_DIALOGOPTIONS_IDNAME, const wxString& caption = SYMBOL_DIALOGOPTIONS_TITLE, const wxPoint& pos = SYMBOL_DIALOGOPTIONS_POSITION, const wxSize& size = SYMBOL_DIALOGOPTIONS_SIZE, long style = SYMBOL_DIALOGOPTIONS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DIALOGOPTIONS_IDNAME, const wxString& caption = SYMBOL_DIALOGOPTIONS_TITLE, const wxPoint& pos = SYMBOL_DIALOGOPTIONS_POSITION, const wxSize& size = SYMBOL_DIALOGOPTIONS_SIZE, long style = SYMBOL_DIALOGOPTIONS_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin DialogOptions event handler declarations

////@end DialogOptions event handler declarations

////@begin DialogOptions member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end DialogOptions member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin DialogOptions member variables
////@end DialogOptions member variables
};

#endif
    // _OPTIONS_H_
