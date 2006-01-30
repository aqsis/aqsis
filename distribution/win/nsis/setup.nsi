; Title: Aqsis 'Standard' Installer for Win32/64 (NSIS)
; Author: Leon Tony Atkinson
; Info: Last tested with NSIS 2.14
; Other: To make updates easier, all message strings have been placed within the top 20-30 lines of this file.


; Helper defines
!define /date YEAR "%Y"

!define PRODUCT_NAME "Aqsis"
!define PRODUCT_FULLNAME "Aqsis Renderer"
; !define PRODUCT_VERSION "1.2.0"
!ifndef PRODUCT_VERSION
	!error "PRODUCT_VERSION not specified"
!endif
; !define PRODUCT_FILE_NUMBER "1_2_0"
!ifndef	PRODUCT_FILE_NUMBER
	!error "PRODUCT_FILE_NUMBER not specified"
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
OutFile "..\..\..\aqsis-setup-${PRODUCT_FILE_NUMBER}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show
SetCompressor lzma
CRCCheck on
XPStyle on


; Pages
!include "MUI.nsh"
!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING
;;!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp"
;;!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp"
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME

; License page
!define MUI_LICENSEPAGE_RADIOBUTTONS
!insertmacro MUI_PAGE_LICENSE "..\..\..\COPYING"

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
ReserveFile "page_tasks.ini"


; Installer 'Version' tab content
VIProductVersion "${PRODUCT_VERSION}.0"
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
  File "..\..\..\output\bin\aqsisrc"
  File "..\..\..\output\bin\*.dll"
  File "..\..\..\output\bin\*.exe"
  SetOutPath "$INSTDIR\doc"
  File "/oname=AUTHORS.txt" "..\..\..\AUTHORS"
  File "/oname=LICENSE.txt" "..\..\..\COPYING"
  File "/oname=README.txt" "..\..\..\README"
  SetOutPath "$INSTDIR\shaders"
  File "..\..\..\output\shaders\*.slx"

  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "searchpath" "string shader" ' '["$INSTDIR\shaders"]' $R0
  ${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "searchpath" "string display" ' '["$INSTDIR\bin"]' $R1

; Shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP\Documentation"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Documentation\Readme.lnk" "$INSTDIR\doc\README.txt"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\${PRODUCT_NAME}.lnk" "$SYSDIR\cmd.exe" '/k "$INSTDIR\bin\aqsis.exe" -h'
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Credits.lnk" "$INSTDIR\doc\AUTHORS.txt"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\License.lnk" "$INSTDIR\doc\LICENSE.txt"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

SectionGroup /e "Content" SEC02
  Section "Example" SEC0201
  SectionIn 1 2
    SetOutPath "$INSTDIR\examples\scenes\vase"
    File "..\..\..\content\ribs\scenes\vase\*.*"

; Shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP\Examples\Scenes"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Examples\Scenes\Vase.lnk" "$SYSDIR\cmd.exe" '/c ""$INSTDIR\bin\aqsis.exe" -progress "$INSTDIR\examples\scenes\vase\vase.rib""'
  !insertmacro MUI_STARTMENU_WRITE_END
  SectionEnd

  Section /o "Source Shaders" SEC0202
  SectionIn 1
    SetOutPath "$INSTDIR\shaders"
    File "..\..\..\output\shaders\*.sl"
  SectionEnd
SectionGroupEnd

Section "Documentation" SEC03
SectionIn 1 2
  SetOutPath "$INSTDIR\doc"
  ;;File "/oname=CHANGES.txt" "..\..\..\output\ChangeLog"
  File "/oname=INSTALL.txt" "..\..\..\INSTALL"
  File "/oname=NOTES.txt" "..\..\..\ReleaseNotes"

; Shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  ;;CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Documentation\What's New.lnk" "$INSTDIR\doc\CHANGES.txt"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section /o "Other" SEC04
SectionIn 1
  SetOutPath "$INSTDIR\include"
  File "..\..\..\output\include\*.h"
  SetOutPath "$INSTDIR\lib"
  File "..\..\..\output\lib\*.lib"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "Rendering application and essential files only"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "Example and shader source files"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC0201} "Example file (.rib)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC0202} "Generic shader source files (.sl)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} "User guides and other information"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC04} "Include and library files"
!insertmacro MUI_FUNCTION_DESCRIPTION_END


Section -AdditionalIcons
  SetOutPath $INSTDIR
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  WriteIniStr "$INSTDIR\doc\website.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Documentation\${PRODUCT_NAME} Website.lnk" "$INSTDIR\doc\website.url"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Uninstall ${PRODUCT_NAME}.lnk" "$INSTDIR\uninst.exe"
  CreateShortCut "$INSTDIR\LICENSE.lnk" "$INSTDIR\doc\LICENSE.txt"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd


Section -AdditionalTasks
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
    WriteRegStr HKCU "Environment" "PATH" "$INSTDIR\bin;$PATH"
    path_nt_end:

  ; Update 'PATH' for all users
  !insertmacro MUI_INSTALLOPTIONS_READ $PATH_NT_ALL "page_tasks.ini" "Field 2" "State"
  StrCmp $PATH_NT_ALL "1" "path_nt_all" "path_nt_all_end"
    path_nt_all:
    ReadRegStr $PATH HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
    WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "$INSTDIR\bin;$PATH"
    path_nt_all_end:

  ; Create 'Desktop' icon
  !insertmacro MUI_INSTALLOPTIONS_READ $DESKTOP_ICON "page_tasks.ini" "Field 3" "State"
  StrCmp $DESKTOP_ICON "1" "desktop" "desktop_end"
    desktop:
    CreateShortCut "$DESKTOP\${PRODUCT_FULLNAME} ${PRODUCT_VERSION}.lnk" "$SYSDIR\cmd.exe" '/k "$INSTDIR\bin\aqsis.exe" -h'
    desktop_end:

  ; Create 'Quick Launch' icon
  !insertmacro MUI_INSTALLOPTIONS_READ $QUICKLAUCH_ICON "page_tasks.ini" "Field 4" "State"
  StrCmp $QUICKLAUCH_ICON "1" "quicklaunch" "quicklaunch_end"
    quicklaunch:
    CreateShortCut "$QUICKLAUNCH\${PRODUCT_FULLNAME}.lnk" "$SYSDIR\cmd.exe" '/k "$INSTDIR\bin\aqsis.exe" -h'
    quicklaunch_end:

  ; Create file association(s)
  !insertmacro MUI_INSTALLOPTIONS_READ $FILE_EXTENSION "page_tasks.ini" "Field 5" "State"
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
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "page_tasks.ini"
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


Section Uninstall
  !insertmacro MUI_STARTMENU_GETFOLDER "Application" $ICONS_GROUP
  Delete "$DESKTOP\${PRODUCT_FULLNAME} ${PRODUCT_VERSION}.lnk"
  Delete "$QUICKLAUNCH\${PRODUCT_FULLNAME}.lnk"

  RMDir /r "$SMPROGRAMS\$ICONS_GROUP"
  RMDir "$SMPROGRAMS\${PRODUCT_FULLNAME}"

  RMDir /r "$INSTDIR"

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
