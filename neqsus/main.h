/////////////////////////////////////////////////////////////////////////////
// Name:        main.h
// Purpose:     
// Author:      Leon Tony Atkinson
// Modified by: 
// Created:     03/11/05 18:25:31
// RCS-ID:      
// Copyright:   Copyright 2005, Leon Tony Atkinson
// Licence:     GPL (General Public License)
/////////////////////////////////////////////////////////////////////////////

#ifndef _MAIN_H_
#define _MAIN_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "main.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/notebook.h"
#include "wx/spinctrl.h"
#include "wx/tglbtn.h"
#include "wx/statline.h"
#include "wx/treectrl.h"
#include "wx/statusbr.h"
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
#define ID_FRAME_MAIN 10000
#define SYMBOL_FRAMEMAIN_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxCLOSE_BOX
#define SYMBOL_FRAMEMAIN_TITLE _("Neqsus - vase.rib")
#define SYMBOL_FRAMEMAIN_IDNAME ID_FRAME_MAIN
#define SYMBOL_FRAMEMAIN_SIZE wxDefaultSize
#define SYMBOL_FRAMEMAIN_POSITION wxDefaultPosition
#define ID_MRU 10021
#define ID_MRU1 10009
#define ID_MRU2 10017
#define ID_MRU3 10018
#define ID_MRU4 10019
#define ID_MRU5 10022
#define ID_MRU6 10023
#define ID_MRU7 10024
#define ID_MRU8 10025
#define ID_MRU9 10026
#define ID_MRU10 10027
#define ID_SHOWSTATUSBAR 10086
#define ID_ENABLEAQSL 10003
#define ID_ENABLETEQSER 10005
#define ID_ENABLEBEEP 10006
#define ID_ENABLECGKIT 10008
#define wxMINIMIZE 10090
#define wxMAXIMIZE 10076
#define ID_SHOWGENERAL 10087
#define ID_SHOWPREVIEW 10088
#define ID_SHOWLOG 10007
#define ID_NOTEBOOK 10001
#define ID_NOTEBOOK_GENERAL 10034
#define ID_PARAMIMAGE 10013
#define ID_WIDTH 10014
#define ID_HEIGHT 10015
#define ID_PIXELASPECTRATIO 10020
#define ID_ASPECTRATIO 10016
#define ID_CONFIGANIMATION 10042
#define ID_ENABLEANIMATION 10043
#define ID_PARAMANIMATION 10010
#define ID_STARTFRAME 10030
#define ID_ENDFRAME 10031
#define ID_EXCLUDEFRAME 10032
#define ID_CONFIGSHUTTER 10041
#define ID_ENABLESHUTTER 10033
#define ID_PARAMSHUTTER 10012
#define ID_SHUTTEROPEN 10028
#define ID_SHUTTERCLOSE 10029
#define ID_CONFIGQUALITY 10046
#define ID_ENABLEQUALITY 10064
#define ID_PARAMQUALITY 10044
#define ID_PIXELSAMPLE 10068
#define ID_SHADINGRATE 10069
#define ID_CONFIGEXPOSURE 10057
#define ID_ENABLEEXPOSURE 10058
#define ID_PARAMEXPOSURE 10047
#define ID_GAIN 10055
#define ID_GAMMA 10056
#define ID_CONFIGCLIPPING 10059
#define ID_ENABLECLIPPING 10060
#define ID_PARAMCLIPPING 10066
#define ID_HITHER 10054
#define ID_YON 10061
#define ID_CONFIGDOF 10040
#define ID_ENABLEDOF 10036
#define ID_PARAMDOF 10045
#define ID_FSTOP 10037
#define ID_FOCALLENGTH 10038
#define ID_FOCALDISTANCE 10039
#define ID_CONFIGSHADOWMAP 10080
#define ID_ENABLESHADOWMAP 10035
#define ID_PARAMSHADOWMAP 10062
#define ID_SHADOWMAPSAMPLES 10063
#define ID_ENABLEFILE 10070
#define ID_ENABLEFRAMEBUFFER 10081
#define ID_RENDER 10084
#define ID_NOTEBOOK_PREVIEW 10065
#define ID_PARAMLIBRARY 10004
#define ID_LIBRARY 10048
#define ID_CONFIGTWEAK 10050
#define ID_ENABLETWEAK 10051
#define ID_PARAMTWEAK 10071
#define ID_OPACITY 10049
#define ID_FOCUS 10053
#define ID_CONFIGTWEAKX 10073
#define ID_ENABLETWEAKX 10074
#define ID_PARAMTWEAKX 10078
#define ID_DUMMY1 10052
#define ID_DUMMY2 10072
#define ID_DUMMY3 10077
#define ID_PREVIEW 10085
#define ID_NOTEBOOK_LOG 10067
#define ID_LOG 10011
#define ID_STATIC_PREVIEW 10075
#define ID_STATUSBAR 10002
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
 * FrameMain class declaration
 */

