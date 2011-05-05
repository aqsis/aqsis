// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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

#include <aqsis/aqsis.h>
#include "lath.h"
#include <aqsis/math/vector3d.h>
#include "surface.h"
#include "polygon.h"

namespace Aqsis {

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
		bool		Finalise();
		void		SubdivideFace(CqLath* pFace, std::vector<CqLath*>& apSubFaces);
		bool		CanUsePatch( CqLath* pFace );
		void		SetInterpolateBoundary( bool state = true )
		{
			m_bInterpolateBoundary = state;
		}
		bool		isInterpolateBoundary( ) const
		{
			return( m_bInterpolateBoundary );
		}
		void		SetHoleFace( TqInt iFaceIndex )
		{
			m_mapHoles[ iFaceIndex ] = true;
		}
		bool		isHoleFace( TqInt iFaceIndex ) const
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
		TqFloat		CornerSharpness(const CqLath* pLath) const
		{
			TqSharpnessMap::const_iterator pos = m_mapSharpCorners.find(pLath);
			if(pos != m_mapSharpCorners.end())
				return pos->second;
			return 0.0f;
		}

		/** \brief Push a point to the limit surface
		 *
		 * Sensible subdivision schemes push any vertex in the mesh toward a
		 * limiting position on the limit surface.  This function determines
		 * the limit point of a given vertex.
		 *
		 * Warning: This function currently ignores edge hardness, since only
		 * the limit mask for the standard subdivision rules is present in the
		 * literature.
		 *
		 * Non-const since some subdivision may have to be performed to obtain
		 * the limit point correctly.
		 *
		 * \param vert - Lath connected to the vertex for which we want the
		 *               limit point.
		 * \return Limit point corresponding to vert.
		 */
		CqVector3D limitPoint(CqLath* vert);

		void AddVertex(CqLath* pVertex, TqInt& iVIndex, TqInt& iFVIndex);
		void AddEdgeVertex(CqLath* pEdge, TqInt& iVIndex, TqInt& iFVIndex);
		void AddFaceVertex(CqLath* pFace, TqInt& iVIndex, TqInt& iFVIndex);
		void DuplicateVertex(CqLath* pVertex, TqInt& iVIndex, TqInt& iFVIndex);

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


		void OutputMesh(const char* fname, std::vector<CqLath*>* paFaces = 0);
		void OutputInfo(const char* fname, std::vector<CqLath*>* paFaces = 0);

		CqSubdivision2* Clone() const;

	private:
		template<class TypeA, class TypeB>
		void CreateVertex(CqParameter* pParamToModify, CqLath* pVertex,
				TqInt iIndex);
		template<class TypeA, class TypeB>
		void CreateEdgeVertex(CqParameter* pParamToModify, CqLath* pEdge,
				TqInt iIndex);
		template<class TypeA, class TypeB>
		void CreateFaceVertex(CqParameter* pParamToModify, CqLath* pFace,
				TqInt iIndex);
		template<class TypeA, class TypeB>
		void DuplicateVertex(CqParameter* pParamToModify, CqLath* pVertex,
				TqInt iIndex);

		void subdivideNeighbourFaces(CqLath* vert);

		typedef std::map<const CqLath*, TqFloat> TqSharpnessMap;

		/// Array of pointers to laths, one each representing each facet.
		std::vector<CqLath*>				m_apFacets;
		/// Array of arrays of pointers to laths each array representing the total laths referencing a single vertex.
		std::vector<std::vector<CqLath*> >	m_aapVertices;
		/// Array of lath pointers, one for each lath generated.
		std::vector<CqLath*>				m_apLaths;
		/// Map of face indices which are to be treated as holes in the surface, i.e. not rendered.
		std::map<TqInt, bool>				m_mapHoles;
		/// Flag indicating whether this surface interpolates it's boundaries or not.
		bool								m_bInterpolateBoundary;
		/// Map of sharp edges.
		std::map<CqLath*, TqFloat>			m_mapSharpEdges;
		/// Map of sharp corners.
		TqSharpnessMap			m_mapSharpCorners;
		/// List of facevertex parameters, for use in convert to patch testing.
		std::vector<CqParameter*> m_faceVertexParams;

		/// Flag indicating whether the topology structures have been finalised.
		bool							m_fFinalised;
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

		virtual	IqAttributesPtr	pAttributes() const
		{
			return ( pTopology()->pPoints()->pAttributes() );
		}
		virtual	IqTransformPtr	pTransform() const
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
		virtual	void	Bound(CqBound* bound) const;
		virtual	CqMicroPolyGridBase* Dice();
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual bool	Diceable(const CqMatrix& matCtoR);

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}

		boost::shared_ptr<CqSubdivision2> Extract( TqInt iTime );

		virtual CqSurface* Clone() const
		{
			// \warning: Should never ever be cloning one of these surfaces.
			assert(false);
			return(NULL);
		}

	private:
		CqMicroPolyGridBase* DiceExtract();

		void StoreDice( CqMicroPolyGrid* pGrid, const boost::shared_ptr<CqPolygonPoints>& pPoints, CqLath* vert, TqInt iVData);
		void StoreDiceAPVar( const boost::shared_ptr<IqShader>& pShader, CqParameter* pParam, TqUint ivA, TqInt ifvA, TqUint indexA );

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
		virtual	void	Bound(CqBound* bound) const;
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
		virtual bool	Diceable(const CqMatrix& /*matCtoR*/)
		{
			return( false );
		}

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 )
		{
			assert( m_pTopology );
			m_pTopology->pPoints()->Transform( matTx, matITTx, matRTx, iTime );
		}

		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
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


} // namespace Aqsis

#endif	//	SUBDIVISION2_H_LOADED
