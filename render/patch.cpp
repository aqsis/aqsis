
// Aqsis
// Copyright � 1997 - 2001, Paul C. Gregory
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
		\brief Implements the classes and support structures for handling RenderMan patch primitives.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"imagebuffer.h"
#include	"micropolygon.h"
#include	"renderer.h"
#include	"patch.h"
#include	"vector2d.h"

START_NAMESPACE( Aqsis )


//---------------------------------------------------------------------
/** Constructor both u and vbasis matrices default to bezier.
 */

CqSurfacePatchBicubic::CqSurfacePatchBicubic() : CqSurface()
{}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqSurfacePatchBicubic::CqSurfacePatchBicubic( const CqSurfacePatchBicubic& From ) :
		CqSurface( From )
{
	*this = From;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqSurfacePatchBicubic::~CqSurfacePatchBicubic()
{}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqSurfacePatchBicubic& CqSurfacePatchBicubic::operator=( const CqSurfacePatchBicubic& From )
{
	// Perform per surface copy function
	CqSurface::operator=( From );

	//	TqInt i;
	//	for(i=0; i<16; i++)
	//		P()[i]=From.P()[i];

	return ( *this );
}



void CqSurfacePatchBicubic::NaturalSubdivide( CqParameter* pParam, CqParameter* pParam1, CqParameter* pParam2, TqBool u )
{
	switch ( pParam->Type() )
	{
			case type_float:
			{
				CqParameterTyped<TqFloat, TqFloat>* pTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam );
				CqParameterTyped<TqFloat, TqFloat>* pTResult1 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam1 );
				CqParameterTyped<TqFloat, TqFloat>* pTResult2 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_integer:
			{
				CqParameterTyped<TqInt, TqFloat>* pTParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam );
				CqParameterTyped<TqInt, TqFloat>* pTResult1 = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam1 );
				CqParameterTyped<TqInt, TqFloat>* pTResult2 = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_point:
			case type_vector:
			case type_normal:
			{
				CqParameterTyped<CqVector3D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam );
				CqParameterTyped<CqVector3D, CqVector3D>* pTResult1 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam1 );
				CqParameterTyped<CqVector3D, CqVector3D>* pTResult2 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_hpoint:
			{
				CqParameterTyped<CqVector4D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam );
				CqParameterTyped<CqVector4D, CqVector3D>* pTResult1 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam1 );
				CqParameterTyped<CqVector4D, CqVector3D>* pTResult2 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}


			case type_color:
			{
				CqParameterTyped<CqColor, CqColor>* pTParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam );
				CqParameterTyped<CqColor, CqColor>* pTResult1 = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam1 );
				CqParameterTyped<CqColor, CqColor>* pTResult2 = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_string:
			{
				CqParameterTyped<CqString, CqString>* pTParam = static_cast<CqParameterTyped<CqString, CqString>*>( pParam );
				CqParameterTyped<CqString, CqString>* pTResult1 = static_cast<CqParameterTyped<CqString, CqString>*>( pParam1 );
				CqParameterTyped<CqString, CqString>* pTResult2 = static_cast<CqParameterTyped<CqString, CqString>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_matrix:
			{
				//			CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam );
				//			CqParameterTyped<CqMatrix, CqMatrix>* pTResult1 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam1 );
				//			CqParameterTyped<CqMatrix, CqMatrix>* pTResult2 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam2 );
				//			TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				//			break;
			}
	}
}


//---------------------------------------------------------------------
/** Get the boundary extents in camera space of the surface patch
 */

CqBound CqSurfacePatchBicubic::Bound() const
{
	// Get the boundary in camera space.
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqInt i;
	for ( i = 0; i < 16; i++ )
	{
		CqVector3D	vecV = ( *P() ) [ i ];
		if ( vecV.x() < vecA.x() ) vecA.x( vecV.x() );
		if ( vecV.y() < vecA.y() ) vecA.y( vecV.y() );
		if ( vecV.x() > vecB.x() ) vecB.x( vecV.x() );
		if ( vecV.y() > vecB.y() ) vecB.y( vecV.y() );
		if ( vecV.z() < vecA.z() ) vecA.z( vecV.z() );
		if ( vecV.z() > vecB.z() ) vecB.z( vecV.z() );
	}
	CqBound	B;
	B.vecMin() = vecA;
	B.vecMax() = vecB;
	return ( B );
}


//---------------------------------------------------------------------
/** Dice the patch into a mesh of micropolygons.
 */


void CqSurfacePatchBicubic::NaturalDice( CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData )
{
	switch ( pParameter->Type() )
	{
			case type_float:
			{
				CqParameterTyped<TqFloat, TqFloat>* pTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}

			case type_integer:
			{
				CqParameterTyped<TqInt, TqFloat>* pTParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}

			case type_point:
			case type_vector:
			case type_normal:
			{
				CqParameterTyped<CqVector3D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}

			case type_hpoint:
			{
				CqParameterTyped<CqVector4D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}

			case type_color:
			{
				CqParameterTyped<CqColor, CqColor>* pTParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}

			case type_string:
			{
				CqParameterTyped<CqString, CqString>* pTParam = static_cast<CqParameterTyped<CqString, CqString>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}

			case type_matrix:
			{
				CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParameter );
				TypedNaturalDice( uDiceSize, vDiceSize, pTParam, pData );
				break;
			}
	}
}

