/*___________________________________________________________________________
**
** Copyright (c) 1988 Pixar. All Rights reserved.
**
**___________________________________________________________________________
*/

//? Is ri.h included already?
#ifndef	RI_H_INCLUDED
//{
#define	RI_H_INCLUDED 1

#include	"float.h"
//#include	"specific.h"

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

typedef	bool		RtBoolean;
typedef	int			RtInt;
typedef	float		RtFloat;

typedef	char		*RtToken;

typedef	RtFloat		RtColor[3];
typedef	RtFloat		RtPoint[3];
typedef	RtFloat		RtMatrix[4][4];
typedef	RtFloat		RtBasis[4][4];
typedef	RtFloat		RtBound[6];
typedef	char		*RtString;

typedef	char		*RtPointer;
typedef	int			RtVoid;
typedef	RtFloat		(*RtFilterFunc)(RtFloat,RtFloat,RtFloat,RtFloat);
typedef	RtFloat		(*RtFloatFunc)();
typedef	RtVoid		(*RtFunc)();
typedef	RtVoid		(*RtErrorFunc)(RtInt code, RtInt severity, const char* message);

typedef	RtPointer	RtObjectHandle;
typedef	RtPointer	RtLightHandle;

		/* Extern Declarations for Predefined RI Data Structures */

#define	RI_FALSE	false
#define	RI_TRUE		true
#define	RI_INFINITY	FLT_MAX
#define	RI_EPSILON	FLT_EPSILON
#define	RI_NULL		((RtToken)0)

#define	RI_FLOATMIN	FLT_MIN
#define	RI_FLOATMAX	FLT_MAX

#define	RI_PI		3.14159265359f
#define	RI_PIO2		RI_PI/2

#define	RI_SHADER_EXTENSION	".slx"

_qShare	extern	RtToken		RI_FRAMEBUFFER, RI_FILE;
_qShare	extern	RtToken		RI_RGB, RI_RGBA, RI_RGBZ, RI_RGBAZ, RI_A, RI_Z, RI_AZ;
_qShare	extern	RtToken		RI_MERGE, RI_ORIGIN;
_qShare	extern	RtToken		RI_PERSPECTIVE, RI_ORTHOGRAPHIC;
_qShare	extern	RtToken		RI_HIDDEN, RI_PAINT;
_qShare	extern	RtToken		RI_CONSTANT, RI_SMOOTH;
_qShare	extern	RtToken		RI_FLATNESS, RI_FOV;

_qShare	extern	RtToken		RI_AMBIENTLIGHT, RI_POINTLIGHT,
								RI_DISTANTLIGHT, RI_SPOTLIGHT;
_qShare	extern	RtToken		RI_INTENSITY, RI_LIGHTCOLOR, RI_FROM, RI_TO,
								RI_CONEANGLE, RI_CONEDELTAANGLE,
								RI_BEAMDISTRIBUTION;
_qShare	extern	RtToken		RI_MATTE, RI_METAL, RI_PLASTIC;
_qShare	extern	RtToken		RI_KA, RI_KD, RI_KS, RI_ROUGHNESS,
								RI_SPECULARCOLOR;
_qShare	extern	RtToken		RI_DEPTHCUE, RI_FOG;
_qShare	extern	RtToken		RI_MINDISTANCE, RI_MAXDISTANCE,
								RI_BACKGROUND, RI_DISTANCE;

_qShare	extern	RtToken		RI_RASTER, RI_SCREEN, RI_CAMERA, RI_WORLD,
								RI_OBJECT;
_qShare	extern	RtToken		RI_INSIDE, RI_OUTSIDE, RI_LH, RI_RH;
_qShare	extern	RtToken		RI_P, RI_PZ, RI_PW, RI_N, RI_NP, RI_CS, RI_OS,
								RI_S, RI_T, RI_ST;
_qShare	extern	RtToken		RI_BILINEAR, RI_BICUBIC;
_qShare	extern	RtToken		RI_PRIMITIVE, RI_INTERSECTION, RI_UNION,
								RI_DIFFERENCE;
_qShare	extern	RtToken		RI_WRAP, RI_NOWRAP, RI_PERIODIC, RI_NONPERIODIC, RI_CLAMP,
								RI_BLACK;
