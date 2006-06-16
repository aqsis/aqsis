
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
		\brief Implements the classes and support structures for handling RenderMan patch primitives.
		\author Paul C. Gregory (pgregory@aqsis.org)
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
{
	STATS_INC( GPR_patch );
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

/* CqSurfacePatchBicubic::CqSurfacePatchBicubic( const CqSurfacePatchBicubic& From ) :
 * 		CqSurface( From )
 * {
 * 	*this = From;
 * 
 * 	STATS_INC( GPR_patch );
 * }
 */


//---------------------------------------------------------------------
/** Destructor.
 */

CqSurfacePatchBicubic::~CqSurfacePatchBicubic()
{}


//---------------------------------------------------------------------
/** Create a clone of this patch surface.
 */

CqSurface* CqSurfacePatchBicubic::Clone() const
{
	CqSurfacePatchBicubic* clone = new CqSurfacePatchBicubic();
	CqSurface::CloneData(clone);

	return ( clone );
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

			default:
			{
				// left blank to avoid compiler warnings about unhandled types
				break;
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
		CqVector3D	vecV = P()->pValue( i )[0];
		if ( vecV.x() < vecA.x() )
			vecA.x( vecV.x() );
		if ( vecV.y() < vecA.y() )
			vecA.y( vecV.y() );
		if ( vecV.x() > vecB.x() )
			vecB.x( vecV.x() );
		if ( vecV.y() > vecB.y() )
			vecB.y( vecV.y() );
		if ( vecV.z() < vecA.z() )
			vecA.z( vecV.z() );
		if ( vecV.z() > vecB.z() )
			vecB.z( vecV.z() );
	}
	CqBound	B;
	B.vecMin() = vecA;
	B.vecMax() = vecB;
	return ( AdjustBoundForTransformationMotion( B ) );
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

			default:
			{
				// left blank to avoid compiler warnings about unhandled types
				break;
			}
	}
}

//---------------------------------------------------------------------
/** Split the patch into smaller patches.
 */

TqInt CqSurfacePatchBicubic::PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, TqBool u )
{
	// Create two new surface of the appropriate type
	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqSurfacePatchBicubic ) );
	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqSurfacePatchBicubic ) );

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
	const CqMatrix & matCtoR = QGetRenderContext() ->matSpaceToSpace( "camera", "raster", NULL, NULL, QGetRenderContext()->Time() );

	// Convert the control hull to raster space.
	CqVector2D	avecHull[ 16 ];
	TqInt i;

	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ];

	for ( i = 0; i < 16; i++ )
		avecHull[ i ] = matCtoR * P()->pValue( i )[0];

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
			if ( vec.Magnitude() > 1 )
				return ( TqFalse );
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
			if ( vec.Magnitude() > 1 )
				return ( TqFalse );
		}
	}


	TqFloat uLen = 0;
	TqFloat vLen = 0;

	for ( u = 0; u < 16; u += 4 )
	{
		CqVector2D	Vec1 = avecHull[ u + 1 ] - avecHull[ u ];
		CqVector2D	Vec2 = avecHull[ u + 2 ] - avecHull[ u + 1 ];
		CqVector2D	Vec3 = avecHull[ u + 3 ] - avecHull[ u + 2 ];
		if ( Vec1.Magnitude2() > uLen )
			uLen = Vec1.Magnitude2();
		if ( Vec2.Magnitude2() > uLen )
			uLen = Vec2.Magnitude2();
		if ( Vec3.Magnitude2() > uLen )
			uLen = Vec3.Magnitude2();
	}
	for ( v = 0; v < 4; v++ )
	{
		CqVector2D	Vec1 = avecHull[ v + 4 ] - avecHull[ v ];
		CqVector2D	Vec2 = avecHull[ v + 8 ] - avecHull[ v + 4 ];
		CqVector2D	Vec3 = avecHull[ v + 12 ] - avecHull[ v + 8 ];
		if ( Vec1.Magnitude2() > vLen )
			vLen = Vec1.Magnitude2();
		if ( Vec2.Magnitude2() > vLen )
			vLen = Vec2.Magnitude2();
		if ( Vec3.Magnitude2() > vLen )
			vLen = Vec3.Magnitude2();
	}

	uLen = sqrt( uLen  / ShadingRate);
	vLen = sqrt( vLen  / ShadingRate);

	m_SplitDir = ( uLen > vLen ) ? SplitDir_U : SplitDir_V;
	// TODO: Should ensure powers of half to prevent cracking.
	uLen *= 3;
	vLen *= 3;
	m_uDiceSize = static_cast<TqInt>( MAX( ROUND( uLen ), 1 ) );
	m_vDiceSize = static_cast<TqInt>( MAX( ROUND( vLen ), 1 ) );

	// Ensure power of 2 to avoid cracking
	const TqInt *binary = pAttributes() ->GetIntegerAttribute( "dice", "binary" );
	if ( binary && *binary)
	{
		m_uDiceSize = CEIL_POW2( m_uDiceSize );
		m_vDiceSize = CEIL_POW2( m_vDiceSize );
	}

	if ( uLen < FLT_EPSILON || vLen < FLT_EPSILON )
	{
		m_fDiscard = TqTrue;
		return ( TqFalse );
	}

	TqFloat gs = 16.0f;
	const TqFloat* poptGridSize = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "SqrtGridSize" );
	if( NULL != poptGridSize )
		gs = poptGridSize[0];

	if( m_uDiceSize * m_vDiceSize > gs * gs )
		return TqFalse;

	return ( TqTrue );
}


