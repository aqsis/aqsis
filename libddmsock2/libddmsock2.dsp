# Microsoft Developer Studio Project File - Name="libddmsock2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libddmsock2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libddmsock2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libddmsock2.mak" CFG="libddmsock2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libddmsock2 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libddmsock2 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libddmsock2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\Object\Release\libddmsock2"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libshaderexecenv" /I "..\libshadervm" /I "..\..\win32libs\include" /I "..\boost" /I "..\thirdparty\tinyxml" /D "NDEBUG" /D "PLUGINS" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDDMSOCK /D BUILD_LIBDDMSOCK=1 /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /D "TIXML_USE_STL" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libddmsock2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\Object\Debug\libddmsock2"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\libshaderexecenv" /I "..\libshadervm" /I "..\..\win32libs\include" /I "..\boost" /I "..\thirdparty\tinyxml" /D "_DEBUG" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBDDMSOCK /D BUILD_LIBDDMSOCK=1 /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /D "TIXML_USE_STL" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\libddmsock2_d.lib"

!ENDIF 

# Begin Target

# Name "libddmsock2 - Win32 Release"
# Name "libddmsock2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bucketcache.cpp
# End Source File
# Begin Source File

SOURCE=.\ddmsock2.cpp
# End Source File
# Begin Source File

SOURCE=.\listener.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\bucketcache.h
# End Source File
# Begin Source File

SOURCE=.\ddmsock2.h
# End Source File
# Begin Source File

SOURCE=.\displaydriver.h
# End Source File
# Begin Source File

SOURCE=.\listener.h
# End Source File
# End Group
# End Target
# End Project
