# Microsoft Developer Studio Project File - Name="Content" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=Content - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Content.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Content.mak" CFG="Content - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Content - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "Content - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Content"
# PROP Scc_LocalPath "."
MTL=midl.exe

!IF  "$(CFG)" == "Content - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Content - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "Content - Win32 Release"
# Name "Content - Win32 Debug"
# Begin Source File

SOURCE=.\Test\brickbump.sl

!IF  "$(CFG)" == "Content - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Test
InputPath=.\Test\brickbump.sl
InputName=brickbump

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Content - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Test
InputPath=.\Test\brickbump.sl
InputName=brickbump

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Test\bucket_test.rib
# End Source File
# Begin Source File

SOURCE=.\Test\disp_test.rib
# End Source File
# Begin Source File

SOURCE=.\Test\DPProctext.h
# End Source File
# Begin Source File

SOURCE=.\Test\eyeplanetest.rib
# End Source File
# Begin Source File

SOURCE=.\Test\mod_st2_test.rib
# End Source File
# Begin Source File

SOURCE=.\Test\mod_st_test.rib
# End Source File
# Begin Source File

SOURCE=.\Test\noises.h
# End Source File
# Begin Source File

SOURCE=.\Test\orient_test.rib
# End Source File
# Begin Source File

SOURCE=.\Test\patterns.h
# End Source File
# Begin Source File

SOURCE=.\Test\periodic_test.rib
# End Source File
# Begin Source File

SOURCE=.\Test\persptest.rib
# End Source File
# Begin Source File

SOURCE=.\Test\shadtest.rib
# End Source File
# Begin Source File

SOURCE=.\Test\show_st.sl

!IF  "$(CFG)" == "Content - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Test
InputPath=.\Test\show_st.sl
InputName=show_st

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Content - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Test
InputPath=.\Test\show_st.sl
InputName=show_st

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Test\space_test.rib
# End Source File
# Begin Source File

SOURCE=.\Test\spacetest.sl

!IF  "$(CFG)" == "Content - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Test
InputPath=.\Test\spacetest.sl
InputName=spacetest

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Content - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Test
InputPath=.\Test\spacetest.sl
InputName=spacetest

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Test\std_st_test.rib
# End Source File
# Begin Source File

SOURCE=.\test.rib
# End Source File
# Begin Source File

SOURCE=.\Test\testincl.rib
# End Source File
# Begin Source File

SOURCE=.\Test\tmap_test.rib
# End Source File
# Begin Source File

SOURCE=.\Test\uv_test.rib
# End Source File
# Begin Source File

SOURCE=.\Test\uvtest.sl

!IF  "$(CFG)" == "Content - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Test
InputPath=.\Test\uvtest.sl
InputName=uvtest

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Content - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Test
InputPath=.\Test\uvtest.sl
InputName=uvtest

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vase.rib
# End Source File
# Begin Source File

SOURCE=.\vase2.rib
# End Source File
# Begin Source File

SOURCE=.\vase_builtin.rib
# End Source File
# Begin Source File

SOURCE=.\vase_shad1.rib
# End Source File
# Begin Source File

SOURCE=.\vase_shad2.rib
# End Source File
# End Target
# End Project
