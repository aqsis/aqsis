// texer.cpp : Defines the entry point for the console application.
//

#include	<conio.h>
#include	<stdio.h>

#ifdef	WIN32
#pragma warning (disable : 4100)
#endif

#include	"aqsis.h"
#include	"version.h"
#include	"arg.h"

int g_cFiles=0;
char* g_aFiles[7];

TqInt	envcube;
TqChar*	swrap="black";
TqChar*	twrap="black";

static void arg_filename(int argc, char**argv);

int main(int argc, char* argv[])
{
	printf("texer version %s - Copyright 2000 Paul C. Gregory\n", VERSION_STR);
	printf("All Rights Reserved\n\n");

    if (arg_parse(argc, argv,
		"", "Usage: %s [options] infile[s] outfile", argv[0],
		"-envcube", ARG_FLAG(&envcube), "px nx py ny pz nz out\n\t\t\tprocess input images as a cubic environment map.",
		"-swrap %S", &swrap, "s mode [black|periodic|clamp]",
		"-twrap %S", &twrap, "t mode [black|periodic|clamp]",
		"", ARG_SUBR(arg_filename), "",
		0) >= 0)
	{
		if(envcube)
		{
			if(g_cFiles!=7)	
			{
				printf("Need 6 images for cubic environment map\n");
				return(-1);
			}
			printf("CubeFace Environment %s,%s,%s,%s,%s,%s-->%s\n",g_aFiles[0], g_aFiles[1],
																   g_aFiles[2], g_aFiles[3],
																   g_aFiles[4], g_aFiles[5],
																   g_aFiles[6]);
			RiMakeCubeFaceEnvironment(g_aFiles[0],g_aFiles[1],g_aFiles[2],g_aFiles[3],g_aFiles[4],g_aFiles[5],g_aFiles[6],
									  0,RiBoxFilter,0,0);
		}
		else
		{
			if(g_cFiles<2)
			{
				printf("Must specify input and output names\n");
				return(-1);
			}
			printf("Texture %s-->%s, smode \"%s\", tmode \"%s\"\n",g_aFiles[0], g_aFiles[1], swrap, twrap);
			RiMakeTexture(g_aFiles[0],g_aFiles[1],swrap,twrap,RiBoxFilter,0,0);
		}
	}
	return(0);
}


static void arg_filename(int argc, char**argv)
{
    int i;

	for(i=0; i<argc; i++)
	{
		if(g_cFiles<7)	g_aFiles[g_cFiles]=argv[i];
		g_cFiles++;
	}
}
