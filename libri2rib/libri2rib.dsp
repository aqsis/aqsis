# Microsoft Developer Studio Project File - Name="libri2rib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=LIBRI2RIB - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libri2rib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libri2rib.mak" CFG="LIBRI2RIB - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libri2rib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libri2rib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "libri2rib - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "libri2rib - Win32 Release_Static_MT" (based on "Win32 (x86) Static Library")
!MESSAGE "libri2rib - Win32 Debug_Static_MT" (based on "Win32 (x86) Static Library")
!MESSAGE "libri2rib - Win32 Release_Static_ST" (based on "Win32 (x86) Static Library")
!MESSAGE "libri2rib - Win32 Debug_Static_ST" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libri2rib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\Object\Release\libri2rib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\renderer" /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\..\win32libs\include" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRI2RIB /D BUILD_LIBRI2RIB=1 /D "NDEBUG" /D "PLUGINS" /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\ri2rib.lib"

!ELSEIF  "$(CFG)" == "libri2rib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\Object\Debug\libri2rib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\..\win32libs\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRI2RIB /D BUILD_LIBRI2RIB=1 /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\ri2rib_d.lib"

!ELSEIF  "$(CFG)" == "libri2rib - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libri2rib___Win32_Profile"
# PROP BASE Intermediate_Dir "libri2rib___Win32_Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Profile"
# PROP Intermediate_Dir "..\Object\Profile\libri2rib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\renderer" /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRI2RIB /D BUILD_LIBRI2RIB=1 /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "..\renderer" /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\..\win32libs\include" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRI2RIB /D BUILD_LIBRI2RIB=1 /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Profile\ri2rib.lib"

!ELSEIF  "$(CFG)" == "libri2rib - Win32 Release_Static_MT"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libri2rib___Win32_Release_Static_MT"
# PROP BASE Intermediate_Dir "libri2rib___Win32_Release_Static_MT"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Static_MT"
# PROP Intermediate_Dir "..\Object\Release_Static_MT\libri2rib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\renderer" /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\..\win32libs\include" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRI2RIB /D BUILD_LIBRI2RIB=1 /D "NDEBUG" /D "PLUGINS" /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\renderer" /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\..\win32libs\include" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRI2RIB /D BUILD_LIBRI2RIB=1 /D "NDEBUG" /D "PLUGINS" /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Static_MT\ri2rib.lib"

!ELSEIF  "$(CFG)" == "libri2rib - Win32 Debug_Static_MT"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libri2rib___Win32_Debug_Static_MT"
# PROP BASE Intermediate_Dir "libri2rib___Win32_Debug_Static_MT"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\Static_MT"
# PROP Intermediate_Dir "..\Object\Debug_Static_MT\libri2rib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\..\win32libs\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRI2RIB /D BUILD_LIBRI2RIB=1 /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\..\win32libs\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRI2RIB /D BUILD_LIBRI2RIB=1 /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Static_MT\ri2rib_d.lib"

!ELSEIF  "$(CFG)" == "libri2rib - Win32 Release_Static_ST"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libri2rib___Win32_Release_Static_ST"
# PROP BASE Intermediate_Dir "libri2rib___Win32_Release_Static_ST"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Static_ST"
# PROP Intermediate_Dir "..\Object\Release_Static_ST\libri2rib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\renderer" /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\..\win32libs\include" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRI2RIB /D BUILD_LIBRI2RIB=1 /D "NDEBUG" /D "PLUGINS" /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "..\renderer" /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\..\win32libs\include" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRI2RIB /D BUILD_LIBRI2RIB=1 /D "NDEBUG" /D "PLUGINS" /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Static_ST\ri2rib.lib"

!ELSEIF  "$(CFG)" == "libri2rib - Win32 Debug_Static_ST"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libri2rib___Win32_Debug_Static_ST"
# PROP BASE Intermediate_Dir "libri2rib___Win32_Debug_Static_ST"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\Static_ST"
# PROP Intermediate_Dir "..\Object\Debug_Static_ST\libri2rib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\..\win32libs\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRI2RIB /D BUILD_LIBRI2RIB=1 /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\..\win32libs\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRI2RIB /D BUILD_LIBRI2RIB=1 /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\Debug\libri2ribd.lib"
# ADD LIB32 /nologo /out:"..\lib\Static_ST\ri2rib_d.lib"

!ENDIF 

# Begin Target

# Name "libri2rib - Win32 Release"
# Name "libri2rib - Win32 Debug"
# Name "libri2rib - Win32 Profile"
# Name "libri2rib - Win32 Release_Static_MT"
# Name "libri2rib - Win32 Debug_Static_MT"
# Name "libri2rib - Win32 Release_Static_ST"
# Name "libri2rib - Win32 Debug_Static_ST"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ascii.cpp
# End Source File
# Begin Source File

SOURCE=.\binary.cpp
# End Source File
# Begin Source File

SOURCE=.\context.cpp
# End Source File
# Begin Source File

SOURCE=.\dictionary.cpp
# End Source File
# Begin Source File

SOURCE=.\error.cpp
# End Source File
# Begin Source File

SOURCE=.\inlineparse.cpp
# End Source File
# Begin Source File

SOURCE=.\output.cpp
# End Source File
# Begin Source File

SOURCE=.\outstream.cpp
# End Source File
# Begin Source File

SOURCE=.\plstore.cpp
# End Source File
# Begin Source File

SOURCE=.\ri.cpp
# End Source File
# Begin Source File

SOURCE=.\tokens.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ascii.h
# End Source File
# Begin Source File

SOURCE=.\binary.h
# End Source File
# Begin Source File

SOURCE=.\context.h
# End Source File
# Begin Source File

SOURCE=.\dictionary.h
# End Source File
# Begin Source File

SOURCE=.\error.h
# End Source File
# Begin Source File

SOURCE=.\inlineparse.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\output.h
# End Source File
# Begin Source File

SOURCE=.\outstream.h
# End Source File
# Begin Source File

SOURCE=.\plstore.h
# End Source File
# End Group
# End Target
# End Project
