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
		\brief Declares the classes for handling micropolygrids and micropolygons.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef MICROPOLYGON_H_INCLUDED
#define MICROPOLYGON_H_INCLUDED 1

#include	"ri.h"

#include	"aqsis.h"

#include	"memorypool.h"
#include	"color.h"
#include	"list.h"
#include	"bound.h"
#include	"vector2d.h"
#include	"vector3d.h"
#include	"vector4d.h"
#include	"shaderexecenv.h"
#include	"shadervariable.h"
#include	"motion.h"
#include	"csgtree.h"

START_NAMESPACE( Aqsis )

class CqVector3D;
class CqImageBuffer;
class CqSurface;
class CqAttributes;

//----------------------------------------------------------------------
/** \class CqMicroPolyGridBase
 * Base class from which all MicroPolyGrids are derived.
 */

class CqMicroPolyGridBase
{
	public:
		CqMicroPolyGridBase()
		{}
		virtual	~CqMicroPolyGridBase()
		{}

		/** Pure virtual function, splits the grid into micropolys.
		 * \param pImage Pointer to the image buffer being rendered.
		 * \param iBucket Index of the bucket begin processed.
		 * \param xmin The minimum x pixel, taking into account clipping etc.
		 * \param xmax The maximum x pixel, taking into account clipping etc.
		 * \param ymin The minimum y pixel, taking into account clipping etc.
		 * \param xmax The maximum y pixel, taking into account clipping etc.
		 */
		virtual	void	Split( CqImageBuffer* pImage, TqInt iBucket, long xmin, long xmax, long ymin, long ymax ) = 0;
		/** Pure virtual, project the grid into raster space.
		 */
		virtual void	Project() = 0;
		/** Pure virtual, shade the grid.
		 */
		virtual	void	Shade() = 0;
		/** Pure virtual, get the bound of the grid.
		 * \return CqBound class representing the conservative boundary.
		 */
		virtual CqBound	Bound() = 0;
		/** Pure virtual, get a pointer to the surface this grid belongs.
		 * \return Pointer to surface, only valid during grid shading.
		 */
		virtual CqSurface*	pSurface() const = 0;

		virtual	CqAttributes* pAttributes() const = 0;

		virtual	CqCSGTreeNode* pCSGNode() const = 0;

	private:
};


//----------------------------------------------------------------------
/** \class CqMicroPolyGrid
 * Class which stores a grid of micropolygons.
 */

class CqMicroPolyGrid : public CqMicroPolyGridBase, public CqShaderExecEnv
{
	public:
		CqMicroPolyGrid();
		CqMicroPolyGrid( TqInt cu, TqInt cv, CqSurface* pSurface );
		virtual	~CqMicroPolyGrid();

		void	CalcNormals();
		/** Set the shading normals flag, indicating this grid has shading (N) normals already specified.
		 * \param f The new state of the flag.
		 */
		void	SetbShadingNormals( TqBool f )
		{
			m_bShadingNormals = f;
		}
		/** Set the geometric normals flag, indicating this grid has geometric (Ng) normals already specified.
		 * \param f The new state of the flag.
		 */
		void	SetbGeometricNormals( TqBool f )
		{
			m_bGeometricNormals = f;
		}
		/** Query whether shading (N) normals have been filled in by the surface at dice time.
		 */
		TqBool bShadingNormals() const
		{
			return(m_bShadingNormals);
		}
		/** Query whether geometric (Ng) normals have been filled in by the surface at dice time.
		 */
		TqBool bGeometricNormals() const
		{
			return(m_bGeometricNormals);
		}
		
		void	Initialise( TqInt cu, TqInt cv, CqSurface* pSurface );

		/** Increase the count of references to this grid.
		 */
		void	AddRef()
		{
			m_cReferences++;
		}
		/** Decrease the count of references to this grid. Delete it if no more references.
		 */
		void	Release()
		{
			assert( m_cReferences > 0 );
			m_cReferences--;
			if ( m_cReferences == 0 )
				delete( this );

		}

		// Overrides from CqMicroPolyGridBase
		virtual	void	Split( CqImageBuffer* pImage, TqInt iBucket, long xmin, long xmax, long ymin, long ymax );
		virtual void	Project();
		virtual	void	Shade();
		virtual CqBound	Bound();
		/** Get a pointer to the surface which this grid belongs.
		 * \return Surface pointer, only valid during shading.
		 */
		virtual CqSurface*	pSurface() const
		{
			return ( CqShaderExecEnv::pSurface() );
		}
		virtual	CqAttributes* pAttributes() const
		{
			return ( m_pAttributes );
		}
		virtual	CqCSGTreeNode* pCSGNode() const
		{
			return ( m_pCSGNode );
		}

