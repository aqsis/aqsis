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

class CqSubdivision2 : public CqRefCount, public CqMotionSpec<CqPolygonPoints*>
{
public:
	///	Constructor.
	CqSubdivision2( CqPolygonPoints* pPoints = NULL );

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
	CqPolygonPoints* pPoints( TqInt TimeIndex = 0 ) const	
	{
		return(GetMotionObject( Time( TimeIndex ) ));
	}

	void		Prepare(TqInt cVerts);
	CqLath*		AddFacet(TqInt cVerts, TqInt* pIndices);
	TqBool		Finalise();
	void		SubdivideFace(CqLath* pFace, std::vector<CqLath*>& apSubFaces);
	TqBool		CanUsePatch( CqLath* pFace );

	TqInt		AddVertex(CqLath* pVertex);
	template<class TypeA, class TypeB>
	void		CreateVertex(CqParameterTyped<TypeA, TypeB>* pParam, CqLath* pVertex, TqInt iIndex)
				{
					TypeA S = TypeA(0.0f);
					TypeA Q = TypeA(0.0f);
					TypeA R = TypeA(0.0f);
					TqInt n;
					
#if 1
					if(pParam->Class() == class_vertex)
					{
						// Determine if we have a boundary vertex.
						if( ( NULL != pVertex->cv() ) && ( NULL != pVertex->ccv() ) )
						{
							// Smooth
							// Vertex point is...
							//    Q     2R     S(n-3)
							//   --- + ---- + --------
							//    n      n        n
							// 
							// Q = Average of face points surrounding old vertex
							// R = average of midpoints of edges surrounding old vertex
							// S = old vertex
							// n = number of edges sharing the old vertex.

							std::vector<CqLath*> aQve;
							pVertex->Qve( aQve );
							n = aQve.size();

							// Get the face points of the surrounding faces
							std::vector<CqLath*> aQvf;
							pVertex->Qvf( aQvf );
							std::vector<CqLath*>::iterator iF;
							for( iF = aQvf.begin(); iF != aQvf.end(); iF++ )
							{
								std::vector<CqLath*> aQfv;
								(*iF)->Qfv(aQfv);
								std::vector<CqLath*>::iterator iV;
								TypeA Val = TypeA(0.0f);
								for( iV = aQfv.begin(); iV != aQfv.end(); iV++ )
									Val += pParam->pValue( (*iV)->VertexIndex() )[0];
								Val /= static_cast<TqFloat>( aQfv.size() );
								Q += Val;
							}
							Q /= aQvf.size();
							Q /= n;
							
							// Get the midpoints of the surrounding edges
							TypeA A = pParam->pValue( pVertex->VertexIndex() )[0];
							TypeA B = TypeA(0.0f);
							std::vector<CqLath*>::iterator iE;
							for( iE = aQve.begin(); iE != aQve.end(); iE++ )
							{
								B = pParam->pValue( (*iE)->ccf()->VertexIndex() )[0];
								R += (A+B)/2.0f;
							}
							R *= 2.0f;
							R /= n;
							R /= n;

							// Get the current vertex;
							S = pParam->pValue( pVertex->VertexIndex() )[0];
							S *= static_cast<TqFloat>(n-3);
							S /= n;
							
							pParam->pValue( iIndex )[0] = Q+R+S;
						}
						else
						{
							// The vertex is on a boundary.
							std::vector<CqLath*> apQve;
							pVertex->Qve(apQve);							
							// Is the valence == 2 ?
							if( apQve.size() == 2 )
							{
								// Yes, boundary with valence 2 is corner.
								pParam->pValue( iIndex )[0] = pParam->pValue( pVertex->VertexIndex() )[0];
							}
							else
							{
								// No, boundary is average of two adjacent boundary edges, and original point.
								// Get the midpoints of the adjacent boundary edges
								std::vector<CqLath*> aQve;
								pVertex->Qve( aQve );

								TypeA A = pParam->pValue( pVertex->VertexIndex() )[0];
								TypeA B = TypeA(0.0f);
								TqInt cBoundaryEdges = 0;
								std::vector<CqLath*>::iterator iE;
								for( iE = aQve.begin(); iE != aQve.end(); iE++ )
								{
									// Only consider the boundary edges.
									if( NULL == (*iE)->ec() )
									{
										B = pParam->pValue( (*iE)->ccf()->VertexIndex() )[0];
										R += (A+B)/2.0f;
										cBoundaryEdges++;
									}
								}
								assert( cBoundaryEdges == 2 );
								R *= 0.5f;
								
								// Get the current vertex;
								S = pParam->pValue( pVertex->VertexIndex() )[0];
								pParam->pValue( iIndex )[0] = ( R+S ) * 0.5f;;
							}
						}
					}
					else
#endif
					{
						pParam->pValue( iIndex )[0] = pParam->pValue( pVertex->VertexIndex() )[0];
					}
				}
	TqInt		AddEdgeVertex(CqLath* pEdge);
	template<class TypeA, class TypeB>
	void		CreateEdgeVertex(CqParameterTyped<TypeA, TypeB>* pParam, CqLath* pEdge, TqInt iIndex)
				{
					TypeA A = TypeA(0.0f);
					TypeA B = TypeA(0.0f);
					TypeA C = TypeA(0.0f);

#if 1
					if(pParam->Class() == class_vertex && ( NULL != pEdge->ec() ) )
					{
						// Edge point is the average of the centrepoint of the original edge and the
						// average of the two new face points of the adjacent faces.
						std::vector<CqLath*> aQef;
						pEdge->Qef( aQef );
						std::vector<CqLath*>::iterator iF;
						for( iF = aQef.begin(); iF != aQef.end(); iF++ )
						{
							std::vector<CqLath*> aQfv;
							(*iF)->Qfv(aQfv);
							std::vector<CqLath*>::iterator iV;
							TypeA Val = TypeA(0.0f);
							for( iV = aQfv.begin(); iV != aQfv.end(); iV++ )
								Val += pParam->pValue( (*iV)->VertexIndex() )[0];
							Val/=static_cast<TqFloat>( aQfv.size() );
							C += Val;
						}
						C /= static_cast<TqFloat>(aQef.size());

						A = pParam->pValue( pEdge->VertexIndex() )[0];
						B = pParam->pValue( pEdge->ccf()->VertexIndex() )[0];
						A = (A+B)/2.0f;
						A = (A+C)/2.0f;
					}
					else
#endif
					{
						A = pParam->pValue( pEdge->VertexIndex() )[0];
						B = pParam->pValue( pEdge->ccf()->VertexIndex() )[0];
						A = (A+B)/2.0f;
					}
					pParam->pValue( iIndex )[0] = A;
				}
	TqInt		AddFaceVertex(CqLath* pFace);
	template<class TypeA, class TypeB>
	void		CreateFaceVertex(CqParameterTyped<TypeA, TypeB>* pParam, CqLath* pFace, TqInt iIndex)
				{
					// Face point is just the average of the original faces vertices.
					std::vector<CqLath*> aQfv;
					pFace->Qfv(aQfv);
					std::vector<CqLath*>::iterator iV;
					TypeA Val = TypeA(0.0f);
					for( iV = aQfv.begin(); iV != aQfv.end(); iV++ )
						Val += pParam->pValue( (*iV)->VertexIndex() )[0];
					Val/=static_cast<TqFloat>( aQfv.size() );
					pParam->pValue( iIndex )[0] = Val;
				}

