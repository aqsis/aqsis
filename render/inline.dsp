# Microsoft Developer Studio Project File - Name="inline" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=inline - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "inline.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "inline.mak" CFG="inline - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "inline - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "inline - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "inline - Win32 Release"

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

!ELSEIF  "$(CFG)" == "inline - Win32 Debug"

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

# Name "inline - Win32 Release"
# Name "inline - Win32 Debug"
# Begin Group "xml"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\api.xml

!IF  "$(CFG)" == "inline - Win32 Release"

USERDEP__API_X="apiheader.xsl"	"apicache.xsl"	"apivalidate.xsl"	
# Begin Custom Build
InputPath=.\api.xml

BuildCmds= \
	echo Building API header inline file \
	..\..\win32libs\bin\xsltproc -o ri.inl apiheader.xsl api.xml \
	echo Building API cache inline file \
	..\..\win32libs\bin\xsltproc -o ri_cache.inl apicache.xsl api.xml \
	echo Building API validate inline file \
	..\..\win32libs\bin\xsltproc -o ri_validate.inl apivalidate.xsl api.xml \
	

"ri.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ri_cache.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ri_validate.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "inline - Win32 Debug"

USERDEP__API_X="apiheader.xsl"	"apicache.xsl"	"apivalidate.xsl"	
# Begin Custom Build
InputPath=.\api.xml

BuildCmds= \
	echo Building API header inline file \
	..\..\win32libs\bin\xsltproc -o ri.inl apiheader.xsl api.xml \
	echo Building API cache inline file \
	..\..\win32libs\bin\xsltproc -o ri_cache.inl apicache.xsl api.xml \
	echo Building API validate inline file \
	..\..\win32libs\bin\xsltproc -o ri_validate.inl apivalidate.xsl api.xml \
	

"ri.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ri_cache.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ri_validate.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "xslt"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\apicache.xsl
# End Source File
# Begin Source File

SOURCE=.\apiheader.xsl
# End Source File
# Begin Source File

SOURCE=.\apivalidate.xsl
# End Source File
# End Group
# End Target
# End Project
