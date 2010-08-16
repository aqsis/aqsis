// Aqsis
// Copyright 1997 - 2001, Paul C. Gregory
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
		\brief Implement the majority of the RenderMan API functions.
		\author Paul C. Gregory (pgregory@aqsis.org)
		\todo: <b>Code Review</b>
*/

#include	<aqsis/aqsis.h>

#include	<fstream>
#include	<stdarg.h>
#include	<math.h>
#include	<stdio.h>
#include    <stdlib.h>

#include	<boost/filesystem/fstream.hpp>

#include	"imagebuffer.h"
#include	"lights.h"
#include	"renderer.h"
#include	"patch.h"
#include	"polygon.h"
#include	"nurbs.h"
#include	"quadrics.h"
#include	"teapot.h"
#include	"bunny.h"
#include	"shaders.h"
#include	"objectinstance.h"
#include	"trimcurve.h"
#include	"genpoly.h"
#include	"points.h"
#include	"curves.h"
#include	"procedural.h"
#include	<aqsis/util/file.h>
#include	<aqsis/util/logging.h>
#include	<aqsis/util/logging_streambufs.h>
#include	<aqsis/tex/maketexture.h>
#include	"stats.h"
#include	<aqsis/math/random.h>

#include	"subdivision2.h"
#include	"condition.h"

#include	"blobby.h"

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


#include	<aqsis/ri/ri.h>
#include    <aqsis/riutil/ri_convenience.h>

#include	<aqsis/util/sstring.h>

using namespace Aqsis;

static RtBoolean ProcessPrimitiveVariables( CqSurface* pSurface, PARAMETERLIST );

RtVoid	CreateGPrim( const boost::shared_ptr<CqSurface>& pSurface );
void SetShaderArgument( const boost::shared_ptr<IqShader>& pShader, const char* name, TqPchar val );

// TODO: Reinstate conditional testing.
bool   IfOk = true;

//---------------------------------------------------------------------
// This file contains the interface functions which are published as the
//	Renderman Interface SPECification (C) 1988 Pixar.
//

CqRandom worldrand;

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
static const TqUlong RIH_S = CqString::hash( RI_S );
static const TqUlong RIH_T = CqString::hash( RI_T );
static const TqUlong RIH_ST = CqString::hash( RI_ST );
static const TqUlong RIH_CS = CqString::hash( RI_CS );
static const TqUlong RIH_OS = CqString::hash( RI_OS );
static const TqUlong RIH_P = CqString::hash( RI_P );
static const TqUlong RIH_PZ = CqString::hash( RI_PZ );
static const TqUlong RIH_PW = CqString::hash( RI_PW );
static const TqUlong RIH_N = CqString::hash( RI_N );
static const TqUlong RIH_NP = CqString::hash( RI_NP );
static const TqUlong RIH_DEPTHFILTER = CqString::hash( "depthfilter" );
static const TqUlong RIH_JITTER = CqString::hash( "jitter" );
static const TqUlong RIH_RENDER = CqString::hash( "render" );
static const TqUlong RIH_INDIRECT = CqString::hash( "indirect" );
static const TqUlong RIH_LIGHT = CqString::hash( "light" );
static const TqUlong RIH_VISIBILITY = CqString::hash( "visibility" );

RtInt	RiLastError = RIE_NOERROR;

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
// RiDeclare
// Declare a new variable to be recognised by the system.
//
RtToken	RiDeclare( RtString name, RtString declaration )
{
	CqPrimvarToken tok;
	if(declaration)
		tok = CqPrimvarToken(declaration, name);
	else // declaration is allowed to be RI_NULL
		tok = CqPrimvarToken(class_invalid, type_invalid, 0, name);
	QGetRenderContext()->tokenDict().insert(tok);

	return ( 0 );
}


//----------------------------------------------------------------------
// SetDefaultRiOptions
// Set some Default Options.
//
void SetDefaultRiOptions()
{
	// Get the root path for the aqsis installation.
	boost::filesystem::path rootPath;
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
#elif AQSIS_SYSTEM_MACOSX

	CFURLRef pluginRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
	CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
	const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
	rootPath = pathPtr;
	CFRelease(macPath);
	CFRelease(pluginRef);
#else
	// Minty: Need to work out the executable path here.
	rootPath = AQSIS_XSTR(DEFAULT_RC_PATH);
#endif

	// Read in the system configuration file.
	boost::filesystem::path systemRcPath = rootPath / AQSIS_XSTR(AQSIS_MAIN_CONFIG_NAME);
	std::ifstream rcFile(systemRcPath.file_string().c_str(), std::ios::binary);
	if(rcFile)
	{
		Aqsis::log() << info
			<< "Reading system config \"" << systemRcPath << "\"\n";
		QGetRenderContext()->parseRibStream(rcFile, systemRcPath.file_string());
		rcFile.close();
	}
	else
	{
		Aqsis::log() << error
			<< "Could not open system config (" << systemRcPath << ")\n";
	}

	// Read in the user-specific config in $HOME/.aqsisrc 
	if(const char* homePath = getenv("HOME"))
	{
		boost::filesystem::path homeRcPath = homePath;
		homeRcPath /= ".aqsisrc";

		std::ifstream rcFile(homeRcPath.file_string().c_str(), std::ios::binary);
		if(rcFile)
		{
			Aqsis::log() << info << "Reading user config \"" << homeRcPath << "\"\n";
			QGetRenderContext()->parseRibStream(rcFile, homeRcPath.file_string());
		}
		else
		{
			boost::filesystem::path homeRcPath2 = homePath;
			homeRcPath2 /= "_aqsisrc";
		
			std::ifstream rcFile(homeRcPath2.file_string().c_str(), std::ios::binary);
			if(rcFile)
			{
				Aqsis::log() << info << "Reading user config \"" << homeRcPath2 << "\"\n";
				QGetRenderContext()->parseRibStream(rcFile, homeRcPath2.file_string());
			}
			else
			{
				Aqsis::log() << info
					<< "Could not open user config \"" << homeRcPath << "\" or \"" << homeRcPath2 << "\"\n";
			}
		}
	}
	else
	{
		Aqsis::log() << info
			<< "Environment variable HOME not set (skipping user config).\n";
	}

	// Read the config file for the current directory.
	std::string currentRcPath = ".aqsisrc";
	rcFile.open(currentRcPath.c_str(), std::ios::binary);
	if(rcFile)
	{
		QGetRenderContext()->parseRibStream(rcFile, currentRcPath);
		rcFile.close();
		Aqsis::log() << info << "Reading project config \"" << currentRcPath << "\"\n";
	}
	else
	{
		std::string currentRcPath2 = "_aqsisrc";
		rcFile.open(currentRcPath2.c_str(), std::ios::binary);

		if(rcFile)
		{
			QGetRenderContext()->parseRibStream(rcFile, currentRcPath);
			rcFile.close();
			Aqsis::log() << info << "Reading project config \"" << currentRcPath2 << "\"\n";
		}
		else
		{
			Aqsis::log() << info
				<< "Could not open project config \"" << currentRcPath << "\" or \"" << currentRcPath2 << "\"\n";
		}
	}

	// Set options from various environment variables.
	const char* popt[ 1 ];
	if(getenv("AQSIS_SHADER_PATH"))
	{
		popt[0] = getenv("AQSIS_SHADER_PATH");
		Aqsis::log() << info << "Applying AQSIS_SHADER_PATH (" << popt[0] << ")" << std::endl;
		RiOption( tokenCast("searchpath"), "shader", &popt, RI_NULL );
	}
	else
	{
		Aqsis::log() << info << "AQSIS_SHADER_PATH not set" << std::endl;
	}

	if(getenv("AQSIS_ARCHIVE_PATH"))
	{
		popt[0] = getenv("AQSIS_ARCHIVE_PATH");
		Aqsis::log() << info << "Applying AQSIS_ARCHIVE_PATH (" << popt[0] << ")" << std::endl;
		RiOption( tokenCast("searchpath"), "archive", &popt, RI_NULL );
	}
	else
	{
		Aqsis::log() << info << "AQSIS_ARCHIVE_PATH not set" << std::endl;
	}

	if(getenv("AQSIS_TEXTURE_PATH"))
	{
		popt[0] = getenv("AQSIS_TEXTURE_PATH");
		Aqsis::log() << info << "Applying AQSIS_TEXTURE_PATH (" << popt[0] << ")" << std::endl;
		RiOption( tokenCast("searchpath"), "texture", &popt, RI_NULL );
	}
	else
	{
		Aqsis::log() << info << "AQSIS_TEXTURE_PATH not set" << std::endl;
	}

	if(getenv("AQSIS_DISPLAY_PATH"))
	{
		popt[0] = getenv("AQSIS_DISPLAY_PATH");
		Aqsis::log() << info << "Applying AQSIS_DISPLAY_PATH (" << popt[0] << ")" << std::endl;
		RiOption( tokenCast("searchpath"), "display", &popt, RI_NULL );
	}
	else
	{
		Aqsis::log() << info << "AQSIS_DISPLAY_PATH not set" << std::endl;
	}

	if(getenv("AQSIS_PROCEDURAL_PATH"))
	{
		popt[0] = getenv("AQSIS_PROCEDURAL_PATH");
		Aqsis::log() << info << "Applying AQSIS_PROCEDURAL_PATH (" << popt[0] << ")" << std::endl;
		RiOption( tokenCast("searchpath"), "procedural", &popt, RI_NULL );
	}
	else
	{
		Aqsis::log() << info << "AQSIS_PROCEDURAL_PATH not set" << std::endl;
	}

	// Setup a default Display
	Aqsis::log() << info << "Setting up default display: Display \"ri.pic\" \"file\" \"rgba\"" << std::endl;
	RiDisplay( tokenCast("ri.pic"), tokenCast("file"), tokenCast("rgba"), NULL );
}

//----------------------------------------------------------------------
// RiBegin
// Begin a Renderman render phase.
//
RtVoid	RiBegin( RtToken name )
{
	// Create a new renderer
	QSetRenderContext( new CqRenderer );

	QGetRenderContext() ->Initialise();
	QGetRenderContext() ->BeginMainModeBlock();
	QGetRenderContext() ->ptransSetTime( CqMatrix() );
	QGetRenderContext() ->SetCameraTransform( QGetRenderContext() ->ptransCurrent() );
	// Clear the lightsources stack.
	Lightsource_stack.clear();

	SetDefaultRiOptions();

	// Setup a default surface shader
	boost::shared_ptr<IqShader> pDefaultSurfaceShader =
	    QGetRenderContext()->getDefaultSurfaceShader();
	QGetRenderContext() ->pattrWriteCurrent() ->SetpshadSurface( pDefaultSurfaceShader, QGetRenderContext() ->Time() );

	// Setup the initial transformation.
	//	QGetRenderContext()->ptransWriteCurrent() ->SetHandedness( false );
	QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "Orientation" ) [ 0 ] = 0;
}

//----------------------------------------------------------------------
// RiEnd
// End the rendermam render stage.
//
RtVoid	RiEnd()
{
	QGetRenderContext() ->EndMainModeBlock();

	// Clear the lightsources stack.
	Lightsource_stack.clear();

	// Delete the renderer
	delete( QGetRenderContext() );
	QSetRenderContext( 0 );
}


//----------------------------------------------------------------------
// RiFrameBegin
// Begin an individual frame, options are saved at this point.
//
RtVoid	RiFrameBegin( RtInt number )
{
	// Initialise the statistics variables. If the RIB doesn't contain
	// a Frame-block the initialisation was previously done in CqStats::Initilise()
	// which has to be called before a rendering session.
	QGetRenderContext() ->Stats().InitialiseFrame();
	// Start the timer. Note: The corresponding call of StopFrameTimer() is
	// done in WorldEnd (!) not FrameEnd since it can happen that there is
	// not FrameEnd (and usually there's not much between WorldEnd and FrameEnd).
	//QGetRenderContext() ->Stats().StartFrameTimer();
	AQSIS_TIMER_START(Frame);

	QGetRenderContext() ->BeginFrameModeBlock();
	QGetRenderContext() ->SetCurrentFrame( number );
	CqCSGTreeNode::SetRequired( false );

	QGetRenderContext() ->Stats().InitialiseFrame();

	QGetRenderContext()->clippingVolume().clear();


	worldrand.Reseed('a'+'q'+'s'+'i'+'s');
}


//----------------------------------------------------------------------
// RiFrameEnd
// End the rendering of an individual frame, options are restored.
//
RtVoid	RiFrameEnd()
{
	QGetRenderContext() ->EndFrameModeBlock();
	QGetRenderContext() ->ClearDisplayRequests();
}