		// Overrides from CqMotionSpec
		virtual	void	ClearMotionObject( CqPolygonPoints*& A ) const
			{}
		;
		virtual	CqPolygonPoints* ConcatMotionObjects( CqPolygonPoints* const & A, CqPolygonPoints* const & B ) const
		{
			return ( A );
		}
		virtual	CqPolygonPoints* LinearInterpolateMotionObjects( TqFloat Fraction, CqPolygonPoints* const & A, CqPolygonPoints* const & B ) const
		{
			return ( A );
		}


	void		OutputMesh(const char* fname, std::vector<CqLath*>* paFaces = 0);
	void		OutputInfo(const char* fname, std::vector<CqLath*>* paFaces = 0);
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

	/// Flag indicating whether the topology structures have been finalised.
	TqBool							m_fFinalised;
};



class CqSurfaceSubdivisionPatch : public CqBasicSurface
{
	public:
		CqSurfaceSubdivisionPatch(CqSubdivision2* pTopology, CqLath* pFace)
		{
			assert(NULL != pTopology);
			// Reference the topology class
			pTopology->AddRef();
			m_pTopology = pTopology;
			m_pFace = pFace;
		}

		virtual	~CqSurfaceSubdivisionPatch()
		{
			assert(NULL != m_pTopology);
			// Unreference the subdivision topology class
			m_pTopology->Release();
		}

		/** Get the pointer to the subdivision surface hull that this patch is part of.
		 */
		CqSubdivision2*	pTopology() const
		{
			return( m_pTopology );
		}

		/** Get the index of the face on the hull that this patch refers to.
		 */
		CqLath*	pFace() const
		{
			return( m_pFace );
		}

		virtual	const IqAttributes*	pAttributes() const
		{
			return ( pTopology()->pPoints()->pAttributes() );
		}
		virtual	const IqTransform*	pTransform() const
		{
			return ( pTopology()->pPoints()->pTransform() );
		}
		// Required implementations from IqSurface
		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx )
		{
			pTopology()->pPoints()->Transform( matTx, matITTx, matRTx );
		}
		// NOTE: These should never be called.
		virtual	TqUint	cUniform() const
		{
			return ( 0 );
		}
		virtual	TqUint	cVarying() const
		{
			return ( 0 );
		}
		virtual	TqUint	cVertex() const
		{
			return ( 0 );
		}
		virtual	TqUint	cFaceVarying() const
		{
			return ( 0 );
		}

		// Implementations required by CqBasicSurface
		virtual	CqBound	Bound() const;
		virtual	CqMicroPolyGridBase* Dice();
		virtual	TqInt	Split( std::vector<CqBasicSurface*>& aSplits );
		virtual TqBool	Diceable();

		void StoreDice( CqMicroPolyGrid* pGrid, CqPolygonPoints* pPoints, TqInt iParam, TqInt iData );

	private:
		CqSubdivision2*	m_pTopology;
		CqLath*			m_pFace;
};

END_NAMESPACE( Aqsis )

#endif	//	SUBDIVISION2_H_LOADED
