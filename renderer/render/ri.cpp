// Aqsis
// Copyright 1997 - 2001, Paul C. Gregory
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
		\brief Implement the majority of the RenderMan API functions.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"MultiTimer.h"

#include	"aqsis.h"

#include	<stdarg.h>
#include	<math.h>
#include	<list>

#include	"imagebuffer.h"
#include	"lights.h"
#include	"renderer.h"
#include	"patch.h"
#include	"polygon.h"
#include	"nurbs.h"
#include	"symbols.h"
#include	"bilinear.h"
#include	"quadrics.h"
#include	"teapot.h"
#include	"shaders.h"
#include	"texturemap.h"
#include	"objectinstance.h"
#include	"trimcurve.h"
#include	"genpoly.h"
#include	"points.h"
#include	"curves.h"
#include	"rifile.h"
#include	"librib2ri.h"
#include	"converter.h"
#include	"shadervm.h"
#include 	"librib.h"
#include 	"libribtypes.h"
#include 	"parserstate.h"
#include	"procedural.h"
#include "logging.h"
#include "logging_streambufs.h"

#include	"ri_cache.h"

#include	"subdivision2.h"
#include	"condition.h"

#ifndef    AQSIS_SYSTEM_WIN32
#include        "unistd.h"
#else
#include	"io.h"
#endif /* AQSIS_SYSTEM_WIN32 */

// These are needed to allow calculation of the default paths
#ifdef AQSIS_SYSTEM_WIN32
  #include <windows.h>
  #ifdef _DEBUG
    #include <crtdbg.h>
extern "C" __declspec(dllimport) void report_refcounts();
#endif // _DEBUG
#endif // !AQSIS_SYSTEM_WIN32

#if defined(AQSIS_SYSTEM_MACOSX)
#include "Carbon/Carbon.h"
#endif


#include	"ri.h"

#include	"sstring.h"

//#include	"share.h"
#include	"validate.h"

#include	"ri_validate.inl"

using namespace Aqsis;

#include	"ri_debug.inl"

static RtBoolean ProcessPrimitiveVariables( CqSurface* pSurface, PARAMETERLIST );
static void ProcessCompression( TqInt *compress, TqInt *quality, TqInt count, RtToken *tokens, RtPointer *values );
RtVoid	CreateGPrim( const boost::shared_ptr<CqSurface>& pSurface );
void SetShaderArgument( const boost::shared_ptr<IqShader>& pShader, const char* name, TqPchar val );
TqBool	ValidateState(...);

TqBool   IfOk = TqTrue;

#define Validate_Conditional \
	{\
	if (!IfOk) \
		return;\
	}

#define Validate_Conditional0 \
	{\
	if (!IfOk) \
		return 0;\
	}

//---------------------------------------------------------------------
// This file contains the interface functions which are published as the
//	Renderman Interface SPECification (C) 1988 Pixar.
//

CqRandom worldrand;

//---------------------------------------------------------------------
// Interface parameter token strings.


RtToken	RI_FRAMEBUFFER	= "framebuffer";
RtToken	RI_FILE	= "file";
RtToken	RI_RGB	= "rgb";
RtToken	RI_RGBA	= "rgba";
RtToken	RI_RGBZ	= "rgbz";
RtToken	RI_RGBAZ	= "rgbaz";
RtToken	RI_A	= "a";
RtToken	RI_Z	= "z";
RtToken	RI_AZ	= "az";
RtToken	RI_MERGE	= "merge";
RtToken	RI_ORIGIN	= "origin";
RtToken	RI_PERSPECTIVE	= "perspective";
RtToken	RI_ORTHOGRAPHIC	= "orthographic";
RtToken	RI_HIDDEN	= "hidden";
RtToken	RI_PAINT	= "paint";
RtToken	RI_CONSTANT	= "constant";
RtToken	RI_SMOOTH	= "smooth";
RtToken	RI_FLATNESS	= "flatness";
RtToken	RI_FOV	= "fov";

RtToken	RI_AMBIENTLIGHT	= "ambientlight";
RtToken	RI_POINTLIGHT	= "pointlight";
RtToken	RI_DISTANTLIGHT	= "distantlight";
RtToken	RI_SPOTLIGHT	= "spotlight";
RtToken	RI_INTENSITY	= "intensity";
RtToken	RI_LIGHTCOLOR	= "lightcolor";
RtToken	RI_FROM	= "from";
RtToken	RI_TO	= "to";
RtToken	RI_CONEANGLE	= "coneangle";
RtToken	RI_CONEDELTAANGLE	= "conedeltaangle";
RtToken	RI_BEAMDISTRIBUTION	= "beamdistribution";
RtToken	RI_MATTE	= "matte";
RtToken	RI_METAL	= "metal";
RtToken	RI_PLASTIC	= "plastic";
RtToken	RI_PAINTEDPLASTIC	= "paintedplastic";
RtToken	RI_KA	= "ka";
RtToken	RI_KD	= "kd";
RtToken	RI_KS	= "ks";
RtToken	RI_ROUGHNESS	= "roughness";
RtToken	RI_SPECULARCOLOR	= "specularcolor";
RtToken	RI_DEPTHCUE	= "depthcue";
RtToken	RI_FOG	= "fog";
RtToken	RI_BUMPY	= "bumpy";
RtToken	RI_MINDISTANCE	= "mindistance";
RtToken	RI_MAXDISTANCE	= "maxdistance";
RtToken	RI_BACKGROUND	= "background";
RtToken	RI_DISTANCE	= "distance";

RtToken	RI_RASTER	= "raster";
RtToken	RI_SCREEN	= "screen";
RtToken	RI_CAMERA	= "camera";
RtToken	RI_WORLD	= "world";
RtToken	RI_OBJECT	= "object";
RtToken	RI_INSIDE	= "inside";
RtToken	RI_OUTSIDE	= "outside";
RtToken	RI_LH	= "lh";
RtToken	RI_RH	= "rh";
RtToken	RI_P	= "P";
RtToken	RI_PZ	= "Pz";
RtToken	RI_PW	= "Pw";
RtToken	RI_N	= "N";
RtToken	RI_NP	= "Np";
RtToken	RI_CS	= "Cs";
RtToken	RI_OS	= "Os";
RtToken	RI_S	= "s";
RtToken	RI_T	= "t";
RtToken	RI_ST	= "st";
RtToken	RI_BILINEAR	= "bilinear";
RtToken	RI_BICUBIC	= "bicubic";
RtToken	RI_CUBIC	= "cubic";
RtToken	RI_LINEAR	= "linear";
RtToken	RI_PRIMITIVE	= "primitive";
RtToken	RI_INTERSECTION	= "intersection";
RtToken	RI_UNION	= "union";
RtToken	RI_DIFFERENCE	= "difference";
RtToken	RI_WRAP	= "wrap";
RtToken	RI_NOWRAP	= "nowrap";
RtToken	RI_PERIODIC	= "periodic";
RtToken	RI_NONPERIODIC	= "nonperiodic";
RtToken	RI_CLAMP	= "clamp";
RtToken	RI_BLACK	= "black";
RtToken	RI_IGNORE	= "ignore";
RtToken	RI_PRINT	= "print";
RtToken	RI_ABORT	= "abort";
RtToken	RI_HANDLER	= "handler";
RtToken	RI_IDENTIFIER	= "identifier";
RtToken	RI_NAME	= "name";
RtToken	RI_CURRENT	= "current";
RtToken	RI_SHADER	= "shader";
RtToken	RI_EYE	= "eye";
RtToken	RI_NDC	= "ndc";
RtToken	RI_AMPLITUDE	=	"amplitude";
RtToken	RI_COMMENT	=	"comment";
RtToken	RI_CONSTANTWIDTH	=	"constantwidth";
RtToken	RI_KR	=	"kr";
RtToken	RI_SHINYMETAL	=	"shinymetal";
RtToken	RI_STRUCTURE	=	"structure";
RtToken	RI_TEXTURENAME	=	"texturename";
RtToken	RI_VERBATIM	=	"verbatim";
RtToken	RI_WIDTH	=	"width";

RtBasis	RiBezierBasis	= {{ -1.0f,       3.0f,      -3.0f,       1.0f},
                         {  3.0f,      -6.0f,       3.0f,       0.0f},
                         { -3.0f,       3.0f,       0.0f,       0.0f},
                         {  1.0f,       0.0f,       0.0f,       0.0f}};
RtBasis	RiBSplineBasis	= {{ -1.0f/6.0f,  0.5f,      -0.5f,       1.0f/6.0f},
                          {  0.5f,      -1.0f,       0.5f,       0.0f},
                          { -0.5f,       0.0f,	      0.5f,       0.0f},
                          {  1.0f/6.0f,  2.0f/3.0f,  1.0f/6.0f,  0.0f}};
RtBasis	RiCatmullRomBasis={{ -0.5f,       1.5f,      -1.5f,       0.5f},
                           {  1.0f,      -2.5f,       2.0f,      -0.5f},
                           { -0.5f,       0.0f,       0.5f,       0.0f},
                           {  0.0f,       1.0f,       0.0f,       0.0f}};
RtBasis	RiHermiteBasis	= {{  2.0f,       1.0f,      -2.0f,       1.0f},
                          { -3.0f,      -2.0f,       3.0f,      -1.0f},
                          {  0.0f,       1.0f,       0.0f,       0.0f},
                          {  1.0f,       0.0f,       0.0f,       0.0f}};
RtBasis	RiPowerBasis	= {{  1.0f,       0.0f,       0.0f,       0.0f},
                        {  0.0f,       1.0f,       0.0f,       0.0f},
                        {  0.0f,       0.0f,       1.0f,       0.0f},
                        {  0.0f,       0.0f,       0.0f,       1.0f}};

enum RIL_POINTS
{
    RIL_NONE = -1,
    RIL_P,
    RIL_Pz,
    RIL_Pw,
    RIL_N,
    RIL_Np,
    RIL_s,
    RIL_t = RIL_s,
    RIL_st,
};
static TqUlong RIH_S = CqString::hash( RI_S );
static TqUlong RIH_T = CqString::hash( RI_T );
static TqUlong RIH_ST = CqString::hash( RI_ST );
static TqUlong RIH_CS = CqString::hash( RI_CS );
static TqUlong RIH_OS = CqString::hash( RI_OS );
static TqUlong RIH_P = CqString::hash( RI_P );
static TqUlong RIH_PZ = CqString::hash( RI_PZ );
static TqUlong RIH_PW = CqString::hash( RI_PW );
static TqUlong RIH_N = CqString::hash( RI_N );
static TqUlong RIH_NP = CqString::hash( RI_NP );
static TqUlong RIH_DEPTHFILTER = CqString::hash( "depthfilter" );
static TqUlong RIH_JITTER = CqString::hash( "jitter" );
static TqUlong RIH_RENDER = CqString::hash( "render" );
static TqUlong RIH_INDIRECT = CqString::hash( "indirect" );
static TqUlong RIH_LIGHT = CqString::hash( "light" );
static TqUlong RIH_VISIBILITY = CqString::hash( "visibility" );

RtInt	RiLastError = 0;

// Mitchell Filter Declarations
class MitchellFilter {
public:
   // MitchellFilter Public Methods
   MitchellFilter(TqFloat b, TqFloat c, TqFloat xw, TqFloat yw)
   {
      B = b;
      C = c;
      invXWidth = 1.0f/xw;
      invYWidth = 1.0f/yw;
   }
   TqFloat Evaluate(TqFloat x, TqFloat y) const {
      return Evaluate(x * invXWidth) * Evaluate(y * invYWidth);
   }
   TqFloat Evaluate(TqFloat x) const {
      x = fabsf(2.f * x);
      if (x > 1.f)
         return ((-B - 6*C) * x*x*x + (6*B + 30*C) * x*x +
         (-12*B - 48*C) * x + (8*B + 24*C)) * (1.f/6.f);
      else
         return ((12 - 9*B - 6*C) * x*x*x +
         (-18 + 12*B + 6*C) * x*x +
         (6 - 2*B)) * (1.f/6.f);
   }
private:
   TqFloat B, C;
   TqFloat invXWidth, invYWidth;
};

//----------------------------------------------------------------------
// CreateGPrim
// Helper function to build a GPrim from any boost::shared_ptr<> type..
template<class T>
inline
RtVoid	CreateGPrim( const boost::shared_ptr<T>& pSurface )
{
	CreateGPrim( boost::static_pointer_cast<CqSurface,T>( pSurface ) );
}

//----------------------------------------------------------------------
// BuildParameterList
// Helper function to build a parameter list to pass on to the V style functions.
// returns a parameter count.

RtInt BuildParameterList( va_list pArgs, std::vector<RtToken>& aTokens, std::vector<RtPointer>& aValues )
{
	RtInt count = 0;
	RtToken pToken = va_arg( pArgs, RtToken );
	RtPointer pValue;
	aTokens.clear();
	aValues.clear();
	while ( pToken != 0 && pToken != RI_NULL )          	// While not RI_NULL
	{
		aTokens.push_back( pToken );
		pValue = va_arg( pArgs, RtPointer );
		aValues.push_back( pValue );
		pToken = va_arg( pArgs, RtToken );
		count++;
	}
	return ( count );
}


//----------------------------------------------------------------------
//	CqRangeCheckCallback implentation
//	Use this with CheckMinMax
//
class	CqLogRangeCheckCallback	: public CqRangeCheckCallback
{
	public:
		CqLogRangeCheckCallback()
		{ }

		virtual ~CqLogRangeCheckCallback()
		{
		}

		void set
			( const char* name )
		{
			m_name = name;
		}

		virtual void operator()( int res )
		{
			switch( res )
			{
					case CqRangeCheckCallback::UPPER_BOUND_HIT:
					{
						Aqsis::log() << error << "Invalid Value for " << m_name << ". Value exceeded upper limit" << std::endl;
					}

					case CqRangeCheckCallback::LOWER_BOUND_HIT:
					{
						Aqsis::log() << error << "Invalid Value for " << m_name << ". Value exceeded lower limit" << std::endl;
					}

					default:
					;
			}
		}


	private:
		const char*	m_name;
};


//----------------------------------------------------------------------
// ValidateState
// Check that the currect graphics state is one of those specified.
//
TqBool	ValidateState(int count, ... )
{
	va_list	pArgs;
	va_start( pArgs, count );

	int currentState = Outside;
	if(  QGetRenderContext() != NULL && QGetRenderContext()->pconCurrent() )
		currentState = QGetRenderContext()->pconCurrent()->Type();

	int i;
	for(i=0; i<count; i++)
	{
		int state = va_arg( pArgs, int );
		if( currentState == state )
			return(TqTrue);
	}
	return(TqFalse);
}


//----------------------------------------------------------------------
// GetStateAsString
// Get a string representing the current state.
//
const char*	GetStateAsString()
{
	int currentState = Outside;
	if( QGetRenderContext()->pconCurrent() )
		currentState = QGetRenderContext()->pconCurrent()->Type();
	switch( currentState )
	{
			case Outside:
			return("Outside");
			break;

			case BeginEnd:
			return("BeginEnd");
			break;

			case Frame:
			return("Frame");
			break;

			case World:
			return("World");
			break;

			case Attribute:
			return("Attribute");
			break;

			case Transform:
			return("Transform");
			break;

			case Solid:
			return("Solid");
			break;

			case Object:
			return("Object");
			break;

			case Motion:
			return("Motion");
			break;
	}
	return("");
}


//----------------------------------------------------------------------
// RiDeclare
// Declare a new variable to be recognised by the system.
//
RtToken	RiDeclare( RtString name, RtString declaration )
{
	Validate_Conditional0

	Cache_RiDeclare

	Validate_RiDeclare

	Debug_RiDeclare

	CqString strName( name ), strDecl( declaration );
	QGetRenderContext() ->AddParameterDecl( strName.c_str(), strDecl.c_str() );
	return ( 0 );
}


//----------------------------------------------------------------------
// SetDefaultRiOptions
// Set some Default Options.
//
void SetDefaultRiOptions( void )
{
	std::string systemRCPath;
	std::string homeRCPath;
	std::string currentRCPath;
	std::string rootPath;
	std::string separator;

#ifdef AQSIS_SYSTEM_WIN32

	char acPath[256];
	char root[256];
	if( GetModuleFileName( NULL, acPath, 256 ) != 0)
	{
		// guaranteed file name of at least one character after path
		*( strrchr( acPath, '\\' ) ) = '\0';
		std::string      stracPath(acPath);
		_fullpath(root,&stracPath[0],256);
	}
	rootPath = root;
	separator = "\\";
#elif AQSIS_SYSTEM_MACOSX

	CFURLRef pluginRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
	CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
	const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
	rootPath = pathPtr;
	separator = "/";
#else
	// Minty: Need to work out the executable path here.
	rootPath = DEFAULT_RC_PATH;
	separator = "/";
#endif

	systemRCPath = rootPath;
	systemRCPath.append( separator );
	systemRCPath.append( "aqsisrc" );

	// Read in the system configuration file if found.
	FILE* rcfile = fopen( systemRCPath.c_str(), "rb" );
	if (rcfile != NULL )
	{
		Aqsis::log() << info << "Reading system config \"" << systemRCPath.c_str() << "\"" << std::endl;
		CqRIBParserState currstate = librib::GetParserState();
		if (currstate.m_pParseCallbackInterface == NULL)
			currstate.m_pParseCallbackInterface = new librib2ri::Engine;
		librib::Parse( rcfile, "System Config", *(currstate.m_pParseCallbackInterface), *(currstate.m_pParseErrorStream), NULL );
		librib::SetParserState( currstate );
		fclose(rcfile);
	}
	else
	{
		Aqsis::log() << error << "Could not open system config (" << systemRCPath.c_str() << ")" << std::endl;
	}

	/* ...then read the .aqsisrc files in $HOME... */
	if(getenv("HOME"))
	{
		homeRCPath = getenv("HOME");
		if (homeRCPath[ homeRCPath.length() ] != separator[0])
		{
			homeRCPath.append(separator);
		};
		homeRCPath.append(".aqsisrc");
		rcfile = fopen( homeRCPath.c_str(), "rb" );
		if (rcfile != NULL )
		{
			Aqsis::log() << info << "Reading user config \"" << homeRCPath.c_str() << "\"" << std::endl;
			CqRIBParserState currstate = librib::GetParserState();
			if (currstate.m_pParseCallbackInterface == NULL)
				currstate.m_pParseCallbackInterface = new librib2ri::Engine;
			librib::Parse( rcfile, "Home Config", *(currstate.m_pParseCallbackInterface), *(currstate.m_pParseErrorStream), NULL );
			librib::SetParserState( currstate );
			fclose(rcfile);
		}
		else
		{
			Aqsis::log() << info << "Could not open user config (" << homeRCPath.c_str() << ")" << std::endl;
	        }
	}
	else
	{
		Aqsis::log() << info << "Environment variable HOME not set (skipping user config)." << std::endl;
	}

	/* ...and the current directory... */
	currentRCPath = ".aqsisrc";
	rcfile = fopen( currentRCPath.c_str(), "rb" );
	if (rcfile != NULL )
	{
		Aqsis::log() << info << "Reading project config \"" << currentRCPath.c_str() << "\"" << std::endl;
		CqRIBParserState currstate = librib::GetParserState();
		if (currstate.m_pParseCallbackInterface == NULL)
			currstate.m_pParseCallbackInterface = new librib2ri::Engine;
		librib::Parse( rcfile, "Current Config", *(currstate.m_pParseCallbackInterface), *(currstate.m_pParseErrorStream), NULL );
		librib::SetParserState( currstate );
		fclose(rcfile);
	}
	else
	{
		Aqsis::log() << info << "Could not open project config (" << currentRCPath.c_str() << ")" << std::endl;
        }

	const char* popt[ 1 ];
	if(getenv("AQSIS_SHADER_PATH"))
	{
		popt[0] = getenv("AQSIS_SHADER_PATH");
		Aqsis::log() << info << "Applying AQSIS_SHADER_PATH (" << popt[0] << ")" << std::endl;
		RiOption( "searchpath", "shader", &popt, RI_NULL );
	}
	else
	{
		Aqsis::log() << info << "AQSIS_SHADER_PATH not set" << std::endl;
	}

	if(getenv("AQSIS_ARCHIVE_PATH"))
	{
		popt[0] = getenv("AQSIS_ARCHIVE_PATH");
		Aqsis::log() << info << "Applying AQSIS_ARCHIVE_PATH (" << popt[0] << ")" << std::endl;
		RiOption( "searchpath", "archive", &popt, RI_NULL );
	}
	else
	{
		Aqsis::log() << info << "AQSIS_ARCHIVE_PATH not set" << std::endl;
	}

	if(getenv("AQSIS_TEXTURE_PATH"))
	{
		popt[0] = getenv("AQSIS_TEXTURE_PATH");
		Aqsis::log() << info << "Applying AQSIS_TEXTURE_PATH (" << popt[0] << ")" << std::endl;
		RiOption( "searchpath", "texture", &popt, RI_NULL );
	}
	else
	{
		Aqsis::log() << info << "AQSIS_TEXTURE_PATH not set" << std::endl;
	}

	if(getenv("AQSIS_DISPLAY_PATH"))
	{
		popt[0] = getenv("AQSIS_DISPLAY_PATH");
		Aqsis::log() << info << "Applying AQSIS_DISPLAY_PATH (" << popt[0] << ")" << std::endl;
		RiOption( "searchpath", "display", &popt, RI_NULL );
	}
	else
	{
		Aqsis::log() << info << "AQSIS_DISPLAY_PATH not set" << std::endl;
	}

	if(getenv("AQSIS_PROCEDURAL_PATH"))
	{
		popt[0] = getenv("AQSIS_PROCEDURAL_PATH");
		Aqsis::log() << info << "Applying AQSIS_PROCEDURAL_PATH (" << popt[0] << ")" << std::endl;
		RiOption( "searchpath", "procedural", &popt, RI_NULL );
	}
	else
	{
		Aqsis::log() << info << "AQSIS_PROCEDURAL_PATH not set" << std::endl;
	}

	if(getenv("AQSIS_PLUGIN_PATH"))
	{
		popt[0] = getenv("AQSIS_PLUGIN_PATH");
		Aqsis::log() << info << "Applying AQSIS_PLUGIN_PATH (" << popt[0] << ")" << std::endl;
		RiOption( "searchpath", "plugin", &popt, RI_NULL );
	}
	else
	{
		Aqsis::log() << info << "AQSIS_PLUGIN_PATH not set" << std::endl;
	}

	// Setup a default Display
	Aqsis::log() << info << "Setting up default display: Display \"ri.pic\" \"file\" \"rgba\"" << std::endl;
	RiDisplay( "ri.pic", "file", "rgba", NULL );
}

//----------------------------------------------------------------------
// RiBegin
// Begin a Renderman render phase.
//
extern "C" char *StandardParameters[][2];
RtVoid	RiBegin( RtToken name )
{
	Validate_RiBegin

	Debug_RiBegin

	// Create a new renderer
	QSetRenderContext( new CqRenderer );

	QGetRenderContext() ->Initialise();
	QGetRenderContext() ->BeginMainModeBlock();
	QGetRenderContext() ->ptransSetTime( CqMatrix() );
	QGetRenderContext() ->SetCameraTransform( QGetRenderContext() ->ptransCurrent() );
	// Clear the lightsources stack.
	Lightsource_stack.clear();

	// Clear any options.
	QGetRenderContext() ->optCurrent().ClearOptions();

	// Include the standard options (how can we opt out of this).
	int param = 0;
	while( StandardParameters[param][0] != NULL )
	{
		RiDeclare(
		    StandardParameters[param][0],
		    StandardParameters[param][1]
		);
		param++;
	};

	SetDefaultRiOptions();

	// Setup a default surface shader
	boost::shared_ptr<IqShader> pDefaultSurfaceShader =
	    QGetRenderContext()->getDefaultSurfaceShader();
	QGetRenderContext() ->pattrWriteCurrent() ->SetpshadSurface( pDefaultSurfaceShader, QGetRenderContext() ->Time() );

	// Setup the initial transformation.
	//	QGetRenderContext()->ptransWriteCurrent() ->SetHandedness( TqFalse );
	QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "Orientation" ) [ 0 ] = 0;

	return ;
}

