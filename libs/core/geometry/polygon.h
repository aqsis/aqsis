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
		\brief Declares the classes and support structures for handling polygons.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef POLYGON_H_INCLUDED
#define POLYGON_H_INCLUDED 1

#include	<aqsis/aqsis.h>

#include	"surface.h"
#include	<aqsis/math/vector4d.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqPolygonBase
 * Polygon base class for split and dice functionality.
 */

class CqPolygonBase
{
	public:
		CqPolygonBase()
		{
			STATS_INC( GPR_poly );
		}
		virtual	~CqPolygonBase()
		{}

		virtual	void	Bound(CqBound* bound) const;
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );

		/** Get a reference to the surface this polygon is associated with.
		 */
		virtual	const CqSurface& Surface() const = 0;
		/** Get a reference to the surface this polygon is associated with.
		 */
		virtual	CqSurface&	Surface() = 0;

		/** Get a reference to the polygon point at the specified index.
		 * \param i Integer index in of the vertex in question.
		 */
		virtual	const	CqVector3D PolyP( TqInt i ) const = 0;
		/** Get a reference to the polygon normal at the specified index.
		 * \param i Integer index in of the vertex in question.
		 */
		virtual	const	CqVector3D& PolyN( TqInt i ) const = 0;
		/** Get a reference to the polygon vertex color at the specified index.
		 * \param i Integer index in of the vertex in question.
		 */
		virtual	const	CqColor& PolyCs( TqInt i ) const = 0;
		/** Get a reference to the polygon vertex opacity at the specified index.
		 * \param i Integer index in of the vertex in question.
		 */
		virtual	const	CqColor& PolyOs( TqInt i ) const = 0;
		/** Get a reference to the polygon texture s coordinate at the specified index.
		 * \param i Integer index in of the vertex in question.
		 */
		virtual	const	TqFloat& Polys( TqInt i ) const = 0;
		/** Get a reference to the polygon texture t coordinate at the specified index.
		 * \param i Integer index in of the vertex in question.
		 */
		virtual	const	TqFloat& Polyt( TqInt i ) const = 0;
		/** Get a reference to the polygon surface u coordinate at the specified index.
		 * \param i Integer index in of the vertex in question.
		 */
		virtual	const	TqFloat& Polyu( TqInt i ) const = 0;
		/** Get a reference to the polygon surface v coordinate at the specified index.
		 * \param i Integer index in of the vertex in question.
		 */
		virtual	const	TqFloat& Polyv( TqInt i ) const = 0;
		/** Get the real index into the points list translated from the polygon vertex index.
		 * \param i Integer index in of the vertex in question.
		 */
		virtual	const	TqInt PolyIndex( TqInt i ) const = 0;

		/** Determine if the polygon has the given indexed primitive variable.
		 * \param index Integer index of the variable in the range EnvVars_Cs<=index<EnvVars_Last.
		 */
		virtual const	bool bHasVar( TqInt index ) const = 0;
		/** Get the real index into the points list translated from the polygon vertex index for facevarying variables.
		 * \param i Integer index in of the vertex in question.
		 */
		virtual	const	TqInt FaceVaryingIndex( TqInt i ) const = 0;

		/** Get the number of vertices in this polygon.
		 */
		virtual	TqInt	NumVertices() const = 0;

		/** Get a pointer to the attributes state associated with this polygon.
		 */
		virtual	IqAttributesPtr	pAttributes() const = 0;
		/** Get a pointer to the transfrom associated with this polygon.
		 */
		virtual	IqTransformPtr	pTransform() const = 0;

		/** Get the index of this polygon if it is a member of a polygon mesh
		 */
		virtual const	TqInt	MeshIndex() const
		{
			return ( 0 );
		}

		/** Get a bit vector representing the standard shader variables this polygon needs.
		 */
		const TqInt	PolyUses() const
		{
			return ( Surface().Uses() );
		}

		void CreatePhantomData(CqParameter* pParam);
};


//----------------------------------------------------------------------
/** \class CqSurfacePolygon
 * Polygon surface primitive.
 */

