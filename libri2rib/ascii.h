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
 *  \brief RIB ascii output class
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#ifndef RI2RIB_ASCII_H
#define RI2RIB_ASCII_H 1

#include <fstream>
#include <stack>
#include <string>
#include "aqsis.h"
#include "ri.h"
#include "dictionary.h"

namespace libri2rib
{

class CqASCII
{
	private:
		CqDictionary dictionary;

		RtInt colorNComps;
		RtInt ObjectHandle;
		RtInt LightHandle;

		struct Steps
		{
			RtInt uStep;
			RtInt vStep;
		};
		std::stack<Steps> steps;

		std::ofstream out;
		void printArray ( RtInt n, RtInt *p );
		void printArray ( RtInt n, RtFloat *p );
		void printToken ( RtToken t );
		void printCharP ( const char *c );
		void printPL ( RtInt n, RtToken tokens[], RtPointer parms[],
		               RtInt vertex = 1, RtInt varying = 1, RtInt uniform = 1 );
		std::string getFilterFuncName ( RtFilterFunc, std::string );

		void push();
		void pop();

	public:
		CqASCII();
		~CqASCII()
		{}

	public:
		RtToken RiDeclare( const char *name, const char *declaration );

		/* RenderMan State machine */
		RtVoid RiBegin( RtToken name );
		RtVoid RiEnd( RtVoid );

		RtVoid RiFrameBegin( RtInt frame );
		RtVoid RiFrameEnd( RtVoid );

		RtVoid RiWorldBegin( RtVoid );
		RtVoid RiWorldEnd( RtVoid );

		RtObjectHandle RiObjectBegin( RtVoid );
		RtVoid RiObjectEnd( RtVoid );
		RtVoid RiObjectInstance( RtObjectHandle handle );

		RtVoid RiAttributeBegin( RtVoid );
		RtVoid RiAttributeEnd( RtVoid );

		RtVoid RiTransformBegin( RtVoid );
		RtVoid RiTransformEnd( RtVoid );

		RtVoid RiSolidBegin( RtToken operation );
		RtVoid RiSolidEnd( RtVoid );

		RtVoid RiMotionBeginV( RtInt n, RtFloat times[] );
		RtVoid RiMotionEnd( RtVoid );


		/* CAMERA OPTIONS */
		RtVoid RiFormat ( RtInt xres, RtInt yres, RtFloat aspect );
		RtVoid RiFrameAspectRatio ( RtFloat aspect );
		RtVoid RiScreenWindow ( RtFloat left, RtFloat right, RtFloat bot, RtFloat top );
		RtVoid RiCropWindow ( RtFloat xmin, RtFloat xmax, RtFloat ymin, RtFloat ymax );
		RtVoid RiProjectionV ( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiClipping( RtFloat hither, RtFloat yon );
		RtVoid RiClippingPlane( RtFloat x, RtFloat y, RtFloat z, RtFloat nx, RtFloat ny, RtFloat nz );
		RtVoid RiDepthOfField ( RtFloat fstop, RtFloat focallength, RtFloat focaldistance );
		RtVoid RiShutter( RtFloat min, RtFloat max );

		/* DISPLAY OPTIONS */
		RtVoid RiPixelVariance( RtFloat variation );
		RtVoid RiPixelSamples( RtFloat xsamples, RtFloat ysamples );
		RtVoid RiPixelFilter( RtFilterFunc filterfunc, RtFloat xwidth, RtFloat ywidth );
		RtVoid RiExposure( RtFloat gain, RtFloat gamma );
		RtVoid RiImagerV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiQuantize( RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ampl );
		RtVoid RiDisplayV( const char *name, RtToken type, RtToken mode,
		                   RtInt n, RtToken tokens[], RtPointer parms[] );

		/* ADDITIONAL OPTIONS */
		RtVoid RiHiderV( const char *type, RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiColorSamples( RtInt n, RtFloat nRGB[], RtFloat RGBn[] );
		RtVoid RiRelativeDetail( RtFloat relativedetail );
		RtVoid RiOptionV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] );


