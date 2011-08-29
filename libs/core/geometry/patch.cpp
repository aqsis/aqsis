
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
		\brief Implements the classes and support structures for handling RenderMan patch primitives.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"patch.h"

#include	"forwarddiff.h"
#include	"imagebuffer.h"
#include	"micropolygon.h"
#include	"renderer.h"
#include	<aqsis/math/vector2d.h>
#include	<aqsis/math/math.h>

namespace Aqsis {

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


namespace {

/** \brief Implementation of natural subdivision for bicubic patches.
 */
template <class T, class SLT>
void bicubicPatchNatSubdiv(CqParameter* pParam, CqParameter* pResult1,
		CqParameter* pResult2, bool u)
{
	TqInt iu, iv;

	CqParameterTyped<T, SLT>* pTParam = static_cast<CqParameterTyped<T, SLT>*>( pParam );
	CqParameterTyped<T, SLT>* pTResult1 = static_cast<CqParameterTyped<T, SLT>*>( pResult1 );
	CqParameterTyped<T, SLT>* pTResult2 = static_cast<CqParameterTyped<T, SLT>*>( pResult2 );
	if ( u )
	{
		for ( iv = 0; iv < 4; iv++ )
		{
			TqUint ivo = ( iv * 4 );
			pTResult1->pValue() [ ivo + 0 ] = pTParam->pValue() [ ivo + 0 ];
			pTResult1->pValue() [ ivo + 1 ] = static_cast<T>( ( pTParam->pValue() [ ivo + 0 ] + pTParam->pValue() [ ivo + 1 ] ) / 2.0f );
			pTResult1->pValue() [ ivo + 2 ] = static_cast<T>( pTResult1->pValue() [ ivo + 1 ] / 2.0f + ( pTParam->pValue() [ ivo + 1 ] + pTParam->pValue() [ ivo + 2 ] ) / 4.0f );

			pTResult2->pValue() [ ivo + 3 ] = pTParam->pValue() [ ivo + 3 ];
			pTResult2->pValue() [ ivo + 2 ] = static_cast<T>( ( pTParam->pValue() [ ivo + 2 ] + pTParam->pValue() [ ivo + 3 ] ) / 2.0f );
			pTResult2->pValue() [ ivo + 1 ] = static_cast<T>( pTResult2->pValue() [ ivo + 2 ] / 2.0f + ( pTParam->pValue() [ ivo + 1 ] + pTParam->pValue() [ ivo + 2 ] ) / 4.0f );

			pTResult1->pValue() [ ivo + 3 ] = static_cast<T>( ( pTResult1->pValue() [ ivo + 2 ] + pTResult2->pValue() [ ivo + 1 ] ) / 2.0f );
			pTResult2->pValue() [ ivo + 0 ] = pTResult1->pValue() [ ivo + 3 ];
		}
	}
	else
	{
		for ( iu = 0; iu < 4; iu++ )
		{
			pTResult1->pValue() [ 0 + iu ] = pTParam->pValue() [ 0 + iu ];
			pTResult1->pValue() [ 4 + iu ] = static_cast<T>( ( pTParam->pValue() [ 0 + iu ] + pTParam->pValue() [ 4 + iu ] ) / 2.0f );
			pTResult1->pValue() [ 8 + iu ] = static_cast<T>( pTResult1->pValue() [ 4 + iu ] / 2.0f + ( pTParam->pValue() [ 4 + iu ] + pTParam->pValue() [ 8 + iu ] ) / 4.0f );

			pTResult2->pValue() [ 12 + iu ] = pTParam->pValue() [ 12 + iu ];
			pTResult2->pValue() [ 8 + iu ] = static_cast<T>( ( pTParam->pValue() [ 8 + iu ] + pTParam->pValue() [ 12 + iu ] ) / 2.0f );
			pTResult2->pValue() [ 4 + iu ] = static_cast<T>( pTResult2->pValue() [ 8 + iu ] / 2.0f + ( pTParam->pValue() [ 4 + iu ] + pTParam->pValue() [ 8 + iu ] ) / 4.0f );

			pTResult1->pValue() [ 12 + iu ] = static_cast<T>( ( pTResult1->pValue() [ 8 + iu ] + pTResult2->pValue() [ 4 + iu ] ) / 2.0f );
			pTResult2->pValue() [ 0 + iu ] = pTResult1->pValue() [ 12 + iu ];
		}
	}
}

} // unnamed namespace

void CqSurfacePatchBicubic::NaturalSubdivide( CqParameter* pParam, CqParameter* pParam1, CqParameter* pParam2, bool u )
{
	switch ( pParam->Type() )
	{
		case type_float:
			bicubicPatchNatSubdiv<TqFloat, TqFloat>(pParam, pParam1, pParam2, u);
			break;
		case type_integer:
			bicubicPatchNatSubdiv<TqInt, TqFloat>(pParam, pParam1, pParam2, u);
			break;
		case type_point:
		case type_vector:
		case type_normal:
			bicubicPatchNatSubdiv<CqVector3D, CqVector3D>(pParam, pParam1, pParam2, u);
			break;
		case type_hpoint:
			bicubicPatchNatSubdiv<CqVector4D, CqVector3D>(pParam, pParam1, pParam2, u);
			break;
		case type_color:
			bicubicPatchNatSubdiv<CqColor, CqColor>(pParam, pParam1, pParam2, u);
			break;
		case type_string:
			bicubicPatchNatSubdiv<CqString, CqString>(pParam, pParam1, pParam2, u);
			break;
		case type_matrix:
			/// \todo why is this commented out?
			//bicubicPatchNatSubdiv<CqMatrix, CqMatrix>(pParam, pParam1, pParam2, u);
			//break;
		default:
			// left blank to avoid compiler warnings about unhandled types
			break;
	}
}


//---------------------------------------------------------------------
/** Get the boundary extents in camera space of the surface patch
 */

void CqSurfacePatchBicubic::Bound(CqBound* bound) const
{
	// Get the boundary in camera space.
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqInt i;
	for ( i = 0; i < 16; i++ )
	{
		CqVector3D	vecV = vectorCast<CqVector3D>(P()->pValue( i )[0]);
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
	bound->vecMin() = vecA;
	bound->vecMax() = vecB;
	AdjustBoundForTransformationMotion( bound );
}


//---------------------------------------------------------------------
namespace {

/** \brief Implementation of dicing for bicubic patches.
 */
template <class T, class SLT>
void bicubicPatchNatDice(TqFloat uSize, TqFloat vSize, CqParameter* pParam,
		IqShaderData* pData)
{
	CqParameterTyped<T, SLT>* pTParam = static_cast<CqParameterTyped<T, SLT>*>(pParam);
	CqForwardDiffBezier<T> vFD0( 1.0f / vSize );
	CqForwardDiffBezier<T> vFD1( 1.0f / vSize );
	CqForwardDiffBezier<T> vFD2( 1.0f / vSize );
	CqForwardDiffBezier<T> vFD3( 1.0f / vSize );
	CqForwardDiffBezier<T> uFD0( 1.0f / uSize );

	IqShaderData* arrayValue;
	TqInt i;
	for(i = 0; i<pTParam->Count(); i++)
	{
		vFD0.CalcForwardDiff( pTParam->pValue(0) [ i ], pTParam->pValue(4) [ i ], pTParam->pValue(8) [ i ], pTParam->pValue(12) [ i ] );
		vFD1.CalcForwardDiff( pTParam->pValue(1) [ i ], pTParam->pValue(5) [ i ], pTParam->pValue(9) [ i ], pTParam->pValue(13) [ i ] );
		vFD2.CalcForwardDiff( pTParam->pValue(2) [ i ], pTParam->pValue(6) [ i ], pTParam->pValue(10) [ i ], pTParam->pValue(14) [ i ] );
		vFD3.CalcForwardDiff( pTParam->pValue(3) [ i ], pTParam->pValue(7) [ i ], pTParam->pValue(11) [ i ], pTParam->pValue(15) [ i ] );

		TqInt iv, iu;
		for ( iv = 0; iv <= vSize; iv++ )
		{
			T vA = vFD0.GetValue();
			T vB = vFD1.GetValue();
			T vC = vFD2.GetValue();
			T vD = vFD3.GetValue();
			uFD0.CalcForwardDiff( vA, vB, vC, vD );

			for ( iu = 0; iu <= uSize; iu++ )
			{
				T vec = uFD0.GetValue();
				TqInt igrid = static_cast<TqInt>( ( iv * ( uSize + 1 ) ) + iu );
				arrayValue = pData->ArrayEntry(i);
				arrayValue->SetValue( paramToShaderType<SLT, T>(vec), igrid );
			}
		}
	}
}

} // unnamed namespace

/** Dice the patch into a mesh of micropolygons.
 */
void CqSurfacePatchBicubic::NaturalDice(CqParameter* pParam, TqInt uDiceSize,
		TqInt vDiceSize, IqShaderData* pData)
{
	switch(pParam->Type())
	{
		case type_float:
			bicubicPatchNatDice<TqFloat, TqFloat>(uDiceSize, vDiceSize, pParam, pData);
			break;
		case type_integer:
			bicubicPatchNatDice<TqInt, TqFloat>( uDiceSize, vDiceSize, pParam, pData );
			break;
		case type_point:
		case type_vector:
		case type_normal:
			bicubicPatchNatDice<CqVector3D, CqVector3D>( uDiceSize, vDiceSize, pParam, pData );
			break;
		case type_hpoint:
			bicubicPatchNatDice<CqVector4D, CqVector3D>(uDiceSize, vDiceSize, pParam, pData);
			break;
		case type_color:
			bicubicPatchNatDice<CqColor, CqColor>(uDiceSize, vDiceSize, pParam, pData);
			break;
		case type_string:
			bicubicPatchNatDice<CqString, CqString>(uDiceSize, vDiceSize, pParam, pData);
			break;
		case type_matrix:
			bicubicPatchNatDice<CqMatrix, CqMatrix>( uDiceSize, vDiceSize, pParam, pData );
			break;
		default:
			// left blank to avoid compiler warnings about unhandled types
			break;
	}
}

//---------------------------------------------------------------------
/** Split the patch into smaller patches.
 */

TqInt CqSurfacePatchBicubic::PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u )
{
	// Create two new surface of the appropriate type
	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqSurfacePatchBicubic ) );
	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqSurfacePatchBicubic ) );

	return ( 2 );
}

