#ifndef LIBRIB2STREAM_H
#define LIBRIB2STREAM_H

#include "librib.h"
#include <iostream>

namespace librib2stream
{

/// Implements the librib::RendermanInterface interface, and sends data to a stream (typically, for debugging librib)
class Stream :
			public librib::RendermanInterface
{
	public:
		Stream( std::ostream& Stream );
		virtual ~Stream();

		virtual	RtLightHandle RiAreaLightSourceV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiAtmosphereV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiAttributeBegin();
		virtual	RtVoid	RiAttributeEnd();
		virtual	RtVoid	RiAttributeV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiBasis( RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep );
		virtual	RtVoid	RiBegin( RtToken name );
		virtual	RtFloat RiBesselFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
		virtual	RtVoid	RiBound( RtBound bound );
		virtual	RtFloat RiBoxFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
		virtual	RtFloat RiMitchellFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
		virtual	RtFloat RiCatmullRomFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
		virtual	RtVoid	RiClipping( RtFloat cnear, RtFloat cfar );
		virtual	RtVoid	RiColor( RtColor Cq );
		virtual	RtVoid	RiColorSamples( RtInt N, RtFloat *nRGB, RtFloat *RGBn );
		virtual	RtVoid	RiConcatTransform( RtMatrix transform );
		virtual	RtVoid	RiConeV( RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiCoordinateSystem( RtToken space );
		virtual	RtVoid	RiCoordSysTransform( RtToken space );
		virtual	RtVoid	RiCropWindow( RtFloat left, RtFloat right, RtFloat top, RtFloat bottom );
		virtual	RtVoid	RiCylinderV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtToken	RiDeclare( RtString name, RtString declaration );
		virtual	RtVoid	RiDeformationV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiDepthOfField( RtFloat fstop, RtFloat focallength, RtFloat focaldistance );
		virtual	RtVoid	RiDetail( RtBound bound );
		virtual	RtVoid	RiDetailRange( RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh );
		virtual	RtFloat RiDiskFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
		virtual	RtVoid	RiDiskV( RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiDisplacementV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiDisplayV( RtToken name, RtToken type, RtToken mode, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiEnd();
		virtual	RtVoid	RiIfBegin(RtString condition);
		virtual	RtVoid	RiIfEnd();
		virtual	RtVoid	RiElse();
		virtual	RtVoid	RiElseIf(RtString condition);
		virtual	RtVoid	RiErrorAbort( RtInt code, RtInt severity, char* message );
		//virtual	RtVoid	RiErrorHandler(RtErrorFunc handler);
		virtual	RtVoid	RiErrorIgnore( RtInt code, RtInt severity, char* message );
		virtual	RtVoid	RiErrorPrint( RtInt code, RtInt severity, char* message );
		virtual	RtVoid	RiExposure( RtFloat gain, RtFloat gamma );
		virtual	RtVoid	RiExteriorV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiFormat( RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio );
		virtual	RtVoid	RiFrameAspectRatio( RtFloat frameratio );
		virtual	RtVoid	RiFrameBegin( RtInt number );
		virtual	RtVoid	RiFrameEnd();
		virtual	RtFloat RiGaussianFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
		virtual	RtVoid	RiGeneralPolygonV( RtInt nloops, RtInt nverts[], RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiGeometricApproximation( RtToken type, RtFloat value );
		virtual	RtVoid	RiGeometryV( RtToken type, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiHiderV( RtToken type, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiHyperboloidV( RtPoint point1, RtPoint point2, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiIdentity();
		virtual	RtVoid	RiIlluminate( RtLightHandle light, RtBoolean onoff );
		virtual	RtVoid	RiImagerV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiInteriorV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtLightHandle RiLightSourceV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiMakeBumpV( RtString imagefile, RtString bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiMakeCubeFaceEnvironmentV( RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiMakeLatLongEnvironmentV( RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiMakeShadowV( RtString picfile, RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiMakeTextureV( RtString imagefile, RtString texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiMakeOcclusionV( RtInt piccount, RtString *picfile, RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiMatte( RtBoolean onoff );
		virtual	RtVoid	RiMotionBeginV( RtInt N, RtFloat times[] );
		virtual	RtVoid	RiMotionEnd();
		virtual	RtVoid	RiNuPatchV( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtObjectHandle	RiObjectBegin();
		virtual	RtVoid	RiObjectEnd();
		virtual	RtVoid	RiObjectInstance( RtObjectHandle handle );
		virtual	RtVoid	RiOpacity( RtColor Os );
		virtual	RtVoid	RiOptionV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiOrientation( RtToken orientation );
		virtual	RtVoid	RiParaboloidV( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiPatchMeshV( RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiPatchV( RtToken type, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiPerspective( RtFloat fov );
		virtual	RtVoid	RiPixelFilter( RtFilterFunc function, RtFloat xwidth, RtFloat ywidth );
		virtual	RtVoid	RiPixelSamples( RtFloat xsamples, RtFloat ysamples );
		virtual	RtVoid	RiPixelVariance( RtFloat variance );
		virtual	RtVoid	RiCurvesV( RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap, RtInt n, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiPointsV( RtInt vertices, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiPointsGeneralPolygonsV( RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiPointsPolygonsV( RtInt npolys, RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiPolygonV( RtInt nvertices, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual RtVoid	RiBlobbyV( RtInt nleaf, RtInt ncode, RtInt code[], RtInt nflt, RtFloat flt[], RtInt nstr, RtToken str[], RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiProcedural( RtPointer data, RtBound bound, RtFunc refineproc, RtFunc freeproc );
		virtual	RtVoid	RiProjectionV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiQuantize( RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude );
		virtual	RtVoid	RiReadArchive( RtToken data, RtArchiveCallback callback );
		virtual	RtVoid	RiRelativeDetail( RtFloat relativedetail );
		virtual	RtVoid	RiReverseOrientation();
		virtual	RtVoid	RiRotate( RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz );
		virtual	RtVoid	RiScale( RtFloat sx, RtFloat sy, RtFloat sz );
		virtual	RtVoid	RiScreenWindow( RtFloat left, RtFloat right, RtFloat bottom, RtFloat top );
		virtual	RtVoid	RiShadingInterpolation( RtToken type );
		virtual	RtVoid	RiShadingRate( RtFloat size );
		virtual	RtVoid	RiShutter( RtFloat opentime, RtFloat closetime );
		virtual	RtVoid	RiSides( RtInt nsides );
		virtual	RtFloat RiSincFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
		virtual	RtVoid	RiSkew( RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1, RtFloat dx2, RtFloat dy2, RtFloat dz2 );
		virtual	RtVoid	RiSolidBegin( RtToken type );
		virtual	RtVoid	RiSolidEnd();
		virtual	RtVoid	RiSphereV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiSubdivisionMeshV( RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiSurfaceV( RtToken name, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiTextureCoordinates( RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4 );
		virtual	RtVoid	RiTorusV( RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual	RtVoid	RiTransform( RtMatrix transform );
		virtual	RtVoid	RiTransformBegin();
		virtual	RtVoid	RiTransformEnd();
		virtual	RtPoint* RiTransformPoints( RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[] );
		virtual	RtVoid	RiTranslate( RtFloat dx, RtFloat dy, RtFloat dz );
		virtual	RtFloat RiTriangleFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
		virtual	RtVoid	RiTrimCurve( RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[] );
		virtual	RtVoid	RiWorldBegin();
		virtual	RtVoid	RiWorldEnd();
		virtual RtVoid	RiShaderLayerV( RtToken type, RtToken name, RtToken layername, RtInt count, RtToken tokens[], RtPointer values[] );
		virtual RtVoid	RiConnectShaderLayers( RtToken type, RtToken layer1, RtToken variable1, RtToken layer2, RtToken variable2 );

		virtual RtFilterFunc	GetFilterFunction( RtToken type );
		virtual RtBasis*	GetBasisMatrix( RtToken type );
		virtual	RtFunc	GetProceduralFunction( RtToken type );

	private:
		std::ostream& m_Stream;
		unsigned int m_CurrentLightHandle;
};
extern "C"
{
	librib::RendermanInterface* CreateRIBEngine();
	void DestroyRIBEngine( librib::RendermanInterface* );
}

}
; // namespace librib2stream

#endif // LIBRIB2STREAM
