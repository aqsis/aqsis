; Title: Aqsis package for Win32 (NSIS)
; Author: Aqsis Team (packages@aqsis.org)
; Info: Last tested with NSIS 2.40
; Other: To make updates easier, all message strings have been placed within the top 20 lines (approx.) of this file.


; Helper defines
!define PACKAGE_SHELLEXT_RIB "Render with @CMAKE_PROJECT_NAME@"
!define PACKAGE_SHELLEXT_RIB_INFO "RenderMan Geometry"
!define PACKAGE_SHELLEXT_RIBGZ_INFO "RenderMan Geometry (Compressed)"
!define PACKAGE_SHELLEXT_SL "Compile with @CMAKE_PROJECT_NAME@"
!define PACKAGE_SHELLEXT_SL_INFO "RenderMan Shader"
!define PACKAGE_SHELLEXT_SLX "Inspect with @CMAKE_PROJECT_NAME@"
!define PACKAGE_SHELLEXT_SLX_INFO "@CMAKE_PROJECT_NAME@ Shader"
!define PACKAGE_WEB_SITE "http://www.aqsis.org"
!define PACKAGE_WEB_SUPPORT "http://community.aqsis.org"
!define PACKAGE_WEB_UPDATE "http://downloads.sourceforge.net/aqsis"
!define PACKAGE_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\aqsis.exe"
!define PACKAGE_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\@CMAKE_PROJECT_NAME@"
!define PACKAGE_UNINST_ROOT_KEY "HKLM"
!define PACKAGE_STARTMENU_REGVAL "NSIS:StartMenuDir"

Name "@AQSIS_PROJECT_NAME@ @VERSION_MAJOR@.@VERSION_MINOR@.@VERSION_BUILD@"
BrandingText "www.aqsis.org"
OutFile "@CMAKE_BINARY_DIR@\@AQSIS_PACKAGE_NAME@"
InstallDir "$PROGRAMFILES\@CMAKE_PROJECT_NAME@"
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin
SetCompressor lzma
CRCCheck on
XPStyle on


; Pages
!include "MUI2.nsh"
!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "@WINPACKAGEDIR@\header.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "@WINPACKAGEDIR@\header.bmp"
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "@WINPACKAGEDIR@\wizard.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "@WINPACKAGEDIR@\wizard.bmp"

; Welcome page
!insertmacro MUI_PAGE_WELCOME

; License page
!define MUI_LICENSEPAGE_RADIOBUTTONS
!insertmacro MUI_PAGE_LICENSE "@CMAKE_SOURCE_DIR@\COPYING"

; Components page
!insertmacro MUI_PAGE_COMPONENTS

; Directory page
!insertmacro MUI_PAGE_DIRECTORY

; Start menu page
Var ICONS_GROUP
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "@AQSIS_PROJECT_NAME@\@VERSION_MAJOR@.@VERSION_MINOR@.@VERSION_BUILD@"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PACKAGE_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PACKAGE_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PACKAGE_STARTMENU_REGVAL}"
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP

; Environment page
Page custom AdditionalTasks AdditionalTasksLeave

; Instfiles page
!insertmacro MUI_PAGE_INSTFILES

; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\redist\vcredist_x86.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Install Microsoft Visual C++ 2005 (SP1) Runtime"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\doc\README.txt"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Language files
!insertmacro MUI_LANGUAGE "English"


; Installer 'Version' tab content
VIProductVersion "@VERSION_MAJOR@.@VERSION_MINOR@.@VERSION_BUILD@.@SCM_REVISION@"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "@AQSIS_PROJECT_VENDOR@"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "@AQSIS_PROJECT_NAME@"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "@VERSION_MAJOR@.@VERSION_MINOR@.@VERSION_BUILD@.@SCM_REVISION@"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "@AQSIS_PROJECT_COPYRIGHT@"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" "@AQSIS_PROJECT_COPYRIGHT_OTHER@"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "@AQSIS_PROJECT_NAME@"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "@VERSION_MAJOR@.@VERSION_MINOR@.@VERSION_BUILD@.@SCM_REVISION@"


; Installation types (i.e. full/minimal/custom)
!include "TextFunc.nsh"
!include "StrFunc.nsh"
${StrRep}
!insertmacro ConfigWrite

InstType "Full"
InstType "Minimal"

