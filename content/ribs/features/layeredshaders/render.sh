#!/usr/bin/bash


echo 
echo
echo ***Compile textures***
echo
teqser /usr/share/aqsis/content/ribs/features/layeredshaders/grid.tif grid.tex


echo 
echo
echo  ***Compile shaders***
echo
aqsl /usr/share/aqsis/content/ribs/features/layeredshaders/texmap.sl


echo 
echo
echo ***Render files***
echo
aqsis -progress --shaders=/usr/share/aqsis/shaders:. /usr/share/aqsis/content/ribs/features/layeredshaders/layered.rib