//---------------------------------------------------------------------
/** Convert from the current basis into Bezier for processing.
 */

void CqSurfacePatchBicubic::ConvertToBezierBasis( CqMatrix& matuBasis, CqMatrix& matvBasis )
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

	CqMatrix matuMj = matuBasis;
	CqMatrix matvMj = matvBasis;

	CqMatrix matuConv = matuMj * matMim1;
	CqMatrix matvConv = matvMj * matMim1;

	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = aUserParams().end();
	for ( iUP = aUserParams().begin(); iUP != end; iUP++ )
	{
		if ( ( *iUP ) ->Class() == class_vertex )
		{
			TqInt ptype = (*iUP)->Type();
			switch( ptype )
			{
					case type_point:
					case type_vector:	///! \todo Not sure if this is correct, do vectors and normals need to be treated differently?
					case type_normal:	///! \todo Not sure if this is correct, do vectors and normals need to be treated differently?
					{
						// Get the parameter pointer as the correct type.
						CqParameterTyped<CqVector3D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );

						// Store the data into a matrix for conversion.
						CqMatrix matCPx, matCPy, matCPz, matCPh;
						for ( i = 0; i < 4; i++ )
						{
							for ( j = 0; j < 4; j++ )
							{
								matCPx[ i ][ j ] = pParam->pValue( i*4 + j )[0][0];
								matCPy[ i ][ j ] = pParam->pValue( i*4 + j )[0][1];
								matCPz[ i ][ j ] = pParam->pValue( i*4 + j )[0][2];
								matCPh[ i ][ j ] = 1.0f;
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
								pParam->pValue( i*4 + j )[0][0] = matCPx[ i ][ j ];
								pParam->pValue( i*4 + j )[0][1] = matCPy[ i ][ j ];
								pParam->pValue( i*4 + j )[0][2] = matCPz[ i ][ j ];
							}
						}
					}
					break;

					case type_hpoint:
					{
						// Get the parameter pointer as the correct type.
						CqParameterTyped<CqVector4D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( ( *iUP ) );

						// Store the data into a matrix for conversion.
						CqMatrix matCPx, matCPy, matCPz, matCPh;
						for ( i = 0; i < 4; i++ )
						{
							for ( j = 0; j < 4; j++ )
							{
								matCPx[ i ][ j ] = pParam->pValue( i*4 + j )[0][0];
								matCPy[ i ][ j ] = pParam->pValue( i*4 + j )[0][1];
								matCPz[ i ][ j ] = pParam->pValue( i*4 + j )[0][2];
								matCPh[ i ][ j ] = pParam->pValue( i*4 + j )[0][3];
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
								pParam->pValue( i*4 + j )[0][0] = matCPx[ i ][ j ];
								pParam->pValue( i*4 + j )[0][1] = matCPy[ i ][ j ];
								pParam->pValue( i*4 + j )[0][2] = matCPz[ i ][ j ];
								pParam->pValue( i*4 + j )[0][3] = matCPh[ i ][ j ];
							}
						}
					}
					break;

					case type_color:
					{
						// Get the parameter pointer as the correct type.
						CqParameterTyped<CqColor, CqColor>* pParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( ( *iUP ) );

						// Store the data into a matrix for conversion.
						CqMatrix matRed, matGreen, matBlue;
						for ( i = 0; i < 4; i++ )
						{
							for ( j = 0; j < 4; j++ )
							{
								matRed[ i ][ j ] = pParam->pValue( i*4 + j )[0][0];
								matGreen[ i ][ j ] = pParam->pValue( i*4 + j )[0][1];
								matBlue[ i ][ j ] = pParam->pValue( i*4 + j )[0][2];
							}
						}
						matRed.SetfIdentity( TqFalse );
						matGreen.SetfIdentity( TqFalse );
						matBlue.SetfIdentity( TqFalse );

						matRed = matuConv.Transpose() * matRed * matvConv;
						matGreen = matuConv.Transpose() * matGreen * matvConv;
						matBlue = matuConv.Transpose() * matBlue * matvConv;

						for ( i = 0; i < 4; i++ )
						{
							for ( j = 0; j < 4; j++ )
							{
								pParam->pValue( i*4 + j )[0][0] = matRed[ i ][ j ];
								pParam->pValue( i*4 + j )[0][1] = matGreen[ i ][ j ];
								pParam->pValue( i*4 + j )[0][2] = matBlue[ i ][ j ];
							}
						}
					}
					break;

					case type_float:
					{
						// Get the parameter pointer as the correct type.
						CqParameterTyped<TqFloat, TqFloat>* pParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( ( *iUP ) );

						// Store the data into a matrix for conversion.
						CqMatrix matCPx;
						for ( i = 0; i < 4; i++ )
						{
							for ( j = 0; j < 4; j++ )
								matCPx[ i ][ j ] = pParam->pValue( i*4 + j )[0];
						}
						matCPx.SetfIdentity( TqFalse );
						matCPx = matuConv.Transpose() * matCPx * matvConv;

						for ( i = 0; i < 4; i++ )
						{
							for ( j = 0; j < 4; j++ )
								pParam->pValue( i*4 + j )[0] = matCPx[ i ][ j ];
						}
					}
					break;

					/// \todo Need to work out how to convert Matrix types to Bezier as well at some point.
			}
		}
	}

	/*	for ( i = 0; i < 4; i++ )
		{
			for ( j = 0; j < 4; j++ )
			{
				CP( i, j ).x( matCPx[ i ][ j ] );
				CP( i, j ).y( matCPy[ i ][ j ] );
				CP( i, j ).z( matCPz[ i ][ j ] );
				CP( i, j ).h( matCPh[ i ][ j ] );
			}
		}*/
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqSurfacePatchBilinear::CqSurfacePatchBilinear() : CqSurface(), m_fHasPhantomFourthVertex( TqFalse ), m_iInternalu( -1 ), m_iInternalv( -1 )
{}


