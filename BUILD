
Win32


Requirements:
	
	Tools:

		MSVC++ 6.0
		flex_pp			(ftp://ftp.th-darmstadt.de/pub/programming/languages/C++/tools/flex++bison++/LATEST/)
		bison_pp		(ftp://ftp.th-darmstadt.de/pub/programming/languages/C++/tools/flex++bison++/LATEST/)
			Make sure bison_pp and flex_pp are available in the executables directory from MSDev Tools/Options settings.
		


	Libraries:

		libtiff		3.5.5 or later	(http://www.libtiff.org)
					Load the file libtiff\libtiff.def into you text editor and add the lines
						TIFFCreateDirectory
						TIFFDefaultStripSize
					From within the libtiff directory type
						nmake -f makefile.vc libtiff.dll
					Add libtiff to your include paths under tools/options.
					Add libtiff to your library paths under tools/options.
		zlib		1.1.3 or later	(http://www.info-zip.org/pub/infozip/zlib/)
					From the install directory type
						nmake -f Makefile.nt
					Add the install directory to your include paths under tools/options.
					Add the install directory to your library paths under tools/options.


	Load the Renderer.dsw workspace from the Aqsis/Renderer directory, select the All project and build all.
