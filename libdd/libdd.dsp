# Microsoft Developer Studio Project File - Name="libdd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libdd - Win32 Profile
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libdd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libdd.mak" CFG="libdd - Win32 Profile"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libdd - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libdd - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "libdd - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "libdd - Win32 Release_Static_MT" (based on "Win32 (x86) Static Library")
!MESSAGE "libdd - Win32 Debug_Static_MT" (based on "Win32 (x86) Static Library")
!MESSAGE "libdd - Win32 Release_Static_ST" (based on "Win32 (x86) Static Library")
!MESSAGE "libdd - Win32 Debug_Static_ST" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "libdd"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libdd - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\Object\Release\libdd"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libddmsock" /I "..\..\win32libs\include" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDD /D BUILD_LIBDD=1 /D "NDEBUG" /D "PLUGINS" /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /c
# SUBTRACT CPP /u
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libdd - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\Object\Debug\libdd"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libddmsock" /I "..\..\win32libs\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDD /D BUILD_LIBDD=1 /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /GZ /c
# SUBTRACT CPP /u
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\libdd_d.lib"

!ELSEIF  "$(CFG)" == "libdd - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libdd___Win32_Profile"
# PROP BASE Intermediate_Dir "libdd___Win32_Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Profile"
# PROP Intermediate_Dir "..\Object\Profile\libdd"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libddmsock" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDD /D BUILD_LIBDD=1 /FR /YX /FD /c
# SUBTRACT BASE CPP /u
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libddmsock" /I "..\..\win32libs\include" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDD /D BUILD_LIBDD=1 /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /c
# SUBTRACT CPP /u
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libdd - Win32 Release_Static_MT"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libdd___Win32_Release_Static_MT"
# PROP BASE Intermediate_Dir "libdd___Win32_Release_Static_MT"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Static_MT"
# PROP Intermediate_Dir "..\Object\Release_Static_MT\libdd"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libddmsock" /I "..\..\win32libs\include" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDD /D BUILD_LIBDD=1 /D "NDEBUG" /D "PLUGINS" /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /c
# SUBTRACT BASE CPP /u
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libddmsock" /I "..\..\win32libs\include" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDD /D BUILD_LIBDD=1 /D "NDEBUG" /D "PLUGINS" /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /c
# SUBTRACT CPP /u
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libdd - Win32 Debug_Static_MT"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libdd___Win32_Debug_Static_MT"
# PROP BASE Intermediate_Dir "libdd___Win32_Debug_Static_MT"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\Static_MT"
# PROP Intermediate_Dir "..\Object\Debug_Static_MT\libdd"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libddmsock" /I "..\..\win32libs\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDD /D BUILD_LIBDD=1 /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /GZ /c
# SUBTRACT BASE CPP /u
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libddmsock" /I "..\..\win32libs\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDD /D BUILD_LIBDD=1 /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /GZ /c
# SUBTRACT CPP /u
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Static_MT\libdd_d.lib"

!ELSEIF  "$(CFG)" == "libdd - Win32 Release_Static_ST"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libdd___Win32_Release_Static_ST"
# PROP BASE Intermediate_Dir "libdd___Win32_Release_Static_ST"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Static_ST"
# PROP Intermediate_Dir "..\Object\Release_Static_ST\libdd"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libddmsock" /I "..\..\win32libs\include" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDD /D BUILD_LIBDD=1 /D "NDEBUG" /D "PLUGINS" /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /c
# SUBTRACT BASE CPP /u
# ADD CPP /nologo /W3 /GR /GX /O2 /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libddmsock" /I "..\..\win32libs\include" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDD /D BUILD_LIBDD=1 /D "NDEBUG" /D "PLUGINS" /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /c
# SUBTRACT CPP /u
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libdd - Win32 Debug_Static_ST"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libdd___Win32_Debug_Static_ST"
# PROP BASE Intermediate_Dir "libdd___Win32_Debug_Static_ST"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\Static_ST"
# PROP Intermediate_Dir "..\Object\Debug_Static_ST\libdd"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libddmsock" /I "..\..\win32libs\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDD /D BUILD_LIBDD=1 /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /GZ /c
# SUBTRACT BASE CPP /u
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libddmsock" /I "..\..\win32libs\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDD /D BUILD_LIBDD=1 /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /GZ /c
# SUBTRACT CPP /u
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Static_ST\libdd_d.lib"

!ENDIF 

# Begin Target

# Name "libdd - Win32 Release"
# Name "libdd - Win32 Debug"
# Name "libdd - Win32 Profile"
# Name "libdd - Win32 Release_Static_MT"
# Name "libdd - Win32 Debug_Static_MT"
# Name "libdd - Win32 Release_Static_ST"
# Name "libdd - Win32 Debug_Static_ST"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\dd.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\dd.h
# End Source File
# End Group
# End Target
# End Project