Section "!Main" SEC01
SectionIn 1 2
  SetOutPath "$INSTDIR\bin"
  File "@CMAKE_BINARY_DIR@\aqsisrc"
  File "@CMAKE_BINARY_DIR@\bin\@CMAKE_BUILD_TYPE@\*.dll"
  File "@CMAKE_BINARY_DIR@\bin\@CMAKE_BUILD_TYPE@\*.exe"
  SetOutPath "$INSTDIR\doc"
  File "/oname=AUTHORS.txt" "@CMAKE_SOURCE_DIR@\AUTHORS"
  File "/oname=LICENSE.txt" "@CMAKE_SOURCE_DIR@\COPYING"
  File "/oname=README.txt" "@CMAKE_SOURCE_DIR@\README"
  SetOutPath "$INSTDIR\shaders"
  File "@CMAKE_BINARY_DIR@\shaders\*.slx"
  SetOutPath "$INSTDIR\redist"
  File "@AQSIS_WIN32LIBS@\MSVC\bin\vcredist_x86.exe"

  ; Convert backslashes, as used by the $INSTDIR variable, to forward-slashes - Defined in RISpec (http://renderman.pixar.com)
  ${StrRep} $R0 $INSTDIR "\" "/"
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string shader" ' '["$R0/shaders"]' $R1
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string archive" ' '["$R0"]' $R2
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string texture" ' '["$R0"]' $R3
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string display" ' '["$R0/bin"]' $R4
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string procedural" ' '["$R0/bin"]' $R5
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string resource" ' '["$R0"]' $R6

; Shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP\Documentation"
  SetOutPath "$INSTDIR\doc"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Documentation\Readme.lnk" "$INSTDIR\doc\README.txt"
  SetOutPath "$INSTDIR\bin"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\@CMAKE_PROJECT_NAME@.lnk" "$INSTDIR\bin\eqsl.exe"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\@CMAKE_PROJECT_NAME@ (Command Line).lnk" "$SYSDIR\cmd.exe" '/k "$INSTDIR\bin\aqsis.exe" -h'
  SetOutPath "$INSTDIR\doc"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Credits.lnk" "$INSTDIR\doc\AUTHORS.txt"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\License.lnk" "$INSTDIR\doc\LICENSE.txt"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

SectionGroup /e "Content" SEC02
  Section "Examples" SEC0201
  SectionIn 1 2
    SetOutPath "$INSTDIR\@CONTENTDIR_NAME@"
    File /r /x ".svn" /x "*.sh" /x "CMake*.*" "@CMAKE_SOURCE_DIR@\content\*"


  ; Shortcuts
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    CreateDirectory "$SMPROGRAMS\$ICONS_GROUP\Examples"
    SetOutPath "$INSTDIR\@CONTENTDIR_NAME@\ribs"
    CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Examples\Features.lnk" "$INSTDIR\@CONTENTDIR_NAME@\ribs\features"
    CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Examples\Scenes.lnk" "$INSTDIR\@CONTENTDIR_NAME@\ribs\scenes"
    !insertmacro MUI_STARTMENU_WRITE_END
  SectionEnd

  Section /o "Scripts" SEC0202
  SectionIn 1
    SetOutPath "$INSTDIR\@SCRIPTSDIR_NAME@"
    File /x "CMake*.*" "@CMAKE_SOURCE_DIR@\tools\mpdump\*.py"
  SectionEnd

  Section /o "Source Shaders" SEC0203
  SectionIn 1
    SetOutPath "$INSTDIR\shaders"
    File "@CMAKE_SOURCE_DIR@\shaders\*.sl"
  SectionEnd
SectionGroupEnd

Section "Documentation" SEC03
SectionIn 1 2
  SetOutPath "$INSTDIR\doc"
  File "/oname=CHANGES.txt" "@CMAKE_SOURCE_DIR@\ChangeLog.txt"
  File "/oname=INSTALL.txt" "@CMAKE_SOURCE_DIR@\INSTALL"
  File "/oname=NOTES.txt" "@CMAKE_SOURCE_DIR@\ReleaseNotes"

; Shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  SetOutPath "$INSTDIR\doc"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Documentation\What's New.lnk" "$INSTDIR\doc\CHANGES.txt"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section /o "Libraries" SEC04
SectionIn 1
  SetOutPath "$INSTDIR\include\aqsis"
  File "@CMAKE_SOURCE_DIR@\aqsistypes\*.h"
  File "@CMAKE_BINARY_DIR@\aqsistypes\aqsis_config.h"
  File "@CMAKE_SOURCE_DIR@\aqsistypes\win32\*.h"
  File "@CMAKE_SOURCE_DIR@\renderer\ddmanager\ndspy.h"
  File "@CMAKE_SOURCE_DIR@\shadercompiler\shadervm\shadeop.h"
  File "@CMAKE_SOURCE_DIR@\rib\api\ri.h"
  File "@CMAKE_BINARY_DIR@\rib\api\ri.inl"
  File "@CMAKE_SOURCE_DIR@\rib\ribparse\*.h"
  SetOutPath "$INSTDIR\lib"
  File /nonfatal "@CMAKE_BINARY_DIR@\bin\@CMAKE_BUILD_TYPE@\*.a"
  File /nonfatal "@CMAKE_BINARY_DIR@\bin\@CMAKE_BUILD_TYPE@\*.lib"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "Rendering application and essential files only"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "Examples, scripts and shader source files"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC0201} "Example files (.rib)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC0202} "Script files (.py)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC0203} "Generic shader source files (.sl)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} "User guides and other information"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC04} "Include and library files"
