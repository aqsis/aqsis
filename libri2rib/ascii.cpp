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
 *  \brief RIB ascii output class implementation.
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#include "ascii.h"
#include "error.h"
#include "inlineparse.h"

using namespace libri2rib;
using std::endl;


CqASCII::CqASCII() : colorNComps(3), lastObjectHandle(0), lastLightHandle(0)
{
    Steps a={RI_BEZIERSTEP,RI_BEZIERSTEP};
    steps.push(a);
}



// *********************************************************************
// ******* ******* ******* STEPS STACK FUNCTIONS ******* ******* *******
// *********************************************************************
void CqASCII::push()
{
    steps.push(steps.top());
}
void CqASCII::pop()
{
    if (steps.size()==0) {
	return;
    }
    steps.pop();
}





// **************************************************************
// ******* ******* ******* PRINTING TOOLS ******* ******* *******
// **************************************************************
void CqASCII::printPL(RtInt n, RtToken tokens[], RtPointer parms[],
		      RtInt vertex, RtInt varying, RtInt uniform)
{
    RtFloat *flt;
    RtInt *nt;
    char **cp;

    TqTokenId id;
    EqTokenType tt;

    RtInt i;
    TqUint j;
    for (i=0; i<n ; i++) {
	try {
	    id=dictionary.getTokenId(std::string(tokens[i]));
	} catch (CqError &r) {
	    r.manage();
	    continue;
	}
	tt=dictionary.getType(id);

	out << "\"" << std::string(tokens[i])<< "\" ";
	out << "[ ";

	for (j=0; j<(dictionary.allocSize(id, vertex, varying, uniform)); j++ ) {
	    switch (tt) {
	    case FLOAT:
	    case POINT:
	    case VECTOR:
	    case NORMAL:
	    case COLOR:
	    case MATRIX:
	    case HPOINT:
		flt=static_cast<RtFloat *> (parms[i]);
		out << flt[j] <<" ";
		break;
	    case STRING:
		cp=static_cast<char **> (parms[i]);
		out <<"\""<< std::string(cp[j]) <<"\" ";
		break;
	    case INTEGER:
		nt=static_cast<RtInt *> (parms[i]);
		out << nt[j] << " ";
		break;
	    }
	}
	out << "] ";
    }
    out << endl;
}

void CqASCII::printToken(RtToken t)
{
    out << "\"" << std::string(t) << "\" ";
}
void CqASCII::printCharP(const char *c)
{
    out << "\"" << std::string(c) << "\" ";
}

void CqASCII::printArray (RtInt n, RtInt *p)
{
    out << "[ ";
    for (RtInt i=0; i<n; i++) {
	out << p[i] << " ";
    }
    out << "] ";
}

void CqASCII::printArray (RtInt n, RtFloat *p)
{
    out << "[ ";
    for (RtInt i=0; i<n; i++) {
	out << p[i] << " ";
    }
    out << "] ";
}

std::string CqASCII::getFilterFuncName(RtFilterFunc filterfunc, std::string message)
{
    if (filterfunc==RiBoxFilter) return "box ";
    else if (filterfunc==RiTriangleFilter) return "triangle ";
    else if (filterfunc==RiCatmullRomFilter) return "catmull-rom ";
    else if (filterfunc==RiSincFilter) return "sinc ";
    else if (filterfunc==RiGaussianFilter) return "gaussian ";
    else if (filterfunc==RiDiskFilter) return "disk ";
    else if (filterfunc==RiBesselFilter) return "bessel ";
    else {
	std::string st("Unknown RiFilterFunc. ");
	st+=message; st.append(" function skipped.");
	throw CqError(RIE_CONSISTENCY, RIE_WARNING, st, TqTrue);
	return "";
    }
}




