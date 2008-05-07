#include <Windows.h>

#define VER_FILEVERSION             ${MAJOR},${MINOR},${BUILD},0
#define VER_FILEVERSION_STR         "${MAJOR}.${MINOR}.${BUILD}.0\0"

#define VER_PRODUCTVERSION          ${MAJOR},${MINOR},0,0
#define VER_PRODUCTVERSION_STR      "${MAJOR}.${MINOR}\0"

#ifndef DEBUG
#define VER_DEBUG                   0
#else
#define VER_DEBUG                   VS_FF_DEBUG
#endif

VS_VERSION_INFO VERSIONINFO
FILEVERSION    	VER_FILEVERSION
PRODUCTVERSION 	VER_PRODUCTVERSION
FILEFLAGSMASK  	VS_FFI_FILEFLAGSMASK
FILEFLAGS      	(VER_DEBUG)
FILEOS         	VOS__WINDOWS32
FILETYPE       	VFT_DLL
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904E4"
        BEGIN
            VALUE "CompanyName",      "${PACKAGE_VENDOR}"
            VALUE "FileDescription",  "${PACKAGE_NAME}"
            VALUE "FileVersion",      VER_FILEVERSION_STR
            VALUE "LegalCopyright",   "${PACKAGE_COPYRIGHT}"
            VALUE "LegalTrademarks1", "${PACKAGE_COPYRIGHT_OTHER}"
            VALUE "ProductName",      "${CMAKE_PROJECT_NAME}"
            VALUE "ProductVersion",   VER_PRODUCTVERSION_STR
        END
    END

    BLOCK "VarFileInfo"
    BEGIN
        /* The following line should only be modified for localized versions.     */
        /* It consists of any number of WORD,WORD pairs, with each pair           */
        /* describing a language,codepage combination supported by the file.      */
        /*                                                                        */
        /* For example, a file might have values "0x409,1252" indicating that it  */
        /* supports English language (0x409) in the Windows ANSI codepage (1252). */

        VALUE "Translation", 0x409, 1252

    END
END