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
		\brief Declares the CqVector2D class which encapsulates a 2D vector.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef VECTOR2D_H_INCLUDED
#define VECTOR2D_H_INCLUDED 1

#include <aqsis/aqsis.h>

#include <iostream>

#include <aqsis/math/math.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** \brief Define class structure for 2D vector.
 */
class CqVector2D
{
	public:
		CqVector2D() : m_x(0.0f), m_y(0.0f)
		{}
		CqVector2D( TqFloat x, TqFloat y ) : m_x( x ), m_y( y )
		{}
		~CqVector2D()
		{}

		/** Get the x component.
		 */
		TqFloat	x() const
		{
			return ( m_x );
		}
		/// Get a reference to the x-componenet.
		TqFloat& x()
		{
			return m_x;
		}
		/** Set the x component.
		 * \param x Float new value.
		 * \deprecated.  Use the x() method returning a non-const reference.
		 */
		void	x( TqFloat x )
		{
			m_x = x;
		}
		/** Get the y component.
		 */
		TqFloat	y() const
		{
			return ( m_y );
		}
		/// Get a reference to the y-componenet.
		TqFloat& y()
		{
			return m_y;
		}
		/** Set the y component.
		 * \param y Float new value.
		 * \deprecated.  Use the y() method returning a non-const reference.
		 */
		void	y( TqFloat y )
		{
			m_y = y;
		}

		/** Access the components as an array.
		 * \param i Integer component index, 0 or 1.
		 * \return Appropriate component or y if invalid index.
		 */
		TqFloat&	operator[] ( TqInt i )
		{
			switch ( i )
			{
				case 0:
					return ( m_x );
					break;
				case 1:
					return ( m_y );
					break;
				default:
					break;
			}
			return ( m_y );
		}

		/** Access the components as an array.
		 * \param i Integer component index, 0 or 1.
		 * \return Appropriate component or y if invalid index.
		 */
		const TqFloat&	operator[] ( TqInt i ) const
		{
			switch ( i )
			{
				case 0:
					return ( m_x );
					break;
				case 1:
					return ( m_y );
					break;
				default:
					break;
			}
			return ( m_y );
		}

		/** Get the length squared.
		 */
		TqFloat	Magnitude2() const
		{
			if (m_y == 0.0) return (m_x * m_x);
			else if (m_x == 0.0) return (m_y * m_y);
			else
			return ( ( m_x * m_x ) + ( m_y * m_y ) );
		}
		/** Get the length.
		 */
		TqFloat	Magnitude() const
		{
			if (m_y == 0.0) return fabs(m_x);
			else if (m_x == 0.0) return fabs(m_y);
			else
			return ( sqrt( ( m_x * m_x ) + ( m_y * m_y ) ) );
		}
		void	Unit()
		{
			TqFloat Mag = Magnitude();

			m_x /= Mag;
			m_y /= Mag;
		}

		/** Addition assignment operator.
		 */
		CqVector2D& operator+=( const CqVector2D &From )
		{
			m_x += From.m_x;
			m_y += From.m_y;
			return ( *this );
		}
		/** Componentwise addition assignment operator.
		 */
		CqVector2D& operator+=( const TqFloat &f )
		{
			m_x += f;
			m_y += f;
			return ( *this );
		}
		/** Subtraction assignment operator.
		 */
		CqVector2D& operator-=( const CqVector2D &From )
		{
			m_x -= From.m_x;
			m_y -= From.m_y;
			return ( *this );
		}
		/** Componentwise subtraction assignment operator.
		 */
		CqVector2D& operator-=( const TqFloat &f )
		{
			m_x -= f;
			m_y -= f;
			return ( *this );
		}
		/** Coponent wise scale operator.
		 */
		CqVector2D& operator*=( const TqFloat Scale )
		{
			m_x *= Scale;
			m_y *= Scale;
			return ( *this );
		}
		/** Scale operator.
		 */
		CqVector2D& operator*=( const CqVector2D &Scale )
		{
			m_x *= Scale.m_x;
			m_y *= Scale.m_y;
			return ( *this );
		}
		/** Inverse scale operator.
		 */
		CqVector2D& operator/=( const CqVector2D &Scale )
		{
			m_x /= Scale.m_x;
			m_y /= Scale.m_y;
			return ( *this );
		}
		/** Component wise inverse scale operator.
		 */
		CqVector2D& operator/=( const TqFloat Scale )
		{
			m_x /= Scale;
			m_y /= Scale;
			return ( *this );
		}
		/** Equality operator.
		 */
		bool	operator==( const CqVector2D &Cmp ) const
		{
			return ( ( m_x == Cmp.m_x ) && ( m_y == Cmp.m_y ) );
		}
		/** Inequality operator.
		 */
		bool	operator!=( const CqVector2D &Cmp ) const
		{
			return ( ( m_x != Cmp.m_x ) || ( m_y != Cmp.m_y ) );
		}