	private:
		TqBool	m_bShadingNormals;		///< Flag indicating shading normals have been filled in and don't need to be calculated during shading.
		TqBool	m_bGeometricNormals;	///< Flag indicating geometric normals have been filled in and don't need to be calculated during shading.
		TqInt	m_cReferences;		///< Count of references to this grid.
		CqAttributes* m_pAttributes;	///< Pointer to the attributes for this grid.
		CqCSGTreeNode* m_pCSGNode;	///< Pointer to the CSG tree node this grid belongs to, NULL if not part of a solid.
}
;


//----------------------------------------------------------------------
/** \class CqMotionMicroPolyGrid
 * Class which stores info about motion blurred micropolygrids.
 */

class CqMotionMicroPolyGrid : public CqMicroPolyGridBase, public CqMotionSpec<CqMicroPolyGridBase*>
{
	public:
		CqMotionMicroPolyGrid() : CqMicroPolyGridBase(), CqMotionSpec<CqMicroPolyGridBase*>( 0 )
		{}
		virtual	~CqMotionMicroPolyGrid()
		{}

		// Overrides from CqMicroPolyGridBase


		virtual	void	Split( CqImageBuffer* pImage, TqInt iBucket, long xmin, long xmax, long ymin, long ymax );
		virtual void	Project();
		virtual	void	Shade();
		virtual CqBound	Bound();
		/** Get a pointer to the surface which this grid belongs.
		 * Actually returns the surface pointer from the first timeslot.
		 * \return Surface pointer, only valid during shading.
		 */
		virtual CqSurface*	pSurface() const
		{
			return ( static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( 0 ) ) ) ->pSurface() );
		}

		virtual CqAttributes* pAttributes() const
		{
			return ( static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( 0 ) ) ) ->pAttributes() );
		}

		virtual CqCSGTreeNode* pCSGNode() const
		{
			return ( static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( 0 ) ) ) ->pCSGNode() );
		}

		virtual	void	ClearMotionObject( CqMicroPolyGridBase*& A ) const
			{}
		/** Overridden from CqMotionSpec, does nothing.
		 */
		virtual	CqMicroPolyGridBase* ConcatMotionObjects( CqMicroPolyGridBase* const & A, CqMicroPolyGridBase* const & B ) const
		{
			return ( B );
		}
		/** Overridden from CqMotionSpec, does nothing.
		 */
		virtual	CqMicroPolyGridBase* LinearInterpolateMotionObjects( TqFloat Fraction, CqMicroPolyGridBase* const & A, CqMicroPolyGridBase* const & B ) const
		{
			return ( A );
		}

	private:
};


//----------------------------------------------------------------------
/** \class CqMicroPolygonBase
 * Abstract base class from which static and motion micropolygons are derived.
 */

class CqMicroPolygonBase
{
	public:
		CqMicroPolygonBase();
		CqMicroPolygonBase( const CqMicroPolygonBase& From );
		virtual	~CqMicroPolygonBase();

	public:
		/** Set up the pointer to the grid this micropoly came from.
		 * \param pGrid CqMicroPolyGrid pointer.
		 */
		void	SetGrid( CqMicroPolyGrid* pGrid )
		{
			if ( m_pGrid ) m_pGrid->Release();
			m_pGrid = pGrid;
			m_pGrid->AddRef();
		}
		/** Get the pointer to the grid this micropoly came from.
		 * \return Pointer to the CqMicroPolyGrid.
		 */
		CqMicroPolyGrid* pGrid() const
		{
			return ( m_pGrid );
		}
		/** Set the index within the donor grid.
		 * \param Index Integer grid index.
		 */
		void	SetIndex( TqInt Index )
		{
			assert( m_pGrid != 0 && m_pGrid->GridSize() > Index );
			m_Index = Index;
		}
		/** Release this micropolys reference to the donor grid.
		 */
		void	Detatch()
		{
			if ( m_pGrid != 0 )
			{
				m_pGrid->Release();
				m_pGrid = 0;
			}
		}
		/** Increment the reference count on this micropoly.
		 */
		void	AddRef()
		{
			m_RefCount++;
		}
		/** Decrement the reference count on this micropoly. Delete it if no longer referenced.
		 */
		void	Release()
		{
			m_RefCount--;
			if ( m_RefCount <= 0 )
				delete( this );
		}
		/** Get the color of this micropoly.
		 * \return CqColor reference.
		 */
		const	CqColor	colColor() const
		{
			CqColor colRes;
			CqVMStackEntry SE;
			m_pGrid->Ci()->GetValue(m_Index, SE);
			SE.Value( colRes );
			return ( colRes );
		}
		/** Get the opacity of this micropoly.
		 * \return CqColor reference.
		 */
		const	CqColor	colOpacity() const
		{
			CqColor colRes;
			CqVMStackEntry SE;
			m_pGrid->Oi()->GetValue(m_Index, SE);
			SE.Value(colRes);
			return ( colRes );
		}