//---------------------------------------------------------------------
/** Split the patch into smaller patches.
 */

TqInt CqSurfacePatchBicubic::PreSubdivide( std::vector<CqBasicSurface*>& aSplits, TqBool u )
{
	// Create two new surface of the appropriate type
	aSplits.push_back( new CqSurfacePatchBicubic );
	aSplits.push_back( new CqSurfacePatchBicubic );

	return ( 2 );
}

//---------------------------------------------------------------------
/** Determine whether or not the patch is diceable
 */

TqBool	CqSurfacePatchBicubic::Diceable()
{
	assert( NULL != P() );
	// If the cull check showed that the primitive cannot be diced due to crossing the e and hither planes,
	// then we can return immediately.
	if ( !m_fDiceable )
		return ( TqFalse );

	// Otherwise we should continue to try to find the most advantageous split direction, OR the dice size.
	const CqMatrix & matCtoR = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" );

	// Convert the control hull to raster space.
	CqVector2D	avecHull[ 16 ];
	TqInt i;
	TqInt gridsize;

	const TqInt* poptGridSize = QGetRenderContext() ->optCurrent().GetIntegerOption( "limits", "gridsize" );
	TqInt m_XBucketSize = 16;
	TqInt m_YBucketSize = 16;
	const TqInt* poptBucketSize = QGetRenderContext() ->optCurrent().GetIntegerOption( "limits", "bucketsize" );
	if ( poptBucketSize != 0 )
	{
		m_XBucketSize = poptBucketSize[ 0 ];
		m_YBucketSize = poptBucketSize[ 1 ];
	}
	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ];
	if ( poptGridSize )
		gridsize = poptGridSize[ 0 ];
	else
		gridsize = static_cast<TqInt>( m_XBucketSize * m_XBucketSize / ShadingRate );
	for ( i = 0; i < 16; i++ )
		avecHull[ i ] = matCtoR * ( *P() ) [ i ];

	// First check flatness, a curve which is too far off flat will
	// produce unreliable results when the length is approximated below.
	m_SplitDir = SplitDir_U;
	TqInt u;
	for ( u = 0; u < 16; u += 4 )
	{
		// Find an initial line
		TqFloat Len = 0;
		CqVector2D	vec0 = avecHull[ u ];
		CqVector2D	vecL;
		TqInt i = 4;
		while ( i-- > 0 && Len < FLT_EPSILON )
		{
			vecL = avecHull[ u + i ] - vec0;
			Len = vecL.Magnitude();
		}
		vecL /= Len;	// Normalise

		i = 0;
		while ( i++ < 4 )
		{
			// Get the distance to the line for each point
			CqVector3D	vec = avecHull[ u + i ] - vec0;
			vec.Unit();
			vec %= vecL;
			if ( vec.Magnitude() > 1 ) return ( TqFalse );
		}
	}
	m_SplitDir = SplitDir_V;
	TqInt v;
	for ( v = 0; v < 4; v++ )
	{
		// Find an initial line
		TqFloat Len = 0;
		CqVector2D	vec0 = avecHull[ v ];
		CqVector2D	vecL;
		TqInt i = 4;
		while ( i-- > 0 && Len < FLT_EPSILON )
		{
			vecL = avecHull[ v + ( i * 4 ) ] - vec0;
			Len = vecL.Magnitude();
		}
		vecL /= Len;	// Normalise

		i = 0;
		while ( i++ < 4 )
		{
			// Get the distance to the line for each point
			CqVector3D	vec = avecHull[ v + ( i * 4 ) ] - vec0;
			vec.Unit();
			vec %= vecL;
			if ( vec.Magnitude() > 1 ) return ( TqFalse );
		}
	}


	TqFloat uLen = 0;
	TqFloat vLen = 0;

	for ( u = 0; u < 16; u += 4 )
	{
		CqVector2D	Vec1 = avecHull[ u + 1 ] - avecHull[ u ];
		CqVector2D	Vec2 = avecHull[ u + 2 ] - avecHull[ u + 1 ];
		CqVector2D	Vec3 = avecHull[ u + 3 ] - avecHull[ u + 2 ];
		if ( Vec1.Magnitude2() > uLen ) uLen = Vec1.Magnitude2();
		if ( Vec2.Magnitude2() > uLen ) uLen = Vec2.Magnitude2();
		if ( Vec3.Magnitude2() > uLen ) uLen = Vec3.Magnitude2();
	}
	for ( v = 0; v < 4; v++ )
	{
		CqVector2D	Vec1 = avecHull[ v + 4 ] - avecHull[ v ];
		CqVector2D	Vec2 = avecHull[ v + 8 ] - avecHull[ v + 4 ];
		CqVector2D	Vec3 = avecHull[ v + 12 ] - avecHull[ v + 8 ];
		if ( Vec1.Magnitude2() > vLen ) vLen = Vec1.Magnitude2();
		if ( Vec2.Magnitude2() > vLen ) vLen = Vec2.Magnitude2();
		if ( Vec3.Magnitude2() > vLen ) vLen = Vec3.Magnitude2();
	}

	ShadingRate = static_cast<float>( sqrt( ShadingRate ) );
	uLen = sqrt( uLen ) / ShadingRate;
	vLen = sqrt( vLen ) / ShadingRate;

	m_SplitDir = ( uLen > vLen ) ? SplitDir_U : SplitDir_V;
	// TODO: Should ensure powers of half to prevent cracking.
	uLen *= 3;
	vLen *= 3;
	m_uDiceSize = static_cast<TqInt>( MAX( ROUND( uLen ), 1 ) );
	m_vDiceSize = static_cast<TqInt>( MAX( ROUND( vLen ), 1 ) );

	// Ensure power of 2 to avoid cracking
	m_uDiceSize = CEIL_POW2( m_uDiceSize );
	m_vDiceSize = CEIL_POW2( m_vDiceSize );

	TqFloat Area = m_uDiceSize * m_vDiceSize;

	if ( uLen < FLT_EPSILON || vLen < FLT_EPSILON )
	{
		m_fDiscard = TqTrue;
		return ( TqFalse );
	}

	if ( fabs( Area ) > gridsize )
		return ( TqFalse );

	return ( TqTrue );
}


