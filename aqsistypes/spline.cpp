// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Implements the CqSplineCubic class for generic spline functionality.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"aqsis.h"
#include	"spline.h"

START_NAMESPACE( Aqsis )

__Basis	gBezierBasis	= {{ -1.0f,       3.0f,      -3.0f,       1.0f},
                        {  3.0f,      -6.0f,       3.0f,       0.0f},
                        { -3.0f,       3.0f,       0.0f,       0.0f},
                        {  1.0f,       0.0f,       0.0f,       0.0f}};
__Basis	gBSplineBasis	= {{ -1.0f/6.0f,  0.5f,      -0.5f,       1.0f/6.0f},
                         {  0.5f,      -1.0f,       0.5f,       0.0f},
                         { -0.5f,       0.0f,	      0.5f,       0.0f},
                         {  1.0f/6.0f,  2.0f/3.0f,  1.0f/6.0f,  0.0f}};
__Basis	gCatmullRomBasis	= {{ -0.5f,       1.5f,      -1.5f,       0.5f},
                            {  1.0f,      -2.5f,       2.0f,      -0.5f},
                            { -0.5f,       0.0f,       0.5f,       0.0f},
                            {  0.0f,       1.0f,       0.0f,       0.0f}};
__Basis	gHermiteBasis	= {{  2.0f,       1.0f,      -2.0f,       1.0f},
                         { -3.0f,      -2.0f,       3.0f,      -1.0f},
                         {  0.0f,       1.0f,       0.0f,       0.0f},
                         {  1.0f,       0.0f,       0.0f,       0.0f}};
__Basis	gPowerBasis	= {{  1.0f,       0.0f,       0.0f,       0.0f},
                       {  0.0f,       1.0f,       0.0f,       0.0f},
                       {  0.0f,       0.0f,       1.0f,       0.0f},
                       {  0.0f,       0.0f,       0.0f,       1.0f}};
__Basis	gLinearBasis	= {{  0.0f,       0.0f,       0.0f,       0.0f},
                        {  0.0f,       0.0f,       0.0f,       0.0f},
                        {  0.0f,      -1.0f,       1.0f,       0.0f},
                        {  0.0f,       1.0f,       0.0f,       0.0f}};


//---------------------------------------------------------------------
/** Default constructor for a cubic spline curve, defaults to Catmull-Rom basis matrix.
 */

CqSplineCubic::CqSplineCubic( TqInt cu ) : m_matBasis( gCatmullRomBasis )
{
	m_aControlPoints.resize( cu );
	m_cu = cu;
	m_Step = 1;
}


//---------------------------------------------------------------------
/** Evaluate a cubic spline curve at the specified time.
 */

CqVector4D CqSplineCubic::Evaluate( TqFloat t ) const
{
	// Set up the geometry vector.
	CqVector4D	Gx;
	CqVector4D	Gy;
	CqVector4D	Gz;

	TqFloat u = static_cast<TqFloat>( cSections() ) * t;
	TqInt iSection = static_cast<TqInt>( u );
	t = u - iSection;
	TqInt iv = iSection * m_Step;

	Gx[ 0 ] = m_aControlPoints[ 0 + iv ].x();
	Gx[ 1 ] = m_aControlPoints[ 1 + iv ].x();
	Gx[ 2 ] = m_aControlPoints[ 2 + iv ].x();
	Gx[ 3 ] = m_aControlPoints[ 3 + iv ].x();

	Gy[ 0 ] = m_aControlPoints[ 0 + iv ].y();
	Gy[ 1 ] = m_aControlPoints[ 1 + iv ].y();
	Gy[ 2 ] = m_aControlPoints[ 2 + iv ].y();
	Gy[ 3 ] = m_aControlPoints[ 3 + iv ].y();

	Gz[ 0 ] = m_aControlPoints[ 0 + iv ].z();
	Gz[ 1 ] = m_aControlPoints[ 1 + iv ].z();
	Gz[ 2 ] = m_aControlPoints[ 2 + iv ].z();
	Gz[ 3 ] = m_aControlPoints[ 3 + iv ].z();

	Gx = Gx * m_matBasis;
	Gy = Gy * m_matBasis;
	Gz = Gz * m_matBasis;

	TqFloat t2 = t * t;
	TqFloat t3 = t2 * t;

	TqFloat x = t3 * Gx[ 0 ] + t2 * Gx[ 1 ] + t * Gx[ 2 ] + Gx[ 3 ];
	TqFloat y = t3 * Gy[ 0 ] + t2 * Gy[ 1 ] + t * Gy[ 2 ] + Gy[ 3 ];
	TqFloat z = t3 * Gz[ 0 ] + t2 * Gz[ 1 ] + t * Gz[ 2 ] + Gz[ 3 ];

	return ( CqVector4D( x, y, z, 1 ) );
}


//---------------------------------------------------------------------
/** Intialise the forward differencing variables.
 */

