// Aqsis
// Copyright 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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
 * \brief Implements the RenderMan C-Style API Functions for writing a rib stream
 **/

#include <iostream>
#include <stdarg.h>
#include <string>
#include <stdio.h>
#include <aqsis/aqsis.h>
#include <aqsis/ri/ri.h>
#include <aqsis/riutil/ri_convenience.h>
//#include <aqsis/util/exception.h>
#include "error.h"
#include "plstore.h"
#include "context.h"

namespace libri2rib
{
static CqContext context;
} // namespace libri2rib

using libri2rib::context;
using libri2rib::CqPLStore;
using libri2rib::CqError;

//------------------------------------------------------------------------------

/// Exception try guard to be inserted at the top of all Ri calls which intend
//  to catch exceptions.
#define RI2RIB_EXCEPTION_TRY_GUARD try {

/// Exception catch guard to prevent exceptions propagating outside of Ri calls
#define RI2RIB_EXCEPTION_CATCH_GUARD(procName, returnValue)                    \
}                                                                              \
catch (CqError& e)                                                             \
{                                                                              \
	e.manage();                                                                \
	return returnValue;                                                        \
}

// Note: The body of these functions must be unique, otherwise the VC
// optimiser combines them into a single function, which causes problems when
// identifying the requested filter from the function address.

RtFloat RiGaussianFilter  (RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
	return 1.0;
}

RtFloat RiMitchellFilter  (RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
	return 2.0;
}

RtFloat RiBoxFilter       (RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
	return 3.0;
}

RtFloat RiTriangleFilter  (RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
	return 4.0;
}

RtFloat RiCatmullRomFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
	return 5.0;
}

RtFloat RiSincFilter      (RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
	return 6.0;
}

RtFloat RiDiskFilter      (RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
	return 7.0;
}

RtFloat RiBesselFilter    (RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
	return 8.0;
}

// \note In this section the required RenderMan Interface basis matrices (used
// for bicubic patches).

RtBasis RiBezierBasis =
{
	{ -1,  3, -3, 1},
	{  3, -6,  3, 0},
	{ -3,  3,  0, 0},
	{  1,  0,  0, 0}
};

RtBasis RiBSplineBasis =
{
	{ -1.0 / 6,  3.0 / 6, -3.0 / 6, 1.0 / 6},
	{  3.0 / 6, -6.0 / 6,  3.0 / 6, 0.0 / 6},
	{ -3.0 / 6,  0.0 / 6,  3.0 / 6, 0.0 / 6},
	{  1.0 / 6,  4.0 / 6,  1.0 / 6, 0.0 / 6}
};

RtBasis RiCatmullRomBasis =
{
	{ -1.0 / 2,  3.0 / 2, -3.0 / 2,  1.0 / 2},
	{  2.0 / 2, -5.0 / 2,  4.0 / 2, -1.0 / 2},
	{ -1.0 / 2,  0.0 / 2,  1.0 / 2,  0.0 / 2},
	{  0.0 / 2,  2.0 / 2,  0.0 / 2,  0.0 / 2}
};

RtBasis RiHermiteBasis =
{
	{  2,  1, -2,  1},
	{ -3, -2,  3, -1},
	{  0,  1,  0,  0},
	{  1,  0,  0,  0}
};

RtBasis RiPowerBasis =
{
	{ 1, 0, 0, 0},
	{ 0, 1, 0, 0},
	{ 0, 0, 1, 0},
	{ 0, 0, 0, 1}
};

//==============================================================================
// RenderMan C-Style API Functions
//==============================================================================

RtVoid RiProcDelayedReadArchive( RtPointer data, RtFloat detail )
{
	std::cout << "RiProcDelayedReadArchive" << std::endl;
}
RtVoid RiProcRunProgram( RtPointer data, RtFloat detail )
{
	std::cout << "RiProcRunProgram" << std::endl;
}
RtVoid RiProcDynamicLoad( RtPointer data, RtFloat detail )
{
	std::cout << "RiProcDynamicLoad" << std::endl;
}

extern "C" RtVoid RiProcFree( RtPointer data )
{
}

RtContextHandle RiGetContext( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	return context.getContext();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiGetContext", RI_NULL)
}

