# Microsoft Developer Studio Project File - Name="libaqsistypes" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libaqsistypes - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "libaqsistypes.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "libaqsistypes.mak" CFG="libaqsistypes - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "libaqsistypes - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "libaqsistypes - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "libaqsistypes - Win32 Profile" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libaqsistypes - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Library\Release"
# PROP Intermediate_Dir "..\Object\Release\libaqsistypes"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /D "_MBCS" /D "_LIB" /D "PLUGINS" /D "NDEBUG" /D "WIN32" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libaqsistypes - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Library\Debug"
# PROP Intermediate_Dir "..\Object\Debug\libaqsistypes"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "PLUGINS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libaqsistypes - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Profile"
# PROP BASE Intermediate_Dir "Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Library\Profile"
# PROP Intermediate_Dir "..\Object\Profile\libaqsistypes"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "PLUGINS" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libaqsistypes - Win32 Release"
# Name "libaqsistypes - Win32 Debug"
# Name "libaqsistypes - Win32 Profile"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bitvector.cpp
# End Source File
# Begin Source File

SOURCE=.\cellnoise.cpp
# End Source File
# Begin Source File

SOURCE=.\color.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\intel\criticalsection.cpp
# End Source File
# Begin Source File

SOURCE=.\file.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\intel\file_system.cpp
# End Source File
# Begin Source File

SOURCE=.\log.cpp
# End Source File
# Begin Source File

SOURCE=.\matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\mtable.cpp
# End Source File
# Begin Source File

SOURCE=.\noise.cpp
# End Source File
# Begin Source File

SOURCE=.\plugins.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\intel\semaphore.cpp
# End Source File
# Begin Source File

SOURCE=.\spline.cpp
# End Source File
# Begin Source File

SOURCE=.\sstring.cpp
# End Source File
# Begin Source File

SOURCE=.\vector2d.cpp
# End Source File
# Begin Source File

SOURCE=.\vector3d.cpp
# End Source File
# Begin Source File

SOURCE=.\vector4d.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\aqsis.h
# End Source File
# Begin Source File

SOURCE=.\win32\intel\aqsis_compiler.h
# End Source File
# Begin Source File

SOURCE=.\aqsis_types.h
# End Source File
# Begin Source File

SOURCE=.\bitvector.h
# End Source File
# Begin Source File

SOURCE=.\cellnoise.h
# End Source File
# Begin Source File

SOURCE=.\color.h
# End Source File
# Begin Source File

SOURCE=.\win32\intel\criticalsection.h
# End Source File
# Begin Source File

SOURCE=.\exception.h
# End Source File
# Begin Source File

SOURCE=.\file.h
# End Source File
# Begin Source File

SOURCE=.\ilog.h
# End Source File
# Begin Source File

SOURCE=.\imtable.h
# End Source File
# Begin Source File

SOURCE=.\list.h
# End Source File
# Begin Source File

SOURCE=.\log.h
# End Source File
# Begin Source File

SOURCE=.\matrix.h
# End Source File
# Begin Source File

SOURCE=.\memorypool.h
# End Source File
# Begin Source File

SOURCE=.\mtable.h
# End Source File
# Begin Source File

SOURCE=.\noise.h
# End Source File
# Begin Source File

SOURCE=.\plugins.h
# End Source File
# Begin Source File

SOURCE=.\random.h
# End Source File
# Begin Source File

SOURCE=.\refcount.h
# End Source File
# Begin Source File

SOURCE=.\win32\intel\semaphore.h
# End Source File
# Begin Source File

SOURCE=.\win32\intel\share.h
# End Source File
# Begin Source File

SOURCE=.\spline.h
# End Source File
# Begin Source File

SOURCE=.\sstring.h
# End Source File
# Begin Source File

SOURCE=.\validate.h
# End Source File
# Begin Source File

SOURCE=.\vector2d.h
# End Source File
# Begin Source File

SOURCE=.\vector3d.h
# End Source File
# Begin Source File

SOURCE=.\vector4d.h
# End Source File
# End Group
# End Target
# End Project
