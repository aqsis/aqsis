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
		\brief Declares the classes and support structures for handling polygons.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef POLYGON_H_INCLUDED
#define POLYGON_H_INCLUDED 1

#include	"aqsis.h"

#include	"surface.h"
#include	"vector4d.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqPolygonBase
 * Polygon base class for split and dice functionality.
 */

class CqPolygonBase
{
	public:
		CqPolygonBase()
		{}
		virtual	~CqPolygonBase()
		{}

		virtual	CqBound	Bound() const;
		virtual	TqInt	Split( std::vector<CqBasicSurface*>& aSplits );

		/** Get a reference to the surface this polygon is associated with.
		 */
		virtual	const CqSurface& Surface() const = 0;
		/** Get a reference to the surface this polygon is associated with.
		 */
		virtual	CqSurface&	Surface() = 0;

		/** Get a reference to the polygon point at the specified index.
		 * \param i Integer index in of the vertex in question.
		 */
		virtual	const	CqVector4D& PolyP( TqInt i ) const = 0;
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

		/** Get the number of vertices in this polygon.
		 */
		virtual	TqInt	NumVertices() const = 0;

		/** Get a pointer to the attributes state associated with this polygon.
		 */
		virtual	const IqAttributes*	pAttributes() const = 0;
		/** Get a pointer to the transfrom associated with this polygon.
		 */
		virtual	const IqTransform*	pTransform() const = 0;


		/** Determine whether this surface has per vertex normals.
		 */
		virtual	const	TqBool	bHasN() const = 0;
		/** Determine whether this surface has per vertex colors.
		 */
		virtual	const	TqBool	bHasCs() const = 0;
		/** Determine whether this surface has per vertex opacities.
		 */
		virtual	const	TqBool	bHasOs() const = 0;
		/** Determine whether this surface has per vertex s cordinates.
		 */
		virtual	const	TqBool	bHass() const = 0;
		/** Determine whether this surface has per vertex t coordinates.
		 */
		virtual	const	TqBool	bHast() const = 0;
		/** Determine whether this surface has per vertex u coordinates.
		 */
		virtual	const	TqBool	bHasu() const = 0;
		/** Determine whether this surface has per vertex v coordinates.
		 */
		virtual	const	TqBool	bHasv() const = 0;
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
		CqSurfacePolygon( const CqSurfacePolygon& From );
		virtual	~CqSurfacePolygon();

#ifdef _DEBUG
		CqString className() const { return CqString("CqSurfacePolygon"); }
#endif

		CqSurfacePolygon& operator=( const CqSurfacePolygon& From );
		TqBool	CheckDegenerate() const;

		// Overridden fro mCqSurface.
		virtual	CqBound	Bound() const
		{
			return ( AdjustBoundForTransformationMotion( CqPolygonBase::Bound() ) );
		}
		virtual	TqInt	Split( std::vector<CqBasicSurface*>& aSplits )
		{
			return ( CqPolygonBase::Split( aSplits ) );
		}