!insertmacro MUI_FUNCTION_DESCRIPTION_END


Section -AdditionalIcons
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  WriteIniStr "$INSTDIR\doc\website.url" "InternetShortcut" "URL" "${PACKAGE_WEB_SITE}"
  SetOutPath "$INSTDIR\doc"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Documentation\@CMAKE_PROJECT_NAME@ Website.lnk" "$INSTDIR\doc\website.url"
  SetOutPath "$INSTDIR"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Uninstall @CMAKE_PROJECT_NAME@.lnk" "$INSTDIR\uninst.exe"
  SetOutPath "$INSTDIR\doc"
  CreateShortCut "$INSTDIR\LICENSE.lnk" "$INSTDIR\doc\LICENSE.txt"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd


Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PACKAGE_DIR_REGKEY}" "" "$INSTDIR\bin\aqsis.exe"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "DisplayIcon" "$INSTDIR\bin\eqsl.exe"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "DisplayVersion" "@VERSION_MAJOR@.@VERSION_MINOR@.@VERSION_BUILD@"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "URLInfoAbout" "${PACKAGE_WEB_SITE}"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "Publisher" "@AQSIS_PROJECT_VENDOR@"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "HelpLink" "${PACKAGE_WEB_SUPPORT}"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "URLUpdateInfo" "${PACKAGE_WEB_UPDATE}"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "Comments" "$INSTDIR"
  WriteRegDWORD ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "NoRepair" 1

  Call EnvironmentCurrent
  Call EnvironmentAll
  Call DesktopIcon
  Call DesktopQuicklaunch
  Call EnvironmentHome
  Call RegisterMIME
SectionEnd


Function AdditionalTasks
  Var /GLOBAL AQSISHOME
  Var /GLOBAL DESKTOP_ICON
  Var /GLOBAL FILE_EXTENSION
  Var /GLOBAL MUI_PAGE_CUSTOM
  Var /GLOBAL PATH_NT
  Var /GLOBAL PATH_NT_ALL
  Var /GLOBAL PATH_NT_NONE
  Var /GLOBAL QUICKLAUCH_ICON

  !include LogicLib.nsh
  !include nsDialogs.nsh
  !insertmacro MUI_HEADER_TEXT "Choose Additional Tasks" "Choose the additional tasks to be performed."

  nsDialogs::Create /NOUNLOAD 1018
    Pop $MUI_PAGE_CUSTOM

    ${If} $MUI_PAGE_CUSTOM == error
      Abort
    ${EndIf}

  ${NSD_CreateLabel} 0 0 100% 20 "Update PATH environment variable:"
    Pop $R9

  ${NSD_CreateRadioButton} 5 20 100% 20 "For current user"
    Pop $PATH_NT

  ${NSD_CreateRadioButton} 5 40 100% 20 "For all users"
    Pop $PATH_NT_ALL
    ${NSD_SetState} $PATH_NT_ALL ${BST_CHECKED}

  ${NSD_CreateRadioButton} 5 60 100% 20 "None"
    Pop $PATH_NT_NONE

  ${NSD_CreateLabel} 0 90 100% 20 "Additional tasks:"
    Pop $R8

  ${NSD_CreateCheckBox} 5 110 100% 20 "Create a desktop icon"
    Pop $DESKTOP_ICON
    ${NSD_SetState} $DESKTOP_ICON ${BST_CHECKED}

  ${NSD_CreateCheckBox} 5 130 100% 20 "Create a Quick Launch icon"
    Pop $QUICKLAUCH_ICON

  ${NSD_CreateCheckBox} 5 150 100% 20 "Create AQSISHOME environment variable"
    Pop $AQSISHOME
    ${NSD_SetState} $AQSISHOME ${BST_CHECKED}

  ${NSD_CreateCheckBox} 5 170 100% 20 "Associate @AQSIS_PROJECT_NAME_SHORT@ with the RIB, SL and SLX file extensions"
    Pop $FILE_EXTENSION
    ${NSD_SetState} $FILE_EXTENSION ${BST_CHECKED}

  nsDialogs::Show
