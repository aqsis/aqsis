/*___________________________________________________________________________
**
** Based on Renderman Interface version 3.2
**
** Renderman Interface is Copyright (c) 1988 Pixar. All Rights reserved.
**
**___________________________________________________________________________
*/

//? Is ri.h included already?
#ifndef	RI_H_INCLUDED 
//{
#define	RI_H_INCLUDED 1

#include	"float.h"

#define		_qShareName	CORE
#include	"share.h"

#ifdef	__cplusplus
extern	"C"
{
#endif


	/*
	 *		RenderMan Interface Standard Include File
	 */

	/* Definitions of Abstract Types Used in RI */

	typedef	short	RtBoolean;
	typedef	int	RtInt;
	typedef	float	RtFloat;

	typedef	char	*RtToken;

	typedef	RtFloat	RtColor[ 3 ];
	typedef	RtFloat	RtPoint[ 3 ];
	typedef	RtFloat	RtMatrix[ 4 ][ 4 ];
	typedef	RtFloat	RtBasis[ 4 ][ 4 ];
	typedef	RtFloat	RtBound[ 6 ];
	typedef	char	*RtString;

	typedef	void	*RtPointer;
	typedef	void	RtVoid;

	typedef	RtFloat	( *RtFilterFunc ) ( RtFloat, RtFloat, RtFloat, RtFloat );
	typedef	RtFloat	( *RtFloatFunc ) ();
	typedef	RtVoid	( *RtFunc ) ();
	typedef	RtVoid	( *RtErrorFunc ) ( RtInt code, RtInt severity, const char * message );
	typedef	RtErrorFunc	RtErrorHandler;

	typedef	RtVoid	( *RtProcSubdivFunc ) ( RtPointer, RtFloat );
	typedef	RtVoid	( *RtProcFreeFunc ) ( RtPointer );
	typedef	RtVoid	( *RtArchiveCallback ) ( RtToken, char *, ... );

	typedef	RtPointer	RtObjectHandle;
	typedef	RtPointer	RtLightHandle;
	typedef	RtPointer	RtContextHandle;

	/* Extern Declarations for Predefined RI Data Structures */

#define	RI_FALSE	0
#define	RI_TRUE		1
#define	RI_INFINITY	FLT_MAX
#define	RI_EPSILON	FLT_EPSILON
#define	RI_NULL		((RtToken)0)

#define	RI_FLOATMIN	FLT_MIN
#define	RI_FLOATMAX	FLT_MAX

#define	RI_PI		3.14159265359f
#define	RI_PIO2		RI_PI/2

#define	RI_SHADER_EXTENSION	".slx"

	extern	_qShareM RtToken	RI_FRAMEBUFFER, RI_FILE;
	extern	_qShareM RtToken	RI_RGB, RI_RGBA, RI_RGBZ, RI_RGBAZ, RI_A, RI_Z, RI_AZ;
	extern	_qShareM RtToken	RI_MERGE, RI_ORIGIN;
	extern	_qShareM RtToken	RI_PERSPECTIVE, RI_ORTHOGRAPHIC;
	extern	_qShareM RtToken	RI_HIDDEN, RI_PAINT;
	extern	_qShareM RtToken	RI_CONSTANT, RI_SMOOTH;
	extern	_qShareM RtToken	RI_FLATNESS, RI_FOV;

	extern	_qShareM RtToken	RI_AMBIENTLIGHT, RI_POINTLIGHT,
		RI_DISTANTLIGHT, RI_SPOTLIGHT;
	extern	_qShareM RtToken	RI_INTENSITY, RI_LIGHTCOLOR, RI_FROM, RI_TO,
		RI_CONEANGLE, RI_CONEDELTAANGLE,
		RI_BEAMDISTRIBUTION;
	extern	_qShareM RtToken	RI_MATTE, RI_METAL, RI_PLASTIC, RI_SHINYMETAL, RI_PAINTEDPLASTIC;
	extern	_qShareM RtToken	RI_KA, RI_KD, RI_KS, RI_ROUGHNESS, RI_KR,
		RI_TEXTURENAME, RI_SPECULARCOLOR;
	extern	_qShareM RtToken	RI_DEPTHCUE, RI_FOG, RI_BUMPY;
	extern	_qShareM RtToken	RI_MINDISTANCE, RI_MAXDISTANCE, RI_BACKGROUND,
		RI_DISTANCE, RI_AMPLITUDE;

	extern	_qShareM RtToken	RI_RASTER, RI_SCREEN, RI_CAMERA, RI_WORLD,
		RI_OBJECT;
	extern	_qShareM RtToken	RI_INSIDE, RI_OUTSIDE, RI_LH, RI_RH;
	extern	_qShareM RtToken	RI_P, RI_PZ, RI_PW, RI_N, RI_NP, RI_CS, RI_OS,
		RI_S, RI_T, RI_ST;
	extern	_qShareM RtToken	RI_BILINEAR, RI_BICUBIC;
	extern	_qShareM RtToken	RI_LINEAR, RI_CUBIC;
	extern	_qShareM RtToken	RI_PRIMITIVE, RI_INTERSECTION, RI_UNION,
		RI_DIFFERENCE;
	extern	_qShareM RtToken	RI_WRAP, RI_NOWRAP, RI_PERIODIC, RI_NONPERIODIC, RI_CLAMP,
		RI_BLACK;
	extern	_qShareM RtToken	RI_IGNORE, RI_PRINT, RI_ABORT, RI_HANDLER;
	extern	_qShareM RtToken	RI_IDENTIFIER, RI_NAME;
	extern	_qShareM RtToken	RI_COMMENT, RI_STRUCTURE, RI_VERBATIM;
	extern	_qShareM RtToken	RI_WIDTH, RI_CONSTANTWIDTH;

	extern	_qShareM RtToken	RI_CURRENT, RI_SHADER, RI_EYE, RI_NDC;

	extern	_qShareM RtBasis	RiBezierBasis, RiBSplineBasis, RiCatmullRomBasis,
		RiHermiteBasis, RiPowerBasis;

#define	RI_BEZIERSTEP		((RtInt)3)
#define	RI_BSPLINESTEP		((RtInt)1)
#define	RI_CATMULLROMSTEP	((RtInt)1)
#define	RI_HERMITESTEP		((RtInt)2)
#define	RI_POWERSTEP		((RtInt)4)

	_qShareM extern	RtInt	RiLastError;

	/* Declarations of All of the RenderMan Interface Subroutines */

#define	PARAMETERLIST	RtInt count, RtToken tokens[], RtPointer values[]

	_qShareM RtToken	RiDeclare( const char * name, const char * declaration );

	_qShareM RtVoid		RiBegin( RtToken name );
	_qShareM RtVoid		RiEnd();
	_qShareM RtVoid		RiFrameBegin( RtInt number );
	_qShareM RtVoid		RiFrameEnd();
	_qShareM RtVoid		RiWorldBegin();
	_qShareM RtVoid		RiWorldEnd();

	_qShareM RtVoid	RiFormat( RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio );
	_qShareM RtVoid	RiFrameAspectRatio( RtFloat frameratio );
	_qShareM RtVoid	RiScreenWindow( RtFloat left, RtFloat right, RtFloat bottom, RtFloat top );
	_qShareM RtVoid	RiCropWindow( RtFloat left, RtFloat right, RtFloat top, RtFloat bottom );
	_qShareM RtVoid	RiProjection( const char * name, ... );
	_qShareM RtVoid	RiProjectionV( const char * name, PARAMETERLIST );
	_qShareM RtVoid	RiClipping( RtFloat cnear, RtFloat cfar );
	_qShareM RtVoid	RiDepthOfField( RtFloat fstop, RtFloat focallength, RtFloat focaldistance );
	_qShareM RtVoid	RiShutter( RtFloat opentime, RtFloat closetime );

	_qShareM RtVoid	RiPixelVariance( RtFloat variance );
	_qShareM RtVoid	RiPixelSamples( RtFloat xsamples, RtFloat ysamples );
	_qShareM RtVoid	RiPixelFilter( RtFilterFunc function, RtFloat xwidth, RtFloat ywidth );
	_qShareM RtVoid	RiExposure( RtFloat gain, RtFloat gamma );
	_qShareM RtVoid	RiImager( const char * name, ... );
	_qShareM RtVoid	RiImagerV( const char * name, PARAMETERLIST );
	_qShareM RtVoid	RiQuantize( RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude );
	_qShareM RtVoid	RiDisplay( const char * name, RtToken type, RtToken mode, ... );
	_qShareM RtVoid	RiDisplayV( const char * name, RtToken type, RtToken mode, PARAMETERLIST );
	_qShareM RtFloat	RiGaussianFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
	_qShareM RtFloat	RiBoxFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
	_qShareM RtFloat	RiTriangleFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
	_qShareM RtFloat	RiCatmullRomFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
	_qShareM RtFloat	RiSincFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
	_qShareM RtFloat	RiDiskFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );
	_qShareM RtFloat	RiBesselFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth );

	_qShareM RtVoid	RiHider( const char * name, ... );
	_qShareM RtVoid	RiHiderV( const char * name, PARAMETERLIST );
	_qShareM RtVoid	RiColorSamples( RtInt N, RtFloat * nRGB, RtFloat * RGBn );
	_qShareM RtVoid	RiRelativeDetail( RtFloat relativedetail );
	_qShareM RtVoid	RiOption( const char * name, ... );
	_qShareM RtVoid	RiOptionV( const char * name, PARAMETERLIST );

	_qShareM RtVoid	RiAttributeBegin();
	_qShareM RtVoid	RiAttributeEnd();
	_qShareM RtVoid	RiColor( RtColor Cq );
	_qShareM RtVoid	RiOpacity( RtColor Os );
	_qShareM RtVoid	RiTextureCoordinates( RtFloat s1, RtFloat t1,
	                             RtFloat s2, RtFloat t2,
	                             RtFloat s3, RtFloat t3,
	                             RtFloat s4, RtFloat t4 );

	_qShareM RtLightHandle	RiLightSource( const char * name, ... );
	_qShareM RtLightHandle	RiLightSourceV( const char * name, PARAMETERLIST );
	_qShareM RtLightHandle	RiAreaLightSource( const char * name, ... );
	_qShareM RtLightHandle	RiAreaLightSourceV( const char * name, PARAMETERLIST );
	_qShareM RtVoid	RiIlluminate( RtLightHandle light, RtBoolean onoff );
	_qShareM RtVoid	RiSurface( const char * name, ... );
	_qShareM RtVoid	RiSurfaceV( const char * name, PARAMETERLIST );
	_qShareM RtVoid	RiAtmosphere( const char * name, ... );
	_qShareM RtVoid	RiAtmosphereV( const char * name, PARAMETERLIST );
	_qShareM RtVoid	RiInterior( const char * name, ... );
	_qShareM RtVoid	RiInteriorV( const char * name, PARAMETERLIST );
	_qShareM RtVoid	RiExterior( const char * name, ... );
	_qShareM RtVoid	RiExteriorV( const char * name, PARAMETERLIST );
	_qShareM RtVoid	RiShadingRate( RtFloat size );
	_qShareM RtVoid	RiShadingInterpolation( RtToken type );
	_qShareM RtVoid	RiMatte( RtBoolean onoff );

	_qShareM RtVoid	RiBound( RtBound bound );
	_qShareM RtVoid	RiDetail( RtBound bound );
	_qShareM RtVoid	RiDetailRange( RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh );
	_qShareM RtVoid	RiGeometricApproximation( RtToken type, RtFloat value );
	_qShareM RtVoid	RiOrientation( RtToken orientation );
	_qShareM RtVoid	RiReverseOrientation();
	_qShareM RtVoid	RiSides( RtInt nsides );

	_qShareM RtVoid	RiIdentity();
	_qShareM RtVoid	RiTransform( RtMatrix transform );
	_qShareM RtVoid	RiConcatTransform( RtMatrix transform );
	_qShareM RtVoid	RiPerspective( RtFloat fov );
	_qShareM RtVoid	RiTranslate( RtFloat dx, RtFloat dy, RtFloat dz );
	_qShareM RtVoid	RiRotate( RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz );
	_qShareM RtVoid	RiScale( RtFloat sx, RtFloat sy, RtFloat sz );
	_qShareM RtVoid	RiSkew( RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
	               RtFloat dx2, RtFloat dy2, RtFloat dz2 );
	_qShareM RtVoid	RiDeformation( const char * name, ... );
	_qShareM RtVoid	RiDeformationV( const char * name, PARAMETERLIST );
	_qShareM RtVoid	RiDisplacement( const char * name, ... );
	_qShareM RtVoid	RiDisplacementV( const char * name, PARAMETERLIST );
	_qShareM RtVoid	RiCoordinateSystem( RtToken space );
	_qShareM RtPoint*	RiTransformPoints( RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[] );
	_qShareM RtVoid	RiTransformBegin();
	_qShareM RtVoid	RiTransformEnd();

	_qShareM RtVoid	RiAttribute( const char * name, ... );
	_qShareM RtVoid	RiAttributeV( const char * name, PARAMETERLIST );

	_qShareM RtVoid	RiPolygon( RtInt nvertices, ... );
	_qShareM RtVoid	RiPolygonV( RtInt nvertices, PARAMETERLIST );
	_qShareM RtVoid	RiGeneralPolygon( RtInt nloops, RtInt nverts[], ... );
	_qShareM RtVoid	RiGeneralPolygonV( RtInt nloops, RtInt nverts[], PARAMETERLIST );
	_qShareM RtVoid	RiPointsPolygons( RtInt npolys, RtInt nverts[], RtInt verts[], ... );
	_qShareM RtVoid	RiPointsPolygonsV( RtInt npolys, RtInt nverts[], RtInt verts[], PARAMETERLIST );
	_qShareM RtVoid	RiPointsGeneralPolygons( RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], ... );
	_qShareM RtVoid	RiPointsGeneralPolygonsV( RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], PARAMETERLIST );
	_qShareM RtVoid	RiBasis( RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep );
	_qShareM RtVoid	RiPatch( RtToken type, ... );
	_qShareM RtVoid	RiPatchV( RtToken type, PARAMETERLIST );
	_qShareM RtVoid	RiPatchMesh( RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, ... );
	_qShareM RtVoid	RiPatchMeshV( RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, PARAMETERLIST );
	_qShareM RtVoid	RiNuPatch( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax,
	                  RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, ... );
	_qShareM RtVoid	RiNuPatchV( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax,
	                   RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, PARAMETERLIST );
	_qShareM RtVoid	RiTrimCurve( RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[] );

	_qShareM RtVoid	RiSphere( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ... );
	_qShareM RtVoid	RiSphereV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST );
	_qShareM RtVoid	RiCone( RtFloat height, RtFloat radius, RtFloat thetamax, ... );
	_qShareM RtVoid	RiConeV( RtFloat height, RtFloat radius, RtFloat thetamax, PARAMETERLIST );
	_qShareM RtVoid	RiCylinder( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ... );
	_qShareM RtVoid	RiCylinderV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST );
	_qShareM RtVoid	RiHyperboloid( RtPoint point1, RtPoint point2, RtFloat thetamax, ... );
	_qShareM RtVoid	RiHyperboloidV( RtPoint point1, RtPoint point2, RtFloat thetamax, PARAMETERLIST );
	_qShareM RtVoid	RiParaboloid( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ... );
	_qShareM RtVoid	RiParaboloidV( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST );
	_qShareM RtVoid	RiDisk( RtFloat height, RtFloat radius, RtFloat thetamax, ... );
	_qShareM RtVoid	RiDiskV( RtFloat height, RtFloat radius, RtFloat thetamax, PARAMETERLIST );
	_qShareM RtVoid	RiTorus( RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, ... );
	_qShareM RtVoid	RiTorusV( RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, PARAMETERLIST );
	_qShareM RtVoid	RiProcedural( RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc );
	_qShareM RtVoid	RiGeometry( RtToken type, ... );
	_qShareM RtVoid	RiGeometryV( RtToken type, PARAMETERLIST );

	_qShareM RtVoid	RiSolidBegin( RtToken type );
	_qShareM RtVoid	RiSolidEnd();
	_qShareM RtObjectHandle	RiObjectBegin();
	_qShareM RtVoid	RiObjectEnd();
	_qShareM RtVoid	RiObjectInstance( RtObjectHandle handle );
	_qShareM RtVoid	RiMotionBegin( RtInt N, ... );
	_qShareM RtVoid	RiMotionBeginV( RtInt N, RtFloat times[] );
	_qShareM RtVoid	RiMotionEnd();

	_qShareM RtVoid	RiMakeTexture( const char * imagefile, const char * texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... );
	_qShareM RtVoid	RiMakeTextureV( const char * imagefile, const char * texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST );
	_qShareM RtVoid	RiMakeBump( const char * imagefile, const char * bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... );
	_qShareM RtVoid	RiMakeBumpV( const char * imagefile, const char * bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST );
	_qShareM RtVoid	RiMakeLatLongEnvironment( const char * imagefile, const char * reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... );
	_qShareM RtVoid	RiMakeLatLongEnvironmentV( const char * imagefile, const char * reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST );
	_qShareM RtVoid	RiMakeCubeFaceEnvironment( const char * px, const char * nx, const char * py, const char * ny, const char * pz, const char * nz, const char * reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... );
	_qShareM RtVoid	RiMakeCubeFaceEnvironmentV( const char * px, const char * nx, const char * py, const char * ny, const char * pz, const char * nz, const char * reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST );
	_qShareM RtVoid	RiMakeShadow( const char * picfile, const char * shadowfile, ... );
	_qShareM RtVoid	RiMakeShadowV( const char * picfile, const char * shadowfile, PARAMETERLIST );

	_qShareM RtVoid	RiErrorHandler( RtErrorFunc handler );
	_qShareM RtVoid	RiErrorIgnore( RtInt code, RtInt severity, const char * message );
	_qShareM RtVoid	RiErrorPrint( RtInt code, RtInt severity, const char * message );
	_qShareM RtVoid	RiErrorAbort( RtInt code, RtInt severity, const char * message );
	_qShareM RtVoid	RiArchiveRecord( RtToken type, char *, ... );

	// ---Additional to spec. v3.1---
	_qShareM RtContextHandle	RiGetContext( void );
	_qShareM RtVoid	RiContext( RtContextHandle );
	_qShareM RtVoid	RiClippingPlane( RtFloat, RtFloat, RtFloat, RtFloat, RtFloat, RtFloat );
	_qShareM RtVoid	RiCoordSysTransform( RtToken space );
	_qShareM RtVoid	RiBlobby( RtInt nleaf, RtInt ncode, RtInt code[], RtInt nflt, RtFloat flt[], RtInt nstr, RtToken str[], ... );
	_qShareM RtVoid	RiBlobbyV( RtInt nleaf, RtInt ncode, RtInt code[], RtInt nflt, RtFloat flt[], RtInt nstr, RtToken str[], PARAMETERLIST );
	_qShareM RtVoid	RiPoints( RtInt npoints, ... );
	_qShareM RtVoid	RiPointsV( RtInt npoints, PARAMETERLIST );
	_qShareM RtVoid	RiCurves( RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap, ... );
	_qShareM RtVoid	RiCurvesV( RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap, PARAMETERLIST );
	_qShareM RtVoid	RiSubdivisionMesh( RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], ... );
	_qShareM RtVoid	RiSubdivisionMeshV( RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], PARAMETERLIST );

	_qShareM RtVoid	RiReadArchive( RtToken name, RtArchiveCallback, ... );
	_qShareM RtVoid	RiReadArchiveV( RtToken name, RtArchiveCallback, PARAMETERLIST );

	_qShareM RtVoid	RiProcFree( RtPointer data );

	_qShareM RtVoid	RiProcDelayedReadArchive( RtPointer data, RtFloat detail );
	_qShareM RtVoid	RiProcRunProgram( RtPointer data, RtFloat detail );
	_qShareM RtVoid	RiProcDynamicLoad( RtPointer data, RtFloat detail );


	// Specific to Aqsis

	typedef	RtVoid	( *RtProgressFunc ) ( RtFloat PercentComplete );

	_qShareM RtBoolean	BasisFromName( RtBasis * b, const char * strName );
	_qShareM RtVoid	RiProgressHandler( RtProgressFunc handler );
	_qShareM RtFunc	RiPreRenderFunction( RtFunc function );

