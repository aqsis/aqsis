; Title: Aqsis Installer for Win32/64 (NSIS)
; Author: Aqsis Team (packages@aqsis.org)
; Info: Last tested with NSIS 2.21
; Other: 1. To make updates easier, all message strings have been placed within the top 40-50 lines of this file.
;        2. To build manually, without using SCons, uncomment lines 10, 11, 15, 19 and 23.


; Helper defines
!define /date YEAR "%Y"
;;!define PROJECT_ROOT "C:\WINDOWS\Temp\aqsis"
;;!define USE_DEFS "0"

!define PRODUCT_NAME "Aqsis"
!define PRODUCT_FULLNAME "Aqsis Renderer"
;;!define PRODUCT_VERSION_MAJOR "1"
!ifndef PRODUCT_VERSION_MAJOR
  !error "PRODUCT_VERSION_MAJOR not specified"
!endif
;;!define PRODUCT_VERSION_MINOR "2"
!ifndef PRODUCT_VERSION_MINOR
  !error "PRODUCT_VERSION_MINOR not specified"
!endif
;;!define PRODUCT_VERSION_BUILD "0"
!ifndef PRODUCT_VERSION_BUILD
  !error "PRODUCT_VERSION_BUILD not specified"
!endif
!ifndef PRODUCT_VERSION
	!define PRODUCT_VERSION "${PRODUCT_VERSION_MAJOR}.${PRODUCT_VERSION_MINOR}.${PRODUCT_VERSION_BUILD}"
!endif
!define PRODUCT_PUBLISHER "Aqsis Team"
!define PRODUCT_COPYRIGHT "Copyright (c) ${YEAR}, ${PRODUCT_PUBLISHER}."
!define PRODUCT_COPYRIGHT_OTHER "RenderMan(r) Interface Procedures and Protocol are Copyright 1988, 1989, Pixar All Rights Reserved."
!define PRODUCT_SHELLEXT_RIB "Render with Aqsis"
!define PRODUCT_SHELLEXT_RIB_INFO "RenderMan Geometry"
!define PRODUCT_SHELLEXT_RIBGZ_INFO "RenderMan Geometry (Compressed)"
!define PRODUCT_SHELLEXT_SL "Compile with Aqsis"
!define PRODUCT_SHELLEXT_SL_INFO "RenderMan Shader"
!define PRODUCT_SHELLEXT_SLX_INFO "Aqsis Shader"
!define PRODUCT_WEB_SITE "http://www.aqsis.org"
;;!define PRODUCT_WEB_SUPPORT "http://support.aqsis.org"
;;!define PRODUCT_WEB_UPDATE "http://download.aqsis.org"
!define PRODUCT_WEB_SUPPORT "http://www.aqsis.org/xoops/modules/newbb"
!define PRODUCT_WEB_UPDATE "http://www.aqsis.org/xoops/modules/mydownloads"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\aqsis.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"

Name "${PRODUCT_FULLNAME} ${PRODUCT_VERSION}"
BrandingText "www.aqsis.org"
OutFile "${PROJECT_ROOT}\output\aqsis-setup-${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
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
!define MUI_HEADERIMAGE_BITMAP "${PROJECT_ROOT}\distribution\win\nsis\header.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${PROJECT_ROOT}\distribution\win\nsis\header.bmp"
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!if ${PRODUCT_VERSION_MINOR} == 1
  !define MUI_WELCOMEFINISHPAGE_BITMAP "${PROJECT_ROOT}\distribution\win\nsis\wizard.bmp"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "${PROJECT_ROOT}\distribution\win\nsis\wizard.bmp"
!else if ${PRODUCT_VERSION_MINOR} == 3
  !define MUI_WELCOMEFINISHPAGE_BITMAP "${PROJECT_ROOT}\distribution\win\nsis\wizard.bmp"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "${PROJECT_ROOT}\distribution\win\nsis\wizard.bmp"
!else if ${PRODUCT_VERSION_MINOR} == 5
  !define MUI_WELCOMEFINISHPAGE_BITMAP "${PROJECT_ROOT}\distribution\win\nsis\wizard.bmp"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "${PROJECT_ROOT}\distribution\win\nsis\wizard.bmp"
!else if ${PRODUCT_VERSION_MINOR} == 7
  !define MUI_WELCOMEFINISHPAGE_BITMAP "${PROJECT_ROOT}\distribution\win\nsis\wizard.bmp"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "${PROJECT_ROOT}\distribution\win\nsis\wizard.bmp"
