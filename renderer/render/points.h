// Aqsis
// Copyright © 2001, Paul C. Gregory
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
		\brief Implements CqPoints primitives using regular polygon (first try).
		\author M. Joron (joron@sympatico.ca)
*/


//? Is .h included already?
#ifndef POINTS_H_INCLUDED
#define POINTS_H_INCLUDED

#include	"aqsis.h"
#include	"matrix.h"
#include	"surface.h"
#include	"vector4d.h"
#include	"kdtree.h"

#include	"ri.h"

#include        "polygon.h"

#include	<algorithm>
#include	<functional>

START_NAMESPACE( Aqsis )


//----------------------------------------------------------------------
/** \class CqPointsKDTreeData
 * Class for handling KDTree data representing the points primitive.
 */

class CqPoints;
class CqPointsKDTreeData : public IqKDTreeData<TqInt>
{
		class CqPointsKDTreeDataComparator
		{
			public:
				CqPointsKDTreeDataComparator(const CqPoints* pPoints, TqInt dimension) : m_pPointsSurface( pPoints ), m_Dim( dimension )
				{}

				bool operator()(TqInt a, TqInt b);

			private:
				const CqPoints*	m_pPointsSurface;
				TqInt		m_Dim;
		};

	public:
		CqPointsKDTreeData( const CqPoints* pPoints ) : m_pPointsSurface( pPoints )
		{}

		virtual void SortElements(std::vector<TqInt>& aLeaves, TqInt dimension)
		{
			std::sort(aLeaves.begin(), aLeaves.end(), CqPointsKDTreeDataComparator(m_pPointsSurface, dimension) );
		}
		virtual TqInt Dimensions() const
		{
			return(3);
		}

		void	SetpPoints( const CqPoints* pPoints );
		void	FreePoints();

	private:
		const CqPoints*	m_pPointsSurface;
};


//----------------------------------------------------------------------
/** \class CqPoints
 * Class encapsulating the functionality of Points geometry.
 */

class CqPoints : public CqSurface//, public CqMotionSpec<boost::shared_ptr<CqPolygonPoints> >
{
	public:

		CqPoints( TqInt nVertices, const boost::shared_ptr<CqPolygonPoints>& pPoints );

		virtual	~CqPoints()
		{}

#ifdef _DEBUG
		CqString className() const
		{
			return CqString("CqPoints");
		}
#endif