//----------------------------------------------------------------------
// RiEnd
// End the rendermam render stage.
//
RtVoid	RiEnd()
{
	Validate_RiEnd

	Debug_RiEnd

	QGetRenderContext() ->EndMainModeBlock();

	// Flush the image cache.
	CqTextureMap::FlushCache();

	// Clear the lightsources stack.
	Lightsource_stack.clear();

	// Delete the renderer
	delete( QGetRenderContext() );
	QSetRenderContext( 0 );

	return ;
}


//----------------------------------------------------------------------
// RiFrameBegin
// Begin an individual frame, options are saved at this point.
//
RtVoid	RiFrameBegin( RtInt number )
{
	Validate_Conditional

	Cache_RiFrameBegin

	Validate_RiFrameBegin

	Debug_RiFrameBegin

	// Initialise the statistics variables. If the RIB doesn't contain
	// a Frame-block the initialisation was previously done in CqStats::Initilise()
	// which has to be called before a rendering session.
	QGetRenderContext() ->Stats().InitialiseFrame();
	// Start the timer. Note: The corresponding call of StopFrameTimer() is
	// done in WorldEnd (!) not FrameEnd since it can happen that there is
	// not FrameEnd (and usually there's not much between WorldEnd and FrameEnd).
	//QGetRenderContext() ->Stats().StartFrameTimer();
	TIMER_START("Frame")

	QGetRenderContext() ->BeginFrameModeBlock();
	QGetRenderContext() ->SetCurrentFrame( number );
	CqCSGTreeNode::SetRequired( TqFalse );

	QGetRenderContext() ->Stats().InitialiseFrame();

	QGetRenderContext()->clippingVolume().clear();


	worldrand.Reseed('a'+'q'+'s'+'i'+'s');

	return ;
}


//----------------------------------------------------------------------
// RiFrameEnd
// End the rendering of an individual frame, options are restored.
//
RtVoid	RiFrameEnd()
{
	Validate_Conditional

	Cache_RiFrameEnd

	Validate_RiFrameEnd

	Debug_RiFrameEnd

	QGetRenderContext() ->EndFrameModeBlock();
	QGetRenderContext() ->ClearDisplayRequests();

	return ;
}

//----------------------------------------------------------------------
// RiWorldBegin
// Start the information for the world, options are now frozen.  The world-to-camera
// transformation is set to the current transformation, and current is set to identity.
//
RtVoid	RiWorldBegin()
{
	Validate_Conditional

	Cache_RiWorldBegin

	Validate_RiWorldBegin

	Debug_RiWorldBegin

	// Call any specified pre world function.
	if ( QGetRenderContext()->pPreWorldFunction() != NULL )
		( *QGetRenderContext()->pPreWorldFunction() ) ();

	// Start the frame timer (just in case there was no FrameBegin block. If there
	// was, nothing happens)
	//QGetRenderContext() ->Stats().StartFrameTimer();
	TIMER_START("Frame")
	TIMER_START("Parse")

	// Now that the options have all been set, setup any undefined camera parameters.
	if ( !QGetRenderContext() ->optCurrent().FrameAspectRatioCalled() )
	{
		// Derive the FAR from the resolution and pixel aspect ratio.
		RtFloat PAR = QGetRenderContext() ->optCurrent().GetFloatOption( "System", "PixelAspectRatio" ) [ 0 ];
		RtFloat resH = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "Resolution" ) [ 0 ];
		RtFloat resV = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "Resolution" ) [ 1 ];
		QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "FrameAspectRatio" ) [ 0 ] = ( resH * PAR ) / resV ;
	}

	if ( !QGetRenderContext() ->optCurrent().ScreenWindowCalled() )
	{
		RtFloat fFAR = QGetRenderContext() ->optCurrent().GetFloatOption( "System", "FrameAspectRatio" ) [ 0 ];

		if ( fFAR >= 1.0 )
		{
			QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "ScreenWindow" ) [ 0 ] = -fFAR ;
			QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "ScreenWindow" ) [ 1 ] = + fFAR ;
			QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "ScreenWindow" ) [ 2 ] = + 1 ;
			QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "ScreenWindow" ) [ 3 ] = -1 ;
		}
		else
		{
			QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "ScreenWindow" ) [ 0 ] = -1 ;
			QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "ScreenWindow" ) [ 1 ] = + 1 ;
			QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "ScreenWindow" ) [ 2 ] = + 1.0 / fFAR ;
			QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "ScreenWindow" ) [ 3 ] = -1.0 / fFAR ;
		}
	}

	// Set the world to camera transformation matrix to the current matrix.

	CqTransformPtr current( QGetRenderContext() ->ptransCurrent() );
	CqTransformPtr camera(new CqTransform());
	QGetRenderContext() ->SetCameraTransform( camera );
	// clear the camera transform to a single state, all camera motion is now transferred to the objects.
	QGetRenderContext()->GetCameraTransform()->ResetTransform( current->matObjectToWorld( current->Time(0) ), current->GetHandedness(QGetRenderContext()->Time()) );
	QGetRenderContext() ->BeginWorldModeBlock();
	// and then reset the current matrix to identity, ready for object transformations.
	if ( current->cTimes() > 1 )
	{
		TqInt i;
		CqMatrix matOpenShutterInverse = current->matObjectToWorld( current->Time( 0 ) );
		matOpenShutterInverse = matOpenShutterInverse.Inverse();
		current->SetCurrentTransform( current->Time( 0 ), CqMatrix() );
		for ( i = 1; i < current->cTimes(); ++i )
		{
			current->SetCurrentTransform( current->Time( i ), matOpenShutterInverse * current->matObjectToWorld( current->Time( i ) ) );
		}
		QGetRenderContext()->pconCurrent()->ptransSetCurrent(current);

		// Store the initial object transformation (including imparted camera motion).
		CqTransformPtr newTrans( new CqTransform( current ) );
		QGetRenderContext()->SetDefObjTransform( newTrans );
	}
	else
	{
		QGetRenderContext() ->ptransSetTime( CqMatrix() );

		// Store the initial object transformation
		CqTransformPtr newTrans( new CqTransform() );
		QGetRenderContext()->SetDefObjTransform( newTrans );
	}

	// If rendering a depth buffer, check that the filter is "box" 1x1, warn if not.
	TqInt iMode = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "DisplayMode" ) [ 0 ];
	if( iMode & ModeZ )
	{
		RtFilterFunc filter = QGetRenderContext() ->optCurrent().funcFilter();
		TqFloat xwidth = QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "FilterWidth" ) [ 0 ];
		TqFloat ywidth = QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "FilterWidth" ) [ 1 ];
		if( filter != RiBoxFilter || xwidth != 1 || ywidth != 1)
			Aqsis::log() << warning << "When rendering a Z buffer the filter mode should be \"box\" with a width of 1x1" << std::endl;
	}



	QGetRenderContext()->SetWorldBegin();

	QGetRenderContext() ->optCurrent().InitialiseCamera();
	QGetRenderContext() ->pImage() ->SetImage();

	worldrand.Reseed('a'+'q'+'s'+'i'+'s');

	return ;
}


//----------------------------------------------------------------------
// RiWorldEnd
// End the specifying of world data, options are released.
//

RtVoid	RiWorldEnd()
{
	Validate_Conditional

	Cache_RiWorldEnd

	Validate_RiWorldEnd

	Debug_RiWorldEnd

	QGetRenderContext()->RenderAutoShadows();

	TqBool fFailed = TqFalse;
	// Call any specified pre render function.
	if ( QGetRenderContext()->pPreRenderFunction() != NULL )
		( *QGetRenderContext()->pPreRenderFunction() ) ();

	// Stop the parsing counter
	TIMER_STOP("Parse")


	const TqInt* poptVerbose = QGetRenderContext() ->optCurrent().GetIntegerOption( "statistics", "renderinfo" );
	if ( poptVerbose )
	{
		QGetRenderContext() -> Stats().PrintInfo();
	}

	const TqInt* poptGridSize = QGetRenderContext() ->optCurrent().GetIntegerOption( "limits", "gridsize" );
	if( NULL != poptGridSize )
		QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "SqrtGridSize" )[0] = sqrt( static_cast<float>(poptGridSize[0]) );

	// Finalise the raytracer database now that all primitives are in.
	if(QGetRenderContext()->pRaytracer())
		QGetRenderContext()->pRaytracer()->Finalise();

	// Render the world
	try
	{
		QGetRenderContext() ->RenderWorld();
	}
	catch ( CqString strError )
	{
		Aqsis::log() << error << strError.c_str() << std::endl;
		fFailed = TqTrue;
	}

	// Delete the world context
	QGetRenderContext() ->EndWorldModeBlock();

	// Stop the frame timer
	//QGetRenderContext() ->Stats().StopFrameTimer();
	TIMER_STOP("Frame")

	if ( !fFailed )
	{
		// Get the verbosity level from the options...
		TqInt verbosity = 0;
		const TqInt* poptEndofframe = QGetRenderContext() ->optCurrent().GetIntegerOption( "statistics", "endofframe" );
		if ( poptEndofframe != 0 )
			verbosity = poptEndofframe[ 0 ];

		// ...and print the statistics.
		QGetRenderContext() ->Stats().PrintStats( verbosity );
	}

	QGetRenderContext()->SetWorldBegin(TqFalse);

	return ;
}


//----------------------------------------------------------------------
// RiFormat
// Specify the setup of the final image.
//
RtVoid	RiFormat( RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio )
{
	Validate_Conditional

	Cache_RiFormat

	Validate_RiFormat

	Debug_RiFormat

	QGetRenderContext() ->optCurrent().GetIntegerOptionWrite( "System", "Resolution" ) [ 0 ] = xresolution ;
	QGetRenderContext() ->optCurrent().GetIntegerOptionWrite( "System", "Resolution" ) [ 1 ] = yresolution ;
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "PixelAspectRatio" ) [ 0 ] = ( pixelaspectratio < 0.0 ) ? 1.0 : pixelaspectratio ;

	// Inform the system that RiFormat has been called, as this takes priority.
	QGetRenderContext() ->optCurrent().CallFormat();

	return ;
}


//----------------------------------------------------------------------
// RiFrameAspectRatio
// Set the aspect ratio of the frame irrespective of the display setup.
//
RtVoid	RiFrameAspectRatio( RtFloat frameratio )
{
	Validate_Conditional

	Cache_RiFrameAspectRatio

	Validate_RiFrameAspectRatio

	Debug_RiFrameAspectRatio

	CqLogRangeCheckCallback rc;

	bool valid = true;

	rc.set( "frameratio");
	if( !CheckMinMax( frameratio, 0.0f, RI_INFINITY, &rc ) )
	{
		valid = false;
	}

	if( !valid )
	{
		Aqsis::log() << error << "RiFrameAspectRatio: Invalid RiFrameAspectRatio, aborting" << std::endl;
		return;
	}

	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "FrameAspectRatio" ) [ 0 ] = frameratio ;

	// Inform the system that RiFrameAspectRatio has been called, as this takes priority.
	QGetRenderContext() ->optCurrent().CallFrameAspectRatio();

	return ;
}


//----------------------------------------------------------------------
// RiScreenWindow
// Set the resolution of the screen window in the image plane specified in the screen
// coordinate system.
//
RtVoid	RiScreenWindow( RtFloat left, RtFloat right, RtFloat bottom, RtFloat top )
{
	Validate_Conditional

	Cache_RiScreenWindow

	Validate_RiScreenWindow

	Debug_RiScreenWindow

	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "ScreenWindow" ) [ 0 ] = left ;
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "ScreenWindow" ) [ 1 ] = right ;
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "ScreenWindow" ) [ 2 ] = top ;
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "ScreenWindow" ) [ 3 ] = bottom ;

	// Inform the system that RiScreenWindow has been called, as this takes priority.
	QGetRenderContext() ->optCurrent().CallScreenWindow();

	return ;
}


//----------------------------------------------------------------------
// RiCropWindow
// Set the position and size of the crop window specified in fractions of the raster
// window.
//
RtVoid	RiCropWindow( RtFloat left, RtFloat right, RtFloat top, RtFloat bottom )
{
	Validate_Conditional

	Cache_RiCropWindow

	Validate_RiCropWindow

	Debug_RiCropWindow

	CqLogRangeCheckCallback rc;

	bool valid = true;

	rc.set( "left");
	if( !CheckMinMax( left, 0.0f, 1.0f, &rc ) )
	{
		valid = false;
	}

	rc.set( "right" );
	if( !CheckMinMax( right, 0.0f, 1.0f, &rc ) )
	{
		valid = false;
	}

	rc.set( "top" );
	if( !CheckMinMax( top, 0.0f, 1.0f, &rc ) )
	{
		valid = false;
	}

	rc.set( "bottom" );
	if( !CheckMinMax( bottom, 0.0f, 1.0f, &rc ) )
	{
		valid = false;
	}

	if( !valid )
	{
		Aqsis::log() << error << "Invalid RiCropWindow, aborting" << std::endl;
		return;
	}

	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "CropWindow" ) [ 0 ] = left ;
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "CropWindow" ) [ 1 ] = right ;
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "CropWindow" ) [ 2 ] = top ;
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "CropWindow" ) [ 3 ] = bottom ;

	return ;
}


//----------------------------------------------------------------------
// RiProjection
// Set the camera projection to be used.
//
RtVoid	RiProjection( RtToken name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiProjectionV( name, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiProjectionV
// List mode version of above.
//
RtVoid	RiProjectionV( RtToken name, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiProjection

	Validate_RiProjection

	Debug_RiProjection

	if ( strcmp( name, RI_PERSPECTIVE ) == 0 )
		QGetRenderContext() ->optCurrent().GetIntegerOptionWrite( "System", "Projection" ) [ 0 ] = ProjectionPerspective ;
	else if	( strcmp( name, RI_ORTHOGRAPHIC ) == 0 )
		QGetRenderContext() ->optCurrent().GetIntegerOptionWrite( "System", "Projection" ) [ 0 ] = ProjectionOrthographic ;
	else if( name != RI_NULL )
	{
		Aqsis::log() << error << "RiProjection: Invalid projection: \"" << name << "\"" << std::endl;
		return ;
	}

	RtInt i;
	for ( i = 0; i < count; ++i )
	{
		RtToken	token = tokens[ i ];
		RtPointer	value = values[ i ];

		if ( strcmp( token, RI_FOV ) == 0 )
			QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "FOV" ) [ 0 ] = *( reinterpret_cast<RtFloat*>( value ) ) ;
	}
	// TODO: need to get the current transformation so that it can be added to the screen transformation.
	QGetRenderContext() ->ptransSetTime( CqMatrix() );

	return ;
}


//----------------------------------------------------------------------
// RiClipping
// Set the near and far clipping planes specified as distances from the camera.
//
RtVoid	RiClipping( RtFloat cnear, RtFloat cfar )
{
	Validate_Conditional

	Cache_RiClipping

	Validate_RiClipping

	Debug_RiClipping

	CqLogRangeCheckCallback rc;

	bool valid = true;

	rc.set( "near");
	if( !CheckMinMax( cnear, RI_EPSILON, cfar, &rc ) )
	{
		valid = false;
	}

	rc.set( "far" );
	if( !CheckMinMax( cfar, cnear, RI_INFINITY, &rc ) )
	{
		valid = false;
	}

	if( !valid )
	{
		Aqsis::log() << error << "RiClipping: Invalid RiClipping, clipping planes set to RI_EPSILON, RI_INFINITY" << std::endl;
		cnear	= RI_EPSILON;
		cfar	= RI_INFINITY;
	}

	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "Clipping" ) [ 0 ] = cnear ;
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "Clipping" ) [ 1 ] = cfar ;

	return ;
}


//----------------------------------------------------------------------
// RiDepthOfField
// Specify the parameters which affect focal blur of the camera.
//
RtVoid	RiDepthOfField( RtFloat fstop, RtFloat focallength, RtFloat focaldistance )
{
	Validate_Conditional

	Cache_RiDepthOfField

	Validate_RiDepthOfField

	Debug_RiDepthOfField

	CqLogRangeCheckCallback rc;

	bool valid = true;

	rc.set( "fstop" );
	if( !CheckMinMax( fstop, 0.0f, RI_INFINITY, &rc ) )
	{
		valid = false;
	}

	rc.set( "focallength" );
	if( !CheckMinMax( focallength, 0.0f, RI_INFINITY, &rc ) )
	{
		valid = false;
	}

	rc.set( "focaldistance" );
	if( !CheckMinMax( focaldistance, 0.0f, RI_INFINITY, &rc ) )
	{
		valid = false;
	}

	if( !valid )
	{
		Aqsis::log() << warning << "RiDepthOfField: Invalid DepthOfField, DepthOfField ignored" << std::endl;
		return;
	}

	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "DepthOfField" ) [ 0 ] = fstop ;
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "DepthOfField" ) [ 1 ] = focallength ;
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "DepthOfField" ) [ 2 ] = focaldistance ;

	QGetRenderContext() ->SetDepthOfFieldData( fstop, focallength, focaldistance );
}


//----------------------------------------------------------------------
// RiShutter
//	Set the times at which the shutter opens and closes, used for motion blur.
//
RtVoid	RiShutter( RtFloat opentime, RtFloat closetime )
{
	Validate_Conditional

	Cache_RiShutter

	Validate_RiShutter

	Debug_RiShutter

	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "Shutter" ) [ 0 ] = opentime;
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "Shutter" ) [ 1 ] = closetime;

	return ;
}


//----------------------------------------------------------------------
// RiPixelVariance
// Set the upper bound on the variance from the true pixel color by the pixel filter
// function.
//
RtVoid	RiPixelVariance( RtFloat variance )
{
	Validate_Conditional

	Cache_RiPixelVariance

	Validate_RiPixelVariance

	Debug_RiPixelVariance

	CqLogRangeCheckCallback rc;

	bool valid = true;

	rc.set( "variance" );
	if( !CheckMinMax( variance, 0.0f, RI_INFINITY, &rc ) )
	{
		valid = false;
	}

	if( !valid )
	{
		Aqsis::log() << warning << "RiPixelVariance: Invalid PixelVariance, PixelVariance set to 0" << std::endl;
		variance = 0;
	}

	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "PixelVariance" ) [ 0 ] = variance ;

	return ;
}


//----------------------------------------------------------------------
// RiPixelSamples
// Set the number of samples per pixel for the hidden surface function.
//
RtVoid	RiPixelSamples( RtFloat xsamples, RtFloat ysamples )
{
	Validate_Conditional

	Cache_RiPixelSamples

	Validate_RiPixelSamples

	Debug_RiPixelSamples

	CqLogRangeCheckCallback rc;

	bool valid = true;

	rc.set( "xsamples" );
	if( !CheckMinMax( xsamples, 1.0f, RI_INFINITY, &rc ) )
	{
		valid = false;
	}

	rc.set( "ysamples" );
	if( !CheckMinMax( ysamples, 1.0f, RI_INFINITY, &rc ) )
	{
		valid = false;
	}

	if( !valid )
	{
		Aqsis::log() << warning << "RiPixelSamples: Invalid PixelSamples, PixelSamples set to 1, 1" << std::endl;
		xsamples = 1;
		ysamples = 1;
	}

	QGetRenderContext() ->optCurrent().GetIntegerOptionWrite( "System", "PixelSamples" ) [ 0 ] = static_cast<TqInt>( xsamples ) ;
	QGetRenderContext() ->optCurrent().GetIntegerOptionWrite( "System", "PixelSamples" ) [ 1 ] = static_cast<TqInt>( ysamples ) ;

	return ;
}


//----------------------------------------------------------------------
// RiPixelFilter
// Set the function used to generate a final pixel value from supersampled values.
//
RtVoid	RiPixelFilter( RtFilterFunc function, RtFloat xwidth, RtFloat ywidth )
{
	Validate_Conditional

	Cache_RiPixelFilter

	Validate_RiPixelFilter

	Debug_RiPixelFilter

	QGetRenderContext() ->optCurrent().SetfuncFilter( function );
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "FilterWidth" ) [ 0 ] = xwidth ;
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "FilterWidth" ) [ 1 ] = ywidth ;

	return ;
}


//----------------------------------------------------------------------
// RiExposure
//	Set the values of the exposure color modification function.
//
RtVoid	RiExposure( RtFloat gain, RtFloat gamma )
{
	Validate_Conditional

	Cache_RiExposure

	Validate_RiExposure

	Debug_RiExposure

	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "Exposure" ) [ 0 ] = gain ;
	QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "Exposure" ) [ 1 ] = gamma ;

	return ;
}


//----------------------------------------------------------------------
// RiImager
// Specify a prepocessing imager shader.
//
RtVoid	RiImager( RtToken name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiImagerV( name, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiImagerV
// List based version of above.
//
RtVoid	RiImagerV( RtToken name, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiImager

	Validate_RiImager

	Debug_RiImager

	// Find the shader.
	boost::shared_ptr<IqShader> pshadImager = QGetRenderContext()->CreateShader( name, Type_Imager );

	if ( pshadImager )
	{
		QGetRenderContext() ->optCurrent().GetStringOptionWrite( "System", "Imager" ) [ 0 ] = name ;
		QGetRenderContext()->optCurrent().SetpshadImager( pshadImager );
		RtInt i;
		for ( i = 0; i < count; ++i )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			SetShaderArgument( pshadImager, token, static_cast<TqPchar>( value ) );
		}
	}
	return ;
}


//----------------------------------------------------------------------
// RiQuantize
// Specify the color quantization parameters.
//
RtVoid	RiQuantize( RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude )
{
	Validate_Conditional

	Cache_RiQuantize

	Validate_RiQuantize

	Debug_RiQuantize

	if ( strcmp( type, "rgba" ) == 0 )
	{
		TqFloat* pColorQuantize = QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "Quantize", "Color" );
		pColorQuantize [ 0 ] = static_cast<TqFloat>( one );
		pColorQuantize [ 1 ] = static_cast<TqFloat>( min );
		pColorQuantize [ 2 ] = static_cast<TqFloat>( max );
		pColorQuantize [ 3 ] = static_cast<TqFloat>( ditheramplitude );
	}
	else if ( strcmp( type, "z" ) == 0 )
	{
		TqFloat* pDepthQuantize = QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "Quantize", "Depth" );
		pDepthQuantize [ 0 ] = static_cast<TqFloat>( one );
		pDepthQuantize [ 1 ] = static_cast<TqFloat>( min );
		pDepthQuantize [ 2 ] = static_cast<TqFloat>( max );
		pDepthQuantize [ 3 ] = static_cast<TqFloat>( ditheramplitude );
	}
	else
	{
		CqNamedParameterList* pOption = QGetRenderContext() ->optCurrent().pOptionWrite( "Quantize" ).get();
		if( pOption )
		{
			CqParameterTypedUniformArray<TqFloat,type_float,TqFloat>* pQuant = new CqParameterTypedUniformArray<TqFloat,type_float,TqFloat>(type,4);
			\
			pQuant->pValue()[0] = static_cast<TqFloat>( one );
			pQuant->pValue()[1] = static_cast<TqFloat>( min );
			pQuant->pValue()[2] = static_cast<TqFloat>( max );
			pQuant->pValue()[3] = static_cast<TqFloat>( ditheramplitude );
			pOption->AddParameter( pQuant );
		}
	}

	return ;
}