//---------------------------------------------------------------------
/** Determine whether or not the patch is diceable
 */

bool	CqSurfacePatchBicubic::Diceable(const CqMatrix& matCtoR)
{
	assert( NULL != P() );
	// If the cull check showed that the primitive cannot be diced due to crossing the e and hither planes,
	// then we can return immediately.
	if ( !m_fDiceable )
		return ( false );

	// Otherwise we should continue to try to find the most advantageous split direction, OR the dice size.

	// Convert the control hull to raster space.
	CqVector3D	avecHull[ 16 ];

	for (TqInt i = 0; i < 16; i++ )
		avecHull[i] = vectorCast<CqVector3D>(matCtoR * P()->pValue(i)[0]);

	TqFloat uLen = 0;
	TqFloat vLen = 0;

	for (TqInt u = 0; u < 16; u += 4 )
	{
		CqVector3D	Vec1 = avecHull[ u + 1 ] - avecHull[ u ];
		CqVector3D	Vec2 = avecHull[ u + 2 ] - avecHull[ u + 1 ];
		CqVector3D	Vec3 = avecHull[ u + 3 ] - avecHull[ u + 2 ];
		if ( Vec1.Magnitude2() > uLen )
			uLen = Vec1.Magnitude2();
		if ( Vec2.Magnitude2() > uLen )
			uLen = Vec2.Magnitude2();
		if ( Vec3.Magnitude2() > uLen )
			uLen = Vec3.Magnitude2();
	}
	for (TqInt v = 0; v < 4; v++ )
	{
		CqVector3D	Vec1 = avecHull[ v + 4 ] - avecHull[ v ];
		CqVector3D	Vec2 = avecHull[ v + 8 ] - avecHull[ v + 4 ];
		CqVector3D	Vec3 = avecHull[ v + 12 ] - avecHull[ v + 8 ];
		if ( Vec1.Magnitude2() > vLen )
			vLen = Vec1.Magnitude2();
		if ( Vec2.Magnitude2() > vLen )
			vLen = Vec2.Magnitude2();
		if ( Vec3.Magnitude2() > vLen )
			vLen = Vec3.Magnitude2();
	}

	TqFloat shadingRate = AdjustedShadingRate();
	uLen = sqrt(uLen/shadingRate);
	vLen = sqrt(vLen/shadingRate);

	m_SplitDir = ( uLen > vLen ) ? SplitDir_U : SplitDir_V;
	// TODO: Should ensure powers of half to prevent cracking.
	uLen *= 3;
	vLen *= 3;
	m_uDiceSize = max<TqInt>(lround( uLen ), 1);
	m_vDiceSize = max<TqInt>(lround( vLen ), 1);

	// Ensure power of 2 to avoid cracking
	const TqInt *binary = pAttributes() ->GetIntegerAttribute( "dice", "binary" );
	if ( binary && *binary)
	{
		m_uDiceSize = ceilPow2( m_uDiceSize );
		m_vDiceSize = ceilPow2( m_vDiceSize );
	}

	if ( uLen < FLT_EPSILON || vLen < FLT_EPSILON )
	{
		m_fDiscard = true;
		return ( false );
	}

	TqFloat gs = 16.0f;
	const TqFloat* poptGridSize = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "SqrtGridSize" );
	if( NULL != poptGridSize )
		gs = poptGridSize[0];

	TqFloat gs2 = gs*gs;
	if( m_uDiceSize > gs2 || m_vDiceSize > gs2 || (m_uDiceSize * m_vDiceSize) > gs2 )
		return false;

	return ( true );
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
		matMim1.SetfIdentity( false );
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
						matCPx.SetfIdentity( false );
						matCPy.SetfIdentity( false );
						matCPz.SetfIdentity( false );
						matCPh.SetfIdentity( false );

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
						matCPx.SetfIdentity( false );
						matCPy.SetfIdentity( false );
						matCPz.SetfIdentity( false );
						matCPh.SetfIdentity( false );

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
						matRed.SetfIdentity( false );
						matGreen.SetfIdentity( false );
						matBlue.SetfIdentity( false );

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
						matCPx.SetfIdentity( false );
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