		friend CqVector2D	operator+( const TqFloat f, const CqVector2D& v )
		{
			return CqVector2D( f + v.x(), f + v.y() );
		}
		friend CqVector2D	operator+( const CqVector2D& v, const TqFloat f )
		{
			CqVector2D r( v );
			return ( r += f );
		}
		friend CqVector2D	operator-( const TqFloat f, const CqVector2D& v )
		{
			return CqVector2D( f -v.x(), f - v.y() );
		}
		friend CqVector2D	operator-( const CqVector2D& v, const TqFloat f )
		{
			CqVector2D r( v );
			return ( r -= f );
		}
		friend CqVector2D	operator*( const TqFloat f, const CqVector2D& v )
		{
			return CqVector2D( f * v.x(), f * v.y() );
		}
		friend CqVector2D	operator*( const CqVector2D& v, const TqFloat f )
		{
			CqVector2D r( v );
			return ( r *= f );
		}
		friend CqVector2D	operator/( const TqFloat f, const CqVector2D& v )
		{
			return CqVector2D( f / v.x(), f / v.y() );
		}
		friend CqVector2D	operator/( const CqVector2D& v, const TqFloat f )
		{
			CqVector2D r( v );
			return ( r /= f );
		}

		friend CqVector2D	operator+( const CqVector2D& a, const CqVector2D& b )
		{
			CqVector2D r( a );
			return ( r += b );
		}
		friend CqVector2D	operator-( const CqVector2D& a, const CqVector2D& b )
		{
			CqVector2D r( a );
			return ( r -= b );
		}
		friend CqVector2D	operator/( const CqVector2D& a, const CqVector2D& b )
		{
			CqVector2D r( a );
			return ( r /= b );
		}
		friend CqVector2D	operator-( const CqVector2D& v )
		{
			return ( CqVector2D( -v.m_x, -v.m_y ) );
		} // Negation

		friend TqFloat	operator*( const CqVector2D& a, const CqVector2D& b )
		{
			return ( a.m_x * b.m_x + a.m_y * b.m_y );
		} // Dot product

	protected:
		TqFloat	m_x;		///< X component.
		TqFloat	m_y;		///< Y component.
};

//------------------------------------------------------------------------------
/** \brief Determine whether two vectors are equal to within some tolerance
 *
 * The closeness criterion for vectors is based on the euclidian norm - ie, the
 * usual distance function between two vectors.  v1 and v2 are "close" if
 *
 * length(v1 - v2) < tol*max(length(v1), length(v2));
 *
 * \param v1, v2 - vectors to compare
 * \param tolerance for comparison
 */
bool isClose(const CqVector2D& v1, const CqVector2D& v2,
		TqFloat tol = 10*std::numeric_limits<TqFloat>::epsilon());

/// Determine a componentwise minimum for two vectors
CqVector2D min(const CqVector2D& v1, const CqVector2D& v2);

/// Determine a componentwise maximum for two vectors
CqVector2D max(const CqVector2D& v1, const CqVector2D& v2);

/** \brief Componenetwise multiplication of two vectors.
 *
 * This may also be thought of as a transformation by the matrix
 * [ trans.x()  0         ]
 * [ 0          trans.y() ]
 *
 * Unfortunately we can't use operator*() for this, since it's already used for
 * the dot product.
 */
CqVector2D compMul(const CqVector2D& trans, const CqVector2D& v);

/** \brief Cross product of two 2D vectors.
 *
 * The 2D cross product yeilds a scalar rather than a vector, with otherwise
 * similar properties to the 3D version.  It can also be thought of as yeilding
 * the component of the 3D cross product pointing out of the 2D plane.
 */
inline TqFloat cross(const CqVector2D a, const CqVector2D b);

/** \brief Maximum of the absolute value of components of v.
 *
 * This is called the "max norm" or "infinity norm" and may be used to estimate
 * the "size" of v without a costly square root.
 */
inline TqFloat maxNorm(CqVector2D v);


/// Stream insertion
std::ostream &operator<<(std::ostream& out, const CqVector2D& v);


//==============================================================================
// Implementation details
//==============================================================================

inline bool isClose(const CqVector2D& v1, const CqVector2D& v2, TqFloat tol)
{
	TqFloat diff2 = (v1 - v2).Magnitude2();
	TqFloat tol2 = tol*tol;
	return diff2 <= tol2*v1.Magnitude2() || diff2 <= tol2*v2.Magnitude2();
}

inline CqVector2D min(const CqVector2D& v1, const CqVector2D& v2)
{
	return CqVector2D(min(v1.x(), v2.x()), min(v1.y(), v2.y()));
}

inline CqVector2D max(const CqVector2D& v1, const CqVector2D& v2)
{
	return CqVector2D(max(v1.x(), v2.x()), max(v1.y(), v2.y()));
}

inline CqVector2D compMul(const CqVector2D& trans, const CqVector2D& v)
{
	return CqVector2D(trans.x()*v.x(), trans.y()*v.y());
}

inline TqFloat cross(const CqVector2D a, const CqVector2D b)
{
	return a.x()*b.y() - a.y()*b.x();
}

inline TqFloat maxNorm(CqVector2D v)
{
	return max(std::fabs(v.x()), std::fabs(v.y()));
}

inline std::ostream &operator<<(std::ostream& out, const CqVector2D& v)
{
	out << v.x() << "," << v.y();
	return out;
}

} // namespace Aqsis

#endif	// !VECTOR2D_H_INCLUDED