//---------------------------------------------------------------------
/** Copy constructor.
 */

/* CqSurfacePatchBilinear::CqSurfacePatchBilinear( const CqSurfacePatchBilinear& From ) :
 * 		CqSurface( From )
 * {
 * 	*this = From;
 * }
 */


//---------------------------------------------------------------------
/** Destructor.
 */

CqSurfacePatchBilinear::~CqSurfacePatchBilinear()
{}


//---------------------------------------------------------------------
/** Create a clone of this patch surface.
 */

CqSurface* CqSurfacePatchBilinear::Clone() const
{
	CqSurfacePatchBilinear* clone = new CqSurfacePatchBilinear();
	CqSurface::CloneData( clone );

	clone->m_fHasPhantomFourthVertex = m_fHasPhantomFourthVertex;
	clone->m_iInternalu = m_iInternalu;
	clone->m_iInternalv = m_iInternalv;

	return ( clone );
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
	for ( i = 0; i < ( m_fHasPhantomFourthVertex ? 3 : 4 ); i++ )
	{
		CqVector3D	vecV = P()->pValue( i )[0];
		if ( vecV.x() < vecA.x() )
			vecA.x( vecV.x() );
		if ( vecV.y() < vecA.y() )
			vecA.y( vecV.y() );
		if ( vecV.x() > vecB.x() )
			vecB.x( vecV.x() );
		if ( vecV.y() > vecB.y() )
			vecB.y( vecV.y() );
		if ( vecV.z() < vecA.z() )
			vecA.z( vecV.z() );
		if ( vecV.z() > vecB.z() )
			vecB.z( vecV.z() );
	}
	CqBound	B;
	B.vecMin() = vecA;
	B.vecMax() = vecB;
	return ( AdjustBoundForTransformationMotion( B ) );
}



