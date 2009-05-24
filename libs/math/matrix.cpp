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


/**
 * \file
 *
 * \brief Implements the CqMatrix 4D homogenous matrix class.
 * \author Paul C. Gregory (pgregory@aqsis.org)
 */

#include <aqsis/math/matrix.h>

#include <iomanip>

#include <aqsis/math/math.h>

namespace Aqsis
{

//---------------------------------------------------------------------

CqMatrix::CqMatrix( const TqFloat angle,
                    const TqFloat dx1, const TqFloat dy1, const TqFloat dz1,
                    const TqFloat dx2, const TqFloat dy2, const TqFloat dz2 )
{
	// For now, base this on what Larry Gritz posted a while back.
	// There are some more optimizations that can be done.

	// Normalize the two vectors, and construct a third perpendicular
	CqVector3D d1( dx1, dy1, dz1 ), d2( dx2, dy2, dz2 );
	d1.Unit();
	d2.Unit();

	// Assumes angle already changed to radians.

	TqFloat d1d2dot = d1 * d2;
	TqFloat axisangle = static_cast<TqFloat>(acos( d1d2dot ));
	if ( angle >= axisangle || angle <= ( axisangle - M_PI ) )
	{
		// Skewed past the axes -- issue error, then just use identity matrix.
		// No access to CqBasicError from here, so this will have to be down
		//   in RiSkew.  That would duplicate the above math calculations
		//   unless they were passed to this constructor which would be odd
		//   looking:
		// CqBasicError(0,Severity_Normal,"RiSkew angle invalid.");
		Identity();
	}
	else
	{
		CqVector3D right = d1 % d2;
		right.Unit();

		// d1ortho will be perpendicular to <d2> and <right> and can be
		// used to construct a rotation matrix
		CqVector3D d1ortho = d2 % right;


		// 1) Rotate to a space where the skew operation is in a major plane.
		// 2) Bend the y axis towards the z axis causing a skew.
		// 3) Rotate back.
		CqMatrix Rot( right[ 0 ], d1ortho[ 0 ], d2[ 0 ], 0,
		              right[ 1 ], d1ortho[ 1 ], d2[ 1 ], 0,
		              right[ 2 ], d1ortho[ 2 ], d2[ 2 ], 0,
		              0, 0, 0, 1 );
		TqFloat par = d1d2dot;               // Amount of d1 parallel to d2
		TqFloat perp = static_cast<TqFloat>(sqrt( 1.0 - par * par ));   // Amount perpendicular
		TqFloat s = static_cast<TqFloat>(tan( angle + acos( perp ) ) * perp - par);
		CqMatrix Skw( 1, 0, 0, 0,
		              0, 1, s, 0,
		              0, 0, 1, 0,
		              0, 0, 0, 1 );
		// Note the Inverse of a rotation matrix is its Transpose.
		*this = Rot.Transpose() * Skw * Rot;
	}
}

//------------------------------------------------------------------------------
// Rotatate by angle about axis.
void CqMatrix::Rotate( const TqFloat angle, const CqVector3D axis )
{
	if ( angle != 0.0f )
	{
		CqMatrix	R;
		R.Identity();
		CqVector3D	RotAxis = axis;
		R.m_fIdentity = false;

		RotAxis.Unit();

		TqFloat	s = static_cast<TqFloat>( sin( angle ) );
		TqFloat	c = static_cast<TqFloat>( cos( angle ) );
		TqFloat	t = 1.0f - c;

		R.m_elements[ 0 ][ 0 ] = t * RotAxis.x() * RotAxis.x() + c;
		R.m_elements[ 1 ][ 1 ] = t * RotAxis.y() * RotAxis.y() + c;
		R.m_elements[ 2 ][ 2 ] = t * RotAxis.z() * RotAxis.z() + c;

		TqFloat	txy = t * RotAxis.x() * RotAxis.y();
		TqFloat	sz = s * RotAxis.z();

		R.m_elements[ 0 ][ 1 ] = txy + sz;
		R.m_elements[ 1 ][ 0 ] = txy - sz;

		TqFloat	txz = t * RotAxis.x() * RotAxis.z();
		TqFloat	sy = s * RotAxis.y();

		R.m_elements[ 0 ][ 2 ] = txz - sy;
		R.m_elements[ 2 ][ 0 ] = txz + sy;

		TqFloat	tyz = t * RotAxis.y() * RotAxis.z();
		TqFloat	sx = s * RotAxis.x();

		R.m_elements[ 1 ][ 2 ] = tyz + sx;
		R.m_elements[ 2 ][ 1 ] = tyz - sx;

		this->PreMultiply( R );
	}
}

//---------------------------------------------------------------------
void CqMatrix::Normalise()
{
	assert(m_elements[ 3 ][ 3 ] != 0);
	for ( TqInt i = 0; i < 4; i++ )
	{
		for ( TqInt j = 0; j < 4; j++ )
		{
			m_elements[ i ][ j ] /= m_elements[ 3 ][ 3 ];
		}
	}
}

CqMatrix &CqMatrix::operator*=( const CqMatrix &from )
{
	if ( m_fIdentity )
		return ( *this = from );
	else if ( from.m_fIdentity )
		return ( *this );

	CqMatrix A( *this );

	m_elements[ 0 ][ 0 ] = from.m_elements[ 0 ][ 0 ] * A.m_elements[ 0 ][ 0 ]
	                        + from.m_elements[ 0 ][ 1 ] * A.m_elements[ 1 ][ 0 ]
	                        + from.m_elements[ 0 ][ 2 ] * A.m_elements[ 2 ][ 0 ]
	                        + from.m_elements[ 0 ][ 3 ] * A.m_elements[ 3 ][ 0 ];
	m_elements[ 0 ][ 1 ] = from.m_elements[ 0 ][ 0 ] * A.m_elements[ 0 ][ 1 ]
	                        + from.m_elements[ 0 ][ 1 ] * A.m_elements[ 1 ][ 1 ]
	                        + from.m_elements[ 0 ][ 2 ] * A.m_elements[ 2 ][ 1 ]
	                        + from.m_elements[ 0 ][ 3 ] * A.m_elements[ 3 ][ 1 ];
	m_elements[ 0 ][ 2 ] = from.m_elements[ 0 ][ 0 ] * A.m_elements[ 0 ][ 2 ]
	                        + from.m_elements[ 0 ][ 1 ] * A.m_elements[ 1 ][ 2 ]
	                        + from.m_elements[ 0 ][ 2 ] * A.m_elements[ 2 ][ 2 ]
	                        + from.m_elements[ 0 ][ 3 ] * A.m_elements[ 3 ][ 2 ];
	m_elements[ 0 ][ 3 ] = from.m_elements[ 0 ][ 0 ] * A.m_elements[ 0 ][ 3 ]
	                        + from.m_elements[ 0 ][ 1 ] * A.m_elements[ 1 ][ 3 ]
	                        + from.m_elements[ 0 ][ 2 ] * A.m_elements[ 2 ][ 3 ]
	                        + from.m_elements[ 0 ][ 3 ] * A.m_elements[ 3 ][ 3 ];

	m_elements[ 1 ][ 0 ] = from.m_elements[ 1 ][ 0 ] * A.m_elements[ 0 ][ 0 ]
	                        + from.m_elements[ 1 ][ 1 ] * A.m_elements[ 1 ][ 0 ]
	                        + from.m_elements[ 1 ][ 2 ] * A.m_elements[ 2 ][ 0 ]
	                        + from.m_elements[ 1 ][ 3 ] * A.m_elements[ 3 ][ 0 ];
	m_elements[ 1 ][ 1 ] = from.m_elements[ 1 ][ 0 ] * A.m_elements[ 0 ][ 1 ]
	                        + from.m_elements[ 1 ][ 1 ] * A.m_elements[ 1 ][ 1 ]
	                        + from.m_elements[ 1 ][ 2 ] * A.m_elements[ 2 ][ 1 ]
	                        + from.m_elements[ 1 ][ 3 ] * A.m_elements[ 3 ][ 1 ];
	m_elements[ 1 ][ 2 ] = from.m_elements[ 1 ][ 0 ] * A.m_elements[ 0 ][ 2 ]
	                        + from.m_elements[ 1 ][ 1 ] * A.m_elements[ 1 ][ 2 ]
	                        + from.m_elements[ 1 ][ 2 ] * A.m_elements[ 2 ][ 2 ]
	                        + from.m_elements[ 1 ][ 3 ] * A.m_elements[ 3 ][ 2 ];
	m_elements[ 1 ][ 3 ] = from.m_elements[ 1 ][ 0 ] * A.m_elements[ 0 ][ 3 ]
	                        + from.m_elements[ 1 ][ 1 ] * A.m_elements[ 1 ][ 3 ]
	                        + from.m_elements[ 1 ][ 2 ] * A.m_elements[ 2 ][ 3 ]
	                        + from.m_elements[ 1 ][ 3 ] * A.m_elements[ 3 ][ 3 ];

	m_elements[ 2 ][ 0 ] = from.m_elements[ 2 ][ 0 ] * A.m_elements[ 0 ][ 0 ]
	                        + from.m_elements[ 2 ][ 1 ] * A.m_elements[ 1 ][ 0 ]
	                        + from.m_elements[ 2 ][ 2 ] * A.m_elements[ 2 ][ 0 ]
	                        + from.m_elements[ 2 ][ 3 ] * A.m_elements[ 3 ][ 0 ];
	m_elements[ 2 ][ 1 ] = from.m_elements[ 2 ][ 0 ] * A.m_elements[ 0 ][ 1 ]
	                        + from.m_elements[ 2 ][ 1 ] * A.m_elements[ 1 ][ 1 ]
	                        + from.m_elements[ 2 ][ 2 ] * A.m_elements[ 2 ][ 1 ]
	                        + from.m_elements[ 2 ][ 3 ] * A.m_elements[ 3 ][ 1 ];
	m_elements[ 2 ][ 2 ] = from.m_elements[ 2 ][ 0 ] * A.m_elements[ 0 ][ 2 ]
	                        + from.m_elements[ 2 ][ 1 ] * A.m_elements[ 1 ][ 2 ]
	                        + from.m_elements[ 2 ][ 2 ] * A.m_elements[ 2 ][ 2 ]
	                        + from.m_elements[ 2 ][ 3 ] * A.m_elements[ 3 ][ 2 ];
	m_elements[ 2 ][ 3 ] = from.m_elements[ 2 ][ 0 ] * A.m_elements[ 0 ][ 3 ]
	                        + from.m_elements[ 2 ][ 1 ] * A.m_elements[ 1 ][ 3 ]
	                        + from.m_elements[ 2 ][ 2 ] * A.m_elements[ 2 ][ 3 ]
	                        + from.m_elements[ 2 ][ 3 ] * A.m_elements[ 3 ][ 3 ];

	m_elements[ 3 ][ 0 ] = from.m_elements[ 3 ][ 0 ] * A.m_elements[ 0 ][ 0 ]
	                        + from.m_elements[ 3 ][ 1 ] * A.m_elements[ 1 ][ 0 ]
	                        + from.m_elements[ 3 ][ 2 ] * A.m_elements[ 2 ][ 0 ]
	                        + from.m_elements[ 3 ][ 3 ] * A.m_elements[ 3 ][ 0 ];
	m_elements[ 3 ][ 1 ] = from.m_elements[ 3 ][ 0 ] * A.m_elements[ 0 ][ 1 ]
	                        + from.m_elements[ 3 ][ 1 ] * A.m_elements[ 1 ][ 1 ]
	                        + from.m_elements[ 3 ][ 2 ] * A.m_elements[ 2 ][ 1 ]
	                        + from.m_elements[ 3 ][ 3 ] * A.m_elements[ 3 ][ 1 ];
	m_elements[ 3 ][ 2 ] = from.m_elements[ 3 ][ 0 ] * A.m_elements[ 0 ][ 2 ]
	                        + from.m_elements[ 3 ][ 1 ] * A.m_elements[ 1 ][ 2 ]
	                        + from.m_elements[ 3 ][ 2 ] * A.m_elements[ 2 ][ 2 ]
	                        + from.m_elements[ 3 ][ 3 ] * A.m_elements[ 3 ][ 2 ];
	m_elements[ 3 ][ 3 ] = from.m_elements[ 3 ][ 0 ] * A.m_elements[ 0 ][ 3 ]
	                        + from.m_elements[ 3 ][ 1 ] * A.m_elements[ 1 ][ 3 ]
	                        + from.m_elements[ 3 ][ 2 ] * A.m_elements[ 2 ][ 3 ]
	                        + from.m_elements[ 3 ][ 3 ] * A.m_elements[ 3 ][ 3 ];

	m_fIdentity = false;
	return ( *this );
}

//---------------------------------------------------------------------
CqMatrix& CqMatrix::PreMultiply( const CqMatrix &from )
{
	if ( m_fIdentity )
		return ( *this = from );
	else if ( from.m_fIdentity )
		return ( *this );

	CqMatrix A( *this );

	m_elements[ 0 ][ 0 ] = A.m_elements[ 0 ][ 0 ] * from.m_elements[ 0 ][ 0 ]
	                        + A.m_elements[ 0 ][ 1 ] * from.m_elements[ 1 ][ 0 ]
	                        + A.m_elements[ 0 ][ 2 ] * from.m_elements[ 2 ][ 0 ]
	                        + A.m_elements[ 0 ][ 3 ] * from.m_elements[ 3 ][ 0 ];
	m_elements[ 0 ][ 1 ] = A.m_elements[ 0 ][ 0 ] * from.m_elements[ 0 ][ 1 ]
	                        + A.m_elements[ 0 ][ 1 ] * from.m_elements[ 1 ][ 1 ]
	                        + A.m_elements[ 0 ][ 2 ] * from.m_elements[ 2 ][ 1 ]
	                        + A.m_elements[ 0 ][ 3 ] * from.m_elements[ 3 ][ 1 ];
	m_elements[ 0 ][ 2 ] = A.m_elements[ 0 ][ 0 ] * from.m_elements[ 0 ][ 2 ]
	                        + A.m_elements[ 0 ][ 1 ] * from.m_elements[ 1 ][ 2 ]
	                        + A.m_elements[ 0 ][ 2 ] * from.m_elements[ 2 ][ 2 ]
	                        + A.m_elements[ 0 ][ 3 ] * from.m_elements[ 3 ][ 2 ];
	m_elements[ 0 ][ 3 ] = A.m_elements[ 0 ][ 0 ] * from.m_elements[ 0 ][ 3 ]
	                        + A.m_elements[ 0 ][ 1 ] * from.m_elements[ 1 ][ 3 ]
	                        + A.m_elements[ 0 ][ 2 ] * from.m_elements[ 2 ][ 3 ]
	                        + A.m_elements[ 0 ][ 3 ] * from.m_elements[ 3 ][ 3 ];

	m_elements[ 1 ][ 0 ] = A.m_elements[ 1 ][ 0 ] * from.m_elements[ 0 ][ 0 ]
	                        + A.m_elements[ 1 ][ 1 ] * from.m_elements[ 1 ][ 0 ]
	                        + A.m_elements[ 1 ][ 2 ] * from.m_elements[ 2 ][ 0 ]
	                        + A.m_elements[ 1 ][ 3 ] * from.m_elements[ 3 ][ 0 ];
	m_elements[ 1 ][ 1 ] = A.m_elements[ 1 ][ 0 ] * from.m_elements[ 0 ][ 1 ]
	                        + A.m_elements[ 1 ][ 1 ] * from.m_elements[ 1 ][ 1 ]
	                        + A.m_elements[ 1 ][ 2 ] * from.m_elements[ 2 ][ 1 ]
	                        + A.m_elements[ 1 ][ 3 ] * from.m_elements[ 3 ][ 1 ];
	m_elements[ 1 ][ 2 ] = A.m_elements[ 1 ][ 0 ] * from.m_elements[ 0 ][ 2 ]
	                        + A.m_elements[ 1 ][ 1 ] * from.m_elements[ 1 ][ 2 ]
	                        + A.m_elements[ 1 ][ 2 ] * from.m_elements[ 2 ][ 2 ]
	                        + A.m_elements[ 1 ][ 3 ] * from.m_elements[ 3 ][ 2 ];
	m_elements[ 1 ][ 3 ] = A.m_elements[ 1 ][ 0 ] * from.m_elements[ 0 ][ 3 ]
	                        + A.m_elements[ 1 ][ 1 ] * from.m_elements[ 1 ][ 3 ]
	                        + A.m_elements[ 1 ][ 2 ] * from.m_elements[ 2 ][ 3 ]
	                        + A.m_elements[ 1 ][ 3 ] * from.m_elements[ 3 ][ 3 ];

	m_elements[ 2 ][ 0 ] = A.m_elements[ 2 ][ 0 ] * from.m_elements[ 0 ][ 0 ]
	                        + A.m_elements[ 2 ][ 1 ] * from.m_elements[ 1 ][ 0 ]
	                        + A.m_elements[ 2 ][ 2 ] * from.m_elements[ 2 ][ 0 ]
	                        + A.m_elements[ 2 ][ 3 ] * from.m_elements[ 3 ][ 0 ];
	m_elements[ 2 ][ 1 ] = A.m_elements[ 2 ][ 0 ] * from.m_elements[ 0 ][ 1 ]
	                        + A.m_elements[ 2 ][ 1 ] * from.m_elements[ 1 ][ 1 ]
	                        + A.m_elements[ 2 ][ 2 ] * from.m_elements[ 2 ][ 1 ]
	                        + A.m_elements[ 2 ][ 3 ] * from.m_elements[ 3 ][ 1 ];
	m_elements[ 2 ][ 2 ] = A.m_elements[ 2 ][ 0 ] * from.m_elements[ 0 ][ 2 ]
	                        + A.m_elements[ 2 ][ 1 ] * from.m_elements[ 1 ][ 2 ]
	                        + A.m_elements[ 2 ][ 2 ] * from.m_elements[ 2 ][ 2 ]
	                        + A.m_elements[ 2 ][ 3 ] * from.m_elements[ 3 ][ 2 ];
	m_elements[ 2 ][ 3 ] = A.m_elements[ 2 ][ 0 ] * from.m_elements[ 0 ][ 3 ]
	                        + A.m_elements[ 2 ][ 1 ] * from.m_elements[ 1 ][ 3 ]
	                        + A.m_elements[ 2 ][ 2 ] * from.m_elements[ 2 ][ 3 ]
	                        + A.m_elements[ 2 ][ 3 ] * from.m_elements[ 3 ][ 3 ];

	m_elements[ 3 ][ 0 ] = A.m_elements[ 3 ][ 0 ] * from.m_elements[ 0 ][ 0 ]
	                        + A.m_elements[ 3 ][ 1 ] * from.m_elements[ 1 ][ 0 ]
	                        + A.m_elements[ 3 ][ 2 ] * from.m_elements[ 2 ][ 0 ]
	                        + A.m_elements[ 3 ][ 3 ] * from.m_elements[ 3 ][ 0 ];
	m_elements[ 3 ][ 1 ] = A.m_elements[ 3 ][ 0 ] * from.m_elements[ 0 ][ 1 ]
	                        + A.m_elements[ 3 ][ 1 ] * from.m_elements[ 1 ][ 1 ]
	                        + A.m_elements[ 3 ][ 2 ] * from.m_elements[ 2 ][ 1 ]
	                        + A.m_elements[ 3 ][ 3 ] * from.m_elements[ 3 ][ 1 ];
	m_elements[ 3 ][ 2 ] = A.m_elements[ 3 ][ 0 ] * from.m_elements[ 0 ][ 2 ]
	                        + A.m_elements[ 3 ][ 1 ] * from.m_elements[ 1 ][ 2 ]
	                        + A.m_elements[ 3 ][ 2 ] * from.m_elements[ 2 ][ 2 ]
	                        + A.m_elements[ 3 ][ 3 ] * from.m_elements[ 3 ][ 2 ];
	m_elements[ 3 ][ 3 ] = A.m_elements[ 3 ][ 0 ] * from.m_elements[ 0 ][ 3 ]
	                        + A.m_elements[ 3 ][ 1 ] * from.m_elements[ 1 ][ 3 ]
	                        + A.m_elements[ 3 ][ 2 ] * from.m_elements[ 2 ][ 3 ]
	                        + A.m_elements[ 3 ][ 3 ] * from.m_elements[ 3 ][ 3 ];

	m_fIdentity = false;
	return ( *this );
}


//---------------------------------------------------------------------
CqMatrix CqMatrix::Inverse() const
{
	CqMatrix b;		// b evolves from identity into inverse(a)
	CqMatrix a( *this );	// a evolves from original matrix into identity

	if ( m_fIdentity )
	{
		b = *this;
	}
	else
	{
		b.Identity();
		b.m_fIdentity = false;

		TqInt i;
		TqInt j;
		TqInt i1;

		// Loop over cols of a from left to right, eliminating above and below diag
		for ( j = 0; j < 4; j++ )    	// Find largest pivot in column j among rows j..3
		{
			i1 = j;
			for ( i = j + 1; i < 4; i++ )
			{
				if ( fabs( a.m_elements[ i ][ j ] ) > fabs( a.m_elements[ i1 ][ j ] ) )
				{
					i1 = i;
				}
			}

			if ( i1 != j )
			{
				// Swap rows i1 and j in a and b to put pivot on diagonal
				TqFloat temp;

				temp = a.m_elements[ i1 ][ 0 ];
				a.m_elements[ i1 ][ 0 ] = a.m_elements[ j ][ 0 ];
				a.m_elements[ j ][ 0 ] = temp;
				temp = a.m_elements[ i1 ][ 1 ];
				a.m_elements[ i1 ][ 1 ] = a.m_elements[ j ][ 1 ];
				a.m_elements[ j ][ 1 ] = temp;
				temp = a.m_elements[ i1 ][ 2 ];
				a.m_elements[ i1 ][ 2 ] = a.m_elements[ j ][ 2 ];
				a.m_elements[ j ][ 2 ] = temp;
				temp = a.m_elements[ i1 ][ 3 ];
				a.m_elements[ i1 ][ 3 ] = a.m_elements[ j ][ 3 ];
				a.m_elements[ j ][ 3 ] = temp;

				temp = b.m_elements[ i1 ][ 0 ];
				b.m_elements[ i1 ][ 0 ] = b.m_elements[ j ][ 0 ];
				b.m_elements[ j ][ 0 ] = temp;
				temp = b.m_elements[ i1 ][ 1 ];
				b.m_elements[ i1 ][ 1 ] = b.m_elements[ j ][ 1 ];
				b.m_elements[ j ][ 1 ] = temp;
				temp = b.m_elements[ i1 ][ 2 ];
				b.m_elements[ i1 ][ 2 ] = b.m_elements[ j ][ 2 ];
				b.m_elements[ j ][ 2 ] = temp;
				temp = b.m_elements[ i1 ][ 3 ];
				b.m_elements[ i1 ][ 3 ] = b.m_elements[ j ][ 3 ];
				b.m_elements[ j ][ 3 ] = temp;
			}

			// Scale row j to have a unit diagonal
			if ( a.m_elements[ j ][ j ] == 0.0f )
			{
				// Can't invert a singular matrix!
				// AQSIS_THROW_XQERROR(XqInternal, EqE_Math,
				//		"Can't invert a singular matrix!");
			}
			TqFloat scale = 1.0f / a.m_elements[ j ][ j ];
			b.m_elements[ j ][ 0 ] *= scale;
			b.m_elements[ j ][ 1 ] *= scale;
			b.m_elements[ j ][ 2 ] *= scale;
			b.m_elements[ j ][ 3 ] *= scale;
			// all elements to left of a[j][j] are already zero
			for ( i1 = j + 1; i1 < 4; i1++ )
			{
				a.m_elements[ j ][ i1 ] *= scale;
			}
			a.m_elements[ j ][ j ] = 1.0f;

			// Eliminate off-diagonal elements in column j of a, doing identical ops to b
			for ( i = 0; i < 4; i++ )
			{
				if ( i != j )
				{
					scale = a.m_elements[ i ][ j ];
					b.m_elements[ i ][ 0 ] -= scale * b.m_elements[ j ][ 0 ];
					b.m_elements[ i ][ 1 ] -= scale * b.m_elements[ j ][ 1 ];
					b.m_elements[ i ][ 2 ] -= scale * b.m_elements[ j ][ 2 ];
					b.m_elements[ i ][ 3 ] -= scale * b.m_elements[ j ][ 3 ];

					// all elements to left of a[j][j] are zero
					// a[j][j] is 1.0
					for ( i1 = j + 1; i1 < 4; i1++ )
					{
						a.m_elements[ i ][ i1 ] -= scale * a.m_elements[ j ][ i1 ];
					}
					a.m_elements[ i ][ j ] = 0.0f;
				}
			}
		}
	}

	return b;
}

//---------------------------------------------------------------------
// Utility functions for CqMatrix::GetDeterminant

namespace {

// Calculate the determinant of a 2x2 matrix
inline TqFloat det2x2( TqFloat a, TqFloat b, TqFloat c, TqFloat d )
{
	return a * d - b * c;
}

// Calculate the determinant of a 3x3 matrix
inline TqFloat det3x3( TqFloat a1, TqFloat a2, TqFloat a3,
                       TqFloat b1, TqFloat b2, TqFloat b3,
                       TqFloat c1, TqFloat c2, TqFloat c3 )
{
	return a1 * det2x2( b2, b3, c2, c3 ) -
	       b1 * det2x2( a2, a3, c2, c3 ) +
	       c1 * det2x2( a2, a3, b2, b3 );
}

} // unnamed namespace

// get the determinant.
TqFloat CqMatrix::Determinant() const
{
	// Assign to individual variable names to aid selecting correct elements
	TqFloat a1 = m_elements[ 0 ][ 0 ];
	TqFloat b1 = m_elements[ 0 ][ 1 ];
	TqFloat c1 = m_elements[ 0 ][ 2 ];
	TqFloat d1 = m_elements[ 0 ][ 3 ];

	TqFloat a2 = m_elements[ 1 ][ 0 ];
	TqFloat b2 = m_elements[ 1 ][ 1 ];
	TqFloat c2 = m_elements[ 1 ][ 2 ];
	TqFloat d2 = m_elements[ 1 ][ 3 ];

	TqFloat a3 = m_elements[ 2 ][ 0 ];
	TqFloat b3 = m_elements[ 2 ][ 1 ];
	TqFloat c3 = m_elements[ 2 ][ 2 ];
	TqFloat d3 = m_elements[ 2 ][ 3 ];

	TqFloat a4 = m_elements[ 3 ][ 0 ];
	TqFloat b4 = m_elements[ 3 ][ 1 ];
	TqFloat c4 = m_elements[ 3 ][ 2 ];
	TqFloat d4 = m_elements[ 3 ][ 3 ];

	return a1 * det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4 ) -
	       b1 * det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4 ) +
	       c1 * det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4 ) -
	       d1 * det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4 );
}

