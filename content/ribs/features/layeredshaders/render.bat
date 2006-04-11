@ECHO OFF

REM ***Compile textures***
teqser grid.tif grid.tex
IF NOT ERRORLEVEL 0 GOTO error

REM ***Compile additional shaders***
aqsl texmap.sl
IF NOT ERRORLEVEL 0 GOTO error

REM ***Render example file***
aqsis -progress layered.rib
IF ERRORLEVEL 0 GOTO end

:error
echo "Render failed, see messages"

:end