//---------------------------------------------------------------------
/** Split the patch into smaller patches.
 */

TqInt CqSurfacePatchBilinear::PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, TqBool u )
{
	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqSurfacePatchBilinear ) );
	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqSurfacePatchBilinear ) );

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
	const CqMatrix & matCtoR = QGetRenderContext() ->matSpaceToSpace( "camera", "raster", NULL, NULL, QGetRenderContext()->Time() );

	// Convert the control hull to raster space.
	CqVector2D	avecHull[ 4 ];
	TqInt i;

	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ];

	for ( i = 0; i < 4; i++ )
		avecHull[ i ] = matCtoR * P()->pValue( i )[0];

	TqFloat uLen = 0;
	TqFloat vLen = 0;

	CqVector2D	Vec1 = avecHull[ 1 ] - avecHull[ 0 ];
	CqVector2D	Vec2 = avecHull[ 3 ] - avecHull[ 2 ];
	uLen = ( Vec1.Magnitude2() > Vec2.Magnitude2() ) ? Vec1.Magnitude2() : Vec2.Magnitude2();

	Vec1 = avecHull[ 2 ] - avecHull[ 0 ];
	Vec2 = avecHull[ 3 ] - avecHull[ 1 ];
	vLen = ( Vec1.Magnitude2() > Vec2.Magnitude2() ) ? Vec1.Magnitude2() : Vec2.Magnitude2();

	uLen = sqrt( uLen / ShadingRate);
	vLen = sqrt( vLen / ShadingRate);

	m_SplitDir = ( uLen > vLen ) ? SplitDir_U : SplitDir_V;

	// TODO: Should ensure powers of half to prevent cracking.
	uLen = MAX( ROUND( uLen ), 1 );
	vLen = MAX( ROUND( vLen ), 1 );

	m_uDiceSize = static_cast<TqInt>( uLen );
	m_vDiceSize = static_cast<TqInt>( vLen );

	// Ensure power of 2 to avoid cracking
	const TqInt *binary = pAttributes() ->GetIntegerAttribute( "dice", "binary" );
	if ( binary && *binary)
	{
		m_uDiceSize = CEIL_POW2( m_uDiceSize );
		m_vDiceSize = CEIL_POW2( m_vDiceSize );
	}

	if ( uLen < FLT_EPSILON || vLen < FLT_EPSILON )
	{
		m_fDiscard = TqTrue;
		return ( TqFalse );
	}

	TqFloat gs = 16.0f;
	const TqFloat* poptGridSize = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "SqrtGridSize" );
	if( NULL != poptGridSize )
		gs = poptGridSize[0];

	TqFloat gs2 = gs*gs;
	if( m_uDiceSize > gs2 || m_vDiceSize > gs2 || (m_uDiceSize * m_vDiceSize) > gs2 )
		return TqFalse;

	return ( TqTrue );
}



/** CqSurfacePatchBilinear::Split
 *  Split the patch into 2 or 3 new patches depending on whether the patch has a phantom
 *  fourth vertex or not. If not, then the patch is split into to in the chose u or v direction.
 *  If it has a phantom fourth vertex, it is split in both u and v, and the patch corresponding
 *  to the phantom corner is discarded. The new patch opposite the phantom vertex in the original
 *  is no longer a phantom patch, the other two are.
 *
 */