//----------------------------------------------------------------------
// RiWorldBegin
// Start the information for the world, options are now frozen.  The world-to-camera
// transformation is set to the current transformation, and current is set to identity.
//
RtVoid	RiWorldBegin()
{
	// Call any specified pre world function.
	if ( QGetRenderContext()->pPreWorldFunction() != NULL )
		( *QGetRenderContext()->pPreWorldFunction() ) ();

	// Start the frame timer (just in case there was no FrameBegin block. If there
	// was, nothing happens)
	//QGetRenderContext() ->Stats().StartFrameTimer();
	AQSIS_TIMER_START(Frame);
	AQSIS_TIMER_START(Parse);

	// Now that the options have all been set, setup any undefined camera parameters.
	const TqInt* pCameraOpts = QGetRenderContext()->poptCurrent()->GetIntegerOption("System", "CameraFlags");
	TqInt cameraOpts = 0;
	if(pCameraOpts != NULL)
		cameraOpts = pCameraOpts[0];
	if ( (cameraOpts & CameraFARSet) == 0 )
	{
		// Derive the FAR from the resolution and pixel aspect ratio.
		RtFloat PAR = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "PixelAspectRatio" ) [ 0 ];
		RtFloat resH = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 0 ];
		RtFloat resV = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 1 ];
		QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "FrameAspectRatio" ) [ 0 ] = ( resH * PAR ) / resV ;
	}

	if ( ( cameraOpts & CameraScreenWindowSet) == 0 )
	{
		RtFloat fFAR = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "FrameAspectRatio" ) [ 0 ];

		if ( fFAR >= 1.0 )
		{
			QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 0 ] = -fFAR ;
			QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 1 ] = + fFAR ;
			QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 2 ] = + 1 ;
			QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 3 ] = -1 ;
		}
		else
		{
			QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 0 ] = -1 ;
			QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 1 ] = + 1 ;
			QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 2 ] = + 1.0 / fFAR ;
			QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 3 ] = -1.0 / fFAR ;
		}
	}

	// Set the world to camera transformation matrix to the current matrix.

	CqTransformPtr current( QGetRenderContext() ->ptransCurrent() );
	QGetRenderContext() ->SetCameraTransform( current );
	QGetRenderContext() ->BeginWorldModeBlock();
	// Set the camera transformation for shadow maps in the sampler cache.
	/// \todo What is the correct coordinate system to use here? "current"? "shader"?
	CqMatrix currToWorldMat;
	QGetRenderContext()->matSpaceToSpace("current", "world", NULL, NULL, 0, currToWorldMat);
	QGetRenderContext()->textureCache().setCurrToWorldMatrix(currToWorldMat);

	// Reset the current transformation to identity, this now represents the object-->world transform.
	QGetRenderContext() ->ptransSetTime( CqMatrix() );

	// Store the initial object transformation
	CqTransformPtr newTrans( new CqTransform() );
	QGetRenderContext()->SetDefObjTransform( newTrans );

	// If rendering a depth buffer, check that the filter is "box" 1x1, warn if not.
	TqInt iMode = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "DisplayMode" ) [ 0 ];
	if( iMode & DMode_Z )
	{
		RtFilterFunc filter = QGetRenderContext() ->poptCurrent()->funcFilter();
		TqFloat xwidth = QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "FilterWidth" ) [ 0 ];
		TqFloat ywidth = QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "FilterWidth" ) [ 1 ];
		if( filter != RiBoxFilter || xwidth != 1 || ywidth != 1)
			Aqsis::log() << warning << "When rendering a Z buffer the filter mode should be \"box\" with a width of 1x1" << std::endl;
	}

	QGetRenderContext()->SetWorldBegin();
	
	// Ensure that the camera and projection matrices are initialised.
	// This is also done in CqRenderer::RenderWorld, but needs to be 
	// done here also in case we're not running in 'multipass' mode, in 
	// which case the primitives all 'fast track' into the pipeline and
	// therefore rely on information setup here.
	QGetRenderContext()->poptWriteCurrent()->InitialiseCamera();
	// initialiseCropWindow() currently needs to be called befure SetImage() (ugh)
	QGetRenderContext()->initialiseCropWindow();
	QGetRenderContext()->pImage()->SetImage();

	worldrand.Reseed('a'+'q'+'s'+'i'+'s');
}


//----------------------------------------------------------------------
// RiWorldEnd
// End the specifying of world data, options are released.
//

RtVoid	RiWorldEnd()
{
	QGetRenderContext()->RenderAutoShadows();

	bool fFailed = false;
	// Call any specified pre render function.
	if ( QGetRenderContext()->pPreRenderFunction() != NULL )
		( *QGetRenderContext()->pPreRenderFunction() ) ();

	// Stop the parsing counter
	AQSIS_TIMER_STOP(Parse);


	QGetRenderContext() -> Stats().PrintInfo();

	const TqInt* poptGridSize = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "limits", "gridsize" );
	if( NULL != poptGridSize )
		QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "SqrtGridSize" )[0] = sqrt( static_cast<float>(poptGridSize[0]) );

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
		fFailed = true;
	}

	// Remove all cached textures.
	QGetRenderContext()->textureCache().flush();

	// Delete the world context
	QGetRenderContext() ->EndWorldModeBlock();

	// Stop the frame timer
	AQSIS_TIMER_STOP(Frame);

	if ( !fFailed )
	{
		// Get the verbosity level from the options..
		TqInt verbosity = 0;
		const TqInt* poptEndofframe = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "statistics", "endofframe" );
		if ( poptEndofframe != 0 )
			verbosity = poptEndofframe[ 0 ];

		// ..and print the statistics.
		QGetRenderContext() ->Stats().PrintStats( verbosity );
	}

	QGetRenderContext()->SetWorldBegin(false);
}


//----------------------------------------------------------------------
// RiFormat
// Specify the setup of the final image.
//
RtVoid	RiFormat( RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio )
{
	QGetRenderContext() ->poptWriteCurrent()->GetIntegerOptionWrite( "System", "Resolution" ) [ 0 ] = xresolution ;
	QGetRenderContext() ->poptWriteCurrent()->GetIntegerOptionWrite( "System", "Resolution" ) [ 1 ] = yresolution ;
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "PixelAspectRatio" ) [ 0 ] = ( pixelaspectratio < 0.0 ) ? 1.0 : pixelaspectratio ;
}


//----------------------------------------------------------------------
// RiFrameAspectRatio
// Set the aspect ratio of the frame irrespective of the display setup.
//
RtVoid	RiFrameAspectRatio( RtFloat frameratio )
{
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "FrameAspectRatio" ) [ 0 ] = frameratio ;

	// Inform the system that RiFrameAspectRatio has been called, as this takes priority.
	QGetRenderContext()->poptWriteCurrent()->GetIntegerOptionWrite("System", "CameraFlags")[0] |= CameraFARSet;
}


//----------------------------------------------------------------------
// RiScreenWindow
// Set the resolution of the screen window in the image plane specified in the screen
// coordinate system.
//
RtVoid	RiScreenWindow( RtFloat left, RtFloat right, RtFloat bottom, RtFloat top )
{
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 0 ] = left ;
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 1 ] = right ;
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 2 ] = top ;
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 3 ] = bottom ;

	// Inform the system that RiScreenWindow has been called, as this takes priority.
	QGetRenderContext()->poptWriteCurrent()->GetIntegerOptionWrite("System", "CameraFlags")[0] |= CameraScreenWindowSet;
}


//----------------------------------------------------------------------
// RiCropWindow
// Set the position and size of the crop window specified in fractions of the raster
// window.
//
RtVoid	RiCropWindow( RtFloat xmin, RtFloat xmax, RtFloat ymin, RtFloat ymax )
{
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "CropWindow" ) [ 0 ] = xmin ;
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "CropWindow" ) [ 1 ] = xmax ;
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "CropWindow" ) [ 2 ] = ymin ;
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "CropWindow" ) [ 3 ] = ymax ;
}


//----------------------------------------------------------------------
// RiProjectionV
// List mode version of above.
//
RtVoid	RiProjectionV( RtToken name, PARAMETERLIST )
{
	if(NULL != name)
	{
		if ( strcmp( name, RI_PERSPECTIVE ) == 0 )
			QGetRenderContext() ->poptWriteCurrent()->GetIntegerOptionWrite( "System", "Projection" ) [ 0 ] = ProjectionPerspective ;
		else if	( strcmp( name, RI_ORTHOGRAPHIC ) == 0 )
			QGetRenderContext() ->poptWriteCurrent()->GetIntegerOptionWrite( "System", "Projection" ) [ 0 ] = ProjectionOrthographic ;
		else if( strlen( name ) == 0 || name == RI_NULL )
			QGetRenderContext() ->poptWriteCurrent()->GetIntegerOptionWrite( "System", "Projection" ) [ 0 ] = ProjectionNone ;
		else
		{
			Aqsis::log() << error << "RiProjection: Invalid projection: \"" << name << "\"" << std::endl;
			return ;
		}

		RtInt i;
		for ( i = 0; i < count; ++i )
		{
			CqPrimvarToken token = QGetRenderContext()->tokenDict().parseAndLookup( tokens[ i ] );
			RtPointer	value = values[ i ];

			if ( token.name() == RI_FOV )
				QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "FOV" ) [ 0 ] = *( reinterpret_cast<RtFloat*>( value ) ) ;
		}
	}
	// TODO: need to get the current transformation so that it can be added to the screen transformation.
	QGetRenderContext() ->SetpreProjectionTransform( QGetRenderContext() ->ptransCurrent() );
	QGetRenderContext() ->ptransSetTime( CqMatrix() );
}


//----------------------------------------------------------------------
// RiClipping
// Set the near and far clipping planes specified as distances from the camera.
//
RtVoid	RiClipping( RtFloat cnear, RtFloat cfar )
{
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "Clipping" ) [ 0 ] = cnear;
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "Clipping" ) [ 1 ] = cfar;
}


//----------------------------------------------------------------------
// RiDepthOfField
// Specify the parameters which affect focal blur of the camera.
//
RtVoid	RiDepthOfField( RtFloat fstop, RtFloat focallength, RtFloat focaldistance )
{
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "DepthOfField" ) [ 0 ] = fstop ;
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "DepthOfField" ) [ 1 ] = focallength ;
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "DepthOfField" ) [ 2 ] = focaldistance ;
}


//----------------------------------------------------------------------
// RiShutter
//	Set the times at which the shutter opens and closes, used for motion blur.
//
RtVoid	RiShutter( RtFloat opentime, RtFloat closetime )
{
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "Shutter" ) [ 0 ] = opentime;
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "Shutter" ) [ 1 ] = closetime;
}


//----------------------------------------------------------------------
// RiPixelVariance
// Set the upper bound on the variance from the true pixel color by the pixel filter
// function.
//
RtVoid	RiPixelVariance( RtFloat variance )
{
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "PixelVariance" ) [ 0 ] = variance ;
}


//----------------------------------------------------------------------
// RiPixelSamples
// Set the number of samples per pixel for the hidden surface function.
//
RtVoid	RiPixelSamples( RtFloat xsamples, RtFloat ysamples )
{
	QGetRenderContext() ->poptWriteCurrent()->GetIntegerOptionWrite( "System", "PixelSamples" ) [ 0 ] = static_cast<TqInt>( xsamples ) ;
	QGetRenderContext() ->poptWriteCurrent()->GetIntegerOptionWrite( "System", "PixelSamples" ) [ 1 ] = static_cast<TqInt>( ysamples ) ;
}


//----------------------------------------------------------------------
// RiPixelFilter
// Set the function used to generate a final pixel value from supersampled values.
//
RtVoid	RiPixelFilter( RtFilterFunc function, RtFloat xwidth, RtFloat ywidth )
{
	QGetRenderContext() ->poptWriteCurrent()->SetfuncFilter( function );
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "FilterWidth" ) [ 0 ] = xwidth ;
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "FilterWidth" ) [ 1 ] = ywidth ;
}


//----------------------------------------------------------------------
// RiExposure
//	Set the values of the exposure color modification function.
//
RtVoid	RiExposure( RtFloat gain, RtFloat gamma )
{
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "Exposure" ) [ 0 ] = gain ;
	QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "Exposure" ) [ 1 ] = gamma ;
}


//----------------------------------------------------------------------
// RiImagerV
// Specify a prepocessing imager shader.
//
RtVoid	RiImagerV( RtToken name, PARAMETERLIST )
{
	// Find the shader.
	boost::shared_ptr<IqShader> pshadImager = QGetRenderContext()->CreateShader( name, Type_Imager );

	if ( pshadImager )
	{
		QGetRenderContext()->poptWriteCurrent()->SetpshadImager( pshadImager );
		RtInt i;
		for ( i = 0; i < count; ++i )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			SetShaderArgument( pshadImager, token, static_cast<TqPchar>( value ) );
		}
		const TqInt* pMultipass = QGetRenderContext()->poptCurrent()->GetIntegerOption("Render", "multipass");
		if(pMultipass && !pMultipass[0])
			pshadImager->PrepareShaderForUse();
	}
}


