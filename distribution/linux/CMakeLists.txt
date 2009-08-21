INSTALL(FILES application.png
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pixmaps
	RENAME aqsis.png)

INSTALL(FILES mime.png
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/192x192/mimetypes
	RENAME aqsis-doc.png)

INSTALL(FILES aqsis.xml
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/mime/packages)

INSTALL(FILES aqsis.desktop
	aqsl.desktop
	aqsltell.desktop
	eqsl.desktop
	piqsl.desktop
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)

INSTALL(PROGRAMS aqsis.sh
	aqsl.sh
	aqsltell.sh
	DESTINATION ${SCRIPTSDIR})