TqInt CqSurfacePatchBilinear::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{
	// Create two new patches
	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqSurfacePatchBilinear ) );
	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqSurfacePatchBilinear ) );

	// If phantom, create a further two.
	/// \note: We can actually avoid this, as we only really need three for a phantom patch.
	if ( m_fHasPhantomFourthVertex )
	{
		aSplits.push_back( boost::shared_ptr<CqSurface>( new CqSurfacePatchBilinear ) );
		aSplits.push_back( boost::shared_ptr<CqSurface>( new CqSurfacePatchBilinear ) );
	}
	TqBool direction = SplitDir() == SplitDir_U;
	TqBool opposite = !direction;
	TqInt i;
	// Fill in the standard data from this donor to the new patches.
	for ( i = 0; i < ( m_fHasPhantomFourthVertex ? 4 : 2 ); i++ )
	{
		aSplits[ i ] ->SetSurfaceParameters( *this );
		aSplits[ i ] ->SetSplitDir( direction ? SplitDir_V : SplitDir_U );
		aSplits[ i ] ->SetEyeSplitCount( EyeSplitCount() );
		aSplits[ i ] ->m_fDiceable = TqTrue;
	}

	// Iterate through any use parameters subdividing and storing the second value in the target surface.
	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = m_aUserParams.end();

	for ( iUP = m_aUserParams.begin(); iUP != end; iUP++ )
	{
		// Clone the parameter and subdivide it in the chosen direction.
		CqParameter* pNewA = ( *iUP ) ->Clone();
		CqParameter* pNewB = ( *iUP ) ->Clone();
		( *iUP ) ->Subdivide( pNewA, pNewB, direction, this );

		if ( m_fHasPhantomFourthVertex )
		{
			// If phantom, clone the two new parameters, and subdivide in the other direction.
			CqParameter * pNewC = pNewA ->Clone();
			CqParameter* pNewD = pNewA ->Clone();
			CqParameter* pNewE = pNewB ->Clone();
			CqParameter* pNewF = pNewB ->Clone();
			pNewA ->Subdivide( pNewC, pNewD, opposite, this );
			pNewB ->Subdivide( pNewE, pNewF, opposite, this );

			static_cast<CqSurface*>( aSplits[ 0 ].get() ) ->AddPrimitiveVariable( pNewC );
			static_cast<CqSurface*>( aSplits[ 1 ].get() ) ->AddPrimitiveVariable( pNewD );
			static_cast<CqSurface*>( aSplits[ 2 ].get() ) ->AddPrimitiveVariable( pNewE );
			static_cast<CqSurface*>( aSplits[ 3 ].get() ) ->AddPrimitiveVariable( pNewF );

			delete( pNewA );
			delete( pNewB );
		}
		else
		{
			static_cast<CqSurface*>( aSplits[ 0 ].get() ) ->AddPrimitiveVariable( pNewA );
			static_cast<CqSurface*>( aSplits[ 1 ].get() ) ->AddPrimitiveVariable( pNewB );
		}
	}

	if ( m_fHasPhantomFourthVertex )
	{
		// If phantom, we can discard the new patch at the phantom vertex.
		aSplits.pop_back();

		// And set the phantom status of the remaining 3 patches.
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 0 ].get() ) ->m_fHasPhantomFourthVertex = TqFalse;
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 1 ].get() ) ->m_fHasPhantomFourthVertex = TqTrue;
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 2 ].get() ) ->m_fHasPhantomFourthVertex = TqTrue;

		return ( 3 );
	}
	else
	{
		// If not phantom, just return the two new halves.
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 0 ].get() ) ->m_fHasPhantomFourthVertex = TqFalse;
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 1 ].get() ) ->m_fHasPhantomFourthVertex = TqFalse;

		return ( 2 );
	}
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqSurfacePatchMeshBicubic::~CqSurfacePatchMeshBicubic()
{}


//---------------------------------------------------------------------
/** Create a clone of this patchmesh surface.
 */

CqSurface* CqSurfacePatchMeshBicubic::Clone() const
{
	CqSurfacePatchMeshBicubic* clone = new CqSurfacePatchMeshBicubic();
	CqSurface::CloneData( clone );

	clone->m_uPatches = m_uPatches;
	clone->m_vPatches = m_vPatches;
	clone->m_nu = m_nu;
	clone->m_nv = m_nv;
	clone->m_uPeriodic = m_uPeriodic;
	clone->m_vPeriodic = m_vPeriodic;

	return ( clone );
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
		CqVector3D	vecV = P()->pValue( i )[0];
		if ( vecV.x() < vecA.x() )
			vecA.x( vecV.x() );
		if ( vecV.y() < vecA.y() )
			vecA.y( vecV.y() );
		if ( vecV.x() > vecB.x() )
			vecB.x( vecV.x() );
		if ( vecV.y() > vecB.y() )
			vecB.y( vecV.y() );
		if ( vecV.z() < vecA.z() )
			vecA.z( vecV.z() );
		if ( vecV.z() > vecB.z() )
			vecB.z( vecV.z() );
	}
	CqBound	B;
	B.vecMin() = vecA;
	B.vecMax() = vecB;
	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split the patch mesh into individual patches and post them.
 */

