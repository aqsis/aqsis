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
		\brief Implements the standard RenderMan quadric primitive classes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"quadrics.h"
#include	"micropolygon.h"
#include	"imagebuffer.h"
#include	"nurbs.h"

#include	"ri.h"

START_NAMESPACE(Aqsis)


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqQuadric&	CqQuadric::operator=(const CqQuadric& From)
{
	CqSurface::operator=(From);
	m_matTx=From.m_matTx;
	m_matITTx=From.m_matITTx;

	return(*this);
}


//---------------------------------------------------------------------
/** Transform the quadric primitive by the specified matrix.
 */

void	CqQuadric::Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx)
{
	m_matTx*=matTx;
	m_matITTx*=m_matITTx;
}


//---------------------------------------------------------------------
/** Dice the quadric into a grid of MPGs for rendering.
 */

CqMicroPolyGridBase* CqQuadric::Dice()
{
	// Create a new CqMicorPolyGrid for this patch
	CqMicroPolyGrid*  pGrid=new CqMicroPolyGrid(m_uDiceSize, m_vDiceSize, this);

	TqInt lUses=Uses();

	// Dice the primitive variables.
	if(USES(lUses,EnvVars_u))	u().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->u());
	if(USES(lUses,EnvVars_v))	v().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->v());
	if(USES(lUses,EnvVars_s))	s().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->s());
	if(USES(lUses,EnvVars_t))	t().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->t());
	if(USES(lUses,EnvVars_Cs))	Cs().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->Cs());
	if(USES(lUses,EnvVars_Os))	Os().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->Os());

	pGrid->Reset();
	CqVector3D	P,N;
	int v,u;
	for(v=0; v<=m_vDiceSize; v++)
	{
		for(u=0; u<=m_uDiceSize; u++)
		{
			P=DicePoint(u,v,N);
			pGrid->P()=m_matTx*P;
			pGrid->N()=m_matITTx*N;
			pGrid->Advance();
		}
	}
	pGrid->SetNormals(TqFalse);
	// Only shade if the ImageBuffer mode is at least RGB
	if(pCurrentRenderer()->optCurrent().iDisplayMode()&ModeRGB)
		pGrid->Shade();

	return(pGrid);
}


//---------------------------------------------------------------------
/** Determine whether the quadric is suitable for dicing.
 */

TqBool	CqQuadric::Diceable()
{
	EqtimateGridSize();
	if ((TqFloat)m_uDiceSize*(TqFloat)m_vDiceSize<=256.0)	return(TqTrue);
	else								return(TqFalse);
}


//---------------------------------------------------------------------
/** Eqtimate the size of the micropolygrid required to dice this GPrim to a suitable shading rate.
 */

