CD Library\Profile
PREP /OM /FT /EXCALL /INC libaqsis.lib /INC libaqsistypes.lib /INC libshaderexecenv.lib /INC libshadervm.lib /INC libddmsock.lib /INC librib2.lib aqsis.exe libaqsis.dll
if errorlevel == 1 goto done 
PROFILE aqsis %1 %2 %3 %4 %5 %6 %7 %8 %9
if errorlevel == 1 goto done 
PREP /M aqsis
if errorlevel == 1 goto done 
PLIST /SC aqsis >aqsis.ftime.lst
MOVE aqsis.ftime.lst ..\..
:done
CD ..\..
