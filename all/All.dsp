# Microsoft Developer Studio Project File - Name="All" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=All - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "All.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "All.mak" CFG="All - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "All - Win32 Release" (basierend auf  "Win32 (x86) Generic Project")
!MESSAGE "All - Win32 Debug" (basierend auf  "Win32 (x86) Generic Project")
!MESSAGE "All - Win32 Profile" (basierend auf  "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "All"
# PROP Scc_LocalPath "."
MTL=midl.exe

!IF  "$(CFG)" == "All - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Target_Dir ""
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Preparing installation
PostBuild_Cmds=REM call ..\prep_install.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "All - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "All - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "All___Win32_Profile"
# PROP BASE Intermediate_Dir "All___Win32_Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Target_Dir ""
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Preparing installation
PostBuild_Cmds=REM call ..\prep_install.bat
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "All - Win32 Release"
# Name "All - Win32 Debug"
# Name "All - Win32 Profile"
# End Target
# End Project