		virtual	TqUint	cUniform() const
		{
			return ( 1 );
		}
		virtual	TqUint	cVarying() const
		{
			return ( m_nVertices );
		}
		virtual	TqUint	cVertex() const
		{
			return ( m_nVertices );
		}
		virtual	TqUint	cFaceVarying() const
		{
			/// \todo Must work out what this value should be.
			return ( m_nVertices );
		}
		// Overrides from CqSurface
		virtual	CqMicroPolyGridBase* Dice();
		virtual TqBool	Diceable();

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 );

		virtual void	RenderComplete()
		{
			ClearKDTree();
			CqBasicSurface::RenderComplete();
		}
		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual TqBool	IsMotionBlurMatch( CqBasicSurface* pSurf )
		{
			return( TqFalse );
		}

		virtual	CqBound	Bound() const;
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqBasicSurface> >& aSplits );

		virtual CqBasicSurface* Clone() const
		{
			//return(new CqPoints(*this));
			return(NULL);
		}

		TqUint	nVertices() const
		{
			return ( m_nVertices );
		}

		boost::shared_ptr<CqPolygonPoints> pPoints( TqInt TimeIndex = 0 )
		{
			return(m_pPoints);
		//	return( GetMotionObject( Time( TimeIndex ) ) );
		}
		boost::shared_ptr<CqPolygonPoints> pPoints( TqInt TimeIndex = 0) const
		{
			return(m_pPoints);
		//	return( GetMotionObject( Time( TimeIndex ) ) );
		}

		const std::vector<CqParameter*>& aUserParams() const
		{
			return ( pPoints()->aUserParams() );
		}
		TqInt CopySplit( std::vector<boost::shared_ptr<CqBasicSurface> >& aSplits, CqPoints* pFrom1, CqPoints* pFrom2 );

		/** Returns a const reference to the "constantwidth" parameter, or
		 * NULL if the parameter is not present. */
		const CqParameterTypedConstant <
		TqFloat, type_float, TqFloat
		> * constantwidth() const
		{
			if ( m_constantwidthParamIndex >= 0 )
			{
				return static_cast <
				       const CqParameterTypedConstant <
				       TqFloat, type_float, TqFloat
				       > *
				       > ( aUserParams()[ m_constantwidthParamIndex ] );
			}
			else
			{
				return ( NULL );
			}
		}

		/** Returns a const reference to the "width" parameter, or NULL if
		 * the parameter is not present. */
		const CqParameterTypedVarying <
		TqFloat, type_float, TqFloat
		> * width( TqInt iTime ) const
		{
			if ( m_widthParamIndex >= 0 )
			{
				return static_cast <
				       const CqParameterTypedVarying <
				       TqFloat, type_float, TqFloat
				       > *
				       > ( pPoints( iTime )->aUserParams()[ m_widthParamIndex ] );
			}
			else
			{
				return ( NULL );
			}

		}
		/** Returns a reference to the "width" parameter, or NULL if
		 * the parameter is not present. */
		CqParameterTypedVarying <
		TqFloat, type_float, TqFloat
		> * width( TqInt iTime )
		{
			if ( m_widthParamIndex >= 0 )
			{
				return static_cast <
				       CqParameterTypedVarying <
				       TqFloat, type_float, TqFloat
				       > *
				       > ( pPoints( iTime )->aUserParams()[ m_widthParamIndex ] );
			}
			else
			{
				return ( NULL );
			}

		}

		/** Get a reference to the user parameter variables array
		 */
		std::vector<CqParameter*>& aUserParams()
		{
			return ( pPoints()->aUserParams() );
		}

		/// Accessor function for the KDTree
		CqKDTree<TqInt>&	KDTree()
		{
			return( m_KDTree);
		}
		const CqKDTree<TqInt>&	KDTree() const
		{
			return( m_KDTree);
		}

		void ClearKDTree()
		{
			m_KDTreeData.FreePoints();
		}

		/// Initialise the KDTree to point to the whole points list.
		void	InitialiseKDTree();
		void CqPoints::InitialiseMaxWidth();

		virtual void	NaturalDice( CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData );

		// Overrides from CqMotionSpec
		virtual	void	ClearMotionObject( boost::shared_ptr<CqPolygonPoints>& A ) const
			{}
		;
		virtual	boost::shared_ptr<CqPolygonPoints> ConcatMotionObjects( const boost::shared_ptr<CqPolygonPoints> & A, const boost::shared_ptr<CqPolygonPoints> & B ) const
		{
			return ( A );
		}
		virtual	boost::shared_ptr<CqPolygonPoints> LinearInterpolateMotionObjects( TqFloat Fraction, const boost::shared_ptr<CqPolygonPoints>& A, const boost::shared_ptr<CqPolygonPoints>& B ) const
		{
			return ( A );
		}

	protected:
		template <class T, class SLT>
		void	TypedNaturalDice( CqParameterTyped<T, SLT>* pParam, IqShaderData* pData )
		{
			TqUint i;
			for ( i = 0; i < nVertices(); i++ )
				pData->SetValue( static_cast<SLT>( pParam->pValue() [ m_KDTree.aLeaves()[ i ] ] ), i );
		}

	private:
/* 		// The copy constructor and assignment operator are private because
 * 		// there's a slight "gotcha" in how to use them.  You need to call
 * 		// InitialiseKDTree() on a CqPoints after constructing it.  Since
 * 		// this is only used from within CqPoints::Split*(), we can safely
 * 		// make this private.
 * 
 * 		CqPoints( const CqPoints& From ) : //CqMotionSpec<boost::shared_ptr<CqPolygonPoints> >(From.pPoints()),
 * 				m_KDTreeData( this ),
 * 				m_KDTree( &m_KDTreeData )
 * 		{
 * 			*this = From;
 * 		}
 */

		boost::shared_ptr<CqPolygonPoints> m_pPoints;				///< Pointer to the surface storing the primtive variables.
		TqInt	m_nVertices;					///< Number of points this surfaces represents.
		CqPointsKDTreeData	m_KDTreeData;		///< KD Tree data handling class.
		CqKDTree<TqInt>		m_KDTree;			///< KD Tree node for this part of the entire primitive.
		TqInt m_widthParamIndex;				///< Index of the "width" primitive variable if specified, -1 if not.
		TqInt m_constantwidthParamIndex;		///< Index of the "constantwidth" primitive variable if specified, -1 if not.
		TqFloat	m_MaxWidth;						///< Maximum width of the points, used for bound calculation.
};


class CqMicroPolyGridPoints : public CqMicroPolyGrid
{
	public:
		CqMicroPolyGridPoints() : CqMicroPolyGrid()
		{}
		CqMicroPolyGridPoints( TqInt cu, TqInt cv, const boost::shared_ptr<CqSurface>& pSurface ) : CqMicroPolyGrid( cu, cv, pSurface )
		{}
		virtual	~CqMicroPolyGridPoints()
		{}

		virtual	void	Split( CqImageBuffer* pImage, long xmin, long xmax, long ymin, long ymax );

		virtual	TqInt	GridSize() const
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->uGridRes() );
		}
};

class CqMotionMicroPolyGridPoints : public CqMotionMicroPolyGrid
{
	public:
		CqMotionMicroPolyGridPoints() : CqMotionMicroPolyGrid()
		{}
		virtual	~CqMotionMicroPolyGridPoints()
		{}

		virtual	void	Split( CqImageBuffer* pImage, long xmin, long xmax, long ymin, long ymax );
};

//----------------------------------------------------------------------
/** \class CqMicroPolygonPoints
 * Specialised micropolygon class for points.
 */

class CqMicroPolygonPoints : public CqMicroPolygon
{
	public:
		CqMicroPolygonPoints() : CqMicroPolygon()
		{}
		virtual	~CqMicroPolygonPoints()
		{}

		/** Overridden operator new to allocate micropolys from a pool.
		    */
		void* operator new( size_t size )
		{
			return( m_thePool.alloc() );
		}

		/** Overridden operator delete to allocate micropolys from a pool.
		 */
		void operator delete( void* p )
		{
			m_thePool.free( reinterpret_cast<CqMicroPolygonPoints*>(p) );
		}

	public:
		void Initialise( TqFloat radius )
		{
			m_radius = radius;
		}
		virtual	CqBound&			GetTotalBound( )
		{
			static CqBound b;
			CqVector3D Pmin, Pmax;
			pGrid()->pVar(EnvVars_P)->GetPoint(Pmin, m_Index);
			Pmax = Pmin;
			Pmin.x( Pmin.x() - m_radius );
			Pmin.y( Pmin.y() - m_radius );
			Pmax.x( Pmax.x() + m_radius );
			Pmax.y( Pmax.y() + m_radius );
			b.vecMin() = Pmin;
			b.vecMax() = Pmax;
			return( b );
		}
		virtual	TqBool	Sample( const SqSampleData& sample, TqFloat& D, TqFloat time, TqBool UsingDof = TqFalse );


	private:
		CqMicroPolygonPoints( const CqMicroPolygonPoints& From )
		{}

		TqFloat	m_radius;

		static	CqObjectPool<CqMicroPolygonPoints>	m_thePool;
}
;

//----------------------------------------------------------------------
/** \class CqMovingMicroPolygonKey
 * Base lass for static micropolygons. Stores point information about the geometry of the micropoly.
 */

