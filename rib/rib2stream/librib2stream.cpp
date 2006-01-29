#include "librib2stream.h"
#pragma warning (disable : 4786)
#include "parserstate.h"
using namespace librib;


namespace librib2stream
{

Stream::Stream( std::ostream& Stream ) :
		m_Stream( Stream ),
		m_CurrentLightHandle( 1 )
{}

Stream::~Stream()
{}

RendermanInterface::RtLightHandle Stream::RiAreaLightSourceV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiAreaLightSourceV()" << std::endl;
	return reinterpret_cast<RtLightHandle>( m_CurrentLightHandle++ );
}
RendermanInterface::RtVoid Stream::RiAtmosphereV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiAtmosphereV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiAttributeBegin()
{
	m_Stream << "RiAttributeBegin()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiAttributeEnd()
{
	m_Stream << "RiAttributeEnd()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiAttributeV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiAttributeV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiBasis( RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep )
{
	m_Stream << "RiBasis()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiBegin( RtToken name )
{
	m_Stream << "RiBegin(" << name << ")" << std::endl;
}
RendermanInterface::RtFloat Stream::RiBesselFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	m_Stream << "RiBesselFilter()" << std::endl;
	return 0;
}
RendermanInterface::RtVoid Stream::RiBound( RtBound bound )
{
	m_Stream << "RiBound()" << std::endl;
}
RendermanInterface::RtFloat Stream::RiBoxFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	m_Stream << "RiBoxFilter()" << std::endl;
	return 0;
}
RendermanInterface::RtFloat Stream::RiCatmullRomFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	m_Stream << "RiCatmullRomFilter()" << std::endl;
	return 0;
}
RendermanInterface::RtVoid Stream::RiClipping( RtFloat cnear, RtFloat cfar )
{
	m_Stream << "RiClipping()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiColor( RtColor Cq )
{
	m_Stream << "RiColor()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiColorSamples( RtInt N, RtFloat *nRGB, RtFloat *RGBn )
{
	m_Stream << "RiColorSamples()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiConcatTransform( RtMatrix transform )
{
	m_Stream << "RiConcatTransform()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiConeV( RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiConeV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiCoordinateSystem( RtToken space )
{
	m_Stream << "RiCoordinateSystem()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiCoordSysTransform( RtToken space )
{
	m_Stream << "RiCoordSysTransform()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiCropWindow( RtFloat left, RtFloat right, RtFloat top, RtFloat bottom )
{
	m_Stream << "RiCropWindow()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiCylinderV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiCylinderV()" << std::endl;
}
RendermanInterface::RtToken Stream::RiDeclare( RtString name, RtString declaration )
{
	m_Stream << "RiDeclare(" << name << ", " << declaration << ")" << std::endl;
	return 0;
}
RendermanInterface::RtVoid Stream::RiDeformationV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiDeformationV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiDepthOfField( RtFloat fstop, RtFloat focallength, RtFloat focaldistance )
{
	m_Stream << "RiDepthOfField()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiDetail( RtBound bound )
{
	m_Stream << "RiDetail()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiDetailRange( RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh )
{
	m_Stream << "RiDetailRange()" << std::endl;
}
RendermanInterface::RtFloat Stream::RiDiskFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	m_Stream << "RiDiskFilter()" << std::endl;
	return 0;
}
RendermanInterface::RtVoid Stream::RiDiskV( RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiDiskV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiDisplacementV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiDisplacementV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiDisplayV( RtToken name, RtToken type, RtToken mode, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiDisplayV(" << name << ", " << type << ", " << mode << ", " << count << ")" << std::endl;
}
RendermanInterface::RtVoid Stream::RiEnd()
{
	m_Stream << "RiEnd()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiIfBegin(RtString condition)
{
	m_Stream << "RiIfBegin(\"" << condition << "\")" << std::endl;
}
RendermanInterface::RtVoid Stream::RiElse()
{
	m_Stream << "RiElse()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiElseIf(RtString condition)
{
	m_Stream << "RiElseIf(\"" << condition << "\")" << std::endl;
}
RendermanInterface::RtVoid Stream::RiIfEnd()
{
	m_Stream << "RiIfEnd()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiErrorAbort( RtInt code, RtInt severity, char* message )
{
	m_Stream << "RiErrorAbort()" << std::endl;
}
//RendermanInterface::RtVoid Stream::RiErrorHandler(RtErrorFunc handler) { m_Stream << "RiErrorHandler()" << std::endl; }
RendermanInterface::RtVoid Stream::RiErrorIgnore( RtInt code, RtInt severity, char* message )
{
	m_Stream << "RiErrorIgnore()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiErrorPrint( RtInt code, RtInt severity, char* message )
{
	m_Stream << "RiErrorPrint()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiExposure( RtFloat gain, RtFloat gamma )
{
	m_Stream << "RiExposure()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiExteriorV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiExteriorV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiFormat( RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio )
{
	m_Stream << "RiFormat(" << xresolution << ", " << yresolution << ", " << pixelaspectratio << ")" << std::endl;
}
RendermanInterface::RtVoid Stream::RiFrameAspectRatio( RtFloat frameratio )
{
	m_Stream << "RiFrameAspectRatio()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiFrameBegin( RtInt number )
{
	m_Stream << "RiFrameBegin()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiFrameEnd()
{
	m_Stream << "RiFrameEnd()" << std::endl;
}
RendermanInterface::RtFloat Stream::RiGaussianFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	m_Stream << "RiGaussianFilter()" << std::endl;
	return 0;
}
RendermanInterface::RtVoid Stream::RiGeneralPolygonV( RtInt nloops, RtInt nverts[], RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiGeneralPolygonV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiGeometricApproximation( RtToken type, RtFloat value )
{
	m_Stream << "RiGeometricApproximation()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiGeometryV( RtToken type, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiGeometryV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiHiderV( RtToken type, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiHiderV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiHyperboloidV( RtPoint point1, RtPoint point2, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiHyperboloidV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiIdentity()
{
	m_Stream << "RiIdentity()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiIlluminate( RtLightHandle light, RtBoolean onoff )
{
	m_Stream << "RiIlluminate()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiImagerV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiImagerV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiInteriorV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiInteriorV()" << std::endl;
}
RendermanInterface::RtLightHandle Stream::RiLightSourceV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiLightSourceV()" << std::endl;
	return reinterpret_cast<RtLightHandle>( m_CurrentLightHandle++ );
}
RendermanInterface::RtVoid Stream::RiMakeBumpV( RtString imagefile, RtString bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiMakeBumpV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiMakeCubeFaceEnvironmentV( RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiMakeCubeFaceEnvironmentV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiMakeLatLongEnvironmentV( RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiMakeLatLongEnvironmentV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiMakeShadowV( RtString picfile, RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiMakeShadowV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiMakeTextureV( RtString imagefile, RtString texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiMakeTextureV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiMakeOcclusionV( RtInt picount, RtString picfile[], RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiMakeOcclusionV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiMatte( RtBoolean onoff )
{
	m_Stream << "RiMatte()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiMotionBeginV( RtInt N, RtFloat times[] )
{
	m_Stream << "RiMotionBeginV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiMotionEnd()
{
	m_Stream << "RiMotionEnd()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiNuPatchV( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiNuPatchV()" << std::endl;
}
RendermanInterface::RtObjectHandle Stream::RiObjectBegin()
{
	m_Stream << "RiObjectBegin()" << std::endl;
	return 0;
}
RendermanInterface::RtVoid Stream::RiObjectEnd()
{
	m_Stream << "RiObjectEnd()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiObjectInstance( RtObjectHandle handle )
{
	m_Stream << "RiObjectInstance()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiOpacity( RtColor Os )
{
	m_Stream << "RiOpacity()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiOptionV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiOptionV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiOrientation( RtToken orientation )
{
	m_Stream << "RiOrientation()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiParaboloidV( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiParaboloidV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiPatchMeshV( RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiPatchMeshV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiPatchV( RtToken type, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiPatchV(" << type << ", " << count << ")" << std::endl;
}
RendermanInterface::RtVoid Stream::RiPerspective( RtFloat fov )
{
	m_Stream << "RiPerspective()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiPixelFilter( RtFilterFunc function, RtFloat xwidth, RtFloat ywidth )
{
	m_Stream << "RiPixelFilter()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiPixelSamples( RtFloat xsamples, RtFloat ysamples )
{
	m_Stream << "RiPixelSamples()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiPixelVariance( RtFloat variance )
{
	m_Stream << "RiPixelVariance()" << std::endl;
}

RendermanInterface::RtVoid Stream::RiProcedural( RtPointer data, RtBound bound, RtFunc refineproc, RtFunc freeproc )
{
	m_Stream << "RiProcedural()" << std::endl;
}

RendermanInterface::RtVoid Stream::RiCurvesV( RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap,
        RtInt n, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiCurvesV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiPointsV( RtInt vertices, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiPointsV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiPointsGeneralPolygonsV( RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiPointsGeneralPolygonsV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiPointsPolygonsV( RtInt npolys, RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiPointsPolygonsV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiPolygonV( RtInt nvertices, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiPolygonV(" << nvertices << ", " << count << ")" << std::endl;
}

RendermanInterface::RtVoid Stream::RiBlobbyV( RtInt nleaf, RtInt ncode, RtInt code[], RtInt nflt, RtFloat flt[], RtInt nstr, RtToken str[], RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiBlobbyV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiProjectionV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiProjectionV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiQuantize( RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude )
{
	m_Stream << "RiQuantize()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiReadArchive( RtToken data, RtArchiveCallback callback )
{
	m_Stream << "RiReadArchive()" << std::endl;

	//CqRiFile	fileArchive( name, "archive" );
	FILE* file = fopen( data, "rb" );

	if ( NULL != file )
	{
		librib::CqRIBParserState currstate = librib::GetParserState();
		if (currstate.m_pParseCallbackInterface == NULL)
			currstate.m_pParseCallbackInterface = new librib2stream::Stream(std::cout);
		librib::Parse( file, data, *(currstate.m_pParseCallbackInterface), *(currstate.m_pParseErrorStream), callback );
		librib::SetParserState( currstate );
		fclose(file);
	}
}
RendermanInterface::RtVoid Stream::RiRelativeDetail( RtFloat relativedetail )
{
	m_Stream << "RiRelativeDetail()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiReverseOrientation()
{
	m_Stream << "RiReverseOrientation()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiRotate( RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz )
{
	m_Stream << "RiRotate()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiScale( RtFloat sx, RtFloat sy, RtFloat sz )
{
	m_Stream << "RiScale()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiScreenWindow( RtFloat left, RtFloat right, RtFloat bottom, RtFloat top )
{
	m_Stream << "RiScreenWindow()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiShadingInterpolation( RtToken type )
{
	m_Stream << "RiShadingInterpolation()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiShadingRate( RtFloat size )
{
	m_Stream << "RiShadingRate()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiShutter( RtFloat opentime, RtFloat closetime )
{
	m_Stream << "RiShutter()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiSides( RtInt nsides )
{
	m_Stream << "RiSides()" << std::endl;
}
RendermanInterface::RtFloat Stream::RiSincFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	m_Stream << "RiSincFilter()" << std::endl;
	return 0;
}
RendermanInterface::RtVoid Stream::RiSkew( RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1, RtFloat dx2, RtFloat dy2, RtFloat dz2 )
{
	m_Stream << "RiSkew()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiSolidBegin( RtToken type )
{
	m_Stream << "RiSolidBegin()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiSolidEnd()
{
	m_Stream << "RiSolidEnd()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiSphereV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiSphereV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiSubdivisionMeshV( RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiSubdivisionMeshV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiSurfaceV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiSurfaceV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiTextureCoordinates( RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4 )
{
	m_Stream << "RiTextureCoordinates()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiTorusV( RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiTorusV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiTransform( RtMatrix transform )
{
	m_Stream << "RiTransform()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiTransformBegin()
{
	m_Stream << "RiTransformBegin()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiTransformEnd()
{
	m_Stream << "RiTransformEnd()" << std::endl;
}
RendermanInterface::RtPoint* Stream::RiTransformPoints( RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[] )
{
	m_Stream << "RiTransformPoints()" << std::endl;
	return 0;
}
RendermanInterface::RtVoid Stream::RiTranslate( RtFloat dx, RtFloat dy, RtFloat dz )
{
	m_Stream << "RiTranslate()" << std::endl;
}
RendermanInterface::RtFloat Stream::RiTriangleFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	m_Stream << "RiTriangleFilter()" << std::endl;
	return 0;
}
RendermanInterface::RtVoid Stream::RiTrimCurve( RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[] )
{
	m_Stream << "RiTrimCurve()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiWorldBegin()
{
	m_Stream << "RiWorldBegin()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiWorldEnd()
{
	m_Stream << "RiWorldEnd()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiShaderLayerV( RtToken type, RtToken name, RtToken layername, RtInt count, RtToken tokens[], RtPointer values[] )
{
	m_Stream << "RiShaderLayerV()" << std::endl;
}
RendermanInterface::RtVoid Stream::RiConnectShaderLayers( RtToken type, RtToken layer1, RtToken variable1, RtToken layer2, RtToken variable2 )
{
	m_Stream << "RiConnectShaderLayers()" << std::endl;
}

RendermanInterface::RtFilterFunc Stream::GetFilterFunction( RtToken type )
{
	m_Stream << "Filter: " << type << std::endl;
	return ( NULL );
}
RendermanInterface::RtBasis* Stream::GetBasisMatrix( RtToken type )
{
	m_Stream << "Filter: " << type << std::endl;
	return ( NULL );
}
RendermanInterface::RtFunc Stream::GetProceduralFunction( RtToken type )
{
	m_Stream << "Filter: " << type << std::endl;
	return ( NULL );
}

RendermanInterface* CreateRIBEngine()
{
	return ( new Stream(std::cout) );
}

void DestroyRIBEngine( RendermanInterface* engine )
{
	delete( engine );
}

}