#define	PatchCoord(v,u)	((((v)%m_nv)*m_nu)+((u)%m_nu))
#define	PatchCorner(v,u)	((((v)%nvaryingv)*nvaryingu)+((u)%nvaryingu));

TqInt CqSurfacePatchMeshBicubic::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
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
			boost::shared_ptr<CqSurfacePatchBicubic> pSurface( new CqSurfacePatchBicubic() );
			pSurface->SetSurfaceParameters( *this );

			RtInt v;

			TqInt iTa = PatchCorner( i, j );
			TqInt iTb = PatchCorner( i, j + 1 );
			TqInt iTc = PatchCorner( i + 1, j );
			TqInt iTd = PatchCorner( i + 1, j + 1 );

			TqFloat u0 = ( 1.0f / m_uPatches ) * j;
			TqFloat u1 = ( 1.0f / m_uPatches ) * ( j + 1 );

			std::vector<CqParameter*>::iterator iUP;
			std::vector<CqParameter*>::iterator end = aUserParams().end();
			for ( iUP = aUserParams().begin(); iUP != end; iUP++ )
			{
				if ( ( *iUP ) ->Class() == class_varying )
				{
					// Copy any 'varying' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName(), ( *iUP ) ->Count() );
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
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName(), ( *iUP ) ->Count() );
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
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName(), ( *iUP ) ->Count() );
					pNewUP->SetSize( pSurface->cUniform() );
					pNewUP->SetValue( ( *iUP ), 0, j );
					pSurface->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_constant )
				{
					// Copy any 'constant' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName(), ( *iUP ) ->Count() );
					pNewUP->SetSize( 1 );
					pNewUP->SetValue( ( *iUP ), 0, 0 );
					pSurface->AddPrimitiveVariable( pNewUP );
				}
			}

			// If the shaders need u/v or s/t and they are not specified, then we need to put them in as defaults.
			if ( USES( MyUses, EnvVars_u ) && !bHasVar(EnvVars_u) )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "u" ) );
				pSurface->u() ->SetSize( 4 );
				pSurface->u() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u0, v0 );
				pSurface->u() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u1, v0 );
				pSurface->u() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u0, v1 );
				pSurface->u() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u1, v1 );
			}

			if ( USES( MyUses, EnvVars_v ) && !bHasVar(EnvVars_v) )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "v" ) );
				pSurface->v() ->SetSize( 4 );
				pSurface->v() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u0, v0 );
				pSurface->v() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u1, v0 );
				pSurface->v() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u0, v1 );
				pSurface->v() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u1, v1 );
			}

			if ( USES( MyUses, EnvVars_s ) && !bHasVar(EnvVars_s) )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "s" ) );
				pSurface->s() ->SetSize( 4 );
				pSurface->s() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u0, v0 );
				pSurface->s() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u1, v0 );
				pSurface->s() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u0, v1 );
				pSurface->s() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u1, v1 );
			}

			if ( USES( MyUses, EnvVars_t ) && !bHasVar(EnvVars_t) )
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

/* CqSurfacePatchMeshBilinear::CqSurfacePatchMeshBilinear( const CqSurfacePatchMeshBilinear& From ) :
 * 		CqSurface( From )
 * {
 * 	*this = From;
 * }
 * 
 */

//---------------------------------------------------------------------
/** Destructor.
 */

CqSurfacePatchMeshBilinear::~CqSurfacePatchMeshBilinear()
{}


//---------------------------------------------------------------------
/** Create a clone of this patchmesh surface.
 */

