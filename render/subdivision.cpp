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
		\brief Implements classes for the subdivision surface GPrim.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"subdivision.h"
#include	"irenderer.h"
#include	"micropolygon.h"
#include	"imagebuffer.h"

START_NAMESPACE(Aqsis)


//---------------------------------------------------------------------
/** Remove an edge reference from the array.
 */

void CqWVert::RemoveEdge(CqWEdge* pE)
{
	TqInt i;

	for(i=0; i<m_apEdges.size(); i++)
	{
		if(m_apEdges[i]==pE)	
		{
			m_apEdges.erase(m_apEdges.begin()+i);
			return;
		}
	}
	assert(false);
}


//---------------------------------------------------------------------
/** Is this edge on the boundary of the mesh?
 */

TqBool CqWEdge::IsBoundary()
{
	if((!pfLeft() || !pfRight()) && !(!pfLeft() && !pfRight()))
		return(TqTrue);
	else
		return(TqFalse);
}


//---------------------------------------------------------------------
/** Is this edge valid? i.e. has it been used by at least one face.
 */

TqBool CqWEdge::IsValid()
{
	if(!pfLeft() && !pfRight())
		return(TqFalse);
	else
		return(TqTrue);
}


//---------------------------------------------------------------------
/** Create a new WVertex for this edge taking into account its sharpness, and that of its neighbours.
 */

CqWVert* CqWEdge::CreateSubdividePoint(CqWSurf* pSurf, TqInt index, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os,
																	TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os)
{
	assert(index<pSurf->P().Size());

	CqWVert* pV=new CqWVert(index);
	pSurf->AddVert(pV);
	m_pvSubdivide=pV;

	CqVector3D P=CreateSubdivideScalar(P,&CqWSurf::SubdP,pSurf);
	pSurf->P()[index]=P;

	if(uses_s && has_s)
	{
		TqFloat s=CreateSubdivideScalar(s,&CqWSurf::Subds,pSurf);
		pSurf->s()[index]=s;
	}

	if(uses_t && has_t)
	{
		TqFloat t=CreateSubdivideScalar(t,&CqWSurf::Subdt,pSurf);
		pSurf->t()[index]=t;
	}
	
	if(uses_Cs && has_Cs)
	{
		CqColor Cq=CreateSubdivideScalar(Cq,&CqWSurf::SubdCs,pSurf);
		pSurf->Cs()[index]=Cq;
	}

	if(uses_Os && has_Os)
	{
		CqColor Os=CreateSubdivideScalar(Os,&CqWSurf::SubdOs,pSurf);
		pSurf->Os()[index]=Os;
	}

	return(pV);
}



//---------------------------------------------------------------------
/** Subdivide this edge into two subedges, can only be called AFTER CreateSubdividePoint.
 */

void CqWEdge::Subdivide(CqWSurf* pSurf)
{
	if(m_pvSubdivide==NULL)
	{	
		//throw("Error : Attempting to split an edge with no midpoint");
		return;
	}

	m_peHeadHalf=new CqWEdge();
	m_peTailHalf=new CqWEdge();
	pSurf->AddEdge(m_peHeadHalf);
	pSurf->AddEdge(m_peTailHalf);
	
	m_peHeadHalf->SetpvHead(pvHead());
	m_peHeadHalf->SetpvTail(m_pvSubdivide);
	m_peTailHalf->SetpvHead(m_pvSubdivide);
	m_peTailHalf->SetpvTail(pvTail());
	
	m_peHeadHalf->SetpeHeadRight(peHeadRight());
	m_peHeadHalf->SetpeHeadLeft(peHeadLeft());

	m_peTailHalf->SetpeTailRight(peTailRight());
	m_peTailHalf->SetpeTailLeft(peTailLeft());
	
	// Calculate the new sharpness values.
	if(m_Sharpness==0)
		m_peHeadHalf->m_Sharpness=m_peTailHalf->m_Sharpness=0;
	else
	{
		// Find the edges which make up this crease.
		TqInt i=0;
		CqWEdge* peA=pvHead()->lEdges()[i];
		while(peA->m_Sharpness<=0 && i<pvHead()->cEdges())
			peA=pvHead()->lEdges()[i++];

		i=0;
		CqWEdge* peC=pvTail()->lEdges()[i];
		while(peC->m_Sharpness<=0 && i<pvTail()->cEdges())
			peC=pvTail()->lEdges()[i++];

		m_peHeadHalf->m_Sharpness=((peA->m_Sharpness+(3.0f*m_Sharpness))/4.0f)-1.0f;
		m_peTailHalf->m_Sharpness=(((3.0f*m_Sharpness)+peC->m_Sharpness)/4.0f)-1.0f;
		
		if(m_peHeadHalf->m_Sharpness<0)	m_peHeadHalf->m_Sharpness=0.0;
		if(m_peTailHalf->m_Sharpness<0)	m_peTailHalf->m_Sharpness=0.0;
	}
	
	m_pvSubdivide->AddEdge(peHeadHalf());
	m_pvSubdivide->AddEdge(peTailHalf());
	pvHead()->AddEdge(peHeadHalf());
	pvTail()->AddEdge(peTailHalf());
}


//---------------------------------------------------------------------
/** Create a new WVertex as the centroid of this face.
 */

CqWVert* CqWFace::CreateSubdividePoint(CqWSurf* pSurf, TqInt index, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os,
																	TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os)
{
	assert(index<pSurf->P().Size());

	CqWVert* pV=new CqWVert(index);
	pSurf->AddVert(pV);
	m_pvSubdivide=pV;

	CqVector3D P=CreateSubdivideScalar(P,&CqWSurf::SubdP,pSurf);
	pSurf->P()[index]=P;

	if(uses_s && has_s)
	{
		TqFloat s=CreateSubdivideScalar(s,&CqWSurf::Subds,pSurf);
		pSurf->s()[index]=s;
	}

	if(uses_t && has_t)
	{
		TqFloat t=CreateSubdivideScalar(t,&CqWSurf::Subdt,pSurf);
		pSurf->t()[index]=t;
	}
	
	if(uses_Cs && has_Cs)
	{
		CqColor Cq=CreateSubdivideScalar(Cq,&CqWSurf::SubdCs,pSurf);
		pSurf->Cs()[index]=Cq;
	}

	if(uses_Os && has_Os)
	{
		CqColor Os=CreateSubdivideScalar(Os,&CqWSurf::SubdOs,pSurf);
		pSurf->Os()[index]=Os;
	}

	return(pV);
}

//---------------------------------------------------------------------
/** Find the specified edge.
 */

CqWEdge* CqWSurf::FindEdge(CqWEdge* pE)
{
	TqInt i;

	// If no vertices or no edges, cannot have been constructed yet.
	if(m_apVerts.size()==0 || m_apEdges.size()==0)	return(NULL);

	for(i=0; i<pE->pvHead()->cEdges(); i++)
	{
		if(pE->pvHead()->pEdge(i)->pvTail()==pE->pvTail() ||
		   pE->pvHead()->pEdge(i)->pvHead()==pE->pvTail())
			return(pE->pvHead()->lEdges()[i]);
	}
	
	for(i=0; i<pE->pvTail()->cEdges(); i++)
	{
		if(pE->pvTail()->pEdge(i)->pvHead()==pE->pvHead() ||
		   pE->pvTail()->pEdge(i)->pvTail()==pE->pvHead())	
			return(pE->pvTail()->lEdges()[i]);
	}
	return(NULL);
}


