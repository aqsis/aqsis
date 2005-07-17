/////////////////////////////////////////////////////////////////////////////
// Name:        neqsus.h
// Purpose:     
// Author:      Leon Tony Atkinson
// Modified by: 
// Created:     03/11/05 18:22:37
// RCS-ID:      
// Copyright:   Copyright 2005, Leon Tony Atkinson
// Licence:     GPL (General Public License)
/////////////////////////////////////////////////////////////////////////////

#ifndef _NEQSUS_H_
#define _NEQSUS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "neqsus.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
#include "main.h"
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
////@end control identifiers

/*!
 * NeqsusApp class declaration
 */

class NeqsusApp: public wxApp
{    
    DECLARE_CLASS( NeqsusApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    NeqsusApp();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin NeqsusApp event handler declarations

////@end NeqsusApp event handler declarations

////@begin NeqsusApp member function declarations

////@end NeqsusApp member function declarations

////@begin NeqsusApp member variables
////@end NeqsusApp member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(NeqsusApp)
////@end declare app

#endif
    // _NEQSUS_H_