//----------------------------------------------------------------------
// RiQuantize
// Specify the color quantization parameters.
//
RtVoid	RiQuantize( RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude )
{
	if ( strcmp( type, "rgba" ) == 0 )
	{
		TqFloat* pColorQuantize = QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "Quantize", "Color" );
		pColorQuantize [ 0 ] = static_cast<TqFloat>( one );
		pColorQuantize [ 1 ] = static_cast<TqFloat>( min );
		pColorQuantize [ 2 ] = static_cast<TqFloat>( max );
		pColorQuantize [ 3 ] = static_cast<TqFloat>( ditheramplitude );
	}
	else if ( strcmp( type, "z" ) == 0 )
	{
		TqFloat* pDepthQuantize = QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "Quantize", "Depth" );
		pDepthQuantize [ 0 ] = static_cast<TqFloat>( one );
		pDepthQuantize [ 1 ] = static_cast<TqFloat>( min );
		pDepthQuantize [ 2 ] = static_cast<TqFloat>( max );
		pDepthQuantize [ 3 ] = static_cast<TqFloat>( ditheramplitude );
	}
	else
	{
		TqFloat* quantOpt = QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite("Quantize", type, 4);
		quantOpt[0] = static_cast<TqFloat>( one );
		quantOpt[1] = static_cast<TqFloat>( min );
		quantOpt[2] = static_cast<TqFloat>( max );
		quantOpt[3] = static_cast<TqFloat>( ditheramplitude );
	}
}


//----------------------------------------------------------------------
// RiDisplayV
// Set the final output name and type.
//
RtVoid	RiDisplayV( RtToken name, RtToken type, RtToken mode, PARAMETERLIST )
{
	CqString strName( name );
	CqString strType( type );

	QGetRenderContext() ->poptWriteCurrent()->GetStringOptionWrite( "System", "DisplayName" ) [ 0 ] = strName.c_str() ;
	QGetRenderContext() ->poptWriteCurrent()->GetStringOptionWrite( "System", "DisplayType" ) [ 0 ] = strType.c_str() ;

	// Append the display mode to the current setting.
	TqInt eValue = 0;
	TqInt index = 0;
	TqInt dataOffset = 0;
	TqInt dataSize = 0;
	std::string dataName = mode;
	if ( strncmp( mode, RI_RGB, strlen(RI_RGB) ) == 0 )
	{
		eValue |= DMode_RGB;
		dataSize += 3;
		index += strlen( RI_RGB );
	}
	if ( strncmp( &mode[index], RI_A, strlen( RI_A ) ) == 0 )
	{
		eValue |= DMode_A;
		dataSize += 1;
		index += strlen( RI_A );
	}
	if ( strncmp( &mode[index], RI_Z, strlen( RI_Z ) ) == 0 )
	{
		eValue |= DMode_Z;
		dataSize += 1;
		index += strlen( RI_Z );
	}

	// Special case test.
	if(strncmp(&mode[index], "depth", strlen("depth") ) == 0 )
	{
		dataSize = 1;
		/// \todo This shouldn't be a constant.
		dataOffset = 6;
	}
	// If none of the standard "rgbaz" strings match, then it is an alternative 'arbitrary output variable'
	else if( eValue == 0 )
	{
		dataOffset = QGetRenderContext()->RegisterOutputData( mode );
		dataSize = QGetRenderContext()->OutputDataSamples( mode );
		CqPrimvarToken tok = QGetRenderContext()->tokenDict().parseAndLookup(mode);
		dataName = tok.name();
	}

	// Check if the display request is valid.
	if(dataOffset >= 0 && dataSize >0)
	{
		// Gather the additional arguments into a map to pass through to the manager.
		std::map<std::string, void*> mapOfArguments;
		TqInt i;
		for( i = 0; i < count; ++i )
			mapOfArguments[ tokens[ i ] ] = values[ i ];

		// Check if the request is to add a display driver.
		if ( strName[ 0 ] == '+' )
		{
			TqInt iMode = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "DisplayMode" ) [ 0 ] | eValue;
			QGetRenderContext() ->poptWriteCurrent()->GetIntegerOptionWrite( "System", "DisplayMode" ) [ 0 ] = iMode;
			strName = strName.substr( 1 );
		}
		else
		{
			QGetRenderContext() ->ClearDisplayRequests();
			QGetRenderContext() ->poptWriteCurrent()->GetIntegerOptionWrite( "System", "DisplayMode" ) [ 0 ] = eValue ;
		}
		// Add a display driver to the list of requested drivers.
		QGetRenderContext() ->AddDisplayRequest( strName.c_str(), strType.c_str(), dataName.c_str(), eValue, dataOffset, dataSize, mapOfArguments );
	}
}

//----------------------------------------------------------------------
// RiHiderV
// Specify a hidden surface calculation mode.
//
RtVoid	RiHiderV( RtToken name, PARAMETERLIST )
{
	if ( !strcmp( name, "hidden" ) || !strcmp( name, "painter" ) )
	{
		QGetRenderContext() ->poptWriteCurrent()->GetStringOptionWrite( "System", "Hider" ) [ 0 ] = name ;
	}

	// Check options.
	for ( TqInt i = 0; i < count; ++i )
	{
		CqPrimvarToken tok;
		try
		{
			tok = QGetRenderContext()->tokenDict().parseAndLookup(tokens[i]);
		}
		catch(XqValidation& e)
		{
			Aqsis::log() << error << "RiHider: " << e.what() << std::endl;
			continue;
		}
		const TqUlong hash = CqString::hash(tok.name().c_str());
		if ( hash == RIH_DEPTHFILTER )
		  RiOption( tokenCast("Hider"), "depthfilter", ( RtToken ) values[ i ], NULL );
		else if ( hash == RIH_JITTER )
		  RiOption( tokenCast("Hider"), "jitter", ( RtFloat* ) values[ i ], NULL );
	}
}


//----------------------------------------------------------------------
// RiColorSamples
// Specify the depth and conversion arrays for color manipulation.
//
RtVoid	RiColorSamples( RtInt N, RtFloat *nRGB, RtFloat *RGBn )
{
	Aqsis::log() << warning << "RiColorSamples not supported" << std::endl;
}


//----------------------------------------------------------------------
// RiRelativeDetail
// Set the scale used for all subsequent level of detail calculations.
//
RtVoid	RiRelativeDetail( RtFloat relativedetail )
{
	if ( relativedetail < 0.0f )
	{
		Aqsis::log() << error << "RiRelativeDetail < 0.0" << std::endl;
	}
	else
	{
		QGetRenderContext() ->poptWriteCurrent()->GetFloatOptionWrite( "System", "RelativeDetail" ) [ 0 ] = relativedetail;
	}
}


//----------------------------------------------------------------------
// RiOptionV
// Specify system specific option.
//
RtVoid	RiOptionV( RtToken name, PARAMETERLIST )
{
	for ( TqInt i = 0; i < count; ++i )
	{
		RtToken	token = tokens[ i ];
		RtPointer value = values[ i ];

		// Search for the parameter in the declarations.
		// Note Options can only be uniform.
		CqPrimvarToken tok;
		try
		{
			tok = QGetRenderContext()->tokenDict().parseAndLookup(token);
		}
		catch(XqValidation& e)
		{
			Aqsis::log() << error << "RiOption: " << e.what() << std::endl;
			continue;
		}
		if ( tok.Class() != class_uniform )
		{
			Aqsis::log() << warning << "RiOption: options can only be uniform ["
				<< token << "]" << std::endl;
			continue;
		}

		TqInt Count = tok.count();
		const char* undecoratedName = tok.name().c_str();
		switch ( tok.type() )
		{
			case type_float:
			{
				RtFloat* pf = reinterpret_cast<RtFloat*>( value );
				TqFloat* pOpt = QGetRenderContext()->poptWriteCurrent()->GetFloatOptionWrite(name, undecoratedName, Count);
				RtInt j;
				for ( j = 0; j < Count; ++j )
					pOpt[ j ] = pf[ j ];
			}
			break;

			case type_integer:
			{
				RtInt* pi = reinterpret_cast<RtInt*>( value );
				TqInt* pOpt = QGetRenderContext()->poptWriteCurrent()->GetIntegerOptionWrite(name, undecoratedName, Count);
				RtInt j;
				for ( j = 0; j < Count; ++j )
					pOpt[ j ] = pi[ j ];
			}
			break;

			case type_string:
			{
				char** ps = reinterpret_cast<char**>( value );
				CqString* pOpt = QGetRenderContext()->poptWriteCurrent()
					->GetStringOptionWrite(name, undecoratedName, Count);
				RtInt j;
				for ( j = 0; j < Count; ++j )
				{
					if ( strcmp( name, "searchpath" ) == 0 )
					{
						// Get the default string for use in escape replacement
						const CqString* pDefSearch
							= QGetRenderContext()->poptWriteCurrent()
							->GetStringOption("defaultsearchpath", undecoratedName);
						std::string defaultSearch;
						if(pDefSearch)
							defaultSearch = *pDefSearch;
						Aqsis::log() << debug << "Old " << undecoratedName << " searchpath = " << pOpt[j] << std::endl;
						pOpt[j] = expandSearchPath(ps[j], pOpt[j], defaultSearch);
						Aqsis::log() << debug << "New " << undecoratedName << " searchpath = " << pOpt[j] << std::endl;
					}
					else if ( strcmp( name, "defaultsearchpath" ) == 0 )
					{
						Aqsis::log() << debug << "Old " << undecoratedName << " defaultsearchpath = " << pOpt[j] << std::endl;
						pOpt[j] = expandSearchPath(ps[j], pOpt[j], std::string());
						Aqsis::log() << debug << "New " << undecoratedName << " defaultsearchpath = " << pOpt[j] << std::endl;
					}
					else
						pOpt[j] = ps[j];
				}
			}
			break;

			case type_color:
			{
				RtFloat* pc = reinterpret_cast<RtFloat*>( value );
				CqColor* pOpt = QGetRenderContext()->poptWriteCurrent()->GetColorOptionWrite(name, undecoratedName, Count);
				RtInt j;
				for ( j = 0; j < Count; ++j )
					pOpt[ j ] = CqColor(pc[ (j*3) ], pc[ (j*3)+1 ], pc[ (j*3)+2 ]);
			}
			break;

			case type_point:
			{
				RtFloat* pc = reinterpret_cast<RtFloat*>( value );
				CqVector3D* pOpt = QGetRenderContext()->poptWriteCurrent()->GetPointOptionWrite(name, undecoratedName, Count);
				RtInt j;
				for ( j = 0; j < Count; ++j )
					pOpt[ j ] = CqVector3D(pc[ (j*3) ], pc[ (j*3)+1 ], pc[ (j*3)+2 ]);
			}
			break;

			case type_normal:
			{
				RtFloat* pc = reinterpret_cast<RtFloat*>( value );
				CqVector3D* pOpt = QGetRenderContext()->poptWriteCurrent()->GetPointOptionWrite(name, undecoratedName, Count);
				RtInt j;
				for ( j = 0; j < Count; ++j )
					pOpt[ j ] = CqVector3D(pc[ (j*3) ], pc[ (j*3)+1 ], pc[ (j*3)+2 ]);
			}
			break;

			case type_vector:
			{
				RtFloat* pc = reinterpret_cast<RtFloat*>( value );
				CqVector3D* pOpt = QGetRenderContext()->poptWriteCurrent()->GetPointOptionWrite(name, undecoratedName, Count);
				RtInt j;
				for ( j = 0; j < Count; ++j )
					pOpt[ j ] = CqVector3D(pc[ (j*3) ], pc[ (j*3)+1 ], pc[ (j*3)+2 ]);
			}
			break;

			case type_hpoint:
			{
/*				RtFloat* pc = reinterpret_cast<RtFloat*>( value );
				CqVector4D* pOpt = QGetRenderContext()->poptWriteCurrent()->GetHPointOptionWrite(name, undecoratedName, Count);
				RtInt j;
				for ( j = 0; j < Count; ++j )
					pOpt[ j ] = CqVector4D(pc[ (j*4) ], pc[ (j*4)+1 ], pc[ (j*4)+2 ], pc[ (j*4)+3]); */
			}
			break;

			case type_matrix:
			{
/*				RtFloat* pc = reinterpret_cast<RtFloat*>( value );
				CqMatrix* pOpt = QGetRenderContext()->poptWriteCurrent()->GetMatrixOptionWrite(name, undecoratedName, Count);
				RtInt j;
				for ( j = 0; j < Count; ++j )
					pOpt[ j ] = CqMatrix(pm[ j    ], pm[ j+1  ], pm[ j+2  ], pm[ j+3  ],
							        pm[ j+4  ], pm[ j+5  ], pm[ j+6  ], pm[ j+7  ],
							        pm[ j+8  ], pm[ j+9  ], pm[ j+10 ], pm[ j+11 ],
							        pm[ j+12 ], pm[ j+13 ], pm[ j+14 ], pm[ j+15 ]); */
			}
			break;

			default:
			break;
		}
	}
}


//----------------------------------------------------------------------
// RiAttributeBegin
// Begin a ne attribute definition, pushes the current attributes.
//
RtVoid	RiAttributeBegin()
{
	QGetRenderContext() ->BeginAttributeModeBlock();
}


//----------------------------------------------------------------------
// RiAttributeEnd
// End the current attribute defintion, pops the previous attributes.
//
RtVoid	RiAttributeEnd()
{
	QGetRenderContext() ->EndAttributeModeBlock();
}