//---------------------------------------------------------------------
/** Convert from the current basis into Bezier for processing.
 */

void CqSurfacePatchBicubic::ConvertToBezierBasis()
{
	static CqMatrix matMim1;
	TqInt i, j;

	if ( matMim1.fIdentity() )
	{
		for ( i = 0; i < 4; i++ )
			for ( j = 0; j < 4; j++ )
				matMim1[ i ][ j ] = RiBezierBasis[ i ][ j ];
		matMim1.SetfIdentity( TqFalse );
		matMim1 = matMim1.Inverse();
	}

	CqMatrix matuMj = matuBasis();
	CqMatrix matvMj = matvBasis();

	CqMatrix matuConv = matuMj * matMim1;
	CqMatrix matvConv = matvMj * matMim1;

	CqMatrix matCPx, matCPy, matCPz, matCPh;
	for ( i = 0; i < 4; i++ )
	{
		for ( j = 0; j < 4; j++ )
		{
			matCPx[ i ][ j ] = CP( i, j ).x();
			matCPy[ i ][ j ] = CP( i, j ).y();
			matCPz[ i ][ j ] = CP( i, j ).z();
			matCPh[ i ][ j ] = CP( i, j ).h();
		}
	}
	matCPx.SetfIdentity( TqFalse );
	matCPy.SetfIdentity( TqFalse );
	matCPz.SetfIdentity( TqFalse );
	matCPh.SetfIdentity( TqFalse );

	matCPx = matuConv.Transpose() * matCPx * matvConv;
	matCPy = matuConv.Transpose() * matCPy * matvConv;
	matCPz = matuConv.Transpose() * matCPz * matvConv;
	matCPh = matuConv.Transpose() * matCPh * matvConv;

	for ( i = 0; i < 4; i++ )
	{
		for ( j = 0; j < 4; j++ )
		{
			CP( i, j ).x( matCPx[ i ][ j ] );
			CP( i, j ).y( matCPy[ i ][ j ] );
			CP( i, j ).z( matCPz[ i ][ j ] );
			CP( i, j ).h( matCPh[ i ][ j ] );
		}
	}
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqSurfacePatchBilinear::CqSurfacePatchBilinear() : CqSurface(), m_fHasPhantomFourthVertex(TqFalse), m_iInternalu(-1), m_iInternalv(-1)
{}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqSurfacePatchBilinear::CqSurfacePatchBilinear( const CqSurfacePatchBilinear& From ) :
		CqSurface( From )
{
	*this = From;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqSurfacePatchBilinear::~CqSurfacePatchBilinear()
{}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqSurfacePatchBilinear& CqSurfacePatchBilinear::operator=( const CqSurfacePatchBilinear& From )
{
	CqSurface::operator=( From );

	m_fHasPhantomFourthVertex = From.m_fHasPhantomFourthVertex;
	m_iInternalu = From.m_iInternalu;
	m_iInternalv = From.m_iInternalv;

	return ( *this );
}


//---------------------------------------------------------------------
/** Generate the vertex normals if not specified.
 */

void CqSurfacePatchBilinear::GenerateGeometricNormals( TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pNormals )
{
	assert( NULL != P() && P() ->Size() == 4 );

	// Get the handedness of the coordinate system (at the time of creation) and
	// the coordinate system specified, to check for normal flipping.
	TqInt O = pAttributes() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ];

	// For each of the four points, calculate the normal as the cross product of its
	// two vectors.
	CqVector3D N1 = ( ( *P() ) [ 1 ] - ( *P() ) [ 0 ] ) % ( ( *P() ) [ 2 ] - ( *P() ) [ 0 ] );
	CqVector3D N2 = ( ( *P() ) [ 3 ] - ( *P() ) [ 1 ] ) % ( ( *P() ) [ 0 ] - ( *P() ) [ 1 ] );
	CqVector3D N3 = ( ( *P() ) [ 0 ] - ( *P() ) [ 2 ] ) % ( ( *P() ) [ 3 ] - ( *P() ) [ 2 ] );
	CqVector3D N4 = ( ( *P() ) [ 2 ] - ( *P() ) [ 3 ] ) % ( ( *P() ) [ 1 ] - ( *P() ) [ 3 ] );

	CqVector3D	N;
	TqInt v, u;
	for ( v = 0; v <= vDiceSize; v++ )
	{
		for ( u = 0; u <= uDiceSize; u++ )
		{
			N = BilinearEvaluate( N1, N2, N3, N4, u, v );
			N = ( O == OrientationLH ) ? N : -N;
			pNormals->SetNormal( N, ( v * ( uDiceSize + 1 ) ) + u );
		}
	}
}


//---------------------------------------------------------------------
/** Return the boundary extents in camera space of the surface patch
 */

CqBound CqSurfacePatchBilinear::Bound() const
{
	assert( NULL != P() );

	// Get the boundary in camera space.
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqInt i;
	for ( i = 0; i < (m_fHasPhantomFourthVertex?3:4); i++ )
	{
		CqVector3D	vecV = ( *P() ) [ i ];
		if ( vecV.x() < vecA.x() ) vecA.x( vecV.x() );
		if ( vecV.y() < vecA.y() ) vecA.y( vecV.y() );
		if ( vecV.x() > vecB.x() ) vecB.x( vecV.x() );
		if ( vecV.y() > vecB.y() ) vecB.y( vecV.y() );
		if ( vecV.z() < vecA.z() ) vecA.z( vecV.z() );
		if ( vecV.z() > vecB.z() ) vecB.z( vecV.z() );
	}
	CqBound	B;
	B.vecMin() = vecA;
	B.vecMax() = vecB;
	return ( B );
}



//---------------------------------------------------------------------
/** Split the patch into smaller patches.
 */

TqInt CqSurfacePatchBilinear::PreSubdivide( std::vector<CqBasicSurface*>& aSplits, TqBool u )
{
	aSplits.push_back( new CqSurfacePatchBilinear );
	aSplits.push_back( new CqSurfacePatchBilinear );

	return ( 2 );
}


//---------------------------------------------------------------------
/** Determine whether or not the patch is diceable
 */

TqBool	CqSurfacePatchBilinear::Diceable()
{
	assert( NULL != P() );

	// If the cull check showed that the primitive cannot be diced due to crossing the e and hither planes,
	// then we can return immediately.
	if ( !m_fDiceable )
		return ( TqFalse );

	// Otherwise we should continue to try to find the most advantageous split direction, OR the dice size.
	const CqMatrix & matCtoR = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" );

	// Convert the control hull to raster space.
	CqVector2D	avecHull[ 4 ];
	TqInt i;
	TqInt gridsize;

	const TqInt* poptGridSize = QGetRenderContext() ->optCurrent().GetIntegerOption( "limits", "gridsize" );
	TqInt m_XBucketSize = 16;
	TqInt m_YBucketSize = 16;
	const TqInt* poptBucketSize = QGetRenderContext() ->optCurrent().GetIntegerOption( "limits", "bucketsize" );
	if ( poptBucketSize != 0 )
	{
		m_XBucketSize = poptBucketSize[ 0 ];
		m_YBucketSize = poptBucketSize[ 1 ];
	}
	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ];
	if ( poptGridSize )
		gridsize = poptGridSize[ 0 ];
	else
		gridsize = static_cast<TqInt>( m_XBucketSize * m_XBucketSize / ShadingRate );
	for ( i = 0; i < 4; i++ )
		avecHull[ i ] = matCtoR * ( *P() ) [ i ];

	TqFloat uLen = 0;
	TqFloat vLen = 0;

	CqVector2D	Vec1 = avecHull[ 1 ] - avecHull[ 0 ];
	CqVector2D	Vec2 = avecHull[ 3 ] - avecHull[ 2 ];
	uLen = ( Vec1.Magnitude2() > Vec2.Magnitude2() ) ? Vec1.Magnitude2() : Vec2.Magnitude2();

	Vec1 = avecHull[ 2 ] - avecHull[ 0 ];
	Vec2 = avecHull[ 3 ] - avecHull[ 1 ];
	vLen = ( Vec1.Magnitude2() > Vec2.Magnitude2() ) ? Vec1.Magnitude2() : Vec2.Magnitude2();

	ShadingRate = static_cast<float>( sqrt( ShadingRate ) );
	uLen = sqrt( uLen ) / ShadingRate;
	vLen = sqrt( vLen ) / ShadingRate;

	m_SplitDir = ( uLen > vLen ) ? SplitDir_U : SplitDir_V;

	// TODO: Should ensure powers of half to prevent cracking.
	uLen = MAX( ROUND( uLen ), 1 );
	vLen = MAX( ROUND( vLen ), 1 );
	TqFloat Area = uLen * vLen;
	m_uDiceSize = static_cast<TqInt>( uLen );
	m_vDiceSize = static_cast<TqInt>( vLen );

	// Ensure power of 2 to avoid cracking
	m_uDiceSize = CEIL_POW2( m_uDiceSize );
	m_vDiceSize = CEIL_POW2( m_vDiceSize );

	if ( uLen < FLT_EPSILON || vLen < FLT_EPSILON )
	{
		m_fDiscard = TqTrue;
		return ( TqFalse );
	}

	if ( fabs( Area ) > gridsize )
		return ( TqFalse );

	return ( TqTrue );
}



