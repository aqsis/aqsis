; Title: Aqsis package for Win32 (NSIS)
; Author: Aqsis Team (packages@aqsis.org)
; Info: Last tested with NSIS 2.37
; Other: To make updates easier, all message strings have been placed within the top 20 lines (approx.) of this file.


; Helper defines
!define PACKAGE_SHELLEXT_EXR "View with @CMAKE_PROJECT_NAME@"
!define PACKAGE_SHELLEXT_EXR_INFO "OpenEXR Image"
!define PACKAGE_SHELLEXT_RIB "Render with @CMAKE_PROJECT_NAME@"
!define PACKAGE_SHELLEXT_RIB_INFO "RenderMan Geometry"
!define PACKAGE_SHELLEXT_RIBGZ_INFO "RenderMan Geometry (Compressed)"
!define PACKAGE_SHELLEXT_SL "Compile with @CMAKE_PROJECT_NAME@"
!define PACKAGE_SHELLEXT_SL_INFO "RenderMan Shader"
!define PACKAGE_SHELLEXT_SLX "Inspect with @CMAKE_PROJECT_NAME@"
!define PACKAGE_SHELLEXT_SLX_INFO "@CMAKE_PROJECT_NAME@ Shader"
!define PACKAGE_SHELLEXT_TIF "View with @CMAKE_PROJECT_NAME@"
!define PACKAGE_SHELLEXT_TIF_INFO "TIFF Image"
!define PACKAGE_WEB_SITE "http://www.aqsis.org"
!define PACKAGE_WEB_SUPPORT "http://www.aqsis.org/xoops/modules/newbb"
!define PACKAGE_WEB_UPDATE "http://www.aqsis.org/xoops/modules/mydownloads"
!define PACKAGE_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\aqsis.exe"
!define PACKAGE_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\@CMAKE_PROJECT_NAME@"
!define PACKAGE_UNINST_ROOT_KEY "HKLM"
!define PACKAGE_STARTMENU_REGVAL "NSIS:StartMenuDir"

Name "@PACKAGE_NAME@ @MAJOR@.@MINOR@.@BUILD@"
BrandingText "www.aqsis.org"
OutFile "@CMAKE_BINARY_DIR@\aqsis-setup-@MAJOR@.@MINOR@.@BUILD@.exe"
InstallDir "$PROGRAMFILES\@CMAKE_PROJECT_NAME@"
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin
SetCompressor lzma
CRCCheck on
XPStyle on


; Pages
!include "MUI.nsh"
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
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "@PACKAGE_NAME@\@MAJOR@.@MINOR@.@BUILD@"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PACKAGE_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PACKAGE_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PACKAGE_STARTMENU_REGVAL}"
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP

; Environment page
Page custom AdditionalTasks

; Instfiles page
!insertmacro MUI_PAGE_INSTFILES

; Finish page
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\doc\README.txt"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
ReserveFile "@CMAKE_SOURCE_DIR@\distribution\win\nsis\page_tasks.ini"


; Installer 'Version' tab content
VIProductVersion "@MAJOR@.@MINOR@.@BUILD@.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "@PACKAGE_VENDOR@"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "@PACKAGE_NAME@"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "@MAJOR@.@MINOR@.@BUILD@"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "@PACKAGE_COPYRIGHT@"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" "@PACKAGE_COPYRIGHT_OTHER@"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "@CMAKE_PROJECT_NAME@"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "@MAJOR@.@MINOR@.@BUILD@"