FunctionEnd


Function AdditionalTasksLeave
  Var /GLOBAL ALLUSERS
  Var /GLOBAL CURRENTUSER
  Var /GLOBAL DESKTOPUSER
  Var /GLOBAL QUICKLAUCHUSER
  Var /GLOBAL HOME
  Var /GLOBAL MIME
  Var /GLOBAL PATH

  ${NSD_GetState} $PATH_NT $CURRENTUSER
  ${NSD_GetState} $PATH_NT_ALL $ALLUSERS
  ${NSD_GetState} $DESKTOP_ICON $DESKTOPUSER
  ${NSD_GetState} $QUICKLAUCH_ICON $QUICKLAUCHUSER
  ${NSD_GetState} $AQSISHOME $HOME
  ${NSD_GetState} $FILE_EXTENSION $MIME
FunctionEnd


Function EnvironmentCurrent
  ; Update environment variables for current user
  ${If} $CURRENTUSER == 1

    DetailPrint "Updating PATH environment variable, please wait..."

    ReadRegStr $PATH HKCU "Environment" "PATH"
    WriteRegExpandStr HKCU "Environment" "PATH" "$INSTDIR\bin;$PATH"
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
  ${EndIf}
FunctionEnd


Function EnvironmentAll
  ; Update environment variables for all users
  ${If} $ALLUSERS == 1

    DetailPrint "Updating PATH environment variable, please wait..."

    ReadRegStr $PATH HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "$INSTDIR\bin;$PATH"
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
  ${EndIf}
FunctionEnd


Function DesktopIcon
  ; Create desktop icon
  ${If} $DESKTOPUSER == 1

    DetailPrint "Creating desktop icon, please wait..."

    SetOutPath "$INSTDIR\bin"
    CreateShortCut "$DESKTOP\@AQSIS_PROJECT_NAME@ @VERSION_MAJOR@.@VERSION_MINOR@.@VERSION_BUILD@.lnk" "$INSTDIR\bin\eqsl.exe"
  ${EndIF}
FunctionEnd


Function DesktopQuicklaunch
  ; Create Quick Lanuch icon
  ${If} $QUICKLAUCHUSER == 1

    DetailPrint "Creating Quick Launch icon, please wait..."

    SetOutPath "$INSTDIR\bin"
    CreateShortCut "$QUICKLAUNCH\@AQSIS_PROJECT_NAME@.lnk" "$INSTDIR\bin\eqsl.exe"
  ${EndIF}
FunctionEnd


Function EnvironmentHome
  ; Update environment variables for all users
  ${If} $HOME == 1

    DetailPrint "Updating AQSISHOME environment variable, please wait..."

    WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "AQSISHOME" "$INSTDIR"
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
  ${EndIf}
FunctionEnd