//---------------------------------------------------------------------
/** Find the specified vertex.
 */

CqWVert* CqWSurf::FindVertex(const CqVector3D& V)
{
	TqInt i;

	// If no vertices or no edges, cannot have been constructed yet.
	if(m_apVerts.size()==0 || m_apEdges.size()==0)	return(NULL);

	for(i=0; i<m_apVerts.size(); i++)
	{
		if(SubdP(m_apVerts[i]->iVertex())==V)
			return(m_apVerts[i]);
	}
	return(NULL);
}


//---------------------------------------------------------------------
/** Add and edge to the list.
 */

CqWEdge* CqWSurf::AddEdge(CqWVert* pA, CqWVert* pB)
{
	CqWEdge* pExist;
	CqWEdge eTemp(pA,pB);
	if((pExist=FindEdge(&eTemp))!=0)
	{
		if(pExist->pvHead()==pA)
		{
			CqBasicError(ErrorID_NonmanifoldSubdivision,Severity_Fatal,"Subdivision Mesh contains non-manifold data");
			return(0);
		}
		return(pExist);
	}
	else
	{
		CqWEdge* pNew=new CqWEdge(pA, pB);
		// TODO: Check if it is valid to return an edge in the opposite direction.
		m_apEdges.push_back(pNew);
		pNew->pvHead()->AddEdge(pNew);
		pNew->pvTail()->AddEdge(pNew);
		return(pNew);
	}
}


//---------------------------------------------------------------------
/** Add a vertex to the list.
 */

CqWVert* CqWSurf::AddVert(const CqVector3D& V)
{
	CqWVert* pExist=FindVertex(V);
	if(pExist!=0)
		return(pExist);
	else
	{
		P().SetSize(P().Size()+1);
		P()[P().Size()-1]=V;
		TqInt iV=P().Size()-1;
		CqWVert* pNew=new CqWVert(iV);
		m_apVerts.push_back(pNew);
		return(pNew);
	}
}


//---------------------------------------------------------------------
/** Add a polygon, passed as an array of edge references.
 */

CqWFace* CqWSurf::AddFace(CqWEdge** pE, TqInt cE)
{
	TqInt i;
	CqWEdge* pCurrE;
	CqWFace* pNewFace=new CqWFace();
	for(i=0; i<cE; i++)
	{
		if((pCurrE=FindEdge(pE[i]))==0)
		{
			// Error, edges must be available.
			delete(pNewFace);
			return(0);
		}
		else
		{
			if(pCurrE->bComplete())
			{
				CqBasicError(ErrorID_NonmanifoldSubdivision,Severity_Fatal,"Subdivision Mesh contains non-manifold data");
				delete(pNewFace);
				return(0);
			}
			else if(pCurrE->cFaces()==0)
				pCurrE->SetpfLeft(pNewFace);
			else
				pCurrE->SetpfRight(pNewFace);

			pCurrE->IncrFaces();
		}
		pNewFace->AddEdge(pCurrE);
	}
	
	// Fill in the wing data
	for(i=0; i<pNewFace->cEdges(); i++)
	{
		if(pNewFace->pEdge(i)->pfLeft()==pNewFace)
		{
			pNewFace->pEdge(i)->SetpeTailLeft((i==pNewFace->cEdges()-1)?pNewFace->pEdge(0):pNewFace->pEdge(i+1));
			pNewFace->pEdge(i)->SetpeHeadLeft((i==0)?					pNewFace->pEdge(pNewFace->cEdges()-1):pNewFace->pEdge(i-1));
		}
		else
		{
			pNewFace->pEdge(i)->SetpeHeadRight((i==pNewFace->cEdges()-1)?pNewFace->pEdge(0):pNewFace->pEdge(i+1));
			pNewFace->pEdge(i)->SetpeTailRight((i==0)?					 pNewFace->pEdge(pNewFace->cEdges()-1):pNewFace->pEdge(i-1));
		}
	}
	m_apFaces.push_back(pNewFace);
	return(pNewFace);
}


//---------------------------------------------------------------------
/** Destructor
 */

CqWSurf::~CqWSurf()
{
	// Delete all edges, vertices and faces.
	int i;
	for(i=0; i<m_apVerts.size(); i++)
		delete(m_apVerts[i]);

	for(i=0; i<m_apFaces.size(); i++)
		delete(m_apFaces[i]);

	for(i=0; i<m_apEdges.size(); i++)
		delete(m_apEdges[i]);
}


//---------------------------------------------------------------------
/** Move the vertex points according to the subdivision 
 */

struct SqVData
{
	CqVector3D	P;
	TqFloat		s;
	TqFloat		t;
	CqColor	Cq;
	CqColor	Os;
};

void CqWSurf::SmoothVertexPoints(TqInt oldcVerts, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os,
												  TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os)
{
	static CqVector3D vecT;
	static TqFloat fT;
	static CqColor colT;

	// NOTE: Not entirely happy about this method, would prefer a more efficient approach!
	// Must create this array here, to ensure we only store the old points, not the subdivided ones.
	std::vector<SqVData> aVertices;
	aVertices.resize(oldcVerts);

	// Smooth vertex points
	TqInt iE,bE,sE,i;
	for(i=0; i<oldcVerts; i++)
	{
		CqWVert* pV=pVert(i);
		if(pV->cEdges()>0)
		{
			// Check for crease vertex
			bE=sE=0;
			for(iE=0; iE<pV->cEdges(); iE++)
			{
				if(pV->pEdge(iE)->IsValid()==TqFalse)	continue;
				if(pV->pEdge(iE)->IsBoundary())			bE++;
				if(pV->pEdge(iE)->Sharpness()>0.0f)		sE++;
			}
			
			// Check for smooth first (most commmon case, less likely to thrash the cache).
			if(sE<=1 && bE==0)
			{
				aVertices[i].P=pV->GetSmoothedScalar(vecT, &CqWSurf::SubdP, this);
				if(uses_s && has_s)		aVertices[i].s=pV->GetSmoothedScalar(fT, &CqWSurf::Subds, this);
				if(uses_t && has_t)		aVertices[i].t=pV->GetSmoothedScalar(fT, &CqWSurf::Subdt, this);
				if(uses_Cs && has_Cs)	aVertices[i].Cq=pV->GetSmoothedScalar(colT, &CqWSurf::SubdCs, this);
				if(uses_Os && has_Os)	aVertices[i].Os=pV->GetSmoothedScalar(colT, &CqWSurf::SubdOs, this);
			}
			else
			{
				// Check for user set sharp edges first.
				if(sE>0)
				{
					if(sE==2)
					{
						aVertices[i].P=pV->GetCreaseScalar(vecT, &CqWSurf::SubdP, this);
						if(uses_s && has_s)		aVertices[i].s=pV->GetCreaseScalar(fT, &CqWSurf::Subds, this);
						if(uses_t && has_t)		aVertices[i].t=pV->GetCreaseScalar(fT, &CqWSurf::Subdt, this);
						if(uses_Cs && has_Cs)	aVertices[i].Cq=pV->GetCreaseScalar(colT, &CqWSurf::SubdCs, this);
						if(uses_Os && has_Os)	aVertices[i].Os=pV->GetCreaseScalar(colT, &CqWSurf::SubdOs, this);
					}
					else
					{
						aVertices[i].P=SubdP(pV->iVertex());
						if(uses_s && has_s)		aVertices[i].s=Subds(pV->iVertex());
						if(uses_t && has_t)		aVertices[i].t=Subdt(pV->iVertex());
						if(uses_Cs && has_Cs)	aVertices[i].Cq=SubdCs(pV->iVertex());
						if(uses_Os && has_Os)	aVertices[i].Os=SubdOs(pV->iVertex());
					}
				}
				else
				{
					if(pV->cEdges()==2)	// Boundary point with valence 2 is corner
					{
						aVertices[i].P=SubdP(pV->iVertex());
						if(uses_s && has_s)		aVertices[i].s=Subds(pV->iVertex());
						if(uses_t && has_t)		aVertices[i].t=Subdt(pV->iVertex());
						if(uses_Cs && has_Cs)	aVertices[i].Cq=SubdCs(pV->iVertex());
						if(uses_Os && has_Os)	aVertices[i].Os=SubdOs(pV->iVertex());
					}
					else				// Boundary points are crease points.
					{
						aVertices[i].P=pV->GetBoundaryScalar(vecT, &CqWSurf::SubdP, this);
						if(uses_s && has_s)		aVertices[i].s=pV->GetBoundaryScalar(fT, &CqWSurf::Subds, this);
						if(uses_t && has_t)		aVertices[i].t=pV->GetBoundaryScalar(fT, &CqWSurf::Subdt, this);
						if(uses_Cs && has_Cs)	aVertices[i].Cq=pV->GetBoundaryScalar(colT, &CqWSurf::SubdCs, this);
						if(uses_Os && has_Os)	aVertices[i].Os=pV->GetBoundaryScalar(colT, &CqWSurf::SubdOs, this);
					}
				}
			}
		}
	}

	// Copy the modified points back to the surface.
	for(i=0; i<oldcVerts; i++)
	{
		CqSurface::P()[pVert(i)->iVertex()]=aVertices[i].P;
		if(uses_s && has_s)		CqSurface::s()[pVert(i)->iVertex()]=aVertices[i].s;
		if(uses_t && has_t)		CqSurface::t()[pVert(i)->iVertex()]=aVertices[i].t;
		if(uses_Cs && has_Cs)	CqSurface::Cs()[pVert(i)->iVertex()]=aVertices[i].Cq;
		if(uses_Os && has_Os)	CqSurface::Os()[pVert(i)->iVertex()]=aVertices[i].Os;
	}
}