		/* SHADING ATTRIBUTES */
		RtVoid RiColor( RtColor color );
		RtVoid RiOpacity( RtColor color );
		RtVoid RiTextureCoordinates( RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2,
		                             RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4 );
		RtLightHandle RiLightSourceV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] );
		RtLightHandle RiAreaLightSourceV( const char *name,
		                                  RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiIlluminate( RtLightHandle light, RtBoolean onoff );
		RtVoid RiSurfaceV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiAtmosphereV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiInteriorV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiExteriorV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiShadingRate( RtFloat size );
		RtVoid RiShadingInterpolation( RtToken type );
		RtVoid RiMatte( RtBoolean onoff );

		/* GEOMETRY ATTRIBUTES */
		RtVoid RiBound( RtBound );
		RtVoid RiDetail( RtBound );
		RtVoid RiDetailRange( RtFloat minvis, RtFloat lowtran, RtFloat uptran, RtFloat maxvis );
		RtVoid RiGeometricApproximation( RtToken type, RtFloat value );
		RtVoid RiBasis( RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep );
		RtVoid RiTrimCurve( RtInt nloops, RtInt ncurves[], RtInt order[],
		                    RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[],
		                    RtFloat u[], RtFloat v[], RtFloat w[] );
		RtVoid RiOrientation( RtToken orientation );
		RtVoid RiReverseOrientation( RtVoid );
		RtVoid RiSides( RtInt sides );
		RtVoid RiDisplacementV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] );

		/* TRANSFORMATIONS */
		RtVoid RiIdentity( RtVoid );
		RtVoid RiTransform( RtMatrix transform );
		RtVoid RiConcatTransform( RtMatrix transform );
		RtVoid RiPerspective( RtFloat fov );
		RtVoid RiTranslate( RtFloat dx, RtFloat dy, RtFloat dz );
		RtVoid RiRotate( RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz );
		RtVoid RiScale( RtFloat sx, RtFloat sy, RtFloat sz );
		RtVoid RiSkew( RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
		               RtFloat dx2, RtFloat dy2, RtFloat dz2 );
		RtVoid RiDeformationV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiCoordinateSystem( RtToken space );
		RtVoid RiCoordSysTransform( RtToken space );

		RtVoid RiAttributeV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] );

		/* PRIMITIVES */
		RtVoid RiPolygonV( RtInt nverts, RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiGeneralPolygonV( RtInt nloops, RtInt nverts[], RtInt n,
		                          RtToken tokens[], RtPointer parms[] );
		RtVoid RiPointsPolygonsV( RtInt npolys, RtInt nverts[], RtInt verts[], RtInt n,
		                          RtToken tokens[], RtPointer parms[] );
		RtVoid RiPointsGeneralPolygonsV( RtInt npolys, RtInt nloops[], RtInt nverts[],
		                                 RtInt verts[], RtInt n, RtToken tokens[],
		                                 RtPointer parms[] );
		RtVoid RiPatchV( RtToken type, RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiPatchMeshV( RtToken type, RtInt nu, RtToken uwrap,
		                     RtInt nv, RtToken vwrap, RtInt n, RtToken tokens[],
		                     RtPointer parms[] );
		RtVoid RiNuPatchV( RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin,
		                   RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[],
		                   RtFloat vmin, RtFloat vmax,
		                   RtInt n, RtToken tokens[], RtPointer parms[] );

		RtVoid RiSphereV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax,
		                  RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiConeV( RtFloat height, RtFloat radius, RtFloat tmax,
		                RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiCylinderV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax,
		                    RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiHyperboloidV( RtPoint point1, RtPoint point2, RtFloat tmax,
		                       RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiParaboloidV( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat tmax,
		                      RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiDiskV( RtFloat height, RtFloat radius, RtFloat tmax,
		                RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiTorusV( RtFloat majrad, RtFloat minrad, RtFloat phimin, RtFloat phimax,
		                 RtFloat tmax, RtInt n, RtToken tokens[], RtPointer parms[] );

		RtVoid RiBlobbyV( RtInt nleaf, RtInt ncode, RtInt code[],
		                  RtInt nflt, RtFloat flt[],
		                  RtInt nstr, RtToken str[],
		                  RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiPointsV( RtInt npoints,
		                  RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiCurvesV( RtToken type, RtInt ncurves,
		                  RtInt nvertices[], RtToken wrap,
		                  RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiSubdivisionMeshV( RtToken mask, RtInt nf, RtInt nverts[],
		                           RtInt verts[],
		                           RtInt ntags, RtToken tags[], RtInt numargs[],
		                           RtInt intargs[], RtFloat floatargs[],
		                           RtInt n, RtToken tokens[], RtPointer parms[] );


		RtVoid RiProcedural( RtPointer data, RtBound bound,
		                     RtVoid ( *subdivfunc ) ( RtPointer, RtFloat ),
		                     RtVoid ( *freefunc ) ( RtPointer ) );
		RtVoid RiGeometryV( RtToken type, RtInt n, RtToken tokens[],
		                    RtPointer parms[] );

		/* MISC */
		RtVoid RiMakeTextureV( const char *pic, const char *tex, RtToken swrap, RtToken twrap,
		                       RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
		                       RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiMakeBumpV( const char *pic, const char *tex, RtToken swrap, RtToken twrap,
		                    RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
		                    RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiMakeLatLongEnvironmentV( const char *pic, const char *tex, RtFilterFunc filterfunc,
		                                  RtFloat swidth, RtFloat twidth,
		                                  RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiMakeCubeFaceEnvironmentV( const char *px, const char *nx, const char *py, const char *ny,
		                                   const char *pz, const char *nz, const char *tex, RtFloat fov,
		                                   RtFilterFunc filterfunc, RtFloat swidth,
		                                   RtFloat ywidth,
		                                   RtInt n, RtToken tokens[], RtPointer parms[] );
		RtVoid RiMakeShadowV( const char *pic, const char *tex,
		                      RtInt n, RtToken tokens[], RtPointer parms[] );

		/* ARCHIVE */
		RtVoid RiArchiveRecord( RtToken type, std::string );
		RtVoid RiReadArchiveV( RtToken name, RtArchiveCallback callback,
		                       RtInt n, RtToken tokens[], RtPointer parms[] );

		/* ERROR HANDLER */
		RtVoid RiErrorHandler( RtErrorFunc handler );
};


} /* namespace libri2rib */
#endif
