#ifndef LIBRIB_H
#define LIBRIB_H

#include <iostream>
#include <string>
#include <stdio.h>

#include "ri.h"

#define		_qShareName	BUILD_LIBRIB
#include	"share.h"

namespace librib
{

// Forward class declarations
class RendermanInterface;

/// Initializes the parser and callback object with a set of standard declarations
void StandardDeclarations( RendermanInterface& CallbackInterface );
/// Parses an input stream, using the supplied callback object and sending error data to the supplied output stream
bool Parse( FILE *InputStream, const std::string StreamName, RendermanInterface& CallbackInterface, std::ostream& ErrorStream );
/// Resets the state of the parser, clearing any symbol tables, etc.
void ResetParser();
/// Setup the defaut setting for the archive searchpath, automatically updated when an appropriate RiOption is seen.
void UpdateArchivePath( std::string strPath );

/// Provides an abstract base class for objects that implement the Renderman Interface
class RendermanInterface
{
	public:
		// The standard set of Renderman Interface types
		typedef bool RtBoolean;
		typedef int RtInt;
		typedef float RtFloat;
		typedef char* RtToken;
		typedef RtFloat RtColor[ 3 ];
		typedef RtFloat RtPoint[ 3 ];
		typedef RtFloat RtMatrix[ 4 ][ 4 ];
		typedef RtFloat RtBasis[ 4 ][ 4 ];
		typedef RtFloat RtBound[ 6 ];
		typedef char* RtString;
		typedef void* RtPointer;
		typedef void RtVoid;
		typedef RtFloat ( *RtFilterFunc ) ( RtFloat, RtFloat, RtFloat, RtFloat );
		typedef RtFloat ( *RtFloatFunc ) ();
		typedef RtVoid ( *RtFunc ) ();
		typedef RtVoid ( *RtErrorFunc ) ( RtInt code, RtInt severity, const RtToken message );
		typedef RtPointer RtObjectHandle;
		typedef RtPointer RtLightHandle;