// *********************************************************************
// ******* ******* ******* RIB OUTPUT FUNCTIONS  ******* ******* *******
// *********************************************************************
RtToken CqASCII::RiDeclare(const char *name, const char *declaration) 
{
    CqInlineParse ip;
    std::string a(name);
    std::string b(declaration);

    b+=" ";
    b+=a;
    ip.parse(b);
    dictionary.addToken(ip.getIdentifier(),ip.getClass(),ip.getType(),ip.getQuantity(), TqFalse);

    out <<"Declare ";
    printCharP(name);
    printCharP(declaration);
    out <<endl;

    return const_cast<RtToken> (name);
}
RtVoid CqASCII::RiBegin(RtToken name)
{
    out.open(name,std::ios::out);
    if (!out) {
	throw CqError(RIE_NOFILE, RIE_ERROR, std::string("Unable to open file ")+std::string(name), TqFalse);
    }
	out << "##RenderMan RIB-Structure 1.0" << endl;
	out << "version 3.03" << endl;
}
RtVoid CqASCII::RiEnd(RtVoid)
{
    out.close();
}
RtVoid CqASCII::RiFrameBegin(RtInt frame)
{
    out <<"FrameBegin "<< frame <<endl;
    push();
}
RtVoid CqASCII::RiFrameEnd(RtVoid)
{
    out <<"FrameEnd"<<endl;
    pop();
}
RtVoid CqASCII::RiWorldBegin(RtVoid)
{
    out <<"WorldBegin"<<endl;
    push();
}
RtVoid CqASCII::RiWorldEnd(RtVoid)
{
    out <<"WorldEnd"<<endl;
    pop();
}
RtObjectHandle CqASCII::RiObjectBegin(RtVoid)
{
    out <<"ObjectBegin"<<endl;

    push();

    RtInt t=lastObjectHandle;
    lastObjectHandle+=1;
    return (RtObjectHandle) t;
}
RtVoid CqASCII::RiObjectEnd(RtVoid)
{
    out <<"ObjectEnd" <<endl;
    pop();
}
RtVoid  CqASCII::RiObjectInstance(RtObjectHandle handle)
{
    out <<"ObjectInstance "<< (RtInt) handle <<endl;
}
RtVoid  CqASCII::RiAttributeBegin(RtVoid)
{
    out <<"AttributeBegin"<<endl;
    push();
}
RtVoid  CqASCII::RiAttributeEnd(RtVoid)
{
    out <<"AttributeEnd"<<endl;
    pop();
}
RtVoid  CqASCII::RiTransformBegin(RtVoid)
{
    out <<"TransformBegin"<<endl;
}
RtVoid  CqASCII::RiTransformEnd(RtVoid)
{
    out <<"TransformEnd"<<endl;
}
RtVoid  CqASCII::RiSolidBegin(RtToken operation)
{
    out <<"SolidBegin ";
    printToken(operation);
    out << endl;
    push();
}
RtVoid  CqASCII::RiSolidEnd(RtVoid)
{
    out <<"SolidEnd"<<endl;
    pop();
}
RtVoid  CqASCII::RiMotionBeginV(RtInt n, RtFloat times[])
{
    out <<"MotionBegin ";
    printArray(n,times);
    out << endl;
}
RtVoid  CqASCII::RiMotionEnd(RtVoid)
{
    out <<"MotionEnd"<<endl; 
}




// **************************************************************
// ******* ******* ******* CAMERA OPTIONS ******* ******* *******
// **************************************************************
RtVoid  CqASCII::RiFormat (RtInt xres, RtInt yres, RtFloat aspect)
{
    out <<"Format "<< xres <<" "<< yres <<" "<< aspect <<endl;
}
RtVoid  CqASCII::RiFrameAspectRatio (RtFloat aspect)
{
    out <<"FrameAspectRatio "<< aspect <<endl; 
}
RtVoid  CqASCII::RiScreenWindow (RtFloat left, RtFloat right, RtFloat bottom, RtFloat top)
{
    out <<"ScreenWindow "<< left <<" "<< right <<" "<< bottom <<" "<< top <<endl;
}
RtVoid  CqASCII::RiCropWindow (RtFloat xmin, RtFloat xmax, RtFloat ymin, RtFloat ymax)
{
    out <<"CropWindow "<< xmin <<" "<< xmax <<" "<< ymin <<" "<< ymax <<endl; 
}
RtVoid  CqASCII::RiProjectionV (const char *name, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Projection ";
    printCharP(name);
    printPL(n,tokens,parms);
}
RtVoid  CqASCII::RiClipping(RtFloat hither, RtFloat yon)
{
    out <<"Clipping "<< hither <<" "<< yon <<endl;
}
RtVoid  CqASCII::RiClippingPlane(RtFloat x, RtFloat y, RtFloat z,
				 RtFloat nx, RtFloat ny, RtFloat nz)
{
    out <<"ClippingPlane "<< x <<" "<< y <<" "<< z <<" "<< nx <<" "<< ny <<" "<< nz <<endl;
}
RtVoid  CqASCII::RiDepthOfField (RtFloat fstop, RtFloat focallength, RtFloat focaldistance)
{
    out <<"DepthOfField "<< fstop <<" "<< focallength <<" "<< focaldistance <<endl;
}
RtVoid  CqASCII::RiShutter(RtFloat min, RtFloat max)
{
    out <<"Shutter "<< min <<" "<< max <<" "<<endl;
}