//---------------------------------------------------------------------
/** Subdivide the surface, using Catmull Clark subdivision rules.
 */

void CqWSurf::Subdivide()
{
	TqInt i;
	static CqVector3D vecT;

	TqInt lUses=Uses();
	TqBool uses_s=USES(lUses,EnvVars_s);
	TqBool uses_t=USES(lUses,EnvVars_t);
	TqBool uses_Cs=USES(lUses,EnvVars_Cs);
	TqBool uses_Os=USES(lUses,EnvVars_Os);

	TqBool has_s=s().Size()>=P().Size();
	TqBool has_t=t().Size()>=P().Size();
	TqBool has_Cs=Cs().Size()>=P().Size();
	TqBool has_Os=Os().Size()>=P().Size();
	
	// Create an array big enough to hold all the additional points to be created.
	TqInt newcVerts=CqSurface::P().Size();
	TqInt oldcVerts=CqSurface::P().Size();
	newcVerts+=cFaces();
	newcVerts+=cEdges();

	CqSurface::P().SetSize(newcVerts);
	if(uses_s && has_s)		CqSurface::s().SetSize(newcVerts);
	if(uses_t && has_t)		CqSurface::t().SetSize(newcVerts);
	if(uses_Cs && has_Cs)	CqSurface::Cs().SetSize(newcVerts);
	if(uses_Os && has_Os)	CqSurface::Os().SetSize(newcVerts);

	m_apVerts.reserve(cVerts()+cFaces()+cEdges());
	m_apFaces.reserve(cFaces()+(cFaces()*4));
	m_apEdges.reserve(cEdges()+(cEdges()*2));

	TqInt index=oldcVerts;

	TqInt ifT=cFaces();
	TqInt ieT=cEdges();

	// Create face points.
	for(i=0; i<ifT; i++)
		pFace(i)->CreateSubdividePoint(this, index++, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);

	// Create edge points.
	for(i=0; i<ieT; i++)
		pEdge(i)->CreateSubdividePoint(this, index++, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);

	// Smooth vertex points
	SmoothVertexPoints(oldcVerts, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);
	
	// Create new edges.
	for(i=0; i<ieT; i++)
		pEdge(i)->Subdivide(this);

	// Create new faces
	std::vector<CqWEdge*> aEdges;
	for(i=0; i<ifT; i++)
	{
		TqInt j;
		// Build all internal edges, external subdivided edges are built by WEdge structures themselves.
		CqWReference grE(pFace(i)->pEdge(0), pFace(i));
		CqWReference grE2;
		aEdges.resize(pFace(i)->cEdges());
		for(j=0; j<pFace(i)->cEdges(); j++)
		{
			aEdges[j]=new CqWEdge;
			aEdges[j]->SetpvHead(pFace(i)->pvSubdivide());
			aEdges[j]->SetpvTail(grE.peCurrent()->pvSubdivide());
			AddEdge(aEdges[j]);
			
			pFace(i)->pvSubdivide()->AddEdge(aEdges[j]);
			grE.peCurrent()->pvSubdivide()->AddEdge(aEdges[j]);
			
			grE.peNext();
		}

		// Now build the faces, building the wings as we go.
		grE.Reset(pFace(i)->pEdge(0), pFace(i));
		grE2.Reset(grE.peCurrent(), pFace(i));
		// Point another intelligent reference to the next external edge.
		grE2.pePrev();
		CqWEdge* peI2;
		for(j=0; j<pFace(i)->cEdges(); j++)
		{
			// Find the next internal edge.
			if(j==0)	peI2=aEdges[pFace(i)->cEdges()-1];	
			else		peI2=aEdges[j-1];
			
			// Create a new face to be filled in.
			CqWFace* pfNew=new CqWFace;	
			
			// Add the edges and set the edge facet references.
			pfNew->AddEdge(grE.peHeadHalf());				grE.SetpfHeadLeft(pfNew);
			pfNew->AddEdge(aEdges[j]);						aEdges[j]->SetpfRight(pfNew);
			pfNew->AddEdge(peI2);							peI2->SetpfLeft(pfNew);			
			pfNew->AddEdge(grE2.peTailHalf());				grE2.SetpfTailLeft(pfNew);
			// Set up wing data for the edges.
			grE.SetpeHeadTailLeft(aEdges[j]);				grE.SetpeHeadHeadLeft(grE2.peTailHalf());
			aEdges[j]->SetpeTailRight(grE.peHeadHalf());	aEdges[j]->SetpeHeadRight(peI2);
			grE2.SetpeTailHeadLeft(peI2);					grE2.SetpeTailTailLeft(grE.peHeadHalf());
			peI2->SetpeTailLeft(grE2.peTailHalf());			peI2->SetpeHeadLeft(aEdges[j]);
			// Add the face.
			AddFace(pfNew);

			grE.peNext();
			grE2.peNext();
		}		
	}

	for(i=0; i<ifT; i++)	
		delete(pFace(i));

	for(i=0; i<ieT; i++)	
	{
		pEdge(i)->pvHead()->RemoveEdge(pEdge(i));
		pEdge(i)->pvTail()->RemoveEdge(pEdge(i));
		delete(pEdge(i));
	}
	m_apFaces.erase(m_apFaces.begin(), m_apFaces.begin()+ifT);
	m_apEdges.erase(m_apEdges.begin(), m_apEdges.begin()+ieT);
}