CqSurface* CqSurfacePatchMeshBilinear::Clone() const
{
	CqSurfacePatchMeshBilinear* clone = new CqSurfacePatchMeshBilinear();
	CqSurface::CloneData( clone );

	clone->m_uPatches = m_uPatches;
	clone->m_vPatches = m_vPatches;
	clone->m_nu = m_nu;
	clone->m_nv = m_nv;
	clone->m_uPeriodic = m_uPeriodic;
	clone->m_vPeriodic = m_vPeriodic;

	return ( clone );
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
		CqVector3D	vecV = P()->pValue( i )[0];
		if ( vecV.x() < vecA.x() )
			vecA.x( vecV.x() );
		if ( vecV.y() < vecA.y() )
			vecA.y( vecV.y() );
		if ( vecV.x() > vecB.x() )
			vecB.x( vecV.x() );
		if ( vecV.y() > vecB.y() )
			vecB.y( vecV.y() );
		if ( vecV.z() < vecA.z() )
			vecA.z( vecV.z() );
		if ( vecV.z() > vecB.z() )
			vecB.z( vecV.z() );
	}
	CqBound	B;
	B.vecMin() = vecA;
	B.vecMax() = vecB;
	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split the patch mesh into individual patches and post them.
 */

#define	PatchCoord(v,u)	((((v)%m_nv)*m_nu)+((u)%m_nu))
#define	PatchCorner(v,u)	((((v)%nvaryingv)*nvaryingu)+((u)%nvaryingu));

TqInt CqSurfacePatchMeshBilinear::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
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
			boost::shared_ptr<CqSurfacePatchBilinear> pSurface( new CqSurfacePatchBilinear );
			pSurface->SetSurfaceParameters( *this );

			RtInt iTa = PatchCoord( i, j );
			RtInt iTb = PatchCoord( i, j + 1 );
			RtInt iTc = PatchCoord( i + 1, j );
			RtInt iTd = PatchCoord( i + 1, j + 1 );

			TqFloat u0 = ( 1.0f / m_uPatches ) * j;
			TqFloat u1 = ( 1.0f / m_uPatches ) * ( j + 1 );

			// Copy any primitive variables.
			std::vector<CqParameter*>::iterator iUP;
			std::vector<CqParameter*>::iterator end = aUserParams().end();
			for ( iUP = aUserParams().begin(); iUP != end; iUP++ )
			{
				if ( ( *iUP ) ->Class() == class_varying )
				{
					// Copy any 'varying' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName(), ( *iUP ) ->Count() );
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
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName(), ( *iUP ) ->Count() );
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
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName(), ( *iUP ) ->Count() );
					pNewUP->SetSize( pSurface->cUniform() );
					pNewUP->SetValue( ( *iUP ), 0, j );
					pSurface->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_constant )
				{
					// Copy any 'constant' class primitive variables.
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName(), ( *iUP ) ->Count() );
					pNewUP->SetSize( 1 );

					pNewUP->SetValue( ( *iUP ), 0, 0 );
					pSurface->AddPrimitiveVariable( pNewUP );
				}
			}

			// If the shaders need u/v or s/t and they are not specified, then we need to put them in as defaults.
			if ( USES( MyUses, EnvVars_u ) && !bHasVar(EnvVars_u) )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "u" ) );
				pSurface->u() ->SetSize( 4 );
				pSurface->u() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u0, v0 );
				pSurface->u() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u1, v0 );
				pSurface->u() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u0, v1 );
				pSurface->u() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u1, v1 );
			}

			if ( USES( MyUses, EnvVars_v ) && !bHasVar(EnvVars_v) )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "v" ) );
				pSurface->v() ->SetSize( 4 );
				pSurface->v() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u0, v0 );
				pSurface->v() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u1, v0 );
				pSurface->v() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u0, v1 );
				pSurface->v() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u1, v1 );
			}

			if ( USES( MyUses, EnvVars_s ) && !bHasVar(EnvVars_s) )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "s" ) );
				pSurface->s() ->SetSize( 4 );
				pSurface->s() ->pValue( 0 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u0, v0 );
				pSurface->s() ->pValue( 1 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u1, v0 );
				pSurface->s() ->pValue( 2 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u0, v1 );
				pSurface->s() ->pValue( 3 ) [ 0 ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u1, v1 );
			}

			if ( USES( MyUses, EnvVars_t ) && !bHasVar(EnvVars_t) )
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