//----------------------------------------------------------------------
// RiColor
//	Set the current color for use by the geometric primitives.
//
RtVoid	RiColor( RtColor Cq )
{
	QGetRenderContext() ->pattrWriteCurrent() ->GetColorAttributeWrite( "System", "Color" ) [ 0 ] = CqColor( Cq );
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiOpacity
// Set the current opacity, for use by the geometric primitives.
//
RtVoid	RiOpacity( RtColor Os )
{
	QGetRenderContext() ->pattrWriteCurrent() ->GetColorAttributeWrite( "System", "Opacity" ) [ 0 ] = CqColor( Os );
	QGetRenderContext() ->AdvanceTime();
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
}


//----------------------------------------------------------------------
// RiLightSourceV
// Create a new light source at the current transformation.
//
RtLightHandle	RiLightSourceV( RtToken name, PARAMETERLIST )
{
	// Find the lightsource shader.
	boost::shared_ptr<IqShader> pShader = QGetRenderContext()->CreateShader( name, Type_Lightsource );
	if(!pShader)
		return 0;

	pShader->SetTransform( QGetRenderContext() ->ptransCurrent() );
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

		const TqInt* pMultipass = QGetRenderContext()->poptCurrent()->GetIntegerOption("Render", "multipass");
		if(pMultipass && !pMultipass[0])
			pShader->PrepareShaderForUse();

		// Add it as a Context light as well in case we are in a context that manages it's own lights.
		QGetRenderContext() ->pconCurrent() ->AddContextLightSource( pNew );
		return ( reinterpret_cast<RtLightHandle>( pNew.get() ) );
	}
	return 0;
}


//----------------------------------------------------------------------
// RiAreaLightSourceV
// Create a new area light source at the current transformation, all
// geometric primitives until the next RiAttributeEnd, become part of this
// area light source.
//
RtLightHandle	RiAreaLightSourceV( RtToken name, PARAMETERLIST )
{
	Aqsis::log() << warning << "RiAreaLightSource not supported, will produce a point light" << std::endl;

	return RiLightSourceV( name, count, tokens, values );
}


//----------------------------------------------------------------------
// RiIlluminate
// Set the current status of the specified light source.
//
RtVoid	RiIlluminate( RtLightHandle light, RtBoolean onoff )
{
	// Check if we are turning the light on or off.
	if ( light == NULL ) return ;

	CqLightsourcePtr pL( reinterpret_cast<CqLightsource*>( light )->shared_from_this() );

	if ( onoff )
		QGetRenderContext() ->pattrWriteCurrent() ->AddLightsource( pL );
	else
		QGetRenderContext() ->pattrWriteCurrent() ->RemoveLightsource( pL );
}


//----------------------------------------------------------------------
// RiSurfaceV
// Set the current surface shader, used by geometric primitives.
//
RtVoid	RiSurfaceV( RtToken name, PARAMETERLIST )
{
	// Find the shader.
	boost::shared_ptr<IqShader> pshadSurface = QGetRenderContext()->CreateShader( name, Type_Surface );

	if ( pshadSurface )
	{
		pshadSurface->SetTransform( QGetRenderContext() ->ptransCurrent() );
		// Execute the intiialisation code here, as we now have our shader context complete.
		pshadSurface->PrepareDefArgs();
		RtInt i;
		for ( i = 0; i < count; ++i )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			SetShaderArgument( pshadSurface, token, static_cast<TqPchar>( value ) );
		}

		const TqInt* pMultipass = QGetRenderContext()->poptCurrent()->GetIntegerOption("Render", "multipass");
		if(pMultipass && !pMultipass[0])
			pshadSurface->PrepareShaderForUse();

		QGetRenderContext() ->pattrWriteCurrent() ->SetpshadSurface( pshadSurface, QGetRenderContext() ->Time() );
	}
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiAtmosphereV
// Set the current atrmospheric shader.
//
RtVoid	RiAtmosphereV( RtToken name, PARAMETERLIST )
{
	// Find the shader.
	boost::shared_ptr<IqShader> pshadAtmosphere = QGetRenderContext()->CreateShader( name, Type_Volume );

	if ( pshadAtmosphere )
	{
		pshadAtmosphere->SetTransform( QGetRenderContext() ->ptransCurrent() );
		// Execute the intiialisation code here, as we now have our shader context complete.
		pshadAtmosphere->PrepareDefArgs();
		RtInt i;
		for ( i = 0; i < count; ++i )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			SetShaderArgument( pshadAtmosphere, token, static_cast<TqPchar>( value ) );
		}
		const TqInt* pMultipass = QGetRenderContext()->poptCurrent()->GetIntegerOption("Render", "multipass");
		if(pMultipass && !pMultipass[0])
			pshadAtmosphere->PrepareShaderForUse();
	}

	QGetRenderContext() ->pattrWriteCurrent() ->SetpshadAtmosphere( pshadAtmosphere, QGetRenderContext() ->Time() );
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiInteriorV
// Set the current interior volumetric shader.
//
RtVoid	RiInteriorV( RtToken name, PARAMETERLIST )
{
	Aqsis::log() << warning << "RiInterior not supported" << std::endl;
}


//----------------------------------------------------------------------
// RiExteriorV
// Set the current exterior volumetric shader.
//
RtVoid	RiExteriorV( RtToken name, PARAMETERLIST )
{
	Aqsis::log() << warning << "ExInterior not supported" << std::endl;
}


//----------------------------------------------------------------------
// RiShadingRate
// Specify the size of the shading area in pixels.
//
RtVoid	RiShadingRate( RtFloat size )
{
	QGetRenderContext() ->pattrWriteCurrent() ->GetFloatAttributeWrite( "System", "ShadingRate" ) [ 0 ] = size;
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiShadingInterpolation
// Specify the method of shading interpolation.
//
RtVoid	RiShadingInterpolation( RtToken type )
{
	if ( strcmp( type, RI_CONSTANT ) == 0 )
		QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "ShadingInterpolation" ) [ 0 ] = ShadingInterp_Constant;
	else if ( strcmp( type, RI_SMOOTH ) == 0 )
		QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "ShadingInterpolation" ) [ 0 ] = ShadingInterp_Smooth;
	else
		Aqsis::log() << error << "RiShadingInterpolation unrecognised value \"" << type << "\"" << std::endl;

	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiMatte
// Set the matte state of subsequent geometric primitives.
//
RtVoid	RiMatte( RtBoolean onoff )
{
	QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "Matte" ) [ 0 ] = onoff;
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiBound
// Set the bounding cube of the current primitives.
//
RtVoid	RiBound( RtBound bound )
{
	// TODO: Need to add a "Bound" attribute here, and fill it in.
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiDetail
// Set the current bounding cube for use by level of detail calculation.
//
RtVoid	RiDetail( RtBound bound )
{
	CqBound Bound( bound );

	TqFloat* boundAttr = QGetRenderContext() ->pattrWriteCurrent() ->GetFloatAttributeWrite( "System", "LODBound" );
	boundAttr[0] = bound[0];
	boundAttr[1] = bound[1];
	boundAttr[2] = bound[2];
	boundAttr[3] = bound[3];
	boundAttr[4] = bound[4];
	boundAttr[5] = bound[5];
}


//----------------------------------------------------------------------
// RiDetailRange
// Set the visible range of any subsequent geometric primitives.
//
RtVoid	RiDetailRange( RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh )
{
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
}


//----------------------------------------------------------------------
// RiGeometricApproximation
// Specify any parameters used by approximation functions during rendering.
//
RtVoid	RiGeometricApproximation( RtToken type, RtFloat value )
{
	std::string typeStr = type;
	if(typeStr == RI_FLATNESS)
	{
		TqFloat* flatnessAttr = QGetRenderContext()->pattrWriteCurrent()->
			GetFloatAttributeWrite("System", "GeometricFlatness");
		flatnessAttr[0] = value;
		Aqsis::log() << warning
			<< "RiGeometricApproximation flatness test not yet implemented\n";
	}
	else if(typeStr == "focusfactor")
	{
		TqFloat* focusFactorAttr = QGetRenderContext()->pattrWriteCurrent()->
			GetFloatAttributeWrite("System", "GeometricFocusFactor");
		focusFactorAttr[0] = value;
	}
	else if(typeStr == "motionfactor")
	{
		TqFloat* motionFactorAttr = QGetRenderContext()->pattrWriteCurrent()->
			GetFloatAttributeWrite("System", "GeometricMotionFactor");
		motionFactorAttr[0] = value;
	}
	else
	{
		Aqsis::log() << warning << "RiGeometricApproximation type not known\n";
	}
}


//----------------------------------------------------------------------
// RiOrientation
// Set the handedness of any subsequent geometric primitives.
//
RtVoid	RiOrientation( RtToken orientation )
{
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
}


//----------------------------------------------------------------------
// RiReverseOrientation
// Reverse the handedness of any subsequent geometric primitives.
//
RtVoid	RiReverseOrientation()
{
	QGetRenderContext() ->pattrWriteCurrent() ->FlipeOrientation( QGetRenderContext() ->Time() );
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiSides
// Set the number of visibles sides for any subsequent geometric primitives.
//
RtVoid	RiSides( RtInt nsides )
{
	QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "Sides" ) [ 0 ] = nsides;
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiIdentity
// Set the current transformation to the identity matrix.
//
RtVoid	RiIdentity()
{
	QGetRenderContext() ->ptransSetTime( CqMatrix() );
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// Set the current transformation to the specified matrix.
//
RtVoid	RiTransform( RtMatrix transform )
{
	CqMatrix matTrans( transform );
	//    if ( matTrans.Determinant() < 0 && ( QGetRenderContext()->pconCurrent()->Type() != Motion || QGetRenderContext()->pconCurrent()->TimeIndex() == 0 ) )
	//        QGetRenderContext() ->ptransWriteCurrent() ->FlipHandedness( QGetRenderContext() ->Time() );

	if( QGetRenderContext()->IsWorldBegin() )
	{
		// If we're in the WorldBegin/End block, we need to take the 'camera' transform as the starting point.
		// This is because we have to transform primitives to 'world' coordinates.
		// So we read the 'default object transform' that is stored at 'RiWorldBegin', set the transform at the current time to that, then concat the specified transform.
		CqMatrix matDefTrans( QGetRenderContext()->GetDefObjTransform()->matObjectToWorld(QGetRenderContext()->Time()) );
		QGetRenderContext() ->ptransSetTime( matDefTrans );
		QGetRenderContext() ->ptransConcatCurrentTime( matTrans );
	}
	else
		QGetRenderContext() ->ptransSetTime( matTrans );
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiConcatTransform
// Concatenate the specified matrix into the current transformation matrix.
//
RtVoid	RiConcatTransform( RtMatrix transform )
{
	// Check if this transformation results in a change in orientation.
	CqMatrix matTrans( transform );
	//    if ( matTrans.Determinant() < 0 && ( QGetRenderContext()->pconCurrent()->Type() != Motion || QGetRenderContext()->pconCurrent()->TimeIndex() == 0 ) )
	//        QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );

	QGetRenderContext() ->ptransConcatCurrentTime( CqMatrix( transform ) );
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiPerspective
// Concatenate a perspective transformation into the current transformation.
//
RtVoid	RiPerspective( RtFloat fov )
{
	if ( fov <= 0 )
	{
		Aqsis::log() << error << "RiPerspective invalid FOV" << std::endl;
		return ;
	}

	fov = tan( degToRad( fov / 2 ) );

	// This matches PRMan 3.9 in testing, but not BMRT 2.6's rgl and rendrib.
	CqMatrix	matP( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, fov, fov, 0, 0, -fov, 0 );

	// Check if this transformation results in a change in orientation.
	//    if ( matP.Determinant() < 0 && ( QGetRenderContext()->pconCurrent()->Type() != Motion || QGetRenderContext()->pconCurrent()->TimeIndex() == 0 ) )
	//        QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );

	QGetRenderContext() ->ptransConcatCurrentTime( matP );
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiTranslate
// Concatenate a translation into the current transformation.
//
RtVoid	RiTranslate( RtFloat dx, RtFloat dy, RtFloat dz )
{
	CqMatrix	matTrans( CqVector3D( dx, dy, dz ) );
	// Check if this transformation results in a change in orientation.
	//    if ( matTrans.Determinant() < 0 && ( QGetRenderContext()->pconCurrent()->Type() != Motion || QGetRenderContext()->pconCurrent()->TimeIndex() == 0 ) )
	//        QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );

	QGetRenderContext() ->ptransConcatCurrentTime( matTrans );
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiRotate
// Concatenate a rotation into the current transformation.
//
RtVoid	RiRotate( RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz )
{
	CqMatrix	matRot( degToRad( angle ), CqVector3D( dx, dy, dz ) );
	// Check if this transformation results in a change in orientation.
	//    if ( matRot.Determinant() < 0 && ( QGetRenderContext()->pconCurrent()->Type() != Motion || QGetRenderContext()->pconCurrent()->TimeIndex() == 0 ) )
	//        QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );

	QGetRenderContext() ->ptransConcatCurrentTime( matRot );
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiScale
// Concatenate a scale into the current transformation.
//
RtVoid	RiScale( RtFloat sx, RtFloat sy, RtFloat sz )
{
	CqMatrix	matScale( sx, sy, sz );
	// Check if this transformation results in a change in orientation.
	//    if ( matScale.Determinant() < 0 && ( QGetRenderContext()->pconCurrent()->Type() != Motion || QGetRenderContext()->pconCurrent()->TimeIndex() == 0 ) )
	//        QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );

	QGetRenderContext() ->ptransConcatCurrentTime( matScale );
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiSkew
// Concatenate a skew into the current transformation.
//
RtVoid	RiSkew( RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
               RtFloat dx2, RtFloat dy2, RtFloat dz2 )
{
	CqMatrix	matSkew( degToRad( angle ), dx1, dy1, dz1, dx2, dy2, dz2 );

	// This transformation can not change orientation.

	QGetRenderContext() ->ptransConcatCurrentTime( matSkew );
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiDeformationV
// Specify a deformation shader to be included into the current transformation.
//
RtVoid	RiDeformationV( RtToken name, PARAMETERLIST )
{
	Aqsis::log() << warning << "RiDeformation not supported" << std::endl;
}


//----------------------------------------------------------------------
// RiDisplacementV
// Specify the current displacement shade used by geometric primitives.
//
RtVoid	RiDisplacementV( RtToken name, PARAMETERLIST )
{
	// Find the shader.
	boost::shared_ptr<IqShader> pshadDisplacement = QGetRenderContext() ->CreateShader( name, Type_Displacement );

	if ( pshadDisplacement )
	{
		pshadDisplacement->SetTransform( QGetRenderContext() ->ptransCurrent() );
		// Execute the intiialisation code here, as we now have our shader context complete.
		pshadDisplacement->PrepareDefArgs();
		RtInt i;
		for ( i = 0; i < count; ++i )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			SetShaderArgument( pshadDisplacement, token, static_cast<TqPchar>( value ) );
		}
		const TqInt* pMultipass = QGetRenderContext()->poptCurrent()->GetIntegerOption("Render", "multipass");
		if(pMultipass && !pMultipass[0])
			pshadDisplacement->PrepareShaderForUse();
	}

	QGetRenderContext() ->pattrWriteCurrent() ->SetpshadDisplacement( pshadDisplacement, QGetRenderContext() ->Time() );
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiCoordinateSystem
// Save the current coordinate system as the specified name.
//
RtVoid	RiCoordinateSystem( RtToken space )
{
	// Insert the named coordinate system into the list help on the renderer.
	QGetRenderContext() ->SetCoordSystem( space, QGetRenderContext() ->matCurrent( QGetRenderContext() ->Time() ) );
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// ---Additional to spec. v3.1---
// RiCoordSysTransform
// Replace the current transform with the named space.

RtVoid	RiCoordSysTransform( RtToken space )
{
	// Insert the named coordinate system into the list help on the renderer.
	CqMatrix matSpaceToWorld;
	QGetRenderContext() ->matSpaceToSpace( space, "world", NULL, NULL, QGetRenderContext()->Time(), matSpaceToWorld ); 

	if( QGetRenderContext()->IsWorldBegin() )
	{
		// If we're in the WorldBegin/End block, we need to take the 'camera' transform as the starting point.
		// This is because we have to transform primitives to 'world' coordinates.
		// So we read the 'default object transform' that is stored at 'RiWorldBegin', set the transform at the current time to that, then concat the specified transform.
		CqMatrix matDefTrans( QGetRenderContext()->GetDefObjTransform()->matObjectToWorld(QGetRenderContext()->Time()) );
		QGetRenderContext() ->ptransSetCurrentTime( matDefTrans );
		QGetRenderContext() ->ptransConcatCurrentTime( matSpaceToWorld );
	}
	else
		QGetRenderContext() ->ptransSetCurrentTime( matSpaceToWorld );

	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiTransformPoints
// Transform a list of points from one coordinate system to another.
//
RtPoint*	RiTransformPoints( RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[] )
{
	CqMatrix matCToW;
	if(QGetRenderContext() ->matSpaceToSpace( fromspace,
	                   tospace, NULL, NULL, QGetRenderContextI()->Time(), matCToW ))
	{
		if (matCToW.fIdentity() != true)
		{
			for(TqInt i =0; i< npoints; i++)
			{
				CqVector3D tmp(points[i]);
				tmp = matCToW * tmp;
				points[i][0] = tmp.x();
				points[i][1] = tmp.y();
				points[i][2] = tmp.z();
			}
		}

		return ( points );
	}
	return 0;
}


//----------------------------------------------------------------------
// RiTransformBegin
// Push the current transformation state.
//
RtVoid	RiTransformBegin()
{
	QGetRenderContext() ->BeginTransformModeBlock();
}


//----------------------------------------------------------------------
// RiTransformEnd
// Pop the previous transformation state.
//
RtVoid	RiTransformEnd()
{
	QGetRenderContext() ->EndTransformModeBlock();
}


//----------------------------------------------------------------------
// RiAttributeV
// Set a system specific attribute.
//
RtVoid	RiAttributeV( RtToken name, PARAMETERLIST )
{
	const TqUlong hash = CqString::hash(name);

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

	for ( TqInt i = 0; i < count; ++i )
	{
		RtToken	token = tokens[ i ];
		RtPointer	value = values[ i ];

		CqParameter* pParam = pAttr->pParameter( token );
		if ( pParam == 0 )
		{
			// Search for the parameter in the declarations.
			// Note attributes can only be uniform.
			CqPrimvarToken tok;
			try
			{
				tok = QGetRenderContext()->tokenDict().parseAndLookup(token);
			}
			catch(XqValidation& e)
			{
				Aqsis::log() << error << e.what() << std::endl;
				continue;
			}
			if ( tok.Class() == class_uniform )
			{
				pParam = CqParameter::Create(tok);
				pAttr->AddParameter( pParam );
			}
			else
			{
				Aqsis::log() << warning << "RiAttribute: Attributes can only be uniform [" << tok.name() << "]\n";
				continue;
			}
		}
		EqVariableType Type = pParam->Type();
		bool bArray = pParam->Count() > 1;

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
							static_cast<CqParameterTypedUniform<CqString, type_string, CqString>*>( pParam ) ->pValue() [ j ] = str;
						}
					}
					else
					{
						CqString str( ps[ 0 ] );
						static_cast<CqParameterTypedUniform<CqString, type_string, CqString>*>( pParam ) ->pValue() [ 0 ] = str;
					}
#ifdef REQUIRED
					if( (strcmp(name, "identifier")==0) && (strcmp(token, "name")==0))
						Aqsis::log() << info << "Identifier: " << ps[ 0 ] << std::endl;
#endif

				}
				break;
				// TODO: Rest of parameter types.
			default:
				Aqsis::log() << warning
					<< "RiAttribute \"" << name
					<< "\": unimplemented attribute type [" << token << "]\n";
		}
	}
}


//----------------------------------------------------------------------
// RiPolygonV
// Specify a coplanar, convex polygon.
//
RtVoid	RiPolygonV( RtInt nvertices, PARAMETERLIST )
{
	// Create a new polygon surface primitive.
	boost::shared_ptr<CqSurfacePolygon> pSurface( new CqSurfacePolygon( nvertices ) );

	// Process any specified primitive variables.
	if ( ProcessPrimitiveVariables( pSurface.get(), count, tokens, values ) )
	{
		if ( !pSurface->CheckDegenerate() )
		{
			TqFloat time = QGetRenderContext()->Time();
			// Transform the points into camera space for processing,
			CqMatrix matOtoW, matNOtoW, matVOtoW;
			QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
			QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
			QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
			pSurface->Transform( matOtoW, matNOtoW, matVOtoW);
			CreateGPrim( pSurface );
		}
		else
		{
			Aqsis::log() << error << "Found degenerate polygon" << std::endl;
		}
	}
}


//----------------------------------------------------------------------
// RiGeneralPolygonV
// Specify a nonconvex coplanar polygon.
//
RtVoid	RiGeneralPolygonV( RtInt nloops, RtInt nverts[], PARAMETERLIST )
{
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
		CqVector3D vecTemp = vectorCast<CqVector3D>(pPointsClass->P()->pValue(0)[0]);
		MinX = MaxX = vecTemp.x();
		MinY = MaxY = vecTemp.y();
		MinZ = MaxZ = vecTemp.z();

		// We need to take into account Orientation here.
		bool O = QGetRenderContext()->pattrCurrent() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ] != 0;

		TqUint iVert;
		for ( iVert = 1; iVert < pPointsClass->P() ->Size(); ++iVert )
		{
			vecTemp = vectorCast<CqVector3D>(pPointsClass->P()->pValue(iVert)[0]);
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
}

//----------------------------------------------------------------------
RtVoid RiBlobbyV( RtInt nleaf, RtInt ncode, RtInt code[], RtInt nflt, RtFloat flt[],
                  RtInt nstr, RtString str[], PARAMETERLIST )
{
	TqInt i;

	// Initialize the blobby structure
	CqBlobby blobby(nleaf, ncode, code, nflt, flt, nstr, str);

	// Get back the bounding box in world coordinates
	CqBound Bound;
	blobby.Bound(&Bound);

	// Transform the bounding box into camera coordinates
	CqMatrix matOtoC;
	QGetRenderContext() ->matSpaceToSpace( "object", "camera", NULL, QGetRenderContext() ->ptransCurrent().get(), QGetRenderContext()->Time(), matOtoC );
	Bound.Transform( matOtoC );

	// The bounding-box stops at camera's plane
	TqFloat camera_z = QGetRenderContext() ->poptCurrent() ->GetFloatOption( "System", "Clipping" ) [ 0 ];
	if(Bound.vecMax().z() < camera_z)
		// Blobby's behind the camera
		return;

	if(Bound.vecMin().z() < camera_z)
		// Cut the bounding-box with camera's plane
		Bound = CqBound(CqVector3D(Bound.vecMin().x(), Bound.vecMin().y(), camera_z), Bound.vecMax());

	// Transform the bounding box into raster coordinates
	CqMatrix matCamToRaster;
	QGetRenderContext() ->matSpaceToSpace( "camera", "raster", NULL, QGetRenderContext() ->ptransCurrent().get(), QGetRenderContext()->Time(), matCamToRaster );
	Bound.Transform( matCamToRaster );

	// Get bounding-box size in pixels
	TqInt  pixels_w = static_cast<TqInt> ( Bound.vecCross().x() );
	TqInt  pixels_h = static_cast<TqInt> ( Bound.vecCross().y() );

	// Adjust to shading rate
	// TODO: Blobbies should be CqSurfaces - in that case they could make of
	// the AdjustedShadingRate() function to adjust the shading for DoF and MB
	TqInt shading_rate = max(1, static_cast<TqInt> ( QGetRenderContext() ->pattrCurrent() ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ]));
	pixels_w /= shading_rate;
	pixels_h /= shading_rate;


	// Polygonize this blobby
	TqInt npoints;
	TqInt npolygons;
	TqInt* nvertices = 0;
	TqInt* vertices = 0;
	TqFloat* points = 0;
	TqInt pieces = blobby.polygonize(pixels_w, pixels_h, npoints, npolygons, nvertices, vertices, points);

	Aqsis::log() << info << "Polygonized : " << npoints << " points, " << npolygons << " triangles." << std::endl;

	TqFloat* colors = new TqFloat[3 * npoints];

	std::vector<TqFloat> splits;
	splits.resize(nleaf);

	/* Parameters: RtInt count, RtToken tokens[], RtPointer values[] */
	bool Cs = false;
	for (TqInt c = 0; c < count; c++)
	{
		if (strstr(tokens[c], RI_CS))
		{

			CqVector3D cg;

			for( int i = 0; i < npoints; i++ )
			{
				TqFloat sum;
				TqFloat ocolors[3] = {0.0f,0.0f,0.0f};

				TqInt m = i * 3;
				cg[0] = points[m];
				cg[1] = points[m + 1];
				cg[2] = points[m + 2];

				sum = blobby.implicit_value(cg, nleaf, splits);

				if (sum != 0.0)
				{
					colors[m] = colors[m+1] = colors[m+2] = 0.0f;
					for (TqInt j=0; j < nleaf; j++)
					{
						TqInt l = j * 3;
						colors[m] += splits[j] * ((TqFloat *) *values)[l];
						colors[m+1] += splits[j] * ((TqFloat *) *values)[l+1];
						colors[m+2] += splits[j] * ((TqFloat *) *values)[l+2];
					}
					colors[m]/=sum;
					colors[m+1]/=sum;
					colors[m+2]/=sum;
					ocolors[0] = colors[m];
					ocolors[1] = colors[m+1];
					ocolors[2] = colors[m+2];
				}
				else
				{
					colors[m] = ocolors[0];
					colors[m+1] = ocolors[1];
					colors[m+2] = ocolors[2];
				}

			}
			Cs = true;
			break;
		}
	}

	pieces = min(8, pieces);
	TqInt m;

	if (Cs)
	{
		for (i=0; i < pieces-1; i ++)
		{
			Aqsis::log() << info << "Creating RiPointsPolygons for piece " << i << "[" << pieces-1 << "]" << std::endl;
			m = (i * npolygons)/pieces;
			RiPointsPolygons(npolygons/pieces, nvertices, &vertices[3 * m], RI_P, points, RI_CS, colors, RI_NULL);
			Aqsis::log() << info << "Done creating RiPointsPolygons for piece " << i << std::endl;
		}

		Aqsis::log() << info << "Creating RiPointsPolygons for piece " << (pieces-1) << "[" << pieces-1 << "]" << std::endl;
		m = ((pieces-1) * npolygons) / pieces;
		TqInt nmax = npolygons - m;
		RiPointsPolygons(nmax, nvertices, &vertices[3 * m], RI_P, points, RI_CS, colors, RI_NULL);
		Aqsis::log() << info << "Done creating RiPointsPolygons for piece " << (pieces-1) << std::endl;
	}
	else
	{
		for (i=0; i < pieces-1; i ++)
		{
			Aqsis::log() << info << "Creating RiPointsPolygons for piece " << i << "[" << pieces-1 << "]" << std::endl;
			m = (i * npolygons)/pieces;
			RiPointsPolygons(npolygons/pieces, nvertices, &vertices[3 * m], RI_P, points, RI_NULL);
			Aqsis::log() << info << "Done creating RiPointsPolygons for piece " << i << std::endl;
		}

		Aqsis::log() << info << "Creating RiPointsPolygons for piece " << (pieces-1) << "[" << pieces-1 << "]" << std::endl;
		m = ((pieces-1) * npolygons) / pieces;
		TqInt nmax = npolygons - m;
		RiPointsPolygons(nmax, nvertices, &vertices[3 * m], RI_P, points, RI_NULL);
		Aqsis::log() << info << "Done creating RiPointsPolygons for piece " << (pieces-1) << std::endl;
	}

	Aqsis::log() << info << "Created RiPointsPolygons for Blobby" << std::endl;

	delete[] nvertices;
	delete[] vertices;
	delete[] points;
	delete[] colors;
}


//----------------------------------------------------------------------
/** Specify a small Points primitives
 *
 *\return	nothing
 **/
RtVoid	RiPointsV( RtInt npoints, PARAMETERLIST )
{
	// Create a storage class for all the points.
	boost::shared_ptr<CqPolygonPoints> pPointsClass( new CqPolygonPoints( npoints, 1, npoints ) );

	// Create a new points storage class
	boost::shared_ptr<CqPoints> pSurface;

	// read in the parameter list
	if ( ProcessPrimitiveVariables( pPointsClass.get(), count, tokens, values ) )
	{
		// Transform the points into camera space for processing,
		// This needs to be done before initialising the KDTree as the tree must be formulated in 'current' (camera) space.
		CqMatrix matOtoW, matNOtoW, matVOtoW;
		QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pPointsClass->pTransform().get(), pPointsClass->pTransform() ->Time(0), matOtoW );
		QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pPointsClass->pTransform().get(), pPointsClass->pTransform() ->Time(0), matNOtoW );
		QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pPointsClass->pTransform().get(), pPointsClass->pTransform() ->Time(0), matVOtoW );
		pPointsClass->Transform( matOtoW, matNOtoW, matVOtoW);

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
RtVoid RiCurvesV( RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap, PARAMETERLIST )
{
	// find out whether the curve is periodic or non-periodic
	bool periodic = false;
	if ( strcmp( wrap, RI_PERIODIC ) == 0 )
	{
		periodic = true;
	}
	else if ( strcmp( wrap, RI_NONPERIODIC ) == 0 )
	{
		periodic = false;
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
			CqMatrix matOtoW, matNOtoW, matVOtoW;
			QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
			QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
			QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
			pSurface->Transform( matOtoW, matNOtoW, matVOtoW);

			CreateGPrim( pSurface );
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
			CqMatrix matOtoW, matNOtoW, matVOtoW;
			QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
			QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
			QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
			pSurface->Transform( matOtoW, matNOtoW, matVOtoW);
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
// RiPointsPolygonsV
// Specify a list of convex coplanar polygons and their shared vertices.
//
RtVoid	RiPointsPolygonsV( RtInt npolys, RtInt nverts[], RtInt verts[], PARAMETERLIST )
{
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
			cVerts = max( ( ( *pVerts ) + 1 ), cVerts );
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
		CqMatrix matOtoW, matNOtoW, matVOtoW;
		QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pPointsClass->pTransform().get(), time, matOtoW );
		QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pPointsClass->pTransform().get(), time, matNOtoW );
		QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pPointsClass->pTransform().get(), time, matVOtoW );
		pPointsClass->Transform( matOtoW, matNOtoW, matVOtoW);
		CreateGPrim(pPsPs);
	}
}


//----------------------------------------------------------------------
// RiPointsGeneralPolygonsV
// Specify a list of coplanar, non-convex polygons and their shared vertices.
//
RtVoid	RiPointsGeneralPolygonsV( RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], PARAMETERLIST )
{
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
				cVerts = max( ( ( *pVerts ) + 1 ), cVerts );
				pVerts++;
			}
		}
	}

	// We need to take into account Orientation here.
	bool O = QGetRenderContext()->pattrCurrent() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ] != 0;

	// Create a storage class for all the points.
	boost::shared_ptr<CqPolygonPoints> pPointsClass( new CqPolygonPoints( cVerts, npolys, sumnVerts ) );
	// Process any specified primitive variables
	if ( ProcessPrimitiveVariables( pPointsClass.get(), count, tokens, values ) )
	{
		pPointsClass->SetDefaultPrimitiveVariables( RI_FALSE );

		// A list of modified values.  We can't modify values in-place, since
		// then the function calling this one will loose track of the
		// associated memory.  Instead we take a copy here, which will let us
		// modify it as necessary.
		std::vector<RtPointer> modifiedValues(values, values+count);

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
				iminindex = min( iminindex, (TqUint) verts[ igvert ] );
				imaxindex = max( imaxindex, (TqUint) verts[ igvert ] );

				CqPolygonGeneral2D polya;
				polya.SetpVertices( pPointsClass );
				TqInt ivert;
				for ( ivert = 0; ivert < nverts[ igloop ]; ++ivert, ++igvert )
				{
					ipoint = verts[ igvert ];
					assert( ipoint < pPointsClass->P() ->Size() );
					polya.aiVertices().push_back( ipoint );
				}
				// Work out which plane to project onto.
				polya.CalcAxis();

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
				bool found = false;
				for( TqUint iv = initial_index; iv != igvert; ++iv )
				{
					if( verts[ iv ] == ivaryingindex )
					{
						aFVList.push_back( iv );
						found = true;
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
		assert( static_cast<TqInt>(aFVList.size()) == fvcount );
		std::vector<void*> aNewParams;
		for( iUserParam = 0; iUserParam < count; ++iUserParam )
		{
			CqPrimvarToken tok = QGetRenderContext()->tokenDict().parseAndLookup(tokens[iUserParam]);
			TqInt elem_size = 0;
			switch( tok.type() )
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
			elem_size *= tok.count();

			if( tok.Class() == class_facevarying || tok.Class() == class_facevertex )
			{
				char* pNew = static_cast<char*>( malloc( elem_size * fvcount) );
				aNewParams.push_back( pNew );
				TqInt iElem;
				for( iElem = 0; iElem < fvcount; ++iElem )
				{
					const unsigned char* pval = static_cast<const unsigned char*>( modifiedValues[ iUserParam ] ) + ( aFVList[ iElem ] * elem_size );
					memcpy( pNew, pval, ( elem_size ));
					pNew += elem_size;
				}
				modifiedValues[ iUserParam ] = aNewParams.back();
			}
			else if( tok.Class() == class_uniform )
			{
				// Allocate enough for 1 value per triangle, then duplicate values from the original list
				// accordingly.
				char* pNew = static_cast<char*>( malloc( elem_size * ctris ) );
				aNewParams.push_back( pNew );
				TqInt iElem;
				const unsigned char* pval = static_cast<const unsigned char*>( modifiedValues[ iUserParam ] );
				for( iElem = 0; iElem < npolys; ++iElem )
				{
					TqInt dup_count = aUVList[ iElem ];
					TqInt dup;
					for(dup=0; dup < dup_count; dup++)
					{
						memcpy( pNew, pval, ( elem_size ));
						pNew += elem_size;
					}
					pval += elem_size;
				}
				modifiedValues[ iUserParam ] = aNewParams.back();
			}
		}

		RiPointsPolygonsV( ctris, &_nverts[ 0 ], &aiTriangles[ 0 ], count, tokens, &(modifiedValues[0]) );

		std::vector<void*>::iterator iNewParam;
		for( iNewParam = aNewParams.begin(); iNewParam != aNewParams.end(); ++iNewParam )
			free( *iNewParam );
	}
}


//----------------------------------------------------------------------
// RiBasis
// Specify the patch basis matrices for the u and v directions, and the knot skip values.
//
RtVoid	RiBasis( RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep )
{
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
	u.SetfIdentity( false );
	v.SetfIdentity( false );

	QGetRenderContext() ->pattrWriteCurrent() ->GetMatrixAttributeWrite( "System", "Basis" ) [ 0 ] = u;
	QGetRenderContext() ->pattrWriteCurrent() ->GetMatrixAttributeWrite( "System", "Basis" ) [ 1 ] = v;
	QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "BasisStep" ) [ 0 ] = ustep;
	QGetRenderContext() ->pattrWriteCurrent() ->GetIntegerAttributeWrite( "System", "BasisStep" ) [ 1 ] = vstep;
	QGetRenderContext() ->AdvanceTime();
}


