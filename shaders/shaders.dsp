# Microsoft Developer Studio Project File - Name="Shaders" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=Shaders - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "shaders.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "shaders.mak" CFG="Shaders - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Shaders - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "Shaders - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE "Shaders - Win32 Profile" (based on "Win32 (x86) Generic Project")
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
# PROP Output_Dir "..\Library\Release"
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
# PROP Output_Dir "..\Library\Debug"
# PROP Intermediate_Dir ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Shaders___Win32_Profile"
# PROP BASE Intermediate_Dir "Shaders___Win32_Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Library\Profile"
# PROP Intermediate_Dir "..\Object\Profile\shaders"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "Shaders - Win32 Release"
# Name "Shaders - Win32 Debug"
# Name "Shaders - Win32 Profile"
# Begin Group "Standard"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ambientlight.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\ambientlight.sl
InputName=ambientlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\ambientlight.sl
InputName=ambientlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\ambientlight.sl
InputName=ambientlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cellnoisetest.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\cellnoisetest.sl
InputName=cellnoisetest

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\cellnoisetest.sl
InputName=cellnoisetest

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\cellnoisetest.sl
InputName=cellnoisetest

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\constant.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\constant.sl
InputName=constant

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\constant.sl
InputName=constant

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\constant.sl
InputName=constant

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dented.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\dented.sl
InputName=dented

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\dented.sl
InputName=dented

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\dented.sl
InputName=dented

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\depthcue.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\depthcue.sl
InputName=depthcue

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\depthcue.sl
InputName=depthcue

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\depthcue.sl
InputName=depthcue

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\distantlight.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\distantlight.sl
InputName=distantlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\distantlight.sl
InputName=distantlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\distantlight.sl
InputName=distantlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fog.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\fog.sl
InputName=fog

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\fog.sl
InputName=fog

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\fog.sl
InputName=fog

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\matte.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\matte.sl
InputName=matte

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\matte.sl
InputName=matte

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\matte.sl
InputName=matte

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\metal.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\metal.sl
InputName=metal

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\metal.sl
InputName=metal

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\metal.sl
InputName=metal

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\paintedplastic.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\paintedplastic.sl
InputName=paintedplastic

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\paintedplastic.sl
InputName=paintedplastic

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\paintedplastic.sl
InputName=paintedplastic

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\plastic.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\plastic.sl
InputName=plastic

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\plastic.sl
InputName=plastic

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\plastic.sl
InputName=plastic

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pointlight.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\pointlight.sl
InputName=pointlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\pointlight.sl
InputName=pointlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\pointlight.sl
InputName=pointlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\randgrid.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\randgrid.sl
InputName=randgrid

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\randgrid.sl
InputName=randgrid

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\randgrid.sl
InputName=randgrid

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shadowspot.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\shadowspot.sl
InputName=shadowspot

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\shadowspot.sl
InputName=shadowspot

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\shadowspot.sl
InputName=shadowspot

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shinymetal.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\shinymetal.sl
InputName=shinymetal

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\shinymetal.sl
InputName=shinymetal

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\shinymetal.sl
InputName=shinymetal

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\spotlight.sl

!IF  "$(CFG)" == "Shaders - Win32 Release"

# PROP Intermediate_Dir "..\Content"
# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Release
InputPath=.\spotlight.sl
InputName=spotlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Debug"

# PROP Intermediate_Dir "..\Content"
# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Debug
InputPath=.\spotlight.sl
InputName=spotlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Shaders - Win32 Profile"

# PROP BASE Intermediate_Dir "..\Content"
# PROP Intermediate_Dir "..\Content"
# Begin Custom Build - Compiling Shader $(InputPath)
InputDir=.
OutDir=.\..\Library\Profile
InputPath=.\spotlight.sl
InputName=spotlight

"$(InputDir)\$(InputName).slx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(OutDir)\aqsl $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