_qShare	extern	RtToken		RI_IGNORE, RI_PRINT, RI_ABORT, RI_HANDLER;

_qShare	extern	RtBasis		RiBezierBasis, RiBSplineBasis, RiCatmullRomBasis,
								RiHermiteBasis, RiPowerBasis;

#define	RI_BEZIERSTEP		((RtInt)3)
#define	RI_BSPLINESTEP		((RtInt)1)
#define	RI_CATMULLROMSTEP	((RtInt)1)
#define	RI_HERMITESTEP		((RtInt)2)
#define	RI_POWERSTEP		((RtInt)4)

_qShare	extern	RtInt		RiLastError;

		/* Declarations of All of the RenderMan Interface Subroutines */

#define	PARAMETERLIST	RtInt count, RtToken tokens[], RtPointer values[]

_qShare	RtToken			RiDeclare(const char *name, const char *declaration);

_qShare	RtVoid 			RiBegin(RtToken name);
_qShare	RtVoid			RiEnd();
_qShare	RtVoid			RiFrameBegin(RtInt number);
_qShare	RtVoid			RiFrameEnd();
_qShare	RtVoid			RiWorldBegin();
_qShare	RtVoid			RiWorldEnd();

_qShare	RtVoid			RiFormat(RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio);
_qShare	RtVoid			RiFrameAspectRatio(RtFloat frameratio);
_qShare	RtVoid			RiScreenWindow(RtFloat left, RtFloat right, RtFloat bottom, RtFloat top);
_qShare	RtVoid			RiCropWindow(RtFloat left, RtFloat right, RtFloat top, RtFloat bottom);
_qShare	RtVoid			RiProjection(const char *name, ...);
_qShare	RtVoid			RiProjectionV(const char * name, PARAMETERLIST);
_qShare	RtVoid			RiClipping(RtFloat cnear, RtFloat cfar);
_qShare	RtVoid			RiDepthOfField(RtFloat fstop, RtFloat focallength, RtFloat focaldistance);
_qShare	RtVoid			RiShutter(RtFloat opentime, RtFloat closetime);

_qShare	RtVoid			RiPixelVariance(RtFloat variance);
_qShare	RtVoid			RiPixelSamples(RtFloat xsamples, RtFloat ysamples);
_qShare	RtVoid			RiPixelFilter(RtFilterFunc function, RtFloat xwidth, RtFloat ywidth);
_qShare	RtVoid			RiExposure(RtFloat gain, RtFloat gamma);
_qShare	RtVoid			RiImager(const char *name, ...);
_qShare	RtVoid			RiImagerV(const char *name, PARAMETERLIST);
_qShare	RtVoid			RiQuantize(RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude);
_qShare	RtVoid			RiDisplay(const char *name, RtToken type, RtToken mode, ...);
_qShare	RtVoid			RiDisplayV(const char *name, RtToken type, RtToken mode, PARAMETERLIST);
_qShare	RtFloat			RiGaussianFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth);
_qShare	RtFloat			RiBoxFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth);
_qShare	RtFloat			RiTriangleFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth);
_qShare	RtFloat			RiCatmullRomFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth);
_qShare	RtFloat			RiSincFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth);

_qShare	RtVoid			RiHider(RtToken type, ...);
_qShare	RtVoid			RiHiderV(RtToken type, PARAMETERLIST);
_qShare	RtVoid			RiColorSamples(RtInt N, RtFloat *nRGB, RtFloat *RGBn);
_qShare	RtVoid			RiRelativeDetail(RtFloat relativedetail);
_qShare	RtVoid			RiOption(const char *name, ...);
_qShare	RtVoid			RiOptionV(const char *name, PARAMETERLIST);

_qShare	RtVoid			RiAttributeBegin();
_qShare	RtVoid			RiAttributeEnd();
_qShare	RtVoid			RiColor(RtColor Cq);
_qShare	RtVoid			RiOpacity(RtColor Os);
_qShare	RtVoid			RiTextureCoordinates(RtFloat s1, RtFloat t1, 
												 RtFloat s2, RtFloat t2, 
												 RtFloat s3, RtFloat t3, 
												 RtFloat s4, RtFloat t4);