		virtual	RtLightHandle RiAreaLightSourceV( const char *name, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiAtmosphereV( const char *name, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiAttributeBegin() = 0;
		virtual	RtVoid	RiAttributeEnd() = 0;
		virtual	RtVoid	RiAttributeV( const char *name, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiBasis( RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep ) = 0;
		virtual	RtVoid	RiBegin( RtToken name ) = 0;
		virtual	RtFloat RiBesselFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth ) = 0;
		virtual RtVoid RiBlobbyV( RtInt nleaf, RtInt ncode, RtInt code[],
		                          RtInt nflt, RtFloat flt[],
		                          RtInt nstr, RtToken str[],
		                          RtInt n, RtToken tokens[], RtPointer parms[] ) = 0;
		virtual	RtVoid	RiBound( RtBound bound ) = 0;
		virtual	RtFloat RiBoxFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth ) = 0;
		virtual	RtFloat RiCatmullRomFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth ) = 0;
		virtual	RtVoid	RiClipping( RtFloat cnear, RtFloat cfar ) = 0;
		virtual	RtVoid	RiColor( RtColor Cq ) = 0;
		virtual	RtVoid	RiColorSamples( RtInt N, RtFloat *nRGB, RtFloat *RGBn ) = 0;
		virtual	RtVoid	RiConcatTransform( RtMatrix transform ) = 0;
		virtual	RtVoid	RiConeV( RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiCoordinateSystem( RtToken space ) = 0;
		virtual	RtVoid	RiCoordSysTransform( RtToken space ) = 0;
		virtual	RtVoid	RiCropWindow( RtFloat left, RtFloat right, RtFloat top, RtFloat bottom ) = 0;
		virtual	RtVoid	RiCylinderV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtToken	RiDeclare( const char *name, const char *declaration ) = 0;
		virtual	RtVoid	RiDeformationV( const char *name, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiDepthOfField( RtFloat fstop, RtFloat focallength, RtFloat focaldistance ) = 0;
		virtual	RtVoid	RiDetail( RtBound bound ) = 0;
		virtual	RtVoid	RiDetailRange( RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh ) = 0;
		virtual	RtFloat RiDiskFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth ) = 0;
		virtual	RtVoid	RiDiskV( RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiDisplacementV( const char *name, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiDisplayV( const char *name, RtToken type, RtToken mode, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiEnd() = 0;
		virtual	RtVoid	RiErrorAbort( RtInt code, RtInt severity, const char* message ) = 0;
		//	virtual	RtVoid	RiErrorHandler(RtErrorFunc handler) = 0;
		virtual	RtVoid	RiErrorIgnore( RtInt code, RtInt severity, const char* message ) = 0;
		virtual	RtVoid	RiErrorPrint( RtInt code, RtInt severity, const char* message ) = 0;
		virtual	RtVoid	RiExposure( RtFloat gain, RtFloat gamma ) = 0;
		virtual	RtVoid	RiExteriorV( const char *name, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiFormat( RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio ) = 0;
		virtual	RtVoid	RiFrameAspectRatio( RtFloat frameratio ) = 0;
		virtual	RtVoid	RiFrameBegin( RtInt number ) = 0;
		virtual	RtVoid	RiFrameEnd() = 0;
		virtual	RtFloat RiGaussianFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth ) = 0;
		virtual	RtVoid	RiGeneralPolygonV( RtInt nloops, RtInt nverts[], RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiGeometricApproximation( RtToken type, RtFloat value ) = 0;
		virtual	RtVoid	RiGeometryV( RtToken type, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiHiderV( RtToken type, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiHyperboloidV( RtPoint point1, RtPoint point2, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiIdentity() = 0;
		virtual	RtVoid	RiIlluminate( RtLightHandle light, RtBoolean onoff ) = 0;
		virtual	RtVoid	RiImagerV( const char *name, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiInteriorV( const char *name, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtLightHandle RiLightSourceV( const char *name, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiMakeBumpV( const char *imagefile, const char *bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiMakeCubeFaceEnvironmentV( const char *px, const char *nx, const char *py, const char *ny, const char *pz, const char *nz, const char *reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiMakeLatLongEnvironmentV( const char *imagefile, const char *reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiMakeShadowV( const char *picfile, const char *shadowfile, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiMakeTextureV( const char *imagefile, const char *texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiMatte( RtBoolean onoff ) = 0;
		virtual	RtVoid	RiMotionBeginV( RtInt N, RtFloat times[] ) = 0;
		virtual	RtVoid	RiMotionEnd() = 0;
		virtual	RtVoid	RiNuPatchV( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtObjectHandle	RiObjectBegin() = 0;
		virtual	RtVoid	RiObjectEnd() = 0;
		virtual	RtVoid	RiObjectInstance( RtObjectHandle handle ) = 0;
		virtual	RtVoid	RiOpacity( RtColor Os ) = 0;
		virtual	RtVoid	RiOptionV( const char *name, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiOrientation( RtToken orientation ) = 0;
		virtual	RtVoid	RiParaboloidV( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiPatchMeshV( RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiPatchV( RtToken type, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiPerspective( RtFloat fov ) = 0;
		virtual	RtVoid	RiPixelFilter( RtFilterFunc function, RtFloat xwidth, RtFloat ywidth ) = 0;
		virtual	RtVoid	RiPixelSamples( RtFloat xsamples, RtFloat ysamples ) = 0;
		virtual	RtVoid	RiPixelVariance( RtFloat variance ) = 0;
		virtual	RtVoid	RiCurvesV( RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap,
		                          RtInt n, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiPointsV( RtInt vertices, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiPointsGeneralPolygonsV( RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiPointsPolygonsV( RtInt npolys, RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiPolygonV( RtInt nvertices, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiProcedural( RtPointer data, RtBound bound, RtFunc refineproc, RtFunc freeproc ) = 0;
		virtual	RtVoid	RiProjectionV( const char * name, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiQuantize( RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude ) = 0;
		virtual	RtVoid	RiRelativeDetail( RtFloat relativedetail ) = 0;
		virtual	RtVoid	RiReverseOrientation() = 0;
		virtual	RtVoid	RiRotate( RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz ) = 0;
		virtual	RtVoid	RiScale( RtFloat sx, RtFloat sy, RtFloat sz ) = 0;
		virtual	RtVoid	RiScreenWindow( RtFloat left, RtFloat right, RtFloat bottom, RtFloat top ) = 0;
		virtual	RtVoid	RiShadingInterpolation( RtToken type ) = 0;
		virtual	RtVoid	RiShadingRate( RtFloat size ) = 0;
		virtual	RtVoid	RiShutter( RtFloat opentime, RtFloat closetime ) = 0;
		virtual	RtVoid	RiSides( RtInt nsides ) = 0;
		virtual	RtFloat RiSincFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth ) = 0;
		virtual	RtVoid	RiSkew( RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1, RtFloat dx2, RtFloat dy2, RtFloat dz2 ) = 0;
		virtual	RtVoid	RiSolidBegin( RtToken type ) = 0;
		virtual	RtVoid	RiSolidEnd() = 0;
		virtual	RtVoid	RiSphereV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiSubdivisionMeshV( RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiSurfaceV( const char *name, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiTextureCoordinates( RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4 ) = 0;
		virtual	RtVoid	RiTorusV( RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[] ) = 0;
		virtual	RtVoid	RiTransform( RtMatrix transform ) = 0;
		virtual	RtVoid	RiTransformBegin() = 0;
		virtual	RtVoid	RiTransformEnd() = 0;
		virtual	RtPoint* RiTransformPoints( RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[] ) = 0;
		virtual	RtVoid	RiTranslate( RtFloat dx, RtFloat dy, RtFloat dz ) = 0;
		virtual	RtFloat RiTriangleFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth ) = 0;
		virtual	RtVoid	RiTrimCurve( RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[] ) = 0;
		virtual	RtVoid	RiWorldBegin() = 0;
		virtual	RtVoid	RiWorldEnd() = 0;

};

}
; // namespace librib


//typedef	librib::RendermanInterface::RtFloat RtFloat;
//extern "C" _qShareM RtFloat RiBesselFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
//extern "C" _qShareM RtFloat RiBoxFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
//extern "C" _qShareM RtFloat RiCatmullRomFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
//extern "C" _qShareM RtFloat RiDiskFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
//extern "C" _qShareM RtFloat RiGaussianFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
//extern "C" _qShareM RtFloat RiSincFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
//extern "C" _qShareM RtFloat RiTriangleFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );

//typedef	librib::RendermanInterface::RtVoid RtVoid;
//typedef	librib::RendermanInterface::RtPointer RtPointer;
//extern "C" _qShareM RtVoid	RiProcDelayedReadArchive( RtPointer data, RtFloat detail );
//extern "C" _qShareM RtVoid	RiProcRunProgram( RtPointer data, RtFloat detail );
//extern "C" _qShareM RtVoid	RiProcDynamicLoad( RtPointer data, RtFloat detail );

//typedef	librib::RendermanInterface::RtBasis RtBasis;
//extern "C" _qShareM RtBasis RiBezierBasis;
//extern "C" _qShareM RtBasis RiBSplineBasis;
//extern "C" _qShareM RtBasis RiCatmullRomBasis;
//extern "C" _qShareM RtBasis RiHermiteBasis;
//extern "C" _qShareM RtBasis RiPowerBasis;

#endif // LIBRIB_H
