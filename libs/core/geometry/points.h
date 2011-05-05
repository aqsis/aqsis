// Aqsis
// Copyright (C) 2001, Paul C. Gregory
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
		\brief Implements CqPoints primitives using regular polygon (first try).
		\author M. Joron (joron@sympatico.ca)
*/


//? Is .h included already?
#ifndef POINTS_H_INCLUDED
#define POINTS_H_INCLUDED

#include	<aqsis/aqsis.h>

#include	<algorithm>
#include	<functional>

#include	<aqsis/math/matrix.h>
#include	"surface.h"
#include	<aqsis/math/vector4d.h>
#include	"kdtree.h"
#include 	"micropolygon.h"
#include	"imagepixel.h"
#include	<aqsis/ri/ri.h>
#include	"polygon.h"
#include	"imagepixel.h"


namespace Aqsis {


//----------------------------------------------------------------------
/** \class CqPointsKDTreeData
 * Class for handling KDTree data representing the points primitive.
 */

class CqPoints;
class CqBucketProcessor;

class CqPointsKDTreeData : public IqKDTreeData<TqInt>
{
	public:
		CqPointsKDTreeData() : m_pPointsSurface( NULL )
		{}
		virtual ~CqPointsKDTreeData()
		{
		};

		virtual void PartitionElements(std::vector<TqInt>& leavesIn,
				TqInt dimension, std::vector<TqInt>& out1, std::vector<TqInt>& out2);

		virtual TqInt Dimensions() const
		{
			return(3);
		}

		void	SetpPoints( const CqPoints* pPoints );

	private:
		class CqPointsKDTreeDataComparator;

		const CqPoints*	m_pPointsSurface;
};


//----------------------------------------------------------------------
/** \class CqPoints
 * Class encapsulating the functionality of Points geometry.
 */

class CqPoints : public CqSurface
{
	public:

		CqPoints( TqInt nVertices, const boost::shared_ptr<CqPolygonPoints>& pPoints );
		CqPoints() : m_KDTree(&m_KDTreeData)
		{
			m_KDTreeData.SetpPoints(this); 
		}

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
			return ( cVarying() );
		}
		// Overrides from CqSurface
		virtual	CqMicroPolyGridBase* Dice();
		virtual bool	Diceable(const CqMatrix& matCtoR);

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 );

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}

		virtual	void	Bound(CqBound* bound) const;
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );

		virtual CqSurface* Clone() const;

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
		TqInt CopySplit( std::vector<boost::shared_ptr<CqSurface> >& aSplits, CqPoints* pFrom1, CqPoints* pFrom2 );

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

		/// Initialise the KDTree to point to the whole points list.
		void	InitialiseKDTree();
		void    InitialiseMaxWidth();

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

	private:
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
		virtual	~CqMicroPolyGridPoints()
		{}

		virtual	void	Split( long xmin, long xmax, long ymin, long ymax );

		virtual	TqUint	GridSize() const
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->uGridRes() );
		}
		virtual	TqUint	numShadingPoints(TqInt cu, TqInt cv) const
		{
			return ( cu * cv );
		}
		virtual bool	hasValidDerivatives() const
		{
			return false;
		}
		/** \brief Set surface derivative in u.
		 * 
		 *  Set the value of du if needed, du is constant across the grid being shaded.
		 */
		virtual void setDu();
		/** \brief Set surface derivative in v.
		 * 
		 *  Set the value of dv if needed, dv is constant across the grid being shaded.
		 */
		virtual void setDv();
		virtual void CalcNormals();
		virtual void CalcSurfaceDerivatives();
};

class CqMotionMicroPolyGridPoints : public CqMotionMicroPolyGrid
{
	public:
		CqMotionMicroPolyGridPoints() : CqMotionMicroPolyGrid()
		{}
		virtual	~CqMotionMicroPolyGridPoints()
		{}

		virtual	void	Split( long xmin, long xmax, long ymin, long ymax );
};

//----------------------------------------------------------------------
/** \class CqMicroPolygonPoints
 * Specialised micropolygon class for points.
 */

class CqMicroPolygonPoints : public CqMicroPolygon
{
	public:
		CqMicroPolygonPoints( CqMicroPolyGridBase* pGrid, TqInt Index ) : CqMicroPolygon(pGrid, Index)
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