; Installation types (i.e. full/minimal/custom)
!include "TextFunc.nsh"
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

  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string shader" ' '["$INSTDIR\shaders"]' $R0
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string archive" ' '["$INSTDIR"]' $R1
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string texture" ' '["$INSTDIR"]' $R2
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string display" ' '["$INSTDIR\bin"]' $R3
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string procedural" ' '["$INSTDIR"]' $R4
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string plugin" ' '["$INSTDIR"]' $R5
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string resource" ' '["$INSTDIR"]' $R6

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
    SetOutPath "$INSTDIR\content\ribs\features\layeredshaders"
    File /x ".svn" /x "*.sh" "@CMAKE_SOURCE_DIR@\content\ribs\features\layeredshaders\*.*"
    SetOutPath "$INSTDIR\content\ribs\scenes\vase"
    File /x ".svn" /x "*.sh" "@CMAKE_SOURCE_DIR@\content\ribs\scenes\vase\*.*"
    SetOutPath "$INSTDIR\content\shaders\displacement"
    File "@CMAKE_SOURCE_DIR@\content\shaders\displacement\dented.sl"
    SetOutPath "$INSTDIR\content\shaders\light"
    File "@CMAKE_SOURCE_DIR@\content\shaders\light\shadowspot.sl"
    

  ; Shortcuts
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    CreateDirectory "$SMPROGRAMS\$ICONS_GROUP\Examples"
    SetOutPath "$INSTDIR\content\ribs"
    CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Examples\Features.lnk" "$INSTDIR\content\ribs\features"
    CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Examples\Scenes.lnk" "$INSTDIR\content\ribs\scenes"
    !insertmacro MUI_STARTMENU_WRITE_END
  SectionEnd

  Section /o "Scripts" SEC0202
  SectionIn 1
    SetOutPath "$INSTDIR\scripts"
    File "@CMAKE_SOURCE_DIR@\tools\mpdump\*.py"
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
  File "@CMAKE_SOURCE_DIR@\shadercompiler\shadervm\shadeop.h"
  File "@CMAKE_SOURCE_DIR@\rib\api\ri.h"
  File "@CMAKE_BINARY_DIR@\rib\api\ri.inl"
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


