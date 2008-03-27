# Find resource fork tool (currently necessray for OS X)

IF(APPLE)
	FIND_PROGRAM(AQSIS_RESOURCE_EXECUTABLE
        	Rez
        	PATHS /Developer/Tools
        	DOC "Location of the resource fork executable (OS X)"
	)

	STRING(COMPARE NOTEQUAL ${AQSIS_RESOURCE_EXECUTABLE} "AQSIS_RESOURCE_EXECUTABLE-NOTFOUND" AQSIS_RESOURCE_EXECUTABLE_FOUND)
ENDIF(APPLE)