			// compute the bound.
			CqVector3D pos;
			pGrid()->pVar(EnvVars_P)->GetPoint(pos, m_Index);
			m_Bound.vecMin() = pos - CqVector3D(m_radius, m_radius, 0);
			m_Bound.vecMax() = pos + CqVector3D(m_radius, m_radius, 0);
		}
		virtual	bool	Sample( CqHitTestCache& hitTestCache, SqSampleData const& sample, TqFloat& D, CqVector2D& uv, TqFloat time, bool UsingDof = false ) const;
		virtual void CacheHitTestValues(CqHitTestCache& cache, bool usingDof) const;

		virtual void CacheOutputInterpCoeffs(SqMpgSampleInfo& cache) const;
		virtual void InterpolateOutputs(const SqMpgSampleInfo& cache,
				const CqVector2D& pos, CqColor& outCol, CqColor& outOpac) const;

	private:
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
		virtual ~CqMovingMicroPolygonKeyPoints()
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
		CqBound	GetBound() const
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
		CqMicroPolygonMotionPoints(CqMicroPolyGridBase* pGrid, TqInt Index) : 
			CqMicroPolygon(pGrid, Index), m_BoundReady( false )
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
		void	DeleteVariables( bool all )
		{}

		virtual	TqInt	cSubBounds( TqUint timeRanges )
		{
			if ( !m_BoundReady )
				BuildBoundList( timeRanges );
			return ( m_BoundList.Size() );
		}
		virtual	CqBound			SubBound( TqInt iIndex, TqFloat& time )
		{
			if ( !m_BoundReady )
			{
				Aqsis::log() << error << "MP Bound list not ready" << std::endl;
				AQSIS_THROW_XQERROR(XqInternal, EqE_Bug, "MP error");
			}
			assert( iIndex < m_BoundList.Size() );
			time = m_BoundList.GetTime( iIndex );
			return ( m_BoundList.GetBound( iIndex ) );
		}
		virtual void	BuildBoundList( TqUint timeRanges );

		virtual bool IsMoving() const
		{
			return true;
		}
		virtual	bool	Sample( CqHitTestCache& hitTestCache, SqSampleData const& sample, TqFloat& D, CqVector2D& uv, TqFloat time, bool UsingDof = false ) const;
		virtual void CacheHitTestValues(CqHitTestCache& cache, bool usingDof) const;
		virtual void CacheOutputInterpCoeffs(SqMpgSampleInfo& cache) const;
		virtual void InterpolateOutputs(const SqMpgSampleInfo& cache,
				const CqVector2D& pos, CqColor& outCol, CqColor& outOpac) const;
	private:
		CqBoundList	m_BoundList;			///< List of bounds to get a tighter fit.
		bool	m_BoundReady;				///< Flag indicating the boundary has been initialised.
		std::vector<TqFloat> m_Times;
		std::vector<CqMovingMicroPolygonKeyPoints*>	m_Keys;

		static	CqObjectPool<CqMicroPolygonMotionPoints>	m_thePool;

};


//----------------------------------------------------------------------
/** \class CqDeformingPointsSurface
 * Templatised class containing a series of motion stages of a specific surface type for motion blurring.
 */

class CqDeformingPointsSurface : public CqDeformingSurface
{
	public:
		CqDeformingPointsSurface( const boost::shared_ptr<CqSurface>& a ) : CqDeformingSurface( a )
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
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
		{
			std::vector<std::vector<boost::shared_ptr<CqSurface> > > aaMotionSplits;
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
				boost::shared_ptr<CqDeformingPointsSurface> pNewMotion( new CqDeformingPointsSurface( boost::shared_ptr<CqSurface>() ) );
				pNewMotion->m_fDiceable = true;
				pNewMotion->m_SplitCount = m_SplitCount + 1;
				TqInt j;
				for ( j = 0; j < cTimes(); j++ )
					pNewMotion->AddTimeSlot( Time( j ), aaMotionSplits[ j ][ i ] );
				aSplits.push_back( pNewMotion );
			}
			return ( cSplits );
		}
	protected:
};

//==============================================================================
// Implementation details
//==============================================================================

inline void CqMicroPolyGridPoints::setDu()
{
	pVar(EnvVars_du)->SetFloat(1.0f);
}

inline void CqMicroPolyGridPoints::setDv()
{
	pVar(EnvVars_dv)->SetFloat(1.0f);
}

inline void	CqMicroPolyGridPoints::CalcNormals()
{
}

inline void	CqMicroPolyGridPoints::CalcSurfaceDerivatives()
{
}


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !POINTS_H_INCLUDED