//----------------------------------------------------------------------
std::ostream &operator<<( std::ostream &outStream, const CqMatrix &matrix )
{
	if ( !matrix.fIdentity() )
	{
		for(TqInt i = 0; i < 4; ++i)
		{
			outStream << "[";
			for(TqInt j = 0; j < 3; ++j)
				outStream << matrix.m_elements[i][j] << ",";
			outStream << matrix.m_elements[i][3] << "]\n";
		}
	}
	else
	{
		outStream <<
		"[" << 1.0f << "," << 0.0f << "," << 0.0f << "," << 0.0f << "]\n" <<
		"[" << 0.0f << "," << 1.0f << "," << 0.0f << "," << 0.0f << "]\n" <<
		"[" << 0.0f << "," << 0.0f << "," << 1.0f << "," << 0.0f << "]\n" <<
		"[" << 0.0f << "," << 0.0f << "," << 0.0f << "," << 1.0f << "]\n";
	}

	return outStream;
}


//---------------------------------------------------------------------
bool isClose(const CqMatrix& m1, const CqMatrix& m2, TqFloat tol)
{
	// if the matrices are at the same address, or both the identity, then
	// they're equal.
	if(&m1 == &m2 || (m1.fIdentity() && m2.fIdentity()))
		return true;
	// Check whether one matrix (but not the other) is marked as the identity.
	// If so, create an identity matrix which is not marked as the identity to
	// compare it with (in principle, the identity flag and the matrix elements
	// may be out of sync I guess)
	if(m1.fIdentity())
	{
		CqMatrix ident;
		ident.SetfIdentity(false);
		return isClose(m2, ident);
	}
	else if(m2.fIdentity())
	{
		CqMatrix ident;
		ident.SetfIdentity(false);
		return isClose(m1, ident);
	}
	TqFloat norm1 = 0;
	TqFloat norm2 = 0;
	TqFloat diffNorm = 0;
	const TqFloat* m1Elts = m1.pElements();
	const TqFloat* m2Elts = m2.pElements();
	for(int i = 0; i < 16; ++i)
	{
		norm1 += m1Elts[i]*m1Elts[i];
		norm2 += m2Elts[i]*m2Elts[i];
		diffNorm += (m1Elts[i] - m2Elts[i])*(m1Elts[i] - m2Elts[i]);
	}
	TqFloat tol2 = tol*tol;
	return diffNorm <= tol2*norm1 || diffNorm <= tol2*norm2;
}

} //namespace Aqsis
