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
		\brief Implements the classes and support structures for handling RenderMan patch primitives.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"imagebuffer.h"
#include	"micropolygon.h"
#include	"irenderer.h"
#include	"patch.h"
#include	"vector2d.h"

START_NAMESPACE(Aqsis)


//---------------------------------------------------------------------
/** Constructor both u and vbasis matrices default to bezier.
 */

CqSurfacePatchBicubic::CqSurfacePatchBicubic() : CqSurface()
{
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqSurfacePatchBicubic::CqSurfacePatchBicubic(const CqSurfacePatchBicubic& From) :
							CqSurface(From)
{
	*this=From;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqSurfacePatchBicubic::~CqSurfacePatchBicubic()
{
}


//---------------------------------------------------------------------
/** Evaluate a bicubic spline patch at the specified intervals.
 * \param s Float interval.
 * \param t Float interval.
 * \return CqVector4D surface position.
 */

CqVector4D CqSurfacePatchBicubic::Evaluate(TqFloat s, TqFloat t) const
{
	// Set up the geometry vector.
	CqMatrix	Gx;
	CqMatrix	Gy;
	CqMatrix	Gz;

	GetGeometryMatrices(s,t,Gx,Gy,Gz);

	return(EvaluateMatrix(s,t,Gx,Gy,Gz));
}

//---------------------------------------------------------------------
/** Build the geometry matrices Gx,Gy,Gz for the patch.
 */

void CqSurfacePatchBicubic::GetGeometryMatrices(TqFloat& s, TqFloat &t,CqMatrix& Gx, CqMatrix& Gy, CqMatrix& Gz) const
{
	const CqVector4D&	P11=P()[0];
	const CqVector4D&	P21=P()[1];
	const CqVector4D&	P31=P()[2];
	const CqVector4D&	P41=P()[3];
	const CqVector4D&	P12=P()[4];
	const CqVector4D&	P22=P()[5];
	const CqVector4D&	P32=P()[6];
	const CqVector4D&	P42=P()[7];
	const CqVector4D&	P13=P()[8];
	const CqVector4D&	P23=P()[9];
	const CqVector4D&	P33=P()[10];
	const CqVector4D&	P43=P()[11];
	const CqVector4D&	P14=P()[12];
	const CqVector4D&	P24=P()[13];
	const CqVector4D&	P34=P()[14];
	const CqVector4D&	P44=P()[15];

	Gx[0][0]=P11.x();	Gx[0][1]=P12.x();	Gx[0][2]=P13.x();	Gx[0][3]=P14.x(); 	
	Gx[1][0]=P21.x();	Gx[1][1]=P22.x();	Gx[1][2]=P23.x();	Gx[1][3]=P24.x(); 	
	Gx[2][0]=P31.x();	Gx[2][1]=P32.x();	Gx[2][2]=P33.x();	Gx[2][3]=P34.x(); 	
	Gx[3][0]=P41.x();	Gx[3][1]=P42.x();	Gx[3][2]=P43.x();	Gx[3][3]=P44.x(); 	

	Gy[0][0]=P11.y();	Gy[0][1]=P12.y();	Gy[0][2]=P13.y();	Gy[0][3]=P14.y(); 	
	Gy[1][0]=P21.y();	Gy[1][1]=P22.y();	Gy[1][2]=P23.y();	Gy[1][3]=P24.y(); 	
	Gy[2][0]=P31.y();	Gy[2][1]=P32.y();	Gy[2][2]=P33.y();	Gy[2][3]=P34.y(); 	
	Gy[3][0]=P41.y();	Gy[3][1]=P42.y();	Gy[3][2]=P43.y();	Gy[3][3]=P44.y(); 	
	
	Gz[0][0]=P11.z();	Gz[0][1]=P12.z();	Gz[0][2]=P13.z();	Gz[0][3]=P14.z(); 	
	Gz[1][0]=P21.z();	Gz[1][1]=P22.z();	Gz[1][2]=P23.z();	Gz[1][3]=P24.z(); 	
	Gz[2][0]=P31.z();	Gz[2][1]=P32.z();	Gz[2][2]=P33.z();	Gz[2][3]=P34.z(); 	
	Gz[3][0]=P41.z();	Gz[3][1]=P42.z();	Gz[3][2]=P43.z();	Gz[3][3]=P44.z(); 	
}

//---------------------------------------------------------------------
/** Evaluate a bicubic spline patch at the specified intervals, given the geometry matrices.
 * \param s Float interval.
 * \param t Float interval.
 * \param Gx Geometry matrix for x.
 * \param Gy Geometry matrix for y.
 * \param Gz Geometry matrix for z.
 * \return CqVector4D surface position.
 */

CqVector4D CqSurfacePatchBicubic::EvaluateMatrix(TqFloat s, TqFloat t, CqMatrix& Gx, CqMatrix& Gy,CqMatrix& Gz) const
{
	TqFloat t2=t*t;
	TqFloat t3=t2*t;
	TqFloat s2=s*s;
	TqFloat s3=s2*s;

	CqVector4D	T;						// T Transpose
	T[0]=t3;	T[1]=t2;	T[2]=t;	T[3]=1;
	CqVector4D	S;						// S
	S[0]=s3;	S[1]=s2;	S[2]=s;	S[3]=1;

	CqMatrix	matvBasisT;
	matvBasisT=pAttributes()->matvBasis().Transpose();

	CqVector4D	vecResult;

	vecResult=T*matvBasisT*Gx*pAttributes()->matuBasis()*S;

	return(vecResult);
}


//---------------------------------------------------------------------
/** Initialise the forward differencing variables given the current geometry matrix.
 */

void CqSurfacePatchBicubic::InitFD(TqInt cu, TqInt cv,
								 CqMatrix&	matDDx,
								 CqMatrix&	matDDy,
								 CqMatrix&	matDDz,
								 CqVector4D&	DDxA,
								 CqVector4D&	DDyA,
								 CqVector4D&	DDzA)
{
	CqMatrix	Gx;
	CqMatrix	Gy;
	CqMatrix	Gz;

	TqFloat	s,t;
	GetGeometryMatrices(s,t,Gx,Gy,Gz);
	Gx.SetfIdentity(TqFalse);
	Gy.SetfIdentity(TqFalse);
	Gz.SetfIdentity(TqFalse);

	TqFloat ud=1.0/static_cast<TqFloat>(cu);
	TqFloat	ud2=ud*ud;
	TqFloat	ud3=ud2*ud;

	TqFloat vd=1.0/static_cast<TqFloat>(cv);
	TqFloat	vd2=vd*vd;
	TqFloat	vd3=vd2*vd;

	TqFloat	EuValues[16]={
							0,0,0,1,
							ud3,ud2,ud,0,
							6*ud3,2*ud2,0,0,
							6*ud3,0,0,0
						};
	CqMatrix	matEu(EuValues);

	TqFloat	EvValues[16]={
							0,vd3,6*vd3,6*vd3,
							0,vd2,2*vd2,0,
							0,vd,0,0,
							1,0,0,0
						};
	CqMatrix	matEvT(EvValues);

	CqMatrix	matvBasisT;
	matvBasisT=pAttributes()->matvBasis().Transpose();

	const CqMatrix& matuBasis=pAttributes()->matuBasis();
	Gx=matvBasisT*Gx*matuBasis;
	Gy=matvBasisT*Gy*matuBasis;
	Gz=matvBasisT*Gz*matuBasis;
	
	matDDx=matEvT*Gx*matEu;
	matDDy=matEvT*Gy*matEu;
	matDDz=matEvT*Gz*matEu;

	DDxA[0]=matDDx[0][0]; DDxA[1]=matDDx[1][0]; DDxA[2]=matDDx[2][0]; DDxA[3]=matDDx[3][0];
	DDyA[0]=matDDy[0][0]; DDyA[1]=matDDy[1][0]; DDyA[2]=matDDy[2][0]; DDyA[3]=matDDy[3][0];
	DDzA[0]=matDDz[0][0]; DDzA[1]=matDDz[1][0]; DDzA[2]=matDDz[2][0]; DDzA[3]=matDDz[3][0];
}


//---------------------------------------------------------------------
/** Evaluate the mext iteration of the forward difference variables.
 */

CqVector4D	CqSurfacePatchBicubic::EvaluateFD(	CqMatrix&	matDDx,
												CqMatrix&	matDDy,
												CqMatrix&	matDDz,
												CqVector4D&	DDxA,
												CqVector4D&	DDyA,
												CqVector4D&	DDzA)
{
	CqVector4D	vecResult(DDxA[0],DDyA[0],DDzA[0],1);

	DDxA[0]+=DDxA[1]; DDxA[1]+=DDxA[2]; DDxA[2]+=DDxA[3]; 
	DDyA[0]+=DDyA[1]; DDyA[1]+=DDyA[2]; DDyA[2]+=DDyA[3]; 
	DDzA[0]+=DDzA[1]; DDzA[1]+=DDzA[2]; DDzA[2]+=DDzA[3]; 

	return(vecResult);
}


//---------------------------------------------------------------------
/** Evaluate the mext iteration of the forward difference variables.
 */

void	CqSurfacePatchBicubic::AdvanceFD(CqMatrix&	matDDx,
										 CqMatrix&	matDDy,
										 CqMatrix&	matDDz,
										 CqVector4D&	DDxA,
										 CqVector4D&	DDyA,
										 CqVector4D&	DDzA)
{
	// Row1 = Row1 + Row2
	matDDx[0][0]+=matDDx[0][1]; matDDx[1][0]+=matDDx[1][1]; matDDx[2][0]+=matDDx[2][1]; matDDx[3][0]+=matDDx[3][1];
	matDDy[0][0]+=matDDy[0][1]; matDDy[1][0]+=matDDy[1][1]; matDDy[2][0]+=matDDy[2][1]; matDDy[3][0]+=matDDy[3][1];
	matDDz[0][0]+=matDDz[0][1]; matDDz[1][0]+=matDDz[1][1]; matDDz[2][0]+=matDDz[2][1]; matDDz[3][0]+=matDDz[3][1];

	// Row2 = Row2 + Row3
	matDDx[0][1]+=matDDx[0][2]; matDDx[1][1]+=matDDx[1][2]; matDDx[2][1]+=matDDx[2][2]; matDDx[3][1]+=matDDx[3][2];
	matDDy[0][1]+=matDDy[0][2]; matDDy[1][1]+=matDDy[1][2]; matDDy[2][1]+=matDDy[2][2]; matDDy[3][1]+=matDDy[3][2];
	matDDz[0][1]+=matDDz[0][2]; matDDz[1][1]+=matDDz[1][2]; matDDz[2][1]+=matDDz[2][2]; matDDz[3][1]+=matDDz[3][2];

	// Row3 = Row3 + Row4
	matDDx[0][2]+=matDDx[0][3]; matDDx[1][2]+=matDDx[1][3]; matDDx[2][2]+=matDDx[2][3]; matDDx[3][2]+=matDDx[3][3];
	matDDy[0][2]+=matDDy[0][3]; matDDy[1][2]+=matDDy[1][3]; matDDy[2][2]+=matDDy[2][3]; matDDy[3][2]+=matDDy[3][3];
	matDDz[0][2]+=matDDz[0][3]; matDDz[1][2]+=matDDz[1][3]; matDDz[2][2]+=matDDz[2][3]; matDDz[3][2]+=matDDz[3][3];

	DDxA[0]=matDDx[0][0]; DDxA[1]=matDDx[1][0]; DDxA[2]=matDDx[2][0]; DDxA[3]=matDDx[3][0];
	DDyA[0]=matDDy[0][0]; DDyA[1]=matDDy[1][0]; DDyA[2]=matDDy[2][0]; DDyA[3]=matDDy[3][0];
	DDzA[0]=matDDz[0][0]; DDzA[1]=matDDz[1][0]; DDzA[2]=matDDz[2][0]; DDzA[3]=matDDz[3][0];
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqSurfacePatchBicubic& CqSurfacePatchBicubic::operator=(const CqSurfacePatchBicubic& From)	
{
	// Perform per surface copy function
	CqSurface::operator=(From);

	TqInt i;
	for(i=0; i<16; i++)
		P()[i]=From.P()[i];

	return(*this);
}


//---------------------------------------------------------------------
/** Subdivide a bicubic patch in the u direction, return the left side.
 */

CqSurfacePatchBicubic* CqSurfacePatchBicubic::uSubdivide()
{
	TqFloat	aDlb[16]=	{
							8,0,0,0,
							4,4,0,0,
							2,4,2,0,
							1,3,3,1
						};

	TqFloat	aDrb[16]=	{
							1,3,3,1,
							0,2,4,2,
							0,0,4,4,
							0,0,0,8
						};
	
	CqMatrix	Dlb(aDlb);
	CqMatrix	Drb(aDrb);
	Dlb=(1.0f/8.0f)*Dlb;
	Drb=(1.0f/8.0f)*Drb;

	CqSurfacePatchBicubic* pmResult=new CqSurfacePatchBicubic(*this);

	// Create the geometry matrix and storage area for the left and right split matrices.
	CqMatrix	G;
	CqMatrix	Gl;
	CqMatrix	Gr;	

	TqInt i;
	for(i=0; i<4; i++)
	{
		G[0][0]=CP(i,0).x();	G[0][1]=CP(i,0).y();	G[0][2]=CP(i,0).z();	G[0][3]=CP(i,0).h();
		G[1][0]=CP(i,1).x();	G[1][1]=CP(i,1).y();	G[1][2]=CP(i,1).z();	G[1][3]=CP(i,1).h();
		G[2][0]=CP(i,2).x();	G[2][1]=CP(i,2).y();	G[2][2]=CP(i,2).z();	G[2][3]=CP(i,2).h();
		G[3][0]=CP(i,3).x();	G[3][1]=CP(i,3).y();	G[3][2]=CP(i,3).z();	G[3][3]=CP(i,3).h();
		G.SetfIdentity(TqFalse);

		Gl=(G*Dlb);
		Gr=(G*Drb);

		CP(i,0)=CqVector4D(Gl[0][0],Gl[0][1],Gl[0][2],1);
		CP(i,1)=CqVector4D(Gl[1][0],Gl[1][1],Gl[1][2],1);
		CP(i,2)=CqVector4D(Gl[2][0],Gl[2][1],Gl[2][2],1);
		CP(i,3)=CqVector4D(Gl[3][0],Gl[3][1],Gl[3][2],1);

		pmResult->CP(i,0)=CqVector4D(Gr[0][0],Gr[0][1],Gr[0][2],1);
		pmResult->CP(i,1)=CqVector4D(Gr[1][0],Gr[1][1],Gr[1][2],1);
		pmResult->CP(i,2)=CqVector4D(Gr[2][0],Gr[2][1],Gr[2][2],1);
		pmResult->CP(i,3)=CqVector4D(Gr[3][0],Gr[3][1],Gr[3][2],1);
	}

	// Subdivide the u/v vectors
	if(USES(Uses(),EnvVars_u))	u().uSubdivide(&pmResult->u());
	if(USES(Uses(),EnvVars_v))	v().uSubdivide(&pmResult->v());

	// Subdivide the s/t vectors
	if(USES(Uses(),EnvVars_s))	s().uSubdivide(&pmResult->s());
	if(USES(Uses(),EnvVars_t))	t().uSubdivide(&pmResult->t());

	// Subdivide the colors
	if(USES(Uses(),EnvVars_Cs))	Cs().uSubdivide(&pmResult->Cs());
	if(USES(Uses(),EnvVars_Os))	Os().uSubdivide(&pmResult->Os());

	return(pmResult);
}


//---------------------------------------------------------------------
/** Subdivide a bicubic patch in the v direction, return the top side.
 */

CqSurfacePatchBicubic* CqSurfacePatchBicubic::vSubdivide()
{
	TqFloat	aDlb[16]=	{
							8,0,0,0,
							4,4,0,0,
							2,4,2,0,
							1,3,3,1
						};

	TqFloat	aDrb[16]=	{
							1,3,3,1,
							0,2,4,2,
							0,0,4,4,
							0,0,0,8
						};
	
	CqMatrix	Dlb(aDlb);
	CqMatrix	Drb(aDrb);
	Dlb=(1.0f/8.0f)*Dlb;
	Drb=(1.0f/8.0f)*Drb;

	CqSurfacePatchBicubic* pmResult=new CqSurfacePatchBicubic(*this);

	// Create the geometry matrix and storage area for the left and right split matrices.
	CqMatrix	G;
	CqMatrix	Gl;
	CqMatrix	Gr;	

	TqInt i;
	for(i=0; i<4; i++)
	{
		G[0][0]=CP(0,i).x();	G[0][1]=CP(0,i).y();	G[0][2]=CP(0,i).z();	G[0][3]=CP(0,i).h();
		G[1][0]=CP(1,i).x();	G[1][1]=CP(1,i).y();	G[1][2]=CP(1,i).z();	G[1][3]=CP(1,i).h();
		G[2][0]=CP(2,i).x();	G[2][1]=CP(2,i).y();	G[2][2]=CP(2,i).z();	G[2][3]=CP(2,i).h();
		G[3][0]=CP(3,i).x();	G[3][1]=CP(3,i).y();	G[3][2]=CP(3,i).z();	G[3][3]=CP(3,i).h();
		G.SetfIdentity(TqFalse);
			
		Gl=(G*Dlb);
		Gr=(G*Drb);

		CP(0,i)=CqVector4D(Gl[0][0],Gl[0][1],Gl[0][2],1);
		CP(1,i)=CqVector4D(Gl[1][0],Gl[1][1],Gl[1][2],1);
		CP(2,i)=CqVector4D(Gl[2][0],Gl[2][1],Gl[2][2],1);
		CP(3,i)=CqVector4D(Gl[3][0],Gl[3][1],Gl[3][2],1);

		pmResult->CP(0,i)=CqVector4D(Gr[0][0],Gr[0][1],Gr[0][2],1);
		pmResult->CP(1,i)=CqVector4D(Gr[1][0],Gr[1][1],Gr[1][2],1);
		pmResult->CP(2,i)=CqVector4D(Gr[2][0],Gr[2][1],Gr[2][2],1);
		pmResult->CP(3,i)=CqVector4D(Gr[3][0],Gr[3][1],Gr[3][2],1);
	}

	// Subdivide the u/v vectors
	if(USES(Uses(),EnvVars_u))	u().vSubdivide(&pmResult->u());
	if(USES(Uses(),EnvVars_v))	v().vSubdivide(&pmResult->v());


	// Subdivide the s/t vectors
	if(USES(Uses(),EnvVars_s))	s().vSubdivide(&pmResult->s());
	if(USES(Uses(),EnvVars_t))	t().vSubdivide(&pmResult->t());

	// Subdivide the colors
	if(USES(Uses(),EnvVars_Cs))	Cs().vSubdivide(&pmResult->Cs());
	if(USES(Uses(),EnvVars_Os))	Os().vSubdivide(&pmResult->Os());

	return(pmResult);
}


//---------------------------------------------------------------------
/** Get the boundary extents in camera space of the surface patch
 */

CqBound CqSurfacePatchBicubic::Bound() const
{
	// Get the boundary in camera space.
	CqVector3D	vecA(FLT_MAX, FLT_MAX, FLT_MAX);
	CqVector3D	vecB(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	TqInt i;
	for(i=0; i<16; i++)
	{
		CqVector3D	vecV=P()[i];
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
/** Dice the patch into a mesh of micropolygons.
 */

CqMicroPolyGridBase* CqSurfacePatchBicubic::Dice()
{
	// Create a new CqMicorPolyGrid for this patch
	CqMicroPolyGrid* pGrid=new CqMicroPolyGrid(m_uDiceSize, m_vDiceSize, this);

	// NOTE: This violates thread safety, look into this.
	CqMatrix	matDDx;
	CqMatrix	matDDy;
	CqMatrix	matDDz;
	CqVector4D	DDxA;
	CqVector4D	DDyA;
	CqVector4D	DDzA;

	// Initialise the forward difference variables.
	InitFD(m_uDiceSize,m_vDiceSize,matDDx,matDDy,matDDz,DDxA,DDyA,DDzA);
	TqFloat diu=1.0/m_uDiceSize;
	TqFloat div=1.0/m_vDiceSize;

	TqInt lUses=Uses();

	// Dice the primitive variables.
	if(USES(lUses,EnvVars_u))	u().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->u());
	if(USES(lUses,EnvVars_v))	v().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->v());
	if(USES(lUses,EnvVars_s))	s().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->s());
	if(USES(lUses,EnvVars_t))	t().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->t());
	if(USES(lUses,EnvVars_Cs))	Cs().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->Cs());
	if(USES(lUses,EnvVars_Os))	Os().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->Os());
	
	pGrid->Reset();
	TqInt iv,iu;
	for(iv=0; iv<=m_vDiceSize; iv++)
	{
		for(iu=0; iu<=m_uDiceSize; iu++)
		{
			pGrid->P()=EvaluateFD(matDDx,matDDy,matDDz,DDxA,DDyA,DDzA);
			pGrid->Advance();
		}
		AdvanceFD(matDDx,matDDy,matDDz,DDxA,DDyA,DDzA);
	}
	// Only shade if the ImageBuffer mode is at least RGB
	if(pCurrentRenderer()->optCurrent().iDisplayMode()&ModeRGB)
		pGrid->Shade();

	return(pGrid);
}

