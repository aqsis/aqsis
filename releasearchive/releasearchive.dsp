# Microsoft Developer Studio Project File - Name="releasearchive" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=releasearchive - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "releasearchive.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "releasearchive.mak" CFG="releasearchive - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "releasearchive - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "releasearchive - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "releasearchive - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "releasearchive - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "releasearchive - Win32 Release"
# Name "releasearchive - Win32 Debug"
# Begin Source File

SOURCE=..\prep_install.bat

!IF  "$(CFG)" == "releasearchive - Win32 Release"

# PROP Intermediate_Dir "..\Release\InstallArchive"
# Begin Custom Build - Generating Release Archive
OutDir=.\Release
InputPath=..\prep_install.bat

"$(OutDir)\Aqsis_Win32.zip" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	call $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "releasearchive - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