class CqSurfacePolygon : public CqSurface, public CqPolygonBase
{
	public:
		CqSurfacePolygon( TqInt cVertices );
		CqSurfacePolygon() : m_cVertices(0)
		{}
		virtual	~CqSurfacePolygon();

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqSurfacePolygon");
		}
#endif

		bool	CheckDegenerate() const;

		// Overridden fro mCqSurface.
		virtual	void	Bound(CqBound* bound) const
		{
			CqPolygonBase::Bound(bound);
			AdjustBoundForTransformationMotion( bound );
		}
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
		{
			return ( CqPolygonBase::Split( aSplits ) );
		}

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}

		virtual	TqUint	cUniform() const
		{
			return ( 1 );
		}
		virtual	TqUint	cVarying() const
		{
			return ( m_cVertices );
		}
		virtual	TqUint	cVertex() const
		{
			return ( m_cVertices );
		}
		virtual	TqUint	cFaceVarying() const
		{
			return ( cVarying() );
		}

		// Overridden from CqPolygonBase
		virtual	const CqSurface& Surface() const
		{
			return ( *this );
		}
		virtual	CqSurface&	Surface()
		{
			return ( *this );
		}

		virtual	const	CqVector3D PolyP( TqInt i ) const
		{
			return vectorCast<CqVector3D>( P()->pValue( i )[0] );
		}
		virtual	const	CqVector3D& PolyN( TqInt i ) const
		{
			return ( N()->pValue( i )[0] );
		}
		virtual	const	CqColor& PolyCs( TqInt i ) const
		{
			return ( Cs()->pValue( i )[0] );
		}
		virtual	const	CqColor& PolyOs( TqInt i ) const
		{
			return ( Os()->pValue( i )[0] );
		}
		virtual	const	TqFloat& Polys( TqInt i ) const
		{
			return ( s()->pValue( i )[0] );
		}
		virtual	const	TqFloat& Polyt( TqInt i ) const
		{
			return ( t()->pValue( i )[0] );
		}
		virtual	const	TqFloat& Polyu( TqInt i ) const
		{
			return ( u()->pValue( i )[0] );
		}
		virtual	const	TqFloat& Polyv( TqInt i ) const
		{
			return ( v()->pValue( i )[0] );
		}
		virtual	const	TqInt PolyIndex( TqInt i ) const
		{
			return ( i );
		}
		virtual	const	TqInt FaceVaryingIndex( TqInt i ) const
		{
			return( i );
		}
		virtual	const	bool	bHasVar(TqInt index) const
		{
			return ( CqSurface::bHasVar(index) );
		}

		virtual	TqInt	NumVertices() const
		{
			return ( cVertex() );
		}

		virtual	IqAttributesPtr	pAttributes() const
		{
			return ( CqSurface::pAttributes() );
		}
		virtual	IqTransformPtr	pTransform() const
		{
			return ( CqSurface::pTransform() );
		}
		virtual CqSurface* Clone() const;

	protected:
		TqInt	m_cVertices;	///< Count of vertices in this polygon.
}
;


//----------------------------------------------------------------------
/** \class CqPolygonPoints
 * Points array for PointsPolygons surface primitive.
 */

class CqPolygonPoints : public CqSurface
{
	public:
		CqPolygonPoints( TqInt cVertices, TqInt cFaces, TqInt sumnVerts ) :
				m_cVertices( cVertices ),
				m_Transformed( false ),
				m_cFaces( cFaces ),
				m_sumnVerts( sumnVerts )
		{}
		CqPolygonPoints() :
				m_cVertices( 0 ),
				m_Transformed( false ),
				m_cFaces( 0 ),
				m_sumnVerts( 0 )
		{}
		virtual	~CqPolygonPoints()
		{}

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqPolygonPoints");
		}
