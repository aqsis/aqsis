# Microsoft Developer Studio Project File - Name="libaqsis" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libaqsis - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libaqsis.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libaqsis.mak" CFG="libaqsis - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libaqsis - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libaqsis - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "libaqsis"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libaqsis - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Library\Release"
# PROP Intermediate_Dir "..\Object\Release\libaqsis"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\libaqsistypes\win32\intel" /I "..\libaqsistypes" /I "..\libshadervm" /I "..\libshaderexecenv" /I "..\librib2ri" /I "..\librib2" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /FR /YX /FD /Zm200 /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libaqsis - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Library\Debug"
# PROP Intermediate_Dir "..\Object\Debug\libaqsis"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\libaqsistypes\win32\intel" /I "..\libaqsistypes" /I "..\libshadervm" /I "..\libshaderexecenv" /I "..\librib2ri" /I "..\librib2" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D _qBUILDING=CORE /D CORE=1 /FR /YX /FD /GZ /Zm200 /c
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

# Name "libaqsis - Win32 Release"
# Name "libaqsis - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\attributes.cpp
# End Source File
# Begin Source File

SOURCE=.\bound.cpp
# End Source File
# Begin Source File

SOURCE=.\context.cpp
# End Source File
# Begin Source File

SOURCE=.\csgtree.cpp
# End Source File
# Begin Source File

SOURCE=.\genpoly.cpp
# End Source File
# Begin Source File

SOURCE=.\imagebuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\imagers.cpp
# End Source File
# Begin Source File

SOURCE=.\lights.cpp
# End Source File
# Begin Source File

SOURCE=.\messages.cpp
# End Source File
# Begin Source File

SOURCE=.\micropolygon.cpp
# End Source File
# Begin Source File

SOURCE=.\nurbs.cpp
# End Source File
# Begin Source File

SOURCE=.\options.cpp
# End Source File
# Begin Source File

SOURCE=.\parameters.cpp
# End Source File
# Begin Source File

SOURCE=.\patch.cpp
# End Source File
# Begin Source File

SOURCE=.\plugins.cpp
# End Source File
# Begin Source File

SOURCE=.\points.cpp
# End Source File
# Begin Source File

SOURCE=.\polygon.cpp
# End Source File
# Begin Source File

SOURCE=.\quadrics.cpp
# End Source File
# Begin Source File

SOURCE=.\render.cpp
# End Source File
# Begin Source File

SOURCE=.\renderer.cpp
# End Source File
# Begin Source File

SOURCE=.\ri.cpp
# End Source File
# Begin Source File

SOURCE=.\shaders.cpp
# End Source File
# Begin Source File

SOURCE=.\stats.cpp
# End Source File
# Begin Source File

SOURCE=.\subdivision.cpp
# End Source File
# Begin Source File

SOURCE=.\surface.cpp
# End Source File
# Begin Source File

SOURCE=.\symbols.cpp
# End Source File
# Begin Source File

SOURCE=.\teapot.cpp
# End Source File
# Begin Source File

SOURCE=.\texturemap.cpp
# End Source File
# Begin Source File

SOURCE=.\transform.cpp
# End Source File
# Begin Source File

SOURCE=.\trimcurve.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\attributes.h
# End Source File
# Begin Source File

SOURCE=.\bilinear.h
# End Source File
# Begin Source File

SOURCE=.\bound.h
# End Source File
# Begin Source File

SOURCE=.\Context.h
# End Source File
# Begin Source File

SOURCE=.\csgtree.h
# End Source File
# Begin Source File

SOURCE=.\genpoly.h
# End Source File
# Begin Source File

SOURCE=.\iattributes.h
# End Source File
# Begin Source File

SOURCE=.\iddmanager.h
# End Source File
# Begin Source File

SOURCE=.\ilightsource.h
# End Source File
# Begin Source File

SOURCE=.\ImageBuffer.h
# End Source File
# Begin Source File

SOURCE=.\imagers.h
# End Source File
# Begin Source File

SOURCE=.\irenderer.h
# End Source File
# Begin Source File

SOURCE=.\ishader.h
# End Source File
# Begin Source File

SOURCE=.\ishaderdata.h
# End Source File
# Begin Source File

SOURCE=.\ishaderexecenv.h
# End Source File
# Begin Source File

SOURCE=.\isurface.h
# End Source File
# Begin Source File

SOURCE=.\itexturemap.h
# End Source File
# Begin Source File

SOURCE=.\itransform.h
# End Source File
# Begin Source File

SOURCE=.\lights.h
# End Source File
# Begin Source File

SOURCE=.\messages.h
# End Source File
# Begin Source File

SOURCE=.\micropolygon.h
# End Source File
# Begin Source File

SOURCE=.\motion.h
# End Source File
# Begin Source File

SOURCE=.\nurbs.h
# End Source File
# Begin Source File

SOURCE=.\Options.h
# End Source File
# Begin Source File

SOURCE=.\parameters.h
# End Source File
# Begin Source File

SOURCE=.\patch.h
# End Source File
# Begin Source File

SOURCE=.\plugins.h
# End Source File
# Begin Source File

SOURCE=.\points.h
# End Source File
# Begin Source File

SOURCE=.\polygon.h
# End Source File
# Begin Source File

SOURCE=.\quadrics.h
# End Source File
# Begin Source File

SOURCE=.\render.h
# End Source File
# Begin Source File

SOURCE=.\Renderer.h
# End Source File
# Begin Source File

SOURCE=.\ri.h
# End Source File
# Begin Source File

SOURCE=.\rifile.h
# End Source File
# Begin Source File

SOURCE=.\shaders.h
# End Source File
# Begin Source File

SOURCE=.\win32\intel\share.h
# End Source File
# Begin Source File

SOURCE=.\stats.h
# End Source File
# Begin Source File

SOURCE=.\subdivision.h
# End Source File
# Begin Source File

SOURCE=.\Surface.h
# End Source File
# Begin Source File

SOURCE=.\symbols.h
# End Source File
# Begin Source File

SOURCE=.\teapot.h
# End Source File
# Begin Source File

SOURCE=.\texturemap.h
# End Source File
# Begin Source File

SOURCE=.\transform.h
# End Source File
# Begin Source File

SOURCE=.\trimcurve.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# End Target
# End Project
