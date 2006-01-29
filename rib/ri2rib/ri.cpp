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

#include <stdarg.h>
#include <string>
#include <stdio.h>
#include "aqsis.h"
#include "ri.h"
#include "error.h"
#include "plstore.h"
#include "context.h"


START_NAMESPACE( libri2rib )
static CqContext context;
END_NAMESPACE( libri2rib )

using libri2rib::context;
using libri2rib::CqPLStore;
using libri2rib::CqError;


RtFloat RiGaussianFilter ( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return 1.0;
}
RtFloat RiBoxFilter ( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return 1.0;
}
RtFloat RiTriangleFilter ( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return 1.0;
}
RtFloat RiCatmullRomFilter ( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return 1.0;
}
RtFloat RiSincFilter ( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return 1.0;
}
RtFloat RiDiskFilter ( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return 1.0;
}
RtFloat RiBesselFilter ( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return 1.0;
}


RtBasis RiBezierBasis = { { -1, 3, -3, 1},
                          { 3, -6, 3, 0},
                          { -3, 3, 0, 0},
                          { 1, 0, 0, 0} };

RtBasis RiBSplineBasis = { { -1.0 / 6, 3.0 / 6, -3.0 / 6, 1.0 / 6},
                           { 3.0 / 6, -6.0 / 6, 3.0 / 6, 0.0 / 6},
                           { -3.0 / 6, 0.0 / 6, 3.0 / 6, 0.0 / 6},
                           { 1.0 / 6, 4.0 / 6, 1.0 / 6, 0.0 / 6} };

RtBasis RiCatmullRomBasis = { { -1.0 / 2, 3.0 / 2, -3.0 / 2, 1.0 / 2},
                              { 2.0 / 2, -5.0 / 2, 4.0 / 2, -1.0 / 2},
                              { -1.0 / 2, 0.0 / 2, 1.0 / 2, 0.0 / 2},
                              { 0.0 / 2, 2.0 / 2, 0.0 / 2, 0.0 / 2} };

RtBasis RiHermiteBasis = { { 2, 1, -2, 1},
                           { -3, -2, 3, -1},
                           { 0, 1, 0, 0},
                           { 1, 0, 0, 0} };

RtBasis RiPowerBasis = { {1, 0, 0, 0},
                         {0, 1, 0, 0},
                         {0, 0, 1, 0},
                         {0, 0, 0, 1} };


RtVoid RiProcDelayedReadArchive ( RtPointer data, RtFloat detail )
{}
RtVoid RiProcRunProgram ( RtPointer data, RtFloat detail )
{}
RtVoid RiProcDynamicLoad ( RtPointer data, RtFloat detail )
{}

extern "C" RtVoid RiProcFree ( RtPointer data )
{}




RtContextHandle RiGetContext ( void )
{
	try
	{
		return context.getContext();
	}
	catch ( CqError & r )
	{
		r.manage();
		return RI_NULL;
	}
}

