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
		\brief Implement the majority of the RenderMan API functions.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<stdarg.h>
#include	<math.h>

#include	"aqsis.h"
#include	"imagebuffer.h"
#include	"lights.h"
#include	"renderer.h"
#include	"patch.h"
#include	"polygon.h"
#include	"nurbs.h"
#include	"subdivision.h"
#include	"symbols.h"
#include	"bilinear.h"
#include	"quadrics.h"
#include	"teapot.h"
#include	"shaders.h"
#include	"texturemap.h"
#include	"messages.h"
#include	"trimcurve.h"
#include	"genpoly.h"

#include	"ri.h"

using namespace Aqsis;

static RtBoolean ProcessPrimitiveVariables( CqSurface* pSurface, PARAMETERLIST );
template <class T>
RtVoid	CreateGPrim( T* pSurface );


//---------------------------------------------------------------------
// This file contains the interface functions which are published as the
//	Renderman Interface SPECification (C) 1988 Pixar.
//

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
RtToken	RI_SHIINYMETAL	= "shinymetal";
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

RtBasis	RiBezierBasis	= {{ -1, 3, -3, 1},
                         {3, -6, 3, 0},
                         { -3, 3, 0, 0},
                         {1, 0, 0, 0}};
RtBasis	RiBSplineBasis	= {{ -1, 3, -3, 1},
                          {3, -6, 3, 0},
                          { -3, 0, 3, 0},
                          {1, 4, 1, 0}};
RtBasis	RiCatmullRomBasis	= {{ -1, 3, -3, 1},
                             {2, -5, 4, -1},
                             { -1, 0, 1, 0},
                             {0, 2, 0, 0}};
RtBasis	RiHermiteBasis	= {{ 2, 1, -2, 1},
                          { -3, -2, 3, -1},
                          {0, 1, 0, 0},
                          {1, 0, 0, 0}};
RtBasis	RiPowerBasis	= {{ 1, 0, 0, 0},
                        {0, 1, 0, 0},
                        {0, 0, 1, 0},
                        {0, 0, 0, 1}};

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

//----------------------------------------------------------------------
// BuildParameterList
// Helper function to build a parameter list to pass on to the V style functions.
// returns a parameter count.

RtInt BuildParameterList( va_list pArgs, RtToken*& pTokens, RtPointer*& pValues )
{
	// TODO: Check this is thread safe.
	static std::vector<RtToken>	aTokens;
	static std::vector<RtPointer>	aValues;

	RtInt count = 0;
	RtToken pToken = va_arg( pArgs, RtToken );
	RtPointer pValue;
	aTokens.clear();
	aValues.clear();
	while ( pToken != 0 && pToken != RI_NULL )       	// While not RI_NULL
	{
		aTokens.push_back( pToken );
		pValue = va_arg( pArgs, RtPointer );
		aValues.push_back( pValue );
		pToken = va_arg( pArgs, RtToken );
		count++;
	}

	pTokens = &aTokens[ 0 ];
	pValues = &aValues[ 0 ];
	return ( count );
}

//----------------------------------------------------------------------
// RiDeclare
// Declare a new variable to be recognised by the system.
//
RtToken	RiDeclare( const char *name, const char *declaration )
{
	CqString strName( name ), strDecl( declaration );
	QGetRenderContext() ->AddParameterDecl( strName.c_str(), strDecl.c_str() );
	return ( 0 );
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
	QGetRenderContext() ->CreateMainContext();
	QGetRenderContext() ->ptransWriteCurrent() ->SetCurrentTransform( QGetRenderContext() ->Time(), CqMatrix() );
	// Clear the lightsources stack.
	CqLightsource* pL = Lightsource_stack.pFirst();
	while ( pL )
	{
		delete( pL );
		pL = Lightsource_stack.pFirst();
	}

	// Clear any options.
	QGetRenderContext() ->optCurrent().ClearOptions();

	return ;
}

//----------------------------------------------------------------------
// RiEnd
// End the rendermam render stage.
//
RtVoid	RiEnd()
{
	QGetRenderContext() ->DeleteMainContext();

	// Flush the image cache.
	CqTextureMap::FlushCache();

	// Clear the lightsources stack.
	CqLightsource* pL = Lightsource_stack.pFirst();
	while ( pL )
	{
		delete( pL );
		pL = Lightsource_stack.pFirst();
	}

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
	// Initialise the statistics variables. If the RIB doesn't contain
	// a Frame-block the initialisation was previously done in CqStats::Initilise()
	// which has to be called before a rendering session.
	QGetRenderContext() ->Stats().InitialiseFrame();
	// Start the timer. Note: The corresponding call of StopFrameTimer() is
	// done in WorldEnd (!) not FrameEnd since it can happen that there is
	// not FrameEnd (and usually there's not much between WorldEnd and FrameEnd).
	QGetRenderContext() ->Stats().StartFrameTimer();

	QGetRenderContext() ->CreateFrameContext();
	return ;
}


//----------------------------------------------------------------------
// RiFrameEnd
// End the rendering of an individual frame, options are restored.
//
RtVoid	RiFrameEnd()
{
	QGetRenderContext() ->DeleteFrameContext();

	return ;
}

//----------------------------------------------------------------------
// RiWorldBegin
// Start the information for the world, options are now frozen.  The world-to-camera
// transformation is set to the current transformation, and current is set to identity.
//
RtVoid	RiWorldBegin()
{
	// Start the frame timer (just in case there was no FrameBegin block. If there
	// was, nothing happens)
	QGetRenderContext() ->Stats().StartFrameTimer();

	// Now that the options have all been set, setup any undefined camera parameters.
	if ( !QGetRenderContext() ->optCurrent().FrameAspectRatioCalled() )
	{
		// Derive the FAR from the resolution and pixel aspect ratio.
		RtFloat PAR = QGetRenderContext() ->optCurrent().fPixelAspectRatio();
		RtFloat resH = QGetRenderContext() ->optCurrent().iXResolution();
		RtFloat resV = QGetRenderContext() ->optCurrent().iYResolution();
		QGetRenderContext() ->optCurrent().SetfFrameAspectRatio( ( resH * PAR ) / resV );
	}

	if ( !QGetRenderContext() ->optCurrent().ScreenWindowCalled() )
	{
		RtFloat fFAR = QGetRenderContext() ->optCurrent().fFrameAspectRatio();

		if ( fFAR >= 1.0 )
		{
			QGetRenderContext() ->optCurrent().SetfScreenWindowLeft( -fFAR );
			QGetRenderContext() ->optCurrent().SetfScreenWindowRight( + fFAR );
			QGetRenderContext() ->optCurrent().SetfScreenWindowBottom( -1 );
			QGetRenderContext() ->optCurrent().SetfScreenWindowTop( + 1 );
		}
		else
		{
			QGetRenderContext() ->optCurrent().SetfScreenWindowLeft( -1 );
			QGetRenderContext() ->optCurrent().SetfScreenWindowRight( + 1 );
			QGetRenderContext() ->optCurrent().SetfScreenWindowBottom( -1.0 / fFAR );
			QGetRenderContext() ->optCurrent().SetfScreenWindowTop( + 1.0 / fFAR );
		}
	}

	QGetRenderContext() ->CreateWorldContext();
	// Set the world to camera transformation matrix to the current matrix.
	QGetRenderContext() ->SetmatCamera( QGetRenderContext() ->matCurrent( QGetRenderContext() ->Time() ) );
	// and then reset the current matrix to identity, ready for object transformations.
	QGetRenderContext() ->ptransWriteCurrent() ->SetCurrentTransform( QGetRenderContext() ->Time(), CqMatrix() );

	QGetRenderContext() ->optCurrent().InitialiseCamera();
	QGetRenderContext() ->pImage() ->SetImage();

	return ;
}


//----------------------------------------------------------------------
// RiWorldEnd
// End the specifying of world data, options are released.
//

RtVoid	RiWorldEnd()
{
	// Call any specified pre render function.
	if ( QGetRenderContext() ->optCurrent().pPreRenderFunction() != NULL )
		( *QGetRenderContext() ->optCurrent().pPreRenderFunction() ) ();

	// Render the world
	QGetRenderContext() ->RenderWorld();

	// Delete the world context
	QGetRenderContext() ->DeleteWorldContext();

	// Stop the frame timer
	QGetRenderContext() ->Stats().StopFrameTimer();

	// Get the verbosity level from the options...
	TqInt verbosity = 0;
	const TqInt* poptEndofframe = QGetRenderContext() ->optCurrent().GetIntegerOption( "statistics", "endofframe" );
	if ( poptEndofframe != 0 )
		verbosity = poptEndofframe[ 0 ];

	// ...and print the statistics.
	QGetRenderContext() ->Stats().PrintStats( verbosity );
	return ;
}


//----------------------------------------------------------------------
// RiFormat
// Specify the setup of the final image.
//
RtVoid	RiFormat( RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio )
{
	QGetRenderContext() ->optCurrent().SetiXResolution( xresolution );
	QGetRenderContext() ->optCurrent().SetiYResolution( yresolution );
	QGetRenderContext() ->optCurrent().SetfPixelAspectRatio( ( pixelaspectratio < 0.0 ) ? 1.0 : pixelaspectratio );

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
	QGetRenderContext() ->optCurrent().SetfFrameAspectRatio( frameratio );

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
	QGetRenderContext() ->optCurrent().SetfScreenWindowLeft( left );
	QGetRenderContext() ->optCurrent().SetfScreenWindowRight( right );
	QGetRenderContext() ->optCurrent().SetfScreenWindowBottom( bottom );
	QGetRenderContext() ->optCurrent().SetfScreenWindowTop( top );

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
	QGetRenderContext() ->optCurrent().SetfCropWindowXMin( left );
	QGetRenderContext() ->optCurrent().SetfCropWindowXMax( right );
	QGetRenderContext() ->optCurrent().SetfCropWindowYMin( top );
	QGetRenderContext() ->optCurrent().SetfCropWindowYMax( bottom );

	return ;
}


