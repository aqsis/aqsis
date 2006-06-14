// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
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
		\author Paul C. Gregory (pgregory@aqsis.org)
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

class CqSubdivision2 : public CqMotionSpec<boost::shared_ptr<CqPolygonPoints> >
{
	public:
		///	Constructor.
		CqSubdivision2( );

		///	Constructor.
		CqSubdivision2( const boost::shared_ptr<CqPolygonPoints>& pPoints );

		///	Destructor.
		virtual ~CqSubdivision2();

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqSubdivision2");
		}
#endif

		CqLath* pFacet(TqInt iIndex);
		CqLath* pVertex(TqInt iIndex);
		const CqLath* pFacet(TqInt iIndex) const;
		const CqLath* pVertex(TqInt iIndex) const;

		/// Get the number of faces representing this topology.
		TqInt	cFacets() const
		{
			return(m_apFacets.size());
		}
		/// Get the number of laths representing this topology.
		TqInt	cLaths() const
		{
			return(m_apLaths.size());
		}
		/// Get the number of faces representing this topology.
		TqInt	cVertices() const
		{
			return(m_aapVertices.size());
		}

		/// Get a refrence to the array of autoatically generated laths.
		const std::vector<CqLath*>& apLaths() const
		{
			return(m_apLaths);
		}

		/// Get pointer to the vertex storage class
		boost::shared_ptr<CqPolygonPoints> pPoints( TqInt TimeIndex = 0 ) const
		{
			return ( GetMotionObject( Time( TimeIndex ) ) );
		}

		void		Prepare(TqInt cVerts);
		CqLath*		AddFacet(TqInt cVerts, TqInt* pIndices, TqInt iFVIndex);
		CqLath*		AddFacet(TqInt cVerts, TqInt* pIndices, TqInt* pFVIndices);
		TqBool		Finalise();
		void		SubdivideFace(CqLath* pFace, std::vector<CqLath*>& apSubFaces);
		TqBool		CanUsePatch( CqLath* pFace );
		void		SetInterpolateBoundary( TqBool state = TqTrue )
		{
			m_bInterpolateBoundary = state;
		}
		TqBool		isInterpolateBoundary( ) const
		{
			return( m_bInterpolateBoundary );
		}
		void		SetHoleFace( TqInt iFaceIndex )
		{
			m_mapHoles[ iFaceIndex ] = TqTrue;
		}
		TqBool		isHoleFace( TqInt iFaceIndex ) const
		{
			return( m_mapHoles.find( iFaceIndex ) != m_mapHoles.end() );
		}
		void		AddSharpEdge( CqLath* pLath, TqFloat Sharpness )
		{
			m_mapSharpEdges[pLath] = Sharpness;
		}
		TqFloat		EdgeSharpness( CqLath* pLath )
		{
			if( m_mapSharpEdges.find( pLath ) != m_mapSharpEdges.end() )
				return( m_mapSharpEdges[ pLath ] );
			return( 0.0f );
		}
		void		AddSharpCorner( CqLath* pLath, TqFloat Sharpness )
		{
			std::vector<CqLath*> aQve;
			pLath->Qve( aQve );
			std::vector<CqLath*>::iterator iVE;
			for( iVE = aQve.begin(); iVE != aQve.end(); iVE++ )
				m_mapSharpCorners[(*iVE)] = Sharpness;
		}
		TqFloat		CornerSharpness( CqLath* pLath )
		{
			if( m_mapSharpCorners.find( pLath ) != m_mapSharpCorners.end() )
				return( m_mapSharpCorners[ pLath ] );
			return( 0.0f );
		}

		void		AddVertex(CqLath* pVertex, TqInt& iVIndex, TqInt& iFVIndex);
		template<class TypeA, class TypeB>
		void		CreateVertex(CqParameterTyped<TypeA, TypeB>* pParam, CqLath* pVertex, TqInt iIndex)
		{
			TqInt arraysize = 0, arrayindex;
			arraysize = pParam->Count();
			for( arrayindex = 0; arrayindex < arraysize; arrayindex++ )
			{
				TypeA S = TypeA(0.0f);
				TypeA Q = TypeA(0.0f);
				TypeA R = TypeA(0.0f);
				TqInt n;

				if(pParam->Class() == class_vertex /*|| pParam->Class() == class_facevarying*/)
				{
					// Get a pointer to the appropriate index accessor function on CqLath based on class.
					TqInt (CqLath::*IndexFunction)() const;
					if( pParam->Class() == class_vertex )
						IndexFunction = &CqLath::VertexIndex;
					else
						IndexFunction = &CqLath::FaceVertexIndex;

					// Determine if we have a boundary vertex.
					if( pVertex->isBoundaryVertex() )
					{
						// The vertex is on a boundary.
						/// \note If "interpolateboundary" is not specified, we will never see this as
						/// the boundary facets aren't rendered. So we don't need to check for "interpolateboundary" here.
						std::vector<CqLath*> apQve;
						pVertex->Qve(apQve);
						// Is the valence == 2 ?
						if( apQve.size() == 2 )
						{
							// Yes, boundary with valence 2 is corner.
							pParam->pValue( iIndex )[arrayindex] = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
						}
						else
						{
							// No, boundary is average of two adjacent boundary edges, and original point.
							// Get the midpoints of the adjacent boundary edges
							std::vector<CqLath*> aQve;
							pVertex->Qve( aQve );

							TqInt cBoundaryEdges = 0;
							std::vector<CqLath*>::iterator iE;
							for( iE = aQve.begin(); iE != aQve.end(); iE++ )
							{
								// Only consider the boundary edges.
								if( NULL == (*iE)->ec() )
								{
									if( (*iE)->VertexIndex() == (pVertex->*IndexFunction)() )
										R += pParam->pValue( ((*iE)->ccf()->*IndexFunction)() )[arrayindex];
									else
										R += pParam->pValue( ((*iE)->*IndexFunction)() )[arrayindex];
									cBoundaryEdges++;
								}
							}
							assert( cBoundaryEdges == 2 );

							// Get the current vertex;
							S = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
							pParam->pValue( iIndex )[arrayindex] = static_cast<TypeA>( ( R + ( S * 6.0f ) ) / 8.0f );
						}
					}
					else
					{
						// Check if a sharp corner vertex.
						if( CornerSharpness( pVertex ) > 0.0f )
						{
							pParam->pValue( iIndex )[arrayindex] = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
						}
						else
						{
							// Check if crease vertex.
							std::vector<CqLath*> aQve;
							pVertex->Qve( aQve );

							CqLath* hardEdge1 = NULL;
							CqLath* hardEdge2 = NULL;
							CqLath* hardEdge3 = NULL;
							TqInt se = 0;
							std::vector<CqLath*>::iterator iEdge;
							for( iEdge = aQve.begin(); iEdge != aQve.end(); iEdge++ )
							{
								float h = EdgeSharpness( (*iEdge) );
								if( hardEdge1 == NULL || h > EdgeSharpness(hardEdge1) )
								{
									hardEdge3 = hardEdge2;
									hardEdge2 = hardEdge1;
									hardEdge1 = *iEdge;
								}
								else if( hardEdge2 == NULL || h > EdgeSharpness(hardEdge2) )
								{
									hardEdge3 = hardEdge2;
									hardEdge2 = *iEdge;
								}
								else if( hardEdge3 == NULL || h > EdgeSharpness(hardEdge3) )
								{
									hardEdge3 = *iEdge;
								}

								if( h > 0.0f )
								{
									se++;
									//		printf("h = %f\n", h);
								}
							}

							TypeA softPos;
							TypeA semiSharpPos;
							TypeA sharpPos;
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
									Val += pParam->pValue( ((*iV)->*IndexFunction)() )[arrayindex];
								Val = static_cast<TypeA>( Val / static_cast<TqFloat>( aQfv.size() ) );
								Q += Val;
							}
							Q /= aQvf.size();
							Q /= n;

							// Get the midpoints of the surrounding edges
							TypeA A = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
							TypeA B = TypeA(0.0f);
							std::vector<CqLath*>::iterator iE;
							for( iE = aQve.begin(); iE != aQve.end(); iE++ )
							{
								B = pParam->pValue( ((*iE)->ccf()->*IndexFunction)() )[arrayindex];
								R += static_cast<TypeA>( (A+B)/2.0f );
							}
							R = static_cast<TypeA>( R * 2.0f );
							R /= n;
							R /= n;

							// Get the current vertex;
							S = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
							S = static_cast<TypeA>( S * static_cast<TqFloat>(n-3) );
							S /= n;

							//pParam->pValue( iIndex )[0] = Q+R+S;
							softPos = Q+R+S;

							if( se >= 2 )
							{
								// Crease
								// Get the midpoints of the surrounding 2 hardest edges
								R = pParam->pValue((hardEdge1->ccf()->*IndexFunction)() )[arrayindex];
								R = R + pParam->pValue((hardEdge2->ccf()->*IndexFunction)() )[arrayindex];

								// Get the current vertex;
								S = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
								semiSharpPos = static_cast<TypeA>( ( R + ( S * 6.0f ) ) / 8.0f );
							}

							sharpPos = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];

							// Blend the three values together weighted by the sharpness values.
							TypeA Pos;
							float h2 = hardEdge2 != NULL ? EdgeSharpness(hardEdge2) : 0.0f;
							float h3 = hardEdge3 != NULL ? EdgeSharpness(hardEdge3) : 0.0f;
							Pos = static_cast<TypeA>( (1.0f - h2)*softPos );
							Pos = static_cast<TypeA>( Pos + (h2 - h3)*semiSharpPos );
							Pos = static_cast<TypeA>( Pos + h3*sharpPos );
							pParam->pValue( iIndex )[arrayindex] = Pos;
						}
					}
				}
				else
				{
					// Get a pointer to the appropriate index accessor function on CqLath based on class.
					TqInt (CqLath::*IndexFunction)() const;
					if( pParam->Class() == class_varying )
						IndexFunction = &CqLath::VertexIndex;
					else
						IndexFunction = &CqLath::FaceVertexIndex;

					TypeA A = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
					pParam->pValue( iIndex )[arrayindex] = A;
				}
			}
		}
		void		AddEdgeVertex(CqLath* pEdge, TqInt& iVIndex, TqInt& iFVIndex);
		template<class TypeA, class TypeB>
		void		CreateEdgeVertex(CqParameterTyped<TypeA, TypeB>* pParam, CqLath* pEdge, TqInt iIndex)
		{
			TqInt arraysize = 0, arrayindex;
			arraysize = pParam->Count();
			for( arrayindex = 0; arrayindex < arraysize; arrayindex++ )
			{
				TypeA A = TypeA(0.0f);
				TypeA B = TypeA(0.0f);
				TypeA C = TypeA(0.0f);

				if(pParam->Class() == class_vertex /*|| pParam->Class() == class_facevarying*/)
				{
					// Get a pointer to the appropriate index accessor function on CqLath based on class.
					TqInt (CqLath::*IndexFunction)() const;
					if( pParam->Class() == class_vertex )
						IndexFunction = &CqLath::VertexIndex;
					else
						IndexFunction = &CqLath::FaceVertexIndex;

					if( NULL != pEdge->ec() )
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
								Val += pParam->pValue( ((*iV)->*IndexFunction)() )[arrayindex];
							Val = static_cast<TypeA>( Val / static_cast<TqFloat>( aQfv.size() ) );
							C += Val;
						}
						C = static_cast<TypeA>( C / static_cast<TqFloat>(aQef.size()) );

						A = pParam->pValue( (pEdge->*IndexFunction)() )[arrayindex];
						B = pParam->pValue( (pEdge->ccf()->*IndexFunction)() )[arrayindex];

						float h = EdgeSharpness( pEdge );
						A = static_cast<TypeA>( ((1.0f+h)*(A+B)) / 2.0f );
						A = static_cast<TypeA>( (A + (1.0f-h)*C) / 2.0f );
					}
					else
					{
						A = pParam->pValue( (pEdge->*IndexFunction)() )[arrayindex];
						B = pParam->pValue( (pEdge->ccf()->*IndexFunction)() )[arrayindex];
						A = static_cast<TypeA>( (A+B)/2.0f );
					}
				}
				else
				{
					// Get a pointer to the appropriate index accessor function on CqLath based on class.
					TqInt (CqLath::*IndexFunction)() const;
					if( pParam->Class() == class_varying )
						IndexFunction = &CqLath::VertexIndex;
					else
						IndexFunction = &CqLath::FaceVertexIndex;

					A = pParam->pValue( (pEdge->*IndexFunction)() )[arrayindex];
					B = pParam->pValue( (pEdge->ccf()->*IndexFunction)() )[arrayindex];
					A = static_cast<TypeA>( (A+B)/2.0f );
				}
				pParam->pValue( iIndex )[arrayindex] = A;
			}
		}
		void		AddFaceVertex(CqLath* pFace, TqInt& iVIndex, TqInt& iFVIndex);
		template<class TypeA, class TypeB>
		void		CreateFaceVertex(CqParameterTyped<TypeA, TypeB>* pParam, CqLath* pFace, TqInt iIndex)
		{
			// Get a pointer to the appropriate index accessor function on CqLath based on class.
			TqInt (CqLath::*IndexFunction)() const;
			if( pParam->Class() == class_vertex || pParam->Class() == class_varying)
				IndexFunction = &CqLath::VertexIndex;
			else
				IndexFunction = &CqLath::FaceVertexIndex;
			// Face point is just the average of the original faces vertices.
			std::vector<CqLath*> aQfv;
			pFace->Qfv(aQfv);
			TqInt arraysize = 0, arrayindex;
			arraysize = pParam->Count();
			for( arrayindex = 0; arrayindex < arraysize; arrayindex++ )
			{
				std::vector<CqLath*>::iterator iV;
				TypeA Val = TypeA(0.0f);
				for( iV = aQfv.begin(); iV != aQfv.end(); iV++ )
				{
					assert( ((*iV)->*IndexFunction)() >= 0 &&
					        ((*iV)->*IndexFunction)() < pParam->Size() );
					Val += pParam->pValue( ((*iV)->*IndexFunction)() )[arrayindex];
				}
				Val = static_cast<TypeA>( Val / static_cast<TqFloat>( aQfv.size() ) );
				pParam->pValue( iIndex )[arrayindex] = Val;
			}
		}

		// Overrides from CqMotionSpec
		virtual	void	ClearMotionObject( boost::shared_ptr<CqPolygonPoints>& A ) const
			{}
		;
		virtual	boost::shared_ptr<CqPolygonPoints> ConcatMotionObjects( boost::shared_ptr<CqPolygonPoints> const & A, boost::shared_ptr<CqPolygonPoints> const & B ) const
		{
			return ( A );
		}
		virtual	boost::shared_ptr<CqPolygonPoints> LinearInterpolateMotionObjects( TqFloat Fraction, boost::shared_ptr<CqPolygonPoints> const & A, boost::shared_ptr<CqPolygonPoints> const & B ) const
		{
			return ( A );
		}


		void		OutputMesh(const char* fname, std::vector<CqLath*>* paFaces = 0);
		void		OutputInfo(const char* fname, std::vector<CqLath*>* paFaces = 0);

		CqSubdivision2* Clone() const;
	private:
		/// Array of pointers to laths, one each representing each facet.
		std::vector<CqLath*>				m_apFacets;
		/// Array of arrays of pointers to laths each array representing the total laths referencing a single vertex.
		std::vector<std::vector<CqLath*> >	m_aapVertices;
		/// Array of lath pointers, one for each lath generated.
		std::vector<CqLath*>				m_apLaths;
		/// Map of face indices which are to be treated as holes in the surface, i.e. not rendered.
		std::map<TqInt, TqBool>				m_mapHoles;
		/// Flag indicating whether this surface interpolates it's boundaries or not.
		TqBool								m_bInterpolateBoundary;
		/// Map of sharp edges.
		std::map<CqLath*, TqFloat>			m_mapSharpEdges;
		/// Map of sharp corners.
		std::map<CqLath*, TqFloat>			m_mapSharpCorners;


		/// Flag indicating whether the topology structures have been finalised.
		TqBool							m_fFinalised;
};