RtVoid RiContext( RtContextHandle ch )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.switchTo( ch );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiContext", )
}

RtToken RiDeclare( RtToken name, RtToken declaration )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	return context.current().RiDeclare( name, declaration );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiDeclare", RI_NULL)
}

RtVoid RiBegin( RtToken name )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.addContext( name );
	context.current().RiBegin( name );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiBegin", )
}

RtVoid RiEnd( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiEnd();
	context.removeCurrent();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiEnd", )
}

RtVoid RiFrameBegin( RtInt frame )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiFrameBegin( frame );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiFrameBegin", )
}

RtVoid RiFrameEnd( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiFrameEnd();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiFrameEnd", )
}

RtVoid RiWorldBegin( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiWorldBegin();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiWorldBegin", )
}

RtVoid RiWorldEnd( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiWorldEnd();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiWorldEnd", )
}

RtVoid RiFormat( RtInt xres, RtInt yres, RtFloat aspect )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiFormat( xres, yres, aspect );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiFormat", )
}

RtVoid RiFrameAspectRatio( RtFloat aspect )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiFrameAspectRatio( aspect );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiFrameAspectRatio", )
}

RtVoid RiScreenWindow( RtFloat left, RtFloat right, RtFloat bot, RtFloat top )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiScreenWindow( left, right, bot, top );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiScreenWindow", )
}

RtVoid RiCropWindow( RtFloat xmin, RtFloat xmax, RtFloat ymin, RtFloat ymax )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiCropWindow( xmin, xmax, ymin, ymax );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiCropWindow", )
}

RtVoid RiProjection( RtToken name, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	if(NULL != name)
	{
		AQSIS_COLLECT_RI_PARAMETERS( name )
		RiProjectionV( name, AQSIS_PASS_RI_PARAMETERS );
	}
	else
	{
		RiProjectionV( name, 0, NULL, NULL );
	}
	RI2RIB_EXCEPTION_CATCH_GUARD("RiProjection", )
}

RtVoid RiProjectionV( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiProjectionV( name, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiProjectionV", )
}

RtVoid RiClipping( RtFloat hither, RtFloat yon )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiClipping( hither, yon );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiClipping", )
}

RtVoid RiClippingPlane( RtFloat x, RtFloat y, RtFloat z,
                        RtFloat nx, RtFloat ny, RtFloat nz )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiClippingPlane( x, y, z, nx, ny, nz );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiClippingPlane", )
}

RtVoid RiDepthOfField( RtFloat fstop, RtFloat focallength, RtFloat focaldistance )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiDepthOfField ( fstop, focallength, focaldistance );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiDepthOfField", )
}

RtVoid RiShutter( RtFloat min, RtFloat max )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiShutter( min, max );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiShutter", )
}

RtVoid RiPixelVariance( RtFloat variation )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiPixelVariance( variation );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPixelVariance", )
}

RtVoid RiPixelSamples( RtFloat xsamples, RtFloat ysamples )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiPixelSamples( xsamples, ysamples );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPixelSamples", )
}

RtVoid RiPixelFilter( RtFilterFunc filterfunc, RtFloat xwidth, RtFloat ywidth )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiPixelFilter( filterfunc, xwidth, ywidth );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPixelFilter", )
}

RtVoid RiExposure( RtFloat gain, RtFloat gamma )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiExposure( gain, gamma );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiExposure", )
}

