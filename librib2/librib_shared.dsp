# Microsoft Developer Studio Project File - Name="librib_shared" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=librib_shared - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "librib_shared.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "librib_shared.mak" CFG="librib_shared - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "librib_shared - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "librib_shared - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "librib_shared - Win32 Profile" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "librib_shared - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Library\Release"
# PROP Intermediate_Dir "..\Object\Release\librib_shared"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBRIB_SHARED_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\librib2" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRIB /D BUILD_LIBRIB=1 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 libz.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Library\Debug"
# PROP Intermediate_Dir "..\Object\Debug\librib_shared"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBRIB_SHARED_EXPORTS" /YX /FD /GZ  /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\librib2" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRIB /D BUILD_LIBRIB=1 /YX /FD /GZ  /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Profile"
# PROP BASE Intermediate_Dir "Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Library\Profile"
# PROP Intermediate_Dir "..\Object\Profile\librib_shared"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBRIB_SHARED_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\librib2" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /I "..\render" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "AQSIS_DYNAMIC_LINK" /D _qBUILDING=BUILD_LIBRIB /D BUILD_LIBRIB=1 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 libz.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ENDIF 

# Begin Target

# Name "librib_shared - Win32 Release"
# Name "librib_shared - Win32 Debug"
# Name "librib_shared - Win32 Profile"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bdec.cpp
# End Source File
# Begin Source File

SOURCE=.\librib.cpp
# End Source File
# Begin Source File

SOURCE=.\parser.yxx

!IF  "$(CFG)" == "librib_shared - Win32 Release"

# Begin Custom Build - Building Parser from $(InputPath)
IntDir=.\..\Object\Release\librib_shared
InputPath=.\parser.yxx

BuildCmds= \
	bison --no-lines --defines -o$(IntDir)\parser.cpp $(InputPath)

"$(IntDir)\parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\parser.hpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Debug"

# Begin Custom Build - Building Parser from $(InputPath)
IntDir=.\..\Object\Debug\librib_shared
InputPath=.\parser.yxx

BuildCmds= \
	bison --no-lines --defines -o$(IntDir)\parser.cpp $(InputPath)

"$(IntDir)\parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\parser.hpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Profile"

# Begin Custom Build - Building Parser from $(InputPath)
IntDir=.\..\Object\Profile\librib_shared
InputPath=.\parser.yxx

BuildCmds= \
	bison --no-lines --defines -o$(IntDir)\parser.cpp $(InputPath)

"$(IntDir)\parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\parser.hpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scanner.lxx

!IF  "$(CFG)" == "librib_shared - Win32 Release"

# Begin Custom Build - Building Lexical Scanner from $(InputPath)
IntDir=.\..\Object\Release\librib_shared
InputPath=.\scanner.lxx

"$(IntDir)\scanner.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o$(IntDir)\scanner.cpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Debug"

# Begin Custom Build - Building Lexical Scanner from $(InputPath)
IntDir=.\..\Object\Debug\librib_shared
InputPath=.\scanner.lxx

"$(IntDir)\scanner.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o$(IntDir)\scanner.cpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Profile"

# Begin Custom Build - Building Lexical Scanner from $(InputPath)
IntDir=.\..\Object\Profile\librib_shared
InputPath=.\scanner.lxx

"$(IntDir)\scanner.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o$(IntDir)\scanner.cpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\bdec.h
# End Source File
# Begin Source File

SOURCE=.\librib.h
# End Source File
# Begin Source File

SOURCE=.\libribtypes.h
# End Source File
# Begin Source File

SOURCE=.\parserstate.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Generated Files"

# PROP Default_Filter ""
# Begin Group "Release"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Object\Release\librib_shared\parser.cpp

!IF  "$(CFG)" == "librib_shared - Win32 Release"

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Profile"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\Release\librib_shared\parser.hpp

!IF  "$(CFG)" == "librib_shared - Win32 Release"

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Profile"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\Release\librib_shared\scanner.cpp

!IF  "$(CFG)" == "librib_shared - Win32 Release"

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Profile"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Object\Debug\librib_shared\parser.cpp

!IF  "$(CFG)" == "librib_shared - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Profile"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\Debug\librib_shared\parser.hpp

!IF  "$(CFG)" == "librib_shared - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Profile"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\Debug\librib_shared\scanner.cpp

!IF  "$(CFG)" == "librib_shared - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Profile"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Profile"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Object\Profile\librib_shared\parser.cpp

!IF  "$(CFG)" == "librib_shared - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\Profile\librib_shared\parser.hpp

!IF  "$(CFG)" == "librib_shared - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\Profile\librib_shared\scanner.cpp

!IF  "$(CFG)" == "librib_shared - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib_shared - Win32 Profile"

!ENDIF 

# End Source File
# End Group
# End Group
# End Target
# End Project
