#include "librib2ri.h"
using namespace librib;

#include "ri.h"

namespace librib2ri
{

Engine::Engine()
{}

Engine::~Engine()
{}

RendermanInterface::RtLightHandle Engine::RiAreaLightSourceV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	return ::RiAreaLightSourceV( name, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiAtmosphereV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiAtmosphereV( name, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiAttributeBegin()
{
	::RiAttributeBegin();
}
RendermanInterface::RtVoid Engine::RiAttributeEnd()
{
	::RiAttributeEnd();
}
RendermanInterface::RtVoid Engine::RiAttributeV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiAttributeV( name, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiBasis( RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep )
{
	::RiBasis( ubasis, ustep, vbasis, vstep );
}
RendermanInterface::RtVoid Engine::RiBegin( RtToken name )
{
	::RiBegin( name );
}
RendermanInterface::RtFloat Engine::RiBesselFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return ::RiBesselFilter( x, y, xwidth, ywidth );
}
RendermanInterface::RtVoid Engine::RiBlobbyV( RtInt nleaf, RtInt ncode, RtInt code[], RtInt nflt, RtFloat flt[], RtInt nstr, RtToken str[], RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiBlobbyV( nleaf, ncode, code, nflt, flt, nstr, str, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiBound( RtBound bound )
{
	::RiBound( bound );
}
RendermanInterface::RtFloat Engine::RiMitchellFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return ::RiMitchellFilter( x, y, xwidth, ywidth );
	return 0;
}
RendermanInterface::RtFloat Engine::RiBoxFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return ::RiBoxFilter( x, y, xwidth, ywidth );
}
RendermanInterface::RtFloat Engine::RiCatmullRomFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return ::RiCatmullRomFilter( x, y, xwidth, ywidth );
}
RendermanInterface::RtVoid Engine::RiClipping( RtFloat cnear, RtFloat cfar )
{
	::RiClipping( cnear, cfar );
}
RendermanInterface::RtVoid Engine::RiColor( RtColor Cq )
{
	::RiColor( Cq );
}
RendermanInterface::RtVoid Engine::RiColorSamples( RtInt N, RtFloat *nRGB, RtFloat *RGBn )
{
	::RiColorSamples( N, nRGB, RGBn );
}
RendermanInterface::RtVoid Engine::RiConcatTransform( RtMatrix transform )
{
	::RiConcatTransform( transform );
}
RendermanInterface::RtVoid Engine::RiConeV( RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiConeV( height, radius, thetamax, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiCoordinateSystem( RtToken space )
{
	::RiCoordinateSystem( space );
}
RendermanInterface::RtVoid Engine::RiCoordSysTransform( RtToken space )
{
	::RiCoordSysTransform( space );
}
RendermanInterface::RtVoid Engine::RiCropWindow( RtFloat left, RtFloat right, RtFloat top, RtFloat bottom )
{
	::RiCropWindow( left, right, top, bottom );
}
RendermanInterface::RtVoid Engine::RiCylinderV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiCylinderV( radius, zmin, zmax, thetamax, count, tokens, values );
}
RendermanInterface::RtToken Engine::RiDeclare( RtString name, RtString declaration )
{
	return ::RiDeclare( name, declaration );
}
RendermanInterface::RtVoid Engine::RiDeformationV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiDeformationV( name, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiDepthOfField( RtFloat fstop, RtFloat focallength, RtFloat focaldistance )
{
	::RiDepthOfField( fstop, focallength, focaldistance );
}
RendermanInterface::RtVoid Engine::RiDetail( RtBound bound )
{
	::RiDetail( bound );
}
RendermanInterface::RtVoid Engine::RiDetailRange( RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh )
{
	::RiDetailRange( offlow, onlow, onhigh, offhigh );
}
RendermanInterface::RtFloat Engine::RiDiskFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return ::RiDiskFilter( x, y, xwidth, ywidth );
}
RendermanInterface::RtVoid Engine::RiDiskV( RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiDiskV( height, radius, thetamax, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiDisplacementV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiDisplacementV( name, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiDisplayV( RtToken name, RtToken type, RtToken mode, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiDisplayV( name, type, mode, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiEnd()
{
	::RiEnd();
}

RendermanInterface::RtVoid Engine::RiIfBegin(RtString condition)
{
	::RiIfBegin(condition);
}
RendermanInterface::RtVoid Engine::RiIfEnd()
{
	::RiIfEnd();
}
RendermanInterface::RtVoid Engine::RiElse()
{
	::RiElse();
}
RendermanInterface::RtVoid Engine::RiElseIf(RtString condition)
{
	::RiElseIf(condition);
}
RendermanInterface::RtVoid Engine::RiErrorAbort( RtInt code, RtInt severity, RtString message )
{
	::RiErrorAbort( code, severity, message );
}
//RendermanInterface::RtVoid Engine::RiErrorHandler(RtErrorFunc handler) { ::RiErrorHandler(handler); }
RendermanInterface::RtVoid Engine::RiErrorIgnore( RtInt code, RtInt severity, RtString message )
{
	::RiErrorIgnore( code, severity, message );
}
RendermanInterface::RtVoid Engine::RiErrorPrint( RtInt code, RtInt severity, RtString message )
{
	::RiErrorPrint( code, severity, message );
}
RendermanInterface::RtVoid Engine::RiExposure( RtFloat gain, RtFloat gamma )
{
	::RiExposure( gain, gamma );
}
RendermanInterface::RtVoid Engine::RiExteriorV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiExteriorV( name, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiFormat( RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio )
{
	::RiFormat( xresolution, yresolution, pixelaspectratio );
}
RendermanInterface::RtVoid Engine::RiFrameAspectRatio( RtFloat frameratio )
{
	::RiFrameAspectRatio( frameratio );
}
RendermanInterface::RtVoid Engine::RiFrameBegin( RtInt number )
{
	::RiFrameBegin( number );
}
RendermanInterface::RtVoid Engine::RiFrameEnd()
{
	::RiFrameEnd();
}
RendermanInterface::RtFloat Engine::RiGaussianFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return ::RiGaussianFilter( x, y, xwidth, ywidth );
}
RendermanInterface::RtVoid Engine::RiGeneralPolygonV( RtInt nloops, RtInt nverts[], RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiGeneralPolygonV( nloops, nverts, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiGeometricApproximation( RtToken type, RtFloat value )
{
	::RiGeometricApproximation( type, value );
}
RendermanInterface::RtVoid Engine::RiGeometryV( RtToken type, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiGeometryV( type, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiHiderV( RtToken type, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiHiderV( type, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiHyperboloidV( RtPoint point1, RtPoint point2, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiHyperboloidV( point1, point2, thetamax, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiIdentity()
{
	::RiIdentity();
}
RendermanInterface::RtVoid Engine::RiIlluminate( RtLightHandle light, RtBoolean onoff )
{
	::RiIlluminate( light, onoff );
}
RendermanInterface::RtVoid Engine::RiImagerV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiImagerV( name, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiInteriorV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiInteriorV( name, count, tokens, values );
}
RendermanInterface::RtLightHandle Engine::RiLightSourceV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	return ::RiLightSourceV( name, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiMakeBumpV( RtString imagefile, RtString bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiMakeBumpV( imagefile, bumpfile, swrap, twrap, filterfunc, swidth, twidth, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiMakeCubeFaceEnvironmentV( RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiMakeCubeFaceEnvironmentV( px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiMakeLatLongEnvironmentV( RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiMakeLatLongEnvironmentV( imagefile, reflfile, filterfunc, swidth, twidth, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiMakeShadowV( RtString picfile, RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiMakeShadowV( picfile, shadowfile, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiMakeOcclusionV( RtInt npics, RtString picfile[], RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiMakeOcclusionV( npics, picfile, shadowfile, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiMakeTextureV( RtString imagefile, RtString texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiMakeTextureV( imagefile, texturefile, swrap, twrap, filterfunc, swidth, twidth, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiMatte( RtBoolean onoff )
{
	::RiMatte( onoff );
}
RendermanInterface::RtVoid Engine::RiMotionBeginV( RtInt N, RtFloat times[] )
{
	::RiMotionBeginV( N, times );
}
RendermanInterface::RtVoid Engine::RiMotionEnd()
{
	::RiMotionEnd();
}
RendermanInterface::RtVoid Engine::RiNuPatchV( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiNuPatchV( nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, count, tokens, values );
}
RendermanInterface::RtObjectHandle Engine::RiObjectBegin()
{
	return ::RiObjectBegin();
}
RendermanInterface::RtVoid Engine::RiObjectEnd()
{
	::RiObjectEnd();
}
RendermanInterface::RtVoid Engine::RiObjectInstance( RtObjectHandle handle )
{
	::RiObjectInstance( handle );
}
RendermanInterface::RtVoid Engine::RiOpacity( RtColor Os )
{
	::RiOpacity( Os );
}
RendermanInterface::RtVoid Engine::RiOptionV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiOptionV( name, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiOrientation( RtToken orientation )
{
	::RiOrientation( orientation );
}
RendermanInterface::RtVoid Engine::RiParaboloidV( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiParaboloidV( rmax, zmin, zmax, thetamax, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiPatchMeshV( RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiPatchMeshV( type, nu, uwrap, nv, vwrap, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiPatchV( RtToken type, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiPatchV( type, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiPerspective( RtFloat fov )
{
	::RiPerspective( fov );
}
RendermanInterface::RtVoid Engine::RiPixelFilter( RtFilterFunc function, RtFloat xwidth, RtFloat ywidth )
{
	::RiPixelFilter( function, xwidth, ywidth );
}
RendermanInterface::RtVoid Engine::RiPixelSamples( RtFloat xsamples, RtFloat ysamples )
{
	::RiPixelSamples( xsamples, ysamples );
}
RendermanInterface::RtVoid Engine::RiPixelVariance( RtFloat variance )
{
	::RiPixelVariance( variance );
}

RendermanInterface::RtVoid Engine::RiCurvesV( RtToken type, RtInt ncurves,
        RtInt nvertices[], RtToken wrap,
        RtInt n, RtToken tokens[], RtPointer values[] )
{
	::RiCurvesV( type, ncurves, nvertices, wrap, n, tokens, values );
}
RendermanInterface::RtVoid Engine::RiPointsV( RtInt vertices, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiPointsV( vertices, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiPointsGeneralPolygonsV( RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiPointsGeneralPolygonsV( npolys, nloops, nverts, verts, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiPointsPolygonsV( RtInt npolys, RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiPointsPolygonsV( npolys, nverts, verts, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiPolygonV( RtInt nvertices, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiPolygonV( nvertices, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiProcedural( RtPointer data, RtBound bound, RtFunc refineproc, RtFunc freeproc )
{
	::RiProcedural( data, bound, ( RtProcSubdivFunc ) refineproc, ( RtProcFreeFunc ) freeproc );
}
RendermanInterface::RtVoid Engine::RiProjectionV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiProjectionV( name, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiQuantize( RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude )
{
	::RiQuantize( type, one, min, max, ditheramplitude );
}
RendermanInterface::RtVoid Engine::RiReadArchive( RtToken data, RtArchiveCallback callback )
{
	::RiReadArchive( data , callback );
}
RendermanInterface::RtVoid Engine::RiRelativeDetail( RtFloat relativedetail )
{
	::RiRelativeDetail( relativedetail );
}
RendermanInterface::RtVoid Engine::RiReverseOrientation()
{
	::RiReverseOrientation();
}
RendermanInterface::RtVoid Engine::RiRotate( RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz )
{
	::RiRotate( angle, dx, dy, dz );
}
RendermanInterface::RtVoid Engine::RiResourceV(RtToken handle, RtToken type, RtInt count, RtToken tokens[], RtPointer values[])
{
	::RiResource(handle, type, count, tokens, values);
}
RendermanInterface::RtVoid Engine::RiResourceBegin()
{
	::RiResourceBegin();
}
RendermanInterface::RtVoid Engine::RiResourceEnd()
{
	::RiResourceEnd();
}
RendermanInterface::RtVoid Engine::RiScale( RtFloat sx, RtFloat sy, RtFloat sz )
{
	::RiScale( sx, sy, sz );
}
RendermanInterface::RtVoid Engine::RiScreenWindow( RtFloat left, RtFloat right, RtFloat bottom, RtFloat top )
{
	::RiScreenWindow( left, right, bottom, top );
}
RendermanInterface::RtVoid Engine::RiShadingInterpolation( RtToken type )
{
	::RiShadingInterpolation( type );
}
RendermanInterface::RtVoid Engine::RiShadingRate( RtFloat size )
{
	::RiShadingRate( size );
}
RendermanInterface::RtVoid Engine::RiShutter( RtFloat opentime, RtFloat closetime )
{
	::RiShutter( opentime, closetime );
}
RendermanInterface::RtVoid Engine::RiSides( RtInt nsides )
{
	::RiSides( nsides );
}
RendermanInterface::RtFloat Engine::RiSincFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return ::RiSincFilter( x, y, xwidth, ywidth );
}
RendermanInterface::RtVoid Engine::RiSkew( RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1, RtFloat dx2, RtFloat dy2, RtFloat dz2 )
{
	::RiSkew( angle, dx1, dy1, dz1, dx2, dy2, dz2 );
}
RendermanInterface::RtVoid Engine::RiSolidBegin( RtToken type )
{
	::RiSolidBegin( type );
}
RendermanInterface::RtVoid Engine::RiSolidEnd()
{
	::RiSolidEnd();
}
RendermanInterface::RtVoid Engine::RiSphereV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiSphereV( radius, zmin, zmax, thetamax, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiSubdivisionMeshV( RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiSubdivisionMeshV( scheme, nfaces, nvertices, vertices, ntags, tags, nargs, intargs, floatargs, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiSurfaceV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiSurfaceV( name, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiTextureCoordinates( RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4 )
{
	::RiTextureCoordinates( s1, t1, s2, t2, s3, t3, s4, t4 );
}
RendermanInterface::RtVoid Engine::RiTorusV( RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiTorusV( majorrad, minorrad, phimin, phimax, thetamax, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiTransform( RtMatrix transform )
{
	::RiTransform( transform );
}
RendermanInterface::RtVoid Engine::RiTransformBegin()
{
	::RiTransformBegin();
}
RendermanInterface::RtVoid Engine::RiTransformEnd()
{
	::RiTransformEnd();
}
RendermanInterface::RtPoint* Engine::RiTransformPoints( RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[] )
{
	return ::RiTransformPoints( fromspace, tospace, npoints, points );
}
RendermanInterface::RtVoid Engine::RiTranslate( RtFloat dx, RtFloat dy, RtFloat dz )
{
	::RiTranslate( dx, dy, dz );
}
RendermanInterface::RtFloat Engine::RiTriangleFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	return ::RiTriangleFilter( x, y, xwidth, ywidth );
}
RendermanInterface::RtVoid Engine::RiTrimCurve( RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[] )
{
	::RiTrimCurve( nloops, ncurves, order, knot, min, max, n, u, v, w );
}
RendermanInterface::RtVoid Engine::RiWorldBegin()
{
	::RiWorldBegin();
}
RendermanInterface::RtVoid Engine::RiWorldEnd()
{
	::RiWorldEnd();
}
RendermanInterface::RtVoid Engine::RiShaderLayerV( RtToken type, RtToken name, RtToken layername, RtInt count, RtToken tokens[], RtPointer values[] )
{
	::RiShaderLayerV( type, name, layername, count, tokens, values );
}
RendermanInterface::RtVoid Engine::RiConnectShaderLayers( RtToken type, RtToken layer1, RtToken variable1, RtToken layer2, RtToken variable2 )
{
	::RiConnectShaderLayers( type, layer1, variable1, layer2, variable2 );
}
RendermanInterface::RtFilterFunc Engine::GetFilterFunction( RtToken type )
{
	if ( strcmp( type, "box" ) == 0 )
		return ( &::RiBoxFilter );
	else if ( strcmp( type, "gaussian" ) == 0 )
		return ( &::RiGaussianFilter );
	else if ( strcmp( type, "triangle" ) == 0 )
		return ( &::RiTriangleFilter );
	else if ( strcmp( type, "mitchell" ) == 0 )
		return ( &::RiMitchellFilter );
	else if ( strcmp( type, "catmull-rom" ) == 0 )
		return ( &::RiCatmullRomFilter );
	else if ( strcmp( type, "sinc" ) == 0 )
		return ( &::RiSincFilter );
	else if ( strcmp( type, "bessel" ) == 0 )
		return ( &::RiBesselFilter );
	else if ( strcmp( type, "disk" ) == 0 )
		return ( &::RiDiskFilter );

	return ( NULL );
}
RendermanInterface::RtBasis* Engine::GetBasisMatrix( RtToken type )
{
	if ( strcmp( type, "bezier" ) == 0 )
		return ( &::RiBezierBasis );
	else if ( strcmp( type, "b-spline" ) == 0 )
		return ( &::RiBSplineBasis );
	else if ( strcmp( type, "catmull-rom" ) == 0 )
		return ( &::RiCatmullRomBasis );
	else if ( strcmp( type, "hermite" ) == 0 )
		return ( &::RiHermiteBasis );
	else if ( strcmp( type, "power" ) == 0 )
		return ( &::RiPowerBasis );

	return ( NULL );
}
RendermanInterface::RtFunc Engine::GetProceduralFunction( RtToken type )
{
	if ( strcmp( type, "DelayedReadArchive" ) == 0 )
		return ( ( void ( * ) ( void ) ) &::RiProcDelayedReadArchive );
	else if ( strcmp( type, "RunProgram" ) == 0 )
		return ( ( void ( * ) ( void ) ) &::RiProcRunProgram );
	else if ( strcmp( type, "DynamicLoad" ) == 0 )
		return ( ( void ( * ) ( void ) ) &::RiProcDynamicLoad );
	else if ( strcmp( type, "RiProcFree" ) == 0 )
		return ( ( void ( * ) ( void ) ) &::RiProcFree );

	return ( NULL );
}


/**
  Create an instance of a RendermanInterface that sends the requests to the RenderMan Interface "C" API.

  The returned object can be used to render RIB files. When the object is no longer
  used it must be destroyed by calling DestroyRIBEngine().

  \return RendermanInterface that passes all requests to the RenderMan Interface "C" API.
  \see DestroyRIBEngine()
 */
RendermanInterface* CreateRIBEngine()
{
	return ( new Engine() );
}

/**
  Destroy a object created with CreateRIBEngine().

  \param engine This is the return value from CreateRIBEngine()
  \see CreateRIBEngine()
 */
void DestroyRIBEngine( RendermanInterface* engine )
{
	delete( engine );
}


}
; // namespace librib2ri