//----------------------------------------------------------------------
// RiPatchV
// Specify a new patch primitive.
//
RtVoid	RiPatchV( RtToken type, PARAMETERLIST )
{
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
			CqMatrix matOtoW, matNOtoW, matVOtoW;
			QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
			QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
			QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
			pSurface->Transform( matOtoW, matNOtoW, matVOtoW);

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
			CqMatrix matOtoW, matNOtoW, matVOtoW;
			QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
			QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
			QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
			pSurface->Transform( matOtoW, matNOtoW, matVOtoW);
			CreateGPrim( pSurface );
		}
	}
	else
	{
		Aqsis::log() << error << "RiPatch invalid patch type \"" << type << "\"" << std::endl;
	}
}


//----------------------------------------------------------------------
// RiPatchMeshV
// Specify a quadrilaterla mesh of patches.
//

RtVoid	RiPatchMeshV( RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, PARAMETERLIST )
{
	if( strcmp( uwrap, RI_PERIODIC ) && strcmp( uwrap, RI_NONPERIODIC ) )
		Aqsis::log() << error << "RiPatchMesh invalid u-wrap type: \"" << uwrap << "\"" << std::endl;

	if( strcmp( vwrap, RI_PERIODIC ) && strcmp( vwrap, RI_NONPERIODIC ) )
		Aqsis::log() << error << "RiPatchMesh invalid v-wrap type: \"" << vwrap << "\"" << std::endl;

	if ( strcmp( type, RI_BICUBIC ) == 0 )
	{
		// Create a surface patch
		bool	uPeriodic = ( strcmp( uwrap, RI_PERIODIC ) == 0 ) ? true : false;
		bool	vPeriodic = ( strcmp( vwrap, RI_PERIODIC ) == 0 ) ? true : false;

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
				CqMatrix matOtoW, matNOtoW, matVOtoW;
				QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
				QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
				QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
				(*iSS)->Transform( matOtoW, matNOtoW, matVOtoW);
				CreateGPrim( *iSS );
			}
		}
	}
	else if ( strcmp( type, RI_BILINEAR ) == 0 )
	{
		// Create a surface patch
		bool	uPeriodic = ( strcmp( uwrap, RI_PERIODIC ) == 0 ) ? true : false;
		bool	vPeriodic = ( strcmp( vwrap, RI_PERIODIC ) == 0 ) ? true : false;

		boost::shared_ptr<CqSurfacePatchMeshBilinear> pSurface( new CqSurfacePatchMeshBilinear( nu, nv, uPeriodic, vPeriodic ) );
		// Fill in primitive variables specified.
		if ( ProcessPrimitiveVariables( pSurface.get(), count, tokens, values ) )
		{
			// Fill in default values for all primitive variables not explicitly specified.
			pSurface->SetDefaultPrimitiveVariables();
			TqFloat time = QGetRenderContext()->Time();
			// Transform the points into camera space for processing,
			CqMatrix matOtoW, matNOtoW, matVOtoW;
			QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
			QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
			QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
			pSurface->Transform( matOtoW, matNOtoW, matVOtoW);
			CreateGPrim( pSurface );
		}
	}
	else
	{
		Aqsis::log() << error << "RiPatchMesh invalid type \"" << type << "\"" << std::endl;
	}
}