CqSurfacePatchBilinear::CqSurfacePatchBilinear() : CqSurface(), m_fHasPhantomFourthVertex( false ), m_iInternalu( -1 ), m_iInternalv( -1 )
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

void CqSurfacePatchBilinear::Bound(CqBound* bound) const
{
	assert( NULL != P() );

	// Get the boundary in camera space.
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqInt i;
	for ( i = 0; i < ( m_fHasPhantomFourthVertex ? 3 : 4 ); i++ )
	{
		CqVector3D	vecV = vectorCast<CqVector3D>(P()->pValue( i )[0]);
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
	bound->vecMin() = vecA;
	bound->vecMax() = vecB;
	AdjustBoundForTransformationMotion( bound );
}



//---------------------------------------------------------------------
/** Split the patch into smaller patches.
 */

TqInt CqSurfacePatchBilinear::PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u )
{
	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqSurfacePatchBilinear ) );
	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqSurfacePatchBilinear ) );

	return ( 2 );
}


//---------------------------------------------------------------------
/** Determine whether or not the patch is diceable
 */

bool	CqSurfacePatchBilinear::Diceable(const CqMatrix& matCtoR)
{
	assert( NULL != P() );

	// If the cull check showed that the primitive cannot be diced due to crossing the e and hither planes,
	// then we can return immediately.
	if ( !m_fDiceable )
		return ( false );

	// Otherwise we should continue to try to find the most advantageous split direction, OR the dice size.

	// Convert the control hull to raster space.
	CqVector3D	avecHull[ 4 ];
	TqInt i;

	for ( i = 0; i < 4; i++ )
		avecHull[i] = vectorCast<CqVector3D>(matCtoR * P()->pValue(i)[0]);

	TqFloat uLen = 0;
	TqFloat vLen = 0;

	CqVector3D	Vec1 = avecHull[ 1 ] - avecHull[ 0 ];
	CqVector3D	Vec2 = avecHull[ 3 ] - avecHull[ 2 ];
	uLen = ( Vec1.Magnitude2() > Vec2.Magnitude2() ) ? Vec1.Magnitude2() : Vec2.Magnitude2();

	Vec1 = avecHull[ 2 ] - avecHull[ 0 ];
	Vec2 = avecHull[ 3 ] - avecHull[ 1 ];
	vLen = ( Vec1.Magnitude2() > Vec2.Magnitude2() ) ? Vec1.Magnitude2() : Vec2.Magnitude2();

	TqFloat shadingRate = AdjustedShadingRate();
	uLen = sqrt(uLen/shadingRate);
	vLen = sqrt(vLen/shadingRate);

	m_SplitDir = ( uLen > vLen ) ? SplitDir_U : SplitDir_V;

	uLen = max<TqInt>(lround(uLen), 1);
	vLen = max<TqInt>(lround(vLen), 1);

	m_uDiceSize = static_cast<TqInt>( uLen );
	m_vDiceSize = static_cast<TqInt>( vLen );

	// Ensure power of 2 to avoid cracking
	const TqInt *binary = pAttributes() ->GetIntegerAttribute( "dice", "binary" );
	if ( binary && *binary)
	{
		m_uDiceSize = ceilPow2( m_uDiceSize );
		m_vDiceSize = ceilPow2( m_vDiceSize );
	}

	if ( uLen < FLT_EPSILON || vLen < FLT_EPSILON )
	{
		m_fDiscard = true;
		return ( false );
	}

	TqFloat gs = 16.0f;
	const TqFloat* poptGridSize = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "SqrtGridSize" );
	if( NULL != poptGridSize )
		gs = poptGridSize[0];

	TqFloat gs2 = gs*gs;
	if( m_uDiceSize > gs2 || m_vDiceSize > gs2 || (m_uDiceSize * m_vDiceSize) > gs2 )
		return false;

	return ( true );
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
	bool direction = SplitDir() == SplitDir_U;
	bool opposite = !direction;
	TqInt i;
	// Fill in the standard data from this donor to the new patches.
	for ( i = 0; i < ( m_fHasPhantomFourthVertex ? 4 : 2 ); i++ )
	{
		aSplits[ i ] ->SetSurfaceParameters( *this );
		aSplits[ i ] ->SetSplitDir( direction ? SplitDir_V : SplitDir_U );
		aSplits[ i ] ->SetSplitCount( SplitCount() + 1 );
		aSplits[ i ] ->m_fDiceable = true;
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
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 0 ].get() ) ->m_fHasPhantomFourthVertex = false;
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 1 ].get() ) ->m_fHasPhantomFourthVertex = true;
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 2 ].get() ) ->m_fHasPhantomFourthVertex = true;

		return ( 3 );
	}
	else
	{
		// If not phantom, just return the two new halves.
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 0 ].get() ) ->m_fHasPhantomFourthVertex = false;
		static_cast<CqSurfacePatchBilinear*>( aSplits[ 1 ].get() ) ->m_fHasPhantomFourthVertex = false;

		return ( 2 );
	}
}