Function RegisterMIME
  ; Create file association(s)
  ${If} $MIME == 1

    DetailPrint "Updating file associations, please wait..."

    WriteRegStr HKCR ".rib" "" "@AQSIS_PROJECT_NAME_SHORT@.RIB"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.RIB" "" "${PACKAGE_SHELLEXT_RIB_INFO}"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.RIB\DefaultIcon" "" "$INSTDIR\bin\eqsl.exe,1"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.RIB\shell\open" "" "${PACKAGE_SHELLEXT_RIB}"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.RIB\shell\open\command" "" '"$INSTDIR\bin\aqsis.exe" -progress "%1"'

    ; Compressed RIB (.rib.gz) support needs looking at - Windows seems to have an issue with double extensions !!!
    WriteRegStr HKCR ".ribz" "" "@AQSIS_PROJECT_NAME_SHORT@.RIBGZ"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.RIBGZ" "" "${PACKAGE_SHELLEXT_RIBGZ_INFO}"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.RIBGZ\DefaultIcon" "" "$INSTDIR\bin\eqsl.exe,1"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.RIBGZ\shell\open" "" "${PACKAGE_SHELLEXT_RIB}"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.RIBGZ\shell\open\command" "" '"$INSTDIR\bin\aqsis.exe" -progress "%1"'
    
    WriteRegStr HKCR ".sl" "" "@AQSIS_PROJECT_NAME_SHORT@.SL"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.SL" "" "${PACKAGE_SHELLEXT_SL_INFO}"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.SL\DefaultIcon" "" "$INSTDIR\bin\eqsl.exe,1"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.SL\shell\open" "" "${PACKAGE_SHELLEXT_SL}"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.SL\shell\open\command" "" '"$INSTDIR\bin\aqsl.exe" "%1"'
    
    WriteRegStr HKCR ".slx" "" "@AQSIS_PROJECT_NAME_SHORT@.SLX"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.SLX" "" "${PACKAGE_SHELLEXT_SLX_INFO}"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.SLX\DefaultIcon" "" "$INSTDIR\bin\eqsl.exe,1"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.SLX\shell\open" "" "${PACKAGE_SHELLEXT_SLX}"
    WriteRegStr HKCR "@AQSIS_PROJECT_NAME_SHORT@.SLX\shell\open\command" "" '"$SYSDIR\cmd.exe" "/k" "$INSTDIR\bin\aqsltell.exe" "%1"'
  ${EndIF}
FunctionEnd


; Uninstaller
!include "WordFunc.nsh"
!insertmacro un.WordReplace

Section Uninstall
  !insertmacro MUI_STARTMENU_GETFOLDER "Application" $ICONS_GROUP
  Delete "$DESKTOP\@AQSIS_PROJECT_NAME@ @VERSION_MAJOR@.@VERSION_MINOR@.@VERSION_BUILD@.lnk"
  Delete "$QUICKLAUNCH\@AQSIS_PROJECT_NAME@.lnk"

  RMDir /r "$SMPROGRAMS\$ICONS_GROUP"
  RMDir "$SMPROGRAMS\@AQSIS_PROJECT_NAME@"

  RMDir /r "$INSTDIR"

  DetailPrint "Updating PATH environment variable, please wait..."
  ReadRegStr $PATH_NT HKCU "Environment" "PATH"
  ${un.WordReplace} "$PATH_NT" "$INSTDIR\bin;" "" "+" $PATH
  WriteRegExpandStr HKCU "Environment" "PATH" "$PATH"
  ${If} $PATH == ""
    DeleteRegValue HKCU "Environment" "PATH"
  ${EndIf}

  ReadRegStr $PATH_NT_ALL HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
  ${un.WordReplace} "$PATH_NT_ALL" "$INSTDIR\bin;" "" "+" $PATH
  WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "$PATH"

  DetailPrint "Updating AQSISHOME environment variable, please wait..."
  DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "AQSISHOME"

  DetailPrint "Updating file associations, please wait..."
  DeleteRegKey HKCR ".rib"
  DeleteRegKey HKCR "@AQSIS_PROJECT_NAME_SHORT@.RIB"
  DeleteRegKey HKCR ".ribz"
  DeleteRegKey HKCR "@AQSIS_PROJECT_NAME_SHORT@.RIBGZ"
  DeleteRegKey HKCR ".sl"
  DeleteRegKey HKCR "@AQSIS_PROJECT_NAME_SHORT@.SL"
  DeleteRegKey HKCR ".slx"
  DeleteRegKey HKCR "@AQSIS_PROJECT_NAME_SHORT@.SLX"

  DeleteRegKey ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}"
  DeleteRegKey HKLM "${PACKAGE_DIR_REGKEY}"

  SetAutoClose true
SectionEnd