class CqMovingMicroPolygonKeyPoints
{
	public:
		CqMovingMicroPolygonKeyPoints()
		{}
		CqMovingMicroPolygonKeyPoints( const CqVector3D& vA, TqFloat radius)
		{
			Initialise( vA, radius );
		}
		~CqMovingMicroPolygonKeyPoints()
		{}

		/** Overridden operator new to allocate micropolys from a pool.
		    */
		void* operator new( size_t size )
		{
			return( m_thePool.alloc() );
		}

		/** Overridden operator delete to allocate micropolys from a pool.
		 */
		void operator delete( void* p )
		{
			m_thePool.free( reinterpret_cast<CqMovingMicroPolygonKeyPoints*>(p) );
		}

	public:
		TqBool	fContains( const CqVector2D& vecP, TqFloat& Depth, TqFloat time ) const
		{
			if( (CqVector2D( m_Point0.x(), m_Point0.y() ) - vecP).Magnitude() < m_radius )
			{
				Depth = m_Point0.z();
				return( TqTrue );
			}
			return( TqFalse );
		}

		CqBound	GetTotalBound() const
		{
			CqVector3D Pmin, Pmax;
			Pmin = Pmax = m_Point0;
			Pmin.x( Pmin.x() - m_radius );
			Pmin.y( Pmin.y() - m_radius );
			Pmax.x( Pmax.x() + m_radius );
			Pmax.y( Pmax.y() + m_radius );
			return( CqBound( Pmin, Pmax ) );
		}

		void	Initialise( const CqVector3D& vA, TqFloat radius )
		{
			m_Point0 = vA;
			m_radius = radius;
		}

		CqVector3D	m_Point0;
		TqFloat		m_radius;

		static	CqObjectPool<CqMovingMicroPolygonKeyPoints>	m_thePool;
}
;


//----------------------------------------------------------------------
/** \class CqMicroPolygonMotion
 * Class which stores a single moving micropolygon.
 */

class CqMicroPolygonMotionPoints : public CqMicroPolygon
{
	public:
		CqMicroPolygonMotionPoints() : CqMicroPolygon(), m_BoundReady( TqFalse )
		{ }
		virtual	~CqMicroPolygonMotionPoints()
		{
			std::vector<CqMovingMicroPolygonKeyPoints*>::iterator	ikey;
			for( ikey = m_Keys.begin(); ikey != m_Keys.end(); ikey++ )
				delete( (*ikey) );
		}

		/** Overridden operator new to allocate micropolys from a pool.
		    */
		void* operator new( size_t size )
		{
			return( m_thePool.alloc() );
		}

		/** Overridden operator delete to allocate micropolys from a pool.
		 */
		void operator delete( void* p )
		{
			m_thePool.free( reinterpret_cast<CqMicroPolygonPoints*>(p) );
		}

	public:
		void	AppendKey( const CqVector3D& vA, TqFloat radius, TqFloat time );
		void	DeleteVariables( TqBool all )
		{}

		// Overrides from CqMicroPolygon
		virtual TqBool	fContains( const CqVector2D& vecP, TqFloat& Depth, TqFloat time ) const;
		virtual void CalculateTotalBound();
		virtual	CqBound&		GetTotalBound( /*TqBool fForce = TqFalse */);
		virtual const CqBound&	GetTotalBound() const
		{
			return ( m_Bound );
		}
		virtual	TqInt			cSubBounds()
		{
			if ( !m_BoundReady )
				BuildBoundList();
			return ( m_BoundList.Size() );
		}
		virtual	CqBound			SubBound( TqInt iIndex, TqFloat& time )
		{
			if ( !m_BoundReady )
				BuildBoundList();
			assert( iIndex < m_BoundList.Size() );
			time = m_BoundList.GetTime( iIndex );
			return ( m_BoundList.GetBound( iIndex ) );
		}
		virtual void	BuildBoundList();

