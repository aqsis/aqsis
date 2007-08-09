// Aqsis
// Copyright © 1997 - 2006, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
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
 		\brief Implements based on the number of plugins for texturemapping in Aqsis
 create a .rib file which allow to convert all the files possible with minimum user inputs.  later one you could edit the output if you want.
 Right Now I assume all the plugins are available; ideal for testing aqsis plugins handling.
		\author Michel Joron joron@sympatico.ca
*/


#include <stdio.h>  /* fopen, printf(), popen,pclose,perror() */
#include <string.h> /* strcpy, strstr() */



static char *extensions[] = {
                                ".tga",
                                ".jpg",
                                ".bmp",
                                ".pcx",
                                ".ppm",
                                ".gif",
                                ".png",
                                ".tif",
                                ".exr",
                                ".TGA",
                                ".JPG",
                                ".BMP",
                                ".PCX",
                                ".PPM",
                                ".GIF",
                                ".PNG",
                                ".TIF",
                                ".EXR",
                                "\0"
                            };

static void modif( char *news, char *tx, char *old)
{
	char *pt;
	int i;
	int n;

	n = 1;

	for (i=0; extensions[i][0] != '\0'; i++)
		n ++;
	n /= 2;

	strcpy(news, old);
	for (i=0; extensions[i][0] != '\0'; i++)
	{
		if ( (pt = strstr(news, extensions[i])) != NULL)
		{
			if (i > n -1)
				strcpy(pt, extensions[i-n]);
			else
				strcpy(pt, extensions[i]);
			strcpy(tx, old);
			if ((pt = strstr(tx, extensions[i])) != NULL)
				strcpy(pt, ".tx");
		}
	}
}

static void putsargc(int argc, char *argv[])
{
	int i;
	for (i=1; i < argc; i++)
		printf(" \"%s\"", argv[i]);
	printf("\n");

}

int main(int argc, char *argv[])
{
	char psBuffer[129];
	char txname[128], newname[128];
	FILE *chkdsk;

#ifdef WIN32

	if( (chkdsk = _popen( "dir /b *.jpg *.tga *.bmp *.png *.pcx *.ppm *.gif *.tif *.exr", "rt" )) != NULL )
	{
#else
	if( (chkdsk = popen( "ls *.jpg *.tga *.bmp *.png *.gif *.pcx *.ppm *.JPG *.TGA *.BMP *.PNG *.PCX *.PPM *.GIF *.tif *.TIF *.exr *.EXR", "rt" )) != NULL )
	{
#endif

		printf("Option \"limits\" \"texturememory\" 8196\n");
		while( !feof( chkdsk ) )
		{
			if( fgets( psBuffer, 128, chkdsk ) != NULL )
			{
				modif(newname, txname, psBuffer);
				printf( "MakeTexture \"%s\" \"%s\" \"periodic\"  \"periodic\" \"box\" 1.0 1.0", newname, txname );
				putsargc(argc,argv);
			}
		}
#ifdef WIN32
		_pclose(chkdsk);
#else

		pclose(chkdsk);
#endif

	}

}
