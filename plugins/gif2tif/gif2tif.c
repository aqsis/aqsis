// Aqsis
// Copyright  1997 - 2001, Paul C. Gregory
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
		\brief Implements a gif importer
		\author Michel Joron (joron@sympatico.ca)
*/


// The conversion process assumes a minimal conversion
// the function to do convertion must be name 
// as the name of the dll or .dso on Unix 
// The conversion must create a valid .tif file and return
// the filename of the new file created.
// Eg. this function gif2tif() created a .tif file at the same place
// as the .gif filename provided.
// Minimal implemantation is just doing a call to gif2tif.exe provided with the
// standard libtiff source codes. 
// IN ORDER TO CONVERSION OCCURRED the dll/dso must be present in 
// procedures directory under <AQSIS_BASE_PATH>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define	_qShareName gif2tif
#include	"share.h"

static char tiffname[1024];

/* Main function to convert any gif to tif format 
 * It used the standard tiff tool name gif2tiff.exe on PC
 * or gif2tiff on unix
 */
_qShareM char *gif2tif(char *in)
{

int errcode;
char cmd[1024];
char *result =NULL;

    
    strcpy(tiffname, in);
	if ((result = strstr(tiffname, ".gif")) != 0) strcpy(result, ".tif");
	if (!result) 
	{
		if ((result = strstr(tiffname, ".GIF")) != 0) strcpy(result, ".tif");
	}
	if (!result) return result;

#ifdef AQSIS_SYSTEM_WIN32

    sprintf(cmd, "gif2tiff.exe %s %s", in, tiffname);
#else
	sprintf(cmd, "gif2tiff %s %s", in, tiffname);
#endif
   errcode = system(cmd);

   if (errcode == 0) 
   {
		/* SUCCESS */
		result = tiffname;
   } 
   return result;
}


#ifdef MAIN
int main(int argc, char *argv[])
{
char *result;

   if (argc != 2) {
      fprintf(stderr, "Usage %s: %s some.gif", argv[0], argv[1]);
      exit(2);
   }
   result = gif2tif(argv[1]);
   if (result) {
      puts(result);
   }
   return 1;
}
#endif
