@echo off

REM Compile Aqsis shaders, cycles through all shaders specified on the command line
REM passing them to slpp and piping the result to aqslcomp.

if "%1"=="" goto usage

:compile
if not exist %1 goto missing
echo Compiling %1
slpp -d PI=3.141592654 -d AQSIS -c6 %1 | aqslcomp
shift
if "%1"=="" goto end
goto compile

:usage
echo Usage: aqsl filename [filename...]
echo where: filename is the name of the .sl file to compile.
goto end

:missing
echo Cannot find shader %1
goto end

:end