void CqSurfacePatchBilinear::PostDice(CqMicroPolyGrid * pGrid)
{
	if(m_fHasPhantomFourthVertex)
		pGrid->SetfTriangular(true);
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

	clone->m_patches.insert(clone->m_patches.begin(), m_patches.begin(),
							m_patches.end());

	return ( clone );
}


//---------------------------------------------------------------------
/** Get the boundary extents in camera space of the surface patch mesh
 */

void CqSurfacePatchMeshBicubic::Bound(CqBound* bound) const
{
	// The patch mesh must first have been split into its child patches,
	//  which are assumed to have been transformed into the Bezier basis
	//  and are also in camera-space coordinates.
	assert(m_patches.size() > 0);

	// Get the bound in camera space, by encapsulating the bounds of the
	//  child patches.
	CqBound patchBound;
	std::vector<boost::shared_ptr<CqSurfacePatchBicubic> >::const_iterator
		iPatch, end;
	iPatch = m_patches.begin();
	end = m_patches.end();
	(*(iPatch++))->Bound(bound);
	for (; iPatch != end; iPatch++)
	{
		(*iPatch)->Bound(&patchBound);
		bound->Encapsulate(&patchBound);
	}
}


//---------------------------------------------------------------------
/** Transforms the surface by the specified matrix.
 */