class CqSurfaceSubdivisionPatch : public CqSurface
{
	public:
		CqSurfaceSubdivisionPatch( const boost::shared_ptr<CqSubdivision2>& pTopology, CqLath* pFace, TqInt faceIndex)
		{
			m_pTopology = pTopology;
			m_pFace = pFace;
			m_Uses = Uses();
			m_Time = QGetRenderContextI()->Time();
			m_FaceIndex = faceIndex;
		}

		virtual	~CqSurfaceSubdivisionPatch()
		{
			assert(m_pTopology);
		}

#ifdef _DEBUG
		CqString className() const
		{
			return CqString("CqSurfaceSubdivisionPatch");
		}
#endif

		/** Get the pointer to the subdivision surface hull that this patch is part of.
		 */
		boost::shared_ptr<CqSubdivision2>	pTopology() const
		{
			return( m_pTopology );
		}

		/** Get the index of the face on the hull that this patch refers to.
		 */
		CqLath*	pFace() const
		{
			return( m_pFace );
		}

		virtual	IqAttributes*	pAttributes() const
		{
			return ( pTopology()->pPoints()->pAttributes() );
		}
		virtual	boost::shared_ptr<IqTransform>	pTransform() const
		{
			return ( pTopology()->pPoints()->pTransform() );
		}
		// Required implementations from IqSurface
		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 )
		{
			//pTopology()->pPoints( iTime )->Transform( matTx, matITTx, matRTx );
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

		// Implementations required by CqSurface
		virtual	CqBound	Bound() const;
		virtual	CqMicroPolyGridBase* Dice();
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual TqBool	Diceable();

		virtual	CqMicroPolyGridBase* DiceExtract();

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual TqBool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( TqFalse );
		}

		void StoreDice( CqMicroPolyGrid* pGrid, const boost::shared_ptr<CqPolygonPoints>& pPoints, TqInt iParam, TqInt iFVParam, TqInt iVData);
		boost::shared_ptr<CqSubdivision2> Extract( TqInt iTime );

		virtual CqSurface* Clone() const
		{
			// \warning: Should never ever be cloning one of these surfaces.
			assert(false);
			return(NULL);
		}

	private:
		boost::shared_ptr<CqSubdivision2>	m_pTopology;
		CqLath*			m_pFace;
		TqInt			m_Uses;
		TqFloat			m_Time;
		TqInt			m_FaceIndex;
};

