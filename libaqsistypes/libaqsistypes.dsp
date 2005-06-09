# Microsoft Developer Studio Project File - Name="libaqsistypes" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=LIBAQSISTYPES - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libaqsistypes.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libaqsistypes.mak" CFG="LIBAQSISTYPES - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libaqsistypes - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libaqsistypes - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "libaqsistypes - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "libaqsistypes - Win32 Release_Static_MT" (based on "Win32 (x86) Static Library")
!MESSAGE "libaqsistypes - Win32 Debug_Static_MT" (based on "Win32 (x86) Static Library")
!MESSAGE "libaqsistypes - Win32 Release_Static_ST" (based on "Win32 (x86) Static Library")
!MESSAGE "libaqsistypes - Win32 Debug_Static_ST" (based on "Win32 (x86) Static Library")
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
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\Object\Release\libaqsistypes"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /I "..\..\win32libs\include" /D "NDEBUG" /D _qBUILDING=CORE /D CORE=1 /D "PLUGINS" /D "_MBCS" /D "_LIB" /D "WIN32" /D "NO_SYSLOG" /D "USE_TIMERS" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\aqsistypes.lib"

!ELSEIF  "$(CFG)" == "libaqsistypes - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\Object\Debug\libaqsistypes"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /I "..\..\win32libs\include" /D "_DEBUG" /D _qBUILDING=CORE /D CORE=1 /D "PLUGINS" /D "_MBCS" /D "_LIB" /D "WIN32" /D "NO_SYSLOG" /D "USE_TIMERS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\aqsistypes_d.lib"

!ELSEIF  "$(CFG)" == "libaqsistypes - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Profile"
# PROP BASE Intermediate_Dir "Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Profile"
# PROP Intermediate_Dir "..\Object\Profile\libaqsistypes"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /I "..\..\win32libs\include" /D "NDEBUG" /D _qBUILDING=CORE /D CORE=1 /D "PLUGINS" /D "_MBCS" /D "_LIB" /D "WIN32" /D "NO_SYSLOG" /D "USE_TIMERS" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Profile\aqsistypes.lib"

!ELSEIF  "$(CFG)" == "libaqsistypes - Win32 Release_Static_MT"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libaqsistypes___Win32_Release_Static_MT"
# PROP BASE Intermediate_Dir "libaqsistypes___Win32_Release_Static_MT"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Static_MT"
# PROP Intermediate_Dir "..\Object\Release_Static_MT\libaqsistype"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /I "..\..\win32libs\include" /D "PLUGINS" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /I "..\..\win32libs\include" /D "NDEBUG" /D _qBUILDING=CORE /D CORE=1 /D "PLUGINS" /D "_MBCS" /D "_LIB" /D "WIN32" /D "NO_SYSLOG" /D "USE_TIMERS" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Static_MT\aqsistypes.lib"

!ELSEIF  "$(CFG)" == "libaqsistypes - Win32 Debug_Static_MT"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libaqsistypes___Win32_Debug_Static_MT"
# PROP BASE Intermediate_Dir "libaqsistypes___Win32_Debug_Static_MT"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\Static_MT"
# PROP Intermediate_Dir "..\Object\Debug_Static_MT\libaqsistypes"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /I "..\..\win32libs\include" /D "_DEBUG" /D "PLUGINS" /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /I "..\..\win32libs\include" /D "_DEBUG" /D _qBUILDING=CORE /D CORE=1 /D "PLUGINS" /D "_MBCS" /D "_LIB" /D "WIN32" /D "NO_SYSLOG" /D "USE_TIMERS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Static_MT\aqsistypes_d.lib"

!ELSEIF  "$(CFG)" == "libaqsistypes - Win32 Release_Static_ST"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libaqsistypes___Win32_Release_Static_ST"
# PROP BASE Intermediate_Dir "libaqsistypes___Win32_Release_Static_ST"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Static_ST"
# PROP Intermediate_Dir "..\Object\Release_Static_ST\libaqsistypes"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /I "..\..\win32libs\include" /D "PLUGINS" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /I "..\..\win32libs\include" /D "NDEBUG" /D _qBUILDING=CORE /D CORE=1 /D "PLUGINS" /D "_MBCS" /D "_LIB" /D "WIN32" /D "NO_SYSLOG" /D "USE_TIMERS" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Static_ST\aqsistypes.lib"

!ELSEIF  "$(CFG)" == "libaqsistypes - Win32 Debug_Static_ST"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libaqsistypes___Win32_Debug_Static_ST"
# PROP BASE Intermediate_Dir "libaqsistypes___Win32_Debug_Static_ST"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\Static_ST"
# PROP Intermediate_Dir "..\Object\Debug_Static_ST\libaqsistypes"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /I "..\..\win32libs\include" /D "_DEBUG" /D "PLUGINS" /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /D "WIN32" /D "NO_SYSLOG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I ".\win32\intel" /I "..\libaqsis" /I ".\\" /I "..\..\win32libs\include" /D "_DEBUG" /D _qBUILDING=CORE /D CORE=1 /D "PLUGINS" /D "_MBCS" /D "_LIB" /D "WIN32" /D "NO_SYSLOG" /D "USE_TIMERS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Static_ST\aqsistypes_d.lib"

!ENDIF 

# Begin Target

# Name "libaqsistypes - Win32 Release"
# Name "libaqsistypes - Win32 Debug"
# Name "libaqsistypes - Win32 Profile"
# Name "libaqsistypes - Win32 Release_Static_MT"
# Name "libaqsistypes - Win32 Debug_Static_MT"
# Name "libaqsistypes - Win32 Release_Static_ST"
# Name "libaqsistypes - Win32 Debug_Static_ST"
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

SOURCE=.\file.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\intel\file_system.cpp
# End Source File
# Begin Source File

SOURCE=.\logging.cpp
# End Source File
# Begin Source File

SOURCE=.\matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\noise.cpp
# End Source File
# Begin Source File

SOURCE=.\plugins.cpp
# End Source File
# Begin Source File

SOURCE=.\random.cpp
# End Source File
# Begin Source File

SOURCE=.\refcount.cpp
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

SOURCE=.\exception.h
# End Source File
# Begin Source File

SOURCE=.\file.h
# End Source File
# Begin Source File

SOURCE=.\list.h
# End Source File
# Begin Source File

SOURCE=.\logging.h
# End Source File
# Begin Source File

SOURCE=.\logging_streambufs.h
# End Source File
# Begin Source File

SOURCE=.\matrix.h
# End Source File
# Begin Source File

SOURCE=.\memorypool.h
# End Source File
# Begin Source File

SOURCE=.\MultiTimer.h
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
