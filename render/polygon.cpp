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
		\brief Implements the classes and support structures for handling polygons.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"polygon.h"
#include	"patch.h"
#include	"imagebuffer.h"

START_NAMESPACE(Aqsis)


//---------------------------------------------------------------------
/** Return the boundary extents in camera space of the polygon
 */

CqBound CqPolygonBase::Bound() const
{
	CqVector3D	vecA(FLT_MAX, FLT_MAX, FLT_MAX);
	CqVector3D	vecB(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	TqInt i;
	for(i=0; i<NumVertices(); i++)
	{
		CqVector3D	vecV=PolyP(i);
		if(vecV.x()<vecA.x())	vecA.x(vecV.x());
		if(vecV.y()<vecA.y())	vecA.y(vecV.y());
		if(vecV.x()>vecB.x())	vecB.x(vecV.x());
		if(vecV.y()>vecB.y())	vecB.y(vecV.y());
		if(vecV.z()<vecA.z())	vecA.z(vecV.z());
		if(vecV.z()>vecB.z())	vecB.z(vecV.z());
	}
	CqBound	B;
	B.vecMin()=vecA;
	B.vecMax()=vecB;
	return(B);
}


//---------------------------------------------------------------------
/** Determine whether the polygon can be diced or not.
 */

TqBool CqPolygonBase::Diceable()
{
	// Get the bound in camera space.
/*	CqBound B=Bound();
	// Convert it to screen space
	B.Transform(pCurrentRenderer()->matSpaceToSpace("camera","raster"));

	// Get the appropriate shading rate.
	float ShadingRate=pAttributes()->fEffectiveShadingRate();
//	if(pCurrentRenderer()->Mode()==RenderMode_Shadows)
//	{
//		const TqFloat* pattrShadowShadingRate=pAttributes()->GetFloatAttribute("render","shadow_shadingrate");
//		if(pattrShadowShadingRate!=0)
//			ShadingRate=pattrShadowShadingRate[0];
//	}	

	ShadingRate=static_cast<float>(sqrt(ShadingRate));

	// Calculate the screen area to decide whether to dice or not.
	TqFloat sx=(B.vecMax().x()-B.vecMin().x())/ShadingRate;
	TqFloat sy=(B.vecMax().y()-B.vecMin().y())/ShadingRate;
	TqFloat area=sx*sy;

	if(area>256)	return(TqFalse);
	else			return(TqTrue);*/
	return(TqFalse);
}


//---------------------------------------------------------------------
/** Dice the polygon into a grid of MPGs for rendering.
 */

CqMicroPolyGridBase* CqPolygonBase::Dice()
{
	return(0);
/*	TqInt	iA,iB,iC,iD;
	TqFloat sA,tA,sB,tB,sC,tC,sD,tD;
	TqFloat uA,vA,uB,vB,uC,vC,uD,vD;

	const CqMatrix& matObjToRaster=pCurrentRenderer()->matSpaceToSpace("camera","raster");

	TqBool	bhasN=bHasNormals();
	TqBool	bhass=bHass();
	TqBool	bhast=bHast();
	TqBool	bhasu=bHasu();
	TqBool	bhasv=bHasv();
	TqBool	bhasCs=bHasCs();
	TqBool	bhasOs=bHasOs();

	TqInt iUses=PolyUses();

	// Get the appropriate shading rate.
	float ShadingRate=pAttributes()->fEffectiveShadingRate();
//	if(pCurrentRenderer()->Mode()==RenderMode_Shadows)
//	{
//		const TqFloat* pattrShadowShadingRate=pAttributes()->GetFloatAttribute("render","shadow_shadingrate");
//		if(pattrShadowShadingRate!=0)
//			ShadingRate=pattrShadowShadingRate[0];
//	}	

	ShadingRate=static_cast<float>(sqrt(ShadingRate));

	// Get the first two vertices
	iA=0;
	iB=1;

	TqInt i=2;
	while(i<NumVertices())
	{
		iC=iD=i;
		i++;
		if(i<NumVertices())
		{
			iD=i;
			i++;
		}
		// Now we have the four indices, do a basic bilinear interpolation of the values.
		// First of all get the screen space coordinates to caluclate an approximate u,v dicing level,
		CqVector3D PA=PolyP(iA);
		CqVector3D PB=PolyP(iB);
		CqVector3D PC=PolyP(iC);
		CqVector3D PD=PolyP(iD);

		if(bhass)
		{
			sA=Polys(iA);
			sB=Polys(iB);
			sC=Polys(iC);
			sD=Polys(iD);
		}

		if(bhast)
		{
			tA=Polyt(iA);
			tB=Polyt(iB);
			tC=Polyt(iC);
			tD=Polyt(iD);
		}

		if(bhasu)
		{
			uA=Polyu(iA);
			uB=Polyu(iB);
			uC=Polyu(iC);
			uD=Polyu(iD);
		}

		if(bhasv)
		{
			vA=Polyv(iA);
			vB=Polyv(iB);
			vC=Polyv(iC);
			vD=Polyv(iD);
		}

		PA=matObjToRaster*PA;
		PB=matObjToRaster*PB;
		PC=matObjToRaster*PC;
		PD=matObjToRaster*PD;

		TqFloat ab=CqVector3D(PB-PA).Magnitude();
		TqFloat bc=CqVector3D(PC-PB).Magnitude();
		TqFloat cd=CqVector3D(PD-PC).Magnitude();
		TqFloat da=CqVector3D(PA-PD).Magnitude();
		TqFloat uLen=MAX(ab,cd);
		TqFloat vLen=MAX(ab,cd);

		// Make sure to take into account the appropriate shading rate when calculating the dice size.
		uLen=uLen/ShadingRate;
		vLen=vLen/ShadingRate;

		TqInt ucount=static_cast<TqInt>(MAX(1,uLen));
		TqInt vcount=static_cast<TqInt>(MAX(1,vLen));

		TqFloat diu=1.0f/ucount;
		TqFloat div=1.0f/vcount;
		TqInt iu;
		TqInt iv;

		CqMicroPolyGrid* pGrid=new CqMicroPolyGrid(ucount, vcount, &Surface());

		if(bhasN)	pGrid->SetNormals(TqTrue);

		// Need to copy a single Cq & Os if used, but not defined.
		if(USES(iUses,EnvVars_Cs) && !bhasCs)	pGrid->Cs().SetValue(pAttributes()->colColor());
		if(USES(iUses,EnvVars_Os) && !bhasOs)	pGrid->Os().SetValue(pAttributes()->colOpacity());

		pGrid->Reset();
		for(iv=0; iv<=vcount; iv++)
		{
			for(iu=0; iu<=ucount; iu++)
			{
				pGrid->P()=BilinearEvaluate<CqVector4D>(PolyP(iA),PolyP(iB),PolyP(iD),PolyP(iC),iu*diu,iv*div);
				if(bhasN && USES(iUses,EnvVars_N))	pGrid->N()=BilinearEvaluate<CqVector3D>(PolyN(iA),PolyN(iB),PolyN(iD),PolyN(iC),iu*diu,iv*div);
				if(USES(iUses,EnvVars_s))
					pGrid->s()=BilinearEvaluate<TqFloat>(sA,sB,sD,sC,iu*diu,iv*div);

				if(USES(iUses,EnvVars_t))
					pGrid->t()=BilinearEvaluate<TqFloat>(tA,tB,tD,tC,iu*diu,iv*div);

				if(USES(iUses,EnvVars_u))
					pGrid->u()=BilinearEvaluate<TqFloat>(uA,uB,uD,uC,iu*diu,iv*div);
				
				if(USES(iUses,EnvVars_v))
					pGrid->v()=BilinearEvaluate<TqFloat>(vA,vB,vD,vC,iu*diu,iv*div);

				if(bhasCs && USES(iUses,EnvVars_Cs))	pGrid->Cs()=BilinearEvaluate<CqColor>(PolyCs(iA),PolyCs(iB),PolyCs(iD),PolyCs(iC),iu*diu,iv*div);
				if(bhasOs && USES(iUses,EnvVars_Os))	pGrid->Os()=BilinearEvaluate<CqColor>(PolyOs(iA),PolyOs(iB),PolyOs(iD),PolyOs(iC),iu*diu,iv*div);
				pGrid->Advance();
			}
		}
		// Only shade if the ImageBuffer mode is at least RGB
		if(pCurrentRenderer()->optCurrent().iDisplayMode()&ModeRGB)
			pGrid->Shade();

		pGrid->Split(pCurrentRenderer()->pImage());
	}*/
}


//---------------------------------------------------------------------
/** Split the polygon into bilinear patches.
 */

TqInt CqPolygonBase::Split(std::vector<CqBasicSurface*>& aSplits)
{
	CqVector3D	vecA,vecB,vecC,vecD;
	CqVector3D	vecNA,vecNB,vecNC,vecND;

	TqFloat		uA,uB,uC,uD,vA,vB,vC,vD;
	TqFloat		sA,sB,sC,sD,tA,tB,tC,tD;

	CqColor	colA,colB,colC,colD;
	CqColor	opaA,opaB,opaC,opaD;

	TqBool	bhasN=bHasNormals();
	TqBool	bhass=bHass();
	TqBool	bhast=bHast();
	TqBool	bhasu=bHasu();
	TqBool	bhasv=bHasv();
	TqBool	bhasCs=bHasCs();
	TqBool	bhasOs=bHasOs();

	TqInt iUses=PolyUses();

	// We need to take into account Orientation here, even though most other 
	// primitives leave it up to the CalcNormals function on the MPGrid, because we
	// are forcing N to be setup here, so clockwise nature is important.
	EqOrientation O=pAttributes()->eOrientation();
	float neg=1;
	if(O!=OrientationLH)	neg=-1;

	// Start by splitting the polygon into 4 point patches.
	vecA=PolyP(0);
	vecB=PolyP(1);

	// Get the normals, or calculate the facet normal if not specified.
	if(bhasN)
	{
		vecNA=PolyN(0);
		vecNB=PolyN(1);
	}
	else
	{
		// Find two suitable vectors, and produce a geometric normal to use.
		TqInt i=1;
		CqVector3D	vecN0, vecN1;
		while(i<NumVertices())
		{
			vecN0=static_cast<CqVector3D>(PolyP(i))-vecA;
			if(vecN0.Magnitude()>FLT_EPSILON)
				break;
			i++;
		}
		i++;
		while(i<NumVertices())
		{
			vecN1=static_cast<CqVector3D>(PolyP(i))-vecA;
			if(vecN1.Magnitude()>FLT_EPSILON && vecN1!=vecN0)
				break;
			i++;
		}
		vecNA=vecN0%vecN1;
		vecNA*=neg;
		vecNA.Unit();
		vecNB=vecNC=vecND=vecNA;
	}
	
	
	if(bhasu)
	{
		uA=Polyu(0);
		uB=Polyu(1);
	}

	if(bhasv)
	{
		vA=Polyv(0);
		vB=Polyv(1);
	}

	// Get the texture coordinates, or use the parameter space values if not specified.
	if(bhass)
	{
		sA=Polys(0);
		sB=Polys(1);
	}

	if(bhast)
	{
		tA=Polyt(0);
		tB=Polyt(1);
	}

	// Get any specified per vertex colors and opacities.
	if(bhasCs)
	{
		colA=PolyCs(0);
		colB=PolyCs(1);
	}
	else
		colA=pAttributes()->colColor();

	if(bhasOs)
	{
		opaA=PolyOs(0);
		opaB=PolyOs(1);
	}
	else
		opaA=pAttributes()->colOpacity();

	TqInt cNew=0;
	TqInt i;
	for(i=2; i<NumVertices(); i+=2)
	{
		vecC=vecD=PolyP(i);
		if(NumVertices()>i+1)	vecD=PolyP(i+1);

		if(bhasu)
		{
			uC=uD=Polyu(i);
			if(NumVertices()>i+1)	uD=Polyu(i+1);
		}

		if(bhasv)
		{
			vC=vD=Polyv(i);
			if(NumVertices()>i+1)	vD=Polyv(i+1);
		}

		if(bhasN)
		{
			vecNC=vecND=PolyN(i);
			if(NumVertices()>i+1)	vecND=PolyN(i+1);
		}
		else
			vecNC=vecND=vecNA;
		
		if(bhass)
		{
			sC=sD=Polys(i);
			if(NumVertices()>i+1)	sD=Polys(i+1);
		}

		if(bhast)
		{
			tC=tD=Polyt(i);
			if(NumVertices()>i+1)	tD=Polyt(i+1);
		}

		// Get any specified per vertex colors and opacities.
		if(bhasCs)
		{
			colC=colD=PolyCs(i);
			if(NumVertices()>i+1)	colD=PolyCs(i+1);
		}

		if(bhasOs)
		{
			opaC=opaD=PolyOs(i);
			if(NumVertices()>i+1)	opaD=PolyOs(i+1);
		}

		// Create bilinear patches
		CqSurfacePatchBilinear* pNew=new CqSurfacePatchBilinear();
		pNew->SetSurfaceParameters(Surface());
		pNew->SetDefaultPrimitiveVariables();

		pNew->P().SetSize(4); pNew->N().SetSize(4); 
		pNew->P()[0]=vecA; pNew->P()[1]=vecB; pNew->P()[2]=vecD;	pNew->P()[3]=vecC;
		pNew->N()[0]=vecNA; pNew->N()[1]=vecNB; pNew->N()[2]=vecND; pNew->N()[3]=vecNC;
		if(USES(iUses,EnvVars_u))	{pNew->u()[0]=uA;	pNew->u()[1]=uB; pNew->u()[2]=uD; pNew->u()[3]=uC;}
		if(USES(iUses,EnvVars_v))	{pNew->v()[0]=vA;	pNew->v()[1]=vB; pNew->v()[2]=vD; pNew->v()[3]=vC;}
		if(USES(iUses,EnvVars_s))	{pNew->s()[0]=sA;	pNew->s()[1]=sB; pNew->s()[2]=sD; pNew->s()[3]=sC;}
		if(USES(iUses,EnvVars_t))	{pNew->t()[0]=tA;	pNew->t()[1]=tB; pNew->t()[2]=tD; pNew->t()[3]=tC;}
		if(USES(iUses,EnvVars_Cs))
		{
			if(bhasCs)
			{
				pNew->Cs().SetSize(4);
				pNew->Cs()[0]=colA;
				pNew->Cs()[1]=colB;
				pNew->Cs()[2]=colD;
				pNew->Cs()[3]=colC;
			}
			else
				pNew->Cs()[0]=colA;
		}
		if(USES(iUses,EnvVars_Os))
		{
			if(bhasOs)
			{
				pNew->Os().SetSize(4);
				pNew->Os()[0]=opaA;
				pNew->Os()[1]=opaB;
				pNew->Os()[2]=opaD;
				pNew->Os()[3]=opaC;
			}
			else
				pNew->Os()[0]=opaA;
		}

		aSplits.push_back(pNew);
		cNew++;

		// Move onto the next quad
		vecB=vecD;
		vecNB=vecND;
		uB=uD; vB=vD;
		sB=sD; tB=tD;
		colB=colD;
		opaB=opaD;
	}
	return(cNew);
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqSurfacePolygon::CqSurfacePolygon(TqInt cVertices) :	CqSurface(),
														m_cVertices(cVertices)
{
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqSurfacePolygon::CqSurfacePolygon(const CqSurfacePolygon& From)
{
	*this=From;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqSurfacePolygon::~CqSurfacePolygon()
{
}


//---------------------------------------------------------------------
/** Check if a polygon is degenerate, i.e. all points collapse to the same or almost the same place.
 */

TqBool CqSurfacePolygon::CheckDegenerate() const
{
	// Check if all points are within a minute distance of each other.
	TqBool	fDegen=TqTrue;
	TqInt i;
	for(i=1; i<NumVertices(); i++)
	{
		if((PolyP(i)-PolyP(i-1)).Magnitude()>FLT_EPSILON)
			fDegen=TqFalse;
	}
	return(fDegen);
}

//---------------------------------------------------------------------
/** Assignment operator.
 */

CqSurfacePolygon& CqSurfacePolygon::operator=(const CqSurfacePolygon& From)
{
	CqSurface::operator=(From);
	return(*this);
}



//---------------------------------------------------------------------
/** Transform the polygon by the specified matrix.
 */

void	CqSurfacePolygon::Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx)
{
	// Tansform the points by the specified matrix.
	TqInt i;
	for(i=P().Size()-1; i>=0; i--)
		P()[i]=matTx*P()[i];

	// Tansform the normals by the specified matrix.
	for(i=N().Size()-1; i>=0; i--)
		N()[i]=matITTx*N()[i];
}


//---------------------------------------------------------------------
/** Transfer x,y points to s,t and u,v if required.
 */

void CqSurfacePolygon::TransferDefaultSurfaceParameters()
{
	// If the shader needs s/t or u/v, and s/t is not specified, then at this point store the object space x,y coordinates.
	TqInt i;
	TqInt iUses=PolyUses();
	if(USES(iUses,EnvVars_s) && !bHass())
	{
		s().SetSize(NumVertices());
		for(i=0; i<NumVertices(); i++)
			s()[i]=P()[i].x();
	}

	if(USES(iUses,EnvVars_t) && !bHast())
	{
		t().SetSize(NumVertices());
		for(i=0; i<NumVertices(); i++)
			t()[i]=P()[i].y();
	}

	if(USES(iUses,EnvVars_u))
	{
		u().SetSize(NumVertices());
		for(i=0; i<NumVertices(); i++)
			u()[i]=P()[i].x();
	}

	if(USES(iUses,EnvVars_v))
	{
		v().SetSize(NumVertices());
		for(i=0; i<NumVertices(); i++)
			v()[i]=P()[i].y();
	}
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqSurfacePointsPolygon::CqSurfacePointsPolygon(const CqSurfacePointsPolygon& From)
{
	*this=From;
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqSurfacePointsPolygon& CqSurfacePointsPolygon::operator=(const CqSurfacePointsPolygon& From)	
{
	TqInt i;
	for(i=From.m_aIndices.size()-1; i>=0; i--)
		m_aIndices[i]=From.m_aIndices[i];

	// Store the old points array pointer, as we must reference first then
	// unreference to avoid accidental deletion if they are the same and we are the
	// last reference.
	CqPolygonPoints*	pOldPoints=m_pPoints;
	m_pPoints=From.m_pPoints;
	m_pPoints->Reference();
	if(pOldPoints)		pOldPoints->UnReference();	

	return(*this);
}




//---------------------------------------------------------------------
/** Transform the points by the specified matrix.
 */

void	CqPolygonPoints::Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx)
{
	if(m_Transformed)	return;
	
	// Tansform the points by the specified matrix.
	TqInt i;
	for(i=P().Size()-1; i>=0; i--)
		P()[i]=matTx*P()[i];

	// Tansform the normals by the specified matrix.
	for(i=N().Size()-1; i>=0; i--)
		N()[i]=matITTx*N()[i];

	m_Transformed=TqTrue;
}


//---------------------------------------------------------------------
/** Transfer x,y points to s,t and u,v if required.
 */

void CqPolygonPoints::TransferDefaultSurfaceParameters()
{
	// If the shader needs s/t or u/v, and s/t is not specified, then at this point store the object space x,y coordinates.
	TqInt i;
	TqInt iUses=Uses();
	if(USES(iUses,EnvVars_s) && !bHass())
	{
		s().SetSize(NumVertices());
		for(i=0; i<NumVertices(); i++)
			s()[i]=P()[i].x();
	}

	if(USES(iUses,EnvVars_t) && !bHast())
	{
		t().SetSize(NumVertices());
		for(i=0; i<NumVertices(); i++)
			t()[i]=P()[i].y();
	}

	if(USES(iUses,EnvVars_u))
	{
		u().SetSize(NumVertices());
		for(i=0; i<NumVertices(); i++)
			u()[i]=P()[i].x();
	}

	if(USES(iUses,EnvVars_v))
	{
		v().SetSize(NumVertices());
		for(i=0; i<NumVertices(); i++)
			v()[i]=P()[i].y();
	}
}

END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