void CqQuadric::EqtimateGridSize()
{	
	TqFloat maxusize,maxvsize;
	maxusize=maxvsize=0;

	CqMatrix matTx=pCurrentRenderer()->matSpaceToSpace("camera","raster")*m_matTx;

	m_uDiceSize=m_vDiceSize=ESTIMATEGRIDSIZE;

	TqFloat udist,vdist;
	CqVector3D p, pum1, pvm1[ESTIMATEGRIDSIZE];

	int v,u;
	for(v=0; v<=ESTIMATEGRIDSIZE; v++)
	{
		for(u=0; u<=ESTIMATEGRIDSIZE; u++)
		{
			p=DicePoint(u,v);
			p=matTx*p;
			// If we are on row two or above, calculate the mp size.
			if(v>=1 && u>=1)
			{
				udist=(p.x()-pum1.x())*(p.x()-pum1.x())+
					  (p.y()-pum1.y())*(p.y()-pum1.y());
				vdist=(pvm1[u-1].x()-pum1.x())*(pvm1[u-1].x()-pum1.x())+
					  (pvm1[u-1].y()-pum1.y())*(pvm1[u-1].y()-pum1.y());

				maxusize=MAX(maxusize,udist);
				maxvsize=MAX(maxvsize,vdist);
			}
			if(u>=1)	pvm1[u-1]=pum1;
			pum1=p;
		}
	}
	maxusize=sqrt(maxusize);
	maxvsize=sqrt(maxvsize);

	float rate=pAttributes()->fEffectiveShadingRate();
//	if(pCurrentRenderer()->Mode()==RenderMode_Shadows)
//	{
//		const TqFloat* pattrShadowShadingRate=m_pAttributes->GetFloatAttribute("render","shadow_shadingrate");
//		if(pattrShadowShadingRate!=0)
//			rate=pattrShadowShadingRate[0];
//	}	
	
	TqInt us=MAX(4,(TqInt)(ESTIMATEGRIDSIZE*maxusize/(rate)));
	TqInt vs=MAX(4,(TqInt)(ESTIMATEGRIDSIZE*maxvsize/(rate)));
	// Make size a power of 2
	us=1<<(TqInt)(log(us)/log(2));
	vs=1<<(TqInt)(log(vs)/log(2));
	m_uDiceSize=us;
	m_vDiceSize=vs;
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqSphere::CqSphere(TqFloat radius,TqFloat zmin,TqFloat zmax,TqFloat thetamin,TqFloat thetamax) :
	m_Radius(radius),
	m_ZMin(zmin),
	m_ZMax(zmax),
	m_ThetaMin(thetamin),
	m_ThetaMax(thetamax)
{
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqSphere&	CqSphere::operator=(const CqSphere& From)
{
	CqQuadric::operator=(From);
	m_Radius=From.m_Radius;
	m_ZMin=From.m_ZMin;
	m_ZMax=From.m_ZMax;
	m_ThetaMin=From.m_ThetaMin;
	m_ThetaMax=From.m_ThetaMax;

	return(*this);
}

//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqSphere::Bound() const
{
	TqFloat xminang,yminang,xmaxang,ymaxang;
/*	xminang=yminang=m_ThetaMin;
	xmaxang=ymaxang=m_ThetaMax;

	while(xminang<0)	xminang=yminang=xminang+360;
	while(xmaxang<0)	xmaxang=ymaxang=xmaxang+360;

	xminang=MIN(xminang,xmaxang);
	ymaxang=MAX(yminang,ymaxang);
	xmaxang=ymaxang;
	yminang=xminang;*/

	TqFloat vangmin=MIN(m_ZMax,m_ZMin);
	TqFloat vangmax=MAX(m_ZMax,m_ZMin);
	if(m_ZMin<0 && m_ZMax>0)	vangmax=0;

	vangmin=asin(vangmin/m_Radius);
	vangmax=asin(vangmax/m_Radius);

	// If start and end in same segement, just use the points.
/*	if(static_cast<TqInt>(m_ThetaMin/90)!=static_cast<TqInt>(m_ThetaMax/90))
	{
		if(yminang<90 && ymaxang>90)	yminang=90;
		if(yminang<270 && ymaxang>270)	ymaxang=270;
		if(xminang<180 && xmaxang>180)	xmaxang=180;
	}*/

	xminang=0;
	yminang=90;
	ymaxang=270;
	xmaxang=180;
	
	TqFloat x1=m_Radius*cos(RAD(xminang))*cos(vangmin);
	TqFloat x2=m_Radius*cos(RAD(xmaxang))*cos(vangmin);
	TqFloat x3=m_Radius*cos(RAD(xminang))*cos(vangmax);
	TqFloat x4=m_Radius*cos(RAD(xmaxang))*cos(vangmax);
	TqFloat y1=m_Radius*sin(RAD(yminang))*cos(vangmin);
	TqFloat y2=m_Radius*sin(RAD(ymaxang))*cos(vangmin);
	TqFloat y3=m_Radius*sin(RAD(yminang))*cos(vangmax);
	TqFloat y4=m_Radius*sin(RAD(ymaxang))*cos(vangmax);

	CqVector3D vecMin(MIN(MIN(MIN(x1,x2),x3),x4), MIN(MIN(MIN(y1,y2),y3),y4), MIN(m_ZMin,m_ZMax));
	CqVector3D vecMax(MAX(MAX(MAX(x1,x2),x3),x4), MAX(MAX(MAX(y1,y2),y3),y4), MAX(m_ZMin,m_ZMax));

	CqBound	B(vecMin, vecMax);
	B.Transform(m_matTx);
	return(B);
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqSphere::Split(std::vector<CqBasicSurface*>& aSplits)
{
	// Create a NURBS patch
	CqSurfaceNURBS* pNew=new CqSurfaceNURBS();

	TqFloat phimin=(m_ZMin>-m_Radius)?asin(m_ZMin/m_Radius):-(RI_PIO2);
	TqFloat phimax=(m_ZMax< m_Radius)?asin(m_ZMax/m_Radius): (RI_PIO2);
	
	CqSurfaceNURBS Curve;
	CqVector3D vA(0,0,0), vB(1,0,0), vC(0,0,1);
	Curve.Circle(vA,vB,vC,m_Radius,phimin,phimax); 
	pNew->SurfaceOfRevolution(Curve,vA,vC,RAD(m_ThetaMax));

	pNew->Transform(m_matTx, m_matITTx, CqMatrix());

	pNew->u()=u();
	pNew->v()=v();
	pNew->s()=s();
	pNew->t()=t();
	pNew->Cs()=Cs();
	pNew->Os()=Os();
	pNew->SetSurfaceParameters(*this);
	pNew->m_fDiceable=TqTrue;
	pNew->m_EyeSplitCount=m_EyeSplitCount;

	aSplits.push_back(pNew);

	return(1);
}


	
//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqSphere::DicePoint(TqInt u, TqInt v)
{	
	TqFloat phimin=(m_ZMin>-m_Radius)?asin(m_ZMin/m_Radius):RAD(-90.0);
	TqFloat phimax=(m_ZMin<m_Radius)?asin(m_ZMax/m_Radius):RAD(90.0);
	TqFloat phi=phimin+(v*(phimax-phimin))/m_vDiceSize;

	TqFloat cosphi=cos(phi);
	TqFloat theta=RAD(m_ThetaMin+(u*(m_ThetaMax-m_ThetaMin))/m_uDiceSize);
	
	return(CqVector3D((m_Radius*cos(theta)*cosphi),(m_Radius*sin(theta)*cosphi),(m_Radius*sin(phi))));
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqSphere::DicePoint(TqInt u, TqInt v, CqVector3D& Normal)
{	
	CqVector3D	p(DicePoint(u,v));
	Normal=p;
	Normal.Unit();
	
	return(p);
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqCone::CqCone(TqFloat height,TqFloat radius,TqFloat thetamin,TqFloat thetamax,TqFloat zmin,TqFloat zmax) :
	m_Height(height),
	m_Radius(radius),
	m_ZMin(zmin),
	m_ZMax(zmax),
	m_ThetaMin(thetamin),
	m_ThetaMax(thetamax)
{
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqCone&	CqCone::operator=(const CqCone& From)
{
	CqQuadric::operator=(From);
	m_Height=From.m_Height;
	m_Radius=From.m_Radius;
	m_ZMin=From.m_ZMin;
	m_ZMax=From.m_ZMax;
	m_ThetaMin=From.m_ThetaMin;
	m_ThetaMax=From.m_ThetaMax;

	return(*this);
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqCone::Bound() const
{
	TqFloat xminang,yminang,xmaxang,ymaxang;
	xminang=yminang=MIN(m_ThetaMin,m_ThetaMax);
	xmaxang=ymaxang=MAX(m_ThetaMin,m_ThetaMax);

	TqFloat rmin=m_Radius*(1.0-m_ZMin/m_Height);
	TqFloat rmax=m_Radius*(1.0-m_ZMax/m_Height);

	// If start and end in same segement, just use the points.
	if(static_cast<TqInt>(m_ThetaMin/90)!=static_cast<TqInt>(m_ThetaMax/90))
	{
		if(yminang<90 && ymaxang>90)	yminang=90;
		if(yminang<270 && ymaxang>270)	ymaxang=270;
		if(xminang<180 && xmaxang>180)	xmaxang=180;
	}
	
	TqFloat x1=rmin*cos(RAD(xminang));
	TqFloat x2=rmin*cos(RAD(xmaxang));
	TqFloat x3=rmax*cos(RAD(xminang));
	TqFloat x4=rmax*cos(RAD(xmaxang));
	TqFloat y1=rmin*sin(RAD(yminang));
	TqFloat y2=rmin*sin(RAD(ymaxang));
	TqFloat y3=rmax*sin(RAD(yminang));
	TqFloat y4=rmax*sin(RAD(ymaxang));

	CqVector3D vecMin(MIN(MIN(MIN(x1,x2),x3),x4), MIN(MIN(MIN(y1,y2),y3),y4), MIN(m_ZMin,m_ZMax));
	CqVector3D vecMax(MAX(MAX(MAX(x1,x2),x3),x4), MAX(MAX(MAX(y1,y2),y3),y4), MAX(m_ZMin,m_ZMax));

	CqBound	B(vecMin, vecMax);
	B.Transform(m_matTx);
	return(B);
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqCone::Split(std::vector<CqBasicSurface*>& aSplits)
{
	// Create a NURBS patch
	CqSurfaceNURBS* pNew=new CqSurfaceNURBS();

	CqSurfaceNURBS Curve;
	CqVector3D vA(m_Radius,0,0), vB(0,0,m_Height), vC(0,0,0), vD(0,0,1);
	Curve.LineSegment(vA,vB); 
	pNew->SurfaceOfRevolution(Curve,vC,vD,RAD(m_ThetaMax));

	pNew->Transform(m_matTx, m_matITTx, CqMatrix());

	pNew->u()=u();
	pNew->v()=v();
	pNew->s()=s();
	pNew->t()=t();
	pNew->Cs()=Cs();
	pNew->Os()=Os();
	pNew->SetSurfaceParameters(*this);
	pNew->m_fDiceable=TqTrue;
	pNew->m_EyeSplitCount=m_EyeSplitCount;

	aSplits.push_back(pNew);

	return(1);
}


	
//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqCone::DicePoint(TqInt u, TqInt v)
{	
	TqFloat theta=RAD(m_ThetaMin+(u*(m_ThetaMax-m_ThetaMin))/m_uDiceSize);

	TqFloat z=m_ZMin+(v*(m_ZMax-m_ZMin))/m_vDiceSize;
	TqFloat r=m_Radius*(1.0-z/m_Height);
	
	return(CqVector3D(r*cos(theta),r*sin(theta),z));
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqCone::DicePoint(TqInt u, TqInt v, CqVector3D& Normal)
{	
	CqVector3D	p(DicePoint(u,v));
	Normal=p;
	Normal.Unit();
	
	return(p);
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqCylinder::CqCylinder(TqFloat radius,TqFloat zmin,TqFloat zmax,TqFloat thetamin,TqFloat thetamax) :
	m_Radius(radius),
	m_ZMin(zmin),
	m_ZMax(zmax),
	m_ThetaMin(thetamin),
	m_ThetaMax(thetamax)
{
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqCylinder&	CqCylinder::operator=(const CqCylinder& From)
{
	CqQuadric::operator=(From);
	m_Radius=From.m_Radius;
	m_ZMin=From.m_ZMin;
	m_ZMax=From.m_ZMax;
	m_ThetaMin=From.m_ThetaMin;
	m_ThetaMax=From.m_ThetaMax;

	return(*this);
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqCylinder::Bound() const
{
	TqFloat xminang,yminang,xmaxang,ymaxang;
	xminang=yminang=MIN(m_ThetaMin,m_ThetaMax);
	xmaxang=ymaxang=MAX(m_ThetaMin,m_ThetaMax);


	// If start and end in same segement, just use the points.
	if(static_cast<TqInt>(m_ThetaMin/90)!=static_cast<TqInt>(m_ThetaMax/90))
	{
		if(yminang<90 && ymaxang>90)	yminang=90;
		if(yminang<270 && ymaxang>270)	ymaxang=270;
		if(xminang<180 && xmaxang>180)	xmaxang=180;
	}
	
	TqFloat x1=m_Radius*cos(RAD(xminang));
	TqFloat x2=m_Radius*cos(RAD(xmaxang));
	TqFloat y1=m_Radius*sin(RAD(yminang));
	TqFloat y2=m_Radius*sin(RAD(ymaxang));

	CqVector3D vecMin(MIN(x1,x2), MIN(y1,y2), MIN(m_ZMin,m_ZMax));
	CqVector3D vecMax(MAX(x1,x2), MAX(y1,y2), MAX(m_ZMin,m_ZMax));

	CqBound	B(vecMin, vecMax);
	B.Transform(m_matTx);
	return(B);
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqCylinder::Split(std::vector<CqBasicSurface*>& aSplits)
{
	// Create a NURBS patch
	CqSurfaceNURBS* pNew=new CqSurfaceNURBS();

	CqSurfaceNURBS Curve;
	CqVector3D vA(m_Radius,0,m_ZMin), vB(m_Radius,0,m_ZMax), vC(0,0,0), vD(0,0,1);
	Curve.LineSegment(vA,vB); 
	pNew->SurfaceOfRevolution(Curve,vC,vD,RAD(m_ThetaMax));

	pNew->Transform(m_matTx, m_matITTx, CqMatrix());

	pNew->u()=u();
	pNew->v()=v();
	pNew->s()=s();
	pNew->t()=t();
	pNew->Cs()=Cs();
	pNew->Os()=Os();
	pNew->SetSurfaceParameters(*this);
	pNew->m_fDiceable=TqTrue;
	pNew->m_EyeSplitCount=m_EyeSplitCount;

	aSplits.push_back(pNew);

	return(1);
}


	
//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqCylinder::DicePoint(TqInt u, TqInt v)
{	
	TqFloat theta=RAD(m_ThetaMin+((m_ThetaMax-m_ThetaMin)*u)/m_uDiceSize);

	TqFloat vz=m_ZMin+(v*(m_ZMax-m_ZMin))/m_vDiceSize;
	return(CqVector3D(m_Radius*cos(theta),m_Radius*sin(theta),vz));
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqCylinder::DicePoint(TqInt u, TqInt v, CqVector3D& Normal)
{	
	CqVector3D	p(DicePoint(u,v));
	Normal=p;
	Normal.z(0);
	Normal.Unit();
	
	return(p);
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqHyperboloid::CqHyperboloid(CqVector3D& point1,CqVector3D& point2,TqFloat thetamin,TqFloat thetamax) :
	m_Point1(point1),
	m_Point2(point2),
	m_ThetaMin(thetamin),
	m_ThetaMax(thetamax)
{
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqHyperboloid&	CqHyperboloid::operator=(const CqHyperboloid& From)
{
	CqQuadric::operator=(From);
	m_Point1=From.m_Point1;
	m_Point2=From.m_Point2;
	m_ThetaMin=From.m_ThetaMin;
	m_ThetaMax=From.m_ThetaMax;

	return(*this);
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqHyperboloid::Bound() const
{
	CqVector3D p1(m_Point1);
	p1.z(0);
	TqFloat r1=p1.Magnitude();
	CqVector3D p2(m_Point2);
	p2.z(0);
	TqFloat r2=p2.Magnitude();
	r1=MAX(r1,r2);
	
	CqVector3D vecMin(-r1, -r1, MIN(m_Point1.z(),m_Point2.z()));
	CqVector3D vecMax( r1,  r1, MAX(m_Point1.z(),m_Point2.z()));

	CqBound	B(vecMin, vecMax);
	B.Transform(m_matTx);
	return(B);
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqHyperboloid::Split(std::vector<CqBasicSurface*>& aSplits)
{
	// Create a NURBS patch
	CqSurfaceNURBS* pNew=new CqSurfaceNURBS();

	CqSurfaceNURBS Curve;
	Curve.LineSegment(m_Point1,m_Point2); 
	CqVector3D vA(0,0,0), vB(0,0,1);
	pNew->SurfaceOfRevolution(Curve,vA,vB,RAD(m_ThetaMax));

	pNew->Transform(m_matTx, m_matITTx, CqMatrix());

	pNew->u()=u();
	pNew->v()=v();
	pNew->s()=s();
	pNew->t()=t();
	pNew->Cs()=Cs();
	pNew->Os()=Os();
	pNew->SetSurfaceParameters(*this);
	pNew->m_fDiceable=TqTrue;
	pNew->m_EyeSplitCount=m_EyeSplitCount;

	aSplits.push_back(pNew);

	return(1);
}


	
//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqHyperboloid::DicePoint(TqInt u, TqInt v)
{	
	TqFloat theta=RAD(m_ThetaMin+(u*(m_ThetaMax-m_ThetaMin))/m_uDiceSize);

	CqVector3D p;
	TqFloat vv=static_cast<TqFloat>(v)/m_vDiceSize;
	p=m_Point1*(1.0-vv)+m_Point2*vv;
	
	return(CqVector3D(p.x()*cos(theta)-p.y()*sin(theta),p.x()*sin(theta)+p.y()*cos(theta),p.z()));
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqHyperboloid::DicePoint(TqInt u, TqInt v, CqVector3D& Normal)
{	
	CqVector3D	p(DicePoint(u,v));
	Normal=p;
	Normal.z(0);
	Normal.Unit();
	
	return(p);
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqParaboloid::CqParaboloid(TqFloat rmax,TqFloat zmin,TqFloat zmax,TqFloat thetamin,TqFloat thetamax) :
	m_RMax(rmax),
	m_ZMin(zmin),
	m_ZMax(zmax),
	m_ThetaMin(thetamin),
	m_ThetaMax(thetamax)
{
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqParaboloid&	CqParaboloid::operator=(const CqParaboloid& From)
{
	CqQuadric::operator=(From);
	m_RMax=From.m_RMax;
	m_ZMin=From.m_ZMin;
	m_ZMax=From.m_ZMax;
	m_ThetaMin=From.m_ThetaMin;
	m_ThetaMax=From.m_ThetaMax;

	return(*this);
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqParaboloid::Bound() const
{
/*	TqFloat xminang,yminang,xmaxang,ymaxang;
	xminang=yminang=MIN(m_ThetaMin,m_ThetaMax);
	xmaxang=ymaxang=MAX(m_ThetaMin,m_ThetaMax);


	// If start and end in same segement, just use the points.
	if(static_cast<TqInt>(m_ThetaMin/90)!=static_cast<TqInt>(m_ThetaMax/90))
	{
		if(yminang<90 && ymaxang>90)	yminang=90;
		if(yminang<270 && ymaxang>270)	ymaxang=270;
		if(xminang<180 && xmaxang>180)	xmaxang=180;
	}*/
	
	TqFloat x1=m_RMax*cos(RAD(0));
	TqFloat x2=m_RMax*cos(RAD(180));
	TqFloat y1=m_RMax*sin(RAD(90));
	TqFloat y2=m_RMax*sin(RAD(270));

	CqVector3D vecMin(MIN(x1,x2), MIN(y1,y2), MIN(m_ZMin,m_ZMax));
	CqVector3D vecMax(MAX(x1,x2), MAX(y1,y2), MAX(m_ZMin,m_ZMax));

	CqBound	B(vecMin, vecMax);
	B.Transform(m_matTx);
	return(B);
}


//---------------------------------------------------------------------
/** Split this GPrim into smaller quadrics.
 */

TqInt CqParaboloid::Split(std::vector<CqBasicSurface*>& aSplits)
{
	TqFloat zcent=(m_ZMin+m_ZMax)*0.5;
	TqFloat arccent=(m_ThetaMin+m_ThetaMax)*0.5;
	TqFloat rcent=m_RMax*sqrt(zcent/m_ZMax);

	CqParaboloid* pNew1=new CqParaboloid(*this);
	CqParaboloid* pNew2=new CqParaboloid(*this);
	if(m_uDiceSize>m_vDiceSize)
	{
		pNew1->m_ThetaMax=arccent;
		pNew2->m_ThetaMin=arccent;
		// Subdivide the parameter values
		pNew1->u().uSubdivide(&pNew2->u());
		pNew1->v().uSubdivide(&pNew2->v());
		pNew1->s().uSubdivide(&pNew2->s());
		pNew1->t().uSubdivide(&pNew2->t());
		pNew1->Cs().uSubdivide(&pNew2->Cs());
		pNew1->Os().uSubdivide(&pNew2->Os());
	}
	else
	{
		pNew1->m_ZMax=zcent;
		pNew1->m_RMax=rcent;
		pNew2->m_ZMin=zcent;
		// Subdivide the parameter values
		pNew1->u().vSubdivide(&pNew2->u());
		pNew1->v().vSubdivide(&pNew2->v());
		pNew1->s().vSubdivide(&pNew2->s());
		pNew1->t().vSubdivide(&pNew2->t());
		pNew1->Cs().vSubdivide(&pNew2->Cs());
		pNew1->Os().vSubdivide(&pNew2->Os());
	}
	pNew1->SetSurfaceParameters(*this);
	pNew2->SetSurfaceParameters(*this);
	pNew1->m_fDiceable=TqTrue;
	pNew2->m_fDiceable=TqTrue;
	pNew1->m_EyeSplitCount=m_EyeSplitCount;
	pNew2->m_EyeSplitCount=m_EyeSplitCount;

	aSplits.push_back(pNew1);
	aSplits.push_back(pNew2);

	return(2);
}


	
//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqParaboloid::DicePoint(TqInt u, TqInt v)
{	
	TqFloat theta=RAD(m_ThetaMin+((m_ThetaMax-m_ThetaMin)*u)/m_uDiceSize);

	TqFloat z=m_ZMin+(v*(m_ZMax-m_ZMin))/m_vDiceSize;
	TqFloat r=m_RMax*sqrt(z/m_ZMax);
	return(CqVector3D(r*cos(theta),r*sin(theta),z));
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqParaboloid::DicePoint(TqInt u, TqInt v, CqVector3D& Normal)
{	
	CqVector3D	p(DicePoint(u,v));
	Normal=p;
	Normal.z(0);
	Normal.Unit();
	
	return(p);
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqTorus::CqTorus(TqFloat majorradius,TqFloat minorradius,TqFloat phimin,TqFloat phimax,TqFloat thetamin,TqFloat thetamax) :
	m_MajorRadius(majorradius),
	m_MinorRadius(minorradius),
	m_PhiMin(phimin),
	m_PhiMax(phimax),
	m_ThetaMin(thetamin),
	m_ThetaMax(thetamax)
{
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqTorus&	CqTorus::operator=(const CqTorus& From)
{
	CqQuadric::operator=(From);
	m_MajorRadius=From.m_MajorRadius;
	m_MinorRadius=From.m_MinorRadius;
	m_PhiMax=From.m_PhiMax;
	m_PhiMin=From.m_PhiMin;
	m_ThetaMin=From.m_ThetaMin;
	m_ThetaMax=From.m_ThetaMax;

	return(*this);
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqTorus::Bound() const
{
	TqFloat xminang,yminang,xmaxang,ymaxang;
	xminang=yminang=MIN(m_ThetaMin,m_ThetaMax);
	xmaxang=ymaxang=MAX(m_ThetaMin,m_ThetaMax);


	// If start and end in same segement, just use the points.
	if(static_cast<TqInt>(m_ThetaMin/90)!=static_cast<TqInt>(m_ThetaMax/90))
	{
		if(yminang<90 && ymaxang>90)	yminang=90;
		if(yminang<270 && ymaxang>270)	ymaxang=270;
		if(xminang<180 && xmaxang>180)	xmaxang=180;
	}

	TqFloat r=m_MajorRadius+m_MinorRadius;
	
	TqFloat x1=r*cos(RAD(0));
	TqFloat x2=r*cos(RAD(180));
	TqFloat y1=r*sin(RAD(90));
	TqFloat y2=r*sin(RAD(270));

	CqVector3D vecMin(MIN(x1,x2), MIN(y1,y2), -m_MinorRadius);
	CqVector3D vecMax(MAX(x1,x2), MAX(y1,y2),  m_MinorRadius);

	CqBound	B(vecMin, vecMax);
	B.Transform(m_matTx);
	return(B);
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqTorus::Split(std::vector<CqBasicSurface*>& aSplits)
{
	// Create a NURBS patch
	CqSurfaceNURBS* pNew=new CqSurfaceNURBS();

	CqSurfaceNURBS Curve;
	CqVector3D vA(m_MajorRadius,0,0), vB(1,0,0), vC(0,0,1), vD(0,0,0);
	Curve.Circle(vA,vB,vC,m_MinorRadius,RAD(m_PhiMin),RAD(m_PhiMax)); 
	pNew->SurfaceOfRevolution(Curve,vD,vC,RAD(m_ThetaMax));

	pNew->Transform(m_matTx, m_matITTx, CqMatrix());

	pNew->u()=u();
	pNew->v()=v();
	pNew->s()=s();
	pNew->t()=t();
	pNew->Cs()=Cs();
	pNew->Os()=Os();
	pNew->SetSurfaceParameters(*this);
	pNew->m_fDiceable=TqTrue;
	pNew->m_EyeSplitCount=m_EyeSplitCount;

	aSplits.push_back(pNew);

	return(1);
}


	
//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqTorus::DicePoint(TqInt u, TqInt v)
{	
	TqFloat theta=RAD(m_ThetaMin+(u*(m_ThetaMax-m_ThetaMin))/m_uDiceSize);
	TqFloat phi=RAD(m_PhiMin+(v*(m_PhiMax-m_PhiMin))/m_vDiceSize);

	TqFloat r=m_MinorRadius*cos(phi);
	TqFloat z=m_MinorRadius*sin(phi);
	return(CqVector3D((m_MajorRadius+r)*cos(theta),(m_MajorRadius+r)*sin(theta),z));
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqTorus::DicePoint(TqInt u, TqInt v, CqVector3D& Normal)
{	
	CqVector3D	p(DicePoint(u,v));
	Normal=p;
	Normal.z(0);
	Normal.Unit();
	
	return(p);
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqDisk::CqDisk(TqFloat height,TqFloat minorradius,TqFloat majorradius,TqFloat thetamin,TqFloat thetamax) :
	m_Height(height),
	m_MajorRadius(majorradius),
	m_MinorRadius(minorradius),
	m_ThetaMin(thetamin),
	m_ThetaMax(thetamax)
{
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqDisk&	CqDisk::operator=(const CqDisk& From)
{
	CqQuadric::operator=(From);
	m_Height=From.m_Height;
	m_MajorRadius=From.m_MajorRadius;
	m_MinorRadius=From.m_MinorRadius;
	m_ThetaMin=From.m_ThetaMin;
	m_ThetaMax=From.m_ThetaMax;

	return(*this);
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqDisk::Bound() const
{
/*	TqFloat xminang,yminang,xmaxang,ymaxang;
	xminang=yminang=MIN(m_ThetaMin,m_ThetaMax);
	xmaxang=ymaxang=MAX(m_ThetaMin,m_ThetaMax);


	// If start and end in same segement, just use the points.
	if(static_cast<TqInt>(m_ThetaMin/90)!=static_cast<TqInt>(m_ThetaMax/90))
	{
		if(yminang<90 && ymaxang>90)	yminang=90;
		if(yminang<270 && ymaxang>270)	ymaxang=270;
		if(xminang<180 && xmaxang>180)	xmaxang=180;
	}
*/
	TqFloat x1=m_MajorRadius*cos(RAD(0));
	TqFloat x2=m_MajorRadius*cos(RAD(180));
	TqFloat y1=m_MajorRadius*sin(RAD(90));
	TqFloat y2=m_MajorRadius*sin(RAD(270));

	CqVector3D vecMin(MIN(x1,x2), MIN(y1,y2), m_Height);
	CqVector3D vecMax(MAX(x1,x2), MAX(y1,y2), m_Height);

	CqBound	B(vecMin, vecMax);
	B.Transform(m_matTx);
	return(B);
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqDisk::Split(std::vector<CqBasicSurface*>& aSplits)
{
	// Create a NURBS patch
	CqSurfaceNURBS* pNew=new CqSurfaceNURBS();

	CqSurfaceNURBS Curve;
	CqVector3D vA(m_MajorRadius,0,m_Height), vB(0,0,m_Height), vC(0,0,0), vD(0,0,1);
	Curve.LineSegment(vA,vB); 
	pNew->SurfaceOfRevolution(Curve,vC,vD,RAD(m_ThetaMax));

	pNew->Transform(m_matTx, m_matITTx, CqMatrix());

	pNew->u()=u();
	pNew->v()=v();
	pNew->s()=s();
	pNew->t()=t();
	pNew->Cs()=Cs();
	pNew->Os()=Os();
	pNew->SetSurfaceParameters(*this);
	pNew->m_fDiceable=TqTrue;
	pNew->m_EyeSplitCount=m_EyeSplitCount;

	aSplits.push_back(pNew);

	return(1);
}


	
//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqDisk::DicePoint(TqInt u, TqInt v)
{	
	TqFloat theta=RAD(m_ThetaMin+(u*(m_ThetaMax-m_ThetaMin))/m_uDiceSize);
	TqFloat vv=m_MajorRadius-(v*(m_MajorRadius-m_MinorRadius))/m_vDiceSize;
	return(CqVector3D(vv*cos(theta),vv*sin(theta),m_Height));
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqDisk::DicePoint(TqInt u, TqInt v, CqVector3D& Normal)
{	
	CqVector3D	p(DicePoint(u,v));
	Normal=CqVector3D(0,0,1);
	
	return(p);
}

END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
