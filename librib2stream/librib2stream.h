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
	Stream(std::ostream& Stream);
	virtual ~Stream();

	virtual	RtLightHandle RiAreaLightSourceV(const char *name, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiAtmosphereV(const char *name, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiAttributeBegin();
	virtual	RtVoid	RiAttributeEnd();
	virtual	RtVoid	RiAttributeV(const char *name, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiBasis(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep);
	virtual	RtVoid	RiBegin(RtToken name);
	virtual	RtVoid	RiBound(RtBound bound);
	virtual	RtFloat RiBoxFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth);
	virtual	RtFloat RiCatmullRomFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth);
	virtual	RtVoid	RiClipping(RtFloat cnear, RtFloat cfar);
	virtual	RtVoid	RiColor(RtColor Cq);
	virtual	RtVoid	RiColorSamples(RtInt N, RtFloat *nRGB, RtFloat *RGBn);
	virtual	RtVoid	RiConcatTransform(RtMatrix transform);
	virtual	RtVoid	RiConeV(RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiCoordinateSystem(RtToken space);
	virtual	RtVoid	RiCoordSysTransform(RtToken space);
	virtual	RtVoid	RiCropWindow(RtFloat left, RtFloat right, RtFloat top, RtFloat bottom);
	virtual	RtVoid	RiCylinderV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtToken	RiDeclare(const char *name, const char *declaration);
	virtual	RtVoid	RiDeformationV(const char *name, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiDepthOfField(RtFloat fstop, RtFloat focallength, RtFloat focaldistance);
	virtual	RtVoid	RiDetail(RtBound bound);
	virtual	RtVoid	RiDetailRange(RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh);
	virtual	RtVoid	RiDiskV(RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiDisplacementV(const char *name, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiDisplayV(const char *name, RtToken type, RtToken mode, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiEnd();
	virtual	RtVoid	RiErrorAbort(RtInt code, RtInt severity, const char* message);
	//virtual	RtVoid	RiErrorHandler(RtErrorFunc handler);
	virtual	RtVoid	RiErrorIgnore(RtInt code, RtInt severity, const char* message);
	virtual	RtVoid	RiErrorPrint(RtInt code, RtInt severity, const char* message);
	virtual	RtVoid	RiExposure(RtFloat gain, RtFloat gamma);
	virtual	RtVoid	RiExteriorV(const char *name, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiFormat(RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio);
	virtual	RtVoid	RiFrameAspectRatio(RtFloat frameratio);
	virtual	RtVoid	RiFrameBegin(RtInt number);
	virtual	RtVoid	RiFrameEnd();
	virtual	RtFloat RiGaussianFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth);
	virtual	RtVoid	RiGeneralPolygonV(RtInt nloops, RtInt nverts[], RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiGeometricApproximation(RtToken type, RtFloat value);
	//virtual	RtVoid	RiGeometry();
	//virtual	RtVoid	RiGeometryV();
	virtual	RtVoid	RiHiderV(RtToken type, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiHyperboloidV(RtPoint point1, RtPoint point2, RtFloat thetamax,RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiIdentity();
	virtual	RtVoid	RiIlluminate(RtLightHandle light, RtBoolean onoff);
	virtual	RtVoid	RiImagerV(const char *name, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiInteriorV(const char *name, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtLightHandle RiLightSourceV(const char *name, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiMakeBumpV(const char *imagefile, const char *bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiMakeCubeFaceEnvironmentV(const char *px, const char *nx, const char *py, const char *ny, const char *pz, const char *nz, const char *reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiMakeLatLongEnvironmentV(const char *imagefile, const char *reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiMakeShadowV(const char *picfile, const char *shadowfile, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiMakeTextureV(const char *imagefile, const char *texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiMatte(RtBoolean onoff);
	virtual	RtVoid	RiMotionBeginV(RtInt N, RtFloat times[]);
	virtual	RtVoid	RiMotionEnd();
	virtual	RtVoid	RiNuPatchV(RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtObjectHandle	RiObjectBegin();
	virtual	RtVoid	RiObjectEnd();
	virtual	RtVoid	RiObjectInstance(RtObjectHandle handle);
	virtual	RtVoid	RiOpacity(RtColor Os);
	virtual	RtVoid	RiOptionV(const char *name, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiOrientation(RtToken orientation);
	virtual	RtVoid	RiParaboloidV(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiPatchMeshV(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiPatchV(RtToken type, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiPerspective(RtFloat fov);
	virtual	RtVoid	RiPixelFilter(RtFilterFunc function, RtFloat xwidth, RtFloat ywidth);
	virtual	RtVoid	RiPixelSamples(RtFloat xsamples, RtFloat ysamples);
	virtual	RtVoid	RiPixelVariance(RtFloat variance);
	virtual	RtVoid	RiPointsGeneralPolygonsV(RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiPointsPolygonsV(RtInt npolys, RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiPolygonV(RtInt nvertices, RtInt count, RtToken tokens[], RtPointer values[]);
//	virtual	RtVoid	RiProcedural(RtPointer data, RtBound bound, RtFunc refineproc, RtFunc freeproc);
	virtual	RtVoid	RiProjectionV(const char * name, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiQuantize(RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude);
	virtual	RtVoid	RiRelativeDetail(RtFloat relativedetail);
	virtual	RtVoid	RiReverseOrientation();
	virtual	RtVoid	RiRotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz);
	virtual	RtVoid	RiScale(RtFloat sx, RtFloat sy, RtFloat sz);
	virtual	RtVoid	RiScreenWindow(RtFloat left, RtFloat right, RtFloat bottom, RtFloat top);
	virtual	RtVoid	RiShadingInterpolation(RtToken type);
	virtual	RtVoid	RiShadingRate(RtFloat size);
	virtual	RtVoid	RiShutter(RtFloat opentime, RtFloat closetime);
	virtual	RtVoid	RiSides(RtInt nsides);
	virtual	RtFloat RiSincFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth);
	virtual	RtVoid	RiSkew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1, RtFloat dx2, RtFloat dy2, RtFloat dz2);
	virtual	RtVoid	RiSolidBegin(RtToken type);
	virtual	RtVoid	RiSolidEnd();
	virtual	RtVoid	RiSphereV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiSubdivisionMeshV(RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiSurfaceV(const char *name, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiTextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4);
	virtual	RtVoid	RiTorusV(RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]);
	virtual	RtVoid	RiTransform(RtMatrix transform);
	virtual	RtVoid	RiTransformBegin();
	virtual	RtVoid	RiTransformEnd();
	virtual	RtPoint* RiTransformPoints(RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[]);
	virtual	RtVoid	RiTranslate(RtFloat dx, RtFloat dy, RtFloat dz);
	virtual	RtFloat RiTriangleFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth);
	virtual	RtVoid	RiTrimCurve(RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[]);
	virtual	RtVoid	RiWorldBegin();
	virtual	RtVoid	RiWorldEnd();

private:
	std::ostream& m_Stream;
	unsigned int m_CurrentLightHandle;
};

}; // namespace librib2stream

#endif // LIBRIB2STREAM