		/** Determine whether the passed surface is valid to be used as a 
		 *  frame in motion blur for this surface.
		 */
		virtual TqBool	IsMotionBlurMatch( CqBasicSurface* pSurf )
		{
			return( TqFalse );
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
			/// \todo Must work out what this value should be.
			return ( 1 );
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

		virtual	const	CqVector4D& PolyP( TqInt i ) const
		{
			return ( ( *P() ) [ i ] );
		}
		virtual	const	CqVector3D& PolyN( TqInt i ) const
		{
			return ( ( *N() ) [ i ] );
		}
		virtual	const	CqColor& PolyCs( TqInt i ) const
		{
			return ( ( *Cs() ) [ i ] );
		}
		virtual	const	CqColor& PolyOs( TqInt i ) const
		{
			return ( ( *Os() ) [ i ] );
		}
		virtual	const	TqFloat& Polys( TqInt i ) const
		{
			return ( ( *s() ) [ i ] );
		}
		virtual	const	TqFloat& Polyt( TqInt i ) const
		{
			return ( ( *t() ) [ i ] );
		}
		virtual	const	TqFloat& Polyu( TqInt i ) const
		{
			return ( ( *u() ) [ i ] );
		}
		virtual	const	TqFloat& Polyv( TqInt i ) const
		{
			return ( ( *v() ) [ i ] );
		}
		virtual	const	TqInt PolyIndex( TqInt i ) const
		{
			return ( i );
		}

		virtual	const	TqBool	bHasN() const
		{
			return ( CqSurface::bHasN() );
		}
		virtual	const	TqBool	bHasCs() const
		{
			return ( CqSurface::bHasCs() );
		}
		virtual	const	TqBool	bHasOs() const
		{
			return ( CqSurface::bHasOs() );
		}
		virtual	const	TqBool	bHass() const
		{
			return ( CqSurface::bHass() );
		}
		virtual	const	TqBool	bHast() const
		{
			return ( CqSurface::bHast() );
		}
		virtual	const	TqBool	bHasu() const
		{
			return ( CqSurface::bHasu() );
		}
		virtual	const	TqBool	bHasv() const
		{
			return ( CqSurface::bHasv() );
		}

		virtual	TqInt	NumVertices() const
		{
			return ( cVertex() );
		}

		virtual	const IqAttributes*	pAttributes() const
		{
			return ( CqSurface::pAttributes() );
		}
		virtual	const IqTransform*	pTransform() const
		{
			return ( CqSurface::pTransform() );
		}

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
		CqPolygonPoints( TqInt cVertices, TqInt cFaces ) :
				m_cVertices( cVertices ),
				m_Transformed( TqFalse ),
				m_cFaces( cFaces )
		{}
		CqPolygonPoints( const CqPolygonPoints& From ) :
				CqSurface( From ),
				m_cVertices( From.m_cVertices ),
				m_Transformed( From.m_Transformed ),
				m_cFaces( From.m_cFaces )
		{}
		virtual	~CqPolygonPoints()
		{
			assert( RefCount() == 0 );
		}

#ifdef _DEBUG
		CqString className() const { return CqString("CqPolygonPoints"); }
#endif

		// Overridden from CqSurface.
		// NOTE: These should never be called.
#ifdef AQSIS_COMPILER_MSVC6
		virtual	CqBound	Bound() const
		{
			static CqBound bTemp; return ( bTemp );
		}
#else // AQSIS_COMPILER_MSVC6
		virtual	CqBound	Bound() const
		{
			return CqBound();
		}
#endif // !AQSIS_COMPILER_MSVC6

		virtual	CqMicroPolyGridBase* Dice()
		{
			return ( 0 );
		}
		virtual	TqInt	Split( std::vector<CqBasicSurface*>& aSplits )
		{
			return ( 0 );
		}
		virtual TqBool	Diceable()
		{
			return ( TqFalse );
		}

		/** Determine whether the passed surface is valid to be used as a 
		 *  frame in motion blur for this surface.
		 */
		virtual TqBool	IsMotionBlurMatch( CqBasicSurface* pSurf )
		{
			return( TqFalse );
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
			/// \todo Must work out what this value should be.
			return ( 1 );
		}

		/** Get the number of vertices in the list.
		 */
		virtual	TqInt	NumVertices() const
		{
			return ( cVertex() );
		}

	protected:
		TqInt	m_cVertices;		///< Count of vertices in this list.
		TqBool	m_Transformed;		///< Flag indicatign that the list has been transformed.
		TqInt	m_cFaces;			///< Expected count of faces referencing this list.
}
;


//----------------------------------------------------------------------
/** \class CqSurfacePointsPolygon
 * Points polygon surface primitive, a single member of the above.
 */

class CqSurfacePointsPolygon : public CqBasicSurface, public CqPolygonBase
{
	public:
		CqSurfacePointsPolygon( CqPolygonPoints* pPoints, TqInt index ) : CqBasicSurface(),
				m_pPoints( pPoints ),
				m_Index( index )
		{
			ADDREF( m_pPoints );
		}
		CqSurfacePointsPolygon( const CqSurfacePointsPolygon& From );
		virtual	~CqSurfacePointsPolygon()
		{
			RELEASEREF( m_pPoints );
		}

		CqSurfacePointsPolygon& operator=( const CqSurfacePointsPolygon& From );

		std::vector<TqInt>&	aIndices()
		{
			return ( m_aIndices );
		}

#ifdef _DEBUG
		CqString className() const { return CqString("CqSurfacePointsPolygon"); }
#endif

		// Overridden from CqBasicSurface
		virtual CqMicroPolyGridBase* Dice()
		{
			assert(TqFalse);
			return( 0 );
		}
		virtual	CqBound	Bound() const
		{
			return ( AdjustBoundForTransformationMotion( CqPolygonBase::Bound() ) );
		}
		virtual	TqInt	Split( std::vector<CqBasicSurface*>& aSplits )
		{
			return ( CqPolygonBase::Split( aSplits ) );
		}
		virtual TqBool	Diceable()	{ return(false); }