void CqSplineCubic::InitFD( TqInt n )
{
	TqFloat d = 1.0 / static_cast<TqFloat>( n );
	TqFloat	d2 = d * d;
	TqFloat	d3 = d2 * d;

	// Calculate the deltas.
	CqVector4D	Cx;
	CqVector4D	Cy;
	CqVector4D	Cz;

	Cx[ 0 ] = m_aControlPoints[ 0 ].x();
	Cx[ 1 ] = m_aControlPoints[ 1 ].x();
	Cx[ 2 ] = m_aControlPoints[ 2 ].x();
	Cx[ 3 ] = m_aControlPoints[ 3 ].x();

	Cy[ 0 ] = m_aControlPoints[ 0 ].y();
	Cy[ 1 ] = m_aControlPoints[ 1 ].y();
	Cy[ 2 ] = m_aControlPoints[ 2 ].y();
	Cy[ 3 ] = m_aControlPoints[ 3 ].y();

	Cz[ 0 ] = m_aControlPoints[ 0 ].z();
	Cz[ 1 ] = m_aControlPoints[ 1 ].z();
	Cz[ 2 ] = m_aControlPoints[ 2 ].z();
	Cz[ 3 ] = m_aControlPoints[ 3 ].z();

	Cx = m_matBasis * Cx;
	Cy = m_matBasis * Cy;
	Cz = m_matBasis * Cz;

	// Thisis basically an optimised version of the matrix multiply of
	//						[0   ,0   ,0,1][a]
	//						[d3  ,d2  ,d,0][b]
	//						[6*d3,2*d2,0,0][c]
	//						[6*d3,0   ,0,0][d]
	Cx[ 2 ] = Cx[ 2 ] * d + Cx[ 1 ] * d2 + Cx[ 0 ] * d3;
	Cx[ 0 ] = Cx[ 0 ] * 6 * d3;
	Cx[ 1 ] = Cx[ 1 ] * 2 * d2 + Cx[ 0 ];

	Cy[ 2 ] = Cy[ 2 ] * d + Cy[ 1 ] * d2 + Cy[ 0 ] * d3;
	Cy[ 0 ] = Cy[ 0 ] * 6 * d3;
	Cy[ 1 ] = Cy[ 1 ] * 2 * d2 + Cy[ 0 ];

	Cz[ 2 ] = Cz[ 2 ] * d + Cz[ 1 ] * d2 + Cz[ 0 ] * d3;
	Cz[ 0 ] = Cz[ 0 ] * 6 * d3;
	Cz[ 1 ] = Cz[ 1 ] * 2 * d2 + Cz[ 0 ];

	m_vecFDPoint = CqVector4D( Cx[ 3 ], Cy[ 3 ], Cz[ 3 ], 1 );
	m_vecFDDelta = CqVector4D( Cx[ 2 ], Cy[ 2 ], Cz[ 2 ], 1 );
	m_vecFDDelta2 = CqVector4D( Cx[ 1 ], Cy[ 1 ], Cz[ 1 ], 1 );
	m_vecFDDelta3 = CqVector4D( Cx[ 0 ], Cy[ 0 ], Cz[ 0 ], 1 );
}


//---------------------------------------------------------------------
/** Evaluate the curve using forward differencing.
 */

CqVector4D CqSplineCubic::EvaluateFD()
{
	CqVector4D	vecPoint = m_vecFDPoint;

	m_vecFDPoint += m_vecFDDelta;
	m_vecFDDelta += m_vecFDDelta2;
	m_vecFDDelta2 += m_vecFDDelta3;

	return ( vecPoint );
}


//---------------------------------------------------------------------
/** Return the number of curve sections in the spline curve
 */

TqInt CqSplineCubic::cSections() const
{
	return ( ( ( m_cu -4 ) / m_Step ) + 1 );
}


//---------------------------------------------------------------------
/* Set the cubic spline basis matrix.
 */
void CqSplineCubic::SetBasis( const CqString& strName )
{
  __Basis * pVals = 0;
  TqInt step = 3;
  if ( strName.compare( "bezier" ) == 0 )
  {
    pVals = &gBezierBasis;
    step = 3;
  }
  else if ( strName.compare( "bspline" ) == 0 )
  {
    pVals = &gBSplineBasis;
    step = 1;
  }
  else if ( strName.compare( "catmull-rom" ) == 0 )
  {
    pVals = &gCatmullRomBasis;
    step = 1;
  }
  else if ( strName.compare( "hermite" ) == 0 )
  {
    pVals = &gHermiteBasis;
    step = 2;
  }
  else if ( strName.compare( "linear") == 0 )
  {
    pVals = &gLinearBasis;
    step = 1;
  }
  else if ( strName.compare( "power" ) == 0 )
  {
    pVals = &gPowerBasis;
    step = 4;
  }else{
    Aqsis::log() << Aqsis::warning << "Unknown spline type \"" << strName.c_str() << "\", default to \"catmull-rom\"" << std::endl;
    pVals = &gCatmullRomBasis;
    step = 1;
  };

  if ( pVals )
  {
    CqMatrix m;
    m = *pVals;
    SetmatBasis( m );
    SetStep( step );
  }
}

END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