!else if ${PRODUCT_VERSION_MINOR} == 9
  !define MUI_WELCOMEFINISHPAGE_BITMAP "${PROJECT_ROOT}\distribution\win\nsis\wizard.bmp"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "${PROJECT_ROOT}\distribution\win\nsis\wizard.bmp"
!else
  !define MUI_WELCOMEFINISHPAGE_BITMAP "${PROJECT_ROOT}\distribution\win\nsis\wizard-${PRODUCT_VERSION_MAJOR}.${PRODUCT_VERSION_MINOR}.bmp"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "${PROJECT_ROOT}\distribution\win\nsis\wizard-${PRODUCT_VERSION_MAJOR}.${PRODUCT_VERSION_MINOR}.bmp"
!endif

; Welcome page
!insertmacro MUI_PAGE_WELCOME

; License page
!define MUI_LICENSEPAGE_RADIOBUTTONS
!insertmacro MUI_PAGE_LICENSE "${PROJECT_ROOT}\COPYING"

; Components page
!insertmacro MUI_PAGE_COMPONENTS

; Directory page
!insertmacro MUI_PAGE_DIRECTORY

; Start menu page
Var ICONS_GROUP
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "${PRODUCT_FULLNAME}\${PRODUCT_VERSION}"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"
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
ReserveFile "${PROJECT_ROOT}\distribution\win\nsis\page_tasks.ini"


; Installer 'Version' tab content
VIProductVersion "${PRODUCT_VERSION_MAJOR}.${PRODUCT_VERSION_MINOR}.${PRODUCT_VERSION_BUILD}.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${PRODUCT_VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${PRODUCT_FULLNAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "${PRODUCT_COPYRIGHT}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" "${PRODUCT_COPYRIGHT_OTHER}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${PRODUCT_VERSION}"


; Installation types (i.e. full/minimal/custom)
!include "TextFunc.nsh"
!insertmacro ConfigWrite

InstType "Full"
InstType "Minimal"

Section "!Main" SEC01
SectionIn 1 2
  SetOutPath "$INSTDIR\bin"
  File "${PROJECT_ROOT}\output\bin\aqsisrc"
  File "${PROJECT_ROOT}\output\bin\*.dll"
  File "${PROJECT_ROOT}\output\bin\*.exe"
  SetOutPath "$INSTDIR\doc"
  File "/oname=AUTHORS.txt" "${PROJECT_ROOT}\AUTHORS"
  File "/oname=LICENSE.txt" "${PROJECT_ROOT}\COPYING"
  File "/oname=README.txt" "${PROJECT_ROOT}\README"
  SetOutPath "$INSTDIR\shaders"
  File "${PROJECT_ROOT}\output\shaders\*.slx"

  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "searchpath" "string shader" ' '["$INSTDIR\shaders"]' $R0
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "searchpath" "string display" ' '["$INSTDIR\bin"]' $R1

; Shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP\Documentation"
  SetOutPath "$INSTDIR\doc"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Documentation\Readme.lnk" "$INSTDIR\doc\README.txt"
  SetOutPath "$INSTDIR\bin"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\${PRODUCT_NAME}.lnk" "$SYSDIR\cmd.exe" '/k "$INSTDIR\bin\aqsis.exe" -h'
  SetOutPath "$INSTDIR\doc"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Credits.lnk" "$INSTDIR\doc\AUTHORS.txt"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\License.lnk" "$INSTDIR\doc\LICENSE.txt"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

SectionGroup /e "Content" SEC02
  Section "Examples" SEC0201
  SectionIn 1 2
    SetOutPath "$INSTDIR\content\ribs\features\layeredshaders"
    File /x ".svn" /x "SConscript" /x "*.sh" "${PROJECT_ROOT}\content\ribs\features\layeredshaders\*.*"
    SetOutPath "$INSTDIR\content\ribs\scenes\vase"
    File /x ".svn" /x "SConscript" /x "*.sh" "${PROJECT_ROOT}\content\ribs\scenes\vase\*.*"
    SetOutPath "$INSTDIR\content\shaders\displacement"
    File "${PROJECT_ROOT}\content\shaders\displacement\dented.sl"
    SetOutPath "$INSTDIR\content\shaders\light"
    File "${PROJECT_ROOT}\content\shaders\light\shadowspot.sl"
    

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
    File "${PROJECT_ROOT}\output\scripts\*.*"
  SectionEnd

  Section /o "Source Shaders" SEC0203
  SectionIn 1
    SetOutPath "$INSTDIR\shaders"
    File "${PROJECT_ROOT}\output\shaders\*.sl"
  SectionEnd