//---------------------------------------------------------------------
/** Split the surface into smaller patches
 */

TqInt CqWSurf::Split(std::vector<CqBasicSurface*>& aSplits)
{
	int i;
	for(i=0; i<cFaces(); i++)
	{
		CqSubdivisionPatch* pNew=new CqSubdivisionPatch(this,i);
		pNew->SetSurfaceParameters(*this);
		pNew->m_fDiceable=TqTrue;
		pNew->m_EyeSplitCount=m_EyeSplitCount;
		aSplits.push_back(pNew);
	}
	return(cFaces());
}

//---------------------------------------------------------------------
/** Transform the control hull by the specified matrix.
 */

void CqWSurf::Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx)
{
	// Tansform the control hull by the specified matrix.
	TqInt i;
	for(i=0; i<P().Size(); i++)
		P()[i]=matTx*P()[i];
}


//---------------------------------------------------------------------
/** Create a new C4D polygonobject from this winged edge surface.
 */

void CqWSurf::_OutputMesh(char* pname)
{
	FILE* pf=fopen(pname,"w");
	TqInt i;
	for(i=0; i<cFaces(); i++)
	{
		CqWReference ref(pFace(i)->pEdge(0),pFace(i));
		CqVector3D vecA=SubdP(ref.pvHead()->iVertex());
		ref.peNext();
		TqInt j=1;
		while(j<pFace(i)->cEdges())
		{
			CqVector3D	vecB,vecC;
			vecB=SubdP(ref.pvHead()->iVertex());
			vecC=SubdP(ref.pvTail()->iVertex());

			fprintf(pf,"%f %f %f " ,vecA.x(),vecA.y(),vecA.z());
			fprintf(pf,"%f %f %f " ,vecB.x(),vecB.y(),vecB.z());
			fprintf(pf,"%f %f %f\n",vecC.x(),vecC.y(),vecC.z());

			ref.peNext();
			j++;
		}
	}
	fclose(pf);
}


//---------------------------------------------------------------------
/** Reset this reference to the specifed edge on the specified face.
 */

void	CqWReference::Reset(CqWEdge* pE, CqWFace* pF)
{
	m_pEdge=pE;
	m_pFace=pF;

	if(pE!=0 && pF!=0)
	{
		if(pE->pfRight()!=pF && pE->pfLeft()!=pF)
		{
			//throw("Error : Tried to create an edge reference with an invalid face/edge pair");
			return;
		}
		m_bLeft=TqFalse;
		if(m_pEdge->pfLeft()==m_pFace)	m_bLeft=TqTrue;
	}
}


//---------------------------------------------------------------------
/** Get the next edge of the coupled face, taking into account orientation.
 */

CqWReference& CqWReference::peNext()
{
	if(m_bLeft)	m_pEdge=m_pEdge->peTailLeft();
	else		m_pEdge=m_pEdge->peHeadRight();
		
	m_bLeft=TqFalse;
	if(m_pEdge->pfLeft()==m_pFace)	m_bLeft=TqTrue;

	return(*this);
}


//---------------------------------------------------------------------
/** Get the previous edge of the coupled face, taking into account orientation.
 */

CqWReference& CqWReference::pePrev()
{
	if(m_bLeft)	m_pEdge=m_pEdge->peHeadLeft();
	else		m_pEdge=m_pEdge->peTailRight();

	m_bLeft=TqFalse;
	if(m_pEdge->pfLeft()==m_pFace)	m_bLeft=TqTrue;

	return(*this);
}

//---------------------------------------------------------------------
/** Get the previous edge of the coupled face, taking into account orientation.
 */

CqWEdge* CqWReference::peHeadRight()
{
	if(m_bLeft)	return(m_pEdge->peHeadRight());
	else		return(m_pEdge->peTailLeft());
}


//---------------------------------------------------------------------
/** Get the previous edge of the coupled face, taking into account orientation.
 */

CqWEdge* CqWReference::peHeadLeft()
{
	if(m_bLeft)	return(m_pEdge->peHeadLeft());
	else		return(m_pEdge->peTailRight());
}


//---------------------------------------------------------------------
/** Get the previous edge of the coupled face, taking into account orientation.
 */

CqWEdge* CqWReference::peTailRight()
{
	if(m_bLeft)	return(m_pEdge->peTailRight());
	else		return(m_pEdge->peHeadLeft());
}


//---------------------------------------------------------------------
/** Get the previous edge of the coupled face, taking into account orientation.
 */

CqWEdge* CqWReference::peTailLeft()
{
	if(m_bLeft)	return(m_pEdge->peTailLeft());
	else		return(m_pEdge->peHeadRight());
}


//---------------------------------------------------------------------
/** Get the head vertex of this edge, taking into account orientation with respect to the coupled face.
 */

CqWVert* CqWReference::pvHead()
{
	if(m_bLeft)	return(m_pEdge->pvHead());
	else		return(m_pEdge->pvTail());
}


//---------------------------------------------------------------------
/** Get the tail vertex of this edge, taking into account orientation with respect to the coupled face.
 */

CqWVert* CqWReference::pvTail()
{
	if(m_bLeft)	return(m_pEdge->pvTail());
	else		return(m_pEdge->pvHead());
}


//---------------------------------------------------------------------
/** Get the subdivided head half edge of this edge, taking into account orientation with respect to the coupled face.
 */

CqWEdge* CqWReference::peHeadHalf()
{
	if(m_bLeft)	return(m_pEdge->peHeadHalf());
	else		return(m_pEdge->peTailHalf());
}


//---------------------------------------------------------------------
/** Get the subdivided tail half edge of this edge, taking into account orientation with respect to the coupled face.
 */

CqWEdge* CqWReference::peTailHalf()
{
	if(m_bLeft)	return(m_pEdge->peTailHalf());
	else		return(m_pEdge->peHeadHalf());
}


