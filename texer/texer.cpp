// texer.cpp : Defines the entry point for the console application.
//

#include	<conio.h>
#include	<stdio.h>

#ifdef	AQSIS_SYSTEM_WIN32
#pragma warning (disable : 4100)
#endif // AQSIS_SYSTEM_WIN32

#include	"aqsis.h"
#include	"version.h"
#include	"argparse.h"
#include	"ri.h"

bool				g_envcube;
ArgParse::apstring	g_swrap="black";
ArgParse::apstring	g_twrap="black";

static void arg_filename(int argc, char**argv);

int main(int argc, const char** argv)
{
	std::cout << "texer version " << VERSION_STR << " - Copyright 2000 Paul C. Gregory" << std::endl;
	std::cout << "All Rights Reserved" << std::endl;

	ArgParse ap;
	ap.usageHeader(ArgParse::apstring("Usage: ") + argv[0] + " [options] outfile");
	ap.argFlag("envcube", " px nx py ny pz nz\aproduce a cubeface environment map from 6 images.", &g_envcube);
	ap.argString("swrap", "=string\as mode [black|periodic|clamp]", &g_swrap);
	ap.argString("twrap", "=string\at mode [black|periodic|clamp]", &g_twrap);
	if (!ap.parse(argc-1, argv+1) || ap.leftovers().size()<=1)
	{
		std::cerr << ap.errmsg() << std::endl << ap.usagemsg();
		exit(1);
	}

	if(g_envcube)
	{
		if(ap.leftovers().size()!=7)	
		{
			std::cerr << "Need 6 images for cubic environment map" << std::endl;
			return(-1);
		}
		std::cout << "CubeFace Environment " << ap.leftovers()[0] << "," <<
												ap.leftovers()[1] << "," <<
												ap.leftovers()[2] << "," <<
												ap.leftovers()[3] << "," <<
												ap.leftovers()[4] << "," << 
												ap.leftovers()[5] << "-->" <<
												ap.leftovers()[6] << std::endl;
		RiMakeCubeFaceEnvironment(ap.leftovers()[0].c_str(),ap.leftovers()[1].c_str(),ap.leftovers()[2].c_str(),ap.leftovers()[3].c_str(),ap.leftovers()[4].c_str(),ap.leftovers()[5].c_str(),ap.leftovers()[6].c_str(),0,RiBoxFilter,0,0);
	}
	else
	{
		std::cout << "Texture " << ap.leftovers()[0] << "-->" << ap.leftovers()[1] << 
					 " smode \"" << g_swrap.c_str() << "\" tmode \"" << g_twrap.c_str() << "\"" << std::endl;
		RiMakeTexture((char*)ap.leftovers()[0].c_str(),(char*)ap.leftovers()[1].c_str(),(char*)g_swrap.c_str(),(char*)g_twrap.c_str(),RiBoxFilter,0,0);
	}
	return(0);
}
