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
				CqPointsKDTreeDataComparator(CqPoints* pPoints, TqInt dimension) : m_pPointsSurface( pPoints ), m_Dim( dimension )
				{}
			
				bool operator()(TqInt a, TqInt b);

			private:
				CqPoints*	m_pPointsSurface;
				TqInt		m_Dim;
		};

	public:
				CqPointsKDTreeData( CqPoints* pPoints = NULL ) : m_pPointsSurface( pPoints )
				{}

		virtual void SortElements(std::vector<TqInt>& aLeaves, TqInt dimension)
				{
					std::sort(aLeaves.begin(), aLeaves.end(), CqPointsKDTreeDataComparator(m_pPointsSurface, dimension) );
				}
		virtual TqInt Dimensions() const	{return(3);}

				void	SetpPoints( CqPoints* pPoints );

	private:
		CqPoints*	m_pPointsSurface;
};


//----------------------------------------------------------------------
/** \class CqPoints
 * Class encapsulating the functionality of Points geometry.
 */

class CqPoints : public CqSurface, public CqMotionSpec<CqPolygonPoints*>
{
	public:

		CqPoints( TqInt nVertices, CqPolygonPoints* pPoints = NULL );
		CqPoints( const CqPoints& From ) : m_KDTree( &m_KDTreeData ),
					CqMotionSpec<CqPolygonPoints*>(From.pPoints())
		{
			*this = From;
		}
		virtual	~CqPoints()
		{
			// Release the reference to our points.
			TqInt i;
			for ( i = 0; i < cTimes(); i++ )
				GetMotionObject( Time( i ) ) ->Release();
		}

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

		virtual	CqBound	Bound() const;
		virtual	TqInt	Split( std::vector<CqBasicSurface*>& aSplits );
		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 )
		{}

		CqPoints&	operator=( const CqPoints& From );

		TqUint	nVertices() const
		{
			return ( m_nVertices );
		}

		CqPolygonPoints* pPoints( TqInt TimeIndex = 0 )
		{
			return( GetMotionObject( Time( TimeIndex ) ) );
		}
		CqPolygonPoints* pPoints( TqInt TimeIndex = 0) const
		{
			return( GetMotionObject( Time( TimeIndex ) ) );
		}

		const std::vector<CqParameter*>& aUserParams() const
		{
			return ( pPoints()->aUserParams() );
		}

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
		CqKDTree<TqInt>&	KDTree()			{return( m_KDTree); }
		const CqKDTree<TqInt>&	KDTree() const	{return( m_KDTree); }

		/// Initialise the KDTree to point to the whole points list.
		void	InitialiseKDTree();

		virtual void	NaturalDice( CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData );

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

	protected:
		template <class T, class SLT>
		void	TypedNaturalDice( CqParameterTyped<T, SLT>* pParam, IqShaderData* pData )
		{
			TqInt i;
			for ( i = 0; i < nVertices(); i++ )
				pData->SetValue( static_cast<SLT>( pParam->pValue() [ m_KDTree.aLeaves()[ i ] ] ), i );
		}

	private:
		CqPolygonPoints* m_pPoints;				///< Pointer to the surface storing the primtive variables.
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
			CqMicroPolyGridPoints() : CqMicroPolyGrid()	{}
			CqMicroPolyGridPoints( TqInt cu, TqInt cv, CqSurface* pSurface ) : CqMicroPolyGrid( cu, cv, pSurface )	{}
			virtual	~CqMicroPolyGridPoints()	{}
	
	virtual	void	Split( CqImageBuffer* pImage, TqInt iBucket, long xmin, long xmax, long ymin, long ymax );
};

class CqMotionMicroPolyGridPoints : public CqMotionMicroPolyGrid
{
	public:
			CqMotionMicroPolyGridPoints() : CqMotionMicroPolyGrid()	{}
			virtual	~CqMotionMicroPolyGridPoints()	{}
	
	virtual	void	Split( CqImageBuffer* pImage, TqInt iBucket, long xmin, long xmax, long ymin, long ymax );
};

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !POINTS_H_INCLUDED
