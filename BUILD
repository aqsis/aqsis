
Win32


Requirements:
	
	Tools:

		MSVC++ 6.0
		flex_pp			(ftp://ftp.th-darmstadt.de/pub/programming/languages/C++/tools/flex++bison++/LATEST/)
		bison_pp		(ftp://ftp.th-darmstadt.de/pub/programming/languages/C++/tools/flex++bison++/LATEST/)
			Make sure bison_pp and flex_pp are available in the executables directory from MSDev Tools/Options settings.
		


	Libraries:

		libtiff		3.5.5 or later	(http://www.libtiff.org)
			NOTE: Some modifications will need to be made to the standard 3.5.5, two additional 
				esports need to be added to the libtiff.def file
					TIFFCreateDirectory
					TIFFDefaultStripSize
		zlib		1.1.3 or later	(http://www.info-zip.org/pub/infozip/zlib/)
		libarg		1.0-1 or later	(http://www.cs.cmu.edu/~ph/859E/src/libarg/)
			NOTE: Small change to arg.c to get it to work under windows
				Change bcopy to memcpy and swap the first two arguments.
				Also need to define M_PI on the command line.
				To work with Aqsis, you should build two versions of this library,
					libarg.lib	-	Linked with MSVC runtime "Multithreaded DLL" for release.
					libargd.lib	-	Linked with MSVC runtime "Debug Multithreaded DLL" for debug.


	All these libraries must be available from the standard include/library directories in MSDev Tools/Options settings.


	Load the Renderer.dsw workspace from the Aqsis/Renderer directory, select the All project and build all.
