@echo off
if not exist ..\binary-archive mkdir ..\binary-archive
if not exist ..\binary-archive\shaders mkdir ..\binary-archive\shaders
if not exist ..\binary-archive\docs mkdir ..\binary-archive\docs
if not exist ..\binary-archive\examples mkdir ..\binary-archive\examples
if not exist ..\binary-archive\lib mkdir ..\binary-archive\lib
if not exist ..\binary-archive\include mkdir ..\binary-archive\include

erase /f /s /q ..\binary-archive\*.*

cd ..\binary-archive

copy ..\library\release\aqsis.exe .
copy ..\library\release\filebuffer.exe .
copy ..\library\release\framebuffer_glut.exe .
copy ..\library\release\framebuffer_glut_z.exe .
copy ..\library\release\shadowmap.exe .
copy ..\library\release\aqsl.exe .
copy ..\library\release\aqslcomp.exe .
copy ..\library\release\slpp.exe .
copy ..\library\release\teqser.exe .
copy ..\library\release\ddmsock.ini .

copy \Libraries\tiff-v3.5.7\libtiff\libtiff.dll .

copy ..\ribber.cfg .

cd shaders
copy ..\..\shaders\*.sl .
copy ..\..\shaders\*.slx .

cd ..\docs
copy ..\..\README .
copy ..\..\NEWS .
copy ..\..\BUILD .
copy ..\..\AUTHORS .
copy ..\..\License.txt .

cd ..\examples
copy ..\..\ribfiles\*.rib .

cd ..\lib
copy ..\..\library\release\libri2rib.lib .

cd ..\include
copy ..\..\render\ri.h .
copy ..\..\libaqsistypes\win32\intel\share.h .

cd ..\..\
erase /f /q Aqsis_Win32.zip
wzzip -a -r -p Aqsis_Win32.zip binary-archive\*.* 