_qShare	RtLightHandle	RiLightSource(const char *name, ...);
_qShare	RtLightHandle	RiLightSourceV(const char *name, PARAMETERLIST);
_qShare	RtLightHandle	RiAreaLightSource(const char *name, ...);
_qShare	RtLightHandle	RiAreaLightSourceV(const char *name, PARAMETERLIST);
_qShare	RtVoid			RiIlluminate(RtLightHandle light, RtBoolean onoff);
_qShare	RtVoid			RiSurface(const char *name, ...);
_qShare	RtVoid			RiSurfaceV(const char *name, PARAMETERLIST);
_qShare	RtVoid			RiAtmosphere(const char *name, ...);
_qShare	RtVoid			RiAtmosphereV(const char *name, PARAMETERLIST);
_qShare	RtVoid			RiInterior(const char *name, ...);
_qShare	RtVoid			RiInteriorV(const char *name, PARAMETERLIST);
_qShare	RtVoid			RiExterior(const char *name, ...);
_qShare	RtVoid			RiExteriorV(const char *name, PARAMETERLIST);
_qShare	RtVoid			RiShadingRate(RtFloat size);
_qShare	RtVoid			RiShadingInterpolation(RtToken type);
_qShare	RtVoid			RiMatte(RtBoolean onoff);

_qShare	RtVoid			RiBound(RtBound bound);
_qShare	RtVoid			RiDetail(RtBound bound);
_qShare	RtVoid			RiDetailRange(RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh);
_qShare	RtVoid			RiGeometricApproximation(RtToken type, RtFloat value);
_qShare	RtVoid			RiOrientation(RtToken orientation);
_qShare	RtVoid			RiReverseOrientation();
_qShare	RtVoid			RiSides(RtInt nsides);

_qShare	RtVoid			RiIdentity();
_qShare	RtVoid			RiTransform(RtMatrix transform);
_qShare	RtVoid			RiConcatTransform(RtMatrix transform);
_qShare	RtVoid			RiPerspective(RtFloat fov);
_qShare	RtVoid			RiTranslate(RtFloat dx, RtFloat dy, RtFloat dz);
_qShare	RtVoid			RiRotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz);
_qShare	RtVoid			RiScale(RtFloat sx, RtFloat sy, RtFloat sz);
_qShare	RtVoid			RiSkew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
								   RtFloat dx2, RtFloat dy2, RtFloat dz2);
_qShare	RtVoid			RiDeformation(const char *name, ...);
_qShare	RtVoid			RiDeformationV(const char *name, PARAMETERLIST);
_qShare	RtVoid			RiDisplacement(const char *name, ...);
_qShare	RtVoid			RiDisplacementV(const char *name, PARAMETERLIST);
_qShare	RtVoid			RiCoordinateSystem(RtToken space);
_qShare	RtPoint*		RiTransformPoints(RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[]);
_qShare	RtVoid			RiTransformBegin();
_qShare	RtVoid			RiTransformEnd();

_qShare	RtVoid			RiAttribute(const char *name, ...);
_qShare	RtVoid			RiAttributeV(const char *name, PARAMETERLIST);

_qShare	RtVoid			RiPolygon(RtInt nvertices, ...);
_qShare	RtVoid			RiPolygonV(RtInt nvertices, PARAMETERLIST);
_qShare	RtVoid			RiGeneralPolygon(RtInt nloops, RtInt nverts[], ...);
_qShare	RtVoid			RiGeneralPolygonV(RtInt nloops, RtInt nverts[], PARAMETERLIST);
_qShare	RtVoid			RiPointsPolygons(RtInt npolys, RtInt nverts[], RtInt verts[], ...);
_qShare	RtVoid			RiPointsPolygonsV(RtInt npolys, RtInt nverts[], RtInt verts[], PARAMETERLIST);
_qShare	RtVoid			RiPointsGeneralPolygons(RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], ...);
_qShare	RtVoid			RiPointsGeneralPolygonsV(RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], PARAMETERLIST);
_qShare	RtVoid			RiBasis(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep);
_qShare	RtVoid			RiPatch(RtToken type, ...);
_qShare	RtVoid			RiPatchV(RtToken type, PARAMETERLIST);
_qShare	RtVoid			RiPatchMesh(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, ...);
_qShare	RtVoid			RiPatchMeshV(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, PARAMETERLIST);
_qShare	RtVoid			RiNuPatch(RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, 
									  RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, ...);