		/** Assigment operator, copies contents of donor micropoly while safely deleting old contents.
		 * \param From Donor micropoly.
		 */
		CqMicroPolygonBase& operator=( const CqMicroPolygonBase& From )
		{
			if ( m_pGrid != NULL ) m_pGrid->Release();
			m_pGrid = From.m_pGrid;
			m_pGrid->AddRef();
			m_Index = From.m_Index;

			return ( *this );
		}
		// Overridables
		/** Pure virtual, get the bound of the micropoly.
		 * \param fForce Flag indicating do not get the stored bound, but recalculate it.
		 * \return CqBound representing the conservative bound.
		 */
		virtual	CqBound&	GetTotalBound( TqBool fForce = TqFalse ) = 0;
		/** Pure virtual, get the bound of the micropoly.
		 * \return CqBound representing the conservative bound.
		 */
		virtual const CqBound&	GetTotalBound() const = 0;
		virtual	TqInt	cSubBounds() = 0;
		virtual	CqBound&	SubBound( TqInt iIndex, TqFloat& time ) = 0;

		/** Pure virtual, check if the sample point is within the micropoly.
		 * \param vecSample 2D sample point.
		 * \param time The frame time at which to check.
		 * \param D storage to put the depth at the sample point if success.
		 * \return Boolean success.
		 */
		virtual	TqBool	Sample( CqVector2D& vecSample, TqFloat time, TqFloat& D ) = 0;

		/** Set the flag to state that the MPG has eben hit by a sample point.
		 */
		void BeenHit()	{ m_bHit = TqTrue; }

	protected:
		CqMicroPolyGrid*	m_pGrid;		///< Pointer to the donor grid.
		TqInt	m_Index;		///< Index within the donor grid.
		TqInt	m_RefCount;		///< Number of references to this micropoly.
		TqBool	m_bHit;			///< Flag indicating the MPG has been sampled.
}
;


//----------------------------------------------------------------------
/** \class CqMicroPolygonStaticBase
 * Base lass for static micropolygons. Stores point information about the geometry of the micropoly.
 */

class CqMicroPolygonStaticBase
{
	public:
		CqMicroPolygonStaticBase()
		{}
		CqMicroPolygonStaticBase( const CqVector3D& vA, const CqVector3D& vB, const CqVector3D& vC, const CqVector3D& vD )
		{
			Initialise( vA, vB, vC, vD );
		}
		CqMicroPolygonStaticBase( const CqMicroPolygonStaticBase& From )
		{
			*this = From;
		}
		//	protected:
		// Private destructor, as destruction should be via Release()
		virtual	~CqMicroPolygonStaticBase()
		{}

	public:
		CqMicroPolygonStaticBase& operator=( const CqMicroPolygonStaticBase& From )
		{
			m_vecPoints[ 0 ] = From.m_vecPoints[ 0 ];
			m_vecPoints[ 1 ] = From.m_vecPoints[ 1 ];
			m_vecPoints[ 2 ] = From.m_vecPoints[ 2 ];
			m_vecPoints[ 3 ] = From.m_vecPoints[ 3 ];
			m_vecN = From.m_vecN;
			m_D = From.m_D;
			return ( *this );
		}
		CqBound	GetTotalBound() const;
		void	Initialise( const CqVector3D& vA, const CqVector3D& vB, const CqVector3D& vC, const CqVector3D& vD );
		TqBool	fContains( const CqVector2D& vecP, TqFloat& Depth );
		CqMicroPolygonStaticBase&	LinearInterpolate( TqFloat Fraction, const CqMicroPolygonStaticBase& MPA, const CqMicroPolygonStaticBase& MPB );
		CqVector2D ReverseBilinear( CqVector2D& v );

	protected:
		CqVector3D	m_vecPoints[ 4 ];		///< Array of 4 3D vectors representing the micropoly.
		CqVector3D	m_vecN;				///< The normal to the micropoly.
		TqFloat	m_D;				///< Distance of the plane from the origin, used for calculating sample depth.
}
;


//----------------------------------------------------------------------
/** \class CqMicroPolygonStatic
 * Class which stores a single static micropolygon.
 */

class CqMicroPolygonStatic : public CqMicroPolygonBase, public CqMicroPolygonStaticBase, public CqPoolable<CqMicroPolygonStatic>
{
	public:
		CqMicroPolygonStatic() : CqMicroPolygonBase(), CqMicroPolygonStaticBase(), m_fTrimmed( TqFalse )
		{}
		CqMicroPolygonStatic( const CqMicroPolygonStatic& From ) : CqMicroPolygonBase( From ), CqMicroPolygonStaticBase( From )
		{
			m_fTrimmed = From.m_fTrimmed;
		}