//----------------------------------------------------------------------
// RiDisplay
// Set the final output name and type.
//
RtVoid	RiDisplay( RtToken name, RtToken type, RtToken mode, ... )
{
	va_list	pArgs;
	va_start( pArgs, mode );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiDisplayV( name, type, mode, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiDisplayV
// List based version of above.
//
RtVoid	RiDisplayV( RtToken name, RtToken type, RtToken mode, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiDisplay

	Validate_RiDisplay

	Debug_RiDisplay

	CqString strName( name );
	CqString strType( type );

	QGetRenderContext() ->optCurrent().GetStringOptionWrite( "System", "DisplayName" ) [ 0 ] = strName.c_str() ;
	QGetRenderContext() ->optCurrent().GetStringOptionWrite( "System", "DisplayType" ) [ 0 ] = strType.c_str() ;

	// Append the display mode to the current setting.
	TqInt eValue = 0;
	TqInt index = 0;
	TqInt dataOffset = 0;
	TqInt dataSize = 0;
	if ( strncmp( mode, RI_RGB, strlen(RI_RGB) ) == 0 )
	{
		eValue |= ModeRGB;
		dataSize += 3;
		index += strlen( RI_RGB );
	}
	if ( strncmp( &mode[index], RI_A, strlen( RI_A ) ) == 0 )
	{
		eValue |= ModeA;
		dataSize += 1;
		index += strlen( RI_A );
	}
	if ( strncmp( &mode[index], RI_Z, strlen( RI_Z ) ) == 0 )
	{
		eValue |= ModeZ;
		dataSize += 1;
		index += strlen( RI_Z );
	}

	// Special case test.
	if(strncmp(&mode[index], "depth", strlen("depth") ) == 0 )
	{
		dataSize = 1;
		dataOffset = 6;
	}

	// If none of the standard "rgbaz" strings match, then it is an alternative 'arbitrary output variable'
	else if( eValue == 0 )
	{
		dataOffset = QGetRenderContext()->RegisterOutputData( mode );
		dataSize = QGetRenderContext()->OutputDataSamples( mode );
	}


	// Gather the additional arguments into a map to pass through to the manager.
	std::map<std::string, void*> mapOfArguments;
	TqInt i;
	for( i = 0; i < count; ++i )
		mapOfArguments[ tokens[ i ] ] = values[ i ];

	// Check if the request is to add a display driver.
	if ( strName[ 0 ] == '+' )
	{
		TqInt iMode = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "DisplayMode" ) [ 0 ] | eValue;
		QGetRenderContext() ->optCurrent().GetIntegerOptionWrite( "System", "DisplayMode" ) [ 0 ] = iMode;
		strName = strName.substr( 1 );
	}
	else
	{
		QGetRenderContext() ->ClearDisplayRequests();
		QGetRenderContext() ->optCurrent().GetIntegerOptionWrite( "System", "DisplayMode" ) [ 0 ] = eValue ;
	}
	// Add a display driver to the list of requested drivers.
	QGetRenderContext() ->AddDisplayRequest( strName.c_str(), strType.c_str(), mode, eValue, dataOffset, dataSize, mapOfArguments );

	return ;
}


//----------------------------------------------------------------------
// RiGaussianFilter
// Gaussian filter used as a possible value passed to RiPixelFilter.
//
RtFloat	RiGaussianFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	/*
	 *  d = distance from origin
	 *  w = filterwidth ([COOK84a] article used 1.5)
	 *      For here use sqrt( (xwidth/2)*(xwidth/2) + (ywidth/2)*(ywidth/2) ).
	 *      Simplifying:
	 *
	 *          w = sqrt( (xwidth*xwidth)/2 + (ywidth*ywidth)/2 )
	 *          w = sqrt( (xwidth*xwidth + ywidth*ywidth)/2 )
	 *        w*w = (xwidth*xwidth + ywidth*ywidth)/2
	 *
	 *  if (d < filterwidth) then 0
	 *  else  exp(-d*d) - exp(-w*w)
	 *
	 */
	//RtFloat d,d2,w,w2;
	//
	///* d = sqrt(x*x+y*y), d*d = (x*x+y*y)  */
	//d2 = (x*x+y*y);
	//d = sqrt(d2);
	//
	//w2 = 0.5*(xwidth*xwidth + ywidth*ywidth);
	//w = sqrt(w2);
	//
	//if(d>w)
	//	return(0.0);
	//else
	//	return(exp(-d2) - exp(-w2));

	// The above version falls faster than the one used by the 3.2 spec
	//   PRMan and RenderDotC.  Since all three match exactly, might as
	//   well change to the code below:
	x *= 2.0 / xwidth;
	y *= 2.0 / ywidth;

	return exp( -2.0 * ( x * x + y * y ) );
}

 
//----------------------------------------------------------------------
// RiMitchellFilter
// Mitchell filter used as a possible value passed to RiPixelFIlter.
//
RtFloat	RiMitchellFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
MitchellFilter mc(1/3.0f, 2/3.0f, xwidth, ywidth);

   return mc.Evaluate(x, y);
}

//----------------------------------------------------------------------
// RiBoxFilter
// Box filter used as a possible value passed to RiPixelFIlter.
//
RtFloat	RiBoxFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	/* [UPST89] -- (RC p. 178) says that x and y will be in the
	 *    following intervals:
	 *           -xwidth/2 <= x <= xwidth/2
	 *           -ywidth/2 <= y <= ywidth/2
	 *    These constraints on x and y really simplifies the
	 *       the following code to just return (1.0).  
	 *
	 */
	return MIN( ( fabs( x ) <= xwidth / 2.0 ? 1.0 : 0.0 ),
	            ( fabs( y ) <= ywidth / 2.0 ? 1.0 : 0.0 ) );
}


//----------------------------------------------------------------------
// RiTriangleFilter
// Triangle filter used as a possible value passed to RiPixelFilter
//
RtFloat	RiTriangleFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	RtFloat	hxw = xwidth / 2.0;
	RtFloat	hyw = ywidth / 2.0;
	RtFloat	absx = fabs( x );
	RtFloat	absy = fabs( y );

	/* This function can be simplified as well by not worrying about
	 *    returning zero if the sample is beyond the filter window.
	 */
	return MIN( ( absx <= hxw ? ( hxw - absx ) / hxw : 0.0 ),
	            ( absy <= hyw ? ( hyw - absy ) / hyw : 0.0 ) );
}


//----------------------------------------------------------------------
// RiCatmullRomFilter
// Catmull Rom filter used as a possible value passed to RiPixelFilter.
//
RtFloat	RiCatmullRomFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	/*
	 * From page 223 of [MITC88] 
	 *
	 * if abs(d) < 1
	 *    f(d) = 1/6*(  (12-9*B-9*C)*abs(d*d*d)
	 *                + (-18 + 12*B + 6*C)*d*d + (6-2*B) )
	 *
	 * if 1 <= abs(d) < 2
	 *    f(d) = 1/6*(  (-B-6*C)*abs(d*d*d)
	 *                + (6*B + 30*C)*d*d
	 *                + (-12*B - 48*C)*d
	 *                + (8*B + 24*C) )
	 *
	 * otherwise  f(d)=0
	 *
	 * -------------------------------------------------------------
	 *  When B = 0.0 and C = 0.5 the filter is a Catmull-Rom cubic spline.
	 *
	 * if abs(d) < 1
	 *    f(d) = 1/6*[  (12-3)*abs(d*d*d) + (-18 + 3)*d*d + (6) ]
	 *
	 * if 1 <= abs(d) < 2
	 *    f(d) = 1/6*[  (-3)*abs(d*d*d) + (15)*d*d + (-24)*d + (12) ]
	 *
	 * otherwise  f(d)=0
	 * -------------------------------------------------------------
	 * Simplifying:
	 *
	 * if abs(d) < 1
	 *    f(d) = (3/2)*abs(d*d*d) - (5/2)*d*d + 1
	 *
	 * if 1 <= abs(d) <2
	 *    f(d) = (-0.5)*abs(d*d*d) + (5/2)*d*d - 4*abs(d) + 2
	 *
	 * otherwise  f(d)=0
	 *
	 */
	/*    RtFloat d, d2;
	 
	    d2 = x * x + y * y; // d*d 
	    d = sqrt( d2 ); // distance from origin 
	 
	    if ( d < 1 )
	        return ( 1.5 * d * d2 - 2.5 * d2 + 1.0 );
	    else if ( d < 2 )
	        return ( -d * d2 * 0.5 + 2.5 * d2 - 4.0 * d + 2.0 );
	    else
	        return 0.0;*/

	/* RI SPec 3.2 */
	RtFloat r2 = (x*x+y*y);
	RtFloat r = sqrt(r2);
	return (r>=2.0)?0.0:
	       (r<1.0)?(3.0*r*r2-5.0*r2+2.0):(-r*r2+5.0*r2-8.0*r+4.0);
}


//----------------------------------------------------------------------
// RiSincFilter
// Sinc filter used as a possible value passed to RiPixelFilter.
//
RtFloat	RiSincFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	//RtFloat d;
	//
	//d = sqrt(x*x+y*y);
	//
	//if(d!=0)
	//	return(sin(RI_PI*d)/(RI_PI*d));
	//else
	//	return(1.0);

	// The above is an un-windowed sinc, below is a windowed sinc
	//   function similar in shape to what PRMan 3.9 uses.
	// tburge 5-28-01

	/* Modified version of the RI Spec 3.2 sinc filter to be
	 *   windowed with a positive lobe of a cosine which is half
	 *   of a cosine period.  
	 */

	/* Uses a -PI to PI cosine window. */
	if ( x != 0.0 )
	{
		x *= RI_PI;
		x = cos( 0.5 * x / xwidth ) * sin( x ) / x;
	}
	else
	{
		x = 1.0;
	}
	if ( y != 0.0 )
	{
		y *= RI_PI;
		y = cos( 0.5 * y / ywidth ) * sin( y ) / y;
	}
	else
	{
		y = 1.0;
	}

	/* This is a square separable filter and is the 2D Fourier
	 * transform of a rectangular box outlining a lowpass bandwidth
	* filter in the frequency domain.
	*/
	return x*y;
}


//----------------------------------------------------------------------
// RiDiskFilter -- this is in Pixar's ri.h
// Cylindrical filter used as a possible value passed to RiPixelFilter
//
RtFloat	RiDiskFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	double d, xx, yy;

	xx = x * x;
	yy = y * y;
	xwidth *= 0.5;
	ywidth *= 0.5;

	d = ( xx ) / ( xwidth * xwidth ) + ( yy ) / ( ywidth * ywidth );
	if ( d < 1.0 )
	{
		return 1.0;
	}
	else
	{
		return 0.0;
	}
}


//----------------------------------------------------------------------
// RiBesselFilter -- this is in Pixar's ri.h
// Besselj0 filter used as a possible value passed to RiPixelFilter
//
RtFloat	RiBesselFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{

	double d, w, xx, yy;

	xx = x * x;
	yy = y * y;

	xwidth *= 0.5;
	ywidth *= 0.5;

	w = ( xx ) / ( xwidth * xwidth ) + ( yy ) / ( ywidth * ywidth );
	if ( w < 1.0 )
	{
		d = sqrt( xx + yy );
		if ( d != 0.0 )
		{
			/* Half cosine window. */
			w = cos( 0.5 * RI_PI * sqrt( w ) );
			return w * 2*j1( RI_PI * d ) / d;
		}
		else
		{
			return RI_PI;
		}
	}
	else
	{
		return 0.0;
	}
}


//----------------------------------------------------------------------
// RiHider
// Specify a hidden surface calculation mode.
//
RtVoid	RiHider( RtToken name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiHiderV( name, count, &aTokens[0], &aValues[0] );

}

//----------------------------------------------------------------------
// RiHiderV
// List based version of above.
//
RtVoid	RiHiderV( RtToken name, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiHider

	Validate_RiHider

	Debug_RiHider

	if ( !strcmp( name, "hidden" ) || !strcmp( name, "painter" ) )
	{
		QGetRenderContext() ->optCurrent().GetStringOptionWrite( "System", "Hider" ) [ 0 ] = name ;
	}

	// Check options.
	TqInt i;
	for ( i = 0; i < count; ++i )
	{
		SqParameterDeclaration Decl;
		try
		{
			Decl = QGetRenderContext()->FindParameterDecl( tokens[ i ] );
		}
		catch( XqException e )
		{
			Aqsis::log() << error << e.strReason().c_str() << std::endl;
			continue;
		}
		TqUlong hash = CqString::hash(Decl.m_strName.c_str());
		if ( hash == RIH_DEPTHFILTER )
			RiOption( "Hider", "depthfilter", ( RtToken ) values[ i ], NULL );
		else if ( hash == RIH_JITTER )
			RiOption( "Hider", "jitter", ( RtFloat* ) values[ i ], NULL );
	}

	return ;
}


//----------------------------------------------------------------------
// RiColorSamples
// Specify the depth and conversion arrays for color manipulation.
//
RtVoid	RiColorSamples( RtInt N, RtFloat *nRGB, RtFloat *RGBn )
{
	Validate_Conditional

	Cache_RiColorSamples

	Validate_RiColorSamples

	Debug_RiColorSamples

	Aqsis::log() << warning << "RiColorSamples not supported" << std::endl;
	return ;
}


//----------------------------------------------------------------------
// RiRelativeDetail
// Set the scale used for all subsequent level of detail calculations.
//
RtVoid	RiRelativeDetail( RtFloat relativedetail )
{
	Validate_Conditional

	Cache_RiRelativeDetail

	Validate_RiRelativeDetail

	Debug_RiRelativeDetail

	if ( relativedetail < 0.0f )
	{
		Aqsis::log() << error << "RiRelativeDetail < 0.0" << std::endl;
	}
	else
	{
		QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "RelativeDetail" ) [ 0 ] = relativedetail;
	}
	return ;
}


//----------------------------------------------------------------------
// RiOption
// Specify system specific option.
//
RtVoid	RiOption( RtToken name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiOptionV( name, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiOptionV
// List based version of above.
//
RtVoid	RiOptionV( RtToken name, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiOption

	Validate_RiOption

	Debug_RiOption

	// Find the parameter on the current options.
	CqNamedParameterList * pOpt = QGetRenderContext() ->optCurrent().pOptionWrite( name ).get();

	RtInt i;
	for ( i = 0; i < count; ++i )
	{
		RtToken	token = tokens[ i ];
		RtPointer	value = values[ i ];

		// Search for the parameter in the declarations.
		// Note Options can only be uniform.
		SqParameterDeclaration Decl;
		try
		{
			Decl = QGetRenderContext()->FindParameterDecl( token );
		}
		catch( XqException e )
		{
			Aqsis::log() << error << e.strReason().c_str() << std::endl;
			continue;
		}
		TqInt Type = Decl.m_Type;
		TqInt Class = Decl.m_Class;
		TqBool bArray = Decl.m_Count > 1;
		CqParameter* pParam = pOpt->pParameter( Decl.m_strName.c_str() );
		if ( pParam == 0 )
		{
			if ( Decl.m_strName != "" && ( Decl.m_Class ) == class_uniform )
			{
				pParam = Decl.m_pCreate( Decl.m_strName.c_str(), Decl.m_Count );
				pOpt->AddParameter( pParam );
			}
			else
			{
				if ( Decl.m_strName == "" )
					Aqsis::log() << warning << "Unrecognised declaration : " << token << std::endl;
				else
					Aqsis::log() << warning << "Options can only be uniform [" << token << "]" << std::endl;
				return ;
			}
		}
		else
		{
			Type = pParam->Type();
			Class = pParam->Class();
			bArray = pParam->Count() > 0;
		}

		switch ( Type )
		{
				case type_float:
				{
					RtFloat * pf = reinterpret_cast<RtFloat*>( value );
					if ( bArray )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); ++j )
							static_cast<CqParameterTypedUniformArray<RtFloat, type_float, RtFloat>*>( pParam ) ->pValue() [ j ] = pf[ j ];
					}
					else
						static_cast<CqParameterTypedUniform<RtFloat, type_float, RtFloat>*>( pParam ) ->pValue() [ 0 ] = pf[ 0 ];
				}
				break;

				case type_integer:
				{
					RtInt* pi = reinterpret_cast<RtInt*>( value );
					if ( bArray )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); ++j )
							static_cast<CqParameterTypedUniformArray<RtInt, type_integer, RtFloat>*>( pParam ) ->pValue() [ j ] = pi[ j ];
					}
					else
						static_cast<CqParameterTypedUniform<RtInt, type_integer, RtFloat>*>( pParam ) ->pValue() [ 0 ] = pi[ 0 ];
				}
				break;

				case type_string:
				{
					char** ps = reinterpret_cast<char**>( value );
					if ( bArray )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); ++j )
						{
							CqString str( "" );
							if ( strcmp( name, "searchpath" ) == 0 )
							{
								// Get the old value for use in escape replacement
								CqString str_old = static_cast<CqParameterTypedUniform<CqString, type_string, CqString>*>( pParam ) ->pValue() [ 0 ];
								// Build the string, checking for & character and replace with old string.
								unsigned int strt = 0;
								unsigned int len = 0;
								while ( 1 )
								{
									if ( ( len = strcspn( &ps[ j ][ strt ], "&" ) ) < strlen( &ps[ j ][ strt ] ) )
									{
										str += CqString( ps[ j ] ).substr( strt, len );
										str += str_old;
										strt += len + 1;
									}
									else
									{
										str += CqString( ps[ j ] ).substr( strt );
										break;
									}
								}
							}
							else
								str = CqString( ps[ j ] );

							static_cast<CqParameterTypedUniformArray<CqString, type_string, CqString>*>( pParam ) ->pValue() [ j ] = str;
						}
					}
					else
					{
						CqString str( "" );
						if ( strcmp( name, "searchpath" ) == 0 )
						{
							// Get the old value for use in escape replacement
							CqString str_old = static_cast<CqParameterTypedUniform<CqString, type_string, CqString>*>( pParam ) ->pValue() [ 0 ];
							// Build the string, checking for & character and replace with old string.
							unsigned int strt = 0;
							unsigned int len = 0;
							while ( 1 )
							{
								if ( ( len = strcspn( &ps[ 0 ][ strt ], "&" ) ) < strlen( &ps[ 0 ][ strt ] ) )
								{
									str += CqString( ps[ 0 ] ).substr( strt, len );
									str += str_old;
									strt += len + 1;
								}
								else
								{
									str += CqString( ps[ 0 ] ).substr( strt );
									break;
								}
							}
						}
						else
							str = CqString( ps[ 0 ] );

						static_cast<CqParameterTyped<CqString, CqString>*>( pParam ) ->pValue() [ 0 ] = str;
					}
				}
				break;

				case type_color:
				{
					RtFloat * pc = reinterpret_cast<RtFloat*>( value );
					if ( bArray )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); j+=3 )
							static_cast<CqParameterTypedUniformArray<CqColor, type_color, CqColor>*>( pParam ) ->pValue() [ j ] = CqColor(pc[ j ], pc[ j+1 ], pc[ j+2 ]);
					}
					else
						static_cast<CqParameterTypedUniform<CqColor, type_color, CqColor>*>( pParam ) ->pValue() [ 0 ] = CqColor(pc[ 0 ], pc[ 1 ], pc[ 2 ]);
				}
				break;

				case type_point:
				{
					RtFloat * pv = reinterpret_cast<RtFloat*>( value );
					if ( bArray )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); j+=3 )
							static_cast<CqParameterTypedUniformArray<CqVector3D, type_point, CqVector3D>*>( pParam ) ->pValue() [ j ] = CqVector3D(pv[ j ], pv[ j+1 ], pv[ j+2 ]);
					}
					else
						static_cast<CqParameterTypedUniform<CqVector3D, type_point, CqVector3D>*>( pParam ) ->pValue() [ 0 ] = CqVector3D(pv[ 0 ], pv[ 1 ], pv[ 2 ]);
				}
				break;

				case type_normal:
				{
					RtFloat * pv = reinterpret_cast<RtFloat*>( value );
					if ( bArray )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); j+=3 )
							static_cast<CqParameterTypedUniformArray<CqVector3D, type_normal, CqVector3D>*>( pParam ) ->pValue() [ j ] = CqVector3D(pv[ j ], pv[ j+1 ], pv[ j+2 ]);
					}
					else
						static_cast<CqParameterTypedUniform<CqVector3D, type_normal, CqVector3D>*>( pParam ) ->pValue() [ 0 ] = CqVector3D(pv[ 0 ], pv[ 1 ], pv[ 2 ]);
				}
				break;

				case type_vector:
				{
					RtFloat * pv = reinterpret_cast<RtFloat*>( value );
					if ( bArray )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); j+=3 )
							static_cast<CqParameterTypedUniformArray<CqVector3D, type_vector, CqVector3D>*>( pParam ) ->pValue() [ j ] = CqVector3D(pv[ j ], pv[ j+1 ], pv[ j+2 ]);
					}
					else
						static_cast<CqParameterTypedUniform<CqVector3D, type_vector, CqVector3D>*>( pParam ) ->pValue() [ 0 ] = CqVector3D(pv[ 0 ], pv[ 1 ], pv[ 2 ]);
				}
				break;

				case type_hpoint:
				{
					RtFloat * pv = reinterpret_cast<RtFloat*>( value );
					if ( bArray )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); j+=4 )
							static_cast<CqParameterTypedUniformArray<CqVector4D, type_hpoint, CqVector3D>*>( pParam ) ->pValue() [ j ] = CqVector4D(pv[ j ], pv[ j+1 ], pv[ j+2 ], pv[ j+3 ]);
					}
					else
						static_cast<CqParameterTypedUniform<CqVector4D, type_hpoint, CqVector3D>*>( pParam ) ->pValue() [ 0 ] = CqVector4D(pv[ 0 ], pv[ 1 ], pv[ 2 ], pv[ 3 ]);
				}
				break;

				case type_matrix:
				{
					RtFloat * pm = reinterpret_cast<RtFloat*>( value );
					if ( bArray )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); j+=16 )
							static_cast<CqParameterTypedUniformArray<CqMatrix, type_matrix, CqMatrix>*>( pParam ) ->pValue() [ j ] = CqMatrix(pm[ j    ], pm[ j+1  ], pm[ j+2  ], pm[ j+3  ],
							        pm[ j+4  ], pm[ j+5  ], pm[ j+6  ], pm[ j+7  ],
							        pm[ j+8  ], pm[ j+9  ], pm[ j+10 ], pm[ j+11 ],
							        pm[ j+12 ], pm[ j+13 ], pm[ j+14 ], pm[ j+15 ]);
					}
					else
						static_cast<CqParameterTypedUniform<CqMatrix, type_matrix, CqMatrix>*>( pParam ) ->pValue() [ 0 ] = CqMatrix(pm[ 0  ], pm[ 1  ], pm[ 2  ], pm[ 3  ],
						        pm[ 4  ], pm[ 5  ], pm[ 6  ], pm[ 7  ],
						        pm[ 8  ], pm[ 9  ], pm[ 10 ], pm[ 11 ],
						        pm[ 12 ], pm[ 13 ], pm[ 14 ], pm[ 15 ]);
				}
				break;
		}
	}
	return ;
}


//----------------------------------------------------------------------
// RiAttributeBegin
// Begin a ne attribute definition, pushes the current attributes.
//
RtVoid	RiAttributeBegin()
{
	Validate_Conditional

	Cache_RiAttributeBegin

	Validate_RiAttributeBegin

	Debug_RiAttributeBegin

	QGetRenderContext() ->BeginAttributeModeBlock();

	return ;
}


//----------------------------------------------------------------------
// RiAttributeEnd
// End the current attribute defintion, pops the previous attributes.
//
RtVoid	RiAttributeEnd()
{
	Validate_Conditional

	Cache_RiAttributeEnd

	Validate_RiAttributeEnd

	Debug_RiAttributeEnd

	QGetRenderContext() ->EndAttributeModeBlock();

	return ;
}