_qShare	RtVoid			RiNuPatchV(RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, 
									   RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, PARAMETERLIST);
_qShare	RtVoid			RiTrimCurve(RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[]);

_qShare	RtVoid			RiSphere(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ...);
_qShare	RtVoid			RiSphereV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST);
_qShare	RtVoid			RiCone(RtFloat height, RtFloat radius, RtFloat thetamax, ...);
_qShare	RtVoid			RiConeV(RtFloat height, RtFloat radius, RtFloat thetamax, PARAMETERLIST);
_qShare	RtVoid			RiCylinder(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ...);
_qShare	RtVoid			RiCylinderV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST);
_qShare	RtVoid			RiHyperboloid(RtPoint point1, RtPoint point2, RtFloat thetamax, ...);
_qShare	RtVoid			RiHyperboloidV(RtPoint point1, RtPoint point2, RtFloat thetamax,PARAMETERLIST);
_qShare	RtVoid			RiParaboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ...);
_qShare	RtVoid			RiParaboloidV(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST);
_qShare	RtVoid			RiDisk(RtFloat height, RtFloat radius, RtFloat thetamax, ...);
_qShare	RtVoid			RiDiskV(RtFloat height, RtFloat radius, RtFloat thetamax, PARAMETERLIST);
_qShare	RtVoid			RiTorus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, ...);
_qShare	RtVoid			RiTorusV(RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, PARAMETERLIST);
_qShare	RtVoid			RiProcedural(RtPointer data, RtBound bound, RtFunc refineproc, RtFunc freeproc);
//_qShare	RtVoid			RiGeometry();
//_qShare	RtVoid			RiGeometryV();

_qShare	RtVoid			RiSolidBegin(RtToken type);
_qShare	RtVoid			RiSolidEnd();
_qShare	RtObjectHandle	RiObjectBegin();
_qShare	RtVoid			RiObjectEnd();
_qShare	RtVoid			RiObjectInstance(RtObjectHandle handle);
_qShare	RtVoid			RiMotionBegin(RtInt N, ...);
_qShare	RtVoid			RiMotionBeginV(RtInt N, RtFloat times[]);
_qShare	RtVoid			RiMotionEnd();

_qShare	RtVoid			RiMakeTexture(const char *imagefile, const char *texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ...);
_qShare	RtVoid			RiMakeTextureV(const char *imagefile, const char *texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST);
_qShare	RtVoid			RiMakeBump(const char *imagefile, const char *bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ...);
_qShare	RtVoid			RiMakeBumpV(const char *imagefile, const char *bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST);
_qShare	RtVoid			RiMakeLatLongEnvironment(const char *imagefile, const char *reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ...);
_qShare	RtVoid			RiMakeLatLongEnvironmentV(const char *imagefile, const char *reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST);
_qShare	RtVoid			RiMakeCubeFaceEnvironment(const char *px, const char *nx, const char *py, const char *ny, const char *pz, const char *nz, const char *reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... );
_qShare	RtVoid			RiMakeCubeFaceEnvironmentV(const char *px, const char *nx, const char *py, const char *ny, const char *pz, const char *nz, const char *reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST);
_qShare	RtVoid			RiMakeShadow(const char *picfile, const char *shadowfile, ...);
_qShare	RtVoid			RiMakeShadowV(const char *picfile, const char *shadowfile, PARAMETERLIST);

_qShare	RtVoid			RiErrorHandler(RtErrorFunc handler);
_qShare	RtVoid			RiErrorIgnore(RtInt code, RtInt severity, const char* message);
_qShare	RtVoid			RiErrorPrint(RtInt code, RtInt severity, const char* message);
_qShare	RtVoid			RiErrorAbort(RtInt code, RtInt severity, const char* message);

// ---Additional to spec. v3.1---
_qShare	RtVoid			RiCoordSysTransform(RtToken space);
_qShare	RtVoid			RiSubdivisionMesh(RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], ...);
_qShare	RtVoid			RiSubdivisionMeshV(RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], PARAMETERLIST);


_qShare	RtBoolean		BasisFromName(RtBasis& b, const char* strName);

#ifdef	__cplusplus
}
#endif


//}  // End of #ifdef RI_H_INCLUDED
#endif
