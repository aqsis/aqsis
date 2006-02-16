// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Implements a PNG importer
		\author Michel Joron (joron@sympatico.ca)
*/


// The conversion process assumes a minimal conversion
// the function to do convertion must be name
// as the name of the dll or .so on Unix
// The conversion must create a valid .tif file and return
// the filename of the new file created.
// Eg. this function png2tif() created a .tif file at the same place
// as the .png filename provided.
//
// It uses png2ppm from the standard libpng distribution and than
// load the ppm2tif.dll/so from aqsis directory to convert the .ppm to
// .tif file and at the end delete .ppm file
// This illustrates the usage of plugins which call other plugins
//   The goal of the plugin is not but the complexity of decode .png in there but
//   reusage work already done by other plugins.
//
//
// IN ORDER TO CONVERSION OCCURRED the dll/dso must be present in
// procedures directory under <AQSIS_BASE_PATH>
// and png2ppm must be found in the current path environment variable.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef   __GNUC__
#include <unistd.h>
#endif   // 

/* #define MAIN / usefull to debug an autonomous png2tif converter */


static char tiffname[1024];

#ifdef	WIN32
#define	__export	__declspec(dllexport)
#else	// WIN32
#define	__export
#endif	// WIN32

/* Main function to convert any png to tif format
 * It used the standard png2ppm toolin the standard libpng distribution
 * than it used ppm2tif standard plugin.
 */
__export char *png2tif(char *in)
{
	int errcode;
	char cmd[1024];
	char *result =NULL;
	extern char *ppm2tif(char *in);


	strcpy(tiffname, in);
	if ((result = strstr(tiffname, ".png")) != 0)
		strcpy(result, ".ppm");
	if (!result)
	{
		if ((result = strstr(tiffname, ".PNG")) != 0)
			strcpy(result, ".ppm");
	}
	if (!result)
		return result;

#ifdef AQSIS_SYSTEM_WIN32

	sprintf(cmd, "png2pnm.exe %s > %s", in, tiffname);
#else

	sprintf(cmd, "png2pnm %s > %s", in, tiffname);
#endif

	errcode = system(cmd);

	if (errcode == 0)
	{
		/* SUCCESS */

		char *ppmresult = ppm2tif(tiffname);

		/* delete the temporary .ppm file */
		unlink(tiffname);
		if (ppmresult)
		{
			strcpy(tiffname, ppmresult);
			result = tiffname;
		}
		else
			result = NULL;
	}
	else
	{
		perror(cmd);
		result = NULL;
	}

	return result;
}

#ifdef MAIN
int main(int argc, char *argv[])
{
	char *result;

	if (argc != 2)
	{
		fprintf(stderr, "Usage %s: %s some.png", argv[0], argv[1]);
		exit(2);
	}
	result = png2tif(argv[1]);
	if (result)
	{
		puts(result);
	}
	return 1;
}
#endif