// ***************************************************************
// ******* ******* ******* DISPLAY OPTIONS ******* ******* *******
// ***************************************************************
RtVoid  CqASCII::RiPixelVariance(RtFloat variation)
{
    out <<"PixelVariance "<< variation <<endl;
}
RtVoid  CqASCII::RiPixelSamples(RtFloat xsamples, RtFloat ysamples)
{
    out <<"PixelSamples "<< xsamples <<" "<< ysamples <<endl;
}
RtVoid  CqASCII::RiPixelFilter(RtFilterFunc filterfunc, RtFloat xwidth, RtFloat ywidth)
{
    std::string st=getFilterFuncName(filterfunc, "RiPixelFilter");
    out <<"PixelFilter "<< st << xwidth << " " << ywidth << endl;
}
RtVoid  CqASCII::RiExposure(RtFloat gain, RtFloat gamma)
{
    out <<"Exposure "<< gain <<" "<< gamma <<endl;
}
RtVoid  CqASCII::RiImagerV(const char *name, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Imager ";
    printCharP(name);
    printPL(n,tokens,parms);
}
RtVoid  CqASCII::RiQuantize(RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ampl)
{
    out <<"Quantize ";
    printToken(type);
    out << one <<" "<< min <<" "<< max <<" "<< ampl <<endl;
}
RtVoid  CqASCII::RiDisplayV(const char *name, RtToken type, RtToken mode,
			    RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Display ";
    printCharP(name);
    printToken(type);
    printToken(mode);
    printPL(n, tokens, parms);
}




// ******************************************************************
// ******* ******* ******* ADDITIONAL OPTIONS ******* ******* *******
// ******************************************************************
RtVoid  CqASCII::RiHiderV(const char *type, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Hider ";
    printCharP(type);
    printPL(n,tokens,parms);
}
RtVoid  CqASCII::RiColorSamples(RtInt n, RtFloat nRGB[], RtFloat RGBn[])
{
    throw CqError (RIE_UNIMPLEMENT, RIE_WARNING, "RiColorSamples not implemented", TqTrue);

    out <<"ColorSamples ";
    printArray(n*3,nRGB);
    printArray(n*3,RGBn);
    out <<endl;

    colorNComps=n;
}
RtVoid  CqASCII::RiRelativeDetail(RtFloat relativedetail)
{
    out <<"RelativeDetail "<< relativedetail <<endl;
}
RtVoid  CqASCII::RiOptionV(const char *name, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Option ";
    printCharP(name);
    printPL(n,tokens,parms);
}




// ******************************************************************
// ******* ******* ******* SHADING ATTRIBUTES ******* ******* *******
// ******************************************************************
RtVoid  CqASCII::RiColor(RtColor color)
{
    out <<"Color ";
    printArray(colorNComps,color);
    out <<endl;  
}
RtVoid  CqASCII::RiOpacity(RtColor color)
{
    out <<"Opacity ";
    printArray(colorNComps,color);
    out <<endl;
}
RtVoid  CqASCII::RiTextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2,
				      RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4)
{
    out <<"TextureCoordinates ";
    out << s1 <<" "<< t1 <<" "<< s2 <<" "<< t2 <<" ";
    out << s3 <<" "<< t3 <<" "<< s4 <<" "<< t4 <<endl;
}
RtLightHandle CqASCII::RiLightSourceV(const char *name, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"LightSource ";
	printCharP(name);
    

    RtInt t=lastLightHandle;
    lastLightHandle+=1;
	out <<(RtInt) lastLightHandle <<" ";
	printPL(n, tokens, parms);
    return (RtLightHandle) t;
}
RtLightHandle CqASCII::RiAreaLightSourceV(const char *name,
					  RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"AreaLightSource ";
	printCharP(name);
    

    RtInt t=lastLightHandle;
    lastLightHandle+=1;
	out <<(RtInt) lastLightHandle <<" ";
	printPL(n, tokens, parms);
    return (RtLightHandle) t;
}
RtVoid  CqASCII::RiIlluminate(RtLightHandle light, RtBoolean onoff)
{
    out <<"Illuminate ";
    out <<(RtInt) light <<" ";
    if (onoff==RI_TRUE)
	out << "1" <<endl;
    else
	out << "0" <<endl;
}
RtVoid  CqASCII::RiSurfaceV(const char *name, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Surface ";
    printCharP(name);
    printPL(n, tokens, parms);
}
RtVoid  CqASCII::RiAtmosphereV(const char *name, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Atmosphere ";
    printCharP(name);
    printPL(n, tokens, parms);
}
RtVoid  CqASCII::RiInteriorV(const char *name, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Interior ";
    printCharP(name);
    printPL(n, tokens, parms);
}
RtVoid  CqASCII::RiExteriorV(const char *name, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Exterior ";
    printCharP(name);
    printPL(n, tokens, parms);
}
RtVoid  CqASCII::RiShadingRate(RtFloat size)
{
    out <<"ShadingRate "<< size <<endl;
}
RtVoid  CqASCII::RiShadingInterpolation(RtToken type)
{
    out <<"ShadingInterpolation ";
    printToken(type);
    out << endl;
}
RtVoid  CqASCII::RiMatte(RtBoolean onoff)
{
    out <<"Matte ";
    if (onoff==RI_TRUE)
	out << "1" <<endl;
    else
	out << "0" <<endl;
}




