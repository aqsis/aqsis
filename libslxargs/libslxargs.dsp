# Microsoft Developer Studio Project File - Name="libslxargs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libslxargs - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libslxargs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libslxargs.mak" CFG="libslxargs - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libslxargs - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libslxargs - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "libslxargs - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "libslxargs - Win32 MPatrol" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libslxargs - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Library\Release"
# PROP Intermediate_Dir "..\Object\Release\libslxargs"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\render" /I "..\librib2ri" /I "..\librib2" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libshadervm" /I "..\libshaderexecenv" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSLXARGS /D BUILD_LIBSLXARGS=1 /D "NDEBUG" /D "WIN32" /D "PLUGINS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libslxargs - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Library\Debug"
# PROP Intermediate_Dir "..\Object\Debug\libslxargs"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\render" /I "..\librib2ri" /I "..\librib2" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libshadervm" /I "..\libshaderexecenv" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSLXARGS /D BUILD_LIBSLXARGS=1 /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libslxargs - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libslxargs___Win32_Profile"
# PROP BASE Intermediate_Dir "libslxargs___Win32_Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Library\Profile"
# PROP Intermediate_Dir "..\Object\Profile\libslxargs"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\render" /I "..\librib2ri" /I "..\librib2" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libshadervm" /I "..\libshaderexecenv" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSLXARGS /D BUILD_LIBSLXARGS=1 /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "..\render" /I "..\librib2ri" /I "..\librib2" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libshadervm" /I "..\libshaderexecenv" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSLXARGS /D BUILD_LIBSLXARGS=1 /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libslxargs - Win32 MPatrol"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libslxargs___Win32_MPatrol"
# PROP BASE Intermediate_Dir "libslxargs___Win32_MPatrol"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Library\MPatrol"
# PROP Intermediate_Dir "..\Object\MPatrol\libslxargs"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "..\render" /I "..\librib2ri" /I "..\librib2" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libshadervm" /I "..\libshaderexecenv" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSLXARGS /D BUILD_LIBSLXARGS=1 /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "..\render" /I "..\librib2ri" /I "..\librib2" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libshadervm" /I "..\libshaderexecenv" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBSLXARGS /D BUILD_LIBSLXARGS=1 /FD /c
# SUBTRACT CPP /YX
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

# Name "libslxargs - Win32 Release"
# Name "libslxargs - Win32 Debug"
# Name "libslxargs - Win32 Profile"
# Name "libslxargs - Win32 MPatrol"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\rcdummy.cpp
# End Source File
# Begin Source File

SOURCE=.\slx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\slx.h
# End Source File
# End Group
# End Target
# End Project
