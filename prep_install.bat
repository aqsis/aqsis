@echo off
mkdir ..\binary-archive
mkdir ..\binary-archive\shaders
mkdir ..\binary-archive\docs
mkdir ..\binary-archive\examples


cd ..\binary-archive

copy ..\library\release\cribber.exe .
copy ..\library\release\filebuffer.exe .
copy ..\library\release\framebuffer.exe .
copy ..\library\release\shadowmap.exe .
copy ..\library\release\slcomp.exe .
copy ..\library\release\slpp.exe .
copy ..\library\release\texer.exe .

copy \Libraries\tiff-v3.5.5\libtiff\libtiff.dll .
copy \Libraries\zlib-1.1.3\zlib.dll .

copy ..\ribber.cfg .
copy ..\aqsis.ini .

cd shaders
copy ..\..\shaders\constant.sl .
copy ..\..\shaders\constant.slx .
copy ..\..\shaders\matte.sl .
copy ..\..\shaders\matte.slx .
copy ..\..\shaders\metal.sl .
copy ..\..\shaders\metal.slx .
copy ..\..\shaders\plastic.sl .
copy ..\..\shaders\plastic.slx .
copy ..\..\shaders\paintedplastic.sl .
copy ..\..\shaders\paintedplastic.slx .
copy ..\..\shaders\distantlight.sl .
copy ..\..\shaders\distantlight.slx .
copy ..\..\shaders\pointlight.sl .
copy ..\..\shaders\pointlight.slx .
copy ..\..\shaders\spotlight.sl .
copy ..\..\shaders\spotlight.slx .
copy ..\..\shaders\fog.sl .
copy ..\..\shaders\fog.slx .
copy ..\..\shaders\depthcue.sl .
copy ..\..\shaders\depthcue.slx .

copy ..\..\shaders\additional\shadowspot.sl .
copy ..\..\shaders\additional\shadowspot.slx .
copy ..\..\shaders\additional\dented.sl .
copy ..\..\shaders\additional\dented.slx .

cd ..\docs
copy ..\..\README .
copy ..\..\NEWS .
copy ..\..\BUILD .
copy ..\..\AUTHORS .
copy ..\..\License.txt .

cd ..\examples
copy ..\..\content\vase_shad1.rib .
copy ..\..\content\vase_shad2.rib .
copy ..\..\content\vase.rib .
copy ..\..\content\test\noshadows.rib .
copy ..\..\content\test\autoshadows.rib .

cd ..\..\
wzzip -a -r -p Aqsis-0.4.4003.zip Install\*.*