// *******************************************************************
// ******* ******* ******* GEOMETRY ATTRIBUTES ******* ******* *******
// *******************************************************************
RtVoid  CqASCII::RiBound(RtBound b)
{
    out <<"Bound ";
    printArray(6,b);
    out << endl;
}
RtVoid  CqASCII::RiDetail(RtBound d)
{
    out <<"Detail ";
    printArray(6,d);
    out << endl;
}
RtVoid  CqASCII::RiDetailRange(RtFloat minvis, RtFloat lowtran, RtFloat uptran, RtFloat maxvis)
{
    out <<"DetailRange "<< minvis <<" "<< lowtran <<" "<< uptran <<" "<< maxvis <<endl;
}
RtVoid  CqASCII::RiGeometricApproximation(RtToken type, RtFloat value)
{
    out <<"GeometricApproximation ";
    printToken(type);
    out << value << endl;
}
RtVoid  CqASCII::RiBasis(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep)
{
    RtInt i;
    out <<"Basis [";
    for (i=0; i<16; i++) {
	out << ubasis[i/4][i%4] << " ";
    }
    out << "] ";
    out << ustep <<" ";
    out << "[ ";
    for (i=0; i<16; i++) {
	out << vbasis[i/4][i%4] << " ";
    }
    out << "] ";
    out << vstep <<endl;


    steps.top().uStep=ustep;
    steps.top().vStep=vstep;
}
RtVoid  CqASCII::RiTrimCurve(RtInt nloops, RtInt ncurves[], RtInt order[],
			     RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[],
			     RtFloat u[], RtFloat v[], RtFloat w[])
{
    RtInt i;
    RtInt ttlc=0;
    for(i=0; i<nloops; i++)
	ttlc+=ncurves[i];

    RtInt nbcoords=0;
    RtInt knotsize=0;
    for(i=0; i<ttlc; i++) {
	nbcoords+=n[i];
	knotsize+=order[i]+n[i];
    }

    out <<"TrimCurve ";
    printArray(nloops, ncurves);
    printArray(ttlc, order);
    printArray(knotsize, knot);
    printArray(ttlc, min);
    printArray(ttlc, max);
    printArray(ttlc, n);
    printArray(nbcoords,u);
    printArray(nbcoords,v);
    printArray(nbcoords,w);
}
RtVoid  CqASCII::RiOrientation(RtToken o)
{
    out <<"Orientation ";
    printToken(o);
    out << endl;
}
RtVoid  CqASCII::RiReverseOrientation(RtVoid)
{
    out <<"ReverseOrientation"<<endl;
}
RtVoid  CqASCII::RiSides(RtInt sides)
{
    out <<"Sides "<< sides <<endl;
}
RtVoid  CqASCII::RiDisplacementV(const char *name, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Displacement ";
    printCharP(name);
    printPL(n, tokens, parms);
}




// ***************************************************************
// ******* ******* ******* TRANSFORMATIONS ******* ******* *******
// ***************************************************************
RtVoid  CqASCII::RiIdentity(RtVoid)
{
    out <<"Identity"<<endl;
}
RtVoid  CqASCII::RiTransform(RtMatrix transform)
{
    out <<"Transform [";
    for (RtInt i=0; i<16; i++) {
	out << transform[i/4][i%4] << " ";
    }
    out << "]" << endl;
}
RtVoid  CqASCII::RiConcatTransform(RtMatrix transform)
{
    out <<"ConcatTransform [";
    for (RtInt i=0; i<16; i++) {
	out << transform[i/4][i%4] << " ";
    }
    out << "]" << endl;
}
RtVoid  CqASCII::RiPerspective(RtFloat fov)
{
    out <<"Perspective "<< fov <<endl;
}
RtVoid  CqASCII::RiTranslate(RtFloat dx, RtFloat dy, RtFloat dz)
{
    out <<"Translate "<< dx <<" "<< dy <<" "<< dz <<" "<<endl;
}
RtVoid  CqASCII::RiRotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz)
{
    out <<"Rotate "<< angle <<" "<< dx <<" "<< dy <<" "<< dz <<" "<<endl;
}
RtVoid  CqASCII::RiScale(RtFloat sx, RtFloat sy, RtFloat sz)
{
    out <<"Scale "<< sx <<" "<< sy <<" "<< sz <<" "<<endl;
}
RtVoid  CqASCII::RiSkew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
			RtFloat dx2, RtFloat dy2, RtFloat dz2)
{
    out <<"Skew " << angle << " ";
    out << dx1 <<" "<< dy1 <<" "<< dz1 <<" ";
    out << dx2 <<" "<< dy2 <<" "<< dz2 <<endl;
}
RtVoid  CqASCII::RiDeformationV(const char *name, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Deformation ";
    printCharP(name);
    printPL(n, tokens, parms);
}
RtVoid  CqASCII::RiCoordinateSystem(RtToken space)
{
    out <<"CoordinateSystem ";
    printToken(space);
    out << endl;
}
RtVoid  CqASCII::RiCoordSysTransform(RtToken space)
{
    out <<"CoordSysTransform ";
    printToken(space);
    out << endl;
}
RtPoint *CqASCII::RiTransformPoints(RtToken fromspace, RtToken tospace, RtInt n,
				    RtPoint points[])
{
    throw CqError(RIE_UNIMPLEMENT,RIE_WARNING,"RiTransformPoints is a C api only call", TqFalse);
    return (RtPoint *) 0;
}
RtVoid CqASCII::RiAttributeV(const char *name, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Attribute ";
    printCharP(name);
    printPL(n, tokens, parms);
}




