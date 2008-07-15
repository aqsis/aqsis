INSTALL(FILES application.png
	aqsis.desktop
	aqsis.xml
	aqsl.desktop
	aqsltell.desktop
	eqsl.desktop
	mime.png
	piqsl.desktop
	DESTINATION ${DESKTOPDIR})

INSTALL(PROGRAMS aqsis.sh
	aqsl.sh
	aqsltell.sh
	DESTINATION ${DESKTOPDIR})
