# Microsoft Developer Studio Project File - Name="Shaders" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=Shaders - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Shaders.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Shaders.mak" CFG="Shaders - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Shaders - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "Shaders - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Shaders"
# PROP Scc_LocalPath ".."
MTL=midl.exe

!IF  "$(CFG)" == "Shaders - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Content"
# PROP Intermediate_Dir ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Content"
# PROP Intermediate_Dir ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "Shaders - Win32 Release"
# Name "Shaders - Win32 Debug"
# Begin Group "Standard"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\constant.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\constant.sl
InputName=constant

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\constant.sl
InputName=constant

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\depthcue.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\depthcue.sl
InputName=depthcue

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\depthcue.sl
InputName=depthcue

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\distantlight.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\distantlight.sl
InputName=distantlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\distantlight.sl
InputName=distantlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fog.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\fog.sl
InputName=fog

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\fog.sl
InputName=fog

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\matte.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\matte.sl
InputName=matte

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\matte.sl
InputName=matte

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\metal.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\metal.sl
InputName=metal

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\metal.sl
InputName=metal

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\paintedplastic.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\paintedplastic.sl
InputName=paintedplastic

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\paintedplastic.sl
InputName=paintedplastic

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\plastic.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\plastic.sl
InputName=plastic

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\plastic.sl
InputName=plastic

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pointlight.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\pointlight.sl
InputName=pointlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\pointlight.sl
InputName=pointlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\spotlight.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# PROP Intermediate_Dir "..\Content"
# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\spotlight.sl
InputName=spotlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# PROP Intermediate_Dir "..\Content"
# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\spotlight.sl
InputName=spotlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Additional"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Additional\Borg_metal.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Additional
InputPath=.\Additional\Borg_metal.sl
InputName=Borg_metal

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Additional
InputPath=.\Additional\Borg_metal.sl
InputName=Borg_metal

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\debug.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\debug.sl
InputName=debug

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
InputPath=.\debug.sl
InputName=debug

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Additional\dented.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Additional
InputPath=.\Additional\dented.sl
InputName=dented

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Additional
InputPath=.\Additional\dented.sl
InputName=dented

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Additional\DPBlueMarble.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Additional
InputPath=.\Additional\DPBlueMarble.sl
InputName=DPBlueMarble

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Additional
InputPath=.\Additional\DPBlueMarble.sl
InputName=DPBlueMarble

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Additional\randgrid.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Additional
InputPath=.\Additional\randgrid.sl
InputName=randgrid

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Additional
InputPath=.\Additional\randgrid.sl
InputName=randgrid

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Additional\shadowspot.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Additional
InputPath=.\Additional\shadowspot.sl
InputName=shadowspot

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.\Additional
InputPath=.\Additional\shadowspot.sl
InputName=shadowspot

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	slcomp $(InputName).sl 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