//----------------------------------------------------------------------
// RiColor
//	Set the current color for use by the geometric primitives.
//
RtVoid	RiColor( RtColor Cq )
{
	Validate_Conditional

	Cache_RiColor

	Validate_RiColor

	Debug_RiColor

	QGetRenderContext() ->pattrWriteCurrent() ->GetColorAttributeWrite( "System", "Color" ) [ 0 ] = CqColor( Cq );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiOpacity
// Set the current opacity, for use by the geometric primitives.
//
RtVoid	RiOpacity( RtColor Os )
{
	Validate_Conditional

	Cache_RiOpacity

	Validate_RiOpacity

	Debug_RiOpacity

	QGetRenderContext() ->pattrWriteCurrent() ->GetColorAttributeWrite( "System", "Opacity" ) [ 0 ] = CqColor( Os );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiTextureCoordinates
// Set the current texture coordinates used by the parametric geometric primitives.
//
RtVoid	RiTextureCoordinates( RtFloat s1, RtFloat t1,
                             RtFloat s2, RtFloat t2,
                             RtFloat s3, RtFloat t3,
                             RtFloat s4, RtFloat t4 )
{
	Validate_Conditional

	Cache_RiTextureCoordinates

	Validate_RiTextureCoordinates

	Debug_RiTextureCoordinates

	TqFloat * pTC = QGetRenderContext() ->pattrWriteCurrent() ->GetFloatAttributeWrite( "System", "TextureCoordinates" );

	assert( NULL != pTC );

	pTC[ 0 ] = s1;
	pTC[ 1 ] = t1;
	pTC[ 2 ] = s2;
	pTC[ 3 ] = t2;
	pTC[ 4 ] = s3;
	pTC[ 5 ] = t3;
	pTC[ 6 ] = s4;
	pTC[ 7 ] = t4;
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiLightSource
// Create a new light source at the current transformation.
//
RtLightHandle	RiLightSource( RtToken name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	return ( RiLightSourceV( name, count, &aTokens[0], &aValues[0] ) );
}


//----------------------------------------------------------------------
// RiLightSourceV
// List based version of above.
//
RtLightHandle	RiLightSourceV( RtToken name, PARAMETERLIST )
{
	Validate_Conditional0

	Cache_RiLightSource

	Validate_RiLightSource

	Debug_RiLightSource

	// Find the lightsource shader.
	//IqShader * pShader = static_cast<CqShader*>( QGetRenderContext() ->CreateShader( name, Type_Lightsource ) );
	boost::shared_ptr<IqShader> pShader = QGetRenderContext()->CreateShader( name, Type_Lightsource );

	// TODO: Report error.
	if ( !pShader )
		return ( 0 );

	pShader->matCurrent() = QGetRenderContext() ->ptransCurrent()->matObjectToWorld(QGetRenderContext()->Time());
	CqLightsourcePtr pNew( new CqLightsource( pShader, RI_TRUE ) );
	Lightsource_stack.push_back(pNew);

	// Execute the intiialisation code here, as we now have our shader context complete.
	pShader->PrepareDefArgs();

	if ( pNew )
	{
		RtInt i;
		for ( i = 0; i < count; ++i )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			SetShaderArgument( pShader, token, static_cast<TqPchar>( value ) );
		}
		QGetRenderContext() ->pattrWriteCurrent() ->AddLightsource( pNew );
		// If this light is being defined outside the WorldBegin, then we can
		// go ahead and initialise the parameters, as they are invariant under changes to the camera space.
		if(!QGetRenderContext()->IsWorldBegin())
			pShader->InitialiseParameters();	

		// Add it as a Context light as well in case we are in a context that manages it's own lights.
		QGetRenderContext() ->pconCurrent() ->AddContextLightSource( pNew );
		return ( reinterpret_cast<RtLightHandle>( pNew.get() ) );
	}
	return ( 0 );
}


//----------------------------------------------------------------------
// RiAreaLightSource
// Create a new area light source at the current transformation, all
// geometric primitives until the next RiAttributeEnd, become part of this
// area light source.
//
RtLightHandle	RiAreaLightSource( RtToken name, ... )
{

	va_list	pArgs;
	va_start( pArgs, name );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	return ( RiAreaLightSourceV( name, count, &aTokens[0], &aValues[0] ) );
}


//----------------------------------------------------------------------
// RiAreaLightSourceV
// List based version of above.
//
RtLightHandle	RiAreaLightSourceV( RtToken name, PARAMETERLIST )
{
	Validate_Conditional0

	Cache_RiAreaLightSource

	Validate_RiAreaLightSource

	Debug_RiAreaLightSource

	Aqsis::log() << warning << "RiAreaLightSource not supported, will produce a point light" << std::endl;

	return ( RiLightSourceV( name, count, tokens, values ) );
}


//----------------------------------------------------------------------
// RiIlluminate
// Set the current status of the specified light source.
//
RtVoid	RiIlluminate( RtLightHandle light, RtBoolean onoff )
{
	Validate_Conditional

	Cache_RiIlluminate

	Validate_RiIlluminate

	Debug_RiIlluminate

	CqLightsourcePtr pL( reinterpret_cast<CqLightsource*>( light )->shared_from_this() );

	// Check if we are turning the light on or off.
	if ( light == NULL ) return ;
	if ( onoff )
		QGetRenderContext() ->pattrWriteCurrent() ->AddLightsource( pL );
	else
		QGetRenderContext() ->pattrWriteCurrent() ->RemoveLightsource( pL );
	return ;
}


//----------------------------------------------------------------------
// RiSurface
// Set the current surface shader, used by geometric primitives.
//
RtVoid	RiSurface( RtToken name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiSurfaceV( name, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiSurfaceV
// List based version of above.
//
RtVoid	RiSurfaceV( RtToken name, PARAMETERLIST )
{

	Validate_Conditional

	Cache_RiSurface

	Validate_RiSurface

	Debug_RiSurface

	// Find the shader.
	//IqShader * pshadSurface = QGetRenderContext() ->CreateShader( name, Type_Surface );
	boost::shared_ptr<IqShader> pshadSurface = QGetRenderContext()->CreateShader( name, Type_Surface );

	if ( pshadSurface )
	{
		TqFloat time = QGetRenderContext()->Time();
		pshadSurface->matCurrent() = QGetRenderContext() ->matCurrent(time);
		// Execute the intiialisation code here, as we now have our shader context complete.
		pshadSurface->PrepareDefArgs();
		RtInt i;
		for ( i = 0; i < count; ++i )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			SetShaderArgument( pshadSurface, token, static_cast<TqPchar>( value ) );
		}
		QGetRenderContext() ->pattrWriteCurrent() ->SetpshadSurface( pshadSurface, QGetRenderContext() ->Time() );
	}
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiAtmosphere
// Set the current atrmospheric shader.
//
RtVoid	RiAtmosphere( RtToken name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiAtmosphereV( name, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiAtmosphereV
// List based version of above.
//
RtVoid	RiAtmosphereV( RtToken name, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiAtmosphere

	Validate_RiAtmosphere

	Debug_RiAtmosphere

	// Find the shader.
	boost::shared_ptr<IqShader> pshadAtmosphere = QGetRenderContext()->CreateShader( name, Type_Volume );

	if ( pshadAtmosphere )
	{
		pshadAtmosphere->matCurrent() = QGetRenderContext() ->matCurrent(QGetRenderContext()->Time());
		// Execute the intiialisation code here, as we now have our shader context complete.
		pshadAtmosphere->PrepareDefArgs();
		RtInt i;
		for ( i = 0; i < count; ++i )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			SetShaderArgument( pshadAtmosphere, token, static_cast<TqPchar>( value ) );
		}
	}

	QGetRenderContext() ->pattrWriteCurrent() ->SetpshadAtmosphere( pshadAtmosphere, QGetRenderContext() ->Time() );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiInterior
// Set the current interior volumetric shader.
//
RtVoid	RiInterior( RtToken name, ... )
{
	Aqsis::log() << warning << "RiInterior not supported" << std::endl;
	return ;
}


//----------------------------------------------------------------------
// RiInteriorV
// List based version of above.
//
RtVoid	RiInteriorV( RtToken name, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiInterior

	Validate_RiInterior

	Debug_RiInterior

	Aqsis::log() << warning << "RiInterior not supported" << std::endl;
	return ;
}


//----------------------------------------------------------------------
// RiExterior
// Set the current exterior volumetric shader.
//
RtVoid	RiExterior( RtToken name, ... )
{
	Aqsis::log() << warning << "RiExterior not supported" << std::endl;
	return ;
}


//----------------------------------------------------------------------
// RiExteriorV
// List based version of above.
//
RtVoid	RiExteriorV( RtToken name, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiExterior

	Validate_RiExterior

	Debug_RiExterior

	Aqsis::log() << warning << "ExInterior not supported" << std::endl;
	return ;
}


//----------------------------------------------------------------------
// RiShadingRate
// Specify the size of the shading area in pixels.
//
RtVoid	RiShadingRate( RtFloat size )
{
	Validate_Conditional

	Cache_RiShadingRate

	Validate_RiShadingRate

	Debug_RiShadingRate

	CqLogRangeCheckCallback rc;

	bool valid = true;

	rc.set( "size" );
	if( !CheckMinMax( size, 0.0f, RI_INFINITY, &rc ) )
	{
		valid = false;
	}

	if( !valid )
	{
		Aqsis::log() << warning << "Invalid ShadingRate, ShadingRate set to 1" << std::endl;
		size = 1;
	}

	QGetRenderContext() ->pattrWriteCurrent() ->GetFloatAttributeWrite( "System", "ShadingRate" ) [ 0 ] = size;
	QGetRenderContext() ->pattrWriteCurrent() ->GetFloatAttributeWrite( "System", "ShadingRateSqrt" ) [ 0 ] = sqrt( size );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiShadingInterpolation
// Specify the method of shading interpolation.
//
RtVoid	RiShadingInterpolation( RtToken type )
{
	Validate_Conditional

	Cache_RiShadingInterpolation

	Validate_RiShadingInterpolation

	Debug_RiShadingInterpolation

	if ( strcmp( type, RI_CONSTANT ) == 0 )
		QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "ShadingInterpolation" ) [ 0 ] = ShadingConstant;
	else
		if ( strcmp( type, RI_SMOOTH ) == 0 )
			QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "ShadingInterpolation" ) [ 0 ] = ShadingSmooth;
		else
			Aqsis::log() << error << "RiShadingInterpolation unrecognised value \"" << type << "\"" << std::endl;

	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiMatte
// Set the matte state of subsequent geometric primitives.
//
RtVoid	RiMatte( RtBoolean onoff )
{
	Validate_Conditional

	Cache_RiMatte

	Validate_RiMatte

	Debug_RiMatte

	QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "Matte" ) [ 0 ] = onoff != 0;
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiBound
// Set the bounding cube of the current primitives.
//
RtVoid	RiBound( RtBound bound )
{
	Validate_Conditional

	Cache_RiBound

	Validate_RiBound

	Debug_RiBound

	// TODO: Need to add a "Bound" attribute here, and fill it in.
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiDetail
// Set the current bounding cube for use by level of detail calculation.
//
RtVoid	RiDetail( RtBound bound )
{
	Validate_Conditional

	Cache_RiDetail

	Validate_RiDetail

	Debug_RiDetail

	CqBound Bound( bound );

	TqFloat* boundAttr = QGetRenderContext() ->pattrWriteCurrent() ->GetFloatAttributeWrite( "System", "LODBound" );
	boundAttr[0] = bound[0];
	boundAttr[1] = bound[1];
	boundAttr[2] = bound[2];
	boundAttr[3] = bound[3];
	boundAttr[4] = bound[4];
	boundAttr[5] = bound[5];

	return ;
}


//----------------------------------------------------------------------
// RiDetailRange
// Set the visible range of any subsequent geometric primitives.
//
RtVoid	RiDetailRange( RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh )
{
	Validate_Conditional

	Cache_RiDetailRange

	Validate_RiDetailRange

	Debug_RiDetailRange

	if ( offlow > onlow || onhigh > offhigh )
	{
		Aqsis::log() << error << "RiDetailRange invalid range" << std::endl;
		return ;
	}

	TqFloat* rangeAttr = QGetRenderContext() ->pattrWriteCurrent() ->GetFloatAttributeWrite( "System", "LODRanges" );
	rangeAttr[0] = offlow;
	rangeAttr[1] = onlow;
	rangeAttr[2] = onhigh;
	rangeAttr[3] = offhigh;
	return ;
}


//----------------------------------------------------------------------
// RiGeometricApproximation
// Specify any parameters used by approximation functions during rendering.
//
RtVoid	RiGeometricApproximation( RtToken type, RtFloat value )
{
	Validate_Conditional

	Cache_RiGeometricApproximation

	Validate_RiGeometricApproximation

	Debug_RiGeometricApproximation

	Aqsis::log() << warning << "RiGeometricApproximation not supported" << std::endl;
	return ;
}


//----------------------------------------------------------------------
// RiOrientation
// Set the handedness of any subsequent geometric primitives.
//
RtVoid	RiOrientation( RtToken orientation )
{
	Validate_Conditional

	Cache_RiOrientation

	Validate_RiOrientation

	Debug_RiOrientation

	if ( orientation != 0 )
	{
		if ( strstr( orientation, RI_RH ) != 0 )
			QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "Orientation" ) [ 0 ] = ( QGetRenderContext() ->ptransCurrent()->GetHandedness(QGetRenderContext()->Time()) ) ? 0 : 1;
		if ( strstr( orientation, RI_LH ) != 0 )
			QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "Orientation" ) [ 0 ] = ( QGetRenderContext() ->ptransCurrent()->GetHandedness(QGetRenderContext()->Time()) ) ? 1 : 0;
		if ( strstr( orientation, RI_INSIDE ) != 0 )
			QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "Orientation" ) [ 0 ] = 1;
		if ( strstr( orientation, RI_OUTSIDE ) != 0 )
			QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "Orientation" ) [ 0 ] = 0;
	}
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiReverseOrientation
// Reverse the handedness of any subsequent geometric primitives.
//
RtVoid	RiReverseOrientation()
{
	Validate_Conditional

	Cache_RiReverseOrientation

	Validate_RiReverseOrientation

	Debug_RiReverseOrientation

	QGetRenderContext() ->pattrWriteCurrent() ->FlipeOrientation( QGetRenderContext() ->Time() );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiSides
// Set the number of visibles sides for any subsequent geometric primitives.
//
RtVoid	RiSides( RtInt nsides )
{
	Validate_Conditional

	Cache_RiSides

	Validate_RiSides

	Debug_RiSides

	QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "Sides" ) [ 0 ] = nsides;
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiIdentity
// Set the current transformation to the identity matrix.
//
RtVoid	RiIdentity()
{
	Validate_Conditional

	Cache_RiIdentity

	Validate_RiIdentity

	Debug_RiIdentity

	QGetRenderContext() ->ptransSetTime( CqMatrix() );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// Set the current transformation to the specified matrix.
//
RtVoid	RiTransform( RtMatrix transform )
{
	Validate_Conditional

	Cache_RiTransform

	Validate_RiTransform

	Debug_RiTransform

	CqMatrix matTrans( transform );
	//    if ( matTrans.Determinant() < 0 && ( QGetRenderContext()->pconCurrent()->Type() != Motion || QGetRenderContext()->pconCurrent()->TimeIndex() == 0 ) )
	//        QGetRenderContext() ->ptransWriteCurrent() ->FlipHandedness( QGetRenderContext() ->Time() );

	if( QGetRenderContext()->IsWorldBegin() )
	{
		CqTransformPtr newTrans( new CqTransform( QGetRenderContext()->GetDefObjTransform() ) );
		QGetRenderContext() ->pconCurrent()->ptransSetCurrent( newTrans );
		QGetRenderContext() ->ptransConcatCurrentTime( CqMatrix( transform ) );
	}
	else
		QGetRenderContext() ->ptransSetCurrentTime( CqMatrix( transform ) );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiConcatTransform
// Concatenate the specified matrix into the current transformation matrix.
//
RtVoid	RiConcatTransform( RtMatrix transform )
{
	Validate_Conditional

	Cache_RiConcatTransform

	Validate_RiConcatTransform

	Debug_RiConcatTransform

	// Check if this transformation results in a change in orientation.
	CqMatrix matTrans( transform );
	//    if ( matTrans.Determinant() < 0 && ( QGetRenderContext()->pconCurrent()->Type() != Motion || QGetRenderContext()->pconCurrent()->TimeIndex() == 0 ) )
	//        QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );

	QGetRenderContext() ->ptransConcatCurrentTime( CqMatrix( transform ) );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiPerspective
// Concatenate a perspective transformation into the current transformation.
//
RtVoid	RiPerspective( RtFloat fov )
{
	Validate_Conditional

	Cache_RiPerspective

	Validate_RiPerspective

	Debug_RiPerspective

	if ( fov <= 0 )
	{
		Aqsis::log() << error << "RiPerspective invalid FOV" << std::endl;
		return ;
	}

	fov = tan( RAD( fov / 2 ) );

	// This matches PRMan 3.9 in testing, but not BMRT 2.6's rgl and rendrib.
	CqMatrix	matP( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, fov, fov, 0, 0, -fov, 0 );

	// Check if this transformation results in a change in orientation.
	//    if ( matP.Determinant() < 0 && ( QGetRenderContext()->pconCurrent()->Type() != Motion || QGetRenderContext()->pconCurrent()->TimeIndex() == 0 ) )
	//        QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );

	QGetRenderContext() ->ptransConcatCurrentTime( matP );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiTranslate
// Concatenate a translation into the current transformation.
//
RtVoid	RiTranslate( RtFloat dx, RtFloat dy, RtFloat dz )
{
	Validate_Conditional

	Cache_RiTranslate

	Validate_RiTranslate

	Debug_RiTranslate

	CqMatrix	matTrans( CqVector3D( dx, dy, dz ) );
	// Check if this transformation results in a change in orientation.
	//    if ( matTrans.Determinant() < 0 && ( QGetRenderContext()->pconCurrent()->Type() != Motion || QGetRenderContext()->pconCurrent()->TimeIndex() == 0 ) )
	//        QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );

	QGetRenderContext() ->ptransConcatCurrentTime( matTrans );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiRotate
// Concatenate a rotation into the current transformation.
//
RtVoid	RiRotate( RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz )
{
	Validate_Conditional

	Cache_RiRotate

	Validate_RiRotate

	Debug_RiRotate

	CqMatrix	matRot( RAD( angle ), CqVector4D( dx, dy, dz ) );
	// Check if this transformation results in a change in orientation.
	//    if ( matRot.Determinant() < 0 && ( QGetRenderContext()->pconCurrent()->Type() != Motion || QGetRenderContext()->pconCurrent()->TimeIndex() == 0 ) )
	//        QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );

	QGetRenderContext() ->ptransConcatCurrentTime( matRot );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiScale
// Concatenate a scale into the current transformation.
//
RtVoid	RiScale( RtFloat sx, RtFloat sy, RtFloat sz )
{
	Validate_Conditional

	Cache_RiScale

	Validate_RiScale

	Debug_RiScale

	CqMatrix	matScale( sx, sy, sz );
	// Check if this transformation results in a change in orientation.
	//    if ( matScale.Determinant() < 0 && ( QGetRenderContext()->pconCurrent()->Type() != Motion || QGetRenderContext()->pconCurrent()->TimeIndex() == 0 ) )
	//        QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );

	QGetRenderContext() ->ptransConcatCurrentTime( matScale );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiSkew
// Concatenate a skew into the current transformation.
//
RtVoid	RiSkew( RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
               RtFloat dx2, RtFloat dy2, RtFloat dz2 )
{
	Validate_Conditional

	Cache_RiSkew

	Validate_RiSkew

	Debug_RiSkew

	CqMatrix	matSkew( RAD( angle ), dx1, dy1, dz1, dx2, dy2, dz2 );

	// This transformation can not change orientation.

	QGetRenderContext() ->ptransConcatCurrentTime( matSkew );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiDeformation
// Specify a deformation shader to be included into the current transformation.
//
RtVoid	RiDeformation( RtToken name, ... )
{
	Aqsis::log() << warning << "RiDeformation not supported" << std::endl;
	return ;
}


//----------------------------------------------------------------------
// RiDeformationV
// List based version of above.
//
RtVoid	RiDeformationV( RtToken name, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiDeformation

	Validate_RiDeformation

	Debug_RiDeformation

	Aqsis::log() << warning << "RiDeformation not supported" << std::endl;
	return ;
}


//----------------------------------------------------------------------
// RiDisplacement
// Specify the current displacement shade used by geometric primitives.
//
RtVoid	RiDisplacement( RtToken name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiDisplacementV( name, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiDisplacementV
// List based version of above.
//
RtVoid	RiDisplacementV( RtToken name, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiDisplacement

	Validate_RiDisplacement

	Debug_RiDisplacement

	// Find the shader.
	boost::shared_ptr<IqShader> pshadDisplacement = QGetRenderContext() ->CreateShader( name, Type_Displacement );

	if ( pshadDisplacement )
	{
		pshadDisplacement->matCurrent() = QGetRenderContext() ->matCurrent(QGetRenderContext()->Time());
		// Execute the intiialisation code here, as we now have our shader context complete.
		pshadDisplacement->PrepareDefArgs();
		RtInt i;
		for ( i = 0; i < count; ++i )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			SetShaderArgument( pshadDisplacement, token, static_cast<TqPchar>( value ) );
		}
	}

	QGetRenderContext() ->pattrWriteCurrent() ->SetpshadDisplacement( pshadDisplacement, QGetRenderContext() ->Time() );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiCoordinateSystem
// Save the current coordinate system as the specified name.
//
RtVoid	RiCoordinateSystem( RtToken space )
{
	Validate_Conditional

	Cache_RiCoordinateSystem

	Validate_RiCoordinateSystem

	Debug_RiCoordinateSystem

	// Insert the named coordinate system into the list help on the renderer.
	QGetRenderContext() ->SetCoordSystem( space, QGetRenderContext() ->matCurrent( QGetRenderContext() ->Time() ) );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// ---Additional to spec. v3.1---
// RiCoordSysTransform
// Replace the current transform with the named space.

RtVoid	RiCoordSysTransform( RtToken space )
{
	Validate_Conditional

	Cache_RiCoordSysTransform

	Validate_RiCoordSysTransform

	Debug_RiCoordSysTransform

	// Insert the named coordinate system into the list help on the renderer.
	QGetRenderContext() ->ptransSetTime( QGetRenderContext() ->matSpaceToSpace( space, "world", CqMatrix(), CqMatrix(), QGetRenderContext()->Time() ) );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiTransformPoints
// Transform a list of points from one coordinate system to another.
//
RtPoint*	RiTransformPoints( RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[] )
{
	Cache_RiTransformPoints
	CqMatrix m1, m2;

	Validate_RiTransformPoints

	Debug_RiTransformPoints

	if (!IfOk)
		return points;

	CqMatrix matCToW = QGetRenderContext() ->matSpaceToSpace( fromspace,
	                   tospace, m1, m2, QGetRenderContextI()->Time() );

	if (matCToW.fIdentity() != TqTrue)
	{
		for(TqInt i =0; i< npoints; i++)
		{
			CqVector3D tmp = points[i];
			tmp = tmp * matCToW;
			points[i][0] = tmp.x();
			points[i][1] = tmp.y();
			points[i][2] = tmp.z();
		}
	}

	return ( points );
}


//----------------------------------------------------------------------
// RiTransformBegin
// Push the current transformation state.
//
RtVoid	RiTransformBegin()
{
	Validate_Conditional

	Cache_RiTransformBegin

	Validate_RiTransformBegin

	Debug_RiTransformBegin

	QGetRenderContext() ->BeginTransformModeBlock();

	return ;
}


//----------------------------------------------------------------------
// RiTransformEnd
// Pop the previous transformation state.
//
RtVoid	RiTransformEnd()
{
	Validate_Conditional

	Cache_RiTransformEnd

	Validate_RiTransformEnd

	Debug_RiTransformEnd

	QGetRenderContext() ->EndTransformModeBlock();

	return ;
}


//----------------------------------------------------------------------
// RiAttribute
// Set a system specific attribute.
//
RtVoid	RiAttribute( RtToken name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	TqUlong hash = CqString::hash(name);

	if (hash == RIH_RENDER)
		return;
	if (hash == RIH_INDIRECT)
		return;
	if (hash == RIH_LIGHT)
		return;
	if (hash == RIH_VISIBILITY)
		return;

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiAttributeV( name, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiAttributeV
// List based version of above.
//
RtVoid	RiAttributeV( RtToken name, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiAttribute

	Validate_RiAttribute

	Debug_RiAttribute

	TqUlong hash = CqString::hash(name);

	if (hash == RIH_RENDER)
		return;
	if (hash == RIH_INDIRECT)
		return;
	if (hash == RIH_LIGHT)
		return;
	if (hash == RIH_VISIBILITY)
		return;

	// Find the parameter on the current options.
	CqNamedParameterList * pAttr = QGetRenderContext() ->pattrWriteCurrent() ->pAttributeWrite( name ).get();

	RtInt i;
	for ( i = 0; i < count; ++i )
	{
		RtToken	token = tokens[ i ];
		RtPointer	value = values[ i ];

		TqInt Type = 0;
		TqInt Class = 0;
		TqBool bArray = false;
		CqParameter* pParam = pAttr->pParameter( token );
		if ( pParam == 0 )
		{
			// Search for the parameter in the declarations.
			// Note attributes can only be uniform.
			SqParameterDeclaration Decl;
			try
			{
				Decl = QGetRenderContext()->FindParameterDecl( token );
			}
			catch( XqException e )
			{
				Aqsis::log() << error << e.strReason().c_str() << std::endl;
				continue;
			}
			if ( Decl.m_strName != "" && Decl.m_Class == class_uniform )
			{
				pParam = Decl.m_pCreate( Decl.m_strName.c_str(), Decl.m_Count );
				Type = Decl.m_Type;
				Class = Decl.m_Class;
				bArray = Decl.m_Count > 0;
				pAttr->AddParameter( pParam );
			}
			else
			{
				if ( Decl.m_strName == "" )
					Aqsis::log() << warning << "Unrecognised declaration \"" << token << "\"" << std::endl;
				else
					Aqsis::log() << warning << "Attributes can only be uniform" << std::endl;
				return ;
			}
		}
		else
		{
			Type = pParam->Type();
			Class = pParam->Class();
			bArray = pParam->Count() > 0;
		}

		switch ( Type )
		{
				case type_float:
				{
					RtFloat * pf = reinterpret_cast<RtFloat*>( value );
					if ( bArray )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); ++j )
							static_cast<CqParameterTypedUniformArray<RtFloat, type_float, RtFloat>*>( pParam ) ->pValue() [ j ] = pf[ j ];
					}
					else
						static_cast<CqParameterTypedUniform<RtFloat, type_float, RtFloat>*>( pParam ) ->pValue() [ 0 ] = pf[ 0 ];
				}
				break;

				case type_integer:
				{
					RtInt* pi = reinterpret_cast<RtInt*>( value );
					if ( bArray )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); ++j )
							static_cast<CqParameterTypedUniformArray<RtInt, type_integer, RtFloat>*>( pParam ) ->pValue() [ j ] = pi[ j ];
					}
					else
						static_cast<CqParameterTypedUniform<RtInt, type_integer, RtFloat>*>( pParam ) ->pValue() [ 0 ] = pi[ 0 ];
				}
				break;

				case type_string:
				{
					char** ps = reinterpret_cast<char**>( value );
					if ( bArray )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); ++j )
						{
							CqString str( ps[ j ] );
							static_cast<CqParameterTypedUniform<CqString, type_string, RtFloat>*>( pParam ) ->pValue() [ j ] = str;
						}
					}
					else
					{
						CqString str( ps[ 0 ] );
						static_cast<CqParameterTypedUniform<CqString, type_string, RtFloat>*>( pParam ) ->pValue() [ 0 ] = str;
					}
#ifdef REQUIRED
					if( (strcmp(name, "identifier")==0) && (strcmp(token, "name")==0))
						Aqsis::log() << info << "Identifier: " << ps[ 0 ] << std::endl;
#endif

				}
				// TODO: Rest of parameter types.
		}
	}
	return ;
}


//----------------------------------------------------------------------
// RiPolygon
// Specify a coplanar, convex polygon.
//
RtVoid	RiPolygon( RtInt nvertices, ... )
{
	va_list	pArgs;
	va_start( pArgs, nvertices );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiPolygonV( nvertices, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiPolygonV
// List based version of above.
//
RtVoid	RiPolygonV( RtInt nvertices, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiPolygon

	Validate_RiPolygon

	Debug_RiPolygon

	// Create a new polygon surface primitive.
	boost::shared_ptr<CqSurfacePolygon> pSurface( new CqSurfacePolygon( nvertices ) );

	// Process any specified primitive variables.
	if ( ProcessPrimitiveVariables( pSurface.get(), count, tokens, values ) )
	{
		if ( !pSurface->CheckDegenerate() )
		{
			TqFloat time = QGetRenderContext()->Time();
			// Transform the points into camera space for processing,
			pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );
			CreateGPrim( pSurface );
		}
		else
		{
			Aqsis::log() << error << "Found degenerate polygon" << std::endl;
		}
	}

	return ;
}


//----------------------------------------------------------------------
// RiGeneralPolygon
// Specify a nonconvex coplanar polygon.
//
RtVoid	RiGeneralPolygon( RtInt nloops, RtInt nverts[], ... )
{
	va_list	pArgs;
	va_start( pArgs, nverts );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiGeneralPolygonV( nloops, nverts, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiGeneralPolygonV
// List based version of above.
//
RtVoid	RiGeneralPolygonV( RtInt nloops, RtInt nverts[], PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiGeneralPolygon

	Validate_RiGeneralPolygon

	Debug_RiGeneralPolygon

	TqInt iloop;

	// Calcualte how many points there are.
	TqInt cVerts = 0;
	for ( iloop = 0; iloop < nloops; ++iloop )
	{
		cVerts += nverts[ iloop ];
		// Check for degenerate loops.
		if( nverts[ iloop ] < 3 )
		{
			CqString objname( "unnamed" );
			const CqString* pattrName = QGetRenderContext()->pattrCurrent()->GetStringAttribute( "identifier", "name" );
			if ( pattrName != 0 )
				objname = pattrName[ 0 ];
			Aqsis::log() << warning << "Degenerate loop in GeneralPolygon object \"" << objname.c_str() << "\"" << std::endl;
		}
	}

	// Create a storage class for all the points.
	boost::shared_ptr<CqPolygonPoints> pPointsClass( new CqPolygonPoints( cVerts, 1, cVerts ) );
	// Process any specified primitive variables
	if ( ProcessPrimitiveVariables( pPointsClass.get(), count, tokens, values ) )
	{
		pPointsClass->SetDefaultPrimitiveVariables( RI_FALSE );

		// Work out which plane to project to.
		TqFloat	MinX, MaxX;
		TqFloat	MinY, MaxY;
		TqFloat	MinZ, MaxZ;
		CqVector3D	vecTemp = pPointsClass->P()->pValue( 0 )[0];
		MinX = MaxX = vecTemp.x();
		MinY = MaxY = vecTemp.y();
		MinZ = MaxZ = vecTemp.z();

		// We need to take into account Orientation here.
		TqBool O = QGetRenderContext()->pattrCurrent() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ] != 0;

		TqUint iVert;
		for ( iVert = 1; iVert < pPointsClass->P() ->Size(); ++iVert )
		{
			vecTemp = pPointsClass->P()->pValue( iVert )[0];
			MinX = ( MinX < vecTemp.x() ) ? MinX : vecTemp.x();
			MinY = ( MinY < vecTemp.y() ) ? MinY : vecTemp.y();
			MinZ = ( MinZ < vecTemp.z() ) ? MinZ : vecTemp.z();
			MaxX = ( MaxX > vecTemp.x() ) ? MaxX : vecTemp.x();
			MaxY = ( MaxY > vecTemp.y() ) ? MaxY : vecTemp.y();
			MaxZ = ( MaxZ > vecTemp.z() ) ? MaxZ : vecTemp.z();
		}
		TqFloat	DiffX = MaxX - MinX;
		TqFloat	DiffY = MaxY - MinY;
		TqFloat	DiffZ = MaxZ - MinZ;

		TqInt Axis;
		if ( DiffX < DiffY && DiffX < DiffZ )
			Axis = CqPolygonGeneral2D::Axis_YZ;
		else if ( DiffY < DiffX && DiffY < DiffZ )
			Axis = CqPolygonGeneral2D::Axis_XZ;
		else
			Axis = CqPolygonGeneral2D::Axis_XY;

		// Create a general 2D polygon using the points in each loop.
		CqPolygonGeneral2D poly;
		TqUint ipoint = 0;
		for ( iloop = 0; iloop < nloops; ++iloop )
		{
			CqPolygonGeneral2D polya;
			polya.SetAxis( Axis );
			polya.SetpVertices( pPointsClass );
			TqInt ivert;
			for ( ivert = 0; ivert < nverts[ iloop ]; ++ivert )
			{
				assert( ipoint < pPointsClass->P() ->Size() );
				polya.aiVertices().push_back( ipoint++ );
			}
			if ( iloop == 0 )
			{
				/// \note: We need to check here if the orientation of the projected poly matches the
				/// expected one, of not, we must swap the direction so that the triangulation routines can
				/// correctly determine the inside/outside nature of points. However, if doing so breaks the
				/// orientation as expected by the rest of the renderer, we need to flip the orientation
				/// attribute as well so that normals are correctly calculated.
				if( O )
				{
					if ( polya.CalcOrientation() != CqPolygonGeneral2D::Orientation_AntiClockwise )
					{
						QGetRenderContext() ->pattrWriteCurrent()->GetIntegerAttributeWrite( "System", "Orientation" ) [ 0 ] = 0;
						polya.SwapDirection();
					}
				}
				else
				{
					if ( polya.CalcOrientation() != CqPolygonGeneral2D::Orientation_Clockwise )
					{
						QGetRenderContext() ->pattrWriteCurrent()->GetIntegerAttributeWrite( "System", "Orientation" ) [ 0 ] = 1;
						polya.SwapDirection();
					}
				}
				poly = polya;
			}
			else
			{
				if( O )
				{
					if ( polya.CalcOrientation() != CqPolygonGeneral2D::Orientation_Clockwise )
						polya.SwapDirection();
				}
				else
				{
					if ( polya.CalcOrientation() != CqPolygonGeneral2D::Orientation_AntiClockwise )
						polya.SwapDirection();
				}
				poly.Combine( polya );
			}
		}
		// Now triangulate the general polygon

		std::vector<TqInt>	aiTriangles;
		poly.CalcOrientation();
		poly.Triangulate( aiTriangles );

		TqUint ctris = aiTriangles.size() / 3;
		// Build an array of point counts (always 3 each).
		std::vector<RtInt> _nverts;
		_nverts.resize( ctris, 3 );

		RiPointsPolygonsV( ctris, &_nverts[ 0 ], &aiTriangles[ 0 ], count, tokens, values );
	}
	return ;
}

RtVoid RiBlobby( RtInt nleaf, RtInt ncodes, RtInt codes[], RtInt nfloats, RtFloat floats[],
                 RtInt nstrings, RtString strings[], ... )
{

	va_list	pArgs;
	va_start( pArgs, strings );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiBlobbyV( nleaf, ncodes, codes, nfloats, floats, nstrings, strings, count, &aTokens[0], &aValues[0] );

	return ;
}

//----------------------------------------------------------------------
/** List based version of above.
 *  
 *\return	nothing
 **/
RtVoid RiBlobbyV( RtInt nleaf, RtInt ncode, RtInt code[], RtInt nflt, RtFloat flt[],
                  RtInt nstr, RtString str[], PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiBlobby

	Validate_RiBlobby

	Debug_RiBlobby

	Aqsis::log() << warning << "RiBlobby not supported" << std::endl;

	return ;
}


//----------------------------------------------------------------------
/** Specify a small Points primitives
 *  
 *\return	nothing
 **/
RtVoid	RiPoints( RtInt nvertices, ... )
{
	va_list	pArgs;
	va_start( pArgs, nvertices );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiPointsV( nvertices, count, &aTokens[0], &aValues[0] );

	return ;
}

//----------------------------------------------------------------------
/** List based version of above.
 *  
 *\return	nothing
 **/
RtVoid	RiPointsV( RtInt npoints, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiPoints

	Validate_RiPoints

	Debug_RiPoints

	// Create a storage class for all the points.
	boost::shared_ptr<CqPolygonPoints> pPointsClass( new CqPolygonPoints( npoints, 1, npoints ) );

	// Create a new points storage class
	boost::shared_ptr<CqPoints> pSurface;

	// read in the parameter list
	if ( ProcessPrimitiveVariables( pPointsClass.get(), count, tokens, values ) )
	{
		// Transform the points into camera space for processing,
		// This needs to be done before initialising the KDTree as the tree must be formulated in 'current' (camera) space.
		pPointsClass->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld(pPointsClass->pTransform() ->Time(0)), pPointsClass->pTransform() ->Time(0) ),
		                         QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld(pPointsClass->pTransform() ->Time(0)), pPointsClass->pTransform() ->Time(0) ),
		                         QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld(pPointsClass->pTransform() ->Time(0)), pPointsClass->pTransform() ->Time(0) ) );

		pSurface = boost::shared_ptr<CqPoints>( new CqPoints( npoints, pPointsClass ) );
		// Initialise the KDTree for the points to contain all.
		pSurface->InitialiseKDTree();
		pSurface->InitialiseMaxWidth();

		if ( QGetRenderContext() ->pattrCurrent() ->GetFloatAttribute( "System", "LODBound" ) [ 1 ] < 0.0f )
		{
			// Cull this geometry for LOD reasons
			return ;
		}

		/// \note:	Have to duplicate the work of CreateGPrim here as we need a special type of CqDeformingSurface.
		///			Not happy about this, need to look at it.
		// If in a motion block, confirm that the current deformation surface can accept the passed one as a keyframe.
		if( QGetRenderContext() ->pconCurrent() ->fMotionBlock() )
		{
			CqMotionModeBlock* pMMB = static_cast<CqMotionModeBlock*>(QGetRenderContext() ->pconCurrent().get());

			boost::shared_ptr<CqDeformingSurface> pMS = pMMB->GetDeformingSurface();
			// If this is the first frame, then generate the appropriate CqDeformingSurface and fill in the first frame.
			// Then cache the pointer on the motion block.
			if( !pMS )
			{
				boost::shared_ptr<CqDeformingPointsSurface> pNewMS( new CqDeformingPointsSurface( pSurface ) );
				pNewMS->AddTimeSlot( QGetRenderContext()->Time(), pSurface );

				pMMB->SetDeformingSurface( pNewMS );
			}
			else
			{
				pMS->AddTimeSlot( QGetRenderContext()->Time(), pSurface );
			}
			QGetRenderContext() ->AdvanceTime();
		}
		else
		{
			QGetRenderContext()->StorePrimitive( pSurface );
			STATS_INC( GPR_created );
		}
	}

	return ;
}

//----------------------------------------------------------------------
/** Specify a small line primitives
 *  
 *\param	type could be "linear" "bicubic"
 *\param	ncurves : number of vertices
 *\param	nvertices: vertices index
 *\param	wrap could be "periodic" "nonperiodic"
 *\return	nothing
 **/

RtVoid RiCurves( RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap, ... )
{
	va_list	pArgs;
	va_start( pArgs, wrap );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiCurvesV( type, ncurves, nvertices, wrap, count, &aTokens[0], &aValues[0] );

	return ;
}

//----------------------------------------------------------------------
/** List based version of above.
 *  
 *\return	nothing
 **/
RtVoid RiCurvesV( RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiCurves

	Validate_RiCurves

	Debug_RiCurves

	// find out whether the curve is periodic or non-periodic
	TqBool periodic = TqFalse;
	if ( strcmp( wrap, RI_PERIODIC ) == 0 )
	{
		periodic = TqTrue;
	}
	else if ( strcmp( wrap, RI_NONPERIODIC ) == 0 )
	{
		periodic = TqFalse;
	}
	else
	{
		// the wrap mode was neither "periodic" nor "nonperiodic"
		Aqsis::log() << error << "RiCurves invalid wrap mode \"" << wrap << "\"" << std::endl;
	}

	// handle creation of linear and cubic curve groups separately
	if ( strcmp( type, RI_CUBIC ) == 0 )
	{
		// create a new group of cubic curves
		boost::shared_ptr<CqCubicCurvesGroup> pSurface( new CqCubicCurvesGroup( ncurves, nvertices, periodic ) );
		// read in the parameter list
		if ( ProcessPrimitiveVariables( pSurface.get(), count, tokens, values ) )
		{
			// set the default primitive variables
			pSurface->SetDefaultPrimitiveVariables();

			TqFloat time = QGetRenderContext()->Time();
			// Transform the points into camera space for processing,
			pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );

			std::vector<boost::shared_ptr<CqSurface> > aSplits;
			pSurface->Split( aSplits );
			std::vector<boost::shared_ptr<CqSurface> >::iterator iSS;
			for ( iSS = aSplits.begin(); iSS != aSplits.end(); ++iSS )
			{
				CreateGPrim( *iSS );
			}
		}
	}
	else if ( strcmp( type, RI_LINEAR ) == 0 )
	{
		// create a new group of linear curves
		boost::shared_ptr<CqLinearCurvesGroup> pSurface( new CqLinearCurvesGroup( ncurves, nvertices, periodic ) );

		// read in the parameter list
		if ( ProcessPrimitiveVariables( pSurface.get(), count, tokens, values ) )
		{
			// set the default primitive variables
			pSurface->SetDefaultPrimitiveVariables();
			TqFloat time = QGetRenderContext()->Time();
			// Transform the points into camera space for processing,
			pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );
			CreateGPrim( pSurface );
		}
	}
	else
	{
		// the type of curve was neither "linear" nor "cubic"
		Aqsis::log() << error << "RiCurves invalid type \"" << type << "\"" << std::endl;
	}
}


//----------------------------------------------------------------------
// RiPointsPolygons
// Specify a list of convex coplanar polygons and their shared vertices.
//
RtVoid	RiPointsPolygons( RtInt npolys, RtInt nverts[], RtInt verts[], ... )
{
	va_list	pArgs;
	va_start( pArgs, verts );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiPointsPolygonsV( npolys, nverts, verts, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiPointsPolygonsV
// List based version of above.
//


RtVoid	RiPointsPolygonsV( RtInt npolys, RtInt nverts[], RtInt verts[], PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiPointsPolygons

	Validate_RiPointsPolygons

	Debug_RiPointsPolygons

	// Calculate how many vertices there are.
	RtInt cVerts = 0;
	RtInt* pVerts = verts;
	RtInt poly;
	RtInt sumnVerts = 0;
	for ( poly = 0; poly < npolys; ++poly )
	{
		RtInt v;
		sumnVerts += nverts[ poly ];
		for ( v = 0; v < nverts[ poly ]; ++v )
		{
			cVerts = MAX( ( ( *pVerts ) + 1 ), cVerts );
			pVerts++;
		}
	}

	// Create a storage class for all the points.
	boost::shared_ptr<CqPolygonPoints> pPointsClass( new CqPolygonPoints( cVerts, npolys, sumnVerts ) );
	// Process any specified primitive variables
	if ( ProcessPrimitiveVariables( pPointsClass.get(), count, tokens, values ) )
	{
		boost::shared_ptr<CqSurfacePointsPolygons> pPsPs( new CqSurfacePointsPolygons(pPointsClass, npolys, nverts, verts ) );
		TqFloat time = QGetRenderContext()->Time();
		// Transform the points into camera space for processing,
		pPointsClass->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld(time), time ),
		                         QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld(time), time ),
		                         QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld(time), time ) );
		CreateGPrim(pPsPs);
	}

	return ;
}


//----------------------------------------------------------------------
// RiPointsGeneralPolygons
// Specify a list of coplanar, non-convex polygons and their shared vertices.
//
RtVoid	RiPointsGeneralPolygons( RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], ... )
{
	va_list	pArgs;
	va_start( pArgs, verts );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiPointsGeneralPolygonsV( npolys, nloops, nverts, verts, count, &aTokens[0], &aValues[0] );

	return ;
}


//----------------------------------------------------------------------
// RiPointsGeneralPolygonsV
// List based version of above.
//
RtVoid	RiPointsGeneralPolygonsV( RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiPointsGeneralPolygons

	Validate_RiPointsGeneralPolygons

	Debug_RiPointsGeneralPolygons

	TqUint ipoly;
	TqUint iloop;
	TqUint igloop = 0;
	TqInt cVerts = 0;
	TqUint igvert = 0;
	TqInt initial_index;
	TqInt sumnVerts = 0;

	// Calculate how many points overall.
	RtInt* pVerts = verts;
	for ( ipoly = 0; ipoly < (TqUint) npolys; ++ipoly )
	{
		for ( iloop = 0; iloop < (TqUint) nloops[ ipoly ]; ++iloop, ++igloop )
		{
			TqInt v;
			sumnVerts += nverts[ igloop ];
			// Check for degenerate loops.
			if( nverts[ igloop ] < 3 )
			{
				CqString objname( "unnamed" );
				const CqString* pattrName = QGetRenderContext()->pattrCurrent()->GetStringAttribute( "identifier", "name" );
				if ( pattrName != 0 )
					objname = pattrName[ 0 ];
				Aqsis::log() << warning << "Degenerate loop in PointsGeneralPolygons object \"" << objname.c_str() << "\"" << std::endl;
			}
			for ( v = 0; v < nverts[ igloop ]; ++v )
			{
				cVerts = MAX( ( ( *pVerts ) + 1 ), cVerts );
				pVerts++;
			}
		}
	}

	// We need to take into account Orientation here.
	TqBool O = QGetRenderContext()->pattrCurrent() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ] != 0;

	// Create a storage class for all the points.
	boost::shared_ptr<CqPolygonPoints> pPointsClass( new CqPolygonPoints( cVerts, npolys, sumnVerts ) );
	// Process any specified primitive variables
	if ( ProcessPrimitiveVariables( pPointsClass.get(), count, tokens, values ) )
	{
		pPointsClass->SetDefaultPrimitiveVariables( RI_FALSE );

		// Reset loop counter.
		igloop = 0;
		TqUint ctris = 0;
		std::vector<TqInt>	aiTriangles;
		std::vector<TqInt> aFVList;
		std::vector<TqInt> aUVList;

		for ( ipoly = 0; ipoly < (TqUint) npolys; ++ipoly )
		{
			initial_index = igvert;
			// Create a general 2D polygon using the points in each loop.
			CqPolygonGeneral2D poly;
			TqUint ipoint = 0;
			TqUint imaxindex, iminindex;
			imaxindex = cVerts;
			iminindex = 0;
			for ( iloop = 0; iloop < (TqUint) nloops[ ipoly ]; ++iloop, ++igloop )
			{
				iminindex = MIN( iminindex, (TqUint) verts[ igvert ] );
				imaxindex = MAX( imaxindex, (TqUint) verts[ igvert ] );
				TqFloat	MinX, MaxX;
				TqFloat	MinY, MaxY;
				TqFloat	MinZ, MaxZ;
				CqVector3D	vecTemp = pPointsClass->P()->pValue( verts[ igvert ] )[0];
				MinX = MaxX = vecTemp.x();
				MinY = MaxY = vecTemp.y();
				MinZ = MaxZ = vecTemp.z();

				CqPolygonGeneral2D polya;
				polya.SetpVertices( pPointsClass );
				TqInt ivert;
				for ( ivert = 0; ivert < nverts[ igloop ]; ++ivert, ++igvert )
				{
					ipoint = verts[ igvert ];
					assert( ipoint < pPointsClass->P() ->Size() );
					polya.aiVertices().push_back( ipoint );

					vecTemp = pPointsClass->P()->pValue( verts[ igvert ] )[0];
					MinX = ( MinX < vecTemp.x() ) ? MinX : vecTemp.x();
					MinY = ( MinY < vecTemp.y() ) ? MinY : vecTemp.y();
					MinZ = ( MinZ < vecTemp.z() ) ? MinZ : vecTemp.z();
					MaxX = ( MaxX > vecTemp.x() ) ? MaxX : vecTemp.x();
					MaxY = ( MaxY > vecTemp.y() ) ? MaxY : vecTemp.y();
					MaxZ = ( MaxZ > vecTemp.z() ) ? MaxZ : vecTemp.z();
				}

				// Work out which plane to project to.
				TqFloat	DiffX = MaxX - MinX;
				TqFloat	DiffY = MaxY - MinY;
				TqFloat	DiffZ = MaxZ - MinZ;

				TqInt Axis;
				if ( DiffX < DiffY && DiffX < DiffZ )
					Axis = CqPolygonGeneral2D::Axis_YZ;
				else if ( DiffY < DiffX && DiffY < DiffZ )
					Axis = CqPolygonGeneral2D::Axis_XZ;
				else
					Axis = CqPolygonGeneral2D::Axis_XY;
				polya.SetAxis( Axis );

				if ( iloop == 0 )
				{
					/// \note: We need to check here if the orientation of the projected poly matches the
					/// expected one, of not, we must swap the direction so that the triangulation routines can
					/// correctly determine the inside/outside nature of points. However, if doing so breaks the
					/// orientation as expected by the rest of the renderer, we need to flip the orientation
					/// attribute as well so that normals are correctly calculated.
					if( !O )
					{
						if ( polya.CalcOrientation() != CqPolygonGeneral2D::Orientation_Clockwise )
							polya.SwapDirection();
					}
					else
					{
						if ( polya.CalcOrientation() != CqPolygonGeneral2D::Orientation_AntiClockwise )
							polya.SwapDirection();
					}
					poly = polya;
				}
				else
				{
					if( !O )
					{
						if ( polya.CalcOrientation() != CqPolygonGeneral2D::Orientation_AntiClockwise )
							polya.SwapDirection();
					}
					else
					{
						if ( polya.CalcOrientation() != CqPolygonGeneral2D::Orientation_Clockwise )
							polya.SwapDirection();
					}
					poly.Combine( polya );
				}
			}
			// Now triangulate the general polygon

			poly.CalcOrientation();
			TqUint iStartTri = aiTriangles.size();
			poly.Triangulate( aiTriangles );
			TqUint iEndTri = aiTriangles.size();
			// Store the facevarying information
			/// \note This code relies on the fact that vertex indices cannot be duplicated
			/// within the loops of a single poly. Make sure this is a reasonable assumption.
			for( TqUint ifv = iStartTri; ifv < iEndTri; ++ifv )
			{
				TqInt ivaryingindex = aiTriangles[ ifv ];
				TqBool found = TqFalse;
				for( TqUint iv = initial_index; iv != igvert; ++iv )
				{
					if( verts[ iv ] == ivaryingindex )
					{
						aFVList.push_back( iv );
						found = TqTrue;
					}
				}
				assert( found );
			}

			// Store the count of triangles generated for this general poly, so that we
			// can duplicate up the uniform values as appropriate.
			/// \note This code relies on the fact that vertex indices cannot be duplicated
			/// within the loops of a single poly. Make sure this is a reasonable assumption.
			aUVList.push_back( ( iEndTri - iStartTri ) / 3 );
		}

		// Build an array of point counts (always 3 each).
		ctris = aiTriangles.size() / 3;
		std::vector<RtInt> _nverts;
		_nverts.resize( ctris, 3 );

		// Rebuild any facevarying or uniform variables.
		TqInt iUserParam;
		TqInt fvcount = ctris * 3;
		assert( aFVList.size() == fvcount );
		std::vector<void*> aNewParams;
		for( iUserParam = 0; iUserParam < count; ++iUserParam )
		{
			SqParameterDeclaration Decl = QGetRenderContext()->FindParameterDecl( tokens[ iUserParam ] );
			TqInt elem_size = 0;
			switch( Decl.m_Type )
			{
				case type_float:
					elem_size = sizeof(RtFloat);
					break;
				case type_integer:
					elem_size = sizeof(RtInt);
					break;
				case type_vector:
				case type_point:
				case type_normal:
					elem_size = sizeof(RtPoint);
					break;
				case type_color:
					elem_size = sizeof(RtColor);
					break;
				case type_matrix:
					elem_size = sizeof(RtMatrix);
					break;
				default:
					break;
			}
			// Take into account array primitive variables.
			TqInt array_size = Decl.m_Count;

			if( Decl.m_Class == class_facevarying )
			{
				char* pNew = static_cast<char*>( malloc( elem_size * fvcount * array_size) );
				aNewParams.push_back( pNew );
				TqInt iElem;
				for( iElem = 0; iElem < fvcount; ++iElem )
				{
					const unsigned char* pval = static_cast<const unsigned char*>( values[ iUserParam ] ) + ( aFVList[ iElem ] * elem_size );
					memcpy( pNew, pval, ( elem_size * array_size ));
					pNew += elem_size;
				}
				values[ iUserParam ] = aNewParams.back();
			}
			else if( Decl.m_Class == class_uniform )
			{
				// Allocate enough for 1 value per triangle, then duplicate values from the original list
				// accordingly.
				char* pNew = static_cast<char*>( malloc( elem_size * ctris * array_size ) );
				aNewParams.push_back( pNew );
				TqInt iElem;
				const unsigned char* pval = static_cast<const unsigned char*>( values[ iUserParam ] );
				for( iElem = 0; iElem < npolys; ++iElem )
				{
					TqInt dup_count = aUVList[ iElem ];
					TqInt dup;
					for(dup=0; dup < dup_count; dup++)
					{
						memcpy( pNew, pval, ( elem_size * array_size ));
						pNew += elem_size;
					}
					pval += elem_size;
				}
				values[ iUserParam ] = aNewParams.back();
			}
		}

		RiPointsPolygonsV( ctris, &_nverts[ 0 ], &aiTriangles[ 0 ], count, tokens, values );

		std::vector<void*>::iterator iNewParam;
		for( iNewParam = aNewParams.begin(); iNewParam != aNewParams.end(); ++iNewParam )
			free( *iNewParam );
	}

	return ;
}


//----------------------------------------------------------------------
// RiBasis
// Specify the patch basis matrices for the u and v directions, and the knot skip values.
//
RtVoid	RiBasis( RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep )
{
	Validate_Conditional

	Cache_RiBasis

	Validate_RiBasis

	Debug_RiBasis

	CqMatrix u;
	CqMatrix v;

	// A good parser will use the Ri*Basis pointers so a quick comparison
	//   can be done.
	//if ( ubasis not same as before )
	//{
	//	// Save off the newly given basis.
	//
	//	// Calculate the (inverse Bezier Basis) * (given basis), but do
	//	//   a quick check for RiPowerBasis since that is an identity
	//	//   matrix requiring no math.
	//	if ( ubasis!=RiPowerBasis )
	//	{
	//	}
	//	else
	//	{
	//	}
	//
	// Do the above again for vbasis.
	// Save off (InvBezier * VBasis) and (Transpose(InvBezier*UBasis)).
	//}

	RtInt i;
	for ( i = 0; i < 4; ++i )
	{
		RtInt j;
		for ( j = 0; j < 4; ++j )
		{
			u.SetElement( i, j, ubasis[ i ][ j ] );
			v.SetElement( i, j, vbasis[ i ][ j ] );
		}
	}
	u.SetfIdentity( TqFalse );
	v.SetfIdentity( TqFalse );

	QGetRenderContext() ->pattrWriteCurrent() ->GetMatrixAttributeWrite( "System", "Basis" ) [ 0 ] = u;
	QGetRenderContext() ->pattrWriteCurrent() ->GetMatrixAttributeWrite( "System", "Basis" ) [ 1 ] = v;
	QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "BasisStep" ) [ 0 ] = ustep;
	QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "BasisStep" ) [ 1 ] = vstep;
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiPatch
// Specify a new patch primitive.
//
RtVoid	RiPatch( RtToken type, ... )
{
	va_list	pArgs;
	va_start( pArgs, type );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiPatchV( type, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiPatchV
// List based version of above.
//
RtVoid	RiPatchV( RtToken type, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiPatch

	Validate_RiPatch

	Debug_RiPatch

	if ( strcmp( type, RI_BICUBIC ) == 0 )
	{
		// Create a surface patch
		boost::shared_ptr<CqSurfacePatchBicubic> pSurface( new CqSurfacePatchBicubic() );
		// Fill in primitive variables specified.
		if ( ProcessPrimitiveVariables( pSurface.get(), count, tokens, values ) )
		{
			// Fill in default values for all primitive variables not explicitly specified.
			pSurface->SetDefaultPrimitiveVariables();
			CqMatrix matuBasis = pSurface->pAttributes() ->GetMatrixAttribute( "System", "Basis" ) [ 0 ];
			CqMatrix matvBasis = pSurface->pAttributes() ->GetMatrixAttribute( "System", "Basis" ) [ 1 ];
			pSurface->ConvertToBezierBasis( matuBasis, matvBasis );

			TqFloat time = QGetRenderContext()->Time();
			// Transform the points into camera space for processing,
			pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );

			CreateGPrim( pSurface );
		}
	}
	else if ( strcmp( type, RI_BILINEAR ) == 0 )
	{
		// Create a surface patch
		boost::shared_ptr<CqSurfacePatchBilinear> pSurface( new CqSurfacePatchBilinear() );
		// Fill in primitive variables specified.
		if ( ProcessPrimitiveVariables( pSurface.get(), count, tokens, values ) )
		{
			// Fill in default values for all primitive variables not explicitly specified.
			pSurface->SetDefaultPrimitiveVariables();
			TqFloat time = QGetRenderContext()->Time();
			// Transform the points into camera space for processing,
			pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );
			CreateGPrim( pSurface );
		}
	}
	else
	{
		Aqsis::log() << error << "RiPatch invalid patch type \"" << type << "\"" << std::endl;
	}

	return ;
}


//----------------------------------------------------------------------
// RiPatchMesh
// Specify a quadrilaterla mesh of patches.
//
RtVoid	RiPatchMesh( RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, ... )
{
	va_list	pArgs;
	va_start( pArgs, vwrap );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiPatchMeshV( type, nu, uwrap, nv, vwrap, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiPatchMeshV
// List based version of above.
//

RtVoid	RiPatchMeshV( RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiPatchMesh

	Validate_RiPatchMesh

	Debug_RiPatchMesh

	if( strcmp( uwrap, RI_PERIODIC ) && strcmp( uwrap, RI_NONPERIODIC ) )
		Aqsis::log() << error << "RiPatchMesh invalid u-wrap type: \"" << uwrap << "\"" << std::endl;

	if( strcmp( vwrap, RI_PERIODIC ) && strcmp( vwrap, RI_NONPERIODIC ) )
		Aqsis::log() << error << "RiPatchMesh invalid v-wrap type: \"" << vwrap << "\"" << std::endl;

	if ( strcmp( type, RI_BICUBIC ) == 0 )
	{
		// Create a surface patch
		TqBool	uPeriodic = ( strcmp( uwrap, RI_PERIODIC ) == 0 ) ? TqTrue : TqFalse;
		TqBool	vPeriodic = ( strcmp( vwrap, RI_PERIODIC ) == 0 ) ? TqTrue : TqFalse;

		boost::shared_ptr<CqSurfacePatchMeshBicubic> pSurface( new CqSurfacePatchMeshBicubic( nu, nv, uPeriodic, vPeriodic ) );
		// Fill in primitive variables specified.
		if ( ProcessPrimitiveVariables( pSurface.get(), count, tokens, values ) )
		{
			// Fill in default values for all primitive variables not explicitly specified.
			pSurface->SetDefaultPrimitiveVariables();
			std::vector<boost::shared_ptr<CqSurface> > aSplits;
			pSurface->Split( aSplits );
			std::vector<boost::shared_ptr<CqSurface> >::iterator iSS;
			for ( iSS = aSplits.begin(); iSS != aSplits.end(); ++iSS )
			{
				CqMatrix matuBasis = pSurface->pAttributes() ->GetMatrixAttribute( "System", "Basis" ) [ 0 ];
				CqMatrix matvBasis = pSurface->pAttributes() ->GetMatrixAttribute( "System", "Basis" ) [ 1 ];
				static_cast<CqSurfacePatchBicubic*>( iSS->get
				                                     () ) ->ConvertToBezierBasis( matuBasis, matvBasis );
				TqFloat time = QGetRenderContext()->Time();
				// Transform the points into camera space for processing,
				(*iSS)->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
				                   QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
				                   QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );
				CreateGPrim( *iSS );
			}
		}
	}
	else if ( strcmp( type, RI_BILINEAR ) == 0 )
	{
		// Create a surface patch
		TqBool	uPeriodic = ( strcmp( uwrap, RI_PERIODIC ) == 0 ) ? TqTrue : TqFalse;
		TqBool	vPeriodic = ( strcmp( vwrap, RI_PERIODIC ) == 0 ) ? TqTrue : TqFalse;

		boost::shared_ptr<CqSurfacePatchMeshBilinear> pSurface( new CqSurfacePatchMeshBilinear( nu, nv, uPeriodic, vPeriodic ) );
		// Fill in primitive variables specified.
		if ( ProcessPrimitiveVariables( pSurface.get(), count, tokens, values ) )
		{
			// Fill in default values for all primitive variables not explicitly specified.
			pSurface->SetDefaultPrimitiveVariables();
			TqFloat time = QGetRenderContext()->Time();
			// Transform the points into camera space for processing,
			pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );
			CreateGPrim( pSurface );
		}
	}
	else
	{
		Aqsis::log() << error << "RiPatchMesh invalid type \"" << type << "\"" << std::endl;
	}

	return ;
}


//----------------------------------------------------------------------
// RiNuPatch
// Specify a new non uniform patch.
//
RtVoid	RiNuPatch( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax,
                  RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, ... )
{
	va_list	pArgs;
	va_start( pArgs, vmax );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiNuPatchV( nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiNuPatchV
// List based version of above.
//
RtVoid	RiNuPatchV( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax,
                   RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiNuPatch

	Validate_RiNuPatch

	Debug_RiNuPatch

	// Create a NURBS patch
	boost::shared_ptr<CqSurfaceNURBS> pSurface( new CqSurfaceNURBS() );
	pSurface->SetfPatchMesh();
	pSurface->Init( uorder, vorder, nu, nv );

	pSurface->Setumin( umin );
	pSurface->Setumax( umax );
	pSurface->Setvmin( vmin );
	pSurface->Setvmax( vmax );

	// Copy the knot vectors.
	RtInt i;
	for ( i = 0; i < nu + uorder; ++i )
		pSurface->auKnots() [ i ] = uknot[ i ];
	for ( i = 0; i < nv + vorder; ++i )
		pSurface->avKnots() [ i ] = vknot[ i ];

	// Process any specified parameters
	if ( ProcessPrimitiveVariables( pSurface.get(), count, tokens, values ) )
	{
		// Set up the default primitive variables.
		pSurface->SetDefaultPrimitiveVariables();
		// Clamp the surface to ensure non-periodic.
		pSurface->Clamp();
		TqFloat time = QGetRenderContext()->Time();
		// Transform the points into camera space for processing,
		pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
		                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
		                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );
		CreateGPrim( pSurface );
	}

	return ;
}

//----------------------------------------------------------------------
// RiTrimCurve
// Specify curves which are used to trim NURBS surfaces.
//
RtVoid	RiTrimCurve( RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[] )
{
	Validate_Conditional

	Cache_RiTrimCurve

	Validate_RiTrimCurve

	Debug_RiTrimCurve

	// Clear the current loop array.
	QGetRenderContext() ->pattrWriteCurrent() ->TrimLoops().Clear();

	// Build an array of curves per specified loop.
	TqInt in = 0;
	TqInt iorder = 0;
	TqInt iknot = 0;
	TqInt ivert = 0;
	TqInt iloop;

	for ( iloop = 0; iloop < nloops; ++iloop )
	{
		CqTrimLoop Loop;
		TqInt icurve;
		for ( icurve = 0; icurve < ncurves[ iloop ]; ++icurve )
		{
			// Create a NURBS patch
			CqTrimCurve Curve;
			TqInt o = order[ iorder++ ];
			TqInt cverts = n[ in++ ];
			Curve.Init( o, cverts );

			// Copy the knot vectors.
			RtInt i;
			for ( i = 0; i < o + cverts; ++i )
				Curve.aKnots() [ i ] = knot[ iknot++ ];

			// Copy the vertices from the u,v,w arrays.
			CqVector3D vec( 0, 0, 1 );
			for ( i = 0; i < cverts; ++i )
			{
				vec.x( u[ ivert ] );
				vec.y( v[ ivert ] );
				vec.z( w[ ivert++ ] );
				Curve.CP( i ) = vec;
			}
			Loop.aCurves().push_back( Curve );
		}
		QGetRenderContext() ->pattrWriteCurrent() ->TrimLoops().aLoops().push_back( Loop );
	}
	return ;
}



//----------------------------------------------------------------------
// RiSphere
// Specify a sphere primitive.
//
RtVoid	RiSphere( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ... )
{
	va_list	pArgs;
	va_start( pArgs, thetamax );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiSphereV( radius, zmin, zmax, thetamax, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiSphereV
// List based version of above.
//
RtVoid	RiSphereV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiSphere

	Validate_RiSphere

	Debug_RiSphere

	CqLogRangeCheckCallback rc;

	rc.set( "sphere zmin" );
	CheckMinMax( zmin, MIN(-radius, radius), MAX(-radius,radius), &rc );
	rc.set( "sphere zmax" );
	CheckMinMax( zmax, MIN(-radius, radius), MAX(-radius,radius), &rc );

	// Create a sphere
	boost::shared_ptr<CqSphere> pSurface( new CqSphere( radius, zmin, zmax, 0, thetamax ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );
	CreateGPrim( pSurface );

	return ;
}


//----------------------------------------------------------------------
// RiCone
// Specify a cone primitive.
//
RtVoid	RiCone( RtFloat height, RtFloat radius, RtFloat thetamax, ... )
{
	va_list	pArgs;
	va_start( pArgs, thetamax );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiConeV( height, radius, thetamax, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiConeV
// List based version of above.
//
RtVoid	RiConeV( RtFloat height, RtFloat radius, RtFloat thetamax, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiCone

	Validate_RiCone

	Debug_RiCone

	/// \note This should be an exception and get caught further up.
	if( thetamax == 0 )
		return;

	// Create a cone
	boost::shared_ptr<CqCone> pSurface( new CqCone( height, radius, 0, thetamax, 0, 1.0f ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );
	CreateGPrim( pSurface );

	return ;
}


//----------------------------------------------------------------------
// RiCylinder
// Specify a culinder primitive.
//
RtVoid	RiCylinder( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ... )
{
	va_list	pArgs;
	va_start( pArgs, thetamax );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiCylinderV( radius, zmin, zmax, thetamax, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiCylinderV
// List based version of above.
//
RtVoid	RiCylinderV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiCylinder

	Validate_RiCylinder

	Debug_RiCylinder

	// Create a cylinder
	boost::shared_ptr<CqCylinder> pSurface( new CqCylinder( radius, zmin, zmax, 0, thetamax ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );
	CreateGPrim( pSurface );

	return ;
}


//----------------------------------------------------------------------
// RiHyperboloid
// Specify a hyperboloid primitive.
//
RtVoid	RiHyperboloid( RtPoint point1, RtPoint point2, RtFloat thetamax, ... )
{
	va_list	pArgs;
	va_start( pArgs, thetamax );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiHyperboloidV( point1, point2, thetamax, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiHyperboloidV
// List based version of above.
//
RtVoid	RiHyperboloidV( RtPoint point1, RtPoint point2, RtFloat thetamax, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiHyperboloid

	Validate_RiHyperboloid

	Debug_RiHyperboloid

	// Create a hyperboloid
	CqVector3D v0( point1[ 0 ], point1[ 1 ], point1[ 2 ] );
	CqVector3D v1( point2[ 0 ], point2[ 1 ], point2[ 2 ] );
	boost::shared_ptr<CqHyperboloid> pSurface( new CqHyperboloid( v0, v1, 0, thetamax ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );
	CreateGPrim( pSurface );

	return ;
}


//----------------------------------------------------------------------
// RiParaboloid
// Specify a paraboloid primitive.
//
RtVoid	RiParaboloid( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ... )
{
	va_list	pArgs;
	va_start( pArgs, thetamax );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiParaboloidV( rmax, zmin, zmax, thetamax, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiParaboloidV
// List based version of above.
//
RtVoid	RiParaboloidV( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiParaboloid

	Validate_RiParaboloid

	Debug_RiParaboloid

	// Create a paraboloid
	boost::shared_ptr<CqParaboloid> pSurface( new CqParaboloid( rmax, zmin, zmax, 0, thetamax ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );
	CreateGPrim( pSurface );

	return ;
}


//----------------------------------------------------------------------
// RiDisk
// Specify a disk primitive.
//
RtVoid	RiDisk( RtFloat height, RtFloat radius, RtFloat thetamax, ... )
{
	va_list	pArgs;
	va_start( pArgs, thetamax );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiDiskV( height, radius, thetamax, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiDiskV
// List based version of above.
//
RtVoid	RiDiskV( RtFloat height, RtFloat radius, RtFloat thetamax, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiDisk

	Validate_RiDisk

	Debug_RiDisk

	// Create a disk
	boost::shared_ptr<CqDisk> pSurface( new CqDisk( height, 0, radius, 0, thetamax ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );

	CreateGPrim( pSurface );

	return ;
}


//----------------------------------------------------------------------
//
//
RtVoid	RiTorus( RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, ... )
{
	va_list	pArgs;
	va_start( pArgs, thetamax );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiTorusV( majorrad, minorrad, phimin, phimax, thetamax, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiTorus
// Specify a torus primitive.
//
RtVoid	RiTorusV( RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiTorus

	Validate_RiTorus

	Debug_RiTorus

	// Create a torus
	boost::shared_ptr<CqTorus> pSurface( new CqTorus( majorrad, minorrad, phimin, phimax, 0, thetamax ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
	                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );

	CreateGPrim( pSurface );

	return ;
}


//----------------------------------------------------------------------
// RiProcedural
// Implement the procedural type primitive.
//
RtVoid	RiProcedural( RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc )
{
	Validate_Conditional

	Cache_RiProcedural

	Validate_RiProcedural

	Debug_RiProcedural

	CqBound B(bound);

	//printf("bound(%f %f %f %f %f %f)\n", bound[0], bound[1], bound[2], bound[3], bound[4], bound[5]);

	// I suspect that in order to handle the RtFreeProc correctly that we need to reference count
	// the instances of CqProcedural so that FreeProc gets called on the final Release();

	boost::shared_ptr<CqProcedural> pProc( new CqProcedural(data, B, refineproc, freeproc ) );
	TqFloat time = QGetRenderContext()->Time();
	pProc->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pProc->pTransform() ->matObjectToWorld(time), time ),
	                  QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pProc->pTransform() ->matObjectToWorld(time), time ),
	                  QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pProc->pTransform() ->matObjectToWorld(time), time ) );
	CreateGPrim( pProc );

	return ;
}



//----------------------------------------------------------------------
// RiGeometry
// Specify a special primitive.
//
RtVoid	RiGeometry( RtToken type, ... )
{
	va_list	pArgs;
	va_start( pArgs, type );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiGeometryV( type, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiGeometryV
// List based version of above.
//
RtVoid	RiGeometryV( RtToken type, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiGeometry

	Validate_RiGeometry

	Debug_RiGeometry

	if ( strcmp( type, "teapot" ) == 0 )
	{

		// Create a standard teapot
		boost::shared_ptr<CqTeapot> pSurface( new CqTeapot( true ) ); // add a bottom if true/false otherwise

		pSurface->SetSurfaceParameters( *pSurface );
		ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
		pSurface->SetDefaultPrimitiveVariables();

		// I don't use the original teapot primitives as defined by T. Burge
		// but an array of Patch Bicubic (stolen from example from Pixar) and
		// those (6 meshes) are registered as standards GPrims right here.
		// Basically I kept the bound, transform and split, dice and diceable methods
		// in teapot.cpp but I suspect they are never called since the work of
		// dicing will rely on the registered Gprimitives (see below in the for loop).
		// I suspect the 6/7 meshes are equivalent in size/definition as the T. Burge
		// definition. The 7th is the bottom part of the teapot (see teapot.cpp).

		for ( int i = 0; i < pSurface->cNbrPatchMeshBicubic; ++i )
		{
			boost::shared_ptr<CqSurface> pMesh = pSurface->pPatchMeshBicubic[ i ];

			TqFloat time = QGetRenderContext()->Time();
			// Transform the points into camera space for processing,
			pMesh->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                  QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
			                  QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );

			CreateGPrim( boost::static_pointer_cast<CqSurface>( pMesh ) );
		}
	}
	else if ( strcmp( type, "sphere" ) == 0 )
	{
		// Create a sphere
		boost::shared_ptr<CqSphere> pSurface( new CqSphere( 1, -1, 1, 0, 360.0 ) );
		ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
		pSurface->SetDefaultPrimitiveVariables();

		TqFloat time = QGetRenderContext()->Time();
		// Transform the points into camera space for processing,
		pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
		                     QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ),
		                     QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pSurface->pTransform() ->matObjectToWorld(time), time ) );

		CreateGPrim( pSurface );
	}
	else
	{
		Aqsis::log() << warning << "RiGeometry unrecognised type \"" << type << "\"" << std::endl;
	}

	return ;
}

//----------------------------------------------------------------------
// RiSolidBegin
// Begin the definition of a CSG object.
//
RtVoid	RiSolidBegin( RtToken type )
{
	Validate_Conditional

	Cache_RiSolidBegin

	Validate_RiSolidBegin

	Debug_RiSolidBegin

	CqString strType( type );
	QGetRenderContext() ->BeginSolidModeBlock( strType );

	return ;
}


//----------------------------------------------------------------------
// RiSolidEnd
// End the definition of a CSG object.
//
RtVoid	RiSolidEnd()
{
	Validate_Conditional

	Cache_RiSolidEnd

	Validate_RiSolidEnd

	Debug_RiSolidEnd

	QGetRenderContext() ->EndSolidModeBlock();

	return ;
}


//----------------------------------------------------------------------
// RiObjectBegin
// Begin the definition of a stored object for use by RiObjectInstance.
//
RtObjectHandle	RiObjectBegin()
{
	Validate_Conditional0

	Cache_RiObjectBegin

	Validate_RiObjectBegin

	Debug_RiObjectBegin

	QGetRenderContext() ->BeginObjectModeBlock();
	RtObjectHandle ObjectHandle = static_cast<RtObjectHandle>(QGetRenderContext() ->OpenNewObjectInstance());

	return ( ObjectHandle );
}


//----------------------------------------------------------------------
// RiObjectEnd
// End the defintion of a stored object for use by RiObjectInstance.
//
RtVoid	RiObjectEnd()
{
	Validate_Conditional

	Validate_RiObjectEnd

	Debug_RiObjectEnd

	QGetRenderContext() ->EndObjectModeBlock();
	QGetRenderContext() ->CloseObjectInstance();

	Cache_RiObjectEnd

	return ;
}


//----------------------------------------------------------------------
// RiObjectInstance
// Instantiate a copt of a pre-stored geometric object.
//
RtVoid	RiObjectInstance( RtObjectHandle handle )
{
	Validate_Conditional

	Cache_RiObjectInstance

	Validate_RiObjectInstance

	Debug_RiObjectInstance

	QGetRenderContext() ->InstantiateObject( reinterpret_cast<CqObjectInstance*>( handle ) );
	return ;
}


//----------------------------------------------------------------------
// RiMotionBegin
// Begin the definition of the motion of an object for use by motion blur.
//
RtVoid	RiMotionBegin( RtInt N, ... )
{
	va_list	pArgs;
	va_start( pArgs, N );

	RtFloat* times = new RtFloat[ N ];
	RtInt i;
	for ( i = 0; i < N; ++i )
		times[ i ] = va_arg( pArgs, double );

	RiMotionBeginV( N, times );

	delete[] ( times );
	return ;
}


//----------------------------------------------------------------------
// RiBeginMotionV
// List based version of above.
//
RtVoid	RiMotionBeginV( RtInt N, RtFloat times[] )
{
	Validate_Conditional

	Cache_RiMotionBeginV

	Validate_RiMotionBeginV

	Debug_RiMotionBeginV

	QGetRenderContext() ->BeginMotionModeBlock( N, times );

	return ;
}


//----------------------------------------------------------------------
// RiMotionEnd
// End the definition of the motion of an object.
//
RtVoid	RiMotionEnd()
{
	Validate_Conditional

	Cache_RiMotionEnd

	Validate_RiMotionEnd

	Debug_RiMotionEnd

	QGetRenderContext() ->EndMotionModeBlock();

	return ;
}


//----------------------------------------------------------------------
// RiMakeTexture
// Convert a picture to a texture.
//
RtVoid RiMakeTexture ( RtString pic, RtString tex, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... )
{
	va_list	pArgs;
	va_start( pArgs, twidth );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiMakeTextureV( pic, tex, swrap, twrap, filterfunc, swidth, twidth, count, &aTokens[0], &aValues[0] );

}


//----------------------------------------------------------------------
// RiMakeTextureV
// List based version of above.
//
RtVoid	RiMakeTextureV( RtString imagefile, RtString texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiMakeTexture

	Validate_RiMakeTexture

	Debug_RiMakeTexture

	char modes[ 1024 ];
	assert( imagefile != 0 && texturefile != 0 && swrap != 0 && twrap != 0 && filterfunc != 0 );

	TIME_SCOPE("Texture")
	// Get the wrap modes first.
	enum EqWrapMode smode = WrapMode_Black;
	if ( strcmp( swrap, RI_PERIODIC ) == 0 )
		smode = WrapMode_Periodic;
	else if ( strcmp( swrap, RI_CLAMP ) == 0 )
		smode = WrapMode_Clamp;
	else if ( strcmp( swrap, RI_BLACK ) == 0 )
		smode = WrapMode_Black;

	enum EqWrapMode tmode = WrapMode_Black;
	if ( strcmp( twrap, RI_PERIODIC ) == 0 )
		tmode = WrapMode_Periodic;
	else if ( strcmp( twrap, RI_CLAMP ) == 0 )
		tmode = WrapMode_Clamp;
	else if ( strcmp( twrap, RI_BLACK ) == 0 )
		tmode = WrapMode_Black;


	sprintf( modes, "%s %s %s %f %f", swrap, twrap, "box", swidth, twidth );
	if ( filterfunc == RiGaussianFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "gaussian", swidth, twidth );
	if ( filterfunc == RiMitchellFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "mitchell", swidth, twidth );
	if ( filterfunc == RiBoxFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "box", swidth, twidth );
	if ( filterfunc == RiTriangleFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "triangle", swidth, twidth );
	if ( filterfunc == RiCatmullRomFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "catmull-rom", swidth, twidth );
	if ( filterfunc == RiSincFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "sinc", swidth, twidth );
	if ( filterfunc == RiDiskFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "disk", swidth, twidth );
	if ( filterfunc == RiBesselFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "bessel", swidth, twidth );


	// Now load the original image.
	CqTextureMap Source( imagefile );
	Source.Open();
	TqInt comp, qual;
	ProcessCompression( &comp, &qual, count, tokens, values );
	Source.SetCompression( comp );
	Source.SetQuality( qual );

	if ( Source.IsValid() && Source.Format() == TexFormat_Plain )
	{
		// Hopefully CqTextureMap will take care of closing the tiff file after
		// it has SAT mapped it so we can overwrite if needs be.
		// Create a new image.
		Source.Interpreted( modes );
		Source.CreateMIPMAP();
		TIFF* ptex = TIFFOpen( texturefile, "w" );

		TIFFCreateDirectory( ptex );
		TIFFSetField( ptex, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
		TIFFSetField( ptex, TIFFTAG_PIXAR_TEXTUREFORMAT, MIPMAP_HEADER );
		TIFFSetField( ptex, TIFFTAG_PIXAR_WRAPMODES, modes );
		TIFFSetField( ptex, TIFFTAG_COMPRESSION, Source.Compression() ); /* COMPRESSION_DEFLATE */
		int log2 = MIN( Source.XRes(), Source.YRes() );
		log2 = ( int ) ( log( static_cast<float>(log2) ) / log( 2.0 ) );


		for ( int i = 0; i < log2; ++i )
		{
			// Write the floating point image to the directory.
			CqTextureMapBuffer* pBuffer = Source.GetBuffer( 0, 0, i );
			if ( !pBuffer )
				break;
			Source.WriteTileImage( ptex, pBuffer, 64, 64, Source.Compression(), Source.Quality() );
		}
		TIFFClose( ptex );
	}

	Source.Close();
}


//----------------------------------------------------------------------
// RiMakeBump
// Convert a picture to a bump map.
//
RtVoid	RiMakeBump( RtString imagefile, RtString bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... )
{
	Aqsis::log() << warning << "RiMakeBump not supported" << std::endl;
	return ;
}


//----------------------------------------------------------------------
// RiMakeBumpV
// List based version of above.
//
RtVoid	RiMakeBumpV( RtString imagefile, RtString bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiMakeBump

	Validate_RiMakeBump

	Debug_RiMakeBump

	Aqsis::log() << warning << "RiMakeBump not supported" << std::endl;
	return ;
}


//----------------------------------------------------------------------
// RiMakeLatLongEnvironment
// Convert a picture to an environment map.
//
RtVoid	RiMakeLatLongEnvironment( RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... )
{
	va_list	pArgs;
	va_start( pArgs, twidth );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiMakeLatLongEnvironmentV( imagefile, reflfile, filterfunc, swidth, twidth, count, &aTokens[0], &aValues[0] );
	return ;
}


//----------------------------------------------------------------------
// RiMakeLatLongEnvironmentV
// List based version of above.
//
RtVoid	RiMakeLatLongEnvironmentV( RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiMakeLatLongEnvironment

	Validate_RiMakeLatLongEnvironment

	Debug_RiMakeLatLongEnvironment

	char modes[ 1024 ];
	char *swrap = "periodic";
	char *twrap = "clamp";

	assert( imagefile != 0 && reflfile != 0 && swrap != 0 && twrap != 0 && filterfunc != 0 );

	TIME_SCOPE("Environment Mapping")

	sprintf( modes, "%s %s %s %f %f", swrap, twrap, "box", swidth, twidth );
	if ( filterfunc == RiGaussianFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "gaussian", swidth, twidth );
	if ( filterfunc == RiMitchellFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "mitchell", swidth, twidth );
	if ( filterfunc == RiBoxFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "box", swidth, twidth );
	if ( filterfunc == RiTriangleFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "triangle", swidth, twidth );
	if ( filterfunc == RiCatmullRomFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "catmull-rom", swidth, twidth );
	if ( filterfunc == RiSincFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "sinc", swidth, twidth );
	if ( filterfunc == RiDiskFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "disk", swidth, twidth );
	if ( filterfunc == RiBesselFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "bessel", swidth, twidth );


	// Now load the original image.
	CqTextureMap Source( imagefile );
	Source.Open();
	TqInt comp, qual;
	ProcessCompression( &comp, &qual, count, tokens, values );
	Source.SetCompression( comp );
	Source.SetQuality( qual );

	if ( Source.IsValid() && Source.Format() == TexFormat_Plain )
	{
		// Hopefully CqTextureMap will take care of closing the tiff file after
		// it has SAT mapped it so we can overwrite if needs be.
		// Create a new image.
		Source.Interpreted( modes );
		Source.CreateMIPMAP();
		TIFF* ptex = TIFFOpen( reflfile, "w" );

		TIFFCreateDirectory( ptex );
		TIFFSetField( ptex, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
		TIFFSetField( ptex, TIFFTAG_PIXAR_TEXTUREFORMAT, LATLONG_HEADER );
		TIFFSetField( ptex, TIFFTAG_PIXAR_WRAPMODES, modes );
		TIFFSetField( ptex, TIFFTAG_SAMPLESPERPIXEL, Source.SamplesPerPixel() );
		TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 8 );
		TIFFSetField( ptex, TIFFTAG_COMPRESSION, Source.Compression() ); /* COMPRESSION_DEFLATE */
		int log2 = MIN( Source.XRes(), Source.YRes() );
		log2 = ( int ) ( log( static_cast<float>(log2) ) / log( 2.0 ) );


		for ( int i = 0; i < log2; ++i )
		{
			// Write the floating point image to the directory.
			CqTextureMapBuffer* pBuffer = Source.GetBuffer( 0, 0, i );
			if ( !pBuffer )
				break;
			Source.WriteTileImage( ptex, pBuffer, 64, 64, Source.Compression(), Source.Quality() );
		}
		TIFFClose( ptex );
	}

	Source.Close();
	return ;
}


//----------------------------------------------------------------------
// RiMakeCubeFaceEnvironment
// Convert a picture to a cubical environment map.
//
RtVoid	RiMakeCubeFaceEnvironment( RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... )
{
	va_list	pArgs;
	va_start( pArgs, twidth );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiMakeCubeFaceEnvironmentV( px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiMakeCubeFaceEnvironment
// List based version of above.
//
RtVoid	RiMakeCubeFaceEnvironmentV( RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiMakeCubeFaceEnvironment

	Validate_RiMakeCubeFaceEnvironment

	Debug_RiMakeCubeFaceEnvironment

	TIME_SCOPE("Environment Mapping")
	assert( px != 0 && nx != 0 && py != 0 && ny != 0 && pz != 0 && nz != 0 &&
	        reflfile != 0 && filterfunc != 0 );

	// Now load the original image.
	CqTextureMap tpx( px );
	CqTextureMap tnx( nx );
	CqTextureMap tpy( py );
	CqTextureMap tny( ny );
	CqTextureMap tpz( pz );
	CqTextureMap tnz( nz );

	tpx.Open();
	tnx.Open();
	tpy.Open();
	tny.Open();
	tpz.Open();
	tnz.Open();

	if ( tpx.Format() != TexFormat_MIPMAP )
		tpx.CreateMIPMAP();
	if ( tnx.Format() != TexFormat_MIPMAP )
		tnx.CreateMIPMAP();
	if ( tpy.Format() != TexFormat_MIPMAP )
		tpy.CreateMIPMAP();
	if ( tny.Format() != TexFormat_MIPMAP )
		tny.CreateMIPMAP();
	if ( tpz.Format() != TexFormat_MIPMAP )
		tpz.CreateMIPMAP();
	if ( tnz.Format() != TexFormat_MIPMAP )
		tnz.CreateMIPMAP();
	if ( tpx.IsValid() && tnx.IsValid() && tpy.IsValid() && tny.IsValid() && tpz.IsValid() && tnz.IsValid() )
	{
		// Check all the same size;
		bool fValid = false;
		if ( tpx.XRes() == tnx.XRes() && tpx.XRes() == tpy.XRes() && tpx.XRes() == tny.XRes() && tpx.XRes() == tpz.XRes() && tpx.XRes() == tnz.XRes() &&
		        tpx.XRes() == tnx.XRes() && tpx.XRes() == tpy.XRes() && tpx.XRes() == tny.XRes() && tpx.XRes() == tpz.XRes() && tpx.XRes() == tnz.XRes() )
			fValid = true;

		if ( !fValid )
		{
			Aqsis::log() << error << "RiMakeCubeFaceEnvironment all images must be the same size" << std::endl;
			return ;
		}

		// Now copy the images to the big map.
		CqTextureMap* Images[ 6 ] =
		    {
		        &tpx,
		        &tpy,
		        &tpz,
		        &tnx,
		        &tny,
		        &tnz
		    };

		// Create a new image.
		TIFF* ptex = TIFFOpen( reflfile, "w" );

		RtInt ii;
		TqInt xRes = tpx.XRes();
		TqInt yRes = tpx.YRes();

		TqInt numsamples = tpx.SamplesPerPixel();
		// Number of mip map levels.
		int log2 = MIN( xRes, yRes );
		log2 = ( int ) ( log( static_cast<float>(log2) ) / log( 2.0 ) );

		for ( ii = 0; ii < log2; ++ii )
		{
			CqTextureMapBuffer* pLevelBuffer = tpx.CreateBuffer( 0, 0, xRes * 3, yRes * 2, numsamples );
			TqInt view;
			for ( view = 0; view < 6; ++view )
			{
				// Get the buffer for the approriate cube side at this level.
				CqTextureMapBuffer* pBuffer = Images[ view ] ->GetBuffer( 0, 0, ii );
				// Work out where in the combined image it goes.
				TqInt xoff = view % 3;
				xoff *= xRes;
				TqInt yoff = view / 3;
				yoff *= yRes;
				TqInt line, col, sample;
				for ( line = 0; line < yRes; ++line )
				{
					for ( col = 0; col < xRes; ++col )
					{
						for ( sample = 0; sample < numsamples; ++sample )
							pLevelBuffer->SetValue( col + xoff, line + yoff, sample, pBuffer->GetValue( col, line, sample ) );
					}
				}
			}

			TIFFCreateDirectory( ptex );
			TIFFSetField( ptex, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
			TIFFSetField( ptex, TIFFTAG_PIXAR_TEXTUREFORMAT, CUBEENVMAP_HEADER );
			TIFFSetField( ptex, TIFFTAG_PIXAR_FOVCOT, 1.0/tan(RAD(fov)/2.0) );
			tpx.WriteTileImage( ptex, pLevelBuffer, 64, 64, tpx.Compression(), tpx.Quality() );
			xRes /= 2;
			yRes /= 2;
		}
		TIFFClose( ptex );
	}
	return ;
}


//----------------------------------------------------------------------
// RiMakeShadow
// Convert a depth map file to a shadow map.
//
RtVoid	RiMakeShadow( RtString picfile, RtString shadowfile, ... )
{
	va_list	pArgs;
	va_start( pArgs, shadowfile );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiMakeShadowV( picfile, shadowfile, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiMakeShadowV
// List based version of above.
//
RtVoid	RiMakeShadowV( RtString picfile, RtString shadowfile, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiMakeShadow

	Validate_RiMakeShadow

	Debug_RiMakeShadow

	TIME_SCOPE("Shadow Mapping")
	CqShadowMap ZFile( picfile );
	ZFile.LoadZFile();

	TqInt comp, qual;
	ProcessCompression( &comp, &qual, count, tokens, values );
	ZFile.SetCompression( comp );
	ZFile.SetQuality( qual );

	ZFile.SaveShadowMap( shadowfile );
	return ;
}


//----------------------------------------------------------------------
// RiMakeOcclusion
// Convert a series of depth maps to an occlusion map.
//
RtVoid	RiMakeOcclusion( RtInt npics, RtString picfiles[], RtString shadowfile, ... )
{
	va_list	pArgs;
	va_start( pArgs, shadowfile );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiMakeOcclusionV( npics, picfiles, shadowfile, count, &aTokens[0], &aValues[0] );
}


//----------------------------------------------------------------------
// RiMakeOcclusionV
// List based version of above.
//
RtVoid	RiMakeOcclusionV( RtInt npics, RtString picfiles[], RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[] )
{
	Validate_Conditional

	Cache_RiMakeOcclusion

	Validate_RiMakeOcclusion

	Debug_RiMakeOcclusion

	TIME_SCOPE("Shadow Mapping")

	RtInt index;
        unlink(shadowfile);
	for( index = 0; index < npics; ++index )
	{
		CqShadowMap ZFile( picfiles[index] );
		ZFile.LoadZFile();

		TqInt comp, qual;
		ProcessCompression( &comp, &qual, count, tokens, values );
		ZFile.SetCompression( comp );
		ZFile.SetQuality( qual );

		ZFile.SaveShadowMap( shadowfile, TqTrue );
	}
	return ;
}

//----------------------------------------------------------------------
/** Conditional handlers for 3.4 new RI Tokens
 * It calls TestCondition(); expect to modify the global variable IfOk.
 *	\param	condition	
 */
RtVoid	RiIfBegin( RtString condition )
{
	IfOk = TestCondition(condition, "RiIfBegin");
}

RtVoid	RiElseIf( RtString condition )
{
	IfOk = TestCondition(condition, "RiElseIf");
}

RtVoid	RiElse( )
{
	IfOk = !IfOk;
}

RtVoid	RiIfEnd( )
{
	IfOk = TqTrue;
}

//----------------------------------------------------------------------
// RiErrorHandler
// Set the function used to report errors.
//
RtVoid	RiErrorHandler( RtErrorFunc handler )
{
	Validate_Conditional

	Cache_RiErrorHandler

	Validate_RiErrorHandler

	Debug_RiErrorHandler

	QGetRenderContext()->SetpErrorHandler( handler );
	return ;
}


//----------------------------------------------------------------------
// RiErrorIgnore
// Function used by RiErrorHandler to continue after errors.
//
RtVoid	RiErrorIgnore( RtInt code, RtInt severity, RtString message )
{
	return ;
}


//----------------------------------------------------------------------
// RiErrorPrint
// Function used by RiErrorHandler to print an error message to stdout and continue.
//
RtVoid	RiErrorPrint( RtInt code, RtInt severity, RtString message )
{
	// Don't use this!
	Aqsis::log() << error << "RiError: " << code << " : " << severity << " : " << message << std::endl;
	return ;
}


//----------------------------------------------------------------------
// RiErrorAbort
// Function used by RiErrorHandler to print and error and stop.
//
RtVoid	RiErrorAbort( RtInt code, RtInt severity, RtString message )
{
	return ;
}


//----------------------------------------------------------------------
// RiSubdivisionMesh
// Specify a subdivision surface hull with tagging.
//
RtVoid	RiSubdivisionMesh( RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], ... )
{
	va_list	pArgs;
	va_start( pArgs, floatargs );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiSubdivisionMeshV( scheme, nfaces, nvertices, vertices, ntags, tags, nargs, intargs, floatargs, count, &aTokens[0], &aValues[0] );
}

//----------------------------------------------------------------------
// RiSubdivisionMeshV
// List based version of above.
//
RtVoid	RiSubdivisionMeshV( RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiSubdivisionMesh

	Validate_RiSubdivisionMesh

	Debug_RiSubdivisionMesh

	// Calculate how many vertices there are.
	RtInt cVerts = 0;
	RtInt* pVerts = vertices;
	RtInt face;
	RtInt sumnVerts = 0;
	for ( face = 0; face < nfaces; ++face )
	{
		RtInt v;
		sumnVerts += nvertices[ face ];
		for ( v = 0; v < nvertices[ face ]; ++v )
		{
			cVerts = MAX( ( ( *pVerts ) + 1 ), cVerts );
			pVerts++;
		}
	}

	// Create a storage class for all the points.
	boost::shared_ptr<CqPolygonPoints> pPointsClass( new CqPolygonPoints( cVerts, nfaces, sumnVerts ) );

	std::vector<boost::shared_ptr<CqPolygonPoints> >	apPoints;
	// Process any specified primitive variables
	if ( ProcessPrimitiveVariables( pPointsClass.get(), count, tokens, values ) )
	{
		// Create experimental version
		if ( strcmp( scheme, "catmull-clark" ) == 0 )
		{
			// Transform the points into camera space for processing,
			pPointsClass->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "world", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld(pPointsClass->pTransform() ->Time(0)), pPointsClass->pTransform() ->Time(0) ),
			                         QGetRenderContext() ->matNSpaceToSpace( "object", "world", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld(pPointsClass->pTransform() ->Time(0)), pPointsClass->pTransform() ->Time(0) ),
			                         QGetRenderContext() ->matVSpaceToSpace( "object", "world", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld(pPointsClass->pTransform() ->Time(0)), pPointsClass->pTransform() ->Time(0) ) );

			boost::shared_ptr<CqSubdivision2> pSubd2( new CqSubdivision2( pPointsClass ) );
			pSubd2->Prepare( cVerts );

			boost::shared_ptr<CqSurfaceSubdivisionMesh> pMesh( new CqSurfaceSubdivisionMesh(pSubd2, nfaces ) );

			RtInt	iP = 0;
			for ( face = 0; face < nfaces; ++face )
			{
				pSubd2->AddFacet( nvertices[ face ], &vertices[ iP ], iP );
				iP += nvertices[ face ];
			}
			if ( pSubd2->Finalise() )
			{
				// Process tags.
				TqInt argcIndex = 0;
				TqInt floatargIndex = 0;
				TqInt intargIndex = 0;
				for ( TqInt i = 0; i < ntags; ++i )
				{
					if ( strcmp( tags[ i ], "interpolateboundary" ) == 0 )
						pSubd2->SetInterpolateBoundary( TqTrue );
					else if ( strcmp( tags [ i ], "crease" ) == 0 )
					{
						TqFloat creaseSharpness = floatargs[ floatargIndex ];
						// convert pixars 0->infinity crease values to our 0->1
						if( creaseSharpness > 5.0f )
							creaseSharpness = 5.0f;
						creaseSharpness /= 5.0f;
						// bend the curve so values behave more like pixars algorithm
						creaseSharpness = pow(creaseSharpness, 0.2f);
						TqInt iEdge = 0;
						while ( iEdge < nargs[ argcIndex ] - 1 )
						{
							if ( intargs[ iEdge + intargIndex ] < pSubd2->cVertices() &&
							        intargs[ iEdge + intargIndex + 1 ] < pSubd2->cVertices() )
							{
								// Store the sharp edge information in the top level mesh.
								pMesh->AddSharpEdge(intargs[ iEdge + intargIndex ], intargs[ iEdge + intargIndex + 1 ], creaseSharpness);
								// Store the crease sharpness.
								CqLath* pEdge = pSubd2->pVertex( intargs[ iEdge + intargIndex ] );
								std::vector<CqLath*> aQve;
								pEdge->Qve( aQve );
								std::vector<CqLath*>::iterator iOpp;
								for( iOpp = aQve.begin(); iOpp != aQve.end(); ++iOpp )
								{
									if( ( NULL != (*iOpp)->ec() ) && (*iOpp)->ec()->VertexIndex() == intargs[ iEdge + intargIndex + 1 ] )
									{
										pSubd2->AddSharpEdge( (*iOpp), creaseSharpness );
										pSubd2->AddSharpEdge( (*iOpp)->ec(), creaseSharpness );
										break;
									}
								}
							}
							iEdge++;
						}
					}
					else if ( strcmp( tags [ i ], "corner" ) == 0 )
					{
						TqInt iVertex = 0;
						while ( iVertex < nargs[ argcIndex ] )
						{
							if ( intargs[ iVertex + intargIndex ] < pSubd2->cVertices() )
							{
								// Store the sharp edge information in the top level mesh.
								pMesh->AddSharpCorner(intargs[ iVertex + intargIndex ], RI_INFINITY);
								// Store the corner sharpness.
								CqLath* pVertex = pSubd2->pVertex( intargs[ iVertex + intargIndex ] );
								pSubd2->AddSharpCorner( pVertex, RI_INFINITY );
							}
							iVertex++;
						}
					}
					else if ( strcmp( tags [ i ], "hole" ) == 0 )
					{
						TqInt iFace = 0;
						while ( iFace < nargs[ argcIndex ] )
						{
							pSubd2->SetHoleFace( intargs[ iFace + intargIndex ] );
							iFace++;
						}
					}

					intargIndex += nargs[ argcIndex++ ];
					floatargIndex += nargs[ argcIndex++ ];
				}

				CreateGPrim(pMesh);
			}
			else
			{
				Aqsis::log() << error << "RiSubdivisionMesh contains non-manifold data" << std::endl;
			}
		}
		else
		{
			Aqsis::log() << error << "RiSubdivisionMesh invalid scheme \"" << scheme << "\"" << std::endl;
		}
	}

	return ;
}


RtVoid RiReadArchive( RtToken name, RtArchiveCallback callback, ... )
{
	va_list	pArgs;
	va_start( pArgs, callback );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiReadArchiveV( name, callback, count, &aTokens[0], &aValues[0] );
}


RtVoid	RiReadArchiveV( RtToken name, RtArchiveCallback callback, PARAMETERLIST )
{
	Validate_Conditional

	Cache_RiReadArchive

	Validate_RiReadArchive

	Debug_RiReadArchive

	CqRiFile	fileArchive( name, "archive" );

	if ( fileArchive.IsValid() )
	{
		CqString strRealName( fileArchive.strRealName() );
		fileArchive.Close();
		FILE *file;
		if ( ( file = fopen( strRealName.c_str(), "rb" ) ) != NULL )
		{
#ifdef REQUIRED
			Aqsis::log() << info << "RiReadArchive: Reading archive \"" << strRealName.c_str() << "\"" << std::endl;
#endif

			CqRIBParserState currstate = librib::GetParserState();
			if (currstate.m_pParseCallbackInterface == NULL)
				currstate.m_pParseCallbackInterface = new librib2ri::Engine;
			librib::Parse( file, name, *(currstate.m_pParseCallbackInterface), *(currstate.m_pParseErrorStream), callback );
			librib::SetParserState( currstate );
			fclose(file);
		}
	}
}


RtVoid	RiArchiveRecord( RtToken type, char * format, ... )
{
	Validate_RiArchiveRecord

	Debug_RiArchiveRecord
}

RtContextHandle	RiGetContext( void )
{
	Validate_RiGetContext

	Debug_RiGetContext

	return( NULL );
}

RtVoid	RiContext( RtContextHandle handle )
{
	Validate_RiContext

	Debug_RiContext
}

RtVoid	RiClippingPlane( RtFloat x, RtFloat y, RtFloat z, RtFloat nx, RtFloat ny, RtFloat nz )
{
	Validate_RiClippingPlane

	Debug_RiClippingPlane
}


RtVoid RiShaderLayer( RtToken type, RtToken name, RtToken layername, ... )
{
	va_list	pArgs;
	va_start( pArgs, layername );

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	RtInt count = BuildParameterList( pArgs, aTokens, aValues );

	RiShaderLayerV( type, name, layername, count, &aTokens[0], &aValues[0] );
}

RtVoid RiShaderLayerV( RtToken type, RtToken name, RtToken layername, RtInt count, RtToken tokens[], RtPointer values[] )
{
	Validate_Conditional

	Cache_RiShaderLayer
	
	Validate_RiShaderLayer

	Debug_RiShaderLayer

	TqFloat time = QGetRenderContext()->Time();
	// If the current shader for the specified type is already a layer container, add this layer to it, if not,
	// create one and add this layer as the first.

	boost::shared_ptr<IqShader> newlayer;
	boost::shared_ptr<IqShader> layeredshader;
	CqString stringtype(type);
	stringtype = stringtype.ToLower();
	if(stringtype.compare("surface")==0)
	{
		newlayer = QGetRenderContext()->CreateShader( name, Type_Surface );
		layeredshader = QGetRenderContext()->pattrCurrent()->pshadSurface(QGetRenderContext()->Time());

		if( !layeredshader || !layeredshader->IsLayered() )
		{
			// Create a new layered shader and add this shader to it.
			layeredshader = boost::shared_ptr<IqShader>(new CqLayeredShader);
			QGetRenderContext() ->pattrWriteCurrent() ->SetpshadSurface( layeredshader, QGetRenderContext() ->Time() );
		}
	}
	else if(stringtype.compare("displacement")==0)
	{
		newlayer = QGetRenderContext()->CreateShader( name, Type_Displacement );
		layeredshader = QGetRenderContext()->pattrCurrent()->pshadDisplacement(QGetRenderContext()->Time());

		if( !layeredshader || !layeredshader->IsLayered() )
		{
			// Create a new layered shader and add this shader to it.
			layeredshader = boost::shared_ptr<IqShader>(new CqLayeredShader);
			QGetRenderContext() ->pattrWriteCurrent() ->SetpshadDisplacement( layeredshader, QGetRenderContext() ->Time() );
		}
	}
	else if(stringtype.compare("imager")==0)
	{
		QGetRenderContext() ->optCurrent().GetStringOptionWrite( "System", "Imager" ) [ 0 ] = name ;
		newlayer = QGetRenderContext()->CreateShader( name, Type_Imager );
		layeredshader = QGetRenderContext()->optCurrent().pshadImager();

		if( !layeredshader || !layeredshader->IsLayered() )
		{
			// Create a new layered shader and add this shader to it.
			layeredshader = boost::shared_ptr<IqShader>(new CqLayeredShader);
			QGetRenderContext() ->optCurrent().SetpshadImager( layeredshader );
		}
	}
	else
		Aqsis::log() << error << "Layered shaders not supported for type \"" << type << "\"" << std::endl;

	if ( newlayer && layeredshader )
	{
		newlayer->matCurrent() = QGetRenderContext() ->matCurrent(time);

		// Just add this layer in
		layeredshader->AddLayer(layername, newlayer);

		// Just check that the transformation hasn't changed between layers, as this is not handled.
		if(newlayer->matCurrent() != layeredshader->matCurrent())
			Aqsis::log() << error << "The shader space has changed between layers, this is not supported" << std::endl;

		// Execute the intiialisation code here, as we now have our shader context complete.
		newlayer->PrepareDefArgs();
		RtInt i;
		for ( i = 0; i < count; ++i )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			SetShaderArgument( newlayer, token, static_cast<TqPchar>( value ) );
		}
	}
}

RtVoid RiConnectShaderLayers( RtToken type, RtToken layer1, RtToken variable1, RtToken layer2, RtToken variable2 )
{
	Validate_Conditional

	Cache_RiConnectShaderLayers
	
	Validate_RiConnectShaderLayers

	Debug_RiConnectShaderLayers

	// If the current shader for the specified type is a layer container, add this connection to it
	CqString stringtype(type);
	stringtype = stringtype.ToLower();
	boost::shared_ptr<IqShader> pcurr;
	if(stringtype.compare("surface")==0)
		pcurr = QGetRenderContext()->pattrWriteCurrent()->pshadSurface(QGetRenderContext()->Time());
	else if(stringtype.compare("displacement")==0)
		pcurr = QGetRenderContext()->pattrWriteCurrent()->pshadDisplacement(QGetRenderContext()->Time());
	else if(stringtype.compare("imager")==0)
		pcurr = QGetRenderContext()->optCurrent().pshadImager();
	else
		Aqsis::log() << error << "Layered shaders not supported for type \"" << type << "\"" << std::endl;
	if( pcurr && pcurr->IsLayered() )
	{
		// Just add this layer in
		pcurr->AddConnection(layer1, variable1, layer2, variable2);
	}
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
// Helper functions

//----------------------------------------------------------------------
// ProcessPrimitiveVariables
// Process and fill in any primitive variables.
// return	:	RI_TRUE if position specified, RI_FALSE otherwise.

static RtBoolean ProcessPrimitiveVariables( CqSurface * pSurface, PARAMETERLIST )
{
	std::vector<TqInt>	aUserParams;

	// Read recognised parameter values.
	RtInt	fP = RIL_NONE;

	RtFloat*	pPoints = 0;

	RtInt i;
	for ( i = 0; i < count; ++i )
	{
		RtToken	token = tokens[ i ];
		RtPointer	value = values[ i ];

		SqParameterDeclaration Decl = QGetRenderContext()->FindParameterDecl( token );
		TqUlong hash = CqString::hash(Decl.m_strName.c_str());

		if ( (hash == RIH_P) && (Decl.m_Class == class_vertex ))
		{
			fP = RIL_P;
			pPoints = ( RtFloat* ) value;
		}
		else if ( (hash == RIH_PZ) && (Decl.m_Class == class_vertex ) )
		{
			fP = RIL_Pz;
			pPoints = ( RtFloat* ) value;
		}
		else if ( (hash == RIH_PW) && (Decl.m_Class == class_vertex ) )
		{
			fP = RIL_Pw;
			pPoints = ( RtFloat* ) value;
		}
		else
		{
			aUserParams.push_back( i );
		}
	}

	// Fill in the position variable according to type.
	if ( fP != RIL_NONE )
	{
		pSurface->AddPrimitiveVariable( new CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D>( "P", 1 ) );
		pSurface->P() ->SetSize( pSurface->cVertex() );
		TqUint i;
		switch ( fP )
		{
				case RIL_P:
				for ( i = 0; i < pSurface->cVertex(); ++i )
					pSurface->P()->pValue( i )[0] = CqVector3D( pPoints[ ( i * 3 ) ], pPoints[ ( i * 3 ) + 1 ], pPoints[ ( i * 3 ) + 2 ] );
				break;

				case RIL_Pz:
				for ( i = 0; i < pSurface->cVertex(); ++i )
				{
					CqVector3D vecP = pSurface->SurfaceParametersAtVertex( i );
					vecP.z( pPoints[ i ] );
					pSurface->P()->pValue( i )[0] = vecP;
				}
				break;

				case RIL_Pw:
				for ( i = 0; i < pSurface->cVertex(); ++i )
					pSurface->P()->pValue( i )[0] = CqVector4D( pPoints[ ( i * 4 ) ], pPoints[ ( i * 4 ) + 1 ], pPoints[ ( i * 4 ) + 2 ], pPoints[ ( i * 4 ) + 3 ] );
				break;
		}
	}

	// Now process any user defined paramter variables.
	if ( aUserParams.size() > 0 )
	{
		std::vector<TqInt>::iterator iUserParam;
		for ( iUserParam = aUserParams.begin(); iUserParam != aUserParams.end(); ++iUserParam )
		{
			SqParameterDeclaration Decl;
			try
			{
				Decl = QGetRenderContext()->FindParameterDecl( tokens[ *iUserParam ] );
			}
			catch( XqException e )
			{
				Aqsis::log() << error << e.strReason().c_str() << std::endl;
				continue;
			}

			CqParameter* pNewParam = ( *Decl.m_pCreate ) ( Decl.m_strName.c_str(), Decl.m_Count );
			// Now go across all values and fill in the parameter variable.
			TqInt cValues = 1;
			switch ( Decl.m_Class )
			{
				case class_uniform:
					cValues = pSurface->cUniform();
					break;

				case class_varying:
					cValues = pSurface->cVarying();
					break;

				case class_vertex:
					cValues = pSurface->cVertex();
					break;

				case class_facevarying:
					cValues = pSurface->cFaceVarying();
					break;

				default:
					break;
			}
			pNewParam->SetSize( cValues );

			TqInt i;
			switch ( Decl.m_Type )
			{
					case type_float:
					{
						CqParameterTyped<TqFloat, TqFloat>* pFloatParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pNewParam );
						TqFloat* pValue = reinterpret_cast<TqFloat*>( values[ *iUserParam ] );
						TqInt iArrayIndex, iValIndex;
						i = 0;
						for ( iValIndex = 0; iValIndex < cValues; ++iValIndex )
							for ( iArrayIndex = 0; iArrayIndex < Decl.m_Count; ++iArrayIndex, ++i )
								pFloatParam->pValue( iValIndex ) [ iArrayIndex ] = pValue[ i ];
					}
					break;

					case type_integer:
					{
						CqParameterTyped<TqInt, TqFloat>* pIntParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pNewParam );
						TqInt* pValue = reinterpret_cast<TqInt*>( values[ *iUserParam ] );
						TqInt iArrayIndex, iValIndex;
						i = 0;
						for ( iValIndex = 0; iValIndex < cValues; ++iValIndex )
							for ( iArrayIndex = 0; iArrayIndex < Decl.m_Count; ++iArrayIndex, ++i )
								pIntParam->pValue( iValIndex ) [ iArrayIndex ] = pValue[ i ];
					}
					break;

					case type_point:
					case type_normal:
					case type_vector:
					{
						CqParameterTyped<CqVector3D, CqVector3D>* pVectorParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pNewParam );
						TqFloat* pValue = reinterpret_cast<TqFloat*>( values[ *iUserParam ] );
						TqInt iArrayIndex, iValIndex;
						i = 0;
						for ( iValIndex = 0; iValIndex < cValues; ++iValIndex )
							for ( iArrayIndex = 0; iArrayIndex < Decl.m_Count; ++iArrayIndex, ++i )
								pVectorParam->pValue( iValIndex ) [ iArrayIndex ] = CqVector3D( pValue[ ( i * 3 ) ], pValue[ ( i * 3 ) + 1 ], pValue[ ( i * 3 ) + 2 ] );
					}
					break;

					case type_string:
					{
						CqParameterTyped<CqString, CqString>* pStringParam = static_cast<CqParameterTyped<CqString, CqString>*>( pNewParam );
						char** pValue = reinterpret_cast<char**>( values[ *iUserParam ] );
						TqInt iArrayIndex, iValIndex;
						i = 0;
						for ( iValIndex = 0; iValIndex < cValues; ++iValIndex )
							for ( iArrayIndex = 0; iArrayIndex < Decl.m_Count; ++iArrayIndex, ++i )
								pStringParam->pValue( iValIndex ) [ iArrayIndex ] = CqString( pValue[ i ] );
					}
					break;

					case type_color:
					{
						CqParameterTyped<CqColor, CqColor>* pColorParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pNewParam );
						TqFloat* pValue = reinterpret_cast<TqFloat*>( values[ *iUserParam ] );
						TqInt iArrayIndex, iValIndex;
						i = 0;
						for ( iValIndex = 0; iValIndex < cValues; ++iValIndex )
							for ( iArrayIndex = 0; iArrayIndex < Decl.m_Count; ++iArrayIndex, ++i )
								pColorParam->pValue( iValIndex ) [ iArrayIndex ] = CqColor( pValue[ ( i * 3 ) ], pValue[ ( i * 3 ) + 1 ], pValue[ ( i * 3 ) + 2 ] );
					}
					break;

					case type_hpoint:
					{
						CqParameterTyped<CqVector4D, CqVector3D>* pVectorParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pNewParam );
						TqFloat* pValue = reinterpret_cast<TqFloat*>( values[ *iUserParam ] );
						TqInt iArrayIndex, iValIndex;
						i = 0;
						for ( iValIndex = 0; iValIndex < cValues; ++iValIndex )
							for ( iArrayIndex = 0; iArrayIndex < Decl.m_Count; ++iArrayIndex, ++i )
								pVectorParam->pValue( iValIndex ) [ iArrayIndex ] = CqVector4D( pValue[ ( i * 4 ) ], pValue[ ( i * 4 ) + 1 ], pValue[ ( i * 4 ) + 2 ], pValue[ ( i * 4 ) + 3 ] );
					}
					break;

					case type_matrix:
					{
						CqParameterTyped<CqMatrix, CqMatrix>* pMatrixParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pNewParam );
						TqFloat* pValue = reinterpret_cast<TqFloat*>( values[ *iUserParam ] );
						TqInt iArrayIndex, iValIndex;
						i = 0;
						for ( iValIndex = 0; iValIndex < cValues; ++iValIndex )
							for ( iArrayIndex = 0; iArrayIndex < Decl.m_Count; ++iArrayIndex, ++i )
								pMatrixParam->pValue( iValIndex ) [ iArrayIndex ] = CqMatrix( pValue[ ( i * 16 ) ], pValue[ ( i * 16 ) + 1 ], pValue[ ( i * 16 ) + 2 ], pValue[ ( i * 16 ) + 3 ],
								        pValue[ ( i * 16 ) + 4 ], pValue[ ( i * 16 ) + 5 ], pValue[ ( i * 16 ) + 6 ], pValue[ ( i * 16 ) + 7 ],
								        pValue[ ( i * 16 ) + 8 ], pValue[ ( i * 16 ) + 9 ], pValue[ ( i * 16 ) + 10 ], pValue[ ( i * 16 ) + 11 ],
								        pValue[ ( i * 16 ) + 12 ], pValue[ ( i * 16 ) + 13 ], pValue[ ( i * 16 ) + 14 ], pValue[ ( i * 16 ) + 15 ]
								                                                            );
					}
					break;

					default:
					{
						// left blank to avoid compiler warnings about unhandled types
						break;
					}
			}
			pSurface->AddPrimitiveVariable( pNewParam );
		}
	}

	return ( pSurface->P() != NULL );
}


//----------------------------------------------------------------------
// CreateGPrin
// Create and register a GPrim according to the current attributes/transform
//
RtVoid	CreateGPrim( const boost::shared_ptr<CqSurface>& pSurface )
{
	// If in a motion block, confirm that the current deformation surface can accept the passed one as a keyframe.
	if( QGetRenderContext() ->pconCurrent() ->fMotionBlock() )
	{
		pSurface->PrepareTrimCurve();

		CqMotionModeBlock* pMMB = static_cast<CqMotionModeBlock*>(QGetRenderContext() ->pconCurrent().get());

		CqDeformingSurface* pMS = pMMB->GetDeformingSurface().get();
		// If this is the first frame, then generate the appropriate CqDeformingSurface and fill in the first frame.
		// Then cache the pointer on the motion block.
		if( pMS == NULL )
		{
			boost::shared_ptr<CqDeformingSurface> pNewMS( new CqDeformingSurface( pSurface ) );
			pNewMS->AddTimeSlot( QGetRenderContext()->Time(), pSurface );
			pMMB->SetDeformingSurface( pNewMS );
		}
		else
		{
			pMS->AddTimeSlot( QGetRenderContext()->Time(), pSurface );
		}
		QGetRenderContext() ->AdvanceTime();
	}
	else
	{
		pSurface->PrepareTrimCurve();
		QGetRenderContext()->StorePrimitive( pSurface );
		STATS_INC( GPR_created );

		// Add to the raytracer database also
		if(QGetRenderContext()->pRaytracer())
			QGetRenderContext()->pRaytracer()->AddPrimitive(pSurface);
	}

	return ;
}


//----------------------------------------------------------------------
/** Get the basis matrix given a standard basis name.
 * \param b Storage for basis matrix.
 * \param strName Name of basis.
 * \return Boolean indicating the basis is valid.
 */

RtBoolean	BasisFromName( RtBasis * b, const char * strName )
{
	RtBasis * pVals = 0;
	if ( !strcmp( strName, "bezier" ) )
		pVals = &RiBezierBasis;
	else if ( !strcmp( strName, "bspline" ) )
		pVals = &RiBSplineBasis;
	else if ( !strcmp( strName, "catmull-rom" ) )
		pVals = &RiCatmullRomBasis;
	else if ( !strcmp( strName, "hermite" ) )
		pVals = &RiHermiteBasis;
	else if ( !strcmp( strName, "power" ) )
		pVals = &RiPowerBasis;

	if ( pVals )
	{
		TqInt i, j;
		for ( i = 0; i < 4; ++i )
			for ( j = 0; j < 4; ++j )
				( *b ) [ i ][ j ] = ( *pVals ) [ i ][ j ];
		return ( TqTrue );
	}
	return ( TqFalse );
}


//----------------------------------------------------------------------
/** Set the function used to report progress.
 *	\param	handler	Pointer to the new function to use.
 */

RtVoid	RiProgressHandler( RtProgressFunc handler )
{
	QGetRenderContext()->SetpProgressHandler( handler );
	return ;
}


//----------------------------------------------------------------------
/** Set the function called just prior to rendering, after the world is complete.
 	\param	function	Pointer to the new function to use.
 	\return	Pointer to the old function.
 */

RtFunc	RiPreRenderFunction( RtFunc function )
{
	RtFunc pOldPreRenderFunction = QGetRenderContext()->pPreRenderFunction();
	QGetRenderContext()->SetpPreRenderFunction( function );
	return ( pOldPreRenderFunction );
}

//----------------------------------------------------------------------
/** Set the function called just prior to world definition.
 	\param	function	Pointer to the new function to use.
 	\return	Pointer to the old function.
 */

RtFunc	RiPreWorldFunction( RtFunc function )
{
	RtFunc pOldPreWorldFunction = QGetRenderContext()->pPreWorldFunction();
	QGetRenderContext()->SetpPreWorldFunction( function );
	return ( pOldPreWorldFunction );
}


void SetShaderArgument( const boost::shared_ptr<IqShader>& pShader, const char * name, TqPchar val )
{
	// Find the relevant variable.
	SqParameterDeclaration Decl;
	try
	{
		Decl = QGetRenderContext() ->FindParameterDecl( name );
	}
	catch( XqException e )
	{
		Aqsis::log() << error << e.strReason().c_str() << std::endl;
		return;
	}

	pShader->SetArgument( Decl.m_strName, Decl.m_Type, Decl.m_strSpace, val );
}


//----------------------------------------------------------------------
/** Analyze the parameter list and figure what kind of compression is required for texturemapping output files.
 
	\param	compression	compression	Pointer to an integer to containing the TIFF compression
	\param	quality it is the quality of jpeg's compression
	\param	count list counter
	\param	tokens list of tokens
	\param	values list of values
 
	\return	nothing
 */

static void ProcessCompression( TqInt * compression, TqInt * quality, TqInt count, RtToken * tokens, RtPointer * values )
{
	*compression = COMPRESSION_NONE;
	*quality = 70;

	for ( int i = 0; i < count; ++i )
	{
		RtToken	token = tokens[ i ];
		RtString *value = ( RtString * ) values[ i ];

		if ( strstr( token, "compression" ) != 0 )
		{

			if ( strstr( *value, "none" ) != 0 )
				* compression = COMPRESSION_NONE;

			else if ( strstr( *value, "lzw" ) != 0 )
				* compression = COMPRESSION_LZW;

			else if ( strstr( *value, "deflate" ) != 0 )
				* compression = COMPRESSION_DEFLATE;

			else if ( strstr( *value, "jpeg" ) != 0 )
				* compression = COMPRESSION_JPEG;

			else if ( strstr( *value, "packbits" ) != 0 )
				* compression = COMPRESSION_PACKBITS;


		}
		else if ( strstr( token, "quality" ) != 0 )
		{

			*quality = ( int ) * ( float * ) value;
			if ( *quality < 0 )
				* quality = 0;
			if ( *quality > 100 )
				* quality = 100;
		}
	}
}