//---------------------------------------------------------------------
/** Set the left face reference for the subdivided tail half of this edge, 
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpfTailLeft(CqWFace* pfTailLeft)
{
	if(m_bLeft)	m_pEdge->peTailHalf()->SetpfLeft(pfTailLeft);
	else		m_pEdge->peHeadHalf()->SetpfRight(pfTailLeft);
}


//---------------------------------------------------------------------
/** Set the left face reference for the subdivided head half of this edge, 
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpfHeadLeft(CqWFace* pfHeadLeft)
{
	if(m_bLeft)	m_pEdge->peHeadHalf()->SetpfLeft(pfHeadLeft);
	else		m_pEdge->peTailHalf()->SetpfRight(pfHeadLeft);
}


//---------------------------------------------------------------------
/** Set the right face reference for the subdivided tail half of this edge, 
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpfTailRight(CqWFace* pfTailRight)
{
	if(m_bLeft)	m_pEdge->peTailHalf()->SetpfRight(pfTailRight);
	else		m_pEdge->peHeadHalf()->SetpfLeft(pfTailRight);
}


//---------------------------------------------------------------------
/** Set the right face reference for the subdivided head half of this edge, 
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpfHeadRight(CqWFace* pfHeadRight)
{
	if(m_bLeft)	m_pEdge->peHeadHalf()->SetpfRight(pfHeadRight);
	else		m_pEdge->peTailHalf()->SetpfLeft(pfHeadRight);
}


//---------------------------------------------------------------------
/** Set the winged edge tail left reference of the tail half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeTailTailLeft(CqWEdge* pe)
{
	if(m_bLeft)	m_pEdge->peTailHalf()->SetpeTailLeft(pe);
	else		m_pEdge->peHeadHalf()->SetpeHeadRight(pe);
}


//---------------------------------------------------------------------
/** Set the winged edge head left reference of the tail half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeTailHeadLeft(CqWEdge* pe)
{
	if(m_bLeft)	m_pEdge->peTailHalf()->SetpeHeadLeft(pe);
	else		m_pEdge->peHeadHalf()->SetpeTailRight(pe);
}


//---------------------------------------------------------------------
/** Set the winged edge tail right reference of the tail half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeTailTailRight(CqWEdge* pe)
{
	if(m_bLeft)	m_pEdge->peTailHalf()->SetpeTailRight(pe);
	else		m_pEdge->peHeadHalf()->SetpeHeadLeft(pe);
}


//---------------------------------------------------------------------
/** Set the winged edge head right reference of the tail half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeTailHeadRight(CqWEdge* pe)
{
	if(m_bLeft)	m_pEdge->peTailHalf()->SetpeTailRight(pe);
	else		m_pEdge->peHeadHalf()->SetpeHeadLeft(pe);
}


//---------------------------------------------------------------------
/** Set the winged edge tail left reference of the head half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeHeadTailLeft(CqWEdge* pe)
{
	if(m_bLeft)	m_pEdge->peHeadHalf()->SetpeTailLeft(pe);
	else		m_pEdge->peTailHalf()->SetpeHeadRight(pe);
}


//---------------------------------------------------------------------
/** Set the winged edge head left reference of the head half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeHeadHeadLeft(CqWEdge* pe)
{
	if(m_bLeft)	m_pEdge->peHeadHalf()->SetpeHeadLeft(pe);
	else		m_pEdge->peTailHalf()->SetpeTailRight(pe);
}


//---------------------------------------------------------------------
/** Set the winged edge tail right reference of the head half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeHeadTailRight(CqWEdge* pe)
{
	if(m_bLeft)	m_pEdge->peHeadHalf()->SetpeTailRight(pe);
	else		m_pEdge->peTailHalf()->SetpeHeadLeft(pe);
}


//---------------------------------------------------------------------
/** Set the winged edge head right reference of the head half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeHeadHeadRight(CqWEdge* pe)
{
	if(m_bLeft)	m_pEdge->peHeadHalf()->SetpeTailRight(pe);
	else		m_pEdge->peTailHalf()->SetpeHeadLeft(pe);
}


//---------------------------------------------------------------------
/** Constructor
 */

CqSubdivisionPatch::CqSubdivisionPatch(CqWSurf* pSurf, TqInt iFace)
{
	TqInt lUses=pSurf->Uses();
	TqBool uses_s=USES(lUses,EnvVars_s);
	TqBool uses_t=USES(lUses,EnvVars_t);
	TqBool uses_Cs=USES(lUses,EnvVars_Cs);
	TqBool uses_Os=USES(lUses,EnvVars_Os);

	TqBool has_s=pSurf->s().Size()>=pSurf->P().Size();
	TqBool has_t=pSurf->t().Size()>=pSurf->P().Size();
	TqBool has_Cs=pSurf->Cs().Size()>=pSurf->P().Size();
	TqBool has_Os=pSurf->Os().Size()>=pSurf->P().Size();

	// Copy the donor face and all of its neghbours into our local face storage.
	CqWFace* pF=pSurf->pFace(iFace);
	TqInt i;
	std::vector<CqWEdge*> apEdges;

	// Initialise the P() array to a sensible size first.
	P().SetSize(0);

	if(pSurf->Cs().Size()==1)	Cs()=pSurf->Cs();
	else						Cs().SetSize(0);
	if(pSurf->Os().Size()==1)	Os()=pSurf->Os();
	else						Os().SetSize(0);

	// Add the main face by adding each edge, by adding each vertex.
	CqWReference rEdge(pF->pEdge(0),pF);
	apEdges.resize(pF->cEdges());
	for(i=0; i<pF->cEdges(); i++)
	{
		CqWVert* pvA=AddVert(pSurf, rEdge.pvHead()->iVertex(), uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);
		CqWVert* pvB=AddVert(pSurf, rEdge.pvTail()->iVertex(), uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);

		apEdges[i]=AddEdge(pvA,pvB);
		apEdges[i]->SetSharpness(rEdge.peCurrent()->Sharpness());

		rEdge.peNext();
	}
	// Now add the facet
	AddFace(&apEdges[0],pF->cEdges());

	// Now do the same for each surrounding face.
	CqWEdge* peStart=0;
	rEdge.Reset(pF->pEdge(0),pF);

	for(i=0; i<pF->cEdges(); i++)
	{
		CqWVert* pvTail=rEdge.pvTail();
		rEdge.peNext();
		TqInt j;
		for(j=0; j<pvTail->cEdges(); j++)
		{
			CqWEdge* peCurr=pvTail->pEdge(j);
			// Only if this edge is not a part of the main facet.
			if(peCurr->pfLeft()!=pF && peCurr->pfRight()!=pF)
			{
				CqWFace* pF2=(peCurr->pvTail()==pvTail)?peCurr->pfRight():peCurr->pfLeft();
				if(pF2!=NULL)
				{
					CqWReference rEdge2(pF2->pEdge(0),pF2);
					apEdges.resize(pF2->cEdges());
					TqInt e;
					for(e=0; e<pF2->cEdges(); e++)
					{
						CqWVert* pvA=AddVert(pSurf, rEdge2.pvHead()->iVertex(), uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);
						CqWVert* pvB=AddVert(pSurf, rEdge2.pvTail()->iVertex(), uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);

						apEdges[e]=AddEdge(pvA,pvB);
						apEdges[e]->SetSharpness(rEdge2.peCurrent()->Sharpness());

						rEdge2.peNext();
					}
					// Now add the facet
					AddFace(&apEdges[0],pF2->cEdges());
				}
			}
		}
	}
}


CqWVert* CqSubdivisionPatch::AddVert(CqWSurf* pSurf, TqInt iVert, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os,
																  TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os)
{
	CqWVert* pNew=CqWSurf::AddVert(pSurf->SubdP(iVert));

	TqInt iV=pNew->iVertex();
	if(uses_s && has_s)	
	{
		if(s().Size()<=iV)	s().SetSize(iV+1);
		s()[iV]=pSurf->s()[iVert];
	}

	if(uses_t && has_t)	
	{
		if(t().Size()<=iV)	t().SetSize(iV+1);
		t()[iV]=pSurf->t()[iVert];
	}

	if(uses_Cs && has_Cs)	
	{
		if(Cs().Size()<=iV)	Cs().SetSize(iV+1);
		Cs()[iV]=pSurf->Cs()[iVert];
	}

	if(uses_Os && has_Os)	
	{
		if(Os().Size()<=iV)	Os().SetSize(iV+1);
		Os()[iV]=pSurf->Os()[iVert];
	}

	return(pNew);
}


//---------------------------------------------------------------------
/** Subdivide the surface, using Catmull Clark subdivision rules.
 * This is a specialised version for use during dicing, makes some assumptions.
 * Must be a quad based patch, i.e. at least one split if more that 4 points.
 * Produces the vertices with indexes usable for creating a MP grid.
 */