		virtual	~CqMicroPolygonStatic()
		{}

		// overrides from CqMicroPolygonBase


		virtual	CqBound&	GetTotalBound( TqBool fForce = TqFalse );
		/** Pure virtual, get the bound of the micropoly.
		 * \return CqBound representing the conservative bound.
		 */
		virtual const CqBound&	GetTotalBound() const
		{
			return ( m_Bound );
		}
		virtual	TqInt	cSubBounds()
		{
			return ( 1 );
		}
		virtual	CqBound&	SubBound( TqInt iIndex, TqFloat& time )
		{
			time = 0.0f;
			return ( m_Bound );
		}

		virtual	TqBool	Sample( CqVector2D& vecSample, TqFloat time, TqFloat& D );

		virtual void	MarkTrimmed()
		{
			m_fTrimmed = TqTrue;
		}

	private:
		CqBound	m_Bound;		///< Stored bound.
		TqBool	m_fTrimmed;		///< Flag indicating that the MPG spans a trim curve.
}
;


//----------------------------------------------------------------------
/** \class CqMicroPolygonMotion
 * Class which stores a single moving micropolygon.
 */

class CqMicroPolygonMotion : public CqMicroPolygonBase, public CqMotionSpec<CqMicroPolygonStaticBase>
{
	public:
		CqMicroPolygonMotion() : CqMicroPolygonBase(), CqMotionSpec<CqMicroPolygonStaticBase>( CqMicroPolygonStaticBase() ), m_BoundReady( TqFalse )
		{ }
		CqMicroPolygonMotion( const CqMicroPolygonMotion& From ) : CqMicroPolygonBase( From ), CqMotionSpec<CqMicroPolygonStaticBase>( From ), m_BoundReady( TqFalse )
		{
			*this = From;
		}
		//	private:
		virtual	~CqMicroPolygonMotion()
		{}

	public:
		CqMicroPolygonMotion& operator=( const CqMicroPolygonMotion& From )
		{
			CqMotionSpec<CqMicroPolygonStaticBase>::operator=( From );
			return ( *this );
		}

		void	ExpandBound( const CqMicroPolygonStaticBase& MP );
		void	Initialise( const CqVector3D& vA, const CqVector3D& vB, const CqVector3D& vC, const CqVector3D& vD, TqFloat time );

		// Overrides from CqMicroPolygonBase
		virtual	CqBound&	GetTotalBound( TqBool fForce = TqFalse );
		/** Pure virtual, get the bound of the micropoly.
		 * \return CqBound representing the conservative bound.
		 */
		virtual const CqBound&	GetTotalBound() const
		{
			return ( m_Bound );
		}
		virtual	TqInt	cSubBounds()
		{
			if ( !m_BoundReady )
				BuildBoundList();
			return ( m_BoundList.Size() );
		}
		virtual	CqBound&	SubBound( TqInt iIndex, TqFloat& time )
		{
			if ( !m_BoundReady )
				BuildBoundList();
			assert( iIndex < m_BoundList.Size() );
			time = m_BoundList.GetTime( iIndex );
			return ( *m_BoundList.GetBound( iIndex ) );
		}
		virtual void	BuildBoundList();

		virtual	TqBool	Sample( CqVector2D& vecSample, TqFloat time, TqFloat& D );

		// Overrides from CqMotionSpec
		virtual	void	ClearMotionObject( CqMicroPolygonStaticBase& A ) const
		{}
		/** Overridden from CqMotionSpec, does nothing.
		 */
		virtual	CqMicroPolygonStaticBase	ConcatMotionObjects( const CqMicroPolygonStaticBase& A, const CqMicroPolygonStaticBase& B ) const
		{
			return ( B );
		}
		/** Overridden from CqMotionSpec, get the position of the micropoly linearly interpolated between the two extremes.
		 * \param Fraction The fractional distance between the two micropolys 0-1.
		 * \param A The start position.
		 * \param B The end position.
		 * \return a new micropoly at the requested position.
		 */
		virtual	CqMicroPolygonStaticBase	LinearInterpolateMotionObjects( TqFloat Fraction, const CqMicroPolygonStaticBase& A, const CqMicroPolygonStaticBase& B ) const
		{
			CqMicroPolygonStaticBase MP;
			return ( MP.LinearInterpolate( Fraction, A, B ) );
		}
	private:
		CqBound	m_Bound;		///< Stored bound.
		CqBoundList	m_BoundList;	///< List of bounds to get a tighter fit.
		TqBool	m_BoundReady;	///< Flag indicating the boundary has been initialised.
}
;



//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !MICROPOLYGON_H_INCLUDED