//---------------------------------------------------------------------
/** Perform post split operations.
 */

void CqSurfacePatchBilinear::PostSubdivide(std::vector<CqBasicSurface*>& aSplits)
{
	if(aSplits.size() == 4)
	{
		delete(aSplits.back());
		aSplits.pop_back();
	}
}


TqInt CqSurfacePatchBilinear::Split( std::vector<CqBasicSurface*>& aSplits )
{
	aSplits.push_back( new CqSurfacePatchBilinear );
	aSplits.push_back( new CqSurfacePatchBilinear );
	
	if(m_fHasPhantomFourthVertex)
	{
		aSplits.push_back( new CqSurfacePatchBilinear );
		aSplits.push_back( new CqSurfacePatchBilinear );
	}

	TqInt i;
	for(i=0; i<(m_fHasPhantomFourthVertex?4:2); i++)
	{
		aSplits[ i ] ->SetSurfaceParameters( *this );
		aSplits[ i ] ->SetSplitDir( ( SplitDir() == SplitDir_U ) ? SplitDir_V : SplitDir_U );
		aSplits[ i ] ->SetEyeSplitCount( EyeSplitCount() );
		aSplits[ i ] ->m_fDiceable = TqTrue;
		aSplits[ i ] ->AddRef();
	}

	// Iterate through any use parameters subdividing and storing the second value in the target surface.
	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		CqParameter* pNewA = ( *iUP ) ->Clone();
		CqParameter* pNewB = ( *iUP ) ->Clone();
		( *iUP ) ->Subdivide( pNewA, pNewB, SplitDir() == SplitDir_U, this );

		if(m_fHasPhantomFourthVertex)
		{
			CqParameter* pNewC = pNewA ->Clone();
			CqParameter* pNewD = pNewA ->Clone();
			CqParameter* pNewE = pNewB ->Clone();
			CqParameter* pNewF = pNewB ->Clone();
			pNewA ->Subdivide( pNewC, pNewD, SplitDir() == SplitDir_V, this );
			pNewB ->Subdivide( pNewE, pNewF, SplitDir() == SplitDir_V, this );

			static_cast<CqSurface*>( aSplits[ 0 ] ) ->AddPrimitiveVariable( pNewC );
			static_cast<CqSurface*>( aSplits[ 1 ] ) ->AddPrimitiveVariable( pNewD );
			static_cast<CqSurface*>( aSplits[ 2 ] ) ->AddPrimitiveVariable( pNewE );
			static_cast<CqSurface*>( aSplits[ 3 ] ) ->AddPrimitiveVariable( pNewF );

			delete(pNewA);
			delete(pNewB);
		}
		else
		{
			static_cast<CqSurface*>( aSplits[ 0 ] ) ->AddPrimitiveVariable( pNewA );
			static_cast<CqSurface*>( aSplits[ 1 ] ) ->AddPrimitiveVariable( pNewB );
		}
	}
	
	if(m_fHasPhantomFourthVertex)
	{
		delete(aSplits.back());
		aSplits.pop_back();

		static_cast<CqSurfacePatchBilinear*>( aSplits[ 0 ] ) ->m_fHasPhantomFourthVertex = TqFalse;
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 1 ] ) ->m_fHasPhantomFourthVertex = TqTrue;
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 2 ] ) ->m_fHasPhantomFourthVertex = TqTrue;

		return ( 3 );
	}
	else
	{
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 0 ] ) ->m_fHasPhantomFourthVertex = TqFalse;
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 1 ] ) ->m_fHasPhantomFourthVertex = TqFalse;

		return ( 2 );
	}
}


