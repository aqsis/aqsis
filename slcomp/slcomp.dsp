# Microsoft Developer Studio Project File - Name="slcomp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=slcomp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "slcomp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "slcomp.mak" CFG="slcomp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "slcomp - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "slcomp - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "slcomp"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "slcomp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Library\Release"
# PROP Intermediate_Dir "..\Object\Release\slcomp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\Object\Release\slcomp" /I "..\Render" /I "..\Render\win32\intel" /D "NDEBUG" /D SLCOMP=1 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__HAVE_NO_ALLOCA" /D _qBUILDING=SLCOMP /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 render.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"..\Library\Release"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "slcomp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Library\Debug"
# PROP Intermediate_Dir "..\Object\Debug\slcomp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\Object\Debug\slcomp" /I "..\Render" /I "..\Render\win32\intel" /D "_DEBUG" /D SLCOMP=2 /D ssVALIDATE=1 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__HAVE_NO_ALLOCA" /D _qBUILDING=SLCOMP /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 render.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\Library\Debug"

!ENDIF 

# Begin Target

# Name "slcomp - Win32 Release"
# Name "slcomp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\funcdef.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\optimise.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\parsenode.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\Parser.y

!IF  "$(CFG)" == "slcomp - Win32 Release"

USERDEP__PARSE="bison.cc"	"bison.h"	
# Begin Custom Build - Building Parser from $(InputPath)
IntDir=.\..\Object\Release\slcomp
InputPath=.\Parser.y

BuildCmds= \
	bison_pp -v -d -h $(IntDir)\parser.h -o $(IntDir)\parser.cpp $(InputPath)

"$(IntDir)\parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "slcomp - Win32 Debug"

USERDEP__PARSE="bison.cc"	"bison.h"	
# Begin Custom Build - Building Parser from $(InputPath)
IntDir=.\..\Object\Debug\slcomp
InputPath=.\Parser.y

BuildCmds= \
	bison_pp -v -d -h $(IntDir)\parser.h -o $(IntDir)\parser.cpp $(InputPath)

"$(IntDir)\parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Scanner.l

!IF  "$(CFG)" == "slcomp - Win32 Release"

# PROP Intermediate_Dir "..\Object\release\slcomp"
USERDEP__SCANN="parser.y"	"flexskel.cc"	"flexskel.h"	
# Begin Custom Build - Building Lexical Scanner from $(InputPath)
IntDir=.\..\Object\release\slcomp
InputPath=.\Scanner.l

BuildCmds= \
	flex_pp -8 -o$(IntDir)\scanner.cpp -h$(IntDir)\scanner.h $(InputPath)

"$(IntDir)\scanner.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\scanner.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "slcomp - Win32 Debug"

# PROP Intermediate_Dir "..\Object\debug\slcomp"
USERDEP__SCANN="parser.y"	"flexskel.cc"	"flexskel.h"	
# Begin Custom Build - Building Lexical Scanner from $(InputPath)
IntDir=.\..\Object\debug\slcomp
InputPath=.\Scanner.l

BuildCmds= \
	flex_pp -8 -o$(IntDir)\scanner.cpp -h$(IntDir)\scanner.h $(InputPath)

"$(IntDir)\scanner.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\scanner.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\slcomp.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\typecheck.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\vardef.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\funcdef.h
# End Source File
# Begin Source File

SOURCE=.\parsenode.h
# End Source File
# Begin Source File

SOURCE=.\vardef.h
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

SOURCE=..\Object\release\slcomp\PARSER.CPP

!IF  "$(CFG)" == "slcomp - Win32 Release"

# ADD CPP /I ".\\"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "slcomp - Win32 Debug"

# PROP Exclude_From_Build 1
# ADD CPP /I ".\\"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\release\slcomp\PARSER.H

!IF  "$(CFG)" == "slcomp - Win32 Release"

!ELSEIF  "$(CFG)" == "slcomp - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\release\slcomp\SCANNER.CPP

!IF  "$(CFG)" == "slcomp - Win32 Release"

# ADD CPP /I ".\\"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "slcomp - Win32 Debug"

# PROP Exclude_From_Build 1
# ADD CPP /I ".\\"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\release\slcomp\SCANNER.H

!IF  "$(CFG)" == "slcomp - Win32 Release"

!ELSEIF  "$(CFG)" == "slcomp - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Object\debug\slcomp\PARSER.CPP

!IF  "$(CFG)" == "slcomp - Win32 Release"

# PROP Exclude_From_Build 1
# ADD CPP /I ".\\"

!ELSEIF  "$(CFG)" == "slcomp - Win32 Debug"

# ADD CPP /I ".\\"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\debug\slcomp\PARSER.H

!IF  "$(CFG)" == "slcomp - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "slcomp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\debug\slcomp\SCANNER.CPP

!IF  "$(CFG)" == "slcomp - Win32 Release"

# PROP Exclude_From_Build 1
# ADD CPP /I ".\\"

!ELSEIF  "$(CFG)" == "slcomp - Win32 Debug"

# ADD CPP /I ".\\"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\debug\slcomp\SCANNER.H

!IF  "$(CFG)" == "slcomp - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "slcomp - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
