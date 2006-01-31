@ECHO OFF


REM ***Compile textures***

CLS
teqser.exe "grid.tif" "grid.tex"


REM ***Compile additional shaders***

CLS
aqsl.exe "texmap.sl"


REM ***Render example file***

CLS
aqsis.exe -progress "layered.rib"
