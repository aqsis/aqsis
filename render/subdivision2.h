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
		\brief Declares the classes for subdivision surfaces.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#ifndef	SUBDIVISION2_H_LOADED
#define	SUBDIVISION2_H_LOADED

#include "aqsis.h"
#include "lath.h"
#include "vector3d.h"
#include "refcount.h"
#include "surface.h"
#include "polygon.h"

START_NAMESPACE( Aqsis )

//------------------------------------------------------------------------------
/**
 *	Container for the topology description of a mesh.
 *	Holds information about which Laths represent which facets and vertices, and 
 *  provides functions to build topology data structures from unstructured meshes.
 */

class CqSubdivision2 : public CqRefCount
{
public:
	///	Constructor.
	CqSubdivision2( CqPolygonPoints* pPoints );

	///	Destructor.
	virtual ~CqSubdivision2();

	CqLath* pFacet(TqInt iIndex);
	CqLath* pVertex(TqInt iIndex);

	/// Get the number of faces representing this topology.
	TqInt	cFacets() const		{return(m_apFacets.size());}
	/// Get the number of laths representing this topology.
	TqInt	cLaths() const		{return(m_apLaths.size());}
	/// Get the number of faces representing this topology.
	TqInt	cVertices() const	{return(m_aapVertices.size());}
	
	/// Get a refrence to the array of autoatically generated laths.
	const std::vector<CqLath*>& apLaths() const	
								{return(m_apLaths);}

	/// Get pointer to the vertex storage class
	CqPolygonPoints* pPoints() const	{return(m_pPoints);}

	void		Prepare(TqInt cVerts);
	CqLath*		AddFacet(TqInt cVerts, TqInt* pIndices);
	void		Finalise();
	void		SubdivideFace(TqInt iF);

	TqInt		AddVertex(CqLath* pVertex);
	template<class TypeA, class TypeB>
	void		CreateVertex(CqParameterTyped<TypeA, TypeB>* pParam, CqLath* pVertex, TqInt iIndex)
				{
					pParam->SetValue( pParam, iIndex, pVertex->VertexIndex() );
				}
	TqInt		AddEdgeVertex(CqLath* pEdge);
	template<class TypeA, class TypeB>
	void		CreateEdgeVertex(CqParameterTyped<TypeA, TypeB>* pParam, CqLath* pEdge, TqInt iIndex)
				{
					TypeA A,B;
					A = pParam->pValue( pEdge->VertexIndex() )[0];
					B = pParam->pValue( pEdge->ccf()->VertexIndex() )[0];
					pParam->pValue( iIndex )[0] = (A+B)/2.0f;
				}
	TqInt		AddFaceVertex(CqLath* pFace);
	template<class TypeA, class TypeB>
	void		CreateFaceVertex(CqParameterTyped<TypeA, TypeB>* pParam, CqLath* pFace, TqInt iIndex)
				{
					std::vector<CqLath*> aQfv;
					pFace->Qfv(aQfv);
					std::vector<CqLath*>::iterator iV;
					TypeA Val = TypeA(0.0f);
					for( iV = aQfv.begin(); iV != aQfv.end(); iV++ )
						Val += pParam->pValue( (*iV)->VertexIndex() )[0];
					Val/=static_cast<TqFloat>( aQfv.size() );
					pParam->pValue( iIndex )[0] = Val;
				}

	void		OutputMesh(const char* fname);
	void		OutputInfo(const char* fname);
private:
	///	Declared private to prevent copying.
	CqSubdivision2(const CqSubdivision2&);
	///	Declared private to prevent copying.
	CqSubdivision2&	operator=(const CqSubdivision2&);

	/// Array of pointers to laths, one each representing each facet.
	std::vector<CqLath*>				m_apFacets;
	/// Array of arrays of pointers to laths each array representing the total laths referencing a single vertex.
	std::vector<std::vector<CqLath*> >	m_aapVertices;
	/// Array of lath pointers, one for each lath generated.
	std::vector<CqLath*>				m_apLaths;

	/// Pointer to the class which holds vertex information for the structure.
	CqPolygonPoints*					m_pPoints;

	/// Flag indicating whether the topology structures have been finalised.
	TqBool							m_fFinalised;
};



class CqSurfaceSubdivisionPatch : public CqBasicSurface
{
	public:
		CqSurfaceSubdivisionPatch(CqSubdivision2* pTopology, TqInt iFace)
		{
			assert(NULL != pTopology);
			// Reference the topology class
			pTopology->AddRef();
			m_pTopology = pTopology;
			m_iFace = iFace;
		}

		virtual	~CqSurfaceSubdivisionPatch()
		{
			assert(NULL != m_pTopology);
			// Unreference the subdivision topology class
			m_pTopology->Release();
		}

	private:
		CqSubdivision2*	m_pTopology;
		TqInt			m_iFace;
};

END_NAMESPACE( Aqsis )

#endif	//	SUBDIVISION2_H_LOADED