//---------------------------------------------------------------------
/** Split the patch into smaller patches.
 */

TqInt CqSurfacePatchBicubic::Split(std::vector<CqBasicSurface*>& aSplits)
{
	// Split the surface in u or v
	CqSurfacePatchBicubic* pNew1=new CqSurfacePatchBicubic(*this);
	CqSurfacePatchBicubic* pNew2;
	
	if(m_SplitDir==SplitDir_U)
		pNew2=pNew1->uSubdivide();
	else
		pNew2=pNew1->vSubdivide();
	
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
/** Determine whether or not the patch is diceable
 */

TqBool	CqSurfacePatchBicubic::Diceable()
{
	const CqMatrix& matCtoR=pCurrentRenderer()->matSpaceToSpace("camera","raster");
	
	// Convert the control hull to raster space.
	CqVector2D	avecHull[16];
	TqInt i;
	for(i=0; i<16; i++)
		avecHull[i]=matCtoR*P()[i];

	// First check flatness, a curve which is too far off flat will
	// produce unreliable results when the length is approximated below.
	m_SplitDir=SplitDir_U;
	TqInt u;
	for(u=0; u<16; u+=4)
	{
		// Find an initial line
		TqFloat Len=0;
		CqVector2D	vec0=avecHull[u];
		CqVector2D	vecL;
		TqInt i=4;
		while(i-->0 && Len<FLT_EPSILON)
		{
			vecL=avecHull[u+i]-vec0;
			Len=vecL.Magnitude();
		}
		vecL/=Len;	// Normalise

		i=0;
		while(i++<4)
		{
			// Get the distance to the line for each point
			CqVector3D	vec=avecHull[u+i]-vec0;
			vec.Unit();
			vec%=vecL;
			if(vec.Magnitude()>1)	return(TqFalse);
		}
	}
	m_SplitDir=SplitDir_V;
	TqInt v;
	for(v=0; v<4; v++)
	{
		// Find an initial line
		TqFloat Len=0;
		CqVector2D	vec0=avecHull[v];
		CqVector2D	vecL;
		TqInt i=4;
		while(i-->0 && Len<FLT_EPSILON)
		{
			vecL=avecHull[v+(i*4)]-vec0;
			Len=vecL.Magnitude();
		}
		vecL/=Len;	// Normalise

		i=0;
		while(i++<4)
		{
			// Get the distance to the line for each point
			CqVector3D	vec=avecHull[v+(i*4)]-vec0;
			vec.Unit();
			vec%=vecL;
			if(vec.Magnitude()>1)	return(TqFalse);
		}
	}


	TqFloat uLen=0;
	TqFloat vLen=0;

	for(u=0; u<16; u+=4)
	{
		CqVector2D	Vec1=avecHull[u+1]-avecHull[u  ];
		CqVector2D	Vec2=avecHull[u+2]-avecHull[u+1];
		CqVector2D	Vec3=avecHull[u+3]-avecHull[u+2];
		if(Vec1.Magnitude2()>uLen)	uLen=Vec1.Magnitude2();
		if(Vec2.Magnitude2()>uLen)	uLen=Vec2.Magnitude2();
		if(Vec3.Magnitude2()>uLen)	uLen=Vec3.Magnitude2();
	}
	for(v=0; v<4; v++)
	{
		CqVector2D	Vec1=avecHull[v+4 ]-avecHull[v  ];
		CqVector2D	Vec2=avecHull[v+8 ]-avecHull[v+4];
		CqVector2D	Vec3=avecHull[v+12]-avecHull[v+8];
		if(Vec1.Magnitude2()>vLen)	vLen=Vec1.Magnitude2();
		if(Vec2.Magnitude2()>vLen)	vLen=Vec2.Magnitude2();
		if(Vec3.Magnitude2()>vLen)	vLen=Vec3.Magnitude2();
	}

	float ShadingRate=pAttributes()->fEffectiveShadingRate();
//	if(pCurrentRenderer()->Mode()==RenderMode_Shadows)
//	{
//		const TqFloat* pattrShadowShadingRate=m_pAttributes->GetFloatAttribute("render","shadow_shadingrate");
//		if(pattrShadowShadingRate!=0)
//			ShadingRate=pattrShadowShadingRate[0];
//	}	

	ShadingRate=static_cast<float>(sqrt(ShadingRate));
	uLen=sqrt(uLen)/ShadingRate;
	vLen=sqrt(vLen)/ShadingRate;

	m_SplitDir=(uLen>vLen)?SplitDir_U:SplitDir_V;
	if(!m_fDiceable)
		return(TqFalse);

	// TODO: Should ensure powers of half to prevent cracking.
	uLen*=3;
	vLen*=3;
	m_uDiceSize=static_cast<TqInt>(MAX(uLen,1));
	m_vDiceSize=static_cast<TqInt>(MAX(vLen,1));
	TqFloat Area=m_uDiceSize*m_vDiceSize;

	if(uLen<FLT_EPSILON || vLen<FLT_EPSILON)
	{
		m_fDiscard=TqTrue;
		return(TqFalse);
	}

	if(fabs(Area)>256)
		return(TqFalse);
	else
		return(TqTrue);
}


//---------------------------------------------------------------------
/** Transform the patch by the specified matrix.
 */

void	CqSurfacePatchBicubic::Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx)
{
	// Tansform the control hull by the specified matrix.
	TqInt i;
	for(i=0; i<16; i++)
		P()[i]=matTx*P()[i];
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqSurfacePatchBilinear::CqSurfacePatchBilinear() : CqSurface()
{
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqSurfacePatchBilinear::CqSurfacePatchBilinear(const CqSurfacePatchBilinear& From) :
							CqSurface(From)
{
	*this=From;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqSurfacePatchBilinear::~CqSurfacePatchBilinear()
{
}


//---------------------------------------------------------------------
/** Evaluate a bilinear spline patch normal at the specified intervals.
 */

CqVector4D CqSurfacePatchBilinear::EvaluateNormal(TqFloat s, TqFloat t) const
{
	CqVector3D vecNAB, vecNCD;
	// Work out where the u points are first, then linear interpolate the v value.
	if(s<=0.0)
	{
		vecNAB=N()[0];
		vecNCD=N()[2];
	}
	else
	{
		if(s>=1.0)
		{
			vecNAB=N()[1];
			vecNCD=N()[3];
		}
		else
		{
			vecNAB=(N()[1]*s)+(N()[0]*(1.0-s));
			vecNCD=(N()[3]*s)+(N()[2]*(1.0-s));
		}
	}

	CqVector3D vecN;
	if(t<=0.0)
		vecN=vecNAB;
	else
	{
		if(t>=1.0)
			vecN=vecNCD;
		else
			vecN=(vecNCD*t)+(vecNAB*(1.0-t));
	}
	
	vecN.Unit();
	return(vecN);
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqSurfacePatchBilinear& CqSurfacePatchBilinear::operator=(const CqSurfacePatchBilinear& From)	
{
	CqSurface::operator=(From);

	return(*this);
}


//---------------------------------------------------------------------
/** Generate the vertex normals if not specified.
 */

void CqSurfacePatchBilinear::GenNormals()
{
	assert(P().Size()==4);
	N().SetSize(4);

	// Get the handedness of the coordinate system (at the time of creation) and
	// the coordinate system specified, to check for normal flipping.
	EqOrientation CSO=pAttributes()->eCoordsysOrientation();
	EqOrientation O=pAttributes()->eOrientation();
	
	// For each of the four points, calculate the normal as the cross product of its 
	// two vectors.
	CqVector3D vecN;
	vecN=(P()[1]-P()[0])%(P()[2]-P()[0]);
	vecN.Unit();
	N()[0]=(CSO==O)?vecN:-vecN;

	vecN=(P()[3]-P()[1])%(P()[0]-P()[1]);
	vecN.Unit();
	N()[1]=(CSO==O)?vecN:-vecN;

	vecN=(P()[0]-P()[2])%(P()[3]-P()[2]);
	vecN.Unit();
	N()[2]=(CSO==O)?vecN:-vecN;

	vecN=(P()[2]-P()[3])%(P()[1]-P()[3]);
	vecN.Unit();
	N()[3]=(CSO==O)?vecN:-vecN;
}


//---------------------------------------------------------------------
/** Subdivide a bicubic patch in the u direction, return the left side.
 */

CqSurfacePatchBilinear* CqSurfacePatchBilinear::uSubdivide()
{
	CqSurfacePatchBilinear* pmResult=new CqSurfacePatchBilinear(*this);

	// Subdivide the vertices
	P().uSubdivide(&pmResult->P());

	// Subdivide the normals
	if(USES(Uses(),EnvVars_N))	N().uSubdivide(&pmResult->N());

	// Subdivide the u/v vectors
	if(USES(Uses(),EnvVars_u))	u().uSubdivide(&pmResult->u());
	if(USES(Uses(),EnvVars_v))	v().uSubdivide(&pmResult->v());

	// Subdivide the s/t vectors
	if(USES(Uses(),EnvVars_s))	s().uSubdivide(&pmResult->s());
	if(USES(Uses(),EnvVars_t))	t().uSubdivide(&pmResult->t());

	// Subdivide the colors
	if(USES(Uses(),EnvVars_Cs))	Cs().uSubdivide(&pmResult->Cs());
	if(USES(Uses(),EnvVars_Os))	Os().uSubdivide(&pmResult->Os());

	return(pmResult);
}


//---------------------------------------------------------------------
/** Subdivide a bicubic patch in the v direction, return the top side.
 */

CqSurfacePatchBilinear* CqSurfacePatchBilinear::vSubdivide()
{
	CqSurfacePatchBilinear* pmResult=new CqSurfacePatchBilinear(*this);

	// Subdivide the vertices.
	P().vSubdivide(&pmResult->P());

	// Subdivide the normals
	if(USES(Uses(),EnvVars_N))	N().vSubdivide(&pmResult->N());

	// Subdivide the u/v vectors
	if(USES(Uses(),EnvVars_u))	u().vSubdivide(&pmResult->u());
	if(USES(Uses(),EnvVars_v))	v().vSubdivide(&pmResult->v());

	// Subdivide the s/t vectors
	if(USES(Uses(),EnvVars_s))	s().vSubdivide(&pmResult->s());
	if(USES(Uses(),EnvVars_t))	t().vSubdivide(&pmResult->t());

	// Subdivide the colors
	if(USES(Uses(),EnvVars_Cs))	Cs().vSubdivide(&pmResult->Cs());
	if(USES(Uses(),EnvVars_Os))	Os().vSubdivide(&pmResult->Os());

	return(pmResult);
}


//---------------------------------------------------------------------
/** Return the boundary extents in camera space of the surface patch
 */

CqBound CqSurfacePatchBilinear::Bound() const
{
	// Get the boundary in camera space.
	CqVector3D	vecA(FLT_MAX, FLT_MAX, FLT_MAX);
	CqVector3D	vecB(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	TqInt i;
	for(i=0; i<4; i++)
	{
		CqVector3D	vecV=P()[i];
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
/** Dice the patch into a mesh of micropolygons.
 */

CqMicroPolyGridBase* CqSurfacePatchBilinear::Dice()
{
	// Create a new CqMicroPolyGrid for this patch
	CqMicroPolyGrid* pGrid=new CqMicroPolyGrid(m_uDiceSize, m_vDiceSize, this);
	
	TqFloat diu=1.0/m_uDiceSize;
	TqFloat div=1.0/m_vDiceSize;
	pGrid->Reset();
	
	TqInt lUses=Uses();

	// Dice the primitive variables.
	if(USES(lUses,EnvVars_u))	u().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->u());
	if(USES(lUses,EnvVars_v))	v().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->v());
	if(USES(lUses,EnvVars_s))	s().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->s());
	if(USES(lUses,EnvVars_t))	t().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->t());
	if(USES(lUses,EnvVars_Cs))	Cs().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->Cs());
	if(USES(lUses,EnvVars_Os))	Os().BilinearDice(m_uDiceSize,m_vDiceSize,&pGrid->Os());
	
	bool bNormals=false;
	if(N().Size()==4)
	{
		pGrid->SetNormals(TqTrue);
		bNormals=true;
	}

	TqInt iv,iu;
	for(iv=0; iv<=m_vDiceSize; iv++)
	{
		for(iu=0; iu<=m_uDiceSize; iu++)
		{
			if(bNormals)	pGrid->N()=EvaluateNormal(iu*diu, iv*div);
			pGrid->P()=BilinearEvaluate<CqVector4D>(P()[0],P()[1],P()[2],P()[3],iu*diu,iv*div);
			pGrid->Advance();
		}
	}

	// Only shade if the ImageBuffer mode is at least RGB
	if(pCurrentRenderer()->optCurrent().iDisplayMode()&ModeRGB)
		pGrid->Shade();

	return(pGrid);
}


//---------------------------------------------------------------------
/** Split the patch into smaller patches.
 */

TqInt CqSurfacePatchBilinear::Split(std::vector<CqBasicSurface*>& aSplits)
{
	// Split the surface in u or v
	CqSurfacePatchBilinear* pNew1=new CqSurfacePatchBilinear(*this);
	CqSurfacePatchBilinear* pNew2;
	
	if(m_SplitDir==SplitDir_U)
		pNew2=pNew1->uSubdivide();
	else
		pNew2=pNew1->vSubdivide();
	
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
/** Determine whether or not the patch is diceable
 */

TqBool	CqSurfacePatchBilinear::Diceable()
{
	const CqMatrix& matCtoR=pCurrentRenderer()->matSpaceToSpace("camera","raster");

	// Convert the control hull to raster space.
	CqVector2D	avecHull[4];
	TqInt i;
	for(i=0; i<4; i++)
		avecHull[i]=matCtoR*P()[i];

	TqFloat uLen=0;
	TqFloat vLen=0;

	CqVector2D	Vec1=avecHull[1]-avecHull[0];
	CqVector2D	Vec2=avecHull[3]-avecHull[2];
	uLen=(Vec1.Magnitude2()>Vec2.Magnitude2())?Vec1.Magnitude2():Vec2.Magnitude2();

				Vec1=avecHull[2]-avecHull[0];
				Vec2=avecHull[3]-avecHull[1];
	vLen=(Vec1.Magnitude2()>Vec2.Magnitude2())?Vec1.Magnitude2():Vec2.Magnitude2();

	float ShadingRate=pAttributes()->fEffectiveShadingRate();
//	if(pCurrentRenderer()->Mode()==RenderMode_Shadows)
//	{
//		const TqFloat* pattrShadowShadingRate=m_pAttributes->GetFloatAttribute("render","shadow_shadingrate");
//		if(pattrShadowShadingRate!=0)
//			ShadingRate=pattrShadowShadingRate[0];
//	}	

	ShadingRate=static_cast<float>(sqrt(ShadingRate));
	uLen=sqrt(uLen)/ShadingRate;
	vLen=sqrt(vLen)/ShadingRate;

	m_SplitDir=(uLen>vLen)?SplitDir_U:SplitDir_V;
	if(!m_fDiceable)
		return(TqFalse);

	// TODO: Should ensure powers of half to prevent cracking.
	TqFloat Area=(MAX(uLen,1))*(MAX(vLen,1));
	m_uDiceSize=static_cast<TqInt>(MAX(uLen,1));
	m_vDiceSize=static_cast<TqInt>(MAX(vLen,1));

	if(uLen<FLT_EPSILON || vLen<FLT_EPSILON)
	{
		m_fDiscard=TqTrue;
		return(TqFalse);
	}

	if(fabs(Area)>256)
		return(TqFalse);
	else
		return(TqTrue);
}


//---------------------------------------------------------------------
/** Transform the patch by the specified matrix.
 */

void	CqSurfacePatchBilinear::Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx)
{
	// Tansform the control hull by the specified matrix.
	TqInt i;
	for(i=0; i<4; i++)
	{
		P()[i]=matTx*P()[i];
		if(N().Size()==4)	N()[i]=matITTx*N()[i];
	}
}


END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
