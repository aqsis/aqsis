1 VERSIONINFO
FILEVERSION ${MAJOR}, ${MINOR}, ${BUILD}, 0
PRODUCTVERSION ${MAJOR}, ${MINOR}, ${BUILD}, 0
FILEOS 4
FILETYPE 1
{
	BLOCK "StringFileInfo" {
		BLOCK "040904b0" {
			VALUE "CompanyName", "${PACKAGE_VENDOR}"
			VALUE "FileDescription", "${PACKAGE_NAME}"
			VALUE "FileVersion", "${MAJOR}.${MINOR}.${BUILD}"
			VALUE "LegalCopyright", "${PACKAGE_COPYRIGHT}"
			VALUE "LegalTrademarks", "${PACKAGE_COPYRIGHT_OTHER}"
			VALUE "ProductName", "${CMAKE_PROJECT_NAME}"
			VALUE "ProductVersion", "${MAJOR}.${MINOR}.${BUILD}"
		}
	}
	BLOCK "VarFileInfo" {
		VALUE "Translation", 0x0409, 0x04B0
	}
}