RtVoid RiImager( RtToken name, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS( name )
	RiImagerV( name, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiImager", )
}

RtVoid RiImagerV( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiImagerV( name, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiImagerV", )
}

RtVoid RiQuantize( RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ampl )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiQuantize( type, one, min, max, ampl );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiQuantize", )
}

RtVoid RiDisplay( RtToken name, RtToken type, RtToken mode, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS( mode )
	RiDisplayV( name, type, mode, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiDisplay", )
}

RtVoid RiDisplayV( RtToken name, RtToken type, RtToken mode,
                   RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiDisplayV( name, type, mode, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiDisplayV", )
}

RtVoid RiHider( RtToken name, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS( name )
	RiHiderV( name, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiHider", )
}

RtVoid RiHiderV( RtToken type, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiHiderV( type, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiHiderV", )
}

RtVoid RiColorSamples( RtInt n, RtFloat nRGB[], RtFloat RGBn[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiColorSamples( n, nRGB, RGBn );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiColorSamples", )
}

RtVoid RiRelativeDetail( RtFloat relativedetail )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiRelativeDetail( relativedetail );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiRelativeDetail", )
}

RtVoid RiOption( RtToken name, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS( name )
	RiOptionV( name, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiOption", )
}

RtVoid RiOptionV( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD

	if ( context.getContext() == ( RtContextHandle ) RI_NULL )
		context.parseOption( name, n, tokens, parms );
	else
		context.current().RiOptionV( name, n, tokens, parms );

	RI2RIB_EXCEPTION_CATCH_GUARD("RiOptionV", )
}

RtVoid RiAttributeBegin( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiAttributeBegin();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiAttributeBegin", )
}

RtVoid RiAttributeEnd( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiAttributeEnd();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiAttributeEnd", )
}

RtVoid RiColor( RtColor color )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiColor( color );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiColor", )
}

RtVoid RiOpacity( RtColor color )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiOpacity( color );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiOpacity", )
}

RtVoid RiTextureCoordinates( RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2,
                             RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4 )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiTextureCoordinates( s1, t1, s2, t2, s3, t3, s4, t4 );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiTextureCoordinates", )
}

RtLightHandle RiLightSource( RtToken name, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS( name )
	return ( RiLightSourceV( name, AQSIS_PASS_RI_PARAMETERS ) );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiLightSource", RI_NULL)
}

RtLightHandle RiLightSourceV( RtToken name, RtInt n, RtToken tokens[],
                              RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	return context.current().RiLightSourceV( name, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiLightSourceV", RI_NULL)
}

RtLightHandle RiAreaLightSource( RtToken name, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS( name )
	return ( RiAreaLightSourceV( name, AQSIS_PASS_RI_PARAMETERS ) );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiAreaLightSource", RI_NULL)
}

RtLightHandle RiAreaLightSourceV( RtToken name,
                                  RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	return context.current().RiAreaLightSourceV( name, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiAreaLightSourceV", RI_NULL)
}

RtVoid RiIlluminate( RtLightHandle light, RtBoolean onoff )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiIlluminate( light, onoff );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiIlluminate", )
}

RtVoid RiSurface( RtToken name, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS( name )
	RiSurfaceV( name, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiSurface", )
}

RtVoid RiSurfaceV( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiSurfaceV( name, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiSurfaceV", )
}

RtVoid RiAtmosphere( RtToken name, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS( name )
	RiAtmosphereV( name, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiAtmosphere", )
}

RtVoid RiAtmosphereV( RtToken name, RtInt n, RtToken tokens[],
                      RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiAtmosphereV( name, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiAtmosphereV", )
}

RtVoid RiInterior( RtToken name, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS( name )
	RiInteriorV( name, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiInterior", )
}

RtVoid RiInteriorV( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiInteriorV( name, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiInteriorV", )
}

RtVoid RiExterior( RtToken name, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS( name )
	RiExteriorV( name, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiExterior", )
}

RtVoid RiExteriorV( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiExteriorV( name, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiExteriorV", )
}

RtVoid RiShadingRate( RtFloat size )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiShadingRate( size );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiShadingRate", )
}

RtVoid RiShadingInterpolation( RtToken type )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiShadingInterpolation( type );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiShadingInterpolation", )
}

RtVoid RiMatte( RtBoolean onoff )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiMatte( onoff );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMatte", )
}

RtVoid RiBound( RtBound bound )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiBound( bound );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiBound", )
}

RtVoid RiDetail( RtBound bound )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiDetail( bound );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiDetail", )
}

RtVoid RiDetailRange( RtFloat minvis, RtFloat lowtran, RtFloat uptran,
                      RtFloat maxvis )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiDetailRange( minvis, lowtran, uptran, maxvis );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiDetailRange", )
}

RtVoid RiGeometricApproximation( RtToken type, RtFloat value )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiGeometricApproximation( type, value );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiGeometricApproximation", )
}

RtVoid RiOrientation( RtToken orientation )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiOrientation( orientation );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiOrientation", )
}

RtVoid RiReverseOrientation( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiReverseOrientation();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiReverseOrientation", )
}

RtVoid RiSides( RtInt sides )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiSides( sides );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiSides", )
}

RtVoid RiIdentity( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiIdentity();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiIdentity", )
}

RtVoid RiTransform( RtMatrix transform )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiTransform( transform );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiTransform", )
}

RtVoid RiConcatTransform( RtMatrix transform )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiConcatTransform( transform );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiConcatTransform", )
}

RtVoid RiPerspective( RtFloat fov )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiPerspective( fov );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPerspective", )
}

