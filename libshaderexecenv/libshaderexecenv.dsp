# Microsoft Developer Studio Project File - Name="libshaderexecenv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libshaderexecenv - Win32 Profile
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libshaderexecenv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libshaderexecenv.mak" CFG="libshaderexecenv - Win32 Profile"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libshaderexecenv - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libshaderexecenv - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "libshaderexecenv - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "libshaderexecenv - Win32 Release_Static_MT" (based on "Win32 (x86) Static Library")
!MESSAGE "libshaderexecenv - Win32 Debug_Static_MT" (based on "Win32 (x86) Static Library")
!MESSAGE "libshaderexecenv - Win32 Release_Static_ST" (based on "Win32 (x86) Static Library")
!MESSAGE "libshaderexecenv - Win32 Debug_Static_ST" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libshaderexecenv - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\Object\Release\libshaderexecenv"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /I "..\libshadervm" /I "..\..\win32libs\include" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSHADEREXECENV /D BUILD_LIBSHADEREXECENV=1 /D "NDEBUG" /D "PLUGINS" /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\shaderexecenv.lib"

!ELSEIF  "$(CFG)" == "libshaderexecenv - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\Object\Debug\libshaderexecenv"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /I "..\libshadervm" /I "..\..\win32libs\include" /D "_DEBUG" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSHADEREXECENV /D BUILD_LIBSHADEREXECENV=1 /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\shaderexecenv_d.lib"

!ELSEIF  "$(CFG)" == "libshaderexecenv - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libshaderexecenv___Win32_Profile"
# PROP BASE Intermediate_Dir "libshaderexecenv___Win32_Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Profile"
# PROP Intermediate_Dir "..\Object\Profile\libshaderexecenv"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /I "..\libshadervm" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSHADEREXECENV /D BUILD_LIBSHADEREXECENV=1 /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /I "..\libshadervm" /I "..\..\win32libs\include" /D "NDEBUG" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSHADEREXECENV /D BUILD_LIBSHADEREXECENV=1 /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Profile\shaderexecenv.lib"

!ELSEIF  "$(CFG)" == "libshaderexecenv - Win32 Release_Static_MT"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libshaderexecenv___Win32_Release_Static_MT"
# PROP BASE Intermediate_Dir "libshaderexecenv___Win32_Release_Static_MT"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Static_MT"
# PROP Intermediate_Dir "..\Object\Release_Static_MT\libshaderexecenv"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /I "..\libshadervm" /I "..\..\win32libs\include" /I "..\boost" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSHADEREXECENV /D BUILD_LIBSHADEREXECENV=1 /D "NDEBUG" /D "PLUGINS" /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /I "..\libshadervm" /I "..\..\win32libs\include" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSHADEREXECENV /D BUILD_LIBSHADEREXECENV=1 /D "NDEBUG" /D "PLUGINS" /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Static_MT\shaderexecenv.lib"

!ELSEIF  "$(CFG)" == "libshaderexecenv - Win32 Debug_Static_MT"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libshaderexecenv___Win32_Debug_Static_MT"
# PROP BASE Intermediate_Dir "libshaderexecenv___Win32_Debug_Static_MT"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\Static_MT"
# PROP Intermediate_Dir "..\Object\Debug_Static_MT\libshaderexecenv"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /I "..\libshadervm" /I "..\..\win32libs\include" /I "..\boost" /D "_DEBUG" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSHADEREXECENV /D BUILD_LIBSHADEREXECENV=1 /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /I "..\libshadervm" /I "..\..\win32libs\include" /D "_DEBUG" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSHADEREXECENV /D BUILD_LIBSHADEREXECENV=1 /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Static_MT\shaderexecenv_d.lib"

!ELSEIF  "$(CFG)" == "libshaderexecenv - Win32 Release_Static_ST"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libshaderexecenv___Win32_Release_Static_ST"
# PROP BASE Intermediate_Dir "libshaderexecenv___Win32_Release_Static_ST"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Static_ST"
# PROP Intermediate_Dir "..\Object\Release_Static_ST\libshaderexecenv"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /I "..\libshadervm" /I "..\..\win32libs\include" /I "..\boost" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSHADEREXECENV /D BUILD_LIBSHADEREXECENV=1 /D "NDEBUG" /D "PLUGINS" /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /I "..\libshadervm" /I "..\..\win32libs\include" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSHADEREXECENV /D BUILD_LIBSHADEREXECENV=1 /D "NDEBUG" /D "PLUGINS" /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Static_ST\shaderexecenv.lib"

!ELSEIF  "$(CFG)" == "libshaderexecenv - Win32 Debug_Static_ST"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libshaderexecenv___Win32_Debug_Static_ST"
# PROP BASE Intermediate_Dir "libshaderexecenv___Win32_Debug_Static_ST"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\Static_ST"
# PROP Intermediate_Dir "..\Object\Debug_Static_ST\libshaderexecenv"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /I "..\libshadervm" /I "..\..\win32libs\include" /I "..\boost" /D "_DEBUG" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSHADEREXECENV /D BUILD_LIBSHADEREXECENV=1 /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /I "..\libshadervm" /I "..\..\win32libs\include" /D "_DEBUG" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSHADEREXECENV /D BUILD_LIBSHADEREXECENV=1 /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Static_ST\shaderexecenv_d.lib"

!ENDIF 

# Begin Target

# Name "libshaderexecenv - Win32 Release"
# Name "libshaderexecenv - Win32 Debug"
# Name "libshaderexecenv - Win32 Profile"
# Name "libshaderexecenv - Win32 Release_Static_MT"
# Name "libshaderexecenv - Win32 Debug_Static_MT"
# Name "libshaderexecenv - Win32 Release_Static_ST"
# Name "libshaderexecenv - Win32 Debug_Static_ST"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\shadeops.cpp
# End Source File
# Begin Source File

SOURCE=.\shaderexecenv.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\shaderexecenv.h
# End Source File
# End Group
# End Target
# End Project