		/** Determine whether the passed surface is valid to be used as a 
		 *  frame in motion blur for this surface.
		 */
		virtual TqBool	IsMotionBlurMatch( CqBasicSurface* pSurf )
		{
			return( TqFalse );
		}

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 )
		{
			m_pPoints->Transform( matTx, matITTx, matRTx );
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

		virtual	const CqVector4D& PolyP( TqInt i ) const
		{
			return ( ( *m_pPoints->P() ) [ m_aIndices[ i ] ] );
		}
		virtual	const CqVector3D& PolyN( TqInt i ) const
		{
			return ( ( *m_pPoints->N() ) [ m_aIndices[ i ] ] );
		}
		virtual	const CqColor& PolyCs( TqInt i ) const
		{
			return ( ( *m_pPoints->Cs() ) [ m_aIndices[ i ] ] );
		}
		virtual	const CqColor& PolyOs( TqInt i ) const
		{
			return ( ( *m_pPoints->Os() ) [ m_aIndices[ i ] ] );
		}
		virtual	const TqFloat& Polys( TqInt i ) const
		{
			return ( ( *m_pPoints->s() ) [ m_aIndices[ i ] ] );
		}
		virtual	const TqFloat& Polyt( TqInt i ) const
		{
			return ( ( *m_pPoints->t() ) [ m_aIndices[ i ] ] );
		}
		virtual	const TqFloat& Polyu( TqInt i ) const
		{
			return ( ( *m_pPoints->u() ) [ m_aIndices[ i ] ] );
		}
		virtual	const TqFloat& Polyv( TqInt i ) const
		{
			return ( ( *m_pPoints->v() ) [ m_aIndices[ i ] ] );
		}
		virtual	const	TqInt PolyIndex( TqInt i ) const
		{
			return ( ( (TqUint) i < m_aIndices.size() ) ? m_aIndices[ i ] : m_aIndices.back() );
		}

		virtual	TqInt	NumVertices() const
		{
			return ( m_aIndices.size() );
		}

		virtual	const IqAttributes*	pAttributes() const
		{
			return ( m_pPoints->pAttributes() );
		}
		virtual	const IqTransform*	pTransform() const
		{
			return ( m_pPoints->pTransform() );
		}

		/** Determine whether this surface has per vertex normals.
		 */
		const	TqBool	bHasN() const
		{
			return ( m_pPoints->bHasN() );
		}
		/** Determine whether this surface has per vertex colors.
		 */
		const	TqBool	bHasCs() const
		{
			return ( m_pPoints->bHasCs() );
		}
		/** Determine whether this surface has per vertex opacities.
		 */
		const	TqBool	bHasOs() const
		{
			return ( m_pPoints->bHasOs() );
		}
		/** Determine whether this surface has per vertex s cordinates.
		 */
		const	TqBool	bHass() const
		{
			return ( m_pPoints->bHass() );
		}
		/** Determine whether this surface has per vertex t coordinates.
		 */
		const	TqBool	bHast() const
		{
			return ( m_pPoints->bHast() );
		}
		/** Determine whether this surface has per vertex u coordinates.
		 */
		const	TqBool	bHasu() const
		{
			return ( m_pPoints->bHasu() );
		}
		/** Determine whether this surface has per vertex v coordinates.
		 */
		const	TqBool	bHasv() const
		{
			return ( m_pPoints->bHasv() );
		}
		/** Get the index of this polygon if it is a member of a polygon mesh
		 */
		virtual const	TqInt	MeshIndex() const
		{
			return ( m_Index );
		}



	protected:
		std::vector<TqInt>	m_aIndices;		///< Array of indices into the associated vertex list.
		CqPolygonPoints*	m_pPoints;		///< Pointer to the associated CqPolygonPoints class.
		TqInt	m_Index;		/// Polygon index, used for looking up Uniform values.
};

//----------------------------------------------------------------------
/** \class CqSurfacePointsPolygons
 * Container surface to store the polygons making up a RiPointsPolygons surface.
 */

class CqSurfacePointsPolygons : public CqSurface
{
	public:	
		CqSurfacePointsPolygons(CqPolygonPoints* pPoints, TqInt NumPolys, TqInt nverts[], TqInt verts[]) : 
			m_NumPolys(NumPolys),
			m_pPoints( pPoints )
		{
			ADDREF( m_pPoints );
			m_PointCounts.resize( NumPolys );
			TqInt i,vindex=0;
			for( i = 0; i < NumPolys; i++ )
			{
				m_PointCounts[i] = nverts[i];
				TqInt polyvertex;
				for( polyvertex = 0; polyvertex < nverts[i]; polyvertex++ )
					m_PointIndices.push_back( verts[vindex++] );
			}
		}
		virtual	~CqSurfacePointsPolygons()
		{
			RELEASEREF( m_pPoints );
		}

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
		 * \param aSplits A reference to a CqBasicSurface array to fill in with the new GPrim pointers.
		 * \return Integer count of new GPrims created.
		 */
		virtual	TqInt	Split( std::vector<CqBasicSurface*>& aSplits );
		/** Determine whether this GPrim is diceable at its current size.
		 */
		virtual TqBool	Diceable()
		{
			return( TqFalse );
		}

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 )
		{
			assert( NULL != m_pPoints );
			m_pPoints->Transform( matTx, matITTx, matRTx, iTime );
		}

		virtual TqBool	IsMotionBlurMatch( CqBasicSurface* pSurf )
		{
			return( TqFalse );
		}

		virtual	TqUint	cUniform() const
		{
			return ( m_NumPolys );
		}
		virtual	TqUint	cVarying() const
		{
			assert( NULL != m_pPoints );
			return ( m_pPoints->cVarying() );
		}
		virtual	TqUint	cVertex() const
		{
			assert( NULL != m_pPoints );
			return ( m_pPoints->cVarying() );
		}
		virtual	TqUint	cFaceVarying() const
		{
			/// \todo Must work out what this value should be.
			return ( 1 );
		}

	private:
		TqInt	m_NumPolys;
		CqPolygonPoints*	m_pPoints;		///< Pointer to the associated CqPolygonPoints class.
		std::vector<TqInt>	m_PointCounts;
		std::vector<TqInt>	m_PointIndices;
};

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !POLYGON_H_INCLUDED