RtVoid RiTranslate( RtFloat dx, RtFloat dy, RtFloat dz )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiTranslate( dx, dy, dz );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiTranslate", )
}

RtVoid RiResource( RtToken handle, RtToken type, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS( type )
	RiResourceV( handle, type, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiResource", )
}

RtVoid RiResourceV( RtToken handle, RtToken type, RtInt n, RtToken tokens[],
                    RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiResourceV( handle, type, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiResourceV", )
}

RtVoid RiResourceBegin( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiResourceBegin();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiResourceBegin", )
}

RtVoid RiResourceEnd( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiResourceEnd();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiResourceEnd", )
}

RtVoid RiRotate( RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiRotate( angle, dx, dy, dz );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiRotate", )
}

RtVoid RiScale( RtFloat sx, RtFloat sy, RtFloat sz )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiScale( sx, sy, sz );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiScale", )
}

RtVoid RiSkew( RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
               RtFloat dx2, RtFloat dy2, RtFloat dz2 )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiSkew( angle, dx1, dy1, dz1, dx2, dy2, dz2 );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiSkew", )
}

RtVoid RiDeformation( RtToken name, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(name)
	RiDeformationV( name, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiDeformation", )
}

RtVoid RiDeformationV( RtToken name, RtInt n, RtToken tokens[],
                       RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiDeformationV( name, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiDeformationV", )
}

RtVoid RiDisplacement( RtToken name, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(name)
	RiDisplacementV( name, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiDisplacement", )
}

RtVoid RiDisplacementV( RtToken name, RtInt n, RtToken tokens[],
                        RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiDisplacementV( name, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiDisplacementV", )
}

RtVoid RiCoordinateSystem( RtToken space )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiCoordinateSystem( space );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiCoordinateSystem", )
}

RtVoid RiCoordSysTransform( RtToken space )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiCoordSysTransform( space );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiCoordSysTransform", )
}

RtPoint * RiTransformPoints( RtToken fromspace, RtToken tospace,
                             RtInt n, RtPoint points[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	CqError r( RIE_UNIMPLEMENT, RIE_WARNING, "RiTransformPoints cannot be "
			   "written to a RIB file.", false );
	throw(r);
	RI2RIB_EXCEPTION_CATCH_GUARD("RiTransformPoints", ( RtPoint * ) RI_NULL)
}

RtVoid RiTransformBegin( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiTransformBegin();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiTransformBegin", )
}

RtVoid RiTransformEnd( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiTransformEnd();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiTransformEnd", )
}

RtVoid RiAttribute( RtToken name, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(name)
	RiAttributeV( name, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiAttribute", )
}

RtVoid RiAttributeV( RtToken name, RtInt n, RtToken tokens[],
                     RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiAttributeV( name, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiAttributeV", )
}

RtVoid RiPolygon( RtInt nverts, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(nverts)
	RiPolygonV( nverts, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPolygon", )
}

RtVoid RiPolygonV( RtInt nverts, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiPolygonV( nverts, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPolygonV", )
}

RtVoid RiGeneralPolygon( RtInt nloops, RtInt nverts[], ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(nverts)
	RiGeneralPolygonV( nloops, nverts, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiGeneralPolygon", )
}

RtVoid RiGeneralPolygonV( RtInt nloops, RtInt nverts[],
                          RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiGeneralPolygonV( nloops, nverts, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiGeneralPolygonV", )
}

RtVoid RiPointsPolygons( RtInt npolys, RtInt nverts[], RtInt verts[], ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(verts)
	RiPointsPolygonsV( npolys, nverts, verts, AQSIS_PASS_RI_PARAMETERS);
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPointsPolygons", )
}

RtVoid RiPointsPolygonsV( RtInt npolys, RtInt nverts[], RtInt verts[],
                          RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiPointsPolygonsV( npolys, nverts, verts, n, tokens,
	                                     parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPointsPolygonsV", )
}

RtVoid RiPointsGeneralPolygons( RtInt npolys, RtInt nloops[], RtInt nverts[],
                                RtInt verts[], ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(verts)
	RiPointsGeneralPolygonsV( npolys, nloops, nverts, verts,
	                          AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPointsGeneralPolygons", )
}

RtVoid RiPointsGeneralPolygonsV( RtInt npolys, RtInt nloops[], RtInt nverts[],
                                 RtInt verts[], RtInt n, RtToken tokens[],
                                 RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiPointsGeneralPolygonsV( npolys, nloops, nverts, verts,
	        n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPointsGeneralPolygonsV", )
}

RtVoid RiBasis( RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiBasis( ubasis, ustep, vbasis, vstep );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiBasis", )
}

RtVoid RiPatch( RtToken type, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(type)
	RiPatchV( type, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPatch", )
}

RtVoid RiPatchV( RtToken type, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiPatchV( type, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPatchV", )
}

RtVoid RiPatchMesh( RtToken type, RtInt nu, RtToken uwrap, RtInt nv,
                    RtToken vwrap, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(vwrap)
	RiPatchMeshV( type, nu, uwrap, nv, vwrap, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPatchMesh", )
}

RtVoid RiPatchMeshV( RtToken type, RtInt nu, RtToken uwrap, RtInt nv,
                     RtToken vwrap, RtInt n, RtToken tokens[],
                     RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiPatchMeshV( type, nu, uwrap, nv, vwrap, n, tokens,
	                                parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPatchMeshV", )
}

RtVoid RiNuPatch( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin,
                  RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[],
                  RtFloat vmin, RtFloat vmax, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(vmax)
	RiNuPatchV( nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax,
	            AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiNuPatch", )
}

RtVoid RiNuPatchV( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin,
                   RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[],
                   RtFloat vmin, RtFloat vmax,
                   RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiNuPatchV( nu, uorder, uknot, umin,
	                              umax, nv, vorder, vknot,
	                              vmin, vmax, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiNuPatchV", )
}

RtVoid RiTrimCurve( RtInt nloops, RtInt ncurves[], RtInt order[],
                    RtFloat knot[], RtFloat min[], RtFloat max[],
                    RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiTrimCurve( nloops, ncurves, order,
	                               knot, min, max,
	                               n, u, v, w );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiTrimCurve", )
}

RtVoid RiSphere( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(tmax)
	RiSphereV( radius, zmin, zmax, tmax, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiSphere", )
}

RtVoid RiSphereV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax,
                  RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiSphereV( radius, zmin, zmax, tmax, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiSphereV", )
}

RtVoid RiCone( RtFloat height, RtFloat radius, RtFloat tmax, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(tmax)
	RiConeV( height, radius, tmax, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiCone", )
}

RtVoid RiConeV( RtFloat height, RtFloat radius, RtFloat tmax,
                RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiConeV( height, radius, tmax, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiConeV", )
}

RtVoid RiCylinder( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax,
                   ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(tmax)
	RiCylinderV( radius, zmin, zmax, tmax, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiCylinder", )
}

RtVoid RiCylinderV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax,
                    RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiCylinderV( radius, zmin, zmax, tmax, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiCylinderV", )
}

RtVoid RiHyperboloid( RtPoint point1, RtPoint point2, RtFloat tmax, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(tmax)
	RiHyperboloidV( point1, point2, tmax, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiHyperboloid", )
}

RtVoid RiHyperboloidV( RtPoint point1, RtPoint point2, RtFloat tmax,
                       RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiHyperboloidV( point1, point2, tmax, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiHyperboloidV", )
}

RtVoid RiParaboloid( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat tmax,
                     ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(tmax)
	RiParaboloidV( rmax, zmin, zmax, tmax, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiParaboloid", )
}

RtVoid RiParaboloidV( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat tmax,
                      RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiParaboloidV( rmax, zmin, zmax, tmax, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiParaboloidV", )
}

RtVoid RiDisk( RtFloat height, RtFloat radius, RtFloat tmax, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(tmax)
	RiDiskV( height, radius, tmax, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiDisk", )
}

RtVoid RiDiskV( RtFloat height, RtFloat radius, RtFloat tmax,
                RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiDiskV( height, radius, tmax, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiDiskV", )
}

RtVoid RiTorus( RtFloat majrad, RtFloat minrad, RtFloat phimin,
                RtFloat phimax, RtFloat tmax, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(tmax)
	RiTorusV( majrad, minrad, phimin, phimax, tmax, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiTorus", )
}

RtVoid RiTorusV( RtFloat majrad, RtFloat minrad,
                 RtFloat phimin, RtFloat phimax, RtFloat tmax,
                 RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiTorusV( majrad, minrad, phimin, phimax, tmax, n, tokens,
	                            parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiTorusV", )
}

RtVoid RiBlobby( RtInt nleaf, RtInt ncode, RtInt code[],
                 RtInt nflt, RtFloat flt[],
                 RtInt nstr, RtToken str[], ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(str)
	RiBlobbyV( nleaf, ncode, code, nflt, flt, nstr, str,
	           AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiBlobby", )
}

RtVoid RiBlobbyV( RtInt nleaf, RtInt ncode, RtInt code[],
                  RtInt nflt, RtFloat flt[],
                  RtInt nstr, RtToken str[],
                  RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiBlobbyV( nleaf, ncode, code, nflt, flt, nstr, str, n,
	                             tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiBlobbyV", )
}

RtVoid RiCurves( RtToken type, RtInt ncurves,
                 RtInt nvertices[], RtToken wrap, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(wrap)
	RiCurvesV( type, ncurves, nvertices, wrap, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiCurves", )
}

RtVoid RiCurvesV( RtToken type, RtInt ncurves,
                  RtInt nvertices[], RtToken wrap,
                  RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiCurvesV( type, ncurves, nvertices, wrap, n, tokens,
	                             parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiCurvesV", )
}

RtVoid RiPoints( RtInt npoints, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(npoints)
	RiPointsV( npoints, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPoints", )
}

RtVoid RiPointsV( RtInt npoints, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiPointsV( npoints, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiPointsV", )
}

RtVoid RiSubdivisionMesh( RtToken mask, RtInt nf, RtInt nverts[], RtInt verts[],
                          RtInt ntags, RtToken tags[], RtInt numargs[],
                          RtInt intargs[], RtFloat floatargs[], ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(floatargs)
	RiSubdivisionMeshV( mask, nf, nverts, verts, ntags, tags, numargs, intargs,
	                    floatargs, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiSubdivisionMesh", )
}

RtVoid RiSubdivisionMeshV( RtToken mask, RtInt nf, RtInt nverts[],
                           RtInt verts[],
                           RtInt ntags, RtToken tags[], RtInt numargs[],
                           RtInt intargs[], RtFloat floatargs[],
                           RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiSubdivisionMeshV( mask, nf, nverts, verts, ntags, tags,
	                                      numargs, intargs, floatargs, n,
	                                      tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiSubdivisionMeshV", )
}

RtVoid RiProcedural( RtPointer data, RtBound bound,
                     RtVoid ( *subdivfunc ) ( RtPointer, RtFloat ),
                     RtVoid ( *freefunc ) ( RtPointer ) )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiProcedural( data, bound, subdivfunc, freefunc );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiProcedural", )
}

RtVoid RiGeometry( RtToken type, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(type)
	RiGeometryV( type, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiGeometry", )
}

RtVoid RiGeometryV( RtToken type, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiGeometryV( type, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiGeometryV", )
}

RtVoid RiSolidBegin( RtToken operation )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiSolidBegin( operation );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiSolidBegin", )
}

RtVoid RiSolidEnd( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiSolidEnd();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiSolidEnd", )
}

RtObjectHandle RiObjectBegin( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	return context.current().RiObjectBegin();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiObjectBegin", RI_NULL)
}

RtVoid RiObjectEnd( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiObjectEnd();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiObjectEnd", )
}

RtVoid RiObjectInstance( RtObjectHandle handle )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiObjectInstance( handle );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiObjectInstance", )
}

RtVoid RiMotionBegin( RtInt N, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD

	va_list pArgs;
	va_start( pArgs, N );

	RtFloat* times = new RtFloat[ N ];
	RtInt i;
	for ( i = 0; i < N; ++i )
		times[ i ] = va_arg( pArgs, double );

	RiMotionBeginV( N, times );

	delete[] ( times );

	RI2RIB_EXCEPTION_CATCH_GUARD("RiMotionBegin", )
}

RtVoid RiMotionBeginV( RtInt n, RtFloat times[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiMotionBeginV( n, times );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMotionBeginV", )
}

RtVoid RiMotionEnd( void )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiMotionEnd();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMotionEnd", )
}

RtVoid RiMakeTexture( RtToken pic, RtToken tex, RtToken swrap, RtToken twrap,
                      RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
                      ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(twidth)
	RiMakeTextureV( pic, tex, swrap, twrap, filterfunc, swidth, twidth,
	                AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMakeTexture", )
}

RtVoid RiMakeTextureV( RtToken pic, RtToken tex, RtToken swrap, RtToken twrap,
                       RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
                       RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiMakeTextureV( pic, tex, swrap, twrap, filterfunc,
	                                  swidth, twidth, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMakeTextureV", )
}

RtVoid RiMakeBump( RtToken pic, RtToken tex, RtToken swrap, RtToken twrap,
                   RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
                   ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(twidth)
	RiMakeBumpV( pic, tex, swrap, twrap, filterfunc, swidth, twidth,
	             AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMakeBump", )
}

RtVoid RiMakeBumpV( RtToken pic, RtToken tex, RtToken swrap, RtToken twrap,
                    RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
                    RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiMakeBumpV( pic, tex, swrap, twrap, filterfunc, swidth,
	                               twidth, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMakeBumpV", )
}

RtVoid RiMakeLatLongEnvironment( RtToken pic, RtToken tex,
                                 RtFilterFunc filterfunc,
                                 RtFloat swidth, RtFloat twidth, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(twidth)
	RiMakeLatLongEnvironmentV( pic, tex, filterfunc, swidth, twidth,
	                           AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMakeLatLongEnvironment", )
}

RtVoid RiMakeLatLongEnvironmentV( RtToken pic, RtToken tex,
                                  RtFilterFunc filterfunc, RtFloat swidth,
                                  RtFloat twidth, RtInt n, RtToken tokens[],
                                  RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiMakeLatLongEnvironmentV( pic, tex, filterfunc, swidth,
	        twidth, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMakeLatLongEnvironmentV", )
}

RtVoid RiMakeCubeFaceEnvironment( RtToken px, RtToken nx, RtToken py,
                                  RtToken ny, RtToken pz, RtToken nz,
                                  RtToken tex, RtFloat fov,
                                  RtFilterFunc filterfunc, RtFloat swidth,
                                  RtFloat ywidth, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(ywidth)
	RiMakeCubeFaceEnvironmentV( px, nx, py, ny, pz, nz, tex, fov, filterfunc,
	                            swidth, ywidth, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMakeCubeFaceEnvironment", )
}

RtVoid RiMakeCubeFaceEnvironmentV( RtToken px, RtToken nx, RtToken py,
                                   RtToken ny, RtToken pz, RtToken nz,
                                   RtToken tex, RtFloat fov,
                                   RtFilterFunc filterfunc, RtFloat swidth,
                                   RtFloat ywidth, RtInt n, RtToken tokens[],
                                   RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiMakeCubeFaceEnvironmentV( px, nx, py, ny, pz, nz, tex,
	        fov, filterfunc, swidth, ywidth, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMakeCubeFaceEnvironmentV", )
}

RtVoid RiMakeShadow( RtToken pic, RtToken tex, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(tex)
	RiMakeShadowV( pic, tex, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMakeShadow", )
}

RtVoid RiMakeShadowV( RtToken pic, RtToken tex, RtInt n, RtToken tokens[],
                      RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiMakeShadowV( pic, tex, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMakeShadowV", )
}

RtVoid RiMakeOcclusion( RtInt npics, RtString picfiles[], RtString shadowfile,
                        ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(shadowfile)
	RiMakeOcclusion( npics, picfiles, shadowfile, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMakeOcclusion", )
}


RtVoid RiMakeOcclusionV( RtInt npics, RtString picfiles[], RtString shadowfile,
                         RtInt count, RtToken tokens[], RtPointer values[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiMakeOcclusionV( npics, picfiles, shadowfile, count,
	                                    tokens, values );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiMakeOcclusionV", )
}

RtVoid RiArchiveRecord( RtToken type, char *format, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD

	TqInt size = 256;
	char* buffer = 0;
	bool longEnough = false;
	while(!longEnough)
	{
		delete[] buffer;
		buffer = new char[size];
		va_list args;
		va_start( args, format );
#		if defined(AQSIS_COMPILER_MSVC6) || defined(AQSIS_COMPILER_MSVC7)
		TqInt len = _vsnprintf(buffer, size, format, args);
		// msdn says that _vsnprintf() returns a negative number if the buffer
		// wasn't long enough.  Add the extra (len < size) for safety in the
		// case MSVC becomes standard-compliant at some stage in the future...
		longEnough = len >= 0 && len < size;
		size *= 2;
#		else
		TqInt len = vsnprintf(buffer, size, format, args);
		// According to the linux man pages, vsnprintf() returns a negative
		// value on error, or a positive value indicating the number of chars
		// which would have been written for an infinite-size buffer, not
		// including the terminating '\0'.  This is claimed to be the
		// C99-conforming behaviour.
		if(len < 0)
			return;
		longEnough = len < size;
		size = len+1;
#		endif
		va_end(args);
	}
	context.current().RiArchiveRecord(type, buffer);

	delete[] buffer;
	RI2RIB_EXCEPTION_CATCH_GUARD("RiArchiveRecord", )
}

RtVoid RiReadArchive( RtToken name, RtArchiveCallback callback, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(callback)
	RiReadArchiveV( name, callback, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiReadArchive", )
}

RtVoid RiReadArchiveV( RtToken name, RtArchiveCallback callback, RtInt n,
                       RtToken tokens[], RtPointer parms[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiReadArchiveV( name, callback, n, tokens, parms );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiReadArchiveV", )
}

RtVoid RiIfBegin( RtString condition )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiIfBegin( condition );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiIfBegin", )
}

RtVoid RiElse( )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiElse();
	RI2RIB_EXCEPTION_CATCH_GUARD("RiElse", )
}

RtVoid RiElseIf( RtString condition )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiElseIf( condition );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiElseIf", )
}

RtVoid RiIfEnd( )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiIfEnd( );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiIfEnd", )
}
RtVoid RiErrorHandler( RtErrorFunc handler )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiErrorHandler( handler );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiErrorHandler", )
}

RtVoid RiErrorIgnore( RtInt code, RtInt severity, RtString message )
{
	std::cout << "RiErrorIgnore" << std::endl;
	return ;
}

RtVoid RiErrorPrint( RtInt code, RtInt severity, RtString message )
{
	std::cout << "RiErrorPrint" << std::endl;
	return ;
}

RtVoid RiErrorAbort( RtInt code, RtInt severity, RtString message )
{
	std::cout << "RiErrorAbort" << std::endl;
	return ;
}

RtVoid RiShaderLayer( RtToken type, RtToken name, RtToken layername, ... )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	AQSIS_COLLECT_RI_PARAMETERS(layername)
	RiShaderLayer( type, name, layername, AQSIS_PASS_RI_PARAMETERS );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiShaderLayer", )
}

RtVoid RiShaderLayerV( RtToken type, RtToken name, RtToken layername,
                       RtInt count, RtToken tokens[], RtPointer values[] )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiShaderLayerV( type, name, layername, count, tokens,
	                                  values );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiShaderLayerV", )
}

RtVoid RiConnectShaderLayers( RtToken type, RtToken layer1, RtToken variable1,
                              RtToken layer2, RtToken variable2 )
{
	RI2RIB_EXCEPTION_TRY_GUARD
	context.current().RiConnectShaderLayers( type, layer1, variable1, layer2,
	        variable2 );
	RI2RIB_EXCEPTION_CATCH_GUARD("RiConnectShaderLayers", )
}
