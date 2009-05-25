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
		\brief Declares the CqCubicSpline class for generic spline functionality.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef SPLINE_H_INCLUDED
#define SPLINE_H_INCLUDED 1

#include <aqsis/aqsis.h>

#include <vector>

#include <aqsis/util/sstring.h>
#include <aqsis/math/matrix.h>

namespace Aqsis {

enum EqSplineBasis
{
	SplineBasis_Bezier,
	SplineBasis_BSpline,
	SplineBasis_CatmullRom,
	SplineBasis_Hermite,
	SplineBasis_Power,	
	SplineBasis_Linear,
	SplineBasis_LAST,	//Marks the last element
};


//----------------------------------------------------------------------
/** \class CqCubicSpline
 * Cubic spline curve
 */
template <class T>
class CqCubicSpline
{
	public:
		CqCubicSpline( EqSplineBasis basis = SplineBasis_CatmullRom, TqUint reservePoints = 0 );
		CqCubicSpline( const CqString& strBasis, TqUint reservePoints = 0 );
		inline virtual ~CqCubicSpline();

		typedef typename std::vector<T>::iterator iterator;

		/** Evaluate curve at a given time.
		 * \param t time value.
		 */
		virtual T evaluate( TqFloat t ) const;

		/** Set the cubic spline basis matrix.
		 * \param basis Basis name.
		 */
		void setBasis( const EqSplineBasis basis );
				
		/** Add new a control point.
		 * \param newPoint Control point.
		 */
		void pushBack( const T newPoint );
		
		/** Removes all control points.
		 */
		inline void clear();

		/** Returns the number of control points.
		 */		
		inline TqInt numControlPoints();
		
		inline iterator	begin();
		inline iterator	end();

		/** Indexed access to the control points.
		 * \param i Integer index.
		 */
		inline const T& operator[] ( TqInt i ) const;

	private:
		CqMatrix m_basis;					///< Basis matrix.
		TqInt m_step;						///< Evaluation window step size.
		std::vector<T> m_controlPoints;		///< Array of control points.
		
		/** Returns the number of sections.
		 */
		virtual TqInt numSections() const;
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Inline/Template function implementations

struct SqSplineBasis
{
	const TqChar* name;
	TqInt step;
	TqFloat basis[4][4];
}; 

typedef SqSplineBasis TqSplineTypes[6];

extern AQSIS_MATH_SHARE TqSplineTypes splineTypes;

template <class T>
inline CqCubicSpline<T>::~CqCubicSpline()
{}


//---------------------------------------------------------------------
/** Default constructor for a cubic spline curve, defaults to Catmull-Rom basis matrix.
 */
template <class T>
CqCubicSpline<T>::CqCubicSpline( EqSplineBasis basis, TqUint reservePoints )
{
	m_basis = *splineTypes[ basis ].basis;
	m_step = splineTypes[ basis ].step;

	m_controlPoints.reserve( reservePoints );
}


//---------------------------------------------------------------------
/** Default constructor for a cubic spline curve, defaults to bezier basis matrix.
 */
template <class T>
CqCubicSpline<T>::CqCubicSpline( const CqString& strBasis, TqUint reservePoints )
{
	TqUint basis = SplineBasis_CatmullRom; 
	
	for ( TqInt i=0; i<SplineBasis_LAST; ++i)
	{
		if ( splineTypes[ i ].name == strBasis )
		{
			basis = i;
		}
	}

	m_basis = *splineTypes[ basis ].basis;
	m_step = splineTypes[ basis ].step;

	m_controlPoints.reserve( reservePoints );
}


//---------------------------------------------------------------------
/** Evaluate a cubic spline curve at the specified time.
 */
template <class T>
T CqCubicSpline<T>::evaluate( TqFloat t ) const
{
	TqFloat u = static_cast<TqFloat>( numSections() ) * t;
	TqInt section = static_cast<TqInt>( u );
	t = u - section;
	TqInt iv = section * m_step;

	T v0 = m_controlPoints[ 0 + iv ];
	T v1 = m_controlPoints[ 1 + iv ];
	T v2 = m_controlPoints[ 2 + iv ];
	T v3 = m_controlPoints[ 3 + iv ];

	T G0 = m_basis[0][0] * v0 + m_basis[0][1] * v1 + m_basis[0][2] * v2 + m_basis[0][3] * v3;
	T G1 = m_basis[1][0] * v0 + m_basis[1][1] * v1 + m_basis[1][2] * v2 + m_basis[1][3] * v3;
	T G2 = m_basis[2][0] * v0 + m_basis[2][1] * v1 + m_basis[2][2] * v2 + m_basis[2][3] * v3;
	T G3 = m_basis[3][0] * v0 + m_basis[3][1] * v1 + m_basis[3][2] * v2 + m_basis[3][3] * v3;
	
	TqFloat t2 = t * t;
	TqFloat t3 = t2 * t;

	T result = t3 * G0 + t2 * G1 + t * G2 + G3;
	
	return result;
}


//---------------------------------------------------------------------
/** Add new control point to the curve
 */
template <class T>
void CqCubicSpline<T>::pushBack( const T newPoint )
{
	m_controlPoints.push_back( newPoint );
}


//---------------------------------------------------------------------
/** Return the number of curve sections in the spline curve
 */
template <class T>
TqInt CqCubicSpline<T>::numSections() const
{
	return ( ( ( m_controlPoints.size() - 4 ) / m_step ) + 1 );
}


//---------------------------------------------------------------------
/* Set the cubic spline basis matrix.
 */
template <class T>
void CqCubicSpline<T>::setBasis( const EqSplineBasis basis )
{	
	m_step = splineTypes[ basis ].step;
	m_basis = *splineTypes[ basis ].basis;
}

template <class T>
inline const T& CqCubicSpline<T>::operator[] ( TqInt i ) const
{
	assert( i < static_cast<TqInt>(m_controlPoints.size()) );
	return ( m_controlPoints[ i ] );
}

template <class T>
inline void CqCubicSpline<T>::clear()
{  
    m_controlPoints.clear();
} 

template <class T>
inline typename CqCubicSpline<T>::iterator CqCubicSpline<T>::begin()
{  
    return m_controlPoints.begin();
} 

template <class T>
inline typename CqCubicSpline<T>::iterator	CqCubicSpline<T>::end()
{  
    return m_controlPoints.end();
} 

template <class T>
inline TqInt CqCubicSpline<T>::numControlPoints()
{  
    return m_controlPoints.size();
} 


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !SPLINE_H_INCLUDED