//----------------------------------------------------------------------
// RiNuPatchV
// Specify a new non uniform patch.
//
RtVoid	RiNuPatchV( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax,
                   RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, PARAMETERLIST )
{
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
		CqMatrix matOtoW, matNOtoW, matVOtoW;
		QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
		QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
		QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
		pSurface->Transform( matOtoW, matNOtoW, matVOtoW);
		CreateGPrim( pSurface );
	}
}

//----------------------------------------------------------------------
// RiTrimCurve
// Specify curves which are used to trim NURBS surfaces.
//
RtVoid	RiTrimCurve( RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[] )
{
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
}


//----------------------------------------------------------------------
// RiSphereV
// Specify a sphere primitive.
//
RtVoid	RiSphereV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST )
{
	// Create a sphere
	boost::shared_ptr<CqSphere> pSurface( new CqSphere( radius, zmin, zmax, 0, thetamax ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	CqMatrix matOtoW, matNOtoW, matVOtoW;
	QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
	QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
	QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
	pSurface->Transform( matOtoW, matNOtoW, matVOtoW);
	CreateGPrim( pSurface );
}


//----------------------------------------------------------------------
// RiConeV
// Specify a cone primitive.
//
RtVoid	RiConeV( RtFloat height, RtFloat radius, RtFloat thetamax, PARAMETERLIST )
{
	/// \note This should be an exception and get caught further up.
	if( thetamax == 0 )
		return;

	// Create a cone
	boost::shared_ptr<CqCone> pSurface( new CqCone( height, radius, 0, thetamax, 0, 1.0f ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	CqMatrix matOtoW, matNOtoW, matVOtoW;
	QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
	QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
	QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
	pSurface->Transform( matOtoW, matNOtoW, matVOtoW);
	CreateGPrim( pSurface );
}


//----------------------------------------------------------------------
// RiCylinderV
// Specify a culinder primitive.
//
RtVoid	RiCylinderV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST )
{
	// Create a cylinder
	boost::shared_ptr<CqCylinder> pSurface( new CqCylinder( radius, zmin, zmax, 0, thetamax ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	CqMatrix matOtoW, matNOtoW, matVOtoW;
	QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
	QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
	QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
	pSurface->Transform( matOtoW, matNOtoW, matVOtoW);
	CreateGPrim( pSurface );
}


//----------------------------------------------------------------------
// RiHyperboloidV
// Specify a hyperboloid primitive.
//
RtVoid	RiHyperboloidV( RtPoint point1, RtPoint point2, RtFloat thetamax, PARAMETERLIST )
{
	// Create a hyperboloid
	CqVector3D v0( point1[ 0 ], point1[ 1 ], point1[ 2 ] );
	CqVector3D v1( point2[ 0 ], point2[ 1 ], point2[ 2 ] );
	boost::shared_ptr<CqHyperboloid> pSurface( new CqHyperboloid( v0, v1, 0, thetamax ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	CqMatrix matOtoW, matNOtoW, matVOtoW;
	QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
	QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
	QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
	pSurface->Transform( matOtoW, matNOtoW, matVOtoW);
	CreateGPrim( pSurface );
}


//----------------------------------------------------------------------
// RiParaboloidV
// Specify a paraboloid primitive.
//
RtVoid	RiParaboloidV( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST )
{
	// Create a paraboloid
	boost::shared_ptr<CqParaboloid> pSurface( new CqParaboloid( rmax, zmin, zmax, 0, thetamax ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	CqMatrix matOtoW, matNOtoW, matVOtoW;
	QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
	QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
	QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
	pSurface->Transform( matOtoW, matNOtoW, matVOtoW);
	CreateGPrim( pSurface );
}


//----------------------------------------------------------------------
// RiDiskV
// Specify a disk primitive.
//
RtVoid	RiDiskV( RtFloat height, RtFloat radius, RtFloat thetamax, PARAMETERLIST )
{
	// Create a disk
	boost::shared_ptr<CqDisk> pSurface( new CqDisk( height, 0, radius, 0, thetamax ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	CqMatrix matOtoW, matNOtoW, matVOtoW;
	QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
	QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
	QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
	pSurface->Transform( matOtoW, matNOtoW, matVOtoW);

	CreateGPrim( pSurface );
}


//----------------------------------------------------------------------
// RiTorus
// Specify a torus primitive.
//
RtVoid	RiTorusV( RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, PARAMETERLIST )
{
	// Create a torus
	boost::shared_ptr<CqTorus> pSurface( new CqTorus( majorrad, minorrad, phimin, phimax, 0, thetamax ) );
	ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
	pSurface->SetDefaultPrimitiveVariables();

	TqFloat time = QGetRenderContext()->Time();
	// Transform the points into camera space for processing,
	CqMatrix matOtoW, matNOtoW, matVOtoW;
	QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
	QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
	QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
	pSurface->Transform( matOtoW, matNOtoW, matVOtoW);

	CreateGPrim( pSurface );
}


//----------------------------------------------------------------------
// RiProcedural
// Implement the procedural type primitive.
//
RtVoid	RiProcedural( RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc )
{
	CqBound B(bound);

	//printf("bound(%f %f %f %f %f %f)\n", bound[0], bound[1], bound[2], bound[3], bound[4], bound[5]);

	// I suspect that in order to handle the RtFreeProc correctly that we need to reference count
	// the instances of CqProcedural so that FreeProc gets called on the final Release();

	boost::shared_ptr<CqProcedural> pProc( new CqProcedural(data, B, refineproc, freeproc ) );
	TqFloat time = QGetRenderContext()->Time();
	CqMatrix matOtoW, matNOtoW, matVOtoW;
	QGetRenderContext()->matSpaceToSpace( "object", "world", NULL, pProc->pTransform().get(), time, matOtoW );
	QGetRenderContext()->matNSpaceToSpace( "object", "world", NULL, pProc->pTransform().get(), time, matNOtoW );
	QGetRenderContext()->matVSpaceToSpace( "object", "world", NULL, pProc->pTransform().get(), time, matVOtoW );
	pProc->Transform( matOtoW, matNOtoW, matVOtoW);
	CreateGPrim( pProc );
}



//----------------------------------------------------------------------
// RiGeometryV
// Specify a special primitive.
//
RtVoid	RiGeometryV( RtToken type, PARAMETERLIST )
{
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
			CqMatrix matOtoW, matNOtoW, matVOtoW;
			QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
			QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
			QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
			pMesh->Transform( matOtoW, matNOtoW, matVOtoW);

			CreateGPrim( boost::static_pointer_cast<CqSurface>( pMesh ) );
		}
	}
	else if ( strcmp( type, "sphere" ) == 0 )
	{
		// \todo <b>Code Review</b> Do we really need this when we have RiSphere?
		// Create a sphere
		boost::shared_ptr<CqSphere> pSurface( new CqSphere( 1, -1, 1, 0, 360.0 ) );
		ProcessPrimitiveVariables( pSurface.get(), count, tokens, values );
		pSurface->SetDefaultPrimitiveVariables();

		TqFloat time = QGetRenderContext()->Time();
		// Transform the points into camera space for processing,
		CqMatrix matOtoW, matNOtoW, matVOtoW;
		QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matOtoW );
		QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matNOtoW );
		QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pSurface->pTransform().get(), time, matVOtoW );
		pSurface->Transform( matOtoW, matNOtoW, matVOtoW);

		CreateGPrim( pSurface );
	} else if ( strcmp( type, "bunny" ) == 0 )
	{
	CqBunny bunny;

	std::vector<RtToken> aTokens;
	std::vector<RtPointer> aValues;
	std::vector<RtToken> aTags;
	aTokens.clear();
	aValues.clear();
	aTags.clear();


	aTokens.push_back(RI_P);
	aTokens.push_back(RI_S);
	aTokens.push_back(RI_T);
	aValues.push_back(bunny.Points());
	aValues.push_back(bunny.S());
	aValues.push_back(bunny.T());
	aTags.push_back(tokenCast("catmull-clark"));
	aTags.push_back(tokenCast("interpolateboundary"));

	static TqInt params[] = { 0, 0 };

	TqInt count = 3;
	// Rotate and scale to match the teapot geometry
	RiAttributeBegin();
	RiTranslate(0.0, 0.0, 2.5);
	RiRotate(90.0, 1.0, 0.0, 0.0);
	RiScale(1.0/30.0, 1.0/30.0, 1.0/30.0);
	RiSubdivisionMeshV( aTags[0],
	                    bunny.NFaces(),
	                    bunny.Faces(),
	                    bunny.Indexes(),
	                    1,
	                    &aTags[1],
	                    params,
	                    0,
	                    0,
	                    AQSIS_PASS_RI_PARAMETERS );
	RiAttributeEnd();

	} else
	{
		Aqsis::log() << warning << "RiGeometry unrecognised type \"" << type << "\"" << std::endl;
	}
}

//----------------------------------------------------------------------
// RiSolidBegin
// Begin the definition of a CSG object.
//
RtVoid	RiSolidBegin( RtToken type )
{
	CqString strType( type );
	QGetRenderContext() ->BeginSolidModeBlock( strType );
}


//----------------------------------------------------------------------
// RiSolidEnd
// End the definition of a CSG object.
//
RtVoid	RiSolidEnd()
{
	QGetRenderContext() ->EndSolidModeBlock();
}


//----------------------------------------------------------------------
// RiObjectBegin
// Begin the definition of a stored object for use by RiObjectInstance.
//
RtObjectHandle	RiObjectBegin()
{
	QGetRenderContext() ->BeginObjectModeBlock();
	RtObjectHandle objectHandle = static_cast<RtObjectHandle>(QGetRenderContext() ->OpenNewObjectInstance());

	return objectHandle;
}


//----------------------------------------------------------------------
// RiObjectEnd
// End the defintion of a stored object for use by RiObjectInstance.
//
RtVoid	RiObjectEnd()
{
	QGetRenderContext() ->EndObjectModeBlock();
	QGetRenderContext() ->CloseObjectInstance();
}


//----------------------------------------------------------------------
// RiObjectInstance
// Instantiate a copt of a pre-stored geometric object.
//
RtVoid	RiObjectInstance( RtObjectHandle handle )
{
	QGetRenderContext() ->InstantiateObject( reinterpret_cast<CqObjectInstance*>( handle ) );
}


//----------------------------------------------------------------------
// RiBeginMotionV
// Begin the definition of the motion of an object for use by motion blur.
//
RtVoid	RiMotionBeginV( RtInt N, RtFloat times[] )
{
	QGetRenderContext() ->BeginMotionModeBlock( N, times );
}


//----------------------------------------------------------------------
// RiMotionEnd
// End the definition of the motion of an object.
//
RtVoid	RiMotionEnd()
{
	QGetRenderContext() ->EndMotionModeBlock();
}


//----------------------------------------------------------------------
// RiMakeTextureV
// Convert a picture to a texture.
//
RtVoid	RiMakeTextureV( RtString imagefile, RtString texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST )
{
	AQSIS_TIME_SCOPE(Make_texture);

	SqWrapModes wrapModes(enumCast<EqWrapMode>(swrap), enumCast<EqWrapMode>(twrap));
	boost::filesystem::path inFileName
		= QGetRenderContext()->poptCurrent()->findRiFile(imagefile, "texture");
	makeTexture(inFileName, texturefile, SqFilterInfo(filterfunc, swidth, twidth),
			wrapModes, CqRiParamList(tokens, values, count));
}


//----------------------------------------------------------------------
// RiMakeBumpV
// Convert a picture to a bump map.
//
RtVoid	RiMakeBumpV( RtString imagefile, RtString bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST )
{
	Aqsis::log() << warning << "RiMakeBump not supported" << std::endl;
}


//----------------------------------------------------------------------
// RiMakeLatLongEnvironmentV
// Convert a picture to an environment map.
//
RtVoid	RiMakeLatLongEnvironmentV( RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST )
{
	assert(imagefile != 0 && reflfile != 0 && filterfunc != 0);

	AQSIS_TIME_SCOPE(Make_texture);

	boost::filesystem::path inFileName = QGetRenderContext()->poptCurrent()
		->findRiFile(imagefile, "texture");
	makeLatLongEnvironment(inFileName, reflfile, SqFilterInfo(filterfunc,
				swidth, twidth), CqRiParamList(tokens, values, count));
}


//----------------------------------------------------------------------
// RiMakeCubeFaceEnvironment
// Convert a picture to a cubical environment map.
//
RtVoid	RiMakeCubeFaceEnvironmentV( RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST )
{
	assert( px != 0 && nx != 0 && py != 0 && ny != 0 && pz != 0 && nz != 0 &&
	        reflfile != 0 && filterfunc != 0 );




	AQSIS_TIME_SCOPE(Make_texture);

	const IqOptions& opts = *QGetRenderContext()->poptCurrent();

	makeCubeFaceEnvironment(
		opts.findRiFile(px, "texture"), opts.findRiFile(nx, "texture"),
		opts.findRiFile(py, "texture"), opts.findRiFile(ny, "texture"),
		opts.findRiFile(pz, "texture"), opts.findRiFile(nz, "texture"),
		reflfile, fov, SqFilterInfo(filterfunc, swidth, twidth),
		CqRiParamList(tokens, values, count)
	);
}


//----------------------------------------------------------------------
// RiMakeShadowV
// Convert a depth map file to a shadow map.
//
RtVoid	RiMakeShadowV( RtString picfile, RtString shadowfile, PARAMETERLIST )
{
	assert(picfile != 0 && shadowfile != 0);

	AQSIS_TIME_SCOPE(Make_texture);

	boost::filesystem::path inFileName = QGetRenderContext()->poptCurrent()
		->findRiFile(picfile, "texture");
	makeShadow(inFileName, shadowfile, CqRiParamList(tokens, values, count));
}


//----------------------------------------------------------------------
// RiMakeOcclusionV
// Convert a series of depth maps to an occlusion map.
//
RtVoid	RiMakeOcclusionV( RtInt npics, RtString picfiles[], RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[] )
{
	AQSIS_TIME_SCOPE(Make_texture);

	std::vector<boost::filesystem::path> fileNames;
	fileNames.reserve(npics);
	for(TqInt i = 0; i < npics; ++i)
	{
		fileNames.push_back( QGetRenderContext()->poptCurrent()
			->findRiFile(picfiles[i], "texture") );
	}
	makeOcclusion(fileNames, shadowfile, CqRiParamList(tokens, values, count));
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
	IfOk = true;
}

//----------------------------------------------------------------------
// RiErrorHandler
// Set the function used to report errors.
//
RtVoid	RiErrorHandler( RtErrorFunc handler )
{
	QGetRenderContext()->SetpErrorHandler( handler );
}


//----------------------------------------------------------------------
// Standard Error Handlers

/** RiErrorIgnore
 * Standard error handler where the error is ignored and no diagnostic
 * message is generated
 * \param code indicates the type of error
 * \param severity indicates how serious the error is
 * \param message is a character string containing an error message
 */
RtVoid	RiErrorIgnore( RtInt code, RtInt severity, RtString message )
{
	RiLastError = code;
}

/** RiErrorPrint
 * Standard error handler where a diagnostic message is generated for the error. The
 * rendering system will attempt to ignore this erroneous information and continue
 * rendering
 * \param code indicates the type of error
 * \param severity indicates how serious the error is
 * \param message is a character string containing an error message
 */
RtVoid	RiErrorPrint( RtInt code, RtInt severity, RtString message )
{
	// Store the code in RiLastError
	RiErrorIgnore(code, severity, message);
	
	// Print the error message
	Aqsis::log() << error << "(" << code << ", " << severity << ") " << message << "\n";
}

/** RiErrorPrint
 * Standard error handler where the first error will cause a diagnostic message to be
 * generated and the rendering system will immediately terminate
 * \param code indicates the type of error
 * \param severity indicates how serious the error is
 * \param message is a character string containing an error message
 */
RtVoid	RiErrorAbort( RtInt code, RtInt severity, RtString message )
{
	// Store the code in RiLastError &  Print the error message
	RiErrorPrint(code, severity, message);
	
	// Abort the engine
	std::exit(code);
}

//----------------------------------------------------------------------
// RiSubdivisionMeshV
// Specify a subdivision surface hull with tagging.
//
RtVoid	RiSubdivisionMeshV( RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], PARAMETERLIST )
{
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
			cVerts = max( ( ( *pVerts ) + 1 ), cVerts );
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
			TqFloat time = QGetRenderContext()->Time();
			CqMatrix matOtoW, matNOtoW, matVOtoW;
			QGetRenderContext() ->matSpaceToSpace( "object", "world", NULL, pPointsClass->pTransform().get(), time, matOtoW );
			QGetRenderContext() ->matNSpaceToSpace( "object", "world", NULL, pPointsClass->pTransform().get(), time, matNOtoW );
			QGetRenderContext() ->matVSpaceToSpace( "object", "world", NULL, pPointsClass->pTransform().get(), time, matVOtoW );
			pPointsClass->Transform( matOtoW, matNOtoW, matVOtoW);

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
						pSubd2->SetInterpolateBoundary( true );
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
}


RtVoid	RiReadArchiveV( RtToken name, RtArchiveCallback callback, PARAMETERLIST )
{
	// Open the archive file
	boost::filesystem::ifstream archiveFile(
			QGetRenderContext()->poptCurrent()->findRiFile(name, "archive"),
			std::ios::binary);
	// Construct a callback functor
	CqRenderer::TqArchiveRecordCallback commentCallback;
	if(callback)
	{
		// Ick, cast to the desired callback type, which differs only from
		// RtArchiveCallback by constness of the two string params.  The
		// alternative is to provide a wrapper class and use const_cast.
		typedef void (*DesiredCallbackType)(const char*, const char*, ...);
		commentCallback = reinterpret_cast<DesiredCallbackType>(callback);
	}
	// Parse the archive
	QGetRenderContext()->parseRibStream(archiveFile, name, commentCallback);
}


RtVoid	RiArchiveRecord( RtToken type, char * format, ... )
{
}

RtContextHandle	RiGetContext( void )
{
	return 0;
}

RtVoid	RiContext( RtContextHandle handle )
{
}

RtVoid	RiClippingPlane( RtFloat x, RtFloat y, RtFloat z, RtFloat nx, RtFloat ny, RtFloat nz )
{
	Aqsis::log() << warning << "Ignoring unimplemented interface call: RiClippingPlane\n";
}


//----------------------------------------------------------------------
// RiResourceV
//
RtVoid	RiResourceV( RtToken handle, RtToken type, PARAMETERLIST )
{
	return;
}


//----------------------------------------------------------------------
// RiResourceBegin
//
RtVoid	RiResourceBegin()
{
}


//----------------------------------------------------------------------
// RiResourceEnd
//
RtVoid	RiResourceEnd()
{
}


RtVoid RiShaderLayerV( RtToken type, RtToken name, RtToken layername, RtInt count, RtToken tokens[], RtPointer values[] )
{
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
			layeredshader->SetTransform( QGetRenderContext() ->ptransCurrent() );
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
			layeredshader->SetTransform( QGetRenderContext() ->ptransCurrent() );
			QGetRenderContext() ->pattrWriteCurrent() ->SetpshadDisplacement( layeredshader, QGetRenderContext() ->Time() );
		}
	}
	else if(stringtype.compare("imager")==0)
	{
		newlayer = QGetRenderContext()->CreateShader( name, Type_Imager );
		layeredshader = QGetRenderContext()->poptCurrent()->pshadImager();

		if( !layeredshader || !layeredshader->IsLayered() )
		{
			// Create a new layered shader and add this shader to it.
			layeredshader = boost::shared_ptr<IqShader>(new CqLayeredShader);
			layeredshader->SetTransform( QGetRenderContext() ->ptransCurrent() );
			QGetRenderContext() ->poptWriteCurrent()->SetpshadImager( layeredshader );
		}
	}
	else
		Aqsis::log() << error << "Layered shaders not supported for type \"" << type << "\"" << std::endl;

	if ( newlayer && layeredshader )
	{
		newlayer->SetTransform( QGetRenderContext() ->ptransCurrent() );

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
		const TqInt* pMultipass = QGetRenderContext()->poptCurrent()->GetIntegerOption("Render", "multipass");
		if(pMultipass && !pMultipass[0])
			newlayer->PrepareShaderForUse();
	}
}

RtVoid RiConnectShaderLayers( RtToken type, RtToken layer1, RtToken variable1, RtToken layer2, RtToken variable2 )
{
	// If the current shader for the specified type is a layer container, add this connection to it
	CqString stringtype(type);
	stringtype = stringtype.ToLower();
	boost::shared_ptr<IqShader> pcurr;
	if(stringtype.compare("surface")==0)
		pcurr = QGetRenderContext()->pattrWriteCurrent()->pshadSurface(QGetRenderContext()->Time());
	else if(stringtype.compare("displacement")==0)
		pcurr = QGetRenderContext()->pattrWriteCurrent()->pshadDisplacement(QGetRenderContext()->Time());
	else if(stringtype.compare("imager")==0)
		pcurr = QGetRenderContext()->poptCurrent()->pshadImager();
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

		CqPrimvarToken tok;
		try
		{
			tok = QGetRenderContext()->tokenDict().parseAndLookup(token);
		}
		catch(XqValidation& e)
		{
			Aqsis::log() << error << e.what() << "\n";
			continue;
		}

		const TqUlong hash = CqString::hash(tok.name().c_str());
		if ( (hash == RIH_P) && (tok.Class() == class_vertex ))
		{
			fP = RIL_P;
			pPoints = ( RtFloat* ) value;
		}
		else if ( (hash == RIH_PZ) && (tok.Class() == class_vertex ) )
		{
			fP = RIL_Pz;
			pPoints = ( RtFloat* ) value;
		}
		else if ( (hash == RIH_PW) && (tok.Class() == class_vertex ) )
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
		CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D>* P
			= new CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D>("P", 1);
		TqInt cVertex = pSurface->cVertex();
		P->SetSize(cVertex);
		switch ( fP )
		{
			case RIL_P:
				for (TqInt i = 0; i < cVertex; ++i )
					P->pValue( i )[0] = CqVector4D( pPoints[ ( i * 3 ) ], pPoints[ ( i * 3 ) + 1 ], pPoints[ ( i * 3 ) + 2 ] );
				break;

			case RIL_Pz:
				for (TqInt i = 0; i < cVertex; ++i )
				{
					CqVector3D vecP = pSurface->SurfaceParametersAtVertex( i );
					vecP.z( pPoints[ i ] );
					P->pValue( i )[0] = vectorCast<CqVector4D>(vecP);
				}
				break;

			case RIL_Pw:
				for (TqInt i = 0; i < cVertex; ++i )
					P->pValue( i )[0] = CqVector4D( pPoints[ ( i * 4 ) ], pPoints[ ( i * 4 ) + 1 ], pPoints[ ( i * 4 ) + 2 ], pPoints[ ( i * 4 ) + 3 ] );
				break;
		}
		pSurface->AddPrimitiveVariable(P);
	}

	// Now process any user defined paramter variables.
	if ( aUserParams.size() > 0 )
	{
		std::vector<TqInt>::iterator iUserParam;
		for ( iUserParam = aUserParams.begin(); iUserParam != aUserParams.end(); ++iUserParam )
		{
			CqPrimvarToken tok = QGetRenderContext()->tokenDict().parseAndLookup(tokens[*iUserParam]);

			CqParameter* pNewParam = CqParameter::Create(tok);
			// Now go across all values and fill in the parameter variable.
			TqInt cValues = 1;
			switch ( tok.Class() )
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

				case class_facevertex:
					cValues = pSurface->cFaceVertex();
					break;

				default:
					break;
			}
			pNewParam->SetSize( cValues );

			TqInt i;
			switch ( tok.type() )
			{
					case type_float:
					{
						CqParameterTyped<TqFloat, TqFloat>* pFloatParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pNewParam );
						TqFloat* pValue = reinterpret_cast<TqFloat*>( values[ *iUserParam ] );
						TqInt iArrayIndex, iValIndex;
						i = 0;
						for ( iValIndex = 0; iValIndex < cValues; ++iValIndex )
							for ( iArrayIndex = 0; iArrayIndex < tok.count(); ++iArrayIndex, ++i )
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
							for ( iArrayIndex = 0; iArrayIndex < tok.count(); ++iArrayIndex, ++i )
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
							for ( iArrayIndex = 0; iArrayIndex < tok.count(); ++iArrayIndex, ++i )
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
							for ( iArrayIndex = 0; iArrayIndex < tok.count(); ++iArrayIndex, ++i )
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
							for ( iArrayIndex = 0; iArrayIndex < tok.count(); ++iArrayIndex, ++i )
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
							for ( iArrayIndex = 0; iArrayIndex < tok.count(); ++iArrayIndex, ++i )
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
							for ( iArrayIndex = 0; iArrayIndex < tok.count(); ++iArrayIndex, ++i )
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
		QGetRenderContext()->StorePrimitive( pSurface );
		STATS_INC( GPR_created );

		// Add to the raytracer database also
		if(QGetRenderContext()->pRaytracer())
			QGetRenderContext()->pRaytracer()->AddPrimitive(pSurface);
	}
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
		return ( true );
	}
	return false;
}


//----------------------------------------------------------------------
/** Set the function used to report progress.
 *	\param	handler	Pointer to the new function to use.
 */
RtVoid	RiProgressHandler( RtProgressFunc handler )
{
	QGetRenderContext()->SetpProgressHandler( handler );
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
	CqPrimvarToken tok;
	try
	{
		tok = QGetRenderContext()->tokenDict().parseAndLookup(name);
	}
	catch(XqValidation& e)
	{
		Aqsis::log() << error << e.what() << std::endl;
		return;
	}

	pShader->SetArgument( tok.name(), tok.type(), "", val );
}

