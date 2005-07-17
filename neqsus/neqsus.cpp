/////////////////////////////////////////////////////////////////////////////
// Name:        neqsus.cpp
// Purpose:     
// Author:      Leon Tony Atkinson
// Modified by: 
// Created:     03/11/05 18:22:37
// RCS-ID:      
// Copyright:   Copyright 2005, Leon Tony Atkinson
// Licence:     GPL (General Public License)
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "neqsus.h"
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

#include "neqsus.h"

////@begin XPM images
////@end XPM images

/*!
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( NeqsusApp )
////@end implement app

/*!
 * NeqsusApp type definition
 */

IMPLEMENT_CLASS( NeqsusApp, wxApp )

/*!
 * NeqsusApp event table definition
 */

BEGIN_EVENT_TABLE( NeqsusApp, wxApp )

////@begin NeqsusApp event table entries
////@end NeqsusApp event table entries

END_EVENT_TABLE()

/*!
 * Constructor for NeqsusApp
 */

NeqsusApp::NeqsusApp()
{
////@begin NeqsusApp member initialisation
////@end NeqsusApp member initialisation
}

/*!
 * Initialisation for NeqsusApp
 */

bool NeqsusApp::OnInit()
{    
////@begin NeqsusApp initialisation
    // Remove the comment markers above and below this block
    // to make permanent changes to the code.

#if wxUSE_XPM
    wxImage::AddHandler( new wxXPMHandler );
#endif
#if wxUSE_LIBPNG
    wxImage::AddHandler( new wxPNGHandler );
#endif
#if wxUSE_LIBJPEG
    wxImage::AddHandler( new wxJPEGHandler );
#endif
#if wxUSE_GIF
    wxImage::AddHandler( new wxGIFHandler );
#endif
    FrameMain* mainWindow = new FrameMain( NULL, ID_FRAME_MAIN );
    mainWindow->Show(true);
////@end NeqsusApp initialisation

#if wxUSE_TIFF
    wxImage::AddHandler( new wxTIFFHandler );
#endif

    return true;
}

/*!
 * Cleanup for NeqsusApp
 */
int NeqsusApp::OnExit()
{    
////@begin NeqsusApp cleanup
    return wxApp::OnExit();
////@end NeqsusApp cleanup
}

