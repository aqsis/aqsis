@ECHO OFF

REM ***Compile shaders***

ECHO === Compiling Shader(s) ===
aqsl.exe "fisheye_projection.sl"

REM ***Render files***

ECHO.
ECHO === Rendering File(s) ===
aqsis.exe -progress "vase.rib"

ECHO Rendering 6 cube environment faces ...
aqsis.exe -progress "environment_faces.rib"
ECHO Rendering fisheye projection from centre ...
aqsis -progress "fisheye.rib"
ECHO Rendering external view of environment with reflective sphere ...
aqsis -progress "environment_scene.rib"
