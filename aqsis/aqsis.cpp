#include "ri.h"
#include "librib.h"
#include "librib2ri.h"
#include "aqsis.h"


#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#include "version.h"
#endif

#include <argparse.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>

#include <stdio.h>

void RenderFile(FILE* file, const char* name);
void GetOptions();

bool g_pause;
#ifdef	_DEBUG
int g_endofframe=3;
#else
int g_endofframe=0;
#endif
bool g_nostandard=0;
bool g_help=0;
bool g_version=0;
bool g_verbose=0;
bool g_fb=0;
ArgParse::apstring g_config="";
ArgParse::apstring g_shaders="";
ArgParse::apstring g_archives="";
ArgParse::apstring g_textures="";
ArgParse::apstring g_displays="";
ArgParse::apstring g_base_path="";
ArgParse::apstring g_type="";
ArgParse::apstring g_addtype="";
ArgParse::apstring g_mode="rgba";

void version(std::ostream& Stream)
{
#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
	Stream << "aqsis version " << VERSION_STR << std::endl;
#else
	Stream << "aqsis version " << VERSION << std::endl;
#endif
}


/** Function to print the progress of the render.
	Used as the callback function to a RiProgressHandler call.
 */
RtVoid PrintProgress(RtFloat percent)
{
	std::cout << std::setw(6) << std::setfill(' ') << std::setprecision(4) << percent << "% Complete\r" << std::flush;
	return;
}


/** Function to setup specific options needed after world loading but before rendering.
	Used as the callback function to a RiPreRenderFunction call.
 */
RtVoid PreRender()
{
	if(g_fb)
	{
		char* type="framebuffer", *mode="rgba";
		RiDisplay("aqsis",type,mode,NULL);
	}
	else if(g_type.compare("")!=0)
	{
		char type[256],mode[256];
		strcpy(type,g_type.c_str());
		strcpy(mode,g_mode.c_str());
		RiDisplay("aqsis",type,mode,NULL);
	}
	else if(g_addtype.compare("")!=0)
	{
		char type[256],mode[256];
		strcpy(type,g_addtype.c_str());
		strcpy(mode,g_mode.c_str());
		RiDisplay("+aqsis",type,mode,NULL);
	}
	return;
}


int main(int argc, const char** argv)
{
	ArgParse ap;
	ap.usageHeader(ArgParse::apstring("Usage: ") + argv[0] + " [options] files(s) to render");
	ap.argFlag("help", "\aprint this help and exit", &g_help);
	ap.argFlag("version", "\aprint version information and exit", &g_version);	
	ap.argFlag("pause", "\await for a keypress on completion", &g_pause);
	ap.argInt("endofframe", "=integer\aequivalent to \"endofframe\" option", &g_endofframe);
	ap.argFlag("nostandard", "\adisables declaration of standard RenderMan parameter types", &g_nostandard);
	ap.argFlag("verbose", "\aoutput environment information", &g_verbose);
	ap.argString("type", "=string\aspecify a display device type to use", &g_type);
	ap.argString("addtype", "=string\aspecify a display device type to add", &g_addtype);
	ap.argString("mode", "=string\aspecify a display device mode to use", &g_mode);
	ap.argFlag("fb", "\aequivalent to --type=\"framebuffer\" --mode=\"rgba\"", &g_fb);
	ap.argString("config", "=string\aspecify a configuration file to load", &g_config);
	ap.argString("base", "=string\aspecify a default base path", &g_base_path);
	ap.argString("shaders", "=string\aspecify a default shaders searchpath", &g_shaders);
	ap.argString("archives", "=string\aspecify a default archives searchpath", &g_archives);
	ap.argString("textures", "=string\aspecify a default textures searchpath", &g_textures);
	ap.argString("displays", "=string\aspecify a default displays searchpath", &g_displays);

	if (argc>1 && !ap.parse(argc-1, argv+1))
	{
		std::cerr << ap.errmsg() << std::endl << ap.usagemsg();
		exit(1);
	}

	if(g_help)
	{
		std::cout << ap.usagemsg();
		exit(0);
	}

	if(g_version)
	{
		version(std::cout);
		exit(0);
	}

	GetOptions();

	if(g_verbose)
	{
		std::cout << "config:   " << g_config.c_str() << std::endl;
		std::cout << "base:     " << g_base_path.c_str() << std::endl;
		std::cout << "shaders:  " << g_shaders.c_str() << std::endl;
		std::cout << "archives: " << g_archives.c_str() << std::endl;
		std::cout << "textures: " << g_textures.c_str() << std::endl;
		std::cout << "displays: " << g_displays.c_str() << std::endl;
	}

	if(ap.leftovers().size()==0) // If no files specified, take input from stdin.
	{
		RenderFile(stdin, "stdin");
	}
	else
	{
		for (ArgParse::apstringvec::const_iterator e = ap.leftovers().begin(); e != ap.leftovers().end(); e++)
		{
			FILE *file=fopen(e->c_str(),"rb");
			if (file!=NULL) {
			    RenderFile(file, e->c_str());
			    fclose(file);
			} else {
			    std::cout << "Warning: Cannot open file \"" << *e << "\"" << std::endl;
			}
		}
	}

	return(0);
}