//----------------------------------------------------------------------
// RiProjection
// Set the camera projection to be used.
//
RtVoid	RiProjection( const char *name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiProjectionV( name, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiProjectionV
// List mode version of above.
//
RtVoid	RiProjectionV( const char *name, PARAMETERLIST )
{
	if ( strcmp( name, RI_PERSPECTIVE ) == 0 )
		QGetRenderContext() ->optCurrent().SeteCameraProjection( ProjectionPerspective );
	else
		QGetRenderContext() ->optCurrent().SeteCameraProjection( ProjectionOrthographic );

	RtInt i;
	for ( i = 0; i < count; i++ )
	{
		RtToken	token = tokens[ i ];
		RtPointer	value = values[ i ];

		if ( strcmp( token, RI_FOV ) == 0 )
			QGetRenderContext() ->optCurrent().SetfFOV( *( reinterpret_cast<RtFloat*>( value ) ) );
	}
	// TODO: need to get the current transformation so that it can be added to the screen transformation.
	QGetRenderContext() ->ptransWriteCurrent() ->SetCurrentTransform( QGetRenderContext() ->Time(), CqMatrix() );

	return ;
}


//----------------------------------------------------------------------
// RiClipping
// Set the near and far clipping planes specified as distances from the camera.
//
RtVoid	RiClipping( RtFloat cnear, RtFloat cfar )
{
	QGetRenderContext() ->optCurrent().SetfClippingPlaneNear( cnear );
	QGetRenderContext() ->optCurrent().SetfClippingPlaneFar( cfar );

	return ;
}


//----------------------------------------------------------------------
// RiDepthOfField
// Specify the parameters which affect focal blur of the camera.
//
RtVoid	RiDepthOfField( RtFloat fstop, RtFloat focallength, RtFloat focaldistance )
{
	QGetRenderContext() ->optCurrent().SetffStop( fstop );
	QGetRenderContext() ->optCurrent().SetfFocalLength( focallength );
	QGetRenderContext() ->optCurrent().SetfFocalDistance( focaldistance );

	return ;
}


//----------------------------------------------------------------------
// RiShutter
//	Set the times at which the shutter opens and closes, used for motion blur.
//
RtVoid	RiShutter( RtFloat opentime, RtFloat closetime )
{
	QGetRenderContext() ->optCurrent().SetfShutterOpen( opentime );
	QGetRenderContext() ->optCurrent().SetfShutterClose( closetime );

	return ;
}


//----------------------------------------------------------------------
// RiPixelVariance
// Set the upper bound on the variance from the true pixel color by the pixel filter
// function.
//
RtVoid	RiPixelVariance( RtFloat variance )
{
	QGetRenderContext() ->optCurrent().SetfPixelVariance( variance );

	return ;
}


//----------------------------------------------------------------------
// RiPixelSamples
// Set the number of samples per pixel for the hidden surface function.
//
RtVoid	RiPixelSamples( RtFloat xsamples, RtFloat ysamples )
{
	QGetRenderContext() ->optCurrent().SetPixelXSamples( static_cast<TqInt>( xsamples ) );
	QGetRenderContext() ->optCurrent().SetPixelYSamples( static_cast<TqInt>( ysamples ) );

	return ;
}


//----------------------------------------------------------------------
// RiPixelFilter
// Set the function used to generate a final pixel value from supersampled values.
//
RtVoid	RiPixelFilter( RtFilterFunc function, RtFloat xwidth, RtFloat ywidth )
{
	QGetRenderContext() ->optCurrent().SetfuncFilter( function );
	QGetRenderContext() ->optCurrent().SetfFilterXWidth( xwidth );
	QGetRenderContext() ->optCurrent().SetfFilterYWidth( ywidth );

	return ;
}


//----------------------------------------------------------------------
// RiExposure
//	Set the values of the exposure color modification function.
//
RtVoid	RiExposure( RtFloat gain, RtFloat gamma )
{
	QGetRenderContext() ->optCurrent().SetfExposureGain( gain );
	QGetRenderContext() ->optCurrent().SetfExposureGamma( gamma );

	return ;
}


//----------------------------------------------------------------------
// RiImager
// Specify a prepocessing imager shader.
//
RtVoid	RiImager( const char *name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiImagerV( name, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiImagerV
// List based version of above.
//
RtVoid	RiImagerV( const char *name, PARAMETERLIST )
{
	RtInt i;

	if ( strlen( name ) )
	{
		QGetRenderContext() ->optCurrent().SetstrImager( name );
		for ( i = 0; i < count; i++ )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			QGetRenderContext() ->optCurrent().SetValueImager(
			    token, static_cast<TqPchar>( value ) );
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
	if ( strcmp( type, "rgba" ) == 0 )
	{
		QGetRenderContext() ->optCurrent().SetiColorQuantizeOne( one );
		QGetRenderContext() ->optCurrent().SetiColorQuantizeMin( min );
		QGetRenderContext() ->optCurrent().SetiColorQuantizeMax( max );
		QGetRenderContext() ->optCurrent().SetfColorQuantizeDitherAmplitude( ditheramplitude );
	}
	else
	{
		QGetRenderContext() ->optCurrent().SetiDepthQuantizeOne( one );
		QGetRenderContext() ->optCurrent().SetiDepthQuantizeMin( min );
		QGetRenderContext() ->optCurrent().SetiDepthQuantizeMax( max );
		QGetRenderContext() ->optCurrent().SetfDepthQuantizeDitherAmplitude( ditheramplitude );
	}

	return ;
}


//----------------------------------------------------------------------
// RiDisplay
// Set the final output name and type.
//
RtVoid	RiDisplay( const char *name, RtToken type, RtToken mode, ... )
{
	va_list	pArgs;
	va_start( pArgs, mode );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiDisplayV( name, type, mode, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiDisplayV
// List based version of above.
//
RtVoid	RiDisplayV( const char *name, RtToken type, RtToken mode, PARAMETERLIST )
{
	CqString strName( name );
	CqString strType( type );

	QGetRenderContext() ->optCurrent().SetstrDisplayName( strName.c_str() );
	QGetRenderContext() ->optCurrent().SetstrDisplayType( strType.c_str() );

	// Append the display mode to the current setting.
	RtInt eValue = 0;
	if ( strstr( mode, RI_RGB ) != NULL )
		eValue |= ModeRGB;
	if ( strstr( mode, RI_A ) != NULL )
		eValue |= ModeA;
	if ( strstr( mode, RI_Z ) != NULL )
		eValue |= ModeZ;

	// Check if the request is to add a display driver.
	if ( strName[ 0 ] == '+' )
	{
		QGetRenderContext() ->optCurrent().SetiDisplayMode( QGetRenderContext() ->optCurrent().iDisplayMode() | eValue );
		strName = strName.substr( 1 );
	}
	else
	{
		QGetRenderContext() ->ClearDisplayRequests();
		QGetRenderContext() ->optCurrent().SetiDisplayMode( eValue );
	}
	// Add a display driver to the list of requested drivers.
	QGetRenderContext() ->AddDisplayRequest( strName.c_str(), strType.c_str(), mode );

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
	RtFloat d, d2;

	d2 = x * x + y * y; /* d*d */
	d = sqrt( d2 ); /* distance from origin */

	if ( d < 1 )
		return ( 1.5 * d * d2 - 2.5 * d2 + 1.0 );
	else if ( d < 2 )
		return ( -d * d2 * 0.5 + 2.5 * d2 - 4.0 * d + 2.0 );
	else
		return 0.0;
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
RtVoid	RiHider( const char *name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiHiderV( name, count, pTokens, pValues );

}

//----------------------------------------------------------------------
// RiHiderV
// List based version of above.
//
RtVoid	RiHiderV( const char *name, PARAMETERLIST )
{
	if ( !strcmp( name, "hider" ) || !strcmp( name, "painter" ) )
	{
		QGetRenderContext() ->optCurrent().SetstrHider( name );
	}

	return ;
}


//----------------------------------------------------------------------
// RiColorSamples
// Specify the depth and conversion arrays for color manipulation.
//
RtVoid	RiColorSamples( RtInt N, RtFloat *nRGB, RtFloat *RGBn )
{
	CqBasicError( 0, Severity_Normal, "RiColorSamples not supported, defaults to 3" );
	return ;
}


//----------------------------------------------------------------------
// RiRelativeDetail
// Set the scale used for all subsequent level of detail calculations.
//
RtVoid	RiRelativeDetail( RtFloat relativedetail )
{
	CqBasicError( 0, Severity_Normal, "RiRelativeDetail not supported" );
	return ;
}


//----------------------------------------------------------------------
// RiOption
// Specify system specific option.
//
RtVoid	RiOption( const char *name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiOptionV( name, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiOptionV
// List based version of above.
//
RtVoid	RiOptionV( const char *name, PARAMETERLIST )
{
	// Find the parameter on the current options.
	CqSystemOption * pOpt = QGetRenderContext() ->optCurrent().pOptionWrite( name );

	RtInt i;
	for ( i = 0; i < count; i++ )
	{
		RtToken	token = tokens[ i ];
		RtPointer	value = values[ i ];

		// Search for the parameter in the declarations.
		// Note Options can only be uniform.
		SqParameterDeclaration Decl = QGetRenderContext() ->FindParameterDecl( token );
		RtInt Type = Decl.m_Type;
		CqParameter* pParam = pOpt->pParameter( token );
		if ( pParam == 0 )
		{
			if ( Decl.m_strName != "" && ( Decl.m_Type & Storage_Mask ) == Type_Uniform )
			{
				pParam = Decl.m_pCreate( token, Decl.m_Count );
				pOpt->AddParameter( pParam );
			}
			else
			{
				if ( Decl.m_strName == "" )
					CqBasicError( ErrorID_UnknownSymbol, Severity_Normal, "Unknown Symbol" );
				else
					CqBasicError( ErrorID_InvalidType, Severity_Normal, "Options can only be uniform" );
				return ;
			}
		}
		else
			Type = pParam->Type();

		switch ( Type & Type_Mask )
		{
				case Type_Float:
				{
					RtFloat * pf = reinterpret_cast<RtFloat*>( value );
					if ( Type & Type_Array )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); j++ )
							static_cast<CqParameterTypedUniformArray<RtFloat, Type_Float>*>( pParam ) ->pValue() [ j ] = pf[ j ];
					}
					else
						static_cast<CqParameterTypedUniform<RtFloat, Type_Float>*>( pParam ) ->pValue() [ 0 ] = pf[ 0 ];
				}
				break;

				case Type_Integer:
				{
					RtInt* pi = reinterpret_cast<RtInt*>( value );
					if ( Type & Type_Array )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); j++ )
							static_cast<CqParameterTypedUniformArray<RtInt, Type_Integer>*>( pParam ) ->pValue() [ j ] = pi[ j ];
					}
					else
						static_cast<CqParameterTypedUniform<RtInt, Type_Integer>*>( pParam ) ->pValue() [ 0 ] = pi[ 0 ];
				}
				break;

				case Type_String:
				{
					char** ps = reinterpret_cast<char**>( value );
					if ( Type & Type_Array )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); j++ )
						{
							CqString str( "" );
							if ( strcmp( name, "searchpath" ) == 0 )
							{
								// Get the old value for use in escape replacement
								CqString str_old = static_cast<CqParameterTypedUniform<CqString, Type_UniformString>*>( pParam ) ->pValue() [ 0 ];
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

							static_cast<CqParameterTypedUniformArray<CqString, Type_String>*>( pParam ) ->pValue() [ j ] = str;
						}
					}
					else
					{
						CqString str( "" );
						if ( strcmp( name, "searchpath" ) == 0 )
						{
							// Get the old value for use in escape replacement
							CqString str_old = static_cast<CqParameterTypedUniform<CqString, Type_UniformString>*>( pParam ) ->pValue() [ 0 ];
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

						static_cast<CqParameterTypedUniform<CqString, Type_String>*>( pParam ) ->pValue() [ 0 ] = str;
					}
				}
				break;
				// TODO: Rest of parameter types.
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
	QGetRenderContext() ->CreateAttributeContext();

	return ;
}


//----------------------------------------------------------------------
// RiAttributeEnd
// End the current attribute defintion, pops the previous attributes.
//
RtVoid	RiAttributeEnd()
{
	QGetRenderContext() ->DeleteAttributeContext();

	return ;
}


//----------------------------------------------------------------------
// RiColor
//	Set the current color for use by the geometric primitives.
//
RtVoid	RiColor( RtColor Cq )
{
	QGetRenderContext() ->pattrWriteCurrent() ->SetcolColor( CqColor( Cq ), QGetRenderContext() ->Time() );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiOpacity
// Set the current opacity, for use by the geometric primitives.
//
RtVoid	RiOpacity( RtColor Os )
{
	QGetRenderContext() ->pattrWriteCurrent() ->SetcolOpacity( CqColor( Os ), QGetRenderContext() ->Time() );
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
	QGetRenderContext() ->pattrWriteCurrent() ->aTextureCoordinates( QGetRenderContext() ->Time() ) [ 0 ] = CqVector2D( s1, t1 );
	QGetRenderContext() ->pattrWriteCurrent() ->aTextureCoordinates( QGetRenderContext() ->Time() ) [ 1 ] = CqVector2D( s2, t2 );
	QGetRenderContext() ->pattrWriteCurrent() ->aTextureCoordinates( QGetRenderContext() ->Time() ) [ 2 ] = CqVector2D( s3, t3 );
	QGetRenderContext() ->pattrWriteCurrent() ->aTextureCoordinates( QGetRenderContext() ->Time() ) [ 3 ] = CqVector2D( s4, t4 );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiLightSource
// Create a new light source at the current transformation.
//
RtLightHandle	RiLightSource( const char *name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	return ( RiLightSourceV( name, count, pTokens, pValues ) );
}


//----------------------------------------------------------------------
// RiLightSourceV
// List based version of above.
//
RtLightHandle	RiLightSourceV( const char *name, PARAMETERLIST )
{
	// Find the lightsource shader.
	CqShader * pShader = static_cast<CqShader*>( QGetRenderContext() ->CreateShader( name, Type_Lightsource ) );

	// TODO: Report error.
	if ( pShader == 0 ) return ( 0 );

	pShader->matCurrent() = QGetRenderContext() ->matCurrent();
	CqLightsource* pNew = new CqLightsource( pShader, RI_TRUE );

	// Execute the intiialisation code here, as we now have our shader context complete.
	pShader->PrepareDefArgs();

	if ( pNew != 0 )
	{
		RtInt i;
		for ( i = 0; i < count; i++ )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			pShader->SetValue( token, static_cast<TqPchar>( value ) );
		}
		QGetRenderContext() ->pattrWriteCurrent() ->AddLightsource( pNew );

		// Add it as a Context light as well in case we are in a context that manages it's own lights.
		QGetRenderContext() ->pconCurrent() ->AddContextLightSource( pNew );
		return ( reinterpret_cast<RtLightHandle>( pNew ) );
	}
	return ( 0 );
}


//----------------------------------------------------------------------
// RiAreaLightSource
// Create a new area light source at the current transformation, all
// geometric primitives until the next RiAttributeEnd, become part of this
// area light source.
//
RtLightHandle	RiAreaLightSource( const char *name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	return ( RiAreaLightSourceV( name, count, pTokens, pValues ) );
}


//----------------------------------------------------------------------
// RiAreaLightSourceV
// List based version of above.
//
RtLightHandle	RiAreaLightSourceV( const char *name, PARAMETERLIST )
{
	CqBasicError( 0, Severity_Normal, "RiAreaLightSource not supported, creating a point lightsource" );
	return ( RiLightSourceV( name, count, tokens, values ) );
}


//----------------------------------------------------------------------
// RiIlluminate
// Set the current status of the specified light source.
//
RtVoid	RiIlluminate( RtLightHandle light, RtBoolean onoff )
{
	// Check if we are turning the light on or off.
	if ( light == NULL ) return ;
	if ( onoff )
		QGetRenderContext() ->pattrWriteCurrent() ->AddLightsource( reinterpret_cast<CqLightsource*>( light ) );
	else
		QGetRenderContext() ->pattrWriteCurrent() ->RemoveLightsource( reinterpret_cast<CqLightsource*>( light ) );
	return ;
}


//----------------------------------------------------------------------
// RiSurface
// Set the current surface shader, used by geometric primitives.
//
RtVoid	RiSurface( const char *name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiSurfaceV( name, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiSurfaceV
// List based version of above.
//
RtVoid	RiSurfaceV( const char *name, PARAMETERLIST )
{
	// Find the shader.
	CqShader * pshadSurface = QGetRenderContext() ->CreateShader( name, Type_Surface );

	if ( pshadSurface != 0 )
	{
		pshadSurface->matCurrent() = QGetRenderContext() ->matCurrent();
		// Execute the intiialisation code here, as we now have our shader context complete.
		pshadSurface->PrepareDefArgs();
		RtInt i;
		for ( i = 0; i < count; i++ )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			pshadSurface->SetValue( token, static_cast<TqPchar>( value ) );
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
RtVoid	RiAtmosphere( const char *name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiAtmosphereV( name, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiAtmosphereV
// List based version of above.
//
RtVoid	RiAtmosphereV( const char *name, PARAMETERLIST )
{
	// Find the shader.
	CqShader * pshadAtmosphere = QGetRenderContext() ->CreateShader( name, Type_Volume );

	if ( pshadAtmosphere != 0 )
	{
		pshadAtmosphere->matCurrent() = QGetRenderContext() ->matCurrent();
		// Execute the intiialisation code here, as we now have our shader context complete.
		pshadAtmosphere->PrepareDefArgs();
		RtInt i;
		for ( i = 0; i < count; i++ )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			pshadAtmosphere->SetValue( token, static_cast<TqPchar>( value ) );
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
RtVoid	RiInterior( const char *name, ... )
{
	CqBasicError( 0, Severity_Normal, "RiInterior shaders not supported" );
	return ;
}


//----------------------------------------------------------------------
// RiInteriorV
// List based version of above.
//
RtVoid	RiInteriorV( const char *name, PARAMETERLIST )
{
	CqBasicError( 0, Severity_Normal, "RiInterior shaders not supported" );
	return ;
}


//----------------------------------------------------------------------
// RiExterior
// Set the current exterior volumetric shader.
//
RtVoid	RiExterior( const char *name, ... )
{
	CqBasicError( 0, Severity_Normal, "RiExterior shaders not supported" );
	return ;
}


//----------------------------------------------------------------------
// RiExteriorV
// List based version of above.
//
RtVoid	RiExteriorV( const char *name, PARAMETERLIST )
{
	CqBasicError( 0, Severity_Normal, "RiExterior shaders not supported" );
	return ;
}


//----------------------------------------------------------------------
// RiShadingRate
// Specify the size of the shading area in pixels.
//
RtVoid	RiShadingRate( RtFloat size )
{
	QGetRenderContext() ->pattrWriteCurrent() ->SetfEffectiveShadingRate( size, QGetRenderContext() ->Time() );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiShadingInterpolation
// Specify the method of shading interpolation.
//
RtVoid	RiShadingInterpolation( RtToken type )
{
	if ( strcmp( type, RI_CONSTANT ) == 0 )
		QGetRenderContext() ->pattrWriteCurrent() ->SeteShadingInterpolation( CqShadingAttributes::ShadingConstant, QGetRenderContext() ->Time() );
	else
		if ( strcmp( type, RI_SMOOTH ) == 0 )
			QGetRenderContext() ->pattrWriteCurrent() ->SeteShadingInterpolation( CqShadingAttributes::ShadingSmooth, QGetRenderContext() ->Time() );
		else
			CqBasicError( ErrorID_InvalidData, Severity_Normal, "Invald shading interpolation" );

	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiMatte
// Set the matte state of subsequent geometric primitives.
//
RtVoid	RiMatte( RtBoolean onoff )
{
	CqBasicError( 0, Severity_Normal, "RiMatte not supported" );
	QGetRenderContext() ->pattrWriteCurrent() ->SetbMatteSurfaceFlag( onoff != 0, QGetRenderContext() ->Time() );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiBound
// Set the bounding cube of the current primitives.
//
RtVoid	RiBound( RtBound bound )
{
	QGetRenderContext() ->pattrWriteCurrent() ->SetBound( CqBound( bound ) );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiDetail
// Set the current bounding cube for use by level of detail calculation.
//
RtVoid	RiDetail( RtBound bound )
{
	CqBasicError( 0, Severity_Normal, "RiDetail not supported" );
	return ;
}


//----------------------------------------------------------------------
// RiDetailRange
// Set the visible range of any subsequent geometric primitives.
//
RtVoid	RiDetailRange( RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh )
{
	CqBasicError( 0, Severity_Normal, "RiDetailRange not supported" );
	return ;
}


//----------------------------------------------------------------------
// RiGeometricApproximation
// Specify any parameters used by approximation functions during rendering.
//
RtVoid	RiGeometricApproximation( RtToken type, RtFloat value )
{
	CqBasicError( 0, Severity_Normal, "RiGeometricApproximation not supported" );
	return ;
}


//----------------------------------------------------------------------
// RiOrientation
// Set the handedness of any subsequent geometric primitives.
//
RtVoid	RiOrientation( RtToken orientation )
{
	if ( orientation != 0 )
	{
		if ( strstr( orientation, RI_LH ) != 0 )
			QGetRenderContext() ->pattrWriteCurrent() ->SeteOrientation( OrientationLH, QGetRenderContext() ->Time() );
		if ( strstr( orientation, RI_RH ) != 0 )
			QGetRenderContext() ->pattrWriteCurrent() ->SeteOrientation( OrientationRH, QGetRenderContext() ->Time() );
		if ( strstr( orientation, RI_INSIDE ) != 0 )
		{
			QGetRenderContext() ->pattrWriteCurrent() ->SeteOrientation( QGetRenderContext() ->pattrCurrent() ->eCoordsysOrientation(), QGetRenderContext() ->Time() );
			QGetRenderContext() ->pattrWriteCurrent() ->FlipeOrientation();
		}
		if ( strstr( orientation, RI_OUTSIDE ) != 0 )
			QGetRenderContext() ->pattrWriteCurrent() ->SeteOrientation( QGetRenderContext() ->pattrCurrent() ->eCoordsysOrientation(), QGetRenderContext() ->Time() );
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
	QGetRenderContext() ->pattrWriteCurrent() ->SetiNumberOfSides( nsides, QGetRenderContext() ->Time() );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiIdentity
// Set the current transformation to the identity matrix.
//
RtVoid	RiIdentity()
{
	QGetRenderContext() ->ptransWriteCurrent() ->SetCurrentTransform( QGetRenderContext() ->Time(), CqMatrix() );

	// Make sure the orientations are correct after the matrix update.
	// We get the camera matrix and see if that is going to alter the default orientation, note
	// that this should still work if we get a call to identity before the WorldBegin, as then the
	// camera transform will be identity, and the behaviour will be correct.
	if ( QGetRenderContext() ->matWorldToCamera().Determinant() < 0 )
	{
		QGetRenderContext() ->pattrWriteCurrent() ->SeteCoordsysOrientation( OrientationRH );
		QGetRenderContext() ->pattrWriteCurrent() ->SeteOrientation( OrientationRH );
	}
	else
	{
		QGetRenderContext() ->pattrWriteCurrent() ->SeteCoordsysOrientation( OrientationLH );
		QGetRenderContext() ->pattrWriteCurrent() ->SeteOrientation( OrientationLH );
	}

	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// Set the current transformation to the specified matrix.
//
RtVoid	RiTransform( RtMatrix transform )
{
	// TODO: Determine if this matrix requires a change in orientation.
	CqMatrix matTrans( transform );
	if ( matTrans.Determinant() < 0 )
	{
		QGetRenderContext() ->pattrWriteCurrent() ->FlipeOrientation( QGetRenderContext() ->Time() );
		QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );
	}
	QGetRenderContext() ->ptransWriteCurrent() ->SetCurrentTransform( QGetRenderContext() ->Time(), CqMatrix( transform ) );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RoConcatTransform
// Concatenate the specified matrix into the current transformation matrix.
//
RtVoid	RiConcatTransform( RtMatrix transform )
{
	// Check if this transformation results in a change in orientation.
	CqMatrix matTrans( transform );
	if ( matTrans.Determinant() < 0 )
	{
		QGetRenderContext() ->pattrWriteCurrent() ->FlipeOrientation( QGetRenderContext() ->Time() );
		QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );
	}

	QGetRenderContext() ->ptransWriteCurrent() ->ConcatCurrentTransform( QGetRenderContext() ->Time(), CqMatrix( transform ) );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiPerspective
// Concatenate a perspective transformation into the current transformation.
//
RtVoid	RiPerspective( RtFloat f )
{
	if ( f <= 0 )
	{
		CqBasicError( 0, Severity_Normal, "RiPerspective given bad fov value." );
		return ;
	}

	f = tan( RAD( f / 2 ) );

	// This matches PRMan 3.9 in testing, but not BMRT 2.6's rgl and rendrib.
	CqMatrix	matP( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, f, f, 0, 0, -f, 0 );

	// Check if this transformation results in a change in orientation.
	if ( matP.Determinant() < 0 )
	{
		QGetRenderContext() ->pattrWriteCurrent() ->FlipeOrientation( QGetRenderContext() ->Time() );
		QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );
	}

	QGetRenderContext() ->ptransWriteCurrent() ->ConcatCurrentTransform( QGetRenderContext() ->Time(), matP );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiTranslate
// Concatenate a translation into the current transformation.
//
RtVoid	RiTranslate( RtFloat dx, RtFloat dy, RtFloat dz )
{
	CqMatrix	matTrans( CqVector3D( dx, dy, dz ) );
	// Check if this transformation results in a change in orientation.
	if ( matTrans.Determinant() < 0 )
	{
		QGetRenderContext() ->pattrWriteCurrent() ->FlipeOrientation( QGetRenderContext() ->Time() );
		QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );
	}

	QGetRenderContext() ->ptransWriteCurrent() ->ConcatCurrentTransform( QGetRenderContext() ->Time(), matTrans );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiRotate
// Concatenate a rotation into the current transformation.
//
RtVoid	RiRotate( RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz )
{
	CqMatrix	matRot( RAD( angle ), CqVector4D( dx, dy, dz ) );
	// Check if this transformation results in a change in orientation.
	if ( matRot.Determinant() < 0 )
	{
		QGetRenderContext() ->pattrWriteCurrent() ->FlipeOrientation( QGetRenderContext() ->Time() );
		QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );
	}

	QGetRenderContext() ->ptransWriteCurrent() ->ConcatCurrentTransform( QGetRenderContext() ->Time(), matRot );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiScale
// Concatenate a scale into the current transformation.
//
RtVoid	RiScale( RtFloat sx, RtFloat sy, RtFloat sz )
{
	CqMatrix	matScale( sx, sy, sz );
	// Check if this transformation results in a change in orientation.
	if ( matScale.Determinant() < 0 )
	{
		QGetRenderContext() ->pattrWriteCurrent() ->FlipeOrientation( QGetRenderContext() ->Time() );
		QGetRenderContext() ->pattrWriteCurrent() ->FlipeCoordsysOrientation( QGetRenderContext() ->Time() );
	}

	QGetRenderContext() ->ptransWriteCurrent() ->ConcatCurrentTransform( QGetRenderContext() ->Time(), matScale );
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
	CqMatrix	matSkew( RAD( angle ), dx1, dy1, dz1, dx2, dy2, dz2 );

	// This transformation can not change orientation.

	QGetRenderContext() ->ptransWriteCurrent() ->ConcatCurrentTransform( QGetRenderContext() ->Time(), matSkew );
	QGetRenderContext() ->AdvanceTime();
	return ;
}


//----------------------------------------------------------------------
// RiDeformation
// Specify a deformation shader to be included into the current transformation.
//
RtVoid	RiDeformation( const char *name, ... )
{
	CqBasicError( 0, Severity_Normal, "RiDeformation shaders not supported" );
	return ;
}


//----------------------------------------------------------------------
// RiDeformationV
// List based version of above.
//
RtVoid	RiDeformationV( const char *name, PARAMETERLIST )
{
	CqBasicError( 0, Severity_Normal, "RiDeformation shaders not supported" );
	return ;
}


//----------------------------------------------------------------------
// RiDisplacement
// Specify the current displacement shade used by geometric primitives.
//
RtVoid	RiDisplacement( const char *name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiDisplacementV( name, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiDisplacementV
// List based version of above.
//
RtVoid	RiDisplacementV( const char *name, PARAMETERLIST )
{
	// Find the shader.
	CqShader * pshadDisplacement = QGetRenderContext() ->CreateShader( name, Type_Displacement );

	if ( pshadDisplacement != 0 )
	{
		pshadDisplacement->matCurrent() = QGetRenderContext() ->matCurrent();
		// Execute the intiialisation code here, as we now have our shader context complete.
		pshadDisplacement->PrepareDefArgs();
		RtInt i;
		for ( i = 0; i < count; i++ )
		{
			RtToken	token = tokens[ i ];
			RtPointer	value = values[ i ];

			pshadDisplacement->SetValue( token, static_cast<TqPchar>( value ) );
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
	// Insert the named coordinate system into the list help on the renderer.
	QGetRenderContext() ->ptransWriteCurrent() ->SetCurrentTransform( QGetRenderContext() ->Time(), QGetRenderContext() ->matSpaceToSpace( space, "world" ) );
	QGetRenderContext() ->AdvanceTime();

	return ;
}


//----------------------------------------------------------------------
// RiTransformPoints
// Transform a list of points from one coordinate system to another.
//
RtPoint*	RiTransformPoints( RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[] )
{
	CqBasicError( 0, Severity_Normal, "RiTransformPoints not supported" );
	return ( 0 );
}


//----------------------------------------------------------------------
// RiTransformBegin
// Push the current transformation state.
//
RtVoid	RiTransformBegin()
{
	QGetRenderContext() ->CreateTransformContext();

	return ;
}


//----------------------------------------------------------------------
// RiTransformEnd
// Pop the previous transformation state.
//
RtVoid	RiTransformEnd()
{
	QGetRenderContext() ->DeleteTransformContext();

	return ;
}


//----------------------------------------------------------------------
// RiAttribute
// Set a system specific attribute.
//
RtVoid	RiAttribute( const char *name, ... )
{
	va_list	pArgs;
	va_start( pArgs, name );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiAttributeV( name, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiAttributeV
// List based version of above.
//
RtVoid	RiAttributeV( const char *name, PARAMETERLIST )
{
	// Find the parameter on the current options.
	CqSystemOption * pAttr = QGetRenderContext() ->pattrWriteCurrent() ->pAttributeWrite( name );

	RtInt i;
	for ( i = 0; i < count; i++ )
	{
		RtToken	token = tokens[ i ];
		RtPointer	value = values[ i ];

		RtInt Type;
		CqParameter* pParam = pAttr->pParameter( token );
		if ( pParam == 0 )
		{
			// Search for the parameter in the declarations.
			// Note attributes can only be uniform.
			SqParameterDeclaration Decl = QGetRenderContext() ->FindParameterDecl( token );
			if ( Decl.m_strName != "" && ( Decl.m_Type & Storage_Mask ) == Type_Uniform )
			{
				pParam = Decl.m_pCreate( Decl.m_strName.c_str(), Decl.m_Count );
				Type = Decl.m_Type;
				pAttr->AddParameter( pParam );
			}
			else
			{
				if ( Decl.m_strName == "" )
					CqBasicError( ErrorID_UnknownSymbol, Severity_Normal, "Unknown Symbol" );
				else
					CqBasicError( ErrorID_InvalidType, Severity_Normal, "Attributes can only be uniform" );
				return ;
			}
		}
		else
			Type = pParam->Type();

		switch ( Type & Type_Mask )
		{
				case Type_Float:
				{
					RtFloat * pf = reinterpret_cast<RtFloat*>( value );
					if ( Type & Type_Array )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); j++ )
							static_cast<CqParameterTypedUniformArray<RtFloat, Type_Float>*>( pParam ) ->pValue() [ j ] = pf[ j ];
					}
					else
						static_cast<CqParameterTypedUniform<RtFloat, Type_Float>*>( pParam ) ->pValue() [ 0 ] = pf[ 0 ];
				}
				break;

				case Type_Integer:
				{
					RtInt* pi = reinterpret_cast<RtInt*>( value );
					if ( Type & Type_Array )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); j++ )
							static_cast<CqParameterTypedUniformArray<RtInt, Type_Integer>*>( pParam ) ->pValue() [ j ] = pi[ j ];
					}
					else
						static_cast<CqParameterTypedUniform<RtInt, Type_Integer>*>( pParam ) ->pValue() [ 0 ] = pi[ 0 ];
				}
				break;

				case Type_String:
				{
					char** ps = reinterpret_cast<char**>( value );
					if ( Type & Type_Array )
					{
						RtInt j;
						for ( j = 0; j < pParam->Count(); j++ )
						{
							CqString str( ps[ j ] );
							static_cast<CqParameterTypedUniform<CqString, Type_String>*>( pParam ) ->pValue() [ j ] = str;
						}
					}
					else
					{
						CqString str( ps[ 0 ] );
						static_cast<CqParameterTypedUniform<CqString, Type_String>*>( pParam ) ->pValue() [ 0 ] = str;
					}
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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiPolygonV( nvertices, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiPolygonV
// List based version of above.
//
RtVoid	RiPolygonV( RtInt nvertices, PARAMETERLIST )
{
	// Create a new polygon surface primitive.
	CqSurfacePolygon * pSurface = new CqSurfacePolygon( nvertices );
	pSurface->AddRef();

	// Process any specified primitive variables.
	pSurface->SetDefaultPrimitiveVariables( RI_FALSE );
	if ( ProcessPrimitiveVariables( pSurface, count, tokens, values ) )
	{
		if ( !pSurface->CheckDegenerate() )
		{
			// Check if s/t, u/v are needed and not specified, and if so get them from the object space points.
			pSurface->TransferDefaultSurfaceParameters();
			CreateGPrim( pSurface );
		}
		else
		{
			CqBasicError( ErrorID_InvalidData, Severity_Normal, "Degenerate polygon found" );
			pSurface->Release();
		}
	}
	else
		pSurface->Release();

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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiGeneralPolygonV( nloops, nverts, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiGeneralPolygonV
// List based version of above.
//
RtVoid	RiGeneralPolygonV( RtInt nloops, RtInt nverts[], PARAMETERLIST )
{
	TqInt iloop;

	// Calcualte how many points there are.
	TqInt cVerts = 0;
	for ( iloop = 0; iloop < nloops; iloop++ )
		cVerts += nverts[ iloop ];

	// Create a storage class for all the points.
	CqPolygonPoints* pPointsClass = new CqPolygonPoints( cVerts );
	// Process any specified primitive variables
	pPointsClass->SetDefaultPrimitiveVariables( RI_FALSE );

	if ( ProcessPrimitiveVariables( pPointsClass, count, tokens, values ) )
	{
		// Work out which plane to project to.
		TqFloat	MinX, MaxX;
		TqFloat	MinY, MaxY;
		TqFloat	MinZ, MaxZ;
		CqVector3D	vecTemp = pPointsClass->P() [ 0 ];
		MinX = MaxX = vecTemp.x();
		MinY = MaxY = vecTemp.y();
		MinZ = MaxZ = vecTemp.z();

		TqUint iVert;
		for ( iVert = 1; iVert < pPointsClass->P().Size(); iVert++ )
		{
			vecTemp = pPointsClass->P() [ iVert ];
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
		for ( iloop = 0; iloop < nloops; iloop++ )
		{
			CqPolygonGeneral2D polya;
			polya.SetAxis( Axis );
			polya.SetpVertices( pPointsClass );
			TqInt ivert;
			for ( ivert = 0; ivert < nverts[ iloop ]; ivert++ )
			{
				assert( ipoint < pPointsClass->P().Size() );
				polya.aiVertices().push_back( ipoint++ );
			}
			if ( iloop == 0 )
			{
				if ( polya.CalcOrientation() != CqPolygonGeneral2D::Orientation_AntiClockwise )
					polya.SwapDirection();
				poly = polya;
			}
			else
			{
				if ( polya.CalcOrientation() != CqPolygonGeneral2D::Orientation_Clockwise )
					polya.SwapDirection();
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


//----------------------------------------------------------------------
// RiPointsPolygons
// Specify a list of convex coplanar polygons and their shared vertices.
//
RtVoid	RiPointsPolygons( RtInt npolys, RtInt nverts[], RtInt verts[], ... )
{
	va_list	pArgs;
	va_start( pArgs, verts );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiPointsPolygonsV( npolys, nverts, verts, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiPointsPolygonsV
// List based version of above.
//


RtVoid	RiPointsPolygonsV( RtInt npolys, RtInt nverts[], RtInt verts[], PARAMETERLIST )
{
	// Calculate how many vertices there are.
	RtInt cVerts = 0;
	RtInt* pVerts = verts;
	RtInt poly;
	for ( poly = 0; poly < npolys; poly++ )
	{
		RtInt v;
		for ( v = 0; v < nverts[ poly ]; v++ )
		{
			if ( ( ( *pVerts ) + 1 ) > cVerts )
				cVerts = ( *pVerts ) + 1;
			pVerts++;
		}
	}

	// Create a storage class for all the points.
	CqPolygonPoints* pPointsClass = new CqPolygonPoints( cVerts );
	pPointsClass->AddRef();
	// Process any specified primitive variables
	pPointsClass->SetDefaultPrimitiveVariables( RI_FALSE );
	if ( ProcessPrimitiveVariables( pPointsClass, count, tokens, values ) )
	{
		// Check if s/t, u/v are needed and not specified, and if so get them from the object space points.
		pPointsClass->TransferDefaultSurfaceParameters();

		if ( QGetRenderContext() ->ptransCurrent() ->cTimes() <= 1 )
		{
			// Transform the points into "current" space,
			pPointsClass->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld() ),
			                         QGetRenderContext() ->matNSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld() ),
			                         QGetRenderContext() ->matVSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld() ) );

			// For each polygon specified create a primitive.
			RtInt	iP = 0;
			for ( poly = 0; poly < npolys; poly++ )
			{
				// Create a surface polygon
				CqSurfacePointsPolygon*	pSurface = new CqSurfacePointsPolygon( pPointsClass );
				pSurface->AddRef();
				RtBoolean fValid = RI_TRUE;

				pSurface->aIndices().resize( nverts[ poly ] );
				RtInt i;
				for ( i = 0; i < nverts[ poly ]; i++ )       	// Fill in the points
				{
					if ( verts[ iP ] >= cVerts )
					{
						fValid = RI_FALSE;
						CqAttributeError( 1, Severity_Normal, "Invalid PointsPolygon index", pSurface->pAttributes() );
						break;
					}
					pSurface->aIndices() [ i ] = verts[ iP ];
					iP++;
				}
				if ( fValid )
					QGetRenderContext() ->pImage() ->PostSurface( pSurface );
			}
			QGetRenderContext() ->Stats().IncGPrims();
		}
		else
		{
			std::vector<CqPolygonPoints*>	apPoints;
			RtInt i;
			apPoints.push_back( pPointsClass );
			for ( i = 1; i < QGetRenderContext() ->ptransCurrent() ->cTimes(); i++ )
			{
				RtFloat time = QGetRenderContext() ->ptransCurrent() ->Time( i );
				CqPolygonPoints* pPointsClass2 = new CqPolygonPoints( *pPointsClass );

				// Transform the points into camera space for processing,
				pPointsClass2->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass2->pTransform() ->matObjectToWorld( time ) ),
				                          QGetRenderContext() ->matNSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass2->pTransform() ->matObjectToWorld( time ) ),
				                          QGetRenderContext() ->matVSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass2->pTransform() ->matObjectToWorld( time ) ) );
				apPoints.push_back( pPointsClass2 );
			}
			pPointsClass->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld() ),
			                         QGetRenderContext() ->matNSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld() ),
			                         QGetRenderContext() ->matVSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld() ) );


			// For each polygon specified create a primitive.
			RtInt	iP = 0;
			for ( poly = 0; poly < npolys; poly++ )
			{
				// Create a surface polygon
				CqMotionSurfacePointsPolygon*	pSurface = new CqMotionSurfacePointsPolygon( pPointsClass );
				RtBoolean fValid = RI_TRUE;

				pSurface->aIndices().resize( nverts[ poly ] );
				RtInt i;
				for ( i = 0; i < nverts[ poly ]; i++ )       	// Fill in the points
				{
					if ( verts[ iP ] >= cVerts )
					{
						fValid = RI_FALSE;
						CqAttributeError( 1, Severity_Normal, "Invalid PointsPolygon index", pSurface->pAttributes() );
						break;
					}
					pSurface->aIndices() [ i ] = verts[ iP ];
					iP++;
				}
				if ( fValid )
				{
					pSurface->AddTimeSlot( QGetRenderContext() ->ptransCurrent() ->Time( 0 ), pPointsClass );
					pPointsClass->AddRef();

					for ( i = 1; i < QGetRenderContext() ->ptransCurrent() ->cTimes(); i++ )
					{
						RtFloat time = QGetRenderContext() ->ptransCurrent() ->Time( i );
						pSurface->AddTimeSlot( time, apPoints[ i ] );
						apPoints[ i ] ->AddRef();
					}
					QGetRenderContext() ->pImage() ->PostSurface( pSurface );
				}
			}
			QGetRenderContext() ->Stats().IncGPrims();
		}
		pPointsClass->Release();
	}
	else
		delete( pPointsClass );

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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiPointsGeneralPolygonsV( npolys, nloops, nverts, verts, count, pTokens, pValues );

	return ;
}


//----------------------------------------------------------------------
// RiPointsGeneralPolygonsV
// List based version of above.
//
RtVoid	RiPointsGeneralPolygonsV( RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], PARAMETERLIST )
{
	TqInt ipoly;
	TqInt iloop;
	TqInt igloop = 0;
	TqInt cVerts = 0;
	TqInt igvert = 0;

	// Calculate how many points overall.
	RtInt* pVerts = verts;
	for( ipoly = 0; ipoly < npolys; ipoly++ )
	{
		for ( iloop = 0; iloop < nloops[ ipoly ]; iloop++, igloop++ )
		{	
			TqInt v;
			for ( v = 0; v < nverts[ igloop ]; v++ )
			{
				if ( ( ( *pVerts ) + 1 ) > cVerts )
					cVerts = ( *pVerts ) + 1;
				pVerts++;
			}
		}
	}

	// Create a storage class for all the points.
	CqPolygonPoints* pPointsClass = new CqPolygonPoints( cVerts );
	pPointsClass->AddRef();
	// Process any specified primitive variables
	pPointsClass->SetDefaultPrimitiveVariables( RI_FALSE );

	if ( ProcessPrimitiveVariables( pPointsClass, count, tokens, values ) )
	{
		// Reset loop counter.
		igloop = 0;
		TqUint ctris = 0;
		std::vector<TqInt>	aiTriangles;

		for( ipoly = 0; ipoly < npolys; ipoly++ )
		{
			// Create a general 2D polygon using the points in each loop.
			CqPolygonGeneral2D poly;
			TqUint ipoint = 0;
			for ( iloop = 0; iloop < nloops[ ipoly ]; iloop++, igloop++ )
			{
				TqFloat	MinX, MaxX;
				TqFloat	MinY, MaxY;
				TqFloat	MinZ, MaxZ;
				CqVector3D	vecTemp = pPointsClass->P() [ verts[ igvert ] ];
				MinX = MaxX = vecTemp.x();
				MinY = MaxY = vecTemp.y();
				MinZ = MaxZ = vecTemp.z();

				CqPolygonGeneral2D polya;
				polya.SetpVertices( pPointsClass );
				TqInt ivert;
				for ( ivert = 0; ivert < nverts[ igloop ]; ivert++, igvert++ )
				{
					ipoint = verts[ igvert ];
					assert( ipoint < pPointsClass->P().Size() );
					polya.aiVertices().push_back( ipoint );

					vecTemp = pPointsClass->P() [ verts[ igvert ] ];
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
					if ( polya.CalcOrientation() != CqPolygonGeneral2D::Orientation_AntiClockwise )
						polya.SwapDirection();
					poly = polya;
				}
				else
				{
					if ( polya.CalcOrientation() != CqPolygonGeneral2D::Orientation_Clockwise )
						polya.SwapDirection();
					poly.Combine( polya );
				}
			}
			// Now triangulate the general polygon

			poly.CalcOrientation();
			poly.Triangulate( aiTriangles );

		}
		pPointsClass->Release();

		// Build an array of point counts (always 3 each).
		ctris = aiTriangles.size() / 3;
		std::vector<RtInt> _nverts;
		_nverts.resize( ctris, 3 );

		RiPointsPolygonsV( ctris, &_nverts[ 0 ], &aiTriangles[ 0 ], count, tokens, values );
	}

	return ;
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
	for ( i = 0; i < 4; i++ )
	{
		RtInt j;
		for ( j = 0; j < 4; j++ )
		{
			u.SetElement( i, j, ubasis[ i ][ j ] );
			v.SetElement( i, j, vbasis[ i ][ j ] );
		}
	}
	u.SetfIdentity( TqFalse );
	v.SetfIdentity( TqFalse );

	QGetRenderContext() ->pattrWriteCurrent() ->SetmatuBasis( u, QGetRenderContext() ->Time() );
	QGetRenderContext() ->pattrWriteCurrent() ->SetmatvBasis( v, QGetRenderContext() ->Time() );
	QGetRenderContext() ->pattrWriteCurrent() ->SetuSteps( ustep, QGetRenderContext() ->Time() );
	QGetRenderContext() ->pattrWriteCurrent() ->SetvSteps( vstep, QGetRenderContext() ->Time() );
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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiPatchV( type, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiPatchV
// List based version of above.
//
RtVoid	RiPatchV( RtToken type, PARAMETERLIST )
{
	if ( strcmp( type, RI_BICUBIC ) == 0 )
	{
		// Create a surface patch
		CqSurfacePatchBicubic * pSurface = new CqSurfacePatchBicubic();
		pSurface->AddRef();
		// Fill in default values for all primitive variables not explicitly specified.
		pSurface->SetDefaultPrimitiveVariables();
		// Fill in primitive variables specified.
		if ( ProcessPrimitiveVariables( pSurface, count, tokens, values ) )
			CreateGPrim( pSurface );
		else
			pSurface->Release();
	}
	else if ( strcmp( type, RI_BILINEAR ) == 0 )
	{
		// Create a surface patch
		CqSurfacePatchBilinear * pSurface = new CqSurfacePatchBilinear();
		pSurface->AddRef();
		// Fill in default values for all primitive variables not explicitly specified.
		pSurface->SetDefaultPrimitiveVariables();
		// Fill in primitive variables specified.
		if ( ProcessPrimitiveVariables( pSurface, count, tokens, values ) )
			CreateGPrim( pSurface );
		else
			pSurface->Release();
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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiPatchMeshV( type, nu, uwrap, nv, vwrap, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiPatchMeshV
// List based version of above.
//

RtVoid	RiPatchMeshV( RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, PARAMETERLIST )
{
	if ( strcmp( type, RI_BICUBIC ) == 0 )
	{
		// Create a surface patch
		TqBool	uPeriodic = ( strcmp( uwrap, RI_PERIODIC ) == 0 ) ? TqTrue : TqFalse;
		TqBool	vPeriodic = ( strcmp( vwrap, RI_PERIODIC ) == 0 ) ? TqTrue : TqFalse;

		CqSurfacePatchMeshBicubic* pSurface = new CqSurfacePatchMeshBicubic( nu, nv, uPeriodic, vPeriodic );
		pSurface->AddRef();
		// Fill in default values for all primitive variables not explicitly specified.
		pSurface->SetDefaultPrimitiveVariables();
		// Fill in primitive variables specified.
		if ( ProcessPrimitiveVariables( pSurface, count, tokens, values ) )
			CreateGPrim( pSurface );
		else
			pSurface->Release();
	}
	else if ( strcmp( type, RI_BILINEAR ) == 0 )
	{
		// Create a surface patch
		TqBool	uPeriodic = ( strcmp( uwrap, RI_PERIODIC ) == 0 ) ? TqTrue : TqFalse;
		TqBool	vPeriodic = ( strcmp( vwrap, RI_PERIODIC ) == 0 ) ? TqTrue : TqFalse;

		CqSurfacePatchMeshBilinear* pSurface = new CqSurfacePatchMeshBilinear( nu, nv, uPeriodic, vPeriodic );
		pSurface->AddRef();
		// Fill in default values for all primitive variables not explicitly specified.
		pSurface->SetDefaultPrimitiveVariables();
		// Fill in primitive variables specified.
		if ( ProcessPrimitiveVariables( pSurface, count, tokens, values ) )
			CreateGPrim( pSurface );
		else
			pSurface->Release();
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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiNuPatchV( nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiNuPatchV
// List based version of above.
//
RtVoid	RiNuPatchV( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax,
                   RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, PARAMETERLIST )
{
	// Create a NURBS patch
	CqSurfaceNURBS * pSurface = new CqSurfaceNURBS();
	pSurface->AddRef();
	pSurface->Init( uorder, vorder, nu, nv );

	// Copy the knot vectors.
	RtInt i;
	for ( i = 0; i < nu + uorder; i++ ) pSurface->auKnots() [ i ] = uknot[ i ];
	for ( i = 0; i < nv + vorder; i++ ) pSurface->avKnots() [ i ] = vknot[ i ];

	// Set up the default primitive variables.
	pSurface->SetDefaultPrimitiveVariables();

	TqInt bUses = pSurface->Uses();

	if ( USES( bUses, EnvVars_u ) )
	{
		pSurface->u().pValue() [ 0 ] = pSurface->u().pValue() [ 2 ] = umin;
		pSurface->u().pValue() [ 1 ] = pSurface->u().pValue() [ 3 ] = umax;
	}

	if ( USES( bUses, EnvVars_v ) )
	{
		pSurface->v().pValue() [ 0 ] = pSurface->v().pValue() [ 1 ] = vmin;
		pSurface->v().pValue() [ 2 ] = pSurface->v().pValue() [ 3 ] = vmax;
	}

	// Process any specified parameters
	if ( ProcessPrimitiveVariables( pSurface, count, tokens, values ) )
	{
		// Clamp the surface to ensure non-periodic.
		pSurface->Clamp();
		CreateGPrim( pSurface );
	}
	else
		pSurface->Release();

	return ;
}

#include <strstream>

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

	for ( iloop = 0; iloop < nloops; iloop++ )
	{
		CqTrimLoop Loop;
		TqInt icurve;
		for ( icurve = 0; icurve < ncurves[ iloop ]; icurve++ )
		{
			// Create a NURBS patch
			CqTrimCurve Curve;
			TqInt o = order[ iorder++ ];
			TqInt cverts = n[ in++ ];
			Curve.Init( o, cverts );

			// Copy the knot vectors.
			RtInt i;
			for ( i = 0; i < o + cverts; i++ ) Curve.aKnots() [ i ] = knot[ iknot++ ];

			// Copy the vertices from the u,v,w arrays.
			CqVector3D vec( 0, 0, 1 );
			for ( i = 0; i < cverts; i++ )
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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiSphereV( radius, zmin, zmax, thetamax, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiSphereV
// List based version of above.
//
RtVoid	RiSphereV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST )
{
	// Create a sphere
	CqSphere * pSurface = new CqSphere( radius, zmin, zmax, 0, thetamax );
	pSurface->AddRef();
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables( pSurface, count, tokens, values );

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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiConeV( height, radius, thetamax, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiConeV
// List based version of above.
//
RtVoid	RiConeV( RtFloat height, RtFloat radius, RtFloat thetamax, PARAMETERLIST )
{
	// Create a cone
	CqCone * pSurface = new CqCone( height, radius, 0, thetamax, 0, height );
	pSurface->AddRef();
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables( pSurface, count, tokens, values );

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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiCylinderV( radius, zmin, zmax, thetamax, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiCylinderV
// List based version of above.
//
RtVoid	RiCylinderV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST )
{
	// Create a cylinder
	CqCylinder * pSurface = new CqCylinder( radius, zmin, zmax, 0, thetamax );
	pSurface->AddRef();
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables( pSurface, count, tokens, values );

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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiHyperboloidV( point1, point2, thetamax, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiHyperboloidV
// List based version of above.
//
RtVoid	RiHyperboloidV( RtPoint point1, RtPoint point2, RtFloat thetamax, PARAMETERLIST )
{
	// Create a hyperboloid
	CqVector3D v0( point1[ 0 ], point1[ 1 ], point1[ 2 ] );
	CqVector3D v1( point2[ 0 ], point2[ 1 ], point2[ 2 ] );
	CqHyperboloid* pSurface = new CqHyperboloid( v0, v1, 0, thetamax );
	pSurface->AddRef();
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables( pSurface, count, tokens, values );

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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiParaboloidV( rmax, zmin, zmax, thetamax, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiParaboloidV
// List based version of above.
//
RtVoid	RiParaboloidV( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST )
{
	// Create a paraboloid
	CqParaboloid * pSurface = new CqParaboloid( rmax, zmin, zmax, 0, thetamax );
	pSurface->AddRef();
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables( pSurface, count, tokens, values );

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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiDiskV( height, radius, thetamax, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiDiskV
// List based version of above.
//
RtVoid	RiDiskV( RtFloat height, RtFloat radius, RtFloat thetamax, PARAMETERLIST )
{
	// Create a disk
	CqDisk * pSurface = new CqDisk( height, 0, radius, 0, thetamax );
	pSurface->AddRef();
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables( pSurface, count, tokens, values );

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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiTorusV( majorrad, minorrad, phimin, phimax, thetamax, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiTorus
// Specify a torus primitive.
//
RtVoid	RiTorusV( RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, PARAMETERLIST )
{
	// Create a torus
	CqTorus * pSurface = new CqTorus( majorrad, minorrad, phimin, phimax, 0, thetamax );
	pSurface->AddRef();
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables( pSurface, count, tokens, values );

	CreateGPrim( pSurface );

	return ;
}


//----------------------------------------------------------------------
// RiProcedural
// Implement the procedural type primitive.
//
RtVoid	RiProcedural( RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc )
{
	CqBasicError( 0, Severity_Normal, "RiProcedural not supported" );
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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiGeometryV( type, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiGeometryV
// List based version of above.
//
RtVoid	RiGeometryV( RtToken type, PARAMETERLIST )
{
	if ( strcmp( type, "teapot" ) == 0 )
	{

		// Create a standard teapot
		CqTeapot * pSurface = new CqTeapot( true ); // add a bottom if true/false otherwise
		pSurface->AddRef();

		pSurface->SetDefaultPrimitiveVariables();
		pSurface->SetSurfaceParameters( *pSurface );
		ProcessPrimitiveVariables( pSurface, count, tokens, values );

		// I don't use the original teapot primitives as defined by T. Burge
		// but an array of Patch Bicubic (stolen from example from Pixar) and
		// those (6 meshes) are registered as standards GPrims right here.
		// Basically I kept the bound, transform and split, dice and diceable methods
		// in teapot.cpp but I suspect they are never called since the work of
		// dicing will rely on the registered Gprimitives (see below in the for loop).
		// I suspect the 6/7 meshes are equivalent in size/definition as the T. Burge
		// definition. The 7th is the bottom part of the teapot (see teapot.cpp).

		for ( int i = 0; i < pSurface->cNbrPatchMeshBicubic; i++ )
		{
			CqSurface *pMesh = pSurface->pPatchMeshBicubic[ i ];

			CreateGPrim( ( CqSurfacePatchMeshBicubic* ) pMesh );
		}
		pSurface->Release();
	}
	else if ( strcmp( type, "sphere" ) == 0 )
	{
		// Create a sphere
		CqSphere * pSurface = new CqSphere( 1, -1, 1, 0, 360.0 );
		pSurface->AddRef();
		pSurface->SetDefaultPrimitiveVariables();
		ProcessPrimitiveVariables( pSurface, count, tokens, values );
		CreateGPrim( pSurface );

	}
	else
	{
		CqBasicError( 0, Severity_Normal, "RiGeometryV, unknown geometry" );
	}

	return ;
}

//----------------------------------------------------------------------
// RiSolidBegin
// Begin the definition of a CSG object.
//
RtVoid	RiSolidBegin( RtToken type )
{
	CqString strType( type );
	QGetRenderContext() ->CreateSolidContext( strType );

	return ;
}


//----------------------------------------------------------------------
// RiSolidEnd
// End the definition of a CSG object.
//
RtVoid	RiSolidEnd()
{
	QGetRenderContext() ->DeleteSolidContext();

	return ;
}


//----------------------------------------------------------------------
// RiObjectBegin
// Begin the definition of a stored object for use by RiObjectInstance.
//
RtObjectHandle	RiObjectBegin()
{
	CqBasicError( 0, Severity_Normal, "RiObjectBegin, instances not supported" );
	QGetRenderContext() ->CreateObjectContext();

	return ( 0 );
}


//----------------------------------------------------------------------
// RiObjectEnd
// End the defintion of a stored object for use by RiObjectInstance.
//
RtVoid	RiObjectEnd()
{
	QGetRenderContext() ->DeleteObjectContext();

	return ;
}


//----------------------------------------------------------------------
// RiObjectInstance
// Instantiate a copt of a pre-stored geometric object.
//
RtVoid	RiObjectInstance( RtObjectHandle handle )
{
	CqBasicError( 0, Severity_Normal, "RiObjectInstance, instances not supported" );
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
	for ( i = 0; i < N; i++ )
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
	QGetRenderContext() ->CreateMotionContext( N, times );

	return ;
}


//----------------------------------------------------------------------
// RiMotionEnd
// End the definition of the motion of an object.
//
RtVoid	RiMotionEnd()
{
	QGetRenderContext() ->DeleteMotionContext();

	return ;
}


//----------------------------------------------------------------------
// RiMakeTexture
// Convert a picture to a texture.
//
RtVoid RiMakeTexture ( const char *pic, const char *tex, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... )
{
	va_list	pArgs;
	va_start( pArgs, twidth );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiMakeTextureV( pic, tex, swrap, twrap, filterfunc, swidth, twidth, count, pTokens, pValues );

}


//----------------------------------------------------------------------
// RiMakeTextureV
// List based version of above.
//
RtVoid	RiMakeTextureV( const char *pic, const char *tex, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST )
{
	char modes[ 1024 ];
	assert( pic != 0 && tex != 0 && swrap != 0 && twrap != 0 && filterfunc != 0 );

	QGetRenderContext() ->Stats().MakeTextureTimer().Start();
	// Get the wrap modes first.
	enum EqWrapMode smode = WrapMode_Clamp;
	if ( strcmp( swrap, RI_PERIODIC ) == 0 )
		smode = WrapMode_Periodic;
	else if ( strcmp( swrap, RI_CLAMP ) == 0 )
		smode = WrapMode_Clamp;
	else if ( strcmp( swrap, RI_BLACK ) == 0 )
		smode = WrapMode_Black;

	enum EqWrapMode tmode = WrapMode_Clamp;
	if ( strcmp( twrap, RI_PERIODIC ) == 0 )
		tmode = WrapMode_Periodic;
	else if ( strcmp( twrap, RI_CLAMP ) == 0 )
		tmode = WrapMode_Clamp;
	else if ( strcmp( twrap, RI_BLACK ) == 0 )
		tmode = WrapMode_Black;


	sprintf( modes, "%s %s %s %f %f", swrap, twrap, "box", swidth, twidth );
	if ( filterfunc == RiGaussianFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "gaussian", swidth, twidth );
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
	CqTextureMap Source( pic );
	Source.Open();

	if ( Source.IsValid() && Source.Format() == TexFormat_Plain )
	{
		// Hopefully CqTextureMap will take care of closing the tiff file after
		// it has SAT mapped it so we can overwrite if needs be.
		// Create a new image.
		Source.Interpreted( modes );
		Source.CreateMIPMAP();
		TIFF* ptex = TIFFOpen( tex, "w" );

		TIFFCreateDirectory( ptex );
		TIFFSetField( ptex, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
		TIFFSetField( ptex, TIFFTAG_PIXAR_TEXTUREFORMAT, MIPMAP_HEADER );
		TIFFSetField( ptex, TIFFTAG_PIXAR_WRAPMODES, modes );
		TIFFSetField( ptex, TIFFTAG_SAMPLESPERPIXEL, Source.SamplesPerPixel() );
		TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 8 );
		TIFFSetField( ptex, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS ); /* COMPRESSION_DEFLATE */
		int log2 = MIN( Source.XRes(), Source.YRes() );
		log2 = ( int ) ( log( log2 ) / log( 2.0 ) );


		for ( int i = 0; i < log2; i ++ )
		{
			// Write the floating point image to the directory.
			CqTextureMapBuffer* pBuffer = Source.GetBuffer( 0, 0, i );
			if ( !pBuffer ) break;
			Source.WriteTileImage( ptex, pBuffer->pBufferData(), Source.XRes() / ( 1 << i ), Source.YRes() / ( 1 << i ), 64, 64, Source.SamplesPerPixel() );
		}
		TIFFClose( ptex );
	}

	Source.Close();
	QGetRenderContext() ->Stats().MakeTextureTimer().Stop();
}


//----------------------------------------------------------------------
// RiMakeBump
// Convert a picture to a bump map.
//
RtVoid	RiMakeBump( const char *imagefile, const char *bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... )
{
	CqBasicError( 0, Severity_Normal, "RiMakeBump not supported" );
	return ;
}


//----------------------------------------------------------------------
// RiMakeBumpV
// List based version of above.
//
RtVoid	RiMakeBumpV( const char *imagefile, const char *bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST )
{
	CqBasicError( 0, Severity_Normal, "RiMakeBump not supported" );
	return ;
}


//----------------------------------------------------------------------
// RiMakeLatLongEnvironment
// Convert a picture to an environment map.
//
RtVoid	RiMakeLatLongEnvironment( const char *imagefile, const char *reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... )
{
	va_list	pArgs;
	va_start( pArgs, twidth );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiMakeLatLongEnvironmentV( imagefile, reflfile, filterfunc, swidth, twidth, count, pTokens, pValues );
	return ;
}


//----------------------------------------------------------------------
// RiMakeLatLongEnvironmentV
// List based version of above.
//
RtVoid	RiMakeLatLongEnvironmentV( const char *pic, const char *tex, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST )
{
	char modes[ 1024 ];
	char *swrap = "periodic";
	char *twrap = "clamp";

	assert( pic != 0 && tex != 0 && swrap != 0 && twrap != 0 && filterfunc != 0 );

	QGetRenderContext() ->Stats().MakeEnvTimer().Start();

	sprintf( modes, "%s %s %s %f %f", swrap, twrap, "box", swidth, twidth );
	if ( filterfunc == RiGaussianFilter )
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "gaussian", swidth, twidth );
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
	CqTextureMap Source( pic );
	Source.Open();

	if ( Source.IsValid() && Source.Format() == TexFormat_Plain )
	{
		// Hopefully CqTextureMap will take care of closing the tiff file after
		// it has SAT mapped it so we can overwrite if needs be.
		// Create a new image.
		Source.Interpreted( modes );
		Source.CreateMIPMAP();
		TIFF* ptex = TIFFOpen( tex, "w" );

		TIFFCreateDirectory( ptex );
		TIFFSetField( ptex, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
		TIFFSetField( ptex, TIFFTAG_PIXAR_TEXTUREFORMAT, LATLONG_HEADER );
		TIFFSetField( ptex, TIFFTAG_PIXAR_WRAPMODES, modes );
		TIFFSetField( ptex, TIFFTAG_SAMPLESPERPIXEL, Source.SamplesPerPixel() );
		TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 8 );
		TIFFSetField( ptex, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS ); /* COMPRESSION_DEFLATE */
		int log2 = MIN( Source.XRes(), Source.YRes() );
		log2 = ( int ) ( log( log2 ) / log( 2.0 ) );


		for ( int i = 0; i < log2; i ++ )
		{
			// Write the floating point image to the directory.
			CqTextureMapBuffer* pBuffer = Source.GetBuffer( 0, 0, i );
			if ( !pBuffer ) break;
			Source.WriteTileImage( ptex, pBuffer->pBufferData(), Source.XRes() / ( 1 << i ), Source.YRes() / ( 1 << i ), 64, 64, Source.SamplesPerPixel() );
		}
		TIFFClose( ptex );
	}

	Source.Close();
	QGetRenderContext() ->Stats().MakeEnvTimer().Stop();
	return ;
}


//----------------------------------------------------------------------
// RiMakeCubeFaceEnvironment
// Convert a picture to a cubical environment map.
//
RtVoid	RiMakeCubeFaceEnvironment( const char *px, const char *nx, const char *py, const char *ny, const char *pz, const char *nz, const char *reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... )
{
	va_list	pArgs;
	va_start( pArgs, twidth );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiMakeCubeFaceEnvironmentV( px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiMakeCubeFaceEnvironment
// List based version of above.
//
RtVoid	RiMakeCubeFaceEnvironmentV( const char *px, const char *nx, const char *py, const char *ny, const char *pz, const char *nz, const char *reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST )
{
	QGetRenderContext() ->Stats().MakeEnvTimer().Start();
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

	if ( tpx.Format() != TexFormat_MIPMAP ) tpx.CreateMIPMAP();
	if ( tnx.Format() != TexFormat_MIPMAP ) tnx.CreateMIPMAP();
	if ( tpy.Format() != TexFormat_MIPMAP ) tpy.CreateMIPMAP();
	if ( tny.Format() != TexFormat_MIPMAP ) tny.CreateMIPMAP();
	if ( tpz.Format() != TexFormat_MIPMAP ) tpz.CreateMIPMAP();
	if ( tnz.Format() != TexFormat_MIPMAP ) tnz.CreateMIPMAP();
	if ( tpx.IsValid() && tnx.IsValid() && tpy.IsValid() && tny.IsValid() && tpz.IsValid() && tnz.IsValid() )
	{
		// Check all the same size;
		bool fValid = false;
		if ( tpx.XRes() == tnx.XRes() && tpx.XRes() == tpy.XRes() && tpx.XRes() == tny.XRes() && tpx.XRes() == tpz.XRes() && tpx.XRes() == tnz.XRes() &&
		        tpx.XRes() == tnx.XRes() && tpx.XRes() == tpy.XRes() && tpx.XRes() == tny.XRes() && tpx.XRes() == tpz.XRes() && tpx.XRes() == tnz.XRes() )
			fValid = true;

		if ( !fValid )
		{
			CqBasicError( ErrorID_InvalidData, Severity_Normal, "RiMakeCubeFaceEnvironment : Images not the same size" );
			return ;
		}

		// Now copy the images to the big map.
		CqTextureMap* Images[ 6 ] =
		    {
		        {&tpz},
		        {&tpx},
		        {&tpy},
		        {&tnx},
		        {&tny},
		        {&tnz}
		    };

		// Create a new image.
		TIFF* ptex = TIFFOpen( reflfile, "w" );

		RtInt ii;
		TqInt xRes = tpx.XRes();
		TqInt yRes = tpx.YRes();

		// Number of mip map levels.
		int log2 = MIN( xRes, yRes );
		log2 = ( int ) ( log( log2 ) / log( 2.0 ) );

		for ( ii = 0; ii < log2; ii++ )
		{

			TqUchar* pImage = new TqUchar[ ( xRes * 3 ) * ( yRes * 2 ) * tpx.SamplesPerPixel() * sizeof( TqUchar ) ];
			TqInt linelen = ( xRes * 3 ) * tpx.SamplesPerPixel() * sizeof( TqUchar );
			TqInt viewlinelen = xRes * tpx.SamplesPerPixel() * sizeof( TqUchar );

			TqInt view = 0;
			for ( view = 0; view < 6; view++ )
			{
				CqTextureMapBuffer* pBuffer = Images[ view ] ->GetBuffer( 0, 0, ii );
				TqInt xoff = view % 3;
				xoff *= viewlinelen;
				TqInt yoff = view / 3;
				yoff *= yRes * linelen;
				TqUchar* ptr = pImage + xoff + yoff;
				TqInt line;
				for ( line = 0; line < yRes; line++ )
				{
					memcpy( ptr, ( pBuffer->pBufferData() + ( line * viewlinelen ) ), viewlinelen );
					ptr += linelen;
				}
			}

			TIFFCreateDirectory( ptex );
			TIFFSetField( ptex, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
			TIFFSetField( ptex, TIFFTAG_PIXAR_TEXTUREFORMAT, CUBEENVMAP_HEADER );
			tpx.WriteTileImage( ptex, pImage, ( xRes * 3 ), ( yRes * 2 ), 64, 64, tpx.SamplesPerPixel() );
			xRes /= 2;
			yRes /= 2;
		}
		TIFFClose( ptex );
	}
	QGetRenderContext() ->Stats().MakeEnvTimer().Stop();
	return ;
}


//----------------------------------------------------------------------
// RiMakeShadow
// Convert a depth map file to a shadow map.
//
RtVoid	RiMakeShadow( const char *picfile, const char *shadowfile, ... )
{
	va_list	pArgs;
	va_start( pArgs, shadowfile );

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiMakeShadowV( picfile, shadowfile, count, pTokens, pValues );
}


//----------------------------------------------------------------------
// RiMakeShadowV
// List based version of above.
//
RtVoid	RiMakeShadowV( const char *picfile, const char *shadowfile, PARAMETERLIST )
{
	QGetRenderContext() ->Stats().MakeShadowTimer().Start();
	CqShadowMap ZFile( picfile );
	ZFile.LoadZFile();
	ZFile.SaveShadowMap( shadowfile );
	QGetRenderContext() ->Stats().MakeShadowTimer().Stop();
	return ;
}


//----------------------------------------------------------------------
// RiErrorHandler
// Set the function used to report errors.
//
RtVoid	RiErrorHandler( RtErrorFunc handler )
{
	QGetRenderContext() ->optCurrent().SetpErrorHandler( handler );
	return ;
}


//----------------------------------------------------------------------
// RiErrorIgnore
// Function used by RiErrorHandler to continue after errors.
//
RtVoid	RiErrorIgnore( RtInt code, RtInt severity, const char* message )
{
	return ;
}


//----------------------------------------------------------------------
// RiErrorPrint
// Function used by RiErrorHandler to print an error message to stdout and continue.
//
RtVoid	RiErrorPrint( RtInt code, RtInt severity, const char* message )
{
	QGetRenderContext() ->PrintMessage( SqMessage( code, severity, message ) );
	return ;
}


//----------------------------------------------------------------------
// RiErrorAbort
// Function used by RiErrorHandler to print and error and stop.
//
RtVoid	RiErrorAbort( RtInt code, RtInt severity, const char* message )
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

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count = BuildParameterList( pArgs, pTokens, pValues );

	RiSubdivisionMeshV( scheme, nfaces, nvertices, vertices, ntags, tags, nargs, intargs, floatargs, count, pTokens, pValues );
}

//----------------------------------------------------------------------
// RiSubdivisionMeshV
// List based version of above.
//
RtVoid	RiSubdivisionMeshV( RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], PARAMETERLIST )
{
	// Calculate how many vertices there are.
	RtInt cVerts = 0;
	RtInt* pVerts = vertices;
	RtInt face;
	for ( face = 0; face < nfaces; face++ )
	{
		RtInt v;
		for ( v = 0; v < nvertices[ face ]; v++ )
		{
			if ( ( ( *pVerts ) + 1 ) > cVerts )
				cVerts = ( *pVerts ) + 1;
			pVerts++;
		}
	}

	// Create a storage class for all the points.
	CqPolygonPoints* pPointsClass = new CqPolygonPoints( cVerts );
	pPointsClass->AddRef();

	CqWSurf* pSubdivision = NULL;
	CqMotionWSurf* pMotionSubdivision = NULL;
	CqSubdivider* pSubdivider = NULL;
	const CqAttributes* pAttr;

	std::vector<CqPolygonPoints*>	apPoints;
	// Process any specified primitive variables
	pPointsClass->SetDefaultPrimitiveVariables( RI_FALSE );
	if ( ProcessPrimitiveVariables( pPointsClass, count, tokens, values ) )
	{
		// Check if s/t, u/v are needed and not specified, and if so get them from the object space points.
		pPointsClass->TransferDefaultSurfaceParameters();

		if ( QGetRenderContext() ->ptransCurrent() ->cTimes() <= 1 )
		{
			// Transform the points into camera space for processing,
			pPointsClass->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld() ),
			                         QGetRenderContext() ->matNSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld() ),
			                         QGetRenderContext() ->matVSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld() ) );

			// Create the subdivision structure class.
			pSubdivider = pSubdivision = new CqWSurf( cVerts, nfaces, pPointsClass );
			pAttr = pSubdivision->pAttributes();
		}
		else
		{
			// Create the subdivision structure class.
			pSubdivider = pMotionSubdivision = new CqMotionWSurf( cVerts, nfaces, pPointsClass );
			pAttr = pMotionSubdivision->pAttributes();

			TqInt i;
			apPoints.push_back( pPointsClass );
			for ( i = 1; i < QGetRenderContext() ->ptransCurrent() ->cTimes(); i++ )
			{
				RtFloat time = QGetRenderContext() ->ptransCurrent() ->Time( i );
				CqPolygonPoints* pPointsClass2 = new CqPolygonPoints( *pPointsClass );

				// Transform the points into camera space for processing,
				pPointsClass2->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass2->pTransform() ->matObjectToWorld( time ) ),
				                          QGetRenderContext() ->matNSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass2->pTransform() ->matObjectToWorld( time ) ),
				                          QGetRenderContext() ->matVSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass2->pTransform() ->matObjectToWorld( time ) ) );
				apPoints.push_back( pPointsClass2 );
			}
			pPointsClass->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld() ),
			                         QGetRenderContext() ->matNSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld() ),
			                         QGetRenderContext() ->matVSpaceToSpace( "object", "camera", CqMatrix(), pPointsClass->pTransform() ->matObjectToWorld() ) );
		}
		// Intitialise the vertices of the hull
		int i;
		for ( i = 0; i < cVerts; i++ )
			pSubdivider->AddVert( new CqWVert( i ) );

		// Add the faces to the hull.
		std::vector<CqWEdge*> apEdges;
		RtInt	iP = 0;
		RtInt	iPStart = 0;
		for ( face = 0; face < nfaces; face++ )
		{
			// Create a surface face
			RtBoolean fValid = RI_TRUE;
			RtInt i;
			for ( i = 0; i < nvertices[ face ]; i++ )       	// Fill in the points
			{
				if ( vertices[ iP ] >= cVerts )
				{
					fValid = RI_FALSE;
					CqAttributeError( 1, Severity_Normal, "Invalid PointsPolygon index", pSubdivision->pAttributes() );
					break;
				}

				if ( i < nvertices[ face ] - 1 )
				{
					CqWEdge * pE = pSubdivider->AddEdge( pSubdivider->pVert( vertices[ iP ] ), pSubdivider->pVert( vertices[ iP + 1 ] ) );
					if ( pE == NULL )
					{
						delete( pSubdivider );
						return ;
					}
					apEdges.push_back( pE );
				}
				else
				{
					CqWEdge* pE = pSubdivider->AddEdge( pSubdivider->pVert( vertices[ iP ] ), pSubdivider->pVert( vertices[ iPStart ] ) );
					if ( pE == NULL )
					{
						delete( pSubdivider );
						return ;
					}
					apEdges.push_back( pE );
				}
				iP++;
			}
			if ( fValid )
			{
				// If NULL returned, an error was encountered.
				if ( pSubdivider->AddFace( &apEdges[ 0 ], nvertices[ face ] ) == 0 )
				{
					delete( pSubdivider );
					return ;
				}
			}
			apEdges.clear();
			iPStart = iP;
		}

		// Set up points references according to motion details.
		pPointsClass->AddRef();
		if ( QGetRenderContext() ->ptransCurrent() ->cTimes() > 1 )
		{
			pMotionSubdivision->AddTimeSlot( QGetRenderContext() ->ptransCurrent() ->Time( 0 ), pPointsClass );

			for ( i = 1; i < QGetRenderContext() ->ptransCurrent() ->cTimes(); i++ )
			{
				RtFloat time = QGetRenderContext() ->ptransCurrent() ->Time( i );
				pMotionSubdivision->AddTimeSlot( time, apPoints[ i ] );
				apPoints[ i ] ->AddRef();
			}
			QGetRenderContext() ->pImage() ->PostSurface( pMotionSubdivision );
			QGetRenderContext() ->Stats().IncGPrims();
		}
		else
		{
			QGetRenderContext() ->pImage() ->PostSurface( pSubdivision );
			QGetRenderContext() ->Stats().IncGPrims();
		}
	}

	// Process tags.
	for ( TqInt i = 0; i < ntags; i++ )
	{
		if ( strcmp( tags[ i ], "interpolateboundary" ) == 0 )
			pSubdivider->InterpolateBoundary( TqTrue );
	}

	return ;
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
// Helper functions

//----------------------------------------------------------------------
// ProcessPrimitiveVariables
// Process and fill in any primitive variables.
// return	:	RI_TRUE if position specified, RI_FALSE otherwise.

static RtBoolean ProcessPrimitiveVariables( CqSurface* pSurface, PARAMETERLIST )
{
	// Read recognised parameter values.
	RtInt	fP = RIL_NONE;
	RtInt	fN = RIL_NONE;
	RtInt	fT = RIL_NONE;
	RtBoolean	fCs = RI_FALSE;
	RtBoolean	fOs = RI_FALSE;

	RtFloat*	pPoints = 0;
	RtFloat*	pNormals = 0;
	RtFloat*	pTextures_s = 0;
	RtFloat*	pTextures_t = 0;
	RtFloat*	pTextures_st = 0;
	RtFloat*	pCs = 0;
	RtFloat*	pOs = 0;
	RtInt i;
	for ( i = 0; i < count; i++ )
	{
		RtToken	token = tokens[ i ];
		RtPointer	value = values[ i ];

		if ( strcmp( token, RI_S ) == 0 )
		{
			fT = RIL_s;
			pTextures_s = ( RtFloat* ) value;
		}
		else if ( strcmp( token, RI_T ) == 0 )
		{
			fT = RIL_t;
			pTextures_t = ( RtFloat* ) value;
		}
		else if ( strcmp( token, RI_ST ) == 0 )
		{
			fT = RIL_st;
			pTextures_st = ( RtFloat* ) value;
		}
		else if ( strcmp( token, RI_CS ) == 0 )
		{
			fCs = RI_TRUE;
			pCs = ( RtFloat* ) value;
		}
		else if ( strcmp( token, RI_OS ) == 0 )
		{
			fOs = RI_TRUE;
			pOs = ( RtFloat* ) value;
		}
		else if ( strcmp( token, RI_P ) == 0 )
		{
			fP = RIL_P;
			pPoints = ( RtFloat* ) value;
		}
		else if ( strcmp( token, RI_PZ ) == 0 )
		{
			fP = RIL_Pz;
			pPoints = ( RtFloat* ) value;
		}
		else if ( strcmp( token, RI_PW ) == 0 )
		{
			fP = RIL_Pw;
			pPoints = ( RtFloat* ) value;
		}
		else if ( strcmp( token, RI_N ) == 0 )
		{
			fN = RIL_N;
			pNormals = ( RtFloat* ) value;
		}
		else if ( strcmp( token, RI_NP ) == 0 )
		{
			fN = RIL_Np;
			pNormals = ( RtFloat* ) value;
		}
	}

	// Fill in the position variable according to type.
	if ( fP != RIL_NONE )
	{
		TqInt i;
		switch ( fP )
		{
				case RIL_P:
				pSurface->P().SetSize( pSurface->cVertex() );
				for ( i = 0; i < pSurface->cVertex(); i++ )
					pSurface->P() [ i ] = CqVector3D( pPoints[ ( i * 3 ) ], pPoints[ ( i * 3 ) + 1 ], pPoints[ ( i * 3 ) + 2 ] );
				break;

				case RIL_Pz:
				pSurface->P().SetSize( pSurface->cVertex() );
				for ( i = 0; i < pSurface->cVertex(); i++ )
					pSurface->P() [ i ] = CqVector3D( i % 1, FLOOR( i / 2 ), pPoints[ i ] );
				break;

				case RIL_Pw:
				pSurface->P().SetSize( pSurface->cVertex() );
				for ( i = 0; i < pSurface->cVertex(); i++ )
					pSurface->P() [ i ] = CqVector4D( pPoints[ ( i * 4 ) ], pPoints[ ( i * 4 ) + 1 ], pPoints[ ( i * 4 ) + 2 ], pPoints[ ( i * 4 ) + 3 ] );
				break;
		}
	}


	// Fill in the position variable according to type.
	if ( fN != RIL_NONE )
	{
		TqUint i;
		switch ( fN )
		{
				case RIL_N:
				pSurface->N().SetSize( pSurface->cVarying() );
				for ( i = 0; i < pSurface->cVarying(); i++ )
					pSurface->N() [ i ] = CqVector3D( pNormals[ ( i * 3 ) ], pNormals[ ( i * 3 ) + 1 ], pNormals[ ( i * 3 ) + 2 ] );
				break;

				case RIL_Np:
				pSurface->N().SetSize( pSurface->cUniform() );
				for ( i = 0; i < pSurface->cUniform(); i++ )
					pSurface->N() [ i ] = CqVector3D( pNormals[ ( i * 3 ) ], pNormals[ ( i * 3 ) + 1 ], pNormals[ ( i * 3 ) + 2 ] );
				break;
		}
	}


	// Copy any specified texture coordinates to the surface.
	if ( fT != RIL_NONE )
	{
		TqUint i;
		switch ( fT )
		{
				case RIL_s:
				if ( pTextures_s != 0 )
				{
					pSurface->s().SetSize( pSurface->cVarying() );
					for ( i = 0; i < pSurface->cVarying(); i++ )
						pSurface->s() [ i ] = pTextures_s[ i ];
				}

				if ( pTextures_t != 0 )
				{
					pSurface->t().SetSize( pSurface->cVarying() );
					for ( i = 0; i < pSurface->cVarying(); i++ )
						pSurface->t() [ i ] = pTextures_t[ i ];
				}
				break;

				case RIL_st:
				assert( pTextures_st != 0 );
				pSurface->s().SetSize( pSurface->cVarying() );
				pSurface->t().SetSize( pSurface->cVarying() );
				for ( i = 0; i < pSurface->cVarying(); i++ )
				{
					pSurface->s() [ i ] = pTextures_st[ ( i * 2 ) ];
					pSurface->t() [ i ] = pTextures_st[ ( i * 2 ) + 1 ];
				}
				break;
		}
	}

	// Copy any specified varying color values to the surface
	if ( fCs )
	{
		TqUint i;
		pSurface->Cs().SetSize( pSurface->cVarying() );
		for ( i = 0; i < pSurface->cVarying(); i++ )
			pSurface->Cs() [ i ] = CqColor( pCs[ ( i * 3 ) ], pCs[ ( i * 3 ) + 1 ], pCs[ ( i * 3 ) + 2 ] );
	}

	if ( fOs )
	{
		TqUint i;
		pSurface->Os().SetSize( pSurface->cVarying() );
		for ( i = 0; i < pSurface->cVarying(); i++ )
			pSurface->Os() [ i ] = CqColor( pOs[ ( i * 3 ) ], pOs[ ( i * 3 ) + 1 ], pOs[ ( i * 3 ) + 2 ] );
	}
	return ( fP != RIL_NONE );
}


//----------------------------------------------------------------------
// CreateGPrin
// Create and register a GPrim according to the current attributes/transform
//
template <class T>
RtVoid	CreateGPrim( T* pSurface )
{
	if ( QGetRenderContext() ->ptransCurrent() ->cTimes() <= 1 )
	{
		// Transform the points into camera space for processing,
		pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "camera", CqMatrix(), pSurface->pTransform() ->matObjectToWorld() ),
		                     QGetRenderContext() ->matNSpaceToSpace( "object", "camera", CqMatrix(), pSurface->pTransform() ->matObjectToWorld() ),
		                     QGetRenderContext() ->matVSpaceToSpace( "object", "camera", CqMatrix(), pSurface->pTransform() ->matObjectToWorld() ) );

		pSurface->PrepareTrimCurve();
		QGetRenderContext() ->pImage() ->PostSurface( pSurface );
		QGetRenderContext() ->Stats().IncGPrims();
	}
	else
	{
		CqMotionSurface<T*>* pMotionSurface = new CqMotionSurface<T*>( 0 );
		// Add the provided surface as time 0, we will transform it later.
		pMotionSurface->AddTimeSlot( QGetRenderContext() ->ptransCurrent() ->Time( 0 ), pSurface );
		RtInt i;
		for ( i = 1; i < QGetRenderContext() ->ptransCurrent() ->cTimes(); i++ )
		{
			RtFloat time = QGetRenderContext() ->ptransCurrent() ->Time( i );

			T* pTimeSurface = new T( *pSurface );
			pMotionSurface->AddTimeSlot( time, pTimeSurface );

			// Transform the points into camera space for processing,
			pTimeSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "camera", CqMatrix(), pTimeSurface->pTransform() ->matObjectToWorld( time ) ),
			                         QGetRenderContext() ->matNSpaceToSpace( "object", "camera", CqMatrix(), pTimeSurface->pTransform() ->matObjectToWorld( time ) ),
			                         QGetRenderContext() ->matVSpaceToSpace( "object", "camera", CqMatrix(), pTimeSurface->pTransform() ->matObjectToWorld( time ) ) );
		}
		// Transform the points into camera space for processing,
		pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "object", "camera", CqMatrix(), pSurface->pTransform() ->matObjectToWorld() ),
		                     QGetRenderContext() ->matNSpaceToSpace( "object", "camera", CqMatrix(), pSurface->pTransform() ->matObjectToWorld() ),
		                     QGetRenderContext() ->matVSpaceToSpace( "object", "camera", CqMatrix(), pSurface->pTransform() ->matObjectToWorld() ) );
		pSurface->PrepareTrimCurve();
		QGetRenderContext() ->pImage() ->PostSurface( pMotionSurface );
		QGetRenderContext() ->Stats().IncGPrims();
	}

	return ;
}


//----------------------------------------------------------------------
/** Get the basis matrix given a standard basis name.
 * \param b Storage for basis matrix.
 * \param strName Name of basis.
 * \return Boolean indicating the basis is valid.
 */

RtBoolean	BasisFromName( RtBasis* b, const char* strName )
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
		for ( i = 0; i < 4; i++ )
			for ( j = 0; j < 4; j++ )
				( *b ) [ i ][ j ] = ( *pVals ) [ i ][ j ];
		return ( TqTrue );
	}
	return ( TqFalse );
}


//----------------------------------------------------------------------
/** Set the function used to report progress.
 
	\param	handler	Pointer to the new function to use.
 */

RtVoid	RiProgressHandler( RtProgressFunc handler )
{
	QGetRenderContext() ->optCurrent().SetpProgressHandler( handler );
	return ;
}


//----------------------------------------------------------------------
/** Set the function called just prior to rendering, after the world is complete.
 
	\param	function	Pointer to the new function to use.
 
	\return	Pointer to the old function.
 */

RtFunc	RiPreRenderFunction( RtFunc function )
{
	RtFunc pOldPreRenderFunction = QGetRenderContext() ->optCurrent().pPreRenderFunction();
	QGetRenderContext() ->optCurrent().SetpPreRenderFunction( function );
	return ( pOldPreRenderFunction );
}