//----------------------------------------------------------------------
/** \class CqSurfacePointsPolygons
 * Container surface to store the polygons making up a RiPointsPolygons surface.
 */

class CqSurfaceSubdivisionMesh : public CqSurface
{
	public:
		CqSurfaceSubdivisionMesh(const boost::shared_ptr<CqSubdivision2>& pTopology, TqInt NumFaces) :
				m_NumFaces(NumFaces),
				m_pTopology( pTopology )
		{}
		virtual	~CqSurfaceSubdivisionMesh()
		{}

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqSurfaceSubdivisionMesh");
		}
#endif

		/** Get the gemoetric bound of this GPrim.
		 */
		virtual	CqBound	Bound() const;
		/** Dice this GPrim.
		 * \return A pointer to a new micropolygrid..
		 */
		virtual	CqMicroPolyGridBase* Dice()
		{
			return(NULL);
		}
		/** Split this GPrim into a number of other GPrims.
		 * \param aSplits A reference to a CqSurface array to fill in with the new GPrim pointers.
		 * \return Integer count of new GPrims created.
		 */
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		/** Determine whether this GPrim is diceable at its current size.
		 */
		virtual TqBool	Diceable()
		{
			return( TqFalse );
		}

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 )
		{
			assert( m_pTopology );
			m_pTopology->pPoints()->Transform( matTx, matITTx, matRTx, iTime );
		}

		virtual TqBool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( TqFalse );
		}

		virtual	TqUint	cUniform() const
		{
			return ( m_NumFaces );
		}
		virtual	TqUint	cVarying() const
		{
			assert( m_pTopology );
			assert( m_pTopology->pPoints() );
			return ( m_pTopology->pPoints()->cVarying() );
		}
		virtual	TqUint	cVertex() const
		{
			assert( m_pTopology );
			assert( m_pTopology->pPoints() );
			return ( m_pTopology->pPoints()->cVarying() );
		}
		virtual	TqUint	cFaceVarying() const
		{
			assert( m_pTopology );
			assert( m_pTopology->pPoints() );
			return ( m_pTopology->pPoints()->cFaceVarying() );
		}
		virtual CqSurface* Clone() const;

		void AddSharpEdge(TqInt a, TqInt b, TqFloat sharpness)
		{
			m_aSharpEdges.push_back(std::pair<std::pair<TqInt, TqInt>, TqFloat>(std::pair<TqInt, TqInt>(a, b), sharpness));
		}
		void AddSharpCorner(TqInt a, TqFloat sharpness)
		{
			m_aSharpCorners.push_back(std::pair<TqInt, TqFloat>(a, sharpness));
		}

	private:
		TqInt	m_NumFaces;
		boost::shared_ptr<CqSubdivision2>	m_pTopology;		///< Pointer to the associated CqSubdivision2 class.
		std::vector<std::pair<std::pair<TqInt,TqInt>, TqFloat> >	m_aSharpEdges; 
		std::vector<std::pair<TqInt, TqFloat> > m_aSharpCorners; 
};


END_NAMESPACE( Aqsis )

#endif	//	SUBDIVISION2_H_LOADED