void CqSurfacePatchMeshBicubic::Transform(
	const CqMatrix& matTx,
	const CqMatrix& matITTx,
	const CqMatrix& matRTx,
	TqInt iTime)
{
	// before we transform the patch mesh, we must first have called
	//  ConvertToBezierBasis(), which in turn calls SplitInternally().
	//  SplitInternally() will have created a cache of child bicubic patch
	//  objects.  in this function, we presume that we are operating on the
	//  child patches, and not on the original patch mesh.
	assert(m_patches.size() != 0);

	// transform each of the child patches
	std::vector<boost::shared_ptr<CqSurfacePatchBicubic> >::iterator
		iPatch, end;
	end = m_patches.end();
	for (iPatch = m_patches.begin(); iPatch != end; iPatch++) {
		(*iPatch)->Transform(matTx, matITTx, matRTx, iTime);
	}
}


//---------------------------------------------------------------------
/** Convert the patch mesh to use a Bezier basis matrix.
 */

void CqSurfacePatchMeshBicubic::ConvertToBezierBasis()
{
	// split the patch mesh internally, so that it becomes a bag of
	//  individual bicubic patches
	SplitInternally();

	CqMatrix matuBasis, matvBasis;
	matuBasis = pAttributes()->GetMatrixAttribute("System", "Basis")[0];
	matvBasis = pAttributes()->GetMatrixAttribute("System", "Basis")[1];

	std::vector<boost::shared_ptr<CqSurfacePatchBicubic> >::iterator
		iPatch, end;
	end = m_patches.end();
	for (iPatch = m_patches.begin(); iPatch != end; iPatch++)
		(*iPatch)->ConvertToBezierBasis(matuBasis, matvBasis);
}