#endif

		// Overridden from CqSurface.
		// NOTE: These should never be called.
		virtual	void	Bound(CqBound* bound) const
		{}

		virtual	CqMicroPolyGridBase* Dice()
		{
			return ( 0 );
		}
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
		{
			return ( 0 );
		}
		virtual bool	Diceable(const CqMatrix& /*matCtoR*/)
		{
			return ( false );
		}

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 );

		virtual	TqUint	cUniform() const
		{
			return ( m_cFaces );
		}
		virtual	TqUint	cVarying() const
		{
			return ( m_cVertices );
		}
		virtual	TqUint	cVertex() const
		{
			return ( m_cVertices );
		}
		virtual	TqUint	cFaceVarying() const
		{
			return ( m_sumnVerts );
		}

		/** Get the number of vertices in the list.
		 */
		virtual	TqInt	NumVertices() const
		{
			return ( cVertex() );
		}
		virtual CqSurface* Clone() const;

	protected:
		TqInt	m_cVertices;		///< Count of vertices in this list.
		bool	m_Transformed;		///< Flag indicatign that the list has been transformed.
		TqInt	m_cFaces;			///< Expected count of faces referencing this list.
		TqInt	m_sumnVerts;
}
;


//----------------------------------------------------------------------
/** \class CqSurfacePointsPolygon
 * Points polygon surface primitive, a single member of the above.
 */

class CqSurfacePointsPolygon : public CqSurface, public CqPolygonBase
{
	public:
		CqSurfacePointsPolygon( const boost::shared_ptr<CqPolygonPoints>& pPoints, TqInt index, TqInt FaceVaryingIndex  ) : CqSurface(),
				m_pPoints( pPoints ),
				m_Index( index ),
				m_FaceVaryingIndex( FaceVaryingIndex )
		{
			STATS_INC( GPR_poly );
		}
		virtual	~CqSurfacePointsPolygon()
		{}

		std::vector<TqInt>&	aIndices()
		{
			return ( m_aIndices );
		}

#ifdef _DEBUG
		CqString className() const
		{
			return CqString("CqSurfacePointsPolygon");
		}
#endif

		// Overridden from CqSurface
		virtual CqMicroPolyGridBase* Dice()
		{
			assert(false);
			return( 0 );
		}
		virtual	void	Bound(CqBound* bound) const
		{
			CqPolygonBase::Bound(bound);
			AdjustBoundForTransformationMotion( bound );
		}
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
		{
			return ( CqPolygonBase::Split( aSplits ) );
		}
		virtual bool	Diceable(const CqMatrix& /*matCtoR*/)
		{
			return(false);
		}

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 )
		{
			// \attention It is invalid to call Transform on a polygon, as the rendering process has begun so the transformation is fixed.
			// The individual polygons cannot be transformed as they all refer to the same points class, if one polygon transforms
			// those points, then another polygon receives the same transform, the points will be transformed twice.
			Aqsis::log() << error << "Transform called on CqSurfacePointsPolygon" << std::endl;
			//m_pPoints->Transform( matTx, matITTx, matRTx );
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
		//---------------

		// Overridden from CqPolygonBase
		virtual	const CqSurface& Surface() const
		{
			return ( *m_pPoints );
		}
		virtual	CqSurface& Surface()
		{
			return ( *m_pPoints );
		}

		virtual	const CqVector3D PolyP( TqInt i ) const
		{
			return vectorCast<CqVector3D>( m_pPoints->P()->pValue( m_aIndices[ i ] )[0] );
		}
		virtual	const CqVector3D& PolyN( TqInt i ) const
		{
			return ( m_pPoints->N()->pValue( m_aIndices[ i ] )[0] );
		}
		virtual	const CqColor& PolyCs( TqInt i ) const
		{
			return ( m_pPoints->Cs()->pValue( m_aIndices[ i ] )[0] );
		}
		virtual	const CqColor& PolyOs( TqInt i ) const
		{
			return ( m_pPoints->Os()->pValue( m_aIndices[ i ] )[0] );
		}
		virtual	const TqFloat& Polys( TqInt i ) const
		{
			return ( m_pPoints->s()->pValue( m_aIndices[ i ] )[0] );
		}
		virtual	const TqFloat& Polyt( TqInt i ) const
		{
			return ( m_pPoints->t()->pValue( m_aIndices[ i ] )[0] );
		}
		virtual	const TqFloat& Polyu( TqInt i ) const
		{
			return ( m_pPoints->u()->pValue( m_aIndices[ i ] )[0] );
		}
		virtual	const TqFloat& Polyv( TqInt i ) const
		{
			return ( m_pPoints->v()->pValue( m_aIndices[ i ] )[0] );
		}
		virtual	const	TqInt PolyIndex( TqInt i ) const
		{
			return ( ( (TqUint) i < m_aIndices.size() ) ? m_aIndices[ i ] : m_aIndices.back() );
		}
		virtual	const	TqInt FaceVaryingIndex( TqInt i ) const
		{
			return ( m_FaceVaryingIndex + i );
		}

		virtual	TqInt	NumVertices() const
		{
			return ( m_aIndices.size() );
		}

		virtual	IqAttributesPtr	pAttributes() const
		{
			return ( m_pPoints->pAttributes() );
		}
		virtual	IqTransformPtr	pTransform() const
		{
			return ( m_pPoints->pTransform() );
		}

		/** Determine whether this surface has the given indexed primitive variable.
		 */
		const	bool	bHasVar(TqInt index) const
		{
			return ( m_pPoints->bHasVar(index) );
		}
		/** Get the index of this polygon if it is a member of a polygon mesh
		 */
		virtual const	TqInt	MeshIndex() const
		{
			return ( m_Index );
		}

		/** Get the start index in arrays of favevarying variables for this face.
		 */
		TqInt FaceVaryingIndex() const
		{
			return( m_FaceVaryingIndex );
		}

		virtual CqSurface* Clone() const
		{
			return(NULL);
		}

	protected:
		std::vector<TqInt>	m_aIndices;		///< Array of indices into the associated vertex list.
		boost::shared_ptr<CqPolygonPoints>	m_pPoints;		///< Pointer to the associated CqPolygonPoints class.
		TqInt	m_Index;		/// Polygon index, used for looking up Uniform values.
		TqInt	m_FaceVaryingIndex;
};