Section -AdditionalTasks
Var /GLOBAL AQSISHOME
Var /GLOBAL DESKTOP_ICON
Var /GLOBAL FILE_EXTENSION
Var /GLOBAL PATH
Var /GLOBAL PATH_NT
Var /GLOBAL PATH_NT_ALL
Var /GLOBAL QUICKLAUCH_ICON

  ; Update 'PATH' for current user
  !insertmacro MUI_INSTALLOPTIONS_READ $PATH_NT "page_tasks.ini" "Field 1" "State"
  StrCmp $PATH_NT "1" "path_nt" "path_nt_end"
    path_nt:
    ReadRegStr $PATH HKCU "Environment" "PATH"
    WriteRegExpandStr HKCU "Environment" "PATH" "$INSTDIR\bin;$PATH"
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
    path_nt_end:

  ; Update 'PATH' for all users
  !insertmacro MUI_INSTALLOPTIONS_READ $PATH_NT_ALL "page_tasks.ini" "Field 2" "State"
  StrCmp $PATH_NT_ALL "1" "path_nt_all" "path_nt_all_end"
    path_nt_all:
    ReadRegStr $PATH HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "$INSTDIR\bin;$PATH"
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
    path_nt_all_end:

  ; Create 'Desktop' icon
  !insertmacro MUI_INSTALLOPTIONS_READ $DESKTOP_ICON "page_tasks.ini" "Field 4" "State"
  StrCmp $DESKTOP_ICON "1" "desktop" "desktop_end"
    desktop:
    SetOutPath "$INSTDIR\bin"
    CreateShortCut "$DESKTOP\@PACKAGE_NAME@ @MAJOR@.@MINOR@.@BUILD@.lnk" "$INSTDIR\bin\eqsl.exe"
    desktop_end:

  ; Create 'Quick Launch' icon
  !insertmacro MUI_INSTALLOPTIONS_READ $QUICKLAUCH_ICON "page_tasks.ini" "Field 5" "State"
  StrCmp $QUICKLAUCH_ICON "1" "quicklaunch" "quicklaunch_end"
    quicklaunch:
    SetOutPath "$INSTDIR\bin"
    CreateShortCut "$QUICKLAUNCH\@PACKAGE_NAME@.lnk" "$INSTDIR\bin\eqsl.exe"
    quicklaunch_end:

  ; Create 'AQSISHOME' for all users
  !insertmacro MUI_INSTALLOPTIONS_READ $AQSISHOME "page_tasks.ini" "Field 6" "State"
  StrCmp $AQSISHOME "1" "aqsishome" "aqsishome_end"
    aqsishome:
    WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "AQSISHOME" "$INSTDIR"
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
    aqsishome_end:

  ; Create file association(s)
  !insertmacro MUI_INSTALLOPTIONS_READ $FILE_EXTENSION "page_tasks.ini" "Field 7" "State"
  StrCmp $FILE_EXTENSION "1" "file" "file_end"
    file:
    WriteRegStr HKCR ".exr" "" "Aqsis.EXR"
    WriteRegStr HKCR "Aqsis.EXR" "" "${PACKAGE_SHELLEXT_EXR_INFO}"
    WriteRegStr HKCR "Aqsis.EXR\DefaultIcon" "" "$INSTDIR\bin\eqsl.exe,1"
    WriteRegStr HKCR "Aqsis.EXR\shell\open" "" "${PACKAGE_SHELLEXT_EXR}"
    WriteRegStr HKCR "Aqsis.EXR\shell\open\command" "" '"$INSTDIR\bin\piqsl.exe" "%1"'

    WriteRegStr HKCR ".rib" "" "Aqsis.RIB"
    WriteRegStr HKCR "Aqsis.RIB" "" "${PACKAGE_SHELLEXT_RIB_INFO}"
    WriteRegStr HKCR "Aqsis.RIB\DefaultIcon" "" "$INSTDIR\bin\eqsl.exe,1"
    WriteRegStr HKCR "Aqsis.RIB\shell\open" "" "${PACKAGE_SHELLEXT_RIB}"
    WriteRegStr HKCR "Aqsis.RIB\shell\open\command" "" '"$INSTDIR\bin\aqsis.exe" -progress "%1"'

    ; Compressed RIB (.rib.gz) support needs looking at - Windows seems to have an issue with double extensions !!!
    WriteRegStr HKCR ".ribz" "" "Aqsis.RIBGZ"
    WriteRegStr HKCR "Aqsis.RIBGZ" "" "${PACKAGE_SHELLEXT_RIBGZ_INFO}"
    WriteRegStr HKCR "Aqsis.RIBGZ\DefaultIcon" "" "$INSTDIR\bin\eqsl.exe,1"
    WriteRegStr HKCR "Aqsis.RIBGZ\shell\open" "" "${PACKAGE_SHELLEXT_RIB}"
    WriteRegStr HKCR "Aqsis.RIBGZ\shell\open\command" "" '"$INSTDIR\bin\aqsis.exe" -progress "%1"'
    
    WriteRegStr HKCR ".sl" "" "Aqsis.SL"
    WriteRegStr HKCR "Aqsis.SL" "" "${PACKAGE_SHELLEXT_SL_INFO}"
    WriteRegStr HKCR "Aqsis.SL\DefaultIcon" "" "$INSTDIR\bin\eqsl.exe,1"
    WriteRegStr HKCR "Aqsis.SL\shell\open" "" "${PACKAGE_SHELLEXT_SL}"
    WriteRegStr HKCR "Aqsis.SL\shell\open\command" "" '"$INSTDIR\bin\aqsl.exe" "%1"'
    
    WriteRegStr HKCR ".slx" "" "Aqsis.SLX"
    WriteRegStr HKCR "Aqsis.SLX" "" "${PACKAGE_SHELLEXT_SLX_INFO}"
    WriteRegStr HKCR "Aqsis.SLX\DefaultIcon" "" "$INSTDIR\bin\eqsl.exe,1"
    WriteRegStr HKCR "Aqsis.SLX\shell\open" "" "${PACKAGE_SHELLEXT_SLX}"
    WriteRegStr HKCR "Aqsis.SLX\shell\open\command" "" '"$SYSDIR\cmd.exe" "/k" "$INSTDIR\bin\aqsltell.exe" "%1"'

    WriteRegStr HKCR ".tif" "" "Aqsis.TIF"
    WriteRegStr HKCR "Aqsis.TIF" "" "${PACKAGE_SHELLEXT_TIF_INFO}"
    WriteRegStr HKCR "Aqsis.TIF\DefaultIcon" "" "$INSTDIR\bin\eqsl.exe,1"
    WriteRegStr HKCR "Aqsis.TIF\shell\open" "" "${PACKAGE_SHELLEXT_TIF}"
    WriteRegStr HKCR "Aqsis.TIF\shell\open\command" "" '"$INSTDIR\bin\piqsl.exe" "%1"'

    WriteRegStr HKCR ".tiff" "" "Aqsis.TIFF"
    WriteRegStr HKCR "Aqsis.TIFF" "" "${PACKAGE_SHELLEXT_TIF_INFO}"
    WriteRegStr HKCR "Aqsis.TIFF\DefaultIcon" "" "$INSTDIR\bin\eqsl.exe,1"
    WriteRegStr HKCR "Aqsis.TIFF\shell\open" "" "${PACKAGE_SHELLEXT_TIF}"
    WriteRegStr HKCR "Aqsis.TIFF\shell\open\command" "" '"$INSTDIR\bin\piqsl.exe" "%1"'
    file_end:
SectionEnd


Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PACKAGE_DIR_REGKEY}" "" "$INSTDIR\bin\aqsis.exe"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "DisplayIcon" "$INSTDIR\bin\eqsl.exe"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "DisplayVersion" "@MAJOR@.@MINOR@.@BUILD@"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "URLInfoAbout" "${PACKAGE_WEB_SITE}"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "Publisher" "@PACKAGE_VENDOR@"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "HelpLink" "${PACKAGE_WEB_SUPPORT}"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "URLUpdateInfo" "${PACKAGE_WEB_UPDATE}"
  WriteRegStr ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "Comments" "$INSTDIR"
  WriteRegDWORD ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}" "NoRepair" 1
SectionEnd


Function .onInit
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT_AS "@CMAKE_SOURCE_DIR@\distribution\win\nsis\page_tasks.ini" "page_tasks.ini"
FunctionEnd


Function AdditionalTasks
  !insertmacro MUI_HEADER_TEXT "Choose Additional Tasks" "Choose the additional tasks to be performed."
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "page_tasks.ini"
FunctionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd


Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd


; Uninstaller
!include "WordFunc.nsh"
!insertmacro un.WordReplace

Section Uninstall
  !insertmacro MUI_STARTMENU_GETFOLDER "Application" $ICONS_GROUP
  Delete "$DESKTOP\@PACKAGE_NAME@ @MAJOR@.@MINOR@.@BUILD@.lnk"
  Delete "$QUICKLAUNCH\@PACKAGE_NAME@.lnk"

  RMDir /r "$SMPROGRAMS\$ICONS_GROUP"
  RMDir "$SMPROGRAMS\@PACKAGE_NAME@"

  RMDir /r "$INSTDIR"

  ReadRegStr $PATH_NT HKCU "Environment" "PATH"
  ${un.WordReplace} "$PATH_NT" "$INSTDIR\bin;" "" "+" $PATH
  WriteRegExpandStr HKCU "Environment" "PATH" "$PATH"

  ReadRegStr $PATH_NT_ALL HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
  ${un.WordReplace} "$PATH_NT_ALL" "$INSTDIR\bin;" "" "+" $PATH
  WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "$PATH"

  DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "AQSISHOME"

  DeleteRegKey HKCR ".exr"
  DeleteRegKey HKCR "Aqsis.EXR"
  DeleteRegKey HKCR ".rib"
  DeleteRegKey HKCR "Aqsis.RIB"
  DeleteRegKey HKCR ".ribz"
  DeleteRegKey HKCR "Aqsis.RIBGZ"
  DeleteRegKey HKCR ".sl"
  DeleteRegKey HKCR "Aqsis.SL"
  DeleteRegKey HKCR ".slx"
  DeleteRegKey HKCR "Aqsis.SLX"
  DeleteRegKey HKCR ".tif"
  DeleteRegKey HKCR "Aqsis.TIF"
  DeleteRegKey HKCR ".tiff"
  DeleteRegKey HKCR "Aqsis.TIFF"

  DeleteRegKey ${PACKAGE_UNINST_ROOT_KEY} "${PACKAGE_UNINST_KEY}"
  DeleteRegKey HKLM "${PACKAGE_DIR_REGKEY}"

  SetAutoClose true
SectionEnd
