// teqser.cpp : Defines the entry point for the console application.
//

#include	<stdio.h>


#ifdef	AQSIS_SYSTEM_WIN32
#include	<conio.h>
#pragma warning (disable : 4100)
#endif // AQSIS_SYSTEM_WIN32

#include	"aqsis.h"
#include	"version.h"
#include	"argparse.h"
#include	"ri.h"

bool				g_version=0;
bool				g_help=0;
bool				g_envcube;
bool				g_shadow;
ArgParse::apstring	g_swrap="black";
ArgParse::apstring	g_twrap="black";
ArgParse::apstring  g_filter="box";
ArgParse::apfloat   g_swidth = 1.0;
ArgParse::apfloat   g_twidth = 1.0;
ArgParse::apfloat   g_fov = 0.0;


void version(std::ostream& Stream)
{
#ifdef	AQSIS_SYSTEM_WIN32
	Stream << "teqser version " << VERSION_STR << std::endl;
#else
	Stream << "teqser version " << VERSION << std::endl;
#endif
}


static void arg_filename(int argc, char**argv);

int main(int argc, const char** argv)
{
	ArgParse ap;
	RtFilterFunc filterfunc;

	ap.usageHeader(ArgParse::apstring("Usage: ") + argv[0] + " [options] outfile");
	ap.argFlag("help", "\aprint this help and exit", &g_help);
	ap.argFlag("version", "print version information and exit", &g_version);	
	ap.argFlag("envcube", " px nx py ny pz nz\aproduce a cubeface environment map from 6 images.", &g_envcube);
	ap.argFlag("shadow", " produce a shadow map from a z file.", &g_shadow);
	ap.argString("swrap", "=string s wrap [black|periodic|clamp]", &g_swrap);
	ap.argString("twrap", "=string t wrap [black|periodic|clamp]", &g_twrap);
	ap.argString("filter", "=string [sinc|box|catmull-rom|triangle|disk|bessel]", &g_filter);
	ap.argFloat("fov(envcube)", "=float [>=0.0f]", &g_fov);
	ap.argFloat("swidth", "=float s width [>0.0f]", &g_swidth);
	ap.argFloat("twidth", "=float t width [>0.0f]", &g_twidth);

	if (argc>1 && !ap.parse(argc-1, argv+1))
	{
		std::cerr << ap.errmsg() << std::endl << ap.usagemsg();
		exit(1);
	}

	if(g_version)
	{
		version(std::cout);
		exit(0);
	}

	if(g_help || ap.leftovers().size()<=1)
	{
		std::cout << ap.usagemsg();
		exit(0);
	}

	if(g_envcube && g_shadow)
	{
		std::cout << "Specify only one of envcube or shadow" << std::endl;
		exit(1);
	}


	/* find the pixel's filter function */
	filterfunc = RiBoxFilter;
	if (g_filter=="box")
		filterfunc = RiBoxFilter;
	else if (g_filter=="sinc")
		filterfunc = RiSincFilter;
	else if (g_filter=="catmull-rom")
		filterfunc =RiCatmullRomFilter;
	else if (g_filter=="disk")
		filterfunc =RiDiskFilter;
	else if (g_filter=="bessel")
		filterfunc =RiBesselFilter;
	else if (g_filter=="triangle")
		filterfunc = RiTriangleFilter;
	

	/* protect the s,t width */
	if (g_swidth < 1.0) {
		std::cerr << "g_swidth is smaller than 1.0." << " 1.0 will be used instead." << std::endl;
		g_swidth = 1.0;
	}
	if (g_twidth < 1.0) {
		std::cerr << "g_twidth is smaller than 1.0." << " 1.0 will be used instead." << std::endl;
		g_twidth = 1.0;
	}

	/* protect the s,t wrap mode */
	if (!((g_swrap=="black") || (g_swrap=="periodic") || (g_swrap=="clamp") ) ) {
		std::cerr  << "Unknown s wrap mode: " << g_swrap << ". black will be used instead." << std::endl;
		g_swrap = "black";
	}
	if (!((g_twrap=="black") || (g_twrap=="periodic") || (g_twrap=="clamp") ) ) {
		std::cerr  << "Unknow t wrap mode: " << g_twrap << ". black will be used instead." << std::endl;
		g_twrap = "black";
    }

	RiBegin("teqser");

	if(g_envcube)
	{
		if(ap.leftovers().size()!=7)	
		{
			std::cerr << "Need 6 images for cubic environment map" << std::endl;
			return(-1);
		}

		printf("CubeFace Environment %s %s %s %s %s %s ----> %s \n\t\"fov\"= %4.1f\n\t\"filter\"= %s \n\t\"swidth\"= %4.1f\n\t\"twidth\"= %4.1f\n",
			            (char*)ap.leftovers()[0].c_str(),
						(char*)ap.leftovers()[1].c_str(),
						(char*)ap.leftovers()[2].c_str(),
						(char*)ap.leftovers()[3].c_str(),
						(char*)ap.leftovers()[4].c_str(),
						(char*)ap.leftovers()[5].c_str(),
						(char*)ap.leftovers()[6].c_str(),
						g_fov,
						g_filter.c_str(),
						g_swidth,
						g_twidth
						);

		RiMakeCubeFaceEnvironment(ap.leftovers()[0].c_str(),ap.leftovers()[1].c_str(),ap.leftovers()[2].c_str(),
			ap.leftovers()[3].c_str(),ap.leftovers()[4].c_str(),ap.leftovers()[5].c_str(),ap.leftovers()[6].c_str(),
			g_fov,
			filterfunc, (float) g_swidth, (float) g_twidth);
	}
	else if(g_shadow)
	{
		printf("Shadow %s ----> %s \n",
			                        (char*)ap.leftovers()[0].c_str(),
									(char*)ap.leftovers()[1].c_str());
												
	
		
		RiMakeShadow((char*)ap.leftovers()[0].c_str(),(char*)ap.leftovers()[1].c_str());
	}
	else
	{
	    printf("Texture %s ----> %s \n\t\"swrap\"= %s \n\t\"twrap\"= %s \n\t\"filter\"= %s \n\t\"swidth\"= %4.1f \n\
\t\"twidth\"= %4.1f\n",
			    (char*)ap.leftovers()[0].c_str(),
				(char*)ap.leftovers()[1].c_str(),
				(char*)g_swrap.c_str(),
				(char*)g_twrap.c_str(),
				(char*)g_filter.c_str(),
				g_swidth,
				g_twidth
				);
	
	    RiMakeTexture((char*)ap.leftovers()[0].c_str(),(char*)ap.leftovers()[1].c_str(),
			            (char*)g_swrap.c_str(),(char*)g_twrap.c_str(), filterfunc, 
						(float)g_swidth, (float)g_twidth);
	
	}

	RiEnd();

	return(0);
}