void CqSubdivisionPatch::DiceSubdivide()
{
	//assert(pFace(0)->cEdges()==4);
	TqInt i;
	static CqVector3D vecT;

	TqInt lUses=Uses();
	TqBool uses_s=USES(lUses,EnvVars_s);
	TqBool uses_t=USES(lUses,EnvVars_t);
	TqBool uses_Cs=USES(lUses,EnvVars_Cs);
	TqBool uses_Os=USES(lUses,EnvVars_Os);

	TqBool has_s=s().Size()>=P().Size();
	TqBool has_t=t().Size()>=P().Size();
	TqBool has_Cs=Cs().Size()>=P().Size();
	TqBool has_Os=Os().Size()>=P().Size();
	
	// NOTE: Not entirely happy about this method, would prefer a more efficient approach!
	// Must create this array here, to ensure we only store the old points, not the subdivided ones.
	std::vector<CqVector3D> aVertices;
	aVertices.resize(cVerts());

	// Create an array big enough to hold all the additional points to be created.
	TqInt newcVerts=CqSurface::P().Size();
	TqInt oldcVerts=CqSurface::P().Size();
	newcVerts+=cFaces();
	newcVerts+=cEdges();

	CqSurface::P().SetSize(newcVerts);
	if(uses_s && has_s)		CqSurface::s().SetSize(newcVerts);
	if(uses_t && has_t)		CqSurface::t().SetSize(newcVerts);
	if(uses_Cs && has_Cs)	CqSurface::Cs().SetSize(newcVerts);
	if(uses_Os && has_Os)	CqSurface::Os().SetSize(newcVerts);
	
	m_apVerts.reserve(cVerts()+cFaces()+cEdges());
	m_apFaces.reserve(cFaces()*4);
	m_apEdges.reserve(cEdges()*2);

	TqInt index=oldcVerts;

	// Keep a count of faces and edges to remove them after subdivision.
	TqInt ifT=cFaces();
	TqInt ieT=cEdges();
	
	// Create face points.
	for(i=0; i<ifT; i++)
		pFace(i)->CreateSubdividePoint(this, index++, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);

	// Create edge points.
	for(i=0; i<ieT; i++)
		pEdge(i)->CreateSubdividePoint(this, index++, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);

	// Smooth vertex points
	SmoothVertexPoints(oldcVerts, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);
	

	// Create new edges.
	for(i=0; i<ieT; i++)
		pEdge(i)->Subdivide(this);

	// Create new faces
	std::vector<CqWEdge*> aEdges;
	for(i=0; i<ifT; i++)
	{
		TqInt j;
		// Build all internal edges, external subdivided edges are built by WEdge structures themselves.
		CqWReference grE(pFace(i)->pEdge(0), pFace(i));
		CqWReference grE2;
		aEdges.resize(pFace(i)->cEdges());
		for(j=0; j<pFace(i)->cEdges(); j++)
		{
			aEdges[j]=new CqWEdge;
			aEdges[j]->SetpvHead(pFace(i)->pvSubdivide());
			aEdges[j]->SetpvTail(grE.peCurrent()->pvSubdivide());
			AddEdge(aEdges[j]);
			
			pFace(i)->pvSubdivide()->AddEdge(aEdges[j]);
			grE.peCurrent()->pvSubdivide()->AddEdge(aEdges[j]);
			
			grE.peNext();
		}

		// Now build the faces, building the wings as we go.
		grE.Reset(pFace(i)->pEdge(0), pFace(i));
		// Point another intelligent reference to the next external edge.
		grE2.Reset(pFace(i)->pEdge(0), pFace(i));
		grE2.pePrev();

		CqWEdge* peI1,*peI2;
		CqWFace* pfNew;

		// Face 1
		peI1=aEdges[0];
		peI2=aEdges[pFace(i)->cEdges()-1];
		pfNew=new CqWFace;
		// Add the edges and set the edge facet references.
		pfNew->AddEdge(grE.peHeadHalf());				grE.SetpfHeadLeft(pfNew);
		pfNew->AddEdge(peI1);							peI1->SetpfRight(pfNew);
		pfNew->AddEdge(peI2);							peI2->SetpfLeft(pfNew);			
		pfNew->AddEdge(grE2.peTailHalf());				grE2.SetpfTailLeft(pfNew);
		// Set up wing data for the edges.
		grE.SetpeHeadTailLeft(peI1);					grE.SetpeHeadHeadLeft(grE2.peTailHalf());
		peI1->SetpeTailRight(grE.peHeadHalf());			peI1->SetpeHeadRight(peI2);
		grE2.SetpeTailHeadLeft(peI2);					grE2.SetpeTailTailLeft(grE.peHeadHalf());
		peI2->SetpeTailLeft(grE2.peTailHalf());			peI2->SetpeHeadLeft(peI1);
		// Add the face.
		AddFace(pfNew);
		grE.peNext();
		grE2.peNext();

		// Face 2
		peI1=aEdges[1];
		peI2=aEdges[0];
		pfNew=new CqWFace;
		// Add the edges and set the edge facet references.
		pfNew->AddEdge(grE2.peTailHalf());				grE2.SetpfTailLeft(pfNew);
		pfNew->AddEdge(grE.peHeadHalf());				grE.SetpfHeadLeft(pfNew);
		pfNew->AddEdge(peI1);							peI1->SetpfRight(pfNew);			
		pfNew->AddEdge(peI2);							peI2->SetpfLeft(pfNew);
		// Set up wing data for the edges.
		grE.SetpeHeadTailLeft(peI1);					grE.SetpeHeadHeadLeft(grE2.peTailHalf());
		peI1->SetpeTailRight(grE.peHeadHalf());			peI1->SetpeHeadRight(peI2);
		grE2.SetpeTailHeadLeft(peI2);					grE2.SetpeTailTailLeft(grE.peHeadHalf());
		peI2->SetpeTailLeft(grE2.peTailHalf());			peI2->SetpeHeadLeft(peI1);
		// Add the face.
		AddFace(pfNew);
		grE.peNext();
		grE2.peNext();

		// Face 3
		peI1=aEdges[2];
		peI2=aEdges[1];
		pfNew=new CqWFace;
		// Add the edges and set the edge facet references.
		pfNew->AddEdge(peI2);							peI2->SetpfLeft(pfNew);
		pfNew->AddEdge(grE2.peTailHalf());				grE2.SetpfTailLeft(pfNew);
		pfNew->AddEdge(grE.peHeadHalf());				grE.SetpfHeadLeft(pfNew);
		pfNew->AddEdge(peI1);							peI1->SetpfRight(pfNew);			
		// Set up wing data for the edges.
		grE.SetpeHeadTailLeft(peI1);					grE.SetpeHeadHeadLeft(grE2.peTailHalf());
		peI1->SetpeTailRight(grE.peHeadHalf());			peI1->SetpeHeadRight(peI2);
		grE2.SetpeTailHeadLeft(peI2);					grE2.SetpeTailTailLeft(grE.peHeadHalf());
		peI2->SetpeTailLeft(grE2.peTailHalf());			peI2->SetpeHeadLeft(peI1);
		// Add the face.
		AddFace(pfNew);
		grE.peNext();
		grE2.peNext();

		// Face 4
		peI1=aEdges[3];
		peI2=aEdges[2];
		pfNew=new CqWFace;
		// Add the edges and set the edge facet references.
		pfNew->AddEdge(peI1);							peI1->SetpfRight(pfNew);			
		pfNew->AddEdge(peI2);							peI2->SetpfLeft(pfNew);
		pfNew->AddEdge(grE2.peTailHalf());				grE2.SetpfTailLeft(pfNew);
		pfNew->AddEdge(grE.peHeadHalf());				grE.SetpfHeadLeft(pfNew);
		// Set up wing data for the edges.
		grE.SetpeHeadTailLeft(peI1);					grE.SetpeHeadHeadLeft(grE2.peTailHalf());
		peI1->SetpeTailRight(grE.peHeadHalf());			peI1->SetpeHeadRight(peI2);
		grE2.SetpeTailHeadLeft(peI2);					grE2.SetpeTailTailLeft(grE.peHeadHalf());
		peI2->SetpeTailLeft(grE2.peTailHalf());			peI2->SetpeHeadLeft(peI1);
		// Add the face.
		AddFace(pfNew);
	}

	for(i=0; i<ifT; i++)	
		delete(pFace(i));

	for(i=0; i<ieT; i++)	
	{
		pEdge(i)->pvHead()->RemoveEdge(pEdge(i));
		pEdge(i)->pvTail()->RemoveEdge(pEdge(i));
		delete(pEdge(i));
	}
	m_apFaces.erase(m_apFaces.begin(), m_apFaces.begin()+ifT);
	m_apEdges.erase(m_apEdges.begin(), m_apEdges.begin()+ieT);
}