#ifdef	__cplusplus
}
#endif

/*
  Error Codes
  
   1 - 10         System and File Errors
  11 - 20         Program Limitations
  21 - 40         State Errors
  41 - 60         Parameter and Protocol Errors
  61 - 80         Execution Errors
*/
#define RIE_NOERROR     ((RtInt)0)

#define RIE_NOMEM       ((RtInt)1)      /* Out of memory */
#define RIE_SYSTEM      ((RtInt)2)      /* Miscellaneous system error */
#define RIE_NOFILE      ((RtInt)3)      /* File nonexistent */
#define RIE_BADFILE     ((RtInt)4)      /* Bad file format */
#define RIE_VERSION     ((RtInt)5)      /* File version mismatch */
#define RIE_DISKFULL    ((RtInt)6)      /* Target disk is full */

#define RIE_INCAPABLE   ((RtInt)11)     /* Optional RI feature */
#define RIE_UNIMPLEMENT ((RtInt)12)     /* Unimplemented feature */
#define RIE_LIMIT       ((RtInt)13)     /* Arbitrary program limit */
#define RIE_BUG         ((RtInt)14)     /* Probably a bug in renderer */

#define RIE_NOTSTARTED  ((RtInt)23)     /* RiBegin not called */
#define RIE_NESTING     ((RtInt)24)     /* Bad begin-end nesting */
#define RIE_NOTOPTIONS  ((RtInt)25)     /* Invalid state for options */
#define RIE_NOTATTRIBS  ((RtInt)26)     /* Invalid state for attribs */
#define RIE_NOTPRIMS    ((RtInt)27)     /* Invalid state for primitives */
#define RIE_ILLSTATE    ((RtInt)28)     /* Other invalid state */
#define RIE_BADMOTION   ((RtInt)29)     /* Badly formed motion block */
#define RIE_BADSOLID    ((RtInt)30)     /* Badly formed solid block */

#define RIE_BADTOKEN    ((RtInt)41)     /* Invalid token for request */
#define RIE_RANGE       ((RtInt)42)     /* Parameter out of range */
#define RIE_CONSISTENCY ((RtInt)43)     /* Parameters inconsistent */
#define RIE_BADHANDLE   ((RtInt)44)     /* Bad object/light handle */
#define RIE_NOSHADER    ((RtInt)45)     /* Can't load requested shader */
#define RIE_MISSINGDATA ((RtInt)46)     /* Required parameters not provided */
#define RIE_SYNTAX      ((RtInt)47)     /* Declare type syntax error */

#define RIE_MATH        ((RtInt)61)     /* Zerodivide, noninvert matrix, etc. */

/* Error severity levels */
#define RIE_INFO        ((RtInt)0)      /* Rendering stats and other info */
#define RIE_WARNING     ((RtInt)1)      /* Something seems wrong, maybe okay */
#define RIE_ERROR       ((RtInt)2)      /* Problem. Results may be wrong */
#define RIE_SEVERE      ((RtInt)3)      /* So bad you should probably abort */


//}  // End of #ifdef RI_H_INCLUDED
#endif

