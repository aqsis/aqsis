# Microsoft Developer Studio Project File - Name="thread" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=thread - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "thread.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "thread.mak" CFG="thread - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "thread - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "thread - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "thread - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\bin"
# PROP Intermediate_Dir "..\Object\Release\thread"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "THREAD_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W1 /GX /O2 /I "..\..\..\boost" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "THREAD_EXPORTS" /D BOOST_THREAD_BUILD_DLL=1 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "thread - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\bin"
# PROP Intermediate_Dir "..\..\..\Object\Debug\thread"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "THREAD_EXPORTS" /YX /FD /GZ  /c
# ADD CPP /nologo /MDd /W1 /Gm /GX /ZI /Od /I "..\..\..\boost" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "THREAD_EXPORTS" /D BOOST_THREAD_BUILD_DLL=1 /YX /FD /GZ  /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"..\..\..\bin/thread_d.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "thread - Win32 Release"
# Name "thread - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\condition.cpp
# End Source File
# Begin Source File

SOURCE=.\src\exceptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\src\once.cpp
# End Source File
# Begin Source File

SOURCE=.\src\recursive_mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thread.cpp
# End Source File
# Begin Source File

SOURCE=.\src\threadmon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tss.cpp
# End Source File
# Begin Source File

SOURCE=.\src\xtime.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\boost\thread\condition.hpp
# End Source File
# Begin Source File

SOURCE=..\..\boost\thread\detail\config.hpp
# End Source File
# Begin Source File

SOURCE=..\..\boost\thread\exceptions.hpp
# End Source File
# Begin Source File

SOURCE=..\..\boost\thread\detail\force_cast.hpp
# End Source File
# Begin Source File

SOURCE=..\..\boost\thread\detail\lock.hpp
# End Source File
# Begin Source File

SOURCE=..\..\boost\thread\mutex.hpp
# End Source File
# Begin Source File

SOURCE=..\..\boost\thread\once.hpp
# End Source File
# Begin Source File

SOURCE=..\..\boost\thread\recursive_mutex.hpp
# End Source File
# Begin Source File

SOURCE=..\..\boost\thread\detail\singleton.hpp
# End Source File
# Begin Source File

SOURCE=..\..\boost\thread\thread.hpp
# End Source File
# Begin Source File

SOURCE=..\..\boost\thread\detail\threadmon.hpp
# End Source File
# Begin Source File

SOURCE=..\..\boost\thread\tss.hpp
# End Source File
# Begin Source File

SOURCE=..\..\boost\thread\xtime.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