// **********************************************************
// ******* ******* ******* PRIMITIVES ******* ******* *******
// **********************************************************
RtVoid  CqASCII::RiPolygonV(RtInt nverts, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Polygon ";
    printPL(n, tokens, parms, nverts, nverts);
}
RtVoid  CqASCII::RiGeneralPolygonV(RtInt nloops, RtInt nverts[], 
				   RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"GeneralPolygon ";
    printArray(nloops,nverts);

    RtInt nbpts=0;
    for (RtInt i=0; i<nloops; i++) {
	nbpts+=nverts[i];
    }
    printPL(n, tokens, parms, nbpts, nbpts);
}
RtVoid  CqASCII::RiPointsPolygonsV(RtInt npolys, RtInt nverts[], RtInt verts[],
				   RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"PointsPolygons ";
    printArray(npolys,nverts);

    RtInt i;
    RtInt nbpts=0;
    for (i=0; i<npolys; i++) {
	nbpts+=nverts[i];
    }
    printArray(nbpts,verts);

    RtInt psize=0;
    for (i=0; i<nbpts; i++) {
	if (psize<verts[i])
	    psize=verts[i];
    }
    printPL(n, tokens, parms, psize+1, psize+1, npolys);
}
RtVoid  CqASCII::RiPointsGeneralPolygonsV(RtInt npolys, RtInt nloops[], RtInt nverts[],
					  RtInt verts[],
					  RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"PointsGeneralpolygons ";
    printArray(npolys,nloops);

    RtInt i;
    RtInt nbvert=0;
    for (i=0; i<npolys; i++) {
	nbvert+=nloops[i];
    }
    printArray(nbvert,nverts);

    RtInt nv=0;
    for (i=0; i<nbvert; i++) {
	nv+=nverts[i];
    }
    printArray(nv,verts);

    RtInt psize=0;
    for (i=0; i<nv; i++) {
	if (psize<verts[i])
	    psize=verts[i];
    }
    printPL(n, tokens, parms, psize+1, psize+1, npolys);
}
RtVoid  CqASCII::RiPatchV(RtToken type, RtInt n, RtToken tokens[], RtPointer parms[])
{
    RtInt nb;
    if (type==RI_BILINEAR)
	nb=4;
    else if (type==RI_BICUBIC)
	nb=16;
    else {
	std::string st("Unknown RiPatch type: ");
	st+=std::string(type);
	st+=std::string("  RiPatch() instruction skipped");
	throw CqError(RIE_BADTOKEN, RIE_ERROR, st, TqTrue);
    }

    out <<"Patch ";
    printToken(type);
    printPL(n, tokens, parms, nb, 4);
}
RtVoid  CqASCII::RiPatchMeshV(RtToken type, RtInt nu, RtToken uwrap,
			      RtInt nv, RtToken vwrap, RtInt n, RtToken tokens[], 
			      RtPointer parms[])
{
    RtInt nuptch, nvptch;
    RtInt ii=0;
    if (type==RI_BILINEAR) {
	if (uwrap==RI_PERIODIC) {
	    nuptch=nu;
	} else if (uwrap==RI_NONPERIODIC) {
	    nuptch=nu-1;
	    ii+=1;
	} else {
	    std::string st("Unknown RiPatchMesh uwrap token:");
	    st+=std::string(uwrap);
	    st+=std::string("  RiPatchMesh instruction skipped");
	    throw CqError(RIE_BADTOKEN, RIE_ERROR, st, TqTrue);
	}
	if (vwrap==RI_PERIODIC) {
	    nvptch=nv;
	} else if (vwrap==RI_NONPERIODIC) {
	    nvptch=nv-1;
	    ii+=1;
	} else {
	    std::string st("Unknown RiPatchMesh vwrap token:");
	    st+=std::string(vwrap);
	    st+=std::string("  RiPatchMesh instruction skipped");
	    throw CqError(RIE_BADTOKEN, RIE_ERROR, st, TqTrue);
	}
	ii+=nuptch+nvptch;


    } else if (type==RI_BICUBIC) {
	RtInt nustep=steps.top().uStep;
	RtInt nvstep=steps.top().vStep;
    
	if (uwrap==RI_PERIODIC) {
	    nuptch=nu/nustep;
	} else if (uwrap==RI_NONPERIODIC) {
	    nuptch=(nu-4)/nustep +1;
	    ii+=1;
	} else {
	    std::string st("Unknown RiPatchMesh uwrap token:");
	    st+=std::string(uwrap);
	    st+=std::string("  RiPatchMesh instruction skipped");
	    throw CqError(RIE_BADTOKEN, RIE_ERROR, st, TqTrue);
	}
	if (vwrap==RI_PERIODIC) {
	    nvptch=nv/nvstep;
	} else if (vwrap==RI_NONPERIODIC) {
	    nvptch=(nv-4)/nvstep +1;
	    ii+=1;
	} else {
	    std::string st("Unknown RiPatchMesh vwrap token:");
	    st+=std::string(vwrap);
	    st+=std::string("  RiPatchMesh instruction skipped");
	    throw CqError(RIE_BADTOKEN, RIE_ERROR, st, TqTrue);
	}
	ii+=nuptch+nvptch;

  
    } else {
	std::string st("Unknown RiPatchMesh type:");
	st+=std::string(type);
	st+=std::string("  RiPatchMesh instruction skipped");
	throw CqError(RIE_BADTOKEN, RIE_ERROR, st, TqTrue);
    }

    out <<"PatchMesh ";
    printToken(type);
    out << nu << " ";
    printToken(uwrap);
    out << nv << " ";
    printToken(vwrap);
    printPL(n, tokens, parms, nu*nv, ii, nuptch*nvptch);
}
RtVoid  CqASCII::RiNuPatchV(RtInt nu, RtInt uorder, RtFloat uknot[],
			    RtFloat umin, RtFloat umax,
			    RtInt nv, RtInt vorder, RtFloat vknot[],
			    RtFloat vmin, RtFloat vmax,
			    RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"NuPatch ";
    out << nu <<" "<< uorder << " ";
    printArray(nu+uorder,uknot);
    out << umin <<" "<< umax <<" ";

    out << nv <<" "<< vorder << " ";
    printArray(nv+vorder,vknot);
    out << vmin <<" "<< vmax <<" ";
    printPL(n, tokens, parms, nu*nv, (2+nu-uorder)*(2+nv-vorder), (1+nu-uorder)*(1+nv-vorder));
}
RtVoid  CqASCII::RiSphereV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax,
			   RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Sphere "<< radius <<" "<< zmin<<" "<< zmax <<" "<< tmax <<" ";
    printPL(n, tokens, parms, 4, 4);
}
RtVoid  CqASCII::RiConeV(RtFloat height, RtFloat radius, RtFloat tmax,
			 RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Cone "<< height <<" "<< radius <<" "<< tmax <<" ";
    printPL(n, tokens, parms, 4, 4);
}
RtVoid  CqASCII::RiCylinderV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax,
			     RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Cylinder "<< radius <<" "<< zmin <<" "<< zmax <<" "<< tmax <<" ";
    printPL(n, tokens, parms, 4, 4);
}
RtVoid  CqASCII::RiHyperboloidV(RtPoint point1, RtPoint point2, RtFloat tmax,
				RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Hyperboloid ";
    printArray(3,point1);
    printArray(3,point2);
    out << tmax << " ";
    printPL(n, tokens, parms, 4, 4);
}
RtVoid  CqASCII::RiParaboloidV(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat tmax,
			       RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Paraboloid "<< rmax <<" "<< zmin <<" "<< zmax <<" "<< tmax <<" ";
    printPL(n, tokens, parms, 4, 4);
}
RtVoid  CqASCII::RiDiskV(RtFloat height, RtFloat radius, RtFloat tmax,
			 RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Disk "<< height <<" "<< radius <<" "<< tmax <<" ";
    printPL(n, tokens, parms, 4, 4);
}
RtVoid  CqASCII::RiTorusV(RtFloat majrad,RtFloat minrad,RtFloat phimin,RtFloat phimax,
			  RtFloat tmax, RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Torus "<< majrad <<" "<< minrad <<" "<< phimin <<" "<< phimax <<" "<< tmax <<" ";
    printPL(n, tokens, parms, 4, 4);
}
RtVoid  CqASCII::RiBlobbyV(RtInt nleaf, RtInt ncode, RtInt code[],
			   RtInt nflt, RtFloat flt[],
			   RtInt nstr, RtToken str[], 
			   RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Blobby " << nleaf << " ";
    printArray(ncode,code);
    printArray(nflt,flt);
    for(RtInt i=0; i<nstr; i++)
	printToken(str[i]);
    printPL(n, tokens, parms, nleaf, nleaf);
}
RtVoid  CqASCII::RiPointsV(RtInt npoints,
			   RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"Points ";
    printPL(n, tokens, parms, npoints, npoints);
}
RtVoid  CqASCII::RiCurvesV(RtToken type, RtInt ncurves,
			   RtInt nvertices[], RtToken wrap,
			   RtInt n, RtToken tokens[], RtPointer parms[])
{
    RtInt i;
    RtInt vval=0;
    if (type==RI_LINEAR) {
	if (wrap==RI_PERIODIC) {
	    for(i=0; i<ncurves; i++) {
		vval+=nvertices[i];
	    }
	} else if (wrap==RI_NONPERIODIC) {
	    for(i=0; i<ncurves; i++) {
		vval+=nvertices[i];
	    }
	} else {
	    std::string st("Unknown RiCurves wrap token:");
	    st+=std::string(wrap);
	    st+=std::string("  RiCurves instruction skipped");
	    throw CqError(RIE_BADTOKEN, RIE_ERROR, st, TqTrue);
	}
    } else if (type==RI_CUBIC) {
	if (wrap==RI_PERIODIC) {
	    for(i=0; i<ncurves; i++) {
		vval+=(nvertices[i]-4)/steps.top().vStep;
	    }
	} else if (wrap==RI_NONPERIODIC) {
	    for(i=0; i<ncurves; i++) {
		vval+=2 + (nvertices[i]-4)/steps.top().vStep;
	    }
	} else {
	    std::string st("Unknown RiCurves wrap token:");
	    st+=std::string(wrap);
	    st+=std::string("  RiCurves instruction skipped");
	    throw CqError(RIE_BADTOKEN, RIE_ERROR, st, TqTrue);
	}
    } else {
	std::string st("Unknown RiCurves type:");
	st+=std::string(type);
	st+=std::string("  RiCurves instruction skipped");
	throw CqError(RIE_BADTOKEN, RIE_ERROR, st, TqTrue);
    }

    out <<"Curves ";
    printToken(type);
    printArray(ncurves,nvertices);
    printToken(wrap);

    RtInt nbpts=0;
    for(i=0; i<ncurves; i++) {
	nbpts+=nvertices[i];
    }
    printPL(n, tokens, parms, nbpts, vval, ncurves);
}
RtVoid  CqASCII::RiSubdivisionMeshV(RtToken mask, RtInt nf, RtInt nverts[],
				    RtInt verts[],
				    RtInt ntags, RtToken tags[], RtInt numargs[],
				    RtInt intargs[], RtFloat floatargs[],
				    RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"SubdivisionMesh ";
    printToken(mask);
    printArray(nf,nverts);

    RtInt i;
    RtInt vsize=0;
    for(i=0; i<nf; i++) {
	vsize+=nverts[i];
    }
    printArray(vsize,verts);
  
    for(i=0; i<ntags; i++) {
	printToken(tags[i]);
    }
    printArray(ntags*2,numargs);

    RtInt isize=0, fsize=0;
    for(i=0; i<ntags*2; i++) {
	if (i%2==0)
	    isize+=numargs[i];
	else
	    fsize+=numargs[i];
    }
    printArray(isize,intargs);
    printArray(fsize,floatargs);

    RtInt psize=0;
    for(i=0; i<vsize; i++) {
	if (psize<verts[i])
	    psize=verts[i];
    }
    printPL(n, tokens, parms, psize+1, psize+1, nf);
}
RtVoid  CqASCII::RiProcedural(RtPointer data, RtBound bound,
			      RtVoid (*subdivfunc)(RtPointer, RtFloat),
			      RtVoid (*freefunc)(RtPointer))
{
    std::string sf;
    RtInt a;
    if (subdivfunc==RiProcDelayedReadArchive) {
	sf="DelayedReadArchive";
	a=1;
    } else if (subdivfunc==RiProcRunProgram) {
	sf="ReadProgram";
	a=2;
    } else if (subdivfunc==RiProcDynamicLoad) {
	sf="DynamicLoad";
	a=3;
    } else {
	throw CqError(RIE_SYNTAX, RIE_ERROR, "Unknow procedural function.",TqTrue);
    }

    out <<"Procedural ";
    RtInt i;
	switch (a) {
    case 1:
	out << sf <<" [ "<< std::string(&((char *) data)[0]) <<" ] [ "; 
	for (i=0; i<6 ; i++)
	    out << bound[i] <<" ";
	out << "]" << endl;
	break;
    case 2:
	out << sf <<" [ ";
	out << std::string(&((char *) data)[0]) <<" ";
	out << std::string(&((char *) data)[1]) <<" ] [ ";
	for (i=0; i<6 ; i++)
	    out << bound[i] <<" ";
	out << "]" << endl;
	break;
    case 3:
	out << sf <<" [ ";
	out << std::string(&((char *) data)[0]) <<" ";
	out << std::string(&((char *) data)[1]) <<" ] [ ";
	for (i=0; i<6 ; i++)
	    out << bound[i] <<" ";
	out << "]" << endl;
	break;
    }
}
RtVoid  CqASCII::RiGeometryV(RtToken type, RtInt n, RtToken tokens[], 
			     RtPointer parms[])
{
    out <<"Geometry ";
    printToken(type);
    printPL(n, tokens, parms);
}