//---------------------------------------------------------------------
/** Split the patch mesh into individual patches and post them.
 */

TqInt CqSurfacePatchMeshBicubic::Split(
	std::vector<boost::shared_ptr<CqSurface> >& aSplits)
{
	// we should already have called ConvertToBezierBasis(), which in turn
	//  calls SplitInternally().  SplitInternally() created an internal cache
	//  of bicubic mesh child objects, which we can now directly append to
	//  the vector of splits.
	assert(m_patches.size() != 0);

	aSplits.insert(aSplits.end(), m_patches.begin(), m_patches.end());
	return m_patches.size();
}


//---------------------------------------------------------------------
/** Internally split the patch mesh into individual child bicubic patches
 *  and cache them.
 */

#define	PatchCoord(v,u)	((((v)%m_nv)*m_nu)+((u)%m_nu))
#define	PatchCorner(v,u)	((((v)%nvaryingv)*nvaryingu)+((u)%nvaryingu));

void CqSurfacePatchMeshBicubic::SplitInternally()
{
	assert(m_patches.size() == 0);  // do not try to split twice!

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

			m_patches.push_back( pSurface );
		}
	}
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

void CqSurfacePatchMeshBilinear::Bound(CqBound* bound) const
{
	assert( NULL != P() );

	// Get the boundary in camera space.
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqUint i;
	for ( i = 0; i < P() ->Size(); i++ )
	{
		CqVector3D	vecV = vectorCast<CqVector3D>(P()->pValue( i )[0]);
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
	bound->vecMin() = vecA;
	bound->vecMax() = vecB;
	AdjustBoundForTransformationMotion( bound );
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

} // namespace Aqsis