//---------------------------------------------------------------------
/**
 * Adds a primitive variable to the list of user parameters.  This method
 * caches the indexes of the "__internal_u" and "__internal_v" parameters within
 * the array of user parameters for later access.
 *
 * @param pParam        Pointer to the parameter to add.
 */
void CqSurfacePatchBilinear::AddPrimitiveVariable( CqParameter* pParam )
{
	// add the primitive variable using the superclass method
	CqSurface::AddPrimitiveVariable( pParam );

	// trap the indexes of "width" and "constantwidth" parameters
	if ( pParam->strName() == "__internal_u" )
		m_iInternalu = m_aUserParams.size() - 1;
	else if ( pParam->strName() == "__internal_v" )
		m_iInternalv = m_aUserParams.size() - 1;
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqSurfacePatchMeshBicubic::CqSurfacePatchMeshBicubic( const CqSurfacePatchMeshBicubic& From ) :
		CqSurface( From )
{
	*this = From;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqSurfacePatchMeshBicubic::~CqSurfacePatchMeshBicubic()
{}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqSurfacePatchMeshBicubic& CqSurfacePatchMeshBicubic::operator=( const CqSurfacePatchMeshBicubic& From )
{
	// Perform per surface copy function
	CqSurface::operator=( From );

	m_uPatches = From.m_uPatches;
	m_vPatches = From.m_vPatches;
	m_nu = From.m_nu;
	m_nv = From.m_nv;
	m_uPeriodic = From.m_uPeriodic;
	m_vPeriodic = From.m_vPeriodic;

	return ( *this );
}


//---------------------------------------------------------------------
/** Get the boundary extents in camera space of the surface patch mesh
 */

CqBound CqSurfacePatchMeshBicubic::Bound() const
{
	assert( NULL != P() );

	// Get the boundary in camera space.
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqUint i;
	for ( i = 0; i < P() ->Size(); i++ )
	{
		CqVector3D	vecV = ( *P() ) [ i ];
		if ( vecV.x() < vecA.x() ) vecA.x( vecV.x() );
		if ( vecV.y() < vecA.y() ) vecA.y( vecV.y() );
		if ( vecV.x() > vecB.x() ) vecB.x( vecV.x() );
		if ( vecV.y() > vecB.y() ) vecB.y( vecV.y() );
		if ( vecV.z() < vecA.z() ) vecA.z( vecV.z() );
		if ( vecV.z() > vecB.z() ) vecB.z( vecV.z() );
	}
	CqBound	B;
	B.vecMin() = vecA;
	B.vecMax() = vecB;
	return ( B );
}


//---------------------------------------------------------------------
/** Split the patch mesh into individual patches and post them.
 */

#define	PatchCoord(v,u)	((((v)%m_nv)*m_nu)+((u)%m_nu))
#define	PatchCorner(v,u)	((((v)%nvaryingv)*nvaryingu)+((u)%nvaryingu));

TqInt CqSurfacePatchMeshBicubic::Split( std::vector<CqBasicSurface*>& aSplits )
{
	TqInt cSplits = 0;

	CqVector4D vecPoint;
	TqInt iP = 0;
	TqInt uStep = pAttributes() ->GetIntegerAttribute( "System", "BasisStep" ) [ 0 ];
	TqInt vStep = pAttributes() ->GetIntegerAttribute( "System", "BasisStep" ) [ 1 ];

	TqInt nvaryingu = ( m_uPeriodic ) ? m_uPatches : m_uPatches + 1;
	TqInt nvaryingv = ( m_vPeriodic ) ? m_vPatches : m_vPatches + 1;

	TqInt MyUses = Uses();

	const TqFloat* pTC = pAttributes() ->GetFloatAttribute( "System", "TextureCoordinates" );
	CqVector2D st1( pTC[ 0 ], pTC[ 1 ] );
	CqVector2D st2( pTC[ 2 ], pTC[ 3 ] );
	CqVector2D st3( pTC[ 4 ], pTC[ 5 ] );
	CqVector2D st4( pTC[ 6 ], pTC[ 7 ] );

	// Fill in the variables.
	TqInt i;
	for ( i = 0; i < m_vPatches; i++ )
	{
		// vRow is the coordinate row of the mesh.
		RtInt	vRow = i * vStep;
		TqFloat v0 = ( 1.0f / m_vPatches ) * i;
		TqFloat v1 = ( 1.0f / m_vPatches ) * ( i + 1 );
		RtInt j;
		for ( j = 0; j < m_uPatches; j++ )
		{
			// uCol is the coordinate column of the mesh.
			RtInt uCol = j * uStep;
			CqSurfacePatchBicubic*	pSurface = new CqSurfacePatchBicubic();
			pSurface->AddRef();
			pSurface->SetSurfaceParameters( *this );

			RtInt v;

			TqInt iTa = PatchCorner( i, j );
			TqInt iTb = PatchCorner( i, j + 1 );
			TqInt iTc = PatchCorner( i + 1, j );
			TqInt iTd = PatchCorner( i + 1, j + 1 );

			TqFloat u0 = ( 1.0f / m_uPatches ) * j;
			TqFloat u1 = ( 1.0f / m_uPatches ) * ( j + 1 );

			std::vector<CqParameter*>::iterator iUP;
			for ( iUP = aUserParams().begin(); iUP != aUserParams().end(); iUP++ )
			{
				if ( ( *iUP ) ->Class() == class_varying )
				{
					// Copy any 'varying' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( pSurface->cVarying() );

					pNewUP->SetValue( ( *iUP ), 0, iTa );
					pNewUP->SetValue( ( *iUP ), 1, iTb );
					pNewUP->SetValue( ( *iUP ), 2, iTc );
					pNewUP->SetValue( ( *iUP ), 3, iTd );
					pSurface->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_vertex )
				{
					// Copy any 'vertex' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->Clear();
					pNewUP->SetSize( pSurface->cVertex() );

					for ( v = 0; v < 4; v++ )
					{
						iP = PatchCoord( vRow + v, uCol );
						pNewUP->SetValue( ( *iUP ), ( v * 4 ), iP );
						iP = PatchCoord( vRow + v, uCol + 1 );
						pNewUP->SetValue( ( *iUP ), ( v * 4 ) + 1, iP );
						iP = PatchCoord( vRow + v, uCol + 2 );
						pNewUP->SetValue( ( *iUP ), ( v * 4 ) + 2, iP );
						iP = PatchCoord( vRow + v, uCol + 3 );
						pNewUP->SetValue( ( *iUP ), ( v * 4 ) + 3, iP );
					}
					pSurface->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_uniform )
				{
					// Copy any 'uniform' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( pSurface->cUniform() );
					pNewUP->SetValue( ( *iUP ), 0, j );
					pSurface->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_constant )
				{
					// Copy any 'constant' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( 1 );
					pNewUP->SetValue( ( *iUP ), 0, 0 );
					pSurface->AddPrimitiveVariable( pNewUP );
				}
			}

			// If the shaders need u/v or s/t and they are not specified, then we need to put them in as defaults.
			if ( USES( MyUses, EnvVars_u ) && !bHasu() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "u" ) );
				pSurface->u() ->SetSize( 4 );
				pSurface->u() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u0, v0 );
				pSurface->u() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u1, v0 );
				pSurface->u() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u0, v1 );
				pSurface->u() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u1, v1 );
			}

			if ( USES( MyUses, EnvVars_v ) && !bHasv() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "v" ) );
				pSurface->v() ->SetSize( 4 );
				pSurface->v() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u0, v0 );
				pSurface->v() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u1, v0 );
				pSurface->v() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u0, v1 );
				pSurface->v() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u1, v1 );
			}

			if ( USES( MyUses, EnvVars_s ) && !bHass() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "s" ) );
				pSurface->s() ->SetSize( 4 );
				pSurface->s() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u0, v0 );
				pSurface->s() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u1, v0 );
				pSurface->s() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u0, v1 );
				pSurface->s() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u1, v1 );
			}

			if ( USES( MyUses, EnvVars_t ) && !bHast() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "t" ) );
				pSurface->t() ->SetSize( 4 );
				pSurface->t() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u0, v0 );
				pSurface->t() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u1, v0 );
				pSurface->t() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u0, v1 );
				pSurface->t() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u1, v1 );
			}

			aSplits.push_back( pSurface );
			cSplits++;
		}
	}
	return ( cSplits );
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqSurfacePatchMeshBilinear::CqSurfacePatchMeshBilinear( const CqSurfacePatchMeshBilinear& From ) :
		CqSurface( From )
{
	*this = From;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqSurfacePatchMeshBilinear::~CqSurfacePatchMeshBilinear()
{}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqSurfacePatchMeshBilinear& CqSurfacePatchMeshBilinear::operator=( const CqSurfacePatchMeshBilinear& From )
{
	// Perform per surface copy function
	CqSurface::operator=( From );

	m_uPatches = From.m_uPatches;
	m_vPatches = From.m_vPatches;
	m_nu = From.m_nu;
	m_nv = From.m_nv;
	m_uPeriodic = From.m_uPeriodic;
	m_vPeriodic = From.m_vPeriodic;

	return ( *this );
}


//---------------------------------------------------------------------
/** Get the boundary extents in camera space of the surface patch mesh
 */

CqBound CqSurfacePatchMeshBilinear::Bound() const
{
	assert( NULL != P() );

	// Get the boundary in camera space.
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqUint i;
	for ( i = 0; i < P() ->Size(); i++ )
	{
		CqVector3D	vecV = ( *P() ) [ i ];
		if ( vecV.x() < vecA.x() ) vecA.x( vecV.x() );
		if ( vecV.y() < vecA.y() ) vecA.y( vecV.y() );
		if ( vecV.x() > vecB.x() ) vecB.x( vecV.x() );
		if ( vecV.y() > vecB.y() ) vecB.y( vecV.y() );
		if ( vecV.z() < vecA.z() ) vecA.z( vecV.z() );
		if ( vecV.z() > vecB.z() ) vecB.z( vecV.z() );
	}
	CqBound	B;
	B.vecMin() = vecA;
	B.vecMax() = vecB;
	return ( B );
}


//---------------------------------------------------------------------
/** Split the patch mesh into individual patches and post them.
 */

#define	PatchCoord(v,u)	((((v)%m_nv)*m_nu)+((u)%m_nu))
#define	PatchCorner(v,u)	((((v)%nvaryingv)*nvaryingu)+((u)%nvaryingu));

TqInt CqSurfacePatchMeshBilinear::Split( std::vector<CqBasicSurface*>& aSplits )
{
	TqInt cSplits = 0;

	// Create a surface patch
	RtInt iP = 0;
	TqInt MyUses = Uses();

	const TqFloat* pTC = pAttributes() ->GetFloatAttribute( "System", "TextureCoordinates" );
	CqVector2D st1( pTC[ 0 ], pTC[ 1 ] );
	CqVector2D st2( pTC[ 2 ], pTC[ 3 ] );
	CqVector2D st3( pTC[ 4 ], pTC[ 5 ] );
	CqVector2D st4( pTC[ 6 ], pTC[ 7 ] );

	TqInt i;
	for ( i = 0; i < m_vPatches; i++ )
	{
		TqFloat v0 = ( 1.0f / m_vPatches ) * i;
		TqFloat v1 = ( 1.0f / m_vPatches ) * ( i + 1 );
		RtInt j;
		for ( j = 0; j < m_uPatches; j++ )
		{
			CqSurfacePatchBilinear*	pSurface = new CqSurfacePatchBilinear;
			pSurface->AddRef();
			pSurface->SetSurfaceParameters( *this );

			RtInt iTa = PatchCoord( i, j );
			RtInt iTb = PatchCoord( i, j + 1 );
			RtInt iTc = PatchCoord( i + 1, j );
			RtInt iTd = PatchCoord( i + 1, j + 1 );

			TqFloat u0 = ( 1.0f / m_uPatches ) * j;
			TqFloat u1 = ( 1.0f / m_uPatches ) * ( j + 1 );

			// Copy any primitive variables.
			std::vector<CqParameter*>::iterator iUP;
			for ( iUP = aUserParams().begin(); iUP != aUserParams().end(); iUP++ )
			{
				if ( ( *iUP ) ->Class() == class_varying )
				{
					// Copy any 'varying' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( pSurface->cVarying() );

					pNewUP->SetValue( ( *iUP ), 0, iTa );
					pNewUP->SetValue( ( *iUP ), 1, iTb );
					pNewUP->SetValue( ( *iUP ), 2, iTc );
					pNewUP->SetValue( ( *iUP ), 3, iTd );

					pSurface->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_vertex )
				{
					// Copy any 'vertex' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( pSurface->cVertex() );

					iP = PatchCoord( i, j );
					pNewUP->SetValue( ( *iUP ), 0, iP );
					iP = PatchCoord( i, j + 1 );
					pNewUP->SetValue( ( *iUP ), 1, iP );
					iP = PatchCoord( i + 1, j );
					pNewUP->SetValue( ( *iUP ), 2, iP );
					iP = PatchCoord( i + 1, j + 1 );
					pNewUP->SetValue( ( *iUP ), 3, iP );

					pSurface->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_uniform )
				{
					// Copy any 'uniform' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( pSurface->cUniform() );
					pNewUP->SetValue( ( *iUP ), 0, j );
					pSurface->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_constant )
				{
					// Copy any 'constant' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( 1 );

					pNewUP->SetValue( ( *iUP ), 0, 0 );
					pSurface->AddPrimitiveVariable( pNewUP );
				}
			}

			// If the shaders need u/v or s/t and they are not specified, then we need to put them in as defaults.
			if ( USES( MyUses, EnvVars_u ) && !bHasu() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "u" ) );
				pSurface->u() ->SetSize( 4 );
				pSurface->u() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u0, v0 );
				pSurface->u() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u1, v0 );
				pSurface->u() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u0, v1 );
				pSurface->u() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u1, v1 );
			}

			if ( USES( MyUses, EnvVars_v ) && !bHasv() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "v" ) );
				pSurface->v() ->SetSize( 4 );
				pSurface->v() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u0, v0 );
				pSurface->v() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u1, v0 );
				pSurface->v() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u0, v1 );
				pSurface->v() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u1, v1 );
			}

			if ( USES( MyUses, EnvVars_s ) && !bHass() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "s" ) );
				pSurface->s() ->SetSize( 4 );
				pSurface->s() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u0, v0 );
				pSurface->s() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u1, v0 );
				pSurface->s() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u0, v1 );
				pSurface->s() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u1, v1 );
			}

			if ( USES( MyUses, EnvVars_t ) && !bHast() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "t" ) );
				pSurface->t() ->SetSize( 4 );
				pSurface->t() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u0, v0 );
				pSurface->t() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u1, v0 );
				pSurface->t() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u0, v1 );
				pSurface->t() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u1, v1 );
			}

			aSplits.push_back( pSurface );
			cSplits++;
		}
	}
	return ( cSplits );
}


END_NAMESPACE( Aqsis )
