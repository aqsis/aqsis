
Win32


Requirements:
	
	Tools:

		MSVC++ 6.0
		flex_pp			(ftp://ftp.th-darmstadt.de/pub/programming/languages/C++/tools/flex++bison++/LATEST/)
		bison_pp		(ftp://ftp.th-darmstadt.de/pub/programming/languages/C++/tools/flex++bison++/LATEST/)
			Make sure bison_pp and flex_pp are available in the executables directory from MSDev Tools/Options settings.
		


	Libraries:

		libtiff		3.5.5 or later	(http://www.libtiff.org)
					Load the file libtiff\libtiff.def into you text editor and add the line
						TIFFCreateDirectory
					From within the libtiff directory type
						nmake -f makefile.vc libtiff.dll
					Add libtiff to your include paths under tools/options.
					Add libtiff to your library paths under tools/options.
		zlib		1.1.3 or later	(http://www.info-zip.org/pub/infozip/zlib/)
					From the install directory type
						nmake -f Makefile.nt
					Add the install directory to your include paths under tools/options.
					Add the install directory to your library paths under tools/options.
		libarg		1.0-1 or later	(http://www.cs.cmu.edu/~ph/859E/src/libarg/)
					In arg.c change bcopy to memcpy and swap the first two arguments.
					Create a new win32 library project and add arg.c, expr.c, expr.h, simple.c and arg.h.
					Add M_PI=3.1416 to the preprocessor definitions.
					Change the output directory to be the same for both release and debug.
					Change the output library name to libarg.lib for the release build and libargd.lib for the debug.
					Change the project settings/C\C++/Code Generation/Use runtime library to Debug Multithreaded DLL 
						for the debug version.
					Change the project settings/C\C++/Code Generation/Use runtime library to Multithreaded DLL for 
						the release version.
					Add the output directory to your include paths under tools/options.
					Add the output directory to your library paths under tools/options.


	Load the Renderer.dsw workspace from the Aqsis/Renderer directory, select the All project and build all.