//----------------------------------------------------------------------
/** \class CqSurfacePointsPolygons
 * Container surface to store the polygons making up a RiPointsPolygons surface.
 */

class CqSurfacePointsPolygons : public CqSurface
{
	public:
		CqSurfacePointsPolygons() : m_NumPolys(0)
		{}
		CqSurfacePointsPolygons(const boost::shared_ptr<CqPolygonPoints>& pPoints, TqInt NumPolys, TqInt nverts[], TqInt verts[]) :
				m_NumPolys(NumPolys),
				m_pPoints( pPoints )
		{
			m_PointCounts.resize( NumPolys );
			TqInt i,vindex=0;
			for( i = 0; i < NumPolys; i++ )
			{
				m_PointCounts[i] = nverts[i];
				TqInt polyvertex;
				for( polyvertex = 0; polyvertex < nverts[i]; polyvertex++ )
					m_PointIndices.push_back( verts[vindex++] );
			}
			STATS_INC( GPR_poly );
		}
		virtual	~CqSurfacePointsPolygons()
	{}

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
			assert( m_pPoints );
			m_pPoints->Transform( matTx, matITTx, matRTx, iTime );
		}

		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}

		virtual	TqUint	cUniform() const
		{
			return ( m_NumPolys );
		}
		virtual	TqUint	cVarying() const
		{
			assert( m_pPoints );
			return ( m_pPoints->cVarying() );
		}
		virtual	TqUint	cVertex() const
		{
			assert( m_pPoints );
			return ( m_pPoints->cVarying() );
		}
		virtual	TqUint	cFaceVarying() const
		{
			assert( m_pPoints );
			return ( m_pPoints->cFaceVarying() );
		}
		virtual CqSurface* Clone() const;

	private:
		TqInt	m_NumPolys;
		boost::shared_ptr<CqPolygonPoints>	m_pPoints;		///< Pointer to the associated CqPolygonPoints class.
		std::vector<TqInt>	m_PointCounts;
		std::vector<TqInt>	m_PointIndices;
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !POLYGON_H_INCLUDED