void GetOptions()
{
	char* env;
		// If --base not specified, check for env.
	if(g_base_path.compare("")==0)
	{
		if((env=getenv("AQSIS_BASE_PATH"))!=NULL)
			g_base_path=env;
		else
			g_base_path=".";

	}

		// If --config not specified try to locate the config file.
	if(g_config.compare("")==0)
	{
		if((env=getenv("AQSIS_CONFIG"))!=NULL)
			g_config=env;
		else
			g_config=g_base_path;
			g_config.append("/.aqsisrc");
			std::ifstream cfgfile(g_config.c_str());
			if(!cfgfile.is_open())
				if((env=getenv("HOME"))!=NULL)
				{
					g_config=env;
					g_config.append("/.aqsisrc");
					std::ifstream cfgfile(g_config.c_str());
					if(!cfgfile.is_open())
					{
						g_config="/etc/.aqsisrc";
					}
				}
	}

		// if --shaders is not specified, try and get a default shaders searchpath.
	if(g_shaders.compare("")==0)
	{
		if((env=getenv("AQSIS_SHADERS_PATH"))!=0)
			g_shaders=env;
		else
		{
			g_shaders=g_base_path;
			g_shaders.append("/shaders");
		}
	}

		// if --archives is not specified, try and get a default archives searchpath.
	if(g_archives.compare("")==0)
	{
		if((env=getenv("AQSIS_ARCHIVES_PATH"))!=0)
			g_archives=env;
		else
		{
			g_archives=g_base_path;
			g_archives.append("/archives");
		}
	}

		// if --textures is not specified, try and get a default textures searchpath.
	if(g_textures.compare("")==0)
	{
		if((env=getenv("AQSIS_TEXTURES_PATH"))!=0)
			g_textures=env;
		else
		{
			g_textures=g_base_path;
			g_textures.append("/textures");
		}
	}

		// if --displays is not specified, try and get a default displays searchpath.
	if(g_displays.compare("")==0)
	{
		if((env=getenv("AQSIS_DISPLAYS_PATH"))!=0)
			g_displays=env;
		else
		{
			g_displays=g_base_path;
			g_displays.append("/displays");
		}
	}
}

void RenderFile(FILE* file, const char* name)
{
	librib2ri::Engine renderengine;

	RiBegin("CRIBBER");

	if(!g_nostandard)
		librib::StandardDeclarations(renderengine);

	RiOption("statistics", "endofframe", &g_endofframe, RI_NULL);
	const char* popt[1];
	popt[0]=g_shaders.c_str();
	RiOption("searchpath", "shader", &popt,RI_NULL);
	popt[0]=g_archives.c_str();
	RiOption("searchpath", "archive", &popt,RI_NULL);
	popt[0]=g_textures.c_str();
	RiOption("searchpath", "texture", &popt,RI_NULL);
	popt[0]=g_displays.c_str();
	RiOption("searchpath", "display", &popt,RI_NULL);

	RiProgressHandler(&PrintProgress);
	RiPreRenderFunction(&PreRender);

	if(g_config.compare(""))
	{
		FILE *cfgfile=fopen(g_config.c_str(),"rb");
		if(cfgfile!=NULL)
		{
			librib::Parse(cfgfile,"config",renderengine,std::cerr);
			fclose(cfgfile);
		}
		else if(g_verbose)
		{
#ifdef  AQSIS_SYSTEM_WIN32
			std::cout << "Warning: Config file not found in" << std::endl <<
						 "%AQSIS_CONFIG%" << std::endl <<
						 "%AQSIS_BASE_PATH%/.aqsisrc" << std::endl << 
						 "%HOME%/.aqsisrc" << std::endl <<
						 "/etc/.aqsisrc" << std::endl;
#else
			std::cout << "Warning: Config file not found in" << std::endl <<
						  "$AQSIS_CONFIG" << std::endl <<
						  "$AQSIS_BASE_PATH/.aqsisrc" << std::endl <<
						  "$HOME/.aqsisrc" << std::endl <<
						  "/etc/.aqsisrc" << std::endl;
#endif
		}
	}
	librib::Parse(file,name,renderengine,std::cerr);

	RiEnd();
}
