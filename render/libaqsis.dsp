# Microsoft Developer Studio Project File - Name="libaqsis" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

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
!MESSAGE "libaqsis - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libaqsis - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libaqsis - Win32 Profile" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libaqsis - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\Object\Release\libaqsis"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBAQSIS_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\libaqsistypes\win32\intel" /I "..\libaqsistypes" /I "..\libshadervm" /I "..\libshaderexecenv" /I "..\librib2ri" /I "..\librib2" /I "..\..\win32libs\include" /I "..\boost" /D "NDEBUG" /D _qBUILDING=CORE /D CORE=1 /D "PLUGINS" /D "_MBCS" /D "_LIB" /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 libz.lib libtiff.lib ws2_32.lib /nologo /dll /machine:I386 /libpath:"..\lib" /libpath:"..\bin" /libpath:"..\..\win32libs\lib"

!ELSEIF  "$(CFG)" == "libaqsis - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\Object\Debug\libaqsis"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBAQSIS_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\libaqsistypes\win32\intel" /I "..\libaqsistypes" /I "..\libshadervm" /I "..\libshaderexecenv" /I "..\librib2ri" /I "..\librib2" /I "..\..\win32libs\include" /I "..\boost" /D "_DEBUG" /D _qBUILDING=CORE /D CORE=1 /D "PLUGINS" /D "_MBCS" /D "_LIB" /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /GZ /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libz.lib libtiff.lib ws2_32.lib /nologo /dll /map /debug /machine:I386 /out:"..\bin/libaqsis_d.dll" /pdbtype:sept /libpath:"..\lib" /libpath:"..\bin" /libpath:"..\..\win32libs\lib"

!ELSEIF  "$(CFG)" == "libaqsis - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libaqsis___Win32_Profile"
# PROP BASE Intermediate_Dir "libaqsis___Win32_Profile"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin\Profile"
# PROP Intermediate_Dir "..\Object\Profile\libaqsis"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\libaqsistypes\win32\intel" /I "..\libaqsistypes" /I "..\libshadervm" /I "..\libshaderexecenv" /I "..\librib2ri" /I "..\librib2" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "PLUGINS" /D "AQSIS_DYNAMIC_LINK" /D "WIN32" /D _qBUILDING=CORE /D CORE=1 /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "..\libaqsistypes\win32\intel" /I "..\libaqsistypes" /I "..\libshadervm" /I "..\libshaderexecenv" /I "..\librib2ri" /I "..\librib2" /I "..\..\win32libs\include" /I "..\boost" /D "NDEBUG" /D CORE=1 /D _qBUILDING=CORE /D "PLUGINS" /D "_MBCS" /D "_LIB" /D "WIN32" /D "NO_SYSLOG" /FR /YX /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libaqsistypes.lib libshaderexecenv.lib libshadervm.lib libddmsock.lib libz.lib libtiff.lib ws2_32.lib librib2.lib librib2ri.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /libpath:"..\Library\Release"
# ADD LINK32 libz.lib libtiff.lib ws2_32.lib /nologo /dll /profile /debug /machine:I386 /libpath:"..\lib\Profile" /libpath:"..\bin\Profile" /libpath:"..\..\win32libs\lib"

!ENDIF 

# Begin Target

# Name "libaqsis - Win32 Release"
# Name "libaqsis - Win32 Debug"
# Name "libaqsis - Win32 Profile"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\attributes.cpp
# End Source File
# Begin Source File

SOURCE=.\bound.cpp
# End Source File
# Begin Source File

SOURCE=.\bucket.cpp
# End Source File
# Begin Source File

SOURCE=.\converter.cpp
# End Source File
# Begin Source File

SOURCE=.\csgtree.cpp
# End Source File
# Begin Source File

SOURCE=.\curves.cpp
# End Source File
# Begin Source File

SOURCE=.\environment.cpp
# End Source File
# Begin Source File

SOURCE=.\genpoly.cpp
# End Source File
# Begin Source File

SOURCE=.\graphicsstate.cpp
# End Source File
# Begin Source File

SOURCE=.\imagebuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\imagepixel.cpp
# End Source File
# Begin Source File

SOURCE=.\imagers.cpp
# End Source File
# Begin Source File

SOURCE=.\inlineparse.cpp
# End Source File
# Begin Source File

SOURCE=.\lath.cpp
# End Source File
# Begin Source File

SOURCE=.\libaqsis.def
# End Source File
# Begin Source File

SOURCE=.\lights.cpp
# End Source File
# Begin Source File

SOURCE=.\micropolygon.cpp
# End Source File
# Begin Source File

SOURCE=.\nurbs.cpp
# End Source File
# Begin Source File

SOURCE=.\occlusion.cpp
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

SOURCE=.\points.cpp
# End Source File
# Begin Source File

SOURCE=.\polygon.cpp
# End Source File
# Begin Source File

SOURCE=.\procedural.cpp
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

SOURCE=.\shadowmap.cpp
# End Source File
# Begin Source File

SOURCE=.\stats.cpp
# End Source File
# Begin Source File

SOURCE=.\subdivision2.cpp
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

SOURCE=.\bucket.h
# End Source File
# Begin Source File

SOURCE=.\converter.h
# End Source File
# Begin Source File

SOURCE=.\csgtree.h
# End Source File
# Begin Source File

SOURCE=.\curves.h
# End Source File
# Begin Source File

SOURCE=.\focus.h
# End Source File
# Begin Source File

SOURCE=.\forwarddiff.h
# End Source File
# Begin Source File

SOURCE=.\genpoly.h
# End Source File
# Begin Source File

SOURCE=.\graphicsstate.h
# End Source File
# Begin Source File

SOURCE=.\iattributes.h
# End Source File
# Begin Source File

SOURCE=.\iddmanager.h
# End Source File
# Begin Source File

SOURCE=.\idsoshadeops.h
# End Source File
# Begin Source File

SOURCE=.\ilightsource.h
# End Source File
# Begin Source File

SOURCE=.\ImageBuffer.h
# End Source File
# Begin Source File

SOURCE=.\imagepixel.h
# End Source File
# Begin Source File

SOURCE=.\imagers.h
# End Source File
# Begin Source File

SOURCE=.\inlineparse.h
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

SOURCE=.\kdtree.h
# End Source File
# Begin Source File

SOURCE=.\lath.h
# End Source File
# Begin Source File

SOURCE=.\lights.h
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

SOURCE=.\objectinstance.h
# End Source File
# Begin Source File

SOURCE=.\occlusion.h
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

SOURCE=.\points.h
# End Source File
# Begin Source File

SOURCE=.\polygon.h
# End Source File
# Begin Source File

SOURCE=.\procedural.h
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

SOURCE=.\ri_cache.h
# End Source File
# Begin Source File

SOURCE=.\rifile.h
# End Source File
# Begin Source File

SOURCE=.\shadeop.h
# End Source File
# Begin Source File

SOURCE=.\shaders.h
# End Source File
# Begin Source File

SOURCE=.\stats.h
# End Source File
# Begin Source File

SOURCE=.\subdivision2.h
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
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
