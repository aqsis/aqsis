# Microsoft Developer Studio Project File - Name="libaqsistypes" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=libaqsistypes - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libaqsistypes.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libaqsistypes.mak" CFG="libaqsistypes - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libaqsistypes - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libaqsistypes - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
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
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBAQSISTYPES_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I ".\win32\intel" /I ".\\" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "PLUGINS" /D "AQSIS_DYNAMIC_LINK" /D "WIN32" /D _qBUILDING=BUILD_LIBAQSISTYPES /D BUILD_LIBAQSISTYPES=1 /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 log4cpp.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

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
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBAQSISTYPES_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".\win32\intel" /I ".\\" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "PLUGINS" /D "AQSIS_DYNAMIC_LINK" /D "WIN32" /D _qBUILDING=BUILD_LIBAQSISTYPES /D BUILD_LIBAQSISTYPES=1 /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 log4cpp.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "libaqsistypes - Win32 Release"
# Name "libaqsistypes - Win32 Debug"
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

SOURCE=.\vector2d.h
# End Source File
# Begin Source File

SOURCE=.\vector3d.h
# End Source File
# Begin Source File

SOURCE=.\vector4d.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
