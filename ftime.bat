CD bin\Profile
PREP /OM /FT /EXC librib2.lib aqsis.exe libaqsis.dll
if errorlevel == 1 goto done 
PROFILE aqsis %1 %2 %3 %4 %5 %6 %7 %8 %9
if errorlevel == 1 goto done 
PREP /M aqsis
if errorlevel == 1 goto done 
PLIST /ST aqsis >aqsis.ftime.lst
MOVE aqsis.ftime.lst ..\..
:done
CD ..\..