RtVoid RiContext ( RtContextHandle ch )
{
	try
	{
		context.switchTo( ch );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtToken RiDeclare ( RtToken name, RtToken declaration )
{
	try
	{
		return context.current().RiDeclare( name, declaration );
	}
	catch ( CqError & r )
	{
		r.manage();
		return RI_NULL;
	}
}

RtVoid RiBegin ( RtToken name )
{
	try
	{
		context.addContext( name );
		context.current().RiBegin( name );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiEnd ( void )
{
	try
	{
		context.current().RiEnd();
		context.removeCurrent();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiFrameBegin ( RtInt frame )
{
	try
	{
		context.current().RiFrameBegin( frame );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiFrameEnd ( void )
{
	try
	{
		context.current().RiFrameEnd();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiWorldBegin ( void )
{
	try
	{
		context.current().RiWorldBegin();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiWorldEnd ( void )
{
	try
	{
		context.current().RiWorldEnd();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiFormat ( RtInt xres, RtInt yres, RtFloat aspect )
{
	try
	{
		context.current().RiFormat( xres, yres, aspect );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiFrameAspectRatio ( RtFloat aspect )
{
	try
	{
		context.current().RiFrameAspectRatio( aspect );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiScreenWindow ( RtFloat left, RtFloat right, RtFloat bot, RtFloat top )
{
	try
	{
		context.current().RiScreenWindow( left, right, bot, top );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiCropWindow ( RtFloat xmin, RtFloat xmax, RtFloat ymin, RtFloat ymax )
{
	try
	{
		context.current().RiCropWindow( xmin, xmax, ymin, ymax );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiProjection ( RtToken name, ... )
{
	try
	{
		va_list args;
		va_start( args, name );
		CqPLStore pls( args );
		va_end( args );

		RiProjectionV( name, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiProjectionV ( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiProjectionV( name, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiClipping ( RtFloat hither, RtFloat yon )
{
	try
	{
		context.current().RiClipping( hither, yon );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiClippingPlane ( RtFloat x, RtFloat y, RtFloat z,
                         RtFloat nx, RtFloat ny, RtFloat nz )
{
	try
	{
		context.current().RiClippingPlane( x, y, z, nx, ny, nz );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiDepthOfField ( RtFloat fstop, RtFloat focallength, RtFloat focaldistance )
{
	try
	{
		context.current().RiDepthOfField ( fstop, focallength, focaldistance );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiShutter ( RtFloat min, RtFloat max )
{
	try
	{
		context.current().RiShutter( min, max );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPixelVariance ( RtFloat variation )
{
	try
	{
		context.current().RiPixelVariance( variation );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPixelSamples ( RtFloat xsamples, RtFloat ysamples )
{
	try
	{
		context.current().RiPixelSamples( xsamples, ysamples );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPixelFilter ( RtFilterFunc filterfunc, RtFloat xwidth, RtFloat ywidth )
{
	try
	{
		context.current().RiPixelFilter( filterfunc, xwidth, ywidth );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiExposure ( RtFloat gain, RtFloat gamma )
{
	try
	{
		context.current().RiExposure( gain, gamma );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiImager ( RtToken name, ... )
{
	try
	{
		va_list args;
		va_start( args, name );
		CqPLStore pls( args );
		va_end( args );

		RiImagerV( name, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiImagerV ( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiImagerV( name, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiQuantize ( RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ampl )
{
	try
	{
		context.current().RiQuantize( type, one, min, max, ampl );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiDisplay ( RtToken name, RtToken type, RtToken mode, ... )
{
	try
	{
		va_list args;
		va_start( args, mode );
		CqPLStore pls( args );
		va_end( args );

		RiDisplayV( name, type, mode, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiDisplayV ( RtToken name, RtToken type, RtToken mode,
                    RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiDisplayV( name, type, mode, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiHider ( RtToken type, ... )
{
	try
	{
		va_list args;
		va_start( args, type );
		CqPLStore pls( args );
		va_end( args );

		RiHiderV( type, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiHiderV ( RtToken type, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiHiderV( type, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiColorSamples ( RtInt n, RtFloat nRGB[], RtFloat RGBn[] )
{
	try
	{
		context.current().RiColorSamples( n, nRGB, RGBn );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiRelativeDetail ( RtFloat relativedetail )
{
	try
	{
		context.current().RiRelativeDetail( relativedetail );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiOption ( RtToken name, ... )
{
	try
	{
		va_list args;
		va_start( args, name );
		CqPLStore pls( args );
		va_end( args );

		RiOptionV( name, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiOptionV ( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		if ( context.getContext() == ( RtContextHandle ) RI_NULL )
		{
			context.parseOption( name, n, tokens, parms );
		}
		else
		{
			context.current().RiOptionV( name, n, tokens, parms );
		}
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiAttributeBegin ( void )
{
	try
	{
		context.current().RiAttributeBegin();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiAttributeEnd ( void )
{
	try
	{
		context.current().RiAttributeEnd();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiColor ( RtColor color )
{
	try
	{
		context.current().RiColor( color );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiOpacity ( RtColor color )
{
	try
	{
		context.current().RiOpacity( color );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiTextureCoordinates( RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2,
                             RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4 )
{
	try
	{
		context.current().RiTextureCoordinates( s1, t1, s2, t2, s3, t3, s4, t4 );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtLightHandle RiLightSource ( RtToken name, ... )
{
	try
	{
		va_list args;
		va_start( args, name );
		CqPLStore pls( args );
		va_end( args );

		return RiLightSourceV( name, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
		return RI_NULL;
	}
}

RtLightHandle RiLightSourceV ( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		return context.current().RiLightSourceV( name, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
		return RI_NULL;
	}
}

RtLightHandle RiAreaLightSource ( RtToken name, ... )
{
	try
	{
		va_list args;
		va_start( args, name );
		CqPLStore pls( args );
		va_end( args );

		return RiAreaLightSourceV( name, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
		return RI_NULL;
	}
}

RtLightHandle RiAreaLightSourceV ( RtToken name,
                                   RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		return context.current().RiAreaLightSourceV( name, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
		return RI_NULL;
	}
}

RtVoid RiIlluminate ( RtLightHandle light, RtBoolean onoff )
{
	try
	{
		context.current().RiIlluminate( light, onoff );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiSurface ( RtToken name, ... )
{
	try
	{
		va_list args;
		va_start( args, name );
		CqPLStore pls( args );
		va_end( args );

		RiSurfaceV( name, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiSurfaceV ( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiSurfaceV( name, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiAtmosphere ( RtToken name, ... )
{
	try
	{
		va_list args;
		va_start( args, name );
		CqPLStore pls( args );
		va_end( args );

		RiAtmosphereV( name, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiAtmosphereV ( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiAtmosphereV( name, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiInterior ( RtToken name, ... )
{
	try
	{
		va_list args;
		va_start( args, name );
		CqPLStore pls( args );
		va_end( args );

		RiInteriorV( name, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiInteriorV ( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiInteriorV( name, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiExterior ( RtToken name, ... )
{
	try
	{
		va_list args;
		va_start( args, name );
		CqPLStore pls( args );
		va_end( args );

		RiExteriorV( name, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiExteriorV ( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiExteriorV( name, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiShadingRate ( RtFloat size )
{
	try
	{
		context.current().RiShadingRate( size );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiShadingInterpolation ( RtToken type )
{
	try
	{
		context.current().RiShadingInterpolation( type );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMatte ( RtBoolean onoff )
{
	try
	{
		context.current().RiMatte( onoff );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiBound ( RtBound bound )
{
	try
	{
		context.current().RiBound( bound );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiDetail ( RtBound bound )
{
	try
	{
		context.current().RiDetail( bound );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiDetailRange ( RtFloat minvis, RtFloat lowtran, RtFloat uptran, RtFloat maxvis )
{
	try
	{
		context.current().RiDetailRange( minvis, lowtran, uptran, maxvis );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiGeometricApproximation ( RtToken type, RtFloat value )
{
	try
	{
		context.current().RiGeometricApproximation( type, value );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiOrientation ( RtToken orientation )
{
	try
	{
		context.current().RiOrientation( orientation );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiReverseOrientation ( void )
{
	try
	{
		context.current().RiReverseOrientation();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiSides ( RtInt sides )
{
	try
	{
		context.current().RiSides( sides );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiIdentity ( void )
{
	try
	{
		context.current().RiIdentity();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiTransform ( RtMatrix transform )
{
	try
	{
		context.current().RiTransform( transform );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiConcatTransform ( RtMatrix transform )
{
	try
	{
		context.current().RiConcatTransform( transform );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPerspective ( RtFloat fov )
{
	try
	{
		context.current().RiPerspective( fov );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiTranslate ( RtFloat dx, RtFloat dy, RtFloat dz )
{
	try
	{
		context.current().RiTranslate( dx, dy, dz );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiRotate ( RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz )
{
	try
	{
		context.current().RiRotate( angle, dx, dy, dz );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiScale ( RtFloat sx, RtFloat sy, RtFloat sz )
{
	try
	{
		context.current().RiScale( sx, sy, sz );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiSkew ( RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
                RtFloat dx2, RtFloat dy2, RtFloat dz2 )
{
	try
	{
		context.current().RiSkew( angle, dx1, dy1, dz1, dx2, dy2, dz2 );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiDeformation ( RtToken name, ... )
{
	try
	{
		va_list args;
		va_start( args, name );
		CqPLStore pls( args );
		va_end( args );

		RiDeformationV( name, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiDeformationV ( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiDeformationV( name, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiDisplacement ( RtToken name, ... )
{
	try
	{
		va_list args;
		va_start( args, name );
		CqPLStore pls( args );
		va_end( args );

		RiDisplacementV( name, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiDisplacementV ( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiDisplacementV( name, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiCoordinateSystem ( RtToken space )
{
	try
	{
		context.current().RiCoordinateSystem( space );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiCoordSysTransform ( RtToken space )
{
	try
	{
		context.current().RiCoordSysTransform( space );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtPoint * RiTransformPoints ( RtToken fromspace, RtToken tospace,
                              RtInt n, RtPoint points[] )
{
	CqError r( RIE_UNIMPLEMENT, RIE_WARNING,
	           "RiTransformPoints cannot be written to a RIB file.", TqFalse );
	r.manage();
	return ( RtPoint * ) RI_NULL;
}

RtVoid RiTransformBegin ( void )
{
	try
	{
		context.current().RiTransformBegin();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiTransformEnd ( void )
{

	try
	{
		context.current().RiTransformEnd();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiAttribute ( RtToken name, ... )
{
	try
	{
		va_list args;
		va_start( args, name );
		CqPLStore pls( args );
		va_end( args );

		RiAttributeV( name, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiAttributeV ( RtToken name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiAttributeV( name, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPolygon ( RtInt nverts, ... )
{
	try
	{
		va_list args;
		va_start( args, nverts );
		CqPLStore pls( args );
		va_end( args );

		RiPolygonV( nverts, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPolygonV ( RtInt nverts, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiPolygonV( nverts, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiGeneralPolygon ( RtInt nloops, RtInt nverts[], ... )
{
	try
	{
		va_list args;
		va_start( args, nverts );
		CqPLStore pls( args );
		va_end( args );

		RiGeneralPolygonV( nloops, nverts, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiGeneralPolygonV ( RtInt nloops, RtInt nverts[],
                           RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiGeneralPolygonV( nloops, nverts, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPointsPolygons ( RtInt npolys, RtInt nverts[], RtInt verts[], ... )
{
	try
	{
		va_list args;
		va_start( args, verts );
		CqPLStore pls( args );
		va_end( args );

		RiPointsPolygonsV( npolys, nverts, verts, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}


RtVoid RiPointsPolygonsV ( RtInt npolys, RtInt nverts[], RtInt verts[],
                           RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiPointsPolygonsV( npolys, nverts, verts, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPointsGeneralPolygons ( RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], ... )
{
	try
	{
		va_list args;
		va_start( args, verts );
		CqPLStore pls( args );
		va_end( args );

		RiPointsGeneralPolygonsV( npolys, nloops, nverts, verts, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPointsGeneralPolygonsV ( RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[],
                                  RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiPointsGeneralPolygonsV( npolys, nloops, nverts, verts, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiBasis ( RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep )
{
	try
	{
		context.current().RiBasis( ubasis, ustep, vbasis, vstep );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPatch ( RtToken type, ... )
{
	try
	{
		va_list args;
		va_start( args, type );
		CqPLStore pls( args );
		va_end( args );

		RiPatchV( type, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPatchV ( RtToken type, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiPatchV( type, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPatchMesh ( RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, ... )
{
	try
	{
		va_list args;
		va_start( args, vwrap );
		CqPLStore pls( args );
		va_end( args );

		RiPatchMeshV( type, nu, uwrap, nv, vwrap, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPatchMeshV ( RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap,
                      RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiPatchMeshV( type, nu, uwrap, nv, vwrap, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiNuPatch ( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin,
                   RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[],
                   RtFloat vmin, RtFloat vmax, ... )
{
	try
	{
		va_list args;
		va_start( args, vmax );
		CqPLStore pls( args );
		va_end( args );

		RiNuPatchV( nu, uorder, uknot, umin,
		            umax, nv, vorder, vknot,
		            vmin, vmax, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiNuPatchV ( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin,
                    RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[],
                    RtFloat vmin, RtFloat vmax,
                    RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiNuPatchV( nu, uorder, uknot, umin,
		                              umax, nv, vorder, vknot,
		                              vmin, vmax, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiTrimCurve ( RtInt nloops, RtInt ncurves[], RtInt order[],
                     RtFloat knot[], RtFloat min[], RtFloat max[],
                     RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[] )
{
	try
	{
		context.current().RiTrimCurve( nloops, ncurves, order,
		                               knot, min, max,
		                               n, u, v, w );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiSphere ( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax, ... )
{
	try
	{
		va_list args;
		va_start( args, tmax );
		CqPLStore pls( args );
		va_end( args );

		RiSphereV( radius, zmin, zmax, tmax, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiSphereV ( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax,
                   RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiSphereV( radius, zmin, zmax, tmax, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiCone ( RtFloat height, RtFloat radius, RtFloat tmax, ... )
{
	try
	{
		va_list args;
		va_start( args, tmax );
		CqPLStore pls( args );
		va_end( args );

		RiConeV( height, radius, tmax, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiConeV ( RtFloat height, RtFloat radius, RtFloat tmax,
                 RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiConeV( height, radius, tmax, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiCylinder ( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax, ... )
{
	try
	{
		va_list args;
		va_start( args, tmax );
		CqPLStore pls( args );
		va_end( args );

		RiCylinderV( radius, zmin, zmax, tmax, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiCylinderV ( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax,
                     RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiCylinderV( radius, zmin, zmax, tmax, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiHyperboloid ( RtPoint point1, RtPoint point2, RtFloat tmax, ... )
{
	try
	{
		va_list args;
		va_start( args, tmax );
		CqPLStore pls( args );
		va_end( args );

		RiHyperboloidV( point1, point2, tmax, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiHyperboloidV ( RtPoint point1, RtPoint point2, RtFloat tmax,
                        RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiHyperboloidV( point1, point2, tmax, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiParaboloid ( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat tmax, ... )
{
	try
	{
		va_list args;
		va_start( args, tmax );
		CqPLStore pls( args );
		va_end( args );

		RiParaboloidV( rmax, zmin, zmax, tmax, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiParaboloidV ( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat tmax,
                       RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiParaboloidV( rmax, zmin, zmax, tmax, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiDisk ( RtFloat height, RtFloat radius, RtFloat tmax, ... )
{
	try
	{
		va_list args;
		va_start( args, tmax );
		CqPLStore pls( args );
		va_end( args );

		RiDiskV( height, radius, tmax, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiDiskV ( RtFloat height, RtFloat radius, RtFloat tmax,
                 RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiDiskV( height, radius, tmax, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiTorus ( RtFloat majrad, RtFloat minrad, RtFloat phimin,
                 RtFloat phimax, RtFloat tmax, ... )
{
	try
	{
		va_list args;
		va_start( args, tmax );
		CqPLStore pls( args );
		va_end( args );

		RiTorusV( majrad, minrad, phimin, phimax, tmax, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiTorusV ( RtFloat majrad, RtFloat minrad,
                  RtFloat phimin, RtFloat phimax, RtFloat tmax,
                  RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiTorusV( majrad, minrad, phimin, phimax, tmax, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiBlobby ( RtInt nleaf, RtInt ncode, RtInt code[],
                  RtInt nflt, RtFloat flt[],
                  RtInt nstr, RtToken str[], ... )
{
	try
	{
		va_list args;
		va_start( args, str );
		CqPLStore pls( args );
		va_end( args );

		RiBlobbyV( nleaf, ncode, code, nflt, flt, nstr, str, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiBlobbyV ( RtInt nleaf, RtInt ncode, RtInt code[],
                   RtInt nflt, RtFloat flt[],
                   RtInt nstr, RtToken str[],
                   RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiBlobbyV( nleaf, ncode, code, nflt, flt, nstr, str, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiCurves ( RtToken type, RtInt ncurves,
                  RtInt nvertices[], RtToken wrap, ... )
{
	try
	{
		va_list args;
		va_start( args, wrap );
		CqPLStore pls( args );
		va_end( args );

		RiCurvesV( type, ncurves, nvertices, wrap, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiCurvesV ( RtToken type, RtInt ncurves,
                   RtInt nvertices[], RtToken wrap,
                   RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiCurvesV( type, ncurves, nvertices, wrap, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPoints( RtInt npoints, ... )
{
	try
	{
		va_list args;
		va_start( args, npoints );
		CqPLStore pls( args );
		va_end( args );

		RiPointsV( npoints, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiPointsV( RtInt npoints, RtInt n, RtToken tokens[], RtPointer parms[] )
{

	try
	{
		context.current().RiPointsV( npoints, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiSubdivisionMesh ( RtToken mask, RtInt nf, RtInt nverts[],
                           RtInt verts[],
                           RtInt ntags, RtToken tags[], RtInt numargs[],
                           RtInt intargs[], RtFloat floatargs[], ... )
{
	try
	{
		va_list args;
		va_start( args, floatargs );
		CqPLStore pls( args );
		va_end( args );

		RiSubdivisionMeshV( mask, nf, nverts, verts, ntags, tags, numargs,
		                    intargs, floatargs, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiSubdivisionMeshV ( RtToken mask, RtInt nf, RtInt nverts[],
                            RtInt verts[],
                            RtInt ntags, RtToken tags[], RtInt numargs[],
                            RtInt intargs[], RtFloat floatargs[],
                            RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiSubdivisionMeshV( mask, nf, nverts, verts,
		                                      ntags, tags, numargs, intargs, floatargs,
		                                      n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiProcedural ( RtPointer data, RtBound bound,
                      RtVoid ( *subdivfunc ) ( RtPointer, RtFloat ),
                      RtVoid ( *freefunc ) ( RtPointer ) )
{
	try
	{
		context.current().RiProcedural( data, bound, subdivfunc, freefunc );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiGeometry ( RtToken type, ... )
{
	try
	{
		va_list args;
		va_start( args, type );
		CqPLStore pls( args );
		va_end( args );

		RiGeometryV( type, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiGeometryV ( RtToken type, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiGeometryV( type, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiSolidBegin ( RtToken operation )
{
	try
	{
		context.current().RiSolidBegin( operation );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiSolidEnd ( void )
{
	try
	{
		context.current().RiSolidEnd();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtObjectHandle RiObjectBegin ( void )
{
	try
	{
		return context.current().RiObjectBegin();
	}
	catch ( CqError & r )
	{
		r.manage();
		return RI_NULL;
	}
}

RtVoid RiObjectEnd ( void )
{
	try
	{
		context.current().RiObjectEnd();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiObjectInstance ( RtObjectHandle handle )
{
	try
	{
		context.current().RiObjectInstance( handle );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMotionBegin ( RtInt n, ... )
{
	try
	{
		va_list args;
		va_start( args, n );
		RtFloat* times = new RtFloat[ n ];

		for ( RtInt i = 0;i < n;i++ )
		{
			times[ i ] = va_arg( args, double );
		}
		va_end( args );

		RiMotionBeginV( n, times );
		delete[] ( times );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMotionBeginV ( RtInt n, RtFloat times[] )
{
	try
	{
		context.current().RiMotionBeginV( n, times );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMotionEnd ( void )
{
	try
	{
		context.current().RiMotionEnd();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMakeTexture ( RtToken pic, RtToken tex, RtToken swrap, RtToken twrap,
                       RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... )
{
	try
	{
		va_list args;
		va_start( args, twidth );
		CqPLStore pls( args );
		va_end( args );

		RiMakeTextureV( pic, tex, swrap, twrap, filterfunc, swidth, twidth,
		                pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMakeTextureV ( RtToken pic, RtToken tex, RtToken swrap, RtToken twrap,
                        RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
                        RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiMakeTextureV( pic, tex, swrap, twrap,
		                                  filterfunc, swidth, twidth,
		                                  n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMakeBump ( RtToken pic, RtToken tex, RtToken swrap, RtToken twrap,
                    RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... )
{
	try
	{
		va_list args;
		va_start( args, twidth );
		CqPLStore pls( args );
		va_end( args );

		RiMakeBumpV( pic, tex, swrap, twrap, filterfunc, swidth, twidth,
		             pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMakeBumpV ( RtToken pic, RtToken tex, RtToken swrap, RtToken twrap,
                     RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
                     RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiMakeBumpV( pic, tex, swrap, twrap,
		                               filterfunc, swidth, twidth,
		                               n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMakeLatLongEnvironment ( RtToken pic, RtToken tex,
                                  RtFilterFunc filterfunc,
                                  RtFloat swidth, RtFloat twidth, ... )
{
	try
	{
		va_list args;
		va_start( args, twidth );
		CqPLStore pls( args );
		va_end( args );

		RiMakeLatLongEnvironmentV( pic, tex, filterfunc, swidth, twidth,
		                           pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMakeLatLongEnvironmentV ( RtToken pic, RtToken tex,
                                   RtFilterFunc filterfunc,
                                   RtFloat swidth, RtFloat twidth,
                                   RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiMakeLatLongEnvironmentV( pic, tex, filterfunc,
		        swidth, twidth,
		        n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMakeCubeFaceEnvironment ( RtToken px, RtToken nx, RtToken py, RtToken ny,
                                   RtToken pz, RtToken nz, RtToken tex, RtFloat fov,
                                   RtFilterFunc filterfunc,
                                   RtFloat swidth, RtFloat ywidth, ... )
{
	try
	{
		va_list args;
		va_start( args, ywidth );
		CqPLStore pls( args );
		va_end( args );

		RiMakeCubeFaceEnvironmentV( px, nx, py, ny, pz, nz, tex, fov,
		                            filterfunc, swidth, ywidth,
		                            pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMakeCubeFaceEnvironmentV ( RtToken px, RtToken nx, RtToken py, RtToken ny,
                                    RtToken pz, RtToken nz, RtToken tex, RtFloat fov,
                                    RtFilterFunc filterfunc, RtFloat swidth, RtFloat ywidth,
                                    RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiMakeCubeFaceEnvironmentV( px, nx, py, ny, pz, nz, tex, fov,
		        filterfunc, swidth, ywidth,
		        n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMakeShadow ( RtToken pic, RtToken tex, ... )
{

	try
	{
		va_list args;
		va_start( args, tex );
		CqPLStore pls( args );
		va_end( args );

		RiMakeShadowV( pic, tex, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiMakeShadowV ( RtToken pic, RtToken tex,
                       RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiMakeShadowV( pic, tex, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiArchiveRecord ( RtToken type, char *format, ... )
{
	try
	{
		va_list args;
		va_start( args, format );

		TqInt size = 256;
		char* buffer = new char[ size ];
#if defined(AQSIS_COMPILER_MSVC6) || defined(AQSIS_COMPILER_MSVC7) 

		while ( _vsnprintf( buffer, 256, format, args ) < 0 )
#else

		while ( vsnprintf( buffer, 256, format, args ) < 0 )
#endif

		{
			size *= 2;
			delete[] ( buffer );
			buffer = new char[ size ];
		}
		std::string i( buffer );
		delete[] ( buffer );

		va_end( args );
		context.current().RiArchiveRecord( type, i );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiReadArchive( RtToken name, RtArchiveCallback callback, ... )
{
	try
	{
		va_list args;
		va_start( args, callback );
		CqPLStore pls( args );
		va_end( args );

		RiReadArchiveV( name, callback, pls.n, pls.tokens(), pls.parms() );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiReadArchiveV( RtToken name, RtArchiveCallback callback,
                       RtInt n, RtToken tokens[], RtPointer parms[] )
{
	try
	{
		context.current().RiReadArchiveV( name, callback, n, tokens, parms );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiIfBegin ( RtString condition )
{
	try
	{
		context.current().RiIfBegin( condition );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiElse( )
{
	try
	{
		context.current().RiElse();
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiElseIf ( RtString condition )
{
	try
	{
		context.current().RiElseIf( condition );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid RiIfEnd ( )
{
	try
	{
		context.current().RiIfEnd( );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}
RtVoid RiErrorHandler ( RtErrorFunc handler )
{
	try
	{
		context.current().RiErrorHandler( handler );
	}
	catch ( CqError & r )
	{
		r.manage();
	}
}

RtVoid	RiErrorIgnore( RtInt code, RtInt severity, RtString message )
{
	return ;
}


RtVoid	RiErrorPrint( RtInt code, RtInt severity, RtString message )
{
	return ;
}


RtVoid	RiErrorAbort( RtInt code, RtInt severity, RtString message )
{
	return ;
}