		virtual TqBool IsMoving()
		{
			return TqTrue;
		}
		virtual	TqBool	Sample( const SqSampleData& sample, TqFloat& D, TqFloat time, TqBool UsingDof = TqFalse );
	private:
		CqBound	m_Bound;					///< Stored bound.
		CqBoundList	m_BoundList;			///< List of bounds to get a tighter fit.
		TqBool	m_BoundReady;				///< Flag indicating the boundary has been initialised.
		std::vector<TqFloat> m_Times;
		std::vector<CqMovingMicroPolygonKeyPoints*>	m_Keys;

		CqMicroPolygonMotionPoints( const CqMicroPolygonMotionPoints& From )
		{}

		static	CqObjectPool<CqMicroPolygonMotionPoints>	m_thePool;

}
;


//----------------------------------------------------------------------
/** \class CqDeformingPointsSurface
 * Templatised class containing a series of motion stages of a specific surface type for motion blurring.
 */

class CqDeformingPointsSurface : public CqDeformingSurface
{
	public:
		CqDeformingPointsSurface( const boost::shared_ptr<CqBasicSurface>& a ) : CqDeformingSurface( a )
		{}
		virtual	~CqDeformingPointsSurface()
		{}

		/** Dice this GPrim, creating a CqMotionMicroPolyGrid with all times in.
		 */
		virtual	CqMicroPolyGridBase* Dice()
		{
			CqMotionMicroPolyGridPoints * pGrid = new CqMotionMicroPolyGridPoints;
			TqInt i;
			for ( i = 0; i < cTimes(); i++ )
			{
				CqMicroPolyGridBase* pGrid2 = GetMotionObject( Time( i ) ) ->Dice();
				pGrid->AddTimeSlot( Time( i ), pGrid2 );
				ADDREF( pGrid2);
			}
			return ( pGrid );
		}
		/** Split this GPrim, creating a series of CqDeformingSurface with all times in.
		 */
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqBasicSurface> >& aSplits )
		{
			std::vector<std::vector<boost::shared_ptr<CqBasicSurface> > > aaMotionSplits;
			aaMotionSplits.resize( cTimes() );
			TqInt cSplits = 0;
			TqInt i;
			cSplits = GetMotionObject( Time( 0 ) ) ->Split( aaMotionSplits[ 0 ] );
			CqPoints* pFrom1 = static_cast<CqPoints*>(aaMotionSplits[ 0 ][ 0 ].get());
			CqPoints* pFrom2 = static_cast<CqPoints*>(aaMotionSplits[ 0 ][ 1 ].get());

			// Now we have the appropriate split information, use this to make sure the rest
			// of the keyframes split at the same point.
			for ( i = 1; i < cTimes(); i++ )
				cSplits = static_cast<CqPoints*>( GetMotionObject( Time( i ) ).get() ) ->CopySplit( aaMotionSplits[ i ], pFrom1, pFrom2 );

			// Now build motion surfaces from the splits and pass them back.
			for ( i = 0; i < cSplits; i++ )
			{
				boost::shared_ptr<CqDeformingPointsSurface> pNewMotion( new CqDeformingPointsSurface( boost::shared_ptr<CqBasicSurface>() ) );
				pNewMotion->m_fDiceable = TqTrue;
				pNewMotion->m_EyeSplitCount = m_EyeSplitCount;
				TqInt j;
				for ( j = 0; j < cTimes(); j++ )
					pNewMotion->AddTimeSlot( Time( j ), aaMotionSplits[ j ][ i ] );
				aSplits.push_back( pNewMotion );
			}
			return ( cSplits );
		}
		virtual void	RenderComplete()
		{
			TqInt i;
			for ( i = 0; i < cTimes(); i++ )
			{
				CqPoints* Points = static_cast<CqPoints*>( GetMotionObject( Time( i ) ).get() );
				Points->ClearKDTree();
			}
			CqBasicSurface::RenderComplete();
		}
	protected:
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !POINTS_H_INCLUDED