// *******************************************************
// ******* ******* ******* TEXTURE ******* ******* *******
// *******************************************************
RtVoid  CqASCII::RiMakeTextureV(const char *pic, const char *tex, RtToken swrap, RtToken twrap,
				RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
				RtInt n, RtToken tokens[], RtPointer parms[])
{
    std::string ff=getFilterFuncName(filterfunc, "RiMakeTexture");

    out <<"MakeTexture ";
    printCharP(pic);
    printCharP(tex);
    printToken(swrap);
    printToken(twrap);
    out << ff <<" "<< swidth <<" "<< twidth <<" ";
    printPL(n, tokens, parms);
}
RtVoid  CqASCII::RiMakeBumpV(const char *pic, const char *tex, RtToken swrap, RtToken twrap,
			     RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
			     RtInt n, RtToken tokens[], RtPointer parms[])
{
    std::string ff=getFilterFuncName(filterfunc, "RiMakeBump");

    out <<"MakeBump ";
    printCharP(pic);
    printCharP(tex);
    printToken(swrap);
    printToken(twrap);
    out << ff <<" "<< swidth <<" "<< twidth <<" ";
    printPL(n, tokens, parms);
}
RtVoid  CqASCII::RiMakeLatLongEnvironmentV(const char *pic, const char *tex, RtFilterFunc filterfunc,
					   RtFloat swidth, RtFloat twidth,
					   RtInt n, RtToken tokens[], RtPointer parms[])
{
    std::string ff=getFilterFuncName(filterfunc, "RiMakeLatLongEnvironment");

    out <<"MakeLatLongEnvironment ";
    printCharP(pic);
    printCharP(tex);
    out << ff <<" "<< swidth <<" "<< twidth <<" ";
    printPL(n, tokens, parms);
}
RtVoid  CqASCII::RiMakeCubeFaceEnvironmentV(const char *px, const char *nx, const char *py, const char *ny,
					    const char *pz, const char *nz, const char *tex, RtFloat fov,
					    RtFilterFunc filterfunc, RtFloat swidth, 
					    RtFloat twidth,
					    RtInt n, RtToken tokens[], RtPointer parms[])
{
    std::string ff=getFilterFuncName(filterfunc, "RiMakeCubeFaceEnvironment");

    out <<"MakeCubeFaceEnvironment ";
    printCharP(px);
    printCharP(nx);
    printCharP(py);
    printCharP(ny);
    printCharP(pz);
    printCharP(nz);
    printCharP(tex);
    out << fov <<" "<< ff <<" "<< swidth <<" "<< twidth <<" ";
    printPL(n, tokens, parms);
}
RtVoid  CqASCII::RiMakeShadowV(const char *pic, const char *tex,
			       RtInt n, RtToken tokens[], RtPointer parms[])
{
    out <<"MakeShadow ";
    printCharP(pic);
    printCharP(tex);
    printPL(n, tokens, parms);
}




