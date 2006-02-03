@ECHO OFF


REM ***Compile additional shaders***

CLS
aqsl.exe "../../../shaders/displacement/dented.sl" "../../../shaders/light/shadowspot.sl"


REM ***Render example file***

CLS
aqsis.exe -progress "vase.rib"