class FrameMain: public wxFrame
{    
    DECLARE_CLASS( FrameMain )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    FrameMain( );
    FrameMain( wxWindow* parent, wxWindowID id = SYMBOL_FRAMEMAIN_IDNAME, const wxString& caption = SYMBOL_FRAMEMAIN_TITLE, const wxPoint& pos = SYMBOL_FRAMEMAIN_POSITION, const wxSize& size = SYMBOL_FRAMEMAIN_SIZE, long style = SYMBOL_FRAMEMAIN_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_FRAMEMAIN_IDNAME, const wxString& caption = SYMBOL_FRAMEMAIN_TITLE, const wxPoint& pos = SYMBOL_FRAMEMAIN_POSITION, const wxSize& size = SYMBOL_FRAMEMAIN_SIZE, long style = SYMBOL_FRAMEMAIN_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin FrameMain event handler declarations

#if defined(__WXMSW__)
    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT
    void OnExitClick( wxCommandEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for wxID_UNDO
    void OnUndoUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for wxID_REDO
    void OnRedoUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for wxID_CUT
    void OnCutUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for wxID_COPY
    void OnCopyUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for wxID_PASTE
    void OnPasteUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXGTK__)
    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_PREFERENCES
    void OnPreferencesClick( wxCommandEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_SHOWSTATUSBAR
    void OnShowstatusbarUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXMAC__)
    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_PREFERENCES
    void OnPreferencesClick( wxCommandEvent& event );

#endif
#if defined(__WXMAC__)
    /// wxEVT_UPDATE_UI event handler for ID_SHOWSTATUSBAR
    void OnShowstatusbarUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_SHOWGENERAL
    void OnShowgeneralUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_SHOWPREVIEW
    void OnShowpreviewUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_SHOWLOG
    void OnShowlogUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_ABOUT
    void OnAboutClick( wxCommandEvent& event );

#endif
#if defined(__WXMAC__)
    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_ABOUT
    void OnAboutClick( wxCommandEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_CONFIGANIMATION
    void OnConfiganimationUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_PARAMANIMATION
    void OnParamanimationUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_CONFIGSHUTTER
    void OnConfigshutterUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_PARAMSHUTTER
    void OnParamshutterUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_CONFIGQUALITY
    void OnConfigqualityUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_PARAMQUALITY
    void OnParamqualityUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_CONFIGEXPOSURE
    void OnConfigexposureUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_PARAMEXPOSURE
    void OnParamexposureUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_CONFIGCLIPPING
    void OnConfigclippingUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_PARAMCLIPPING
    void OnParamclippingUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_CONFIGDOF
    void OnConfigdofUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_PARAMDOF
    void OnParamdofUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_CONFIGSHADOWMAP
    void OnConfigshadowmapUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_PARAMSHADOWMAP
    void OnParamshadowmapUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_CONFIGTWEAK
    void OnConfigtweakUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_PARAMTWEAK
    void OnParamtweakUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_CONFIGTWEAKX
    void OnConfigtweakxUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_PARAMTWEAKX
    void OnParamtweakxUpdate( wxUpdateUIEvent& event );

#endif
#if defined(__WXMSW__) || defined(__WXGTK__)
    /// wxEVT_UPDATE_UI event handler for ID_STATIC_PREVIEW
    void OnStaticPreviewUpdate( wxUpdateUIEvent& event );

#endif
////@end FrameMain event handler declarations

////@begin FrameMain member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end FrameMain member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin FrameMain member variables
////@end FrameMain member variables
};

#endif
    // _MAIN_H_