SectionGroupEnd

Section "Documentation" SEC03
SectionIn 1 2
  SetOutPath "$INSTDIR\doc"
  ;;File "/oname=CHANGES.txt" "${PROJECT_ROOT}\output\ChangeLog"
  File "/oname=INSTALL.txt" "${PROJECT_ROOT}\INSTALL"
  File "/oname=NOTES.txt" "${PROJECT_ROOT}\ReleaseNotes"

; Shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  ;;SetOutPath "$INSTDIR\doc"
  ;;CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Documentation\What's New.lnk" "$INSTDIR\doc\CHANGES.txt"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section /o "Libraries" SEC04
SectionIn 1
  SetOutPath "$INSTDIR\include\aqsis"
  File "${PROJECT_ROOT}\output\include\aqsis\*.h"
  File "${PROJECT_ROOT}\output\include\aqsis\*.inl"
  SetOutPath "$INSTDIR\lib"
  File "${PROJECT_ROOT}\output\bin\libaqsis.a"
  File "${PROJECT_ROOT}\output\bin\libaqsistypes.a"
  File "${PROJECT_ROOT}\output\bin\libri2rib.a"
  File "${PROJECT_ROOT}\output\bin\libshadervm.a"
  File "${PROJECT_ROOT}\output\bin\libslxargs.a"
  !if ${USE_DEFS} != 0
    File "${PROJECT_ROOT}\output\bin\aqsis.def"
  !endif
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
  WriteIniStr "$INSTDIR\doc\website.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  SetOutPath "$INSTDIR\doc"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Documentation\${PRODUCT_NAME} Website.lnk" "$INSTDIR\doc\website.url"
  SetOutPath "$INSTDIR"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Uninstall ${PRODUCT_NAME}.lnk" "$INSTDIR\uninst.exe"
  SetOutPath "$INSTDIR\doc"
  CreateShortCut "$INSTDIR\LICENSE.lnk" "$INSTDIR\doc\LICENSE.txt"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd


Section -AdditionalTasks
Var /GLOBAL AQSISHOME
Var /GLOBAL DESKTOP_ICON
Var /GLOBAL FILE_EXTENSION
Var /GLOBAL PATH
;;Var /GLOBAL PATH_95
Var /GLOBAL PATH_NT
Var /GLOBAL PATH_NT_ALL
Var /GLOBAL QUICKLAUCH_ICON

  ; Update 'PATH' for current user
  !insertmacro MUI_INSTALLOPTIONS_READ $PATH_NT "page_tasks.ini" "Field 1" "State"
  StrCmp $PATH_NT "1" "path_nt" "path_nt_end"
    path_nt:
    ReadRegStr $PATH HKCU "Environment" "PATH"
    WriteRegStr HKCU "Environment" "PATH" "$PATH;$INSTDIR\bin"
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
    path_nt_end:

  ; Update 'PATH' for all users
  !insertmacro MUI_INSTALLOPTIONS_READ $PATH_NT_ALL "page_tasks.ini" "Field 2" "State"
  StrCmp $PATH_NT_ALL "1" "path_nt_all" "path_nt_all_end"
    path_nt_all:
    ReadRegStr $PATH HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
    WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "$PATH;$INSTDIR\bin"
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
    path_nt_all_end:

  ; Create 'Desktop' icon
  !insertmacro MUI_INSTALLOPTIONS_READ $DESKTOP_ICON "page_tasks.ini" "Field 4" "State"
  StrCmp $DESKTOP_ICON "1" "desktop" "desktop_end"
    desktop:
    SetOutPath "$INSTDIR\bin"
    CreateShortCut "$DESKTOP\${PRODUCT_FULLNAME} ${PRODUCT_VERSION}.lnk" "$SYSDIR\cmd.exe" '/k "$INSTDIR\bin\aqsis.exe" -h'
    desktop_end:

  ; Create 'Quick Launch' icon
  !insertmacro MUI_INSTALLOPTIONS_READ $QUICKLAUCH_ICON "page_tasks.ini" "Field 5" "State"
  StrCmp $QUICKLAUCH_ICON "1" "quicklaunch" "quicklaunch_end"
    quicklaunch:
    SetOutPath "$INSTDIR\bin"
    CreateShortCut "$QUICKLAUNCH\${PRODUCT_FULLNAME}.lnk" "$SYSDIR\cmd.exe" '/k "$INSTDIR\bin\aqsis.exe" -h'
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
    WriteRegStr HKCR ".rib" "" "Aqsis.RIB"
    WriteRegStr HKCR "Aqsis.RIB" "" "${PRODUCT_SHELLEXT_RIB_INFO}"
    WriteRegStr HKCR "Aqsis.RIB\DefaultIcon" "" "$INSTDIR\bin\aqsis.exe,0"
    WriteRegStr HKCR "Aqsis.RIB\shell\open" "" "${PRODUCT_SHELLEXT_RIB}"
    WriteRegStr HKCR "Aqsis.RIB\shell\open\command" "" '"$INSTDIR\bin\aqsis.exe" -progress "%1"'

    ; Compressed RIB (.rib.gz) support needs looking at - Windows seems to have an issue with double extensions !!!
    WriteRegStr HKCR ".ribz" "" "Aqsis.RIBGZ"
    WriteRegStr HKCR "Aqsis.RIBGZ" "" "${PRODUCT_SHELLEXT_RIBGZ_INFO}"
    WriteRegStr HKCR "Aqsis.RIBGZ\DefaultIcon" "" "$INSTDIR\bin\aqsis.exe,0"
    WriteRegStr HKCR "Aqsis.RIBGZ\shell\open" "" "${PRODUCT_SHELLEXT_RIB}"
    WriteRegStr HKCR "Aqsis.RIBGZ\shell\open\command" "" '"$INSTDIR\bin\aqsis.exe" -progress "%1"'
    
    WriteRegStr HKCR ".sl" "" "Aqsis.SL"
    WriteRegStr HKCR "Aqsis.SL" "" "${PRODUCT_SHELLEXT_SL_INFO}"
    WriteRegStr HKCR "Aqsis.SL\DefaultIcon" "" "$INSTDIR\bin\aqsl.exe,1"
    WriteRegStr HKCR "Aqsis.SL\shell\open" "" "${PRODUCT_SHELLEXT_SL}"
    WriteRegStr HKCR "Aqsis.SL\shell\open\command" "" '"$INSTDIR\bin\aqsl.exe" "%1"'
    
    WriteRegStr HKCR ".slx" "" "Aqsis.SLX"
    WriteRegStr HKCR "Aqsis.SLX" "" "${PRODUCT_SHELLEXT_SLX_INFO}"
    WriteRegStr HKCR "Aqsis.SLX\DefaultIcon" "" "$INSTDIR\bin\aqsl.exe,1"
    file_end:
SectionEnd


Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\bin\aqsis.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\bin\aqsis.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "HelpLink" "${PRODUCT_WEB_SUPPORT}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLUpdateInfo" "${PRODUCT_WEB_UPDATE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Comments" "$INSTDIR"
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoRepair" 1
SectionEnd


Function .onInit
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT_AS "${PROJECT_ROOT}\distribution\win\nsis\page_tasks.ini" "page_tasks.ini"
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
  Delete "$DESKTOP\${PRODUCT_FULLNAME} ${PRODUCT_VERSION}.lnk"
  Delete "$QUICKLAUNCH\${PRODUCT_FULLNAME}.lnk"

  RMDir /r "$SMPROGRAMS\$ICONS_GROUP"
  RMDir "$SMPROGRAMS\${PRODUCT_FULLNAME}"

  RMDir /r "$INSTDIR"

  ReadRegStr $PATH_NT HKCU "Environment" "PATH"
  ${un.WordReplace} "$PATH_NT" ";$INSTDIR\bin" "" "+" $PATH
  WriteRegStr HKCU "Environment" "PATH" "$PATH"

  ReadRegStr $PATH_NT_ALL HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
  ${un.WordReplace} "$PATH_NT_ALL" ";$INSTDIR\bin" "" "+" $PATH
  WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "$PATH"

  DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "AQSISHOME"

  DeleteRegKey HKCR ".rib"
  DeleteRegKey HKCR "Aqsis.RIB"
  DeleteRegKey HKCR ".ribz"
  DeleteRegKey HKCR "Aqsis.RIBGZ"
  DeleteRegKey HKCR ".sl"
  DeleteRegKey HKCR "Aqsis.SL"
  DeleteRegKey HKCR ".slx"
  DeleteRegKey HKCR "Aqsis.SLX"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

  SetAutoClose true
SectionEnd
