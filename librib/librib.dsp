# Microsoft Developer Studio Project File - Name="librib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=librib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "librib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "librib.mak" CFG="librib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "librib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "librib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "librib"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "librib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Library\Release"
# PROP Intermediate_Dir "..\Object\Release\librib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\Object\Release\librib" /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "librib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Library\Debug"
# PROP Intermediate_Dir "..\Object\Debug\librib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\Object\Debug\librib" /I "..\render" /I "..\libaqsistypes" /I "..\libaqsistypes\win32\intel" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "librib - Win32 Release"
# Name "librib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\parser.y

!IF  "$(CFG)" == "librib - Win32 Release"

# Begin Custom Build - Building Parser from $(InputPath)
IntDir=.\..\Object\Release\librib
InputPath=.\parser.y

BuildCmds= \
	bison_pp -v -d -h $(IntDir)\parser.h -o $(IntDir)\parser.cpp $(InputPath)

"$(IntDir)\parser.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\parser.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "librib - Win32 Debug"

# PROP Intermediate_Dir "..\Object\Debug\librib"
# Begin Custom Build - Building Parser from $(InputPath)
IntDir=.\..\Object\Debug\librib
InputPath=.\parser.y

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

SOURCE=.\ribcompiler.cpp
# End Source File
# Begin Source File

SOURCE=.\scanner.l

!IF  "$(CFG)" == "librib - Win32 Release"

# Begin Custom Build - Building Lexical Scanner from $(InputPath)
IntDir=.\..\Object\Release\librib
InputPath=.\scanner.l

BuildCmds= \
	flex_pp -8 -o$(IntDir)\scanner.cpp -h$(IntDir)\scanner.h $(InputPath)

"$(IntDir)\scanner.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\scanner.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "librib - Win32 Debug"

# PROP Intermediate_Dir "..\Object\Debug\librib"
# Begin Custom Build - Building Lexical Scanner from $(InputPath)
IntDir=.\..\Object\Debug\librib
InputPath=.\scanner.l

BuildCmds= \
	flex_pp -8 -o$(IntDir)\scanner.cpp -h$(IntDir)\scanner.h $(InputPath)

"$(IntDir)\scanner.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\scanner.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\iribcompiler.h
# End Source File
# Begin Source File

SOURCE=.\ribcompiler.h
# End Source File
# End Group
# Begin Group "Generated Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Object\Debug\librib\PARSER.CPP

!IF  "$(CFG)" == "librib - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\Release\librib\PARSER.CPP

!IF  "$(CFG)" == "librib - Win32 Release"

!ELSEIF  "$(CFG)" == "librib - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\Debug\librib\PARSER.H

!IF  "$(CFG)" == "librib - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\Release\librib\PARSER.H

!IF  "$(CFG)" == "librib - Win32 Release"

!ELSEIF  "$(CFG)" == "librib - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\Debug\librib\SCANNER.CPP

!IF  "$(CFG)" == "librib - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\Release\librib\SCANNER.CPP

!IF  "$(CFG)" == "librib - Win32 Release"

!ELSEIF  "$(CFG)" == "librib - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\Debug\librib\SCANNER.H

!IF  "$(CFG)" == "librib - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "librib - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Object\Release\librib\SCANNER.H

!IF  "$(CFG)" == "librib - Win32 Release"

!ELSEIF  "$(CFG)" == "librib - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