//---------------------------------------------------------------------
/** Determine whether the patch can be diced based on its screen size, if so work
 * out how many subdivisions to perform to get a MP grid and store it.
 */

CqBound CqWSurf::Bound() const
{
	// Get the boundary in camera space.
	CqVector3D	vecA(FLT_MAX, FLT_MAX, FLT_MAX);
	CqVector3D	vecB(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	TqInt i;
	for(i=0; i<P().Size(); i++)
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


// Round x up to the nearest power of 2.

static unsigned upto2 (unsigned x)
{
    // if x is already a power of 2, great!  Return it.
    if ((x & (x - 1)) == 0)
	return x;
    return 1 << (int)(_logb ((double)x) + 1);
}

//---------------------------------------------------------------------
/** Determine whether the patch can be diced based on its screen size, if so work
 * out how many subdivisions to perform to get a MP grid and store it.
 */

TqBool CqSubdivisionPatch::Diceable()
{
	// Fail if not a quad patch
	TqInt iF;
	for(iF=0; iF<cFaces(); iF++)
		if(pFace(iF)->cEdges()!=4)	return(TqFalse);

	const CqMatrix& matCtoR=pCurrentRenderer()->matSpaceToSpace("camera","raster");

	// Get the sides of the main quad (the first if everything goes according to plan.
	CqVector3D vecA=SubdP(pFace(0)->pEdge(0)->pvHead()->iVertex());
	CqVector3D vecB=SubdP(pFace(0)->pEdge(1)->pvHead()->iVertex());
	CqVector3D vecC=SubdP(pFace(0)->pEdge(2)->pvHead()->iVertex());
	CqVector3D vecD=SubdP(pFace(0)->pEdge(3)->pvHead()->iVertex());

	vecA=matCtoR*vecA;
	vecB=matCtoR*vecB;
	vecC=matCtoR*vecC;
	vecD=matCtoR*vecD;

	TqFloat lA=(vecB-vecA).Magnitude2();
	TqFloat lB=(vecC-vecB).Magnitude2();
	TqFloat lC=(vecD-vecC).Magnitude2();
	TqFloat lD=(vecA-vecD).Magnitude2();

	TqFloat l=MAX(lA,MAX(lB,(MAX(lC,lD))));

	l=sqrt(l);

	// Get the shading rate.
	float ShadingRate=pAttributes()->fEffectiveShadingRate();
//	if(pCurrentRenderer()->Mode()==RenderMode_Shadows)
//	{
//		const TqFloat* pattrShadowShadingRate=m_pAttributes->GetFloatAttribute("render","shadow_shadingrate");
//		if(pattrShadowShadingRate!=0)
//			ShadingRate=pattrShadowShadingRate[0];
//	}	
	l/=ShadingRate;

	if(l>16)	return(TqFalse);
	else			
	{
		m_DiceCount=l;
		m_DiceCount=upto2(m_DiceCount);
		m_DiceCount=(m_DiceCount==16)?4:(m_DiceCount==8)?3:(m_DiceCount==4)?2:(m_DiceCount==2)?1:0;
		return(TqTrue);
	}
}


//---------------------------------------------------------------------
/** Split the patch into a further 4 smaller patches using subdivision.
 */

TqInt CqSubdivisionPatch::Split(std::vector<CqBasicSurface*>& aSplits)
{
	// Split the surface in u or v
	TqInt iE=pFace(0)->cEdges();
	
	Subdivide();

	TqInt i;
	for(i=0; i<iE; i++)
	{
		CqSubdivisionPatch* pNewA=new CqSubdivisionPatch(this,i);
		pNewA->SetSurfaceParameters(*this);
		pNewA->m_fDiceable=TqTrue;
		pNewA->m_EyeSplitCount=m_EyeSplitCount;
		aSplits.push_back(pNewA);
	}
	return(iE);
}


//---------------------------------------------------------------------
/** Dice the patch into a micropolygon grid for shading and rendering.
 */

CqMicroPolyGridBase* CqSubdivisionPatch::Dice()
{
	// Create a new CqMicroPolyGrid for this patch
	TqInt cuv=(1<<m_DiceCount);
	CqMicroPolyGrid* pGrid=new CqMicroPolyGrid(cuv, cuv, this);
	
	TqInt lUses=Uses();
	TqBool uses_s=USES(lUses,EnvVars_s);
	TqBool uses_t=USES(lUses,EnvVars_t);
	TqBool uses_Cs=USES(lUses,EnvVars_Cs);
	TqBool uses_Os=USES(lUses,EnvVars_Os);

	TqBool has_s=s().Size()>=P().Size();
	TqBool has_t=t().Size()>=P().Size();
	TqBool has_Cs=Cs().Size()>=P().Size();
	TqBool has_Os=Os().Size()>=P().Size();

	if(uses_Cs && !has_Cs)	Cs().BilinearDice(cuv,cuv,&pGrid->Cs());
	if(uses_Os && !has_Os)	Os().BilinearDice(cuv,cuv,&pGrid->Os());

	DiceSubdivide(m_DiceCount);

	TqInt NumMPs=static_cast<TqInt>(pow(4,m_DiceCount));
	TqInt iU=0, iV=0;

	TqInt iFace=0;
	StoreDice(m_DiceCount, iFace, 0, 0, cuv+1, pGrid, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);
	// Only shade if the ImageBuffer mode is at least RGB
	if(pCurrentRenderer()->optCurrent().iDisplayMode()&ModeRGB)
		pGrid->Shade();

	return(pGrid);
}


void CqSubdivisionPatch::StoreDice(TqInt Level, TqInt& iFace, TqInt uOff, TqInt vOff, TqInt cuv, CqMicroPolyGrid* pGrid,
												TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os,
												TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os)
{
	CqWFace* pF;
	CqWReference rE;
	
	if(Level>1)
		StoreDice(Level-1,iFace,uOff,vOff,cuv,pGrid, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);
	else
	{
		pF=pFace(iFace++);
		rE.Reset(pF->pEdge(0), pF);
		TqInt ivA=rE.pvHead()->iVertex();
		TqInt ivB=rE.peNext().pvHead()->iVertex();
		TqInt ivC=rE.peNext().pvHead()->iVertex();
		TqInt ivD=rE.peNext().pvHead()->iVertex();

		pGrid->P()[((vOff  )*cuv)+uOff  ]=SubdP(ivA);
		pGrid->P()[((vOff  )*cuv)+uOff+1]=SubdP(ivB);	
		pGrid->P()[((vOff+1)*cuv)+uOff+1]=SubdP(ivC);
		pGrid->P()[((vOff+1)*cuv)+uOff  ]=SubdP(ivD);

		if(uses_s && has_s)		
		{
			pGrid->s()[((vOff  )*cuv)+uOff  ]=Subds(ivA);
			pGrid->s()[((vOff  )*cuv)+uOff+1]=Subds(ivB);	
			pGrid->s()[((vOff+1)*cuv)+uOff+1]=Subds(ivC);
			pGrid->s()[((vOff+1)*cuv)+uOff  ]=Subds(ivD);
		}
		if(uses_t && has_t)		
		{
			pGrid->t()[((vOff  )*cuv)+uOff  ]=Subdt(ivA);
			pGrid->t()[((vOff  )*cuv)+uOff+1]=Subdt(ivB);	
			pGrid->t()[((vOff+1)*cuv)+uOff+1]=Subdt(ivC);
			pGrid->t()[((vOff+1)*cuv)+uOff  ]=Subdt(ivD);
		}
		if(uses_Cs && has_Cs)	
		{
			pGrid->Cs()[((vOff  )*cuv)+uOff  ]=SubdCs(ivA);
			pGrid->Cs()[((vOff  )*cuv)+uOff+1]=SubdCs(ivB);	
			pGrid->Cs()[((vOff+1)*cuv)+uOff+1]=SubdCs(ivC);
			pGrid->Cs()[((vOff+1)*cuv)+uOff  ]=SubdCs(ivD);
		}
		if(uses_Os && has_Os)	
		{
			pGrid->Os()[((vOff  )*cuv)+uOff  ]=SubdOs(ivA);
			pGrid->Os()[((vOff  )*cuv)+uOff+1]=SubdOs(ivB);	
			pGrid->Os()[((vOff+1)*cuv)+uOff+1]=SubdOs(ivC);
			pGrid->Os()[((vOff+1)*cuv)+uOff  ]=SubdOs(ivD);
		}
	}

	uOff+=1<<(Level-1);
	if(Level>1)
		StoreDice(Level-1,iFace,uOff,vOff,cuv,pGrid, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);
	else
	{
		pF=pFace(iFace++);
		rE.Reset(pF->pEdge(1), pF);
		TqInt ivB=rE.pvHead()->iVertex();
		TqInt ivC=rE.peNext().pvHead()->iVertex();

		pGrid->P()[((vOff  )*cuv)+uOff+1]=SubdP(ivB);	
		pGrid->P()[((vOff+1)*cuv)+uOff+1]=SubdP(ivC);

		if(uses_s && has_s)		
		{
			pGrid->s()[((vOff  )*cuv)+uOff+1]=Subds(ivB);	
			pGrid->s()[((vOff+1)*cuv)+uOff+1]=Subds(ivC);
		}
		if(uses_t && has_t)		
		{
			pGrid->t()[((vOff  )*cuv)+uOff+1]=Subdt(ivB);	
			pGrid->t()[((vOff+1)*cuv)+uOff+1]=Subdt(ivC);
		}
		if(uses_Cs && has_Cs)	
		{
			pGrid->Cs()[((vOff  )*cuv)+uOff+1]=SubdCs(ivB);	
			pGrid->Cs()[((vOff+1)*cuv)+uOff+1]=SubdCs(ivC);
		}
		if(uses_Os && has_Os)	
		{
			pGrid->Os()[((vOff  )*cuv)+uOff+1]=SubdOs(ivB);	
			pGrid->Os()[((vOff+1)*cuv)+uOff+1]=SubdOs(ivC);
		}
	}

	vOff+=1<<(Level-1);
	if(Level>1)
		StoreDice(Level-1,iFace,uOff,vOff,cuv,pGrid, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);
	else
	{
		pF=pFace(iFace++);
		rE.Reset(pF->pEdge(2), pF);
		TqInt ivC=rE.pvHead()->iVertex();
		TqInt ivD=rE.peNext().pvHead()->iVertex();

		pGrid->P()[((vOff+1)*cuv)+uOff+1]=SubdP(ivC);	
		pGrid->P()[((vOff+1)*cuv)+uOff  ]=SubdP(ivD);

		if(uses_s && has_s)		
		{
			pGrid->s()[((vOff+1)*cuv)+uOff+1]=Subds(ivC);	
			pGrid->s()[((vOff+1)*cuv)+uOff  ]=Subds(ivD);
		}
		if(uses_t && has_t)		
		{
			pGrid->t()[((vOff+1)*cuv)+uOff+1]=Subdt(ivC);	
			pGrid->t()[((vOff+1)*cuv)+uOff  ]=Subdt(ivD);
		}
		if(uses_Cs && has_Cs)	
		{
			pGrid->Cs()[((vOff+1)*cuv)+uOff+1]=SubdCs(ivC);	
			pGrid->Cs()[((vOff+1)*cuv)+uOff  ]=SubdCs(ivD);
		}
		if(uses_Os && has_Os)	
		{
			pGrid->Os()[((vOff+1)*cuv)+uOff+1]=SubdOs(ivC);	
			pGrid->Os()[((vOff+1)*cuv)+uOff  ]=SubdOs(ivD);
		}
	}

	uOff-=1<<(Level-1);
	if(Level>1)
		StoreDice(Level-1,iFace,uOff,vOff,cuv,pGrid, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os);
	else
	{
		pF=pFace(iFace++);
		rE.Reset(pF->pEdge(3), pF);
		TqInt ivD=rE.pvHead()->iVertex();

		pGrid->P()[((vOff+1)*cuv)+uOff  ]=SubdP(ivD);
		if(uses_s && has_s)		pGrid->s()[((vOff+1)*cuv)+uOff  ]=Subds(ivD);
		if(uses_t && has_t)		pGrid->t()[((vOff+1)*cuv)+uOff  ]=Subdt(ivD);
		if(uses_Cs && has_Cs)	pGrid->Cs()[((vOff+1)*cuv)+uOff  ]=SubdCs(ivD);
		if(uses_Os && has_Os)	pGrid->Os()[((vOff+1)*cuv)+uOff  ]=SubdOs(ivD);
	}

	vOff-=1<<(Level-1);
}

//---------------------------------------------------------------------
/** Output only the relevant faces from this patch ignoring the neighbours.
 */

void CqSubdivisionPatch::_OutputMesh(FILE* pf, TqInt Subd, char* name, unsigned int col)
{
	fprintf(pf,"%s\n",name);
	TqInt i;
	fprintf(pf,"%d\n",cVerts());
	for(i=0; i<P().Size(); i++)
	{
		CqVector3D	vecA;
		vecA=P()[i];
		fprintf(pf,"%f %f %f\n" ,vecA.x(), vecA.y(), vecA.z());
	}

	fprintf(pf,"%d\n",(TqInt)pow(4,Subd));
	for(i=0; i<(TqInt)pow(4,Subd); i++)
	{
		CqWReference ref(pFace(i)->pEdge(0),pFace(i));
		TqInt j=0;
		fprintf(pf,"%d",pFace(i)->cEdges());
		while(j<pFace(i)->cEdges())
		{
			TqInt	iA;
			iA=ref.pvHead()->iVertex();

			fprintf(pf," %d" ,iA);

			ref.peNext();
			j++;
		}
		fprintf(pf," 0x%X both\n",col);
	}
}

//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