// *******************************************************
// ******* ******* ******* ARCHIVE ******* ******* *******
// *******************************************************
RtVoid CqASCII::RiArchiveRecord(RtToken type, std::string txt)
{
    std::string tmp;

    if (type==RI_COMMENT) tmp="#";
    else if (type==RI_STRUCTURE) tmp="##";
    else if (type==RI_VERBATIM) { out << txt; return; }
    else {
	std::string st("Unknown ArchiveRecord type: ");
	st+=std::string(type);
	throw CqError(RIE_BADTOKEN, RIE_ERROR, st, TqTrue);
    }
    out << tmp << txt << endl;
}

RtVoid CqASCII::RiReadArchiveV(RtToken name, RtArchiveCallback callback,
			       RtInt n, RtToken tokens[], RtPointer parms[])
{
    out << "ReadArchive ";
    printToken(name);
    out << endl;
}




// *************************************************************
// ******* ******* ******* ERROR HANDLER ******* ******* *******
// *************************************************************
RtVoid CqASCII::RiErrorHandler(RtErrorFunc handler)
{
    std::string ch;
    if (handler==RiErrorIgnore) ch="\"ignore\"";
    else if (handler==RiErrorPrint) ch="\"print\"";
    else if (handler==RiErrorAbort) ch="\"abort\"";
    else {
	throw CqError(RIE_CONSISTENCY, RIE_ERROR, "Unknown Error handler", TqTrue);
    }
    out <<"ErrorHandler "<< ch <<endl;
}
