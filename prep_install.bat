@echo off
mkdir ..\binary-archive
mkdir ..\binary-archive\shaders
mkdir ..\binary-archive\docs
mkdir ..\binary-archive\examples


cd ..\binary-archive

copy ..\library\release\aqsis.exe .
copy ..\library\release\filebuffer.exe .
copy ..\library\release\framebuffer.exe .
copy ..\library\release\shadowmap.exe .
copy ..\library\release\aqsl.exe .
copy ..\library\release\aqslcomp.exe .
copy ..\library\release\slpp.exe .
copy ..\library\release\texer.exe .
copy ..\library\release\ddmsock.ini .

copy \Libraries\tiff-v3.5.5\libtiff\libtiff.dll .
copy \Libraries\zlib-1.1.3\zlib.dll .

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

cd ..\..\
wzzip -a -r -p Aqsis_Win32.zip binary-archive\*.* 
