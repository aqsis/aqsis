
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


//---------------------------------------------------------------------
/** Subdivide a bicubic patch in the u direction, return the left side.
 */

void CqSurfacePatchBicubic::uSubdivide( CqSurfacePatchBicubic* pNew1, CqSurfacePatchBicubic* pNew2 )
{
	pNew1->P().SetSize( cVertex() );
	pNew2->P().SetSize( cVertex() );

	TqUint iv;
	for( iv = 0; iv < 4; iv++ )
	{
		TqUint ivo = ( iv * 4 );
		pNew1->P()[ ivo + 0 ] = static_cast<CqVector3D>( P()[ ivo + 0 ] );
		pNew1->P()[ ivo + 1 ] = static_cast<CqVector3D>( ( P()[ ivo + 0 ] + P()[ ivo + 1 ] ) / 2.0f );
		pNew1->P()[ ivo + 2 ] = static_cast<CqVector3D>( pNew1->P()[ ivo + 1 ] / 2.0f + ( P()[ ivo + 1 ] + P()[ ivo + 2 ] ) / 4.0f );

		pNew2->P()[ ivo + 3 ] = static_cast<CqVector3D>( P()[ ivo + 3 ] );
		pNew2->P()[ ivo + 2 ] = static_cast<CqVector3D>( ( P()[ ivo + 2 ] + P()[ ivo + 3 ] ) / 2.0f );
		pNew2->P()[ ivo + 1 ] = static_cast<CqVector3D>( pNew2->P()[ ivo + 2 ] / 2.0f + ( P()[ ivo + 1 ] + P()[ ivo + 2 ] ) / 4.0f );

		pNew1->P()[ ivo + 3 ] = static_cast<CqVector3D>( ( pNew1->P()[ ivo + 2] + pNew2->P()[ ivo + 1 ] ) / 2.0f );
		pNew2->P()[ ivo + 0 ] = static_cast<CqVector3D>( pNew1->P()[ ivo + 3 ] );
	}


	// Now do the same for 'vertex' class primitive variables.
	std::vector<CqParameter*>::iterator iUP;
	for( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		if( (*iUP)->Class() == class_vertex )
		{
			switch( (*iUP)->Type() )
			{
				case type_float:
				{
					CqParameterTyped<TqFloat, TqFloat>* pTParam =  static_cast<CqParameterTyped<TqFloat, TqFloat>*>(*iUP);
					CqParameterTyped<TqFloat, TqFloat>* pTParam1 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>((*iUP)->Clone());
					CqParameterTyped<TqFloat, TqFloat>* pTParam2 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>((*iUP)->Clone());
					pTParam1->SetSize( pNew1->cVertex() );
					pTParam2->SetSize( pNew2->cVertex() );
					for( iv = 0; iv < 4; iv++ )
					{
						TqUint ivo = ( iv * 4 );
						pTParam1->pValue()[ ivo + 0 ] = pTParam->pValue()[ ivo + 0 ];
						pTParam1->pValue()[ ivo + 1 ] = ( pTParam->pValue()[ ivo + 0 ] + pTParam->pValue()[ ivo + 1 ] ) / 2.0f;
						pTParam1->pValue()[ ivo + 2 ] = pTParam1->pValue()[ ivo + 1 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f;

						pTParam2->pValue()[ ivo + 3 ] = pTParam->pValue()[ ivo + 3 ];
						pTParam2->pValue()[ ivo + 2 ] = ( pTParam->pValue()[ ivo + 2 ] + pTParam->pValue()[ ivo + 3 ] ) / 2.0f;
						pTParam2->pValue()[ ivo + 1 ] = pTParam2->pValue()[ ivo + 2 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f;

						pTParam1->pValue()[ ivo + 3 ] = ( pTParam1->pValue()[ ivo + 2] + pTParam2->pValue()[ ivo + 1 ] ) / 2.0f;
						pTParam2->pValue()[ ivo + 0 ] = pTParam1->pValue()[ ivo + 3 ];
					}
					pNew1->AddPrimitiveVariable( pTParam1 );
					pNew2->AddPrimitiveVariable( pTParam2 );
					break;
				}

				case type_integer:
				{
					CqParameterTyped<TqInt, TqFloat>* pTParam =  static_cast<CqParameterTyped<TqInt, TqFloat>*>(*iUP);
					CqParameterTyped<TqInt, TqFloat>* pTParam1 = static_cast<CqParameterTyped<TqInt, TqFloat>*>((*iUP)->Clone());
					CqParameterTyped<TqInt, TqFloat>* pTParam2 = static_cast<CqParameterTyped<TqInt, TqFloat>*>((*iUP)->Clone());
					pTParam1->SetSize( pNew1->cVertex() );
					pTParam2->SetSize( pNew2->cVertex() );
					for( iv = 0; iv < 4; iv++ )
					{
						TqUint ivo = ( iv * 4 );
						pTParam1->pValue()[ ivo + 0 ] = pTParam->pValue()[ ivo + 0 ];
						pTParam1->pValue()[ ivo + 1 ] = ( pTParam->pValue()[ ivo + 0 ] + pTParam->pValue()[ ivo + 1 ] ) / 2.0f;
						pTParam1->pValue()[ ivo + 2 ] = pTParam1->pValue()[ ivo + 1 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f;

						pTParam2->pValue()[ ivo + 3 ] = pTParam->pValue()[ ivo + 3 ];
						pTParam2->pValue()[ ivo + 2 ] = ( pTParam->pValue()[ ivo + 2 ] + pTParam->pValue()[ ivo + 3 ] ) / 2.0f;
						pTParam2->pValue()[ ivo + 1 ] = pTParam2->pValue()[ ivo + 2 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f;

						pTParam1->pValue()[ ivo + 3 ] = ( pTParam1->pValue()[ ivo + 2] + pTParam2->pValue()[ ivo + 1 ] ) / 2.0f;
						pTParam2->pValue()[ ivo + 0 ] = pTParam1->pValue()[ ivo + 3 ];
					}
					pNew1->AddPrimitiveVariable( pTParam1 );
					pNew2->AddPrimitiveVariable( pTParam2 );
					break;
				}

				case type_point:
				case type_vector:
				case type_normal:
				{
					CqParameterTyped<CqVector3D, CqVector3D>* pTParam =  static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>(*iUP);
					CqParameterTyped<CqVector3D, CqVector3D>* pTParam1 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>((*iUP)->Clone());
					CqParameterTyped<CqVector3D, CqVector3D>* pTParam2 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>((*iUP)->Clone());
					pTParam1->SetSize( pNew1->cVertex() );
					pTParam2->SetSize( pNew2->cVertex() );
					for( iv = 0; iv < 4; iv++ )
					{
						TqUint ivo = ( iv * 4 );
						pTParam1->pValue()[ ivo + 0 ] = pTParam->pValue()[ ivo + 0 ];
						pTParam1->pValue()[ ivo + 1 ] = ( pTParam->pValue()[ ivo + 0 ] + pTParam->pValue()[ ivo + 1 ] ) / 2.0f;
						pTParam1->pValue()[ ivo + 2 ] = pTParam1->pValue()[ ivo + 1 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f;

						pTParam2->pValue()[ ivo + 3 ] = pTParam->pValue()[ ivo + 3 ];
						pTParam2->pValue()[ ivo + 2 ] = ( pTParam->pValue()[ ivo + 2 ] + pTParam->pValue()[ ivo + 3 ] ) / 2.0f;
						pTParam2->pValue()[ ivo + 1 ] = pTParam2->pValue()[ ivo + 2 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f;

						pTParam1->pValue()[ ivo + 3 ] = ( pTParam1->pValue()[ ivo + 2] + pTParam2->pValue()[ ivo + 1 ] ) / 2.0f;
						pTParam2->pValue()[ ivo + 0 ] = pTParam1->pValue()[ ivo + 3 ];
					}
					pNew1->AddPrimitiveVariable( pTParam1 );
					pNew2->AddPrimitiveVariable( pTParam2 );
					break;
				}

				case type_hpoint:
				{
					CqParameterTyped<CqVector4D, CqVector3D>* pTParam =  static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>(*iUP);
					CqParameterTyped<CqVector4D, CqVector3D>* pTParam1 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>((*iUP)->Clone());
					CqParameterTyped<CqVector4D, CqVector3D>* pTParam2 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>((*iUP)->Clone());
					pTParam1->SetSize( pNew1->cVertex() );
					pTParam2->SetSize( pNew2->cVertex() );
					for( iv = 0; iv < 4; iv++ )
					{
						TqUint ivo = ( iv * 4 );
						pTParam1->pValue()[ ivo + 0 ] = static_cast<CqVector3D>( pTParam->pValue()[ ivo + 0 ] );
						pTParam1->pValue()[ ivo + 1 ] = static_cast<CqVector3D>( ( pTParam->pValue()[ ivo + 0 ] + pTParam->pValue()[ ivo + 1 ] ) / 2.0f );
						pTParam1->pValue()[ ivo + 2 ] = static_cast<CqVector3D>( pTParam1->pValue()[ ivo + 1 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f );

						pTParam2->pValue()[ ivo + 3 ] = static_cast<CqVector3D>( pTParam->pValue()[ ivo + 3 ] );
						pTParam2->pValue()[ ivo + 2 ] = static_cast<CqVector3D>( ( pTParam->pValue()[ ivo + 2 ] + pTParam->pValue()[ ivo + 3 ] ) / 2.0f );
						pTParam2->pValue()[ ivo + 1 ] = static_cast<CqVector3D>( pTParam2->pValue()[ ivo + 2 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f );

						pTParam1->pValue()[ ivo + 3 ] = static_cast<CqVector3D>( ( pTParam1->pValue()[ ivo + 2] + pTParam2->pValue()[ ivo + 1 ] ) / 2.0f );
						pTParam2->pValue()[ ivo + 0 ] = static_cast<CqVector3D>( pTParam1->pValue()[ ivo + 3 ] );
					}
					pNew1->AddPrimitiveVariable( pTParam1 );
					pNew2->AddPrimitiveVariable( pTParam2 );
					break;
				}


				case type_color:
				{
					CqParameterTyped<CqColor, CqColor>* pTParam =  static_cast<CqParameterTyped<CqColor, CqColor>*>(*iUP);
					CqParameterTyped<CqColor, CqColor>* pTParam1 = static_cast<CqParameterTyped<CqColor, CqColor>*>((*iUP)->Clone());
					CqParameterTyped<CqColor, CqColor>* pTParam2 = static_cast<CqParameterTyped<CqColor, CqColor>*>((*iUP)->Clone());
					pTParam1->SetSize( pNew1->cVertex() );
					pTParam2->SetSize( pNew2->cVertex() );
					for( iv = 0; iv < 4; iv++ )
					{
						TqUint ivo = ( iv * 4 );
						pTParam1->pValue()[ ivo + 0 ] = pTParam->pValue()[ ivo + 0 ];
						pTParam1->pValue()[ ivo + 1 ] = ( pTParam->pValue()[ ivo + 0 ] + pTParam->pValue()[ ivo + 1 ] ) / 2.0f;
						pTParam1->pValue()[ ivo + 2 ] = pTParam1->pValue()[ ivo + 1 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f;

						pTParam2->pValue()[ ivo + 3 ] = pTParam->pValue()[ ivo + 3 ];
						pTParam2->pValue()[ ivo + 2 ] = ( pTParam->pValue()[ ivo + 2 ] + pTParam->pValue()[ ivo + 3 ] ) / 2.0f;
						pTParam2->pValue()[ ivo + 1 ] = pTParam2->pValue()[ ivo + 2 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f;

						pTParam1->pValue()[ ivo + 3 ] = ( pTParam1->pValue()[ ivo + 2] + pTParam2->pValue()[ ivo + 1 ] ) / 2.0f;
						pTParam2->pValue()[ ivo + 0 ] = pTParam1->pValue()[ ivo + 3 ];
					}
					pNew1->AddPrimitiveVariable( pTParam1 );
					pNew2->AddPrimitiveVariable( pTParam2 );
					break;
				}

				case type_string:
				{
					CqParameterTyped<CqString, CqString>* pTParam =  static_cast<CqParameterTyped<CqString, CqString>*>(*iUP);
					CqParameterTyped<CqString, CqString>* pTParam1 = static_cast<CqParameterTyped<CqString, CqString>*>((*iUP)->Clone());
					CqParameterTyped<CqString, CqString>* pTParam2 = static_cast<CqParameterTyped<CqString, CqString>*>((*iUP)->Clone());
					pTParam1->SetSize( pNew1->cVertex() );
					pTParam2->SetSize( pNew2->cVertex() );
					for( iv = 0; iv < 4; iv++ )
					{
						TqUint ivo = ( iv * 4 );
						pTParam1->pValue()[ ivo + 0 ] = pTParam->pValue()[ ivo + 0 ];
						pTParam1->pValue()[ ivo + 1 ] = ( pTParam->pValue()[ ivo + 0 ] + pTParam->pValue()[ ivo + 1 ] ) / 2.0f;
						pTParam1->pValue()[ ivo + 2 ] = pTParam1->pValue()[ ivo + 1 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f;

						pTParam2->pValue()[ ivo + 3 ] = pTParam->pValue()[ ivo + 3 ];
						pTParam2->pValue()[ ivo + 2 ] = ( pTParam->pValue()[ ivo + 2 ] + pTParam->pValue()[ ivo + 3 ] ) / 2.0f;
						pTParam2->pValue()[ ivo + 1 ] = pTParam2->pValue()[ ivo + 2 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f;

						pTParam1->pValue()[ ivo + 3 ] = ( pTParam1->pValue()[ ivo + 2] + pTParam2->pValue()[ ivo + 1 ] ) / 2.0f;
						pTParam2->pValue()[ ivo + 0 ] = pTParam1->pValue()[ ivo + 3 ];
					}
					pNew1->AddPrimitiveVariable( pTParam1 );
					pNew2->AddPrimitiveVariable( pTParam2 );
					break;
				}

				case type_matrix:
				{
//					CqParameterTyped<CqMatrix, CqMatrix>* pTParam =  static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>(*iUP);
//					CqParameterTyped<CqMatrix, CqMatrix>* pTParam1 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>((*iUP)->Clone());
//					CqParameterTyped<CqMatrix, CqMatrix>* pTParam2 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>((*iUP)->Clone());
//					pTParam1->SetSize( pNew1->cVertex() );
//					pTParam2->SetSize( pNew2->cVertex() );
//					for( iv = 0; iv < 4; iv++ )
//					{
//						TqUint ivo = ( iv * 4 );
//						pTParam1->pValue()[ ivo + 0 ] = pTParam->pValue()[ ivo + 0 ];
//						pTParam1->pValue()[ ivo + 1 ] = ( pTParam->pValue()[ ivo + 0 ] + pTParam->pValue()[ ivo + 1 ] ) / 2.0f;
//						pTParam1->pValue()[ ivo + 2 ] = pTParam1->pValue()[ ivo + 1 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f;
//
//						pTParam2->pValue()[ ivo + 3 ] = pTParam->pValue()[ ivo + 3 ];
//						pTParam2->pValue()[ ivo + 2 ] = ( pTParam->pValue()[ ivo + 2 ] + pTParam->pValue()[ ivo + 3 ] ) / 2.0f;
//						pTParam2->pValue()[ ivo + 1 ] = pTParam2->pValue()[ ivo + 2 ] / 2.0f + ( pTParam->pValue()[ ivo + 1 ] + pTParam->pValue()[ ivo + 2 ] ) / 4.0f;
//
//						pTParam1->pValue()[ ivo + 3 ] = ( pTParam1->pValue()[ ivo + 2] + pTParam2->pValue()[ ivo + 1 ] ) / 2.0f;
//						pTParam2->pValue()[ ivo + 0 ] = pTParam1->pValue()[ ivo + 3 ];
//					}
//					pNew1->AddPrimitiveVariable( pTParam1 );
//					pNew2->AddPrimitiveVariable( pTParam2 );
					break;
				}
			}
		}
	}

	// Subdivide the normals
	if ( USES( Uses(), EnvVars_N ) && bHasN() ) 
	{
		pNew1->N().SetSize( cVertex() );
		pNew2->N().SetSize( cVertex() );
		// Subdivide the Normals.
		pNew1->N() = N();
		pNew1->N().uSubdivide( &pNew2->N() );
	}

	uSubdivideUserParameters( pNew1, pNew2 );
}


//---------------------------------------------------------------------
/** Subdivide a bicubic patch in the v direction, return the top side.
 */

void CqSurfacePatchBicubic::vSubdivide( CqSurfacePatchBicubic* pNew1, CqSurfacePatchBicubic* pNew2 )
{
	pNew1->P().SetSize( cVertex() );
	pNew2->P().SetSize( cVertex() );

	TqUint iu;
	for( iu = 0; iu < 4; iu++ )
	{
		pNew1->P()[  0 + iu ] = static_cast<CqVector3D>( P()[ 0 + iu ] );
		pNew1->P()[  4 + iu ] = static_cast<CqVector3D>( ( P()[ 0 + iu ] + P()[ 4 + iu ] ) / 2.0f );
		pNew1->P()[  8 + iu ] = static_cast<CqVector3D>( pNew1->P()[ 4 + iu ] / 2.0f + ( P()[ 4 + iu ] + P()[ 8 + iu ] ) / 4.0f );

		pNew2->P()[ 12 + iu ] = static_cast<CqVector3D>( P()[ 12 + iu ] );
		pNew2->P()[  8 + iu ] = static_cast<CqVector3D>( ( P()[ 8 + iu ] + P()[ 12 + iu ] ) / 2.0f );
		pNew2->P()[  4 + iu ] = static_cast<CqVector3D>( pNew2->P()[ 8 + iu ] / 2.0f + ( P()[ 4 + iu ] + P()[ 8 + iu ] ) / 4.0f );

		pNew1->P()[ 12 + iu ] = static_cast<CqVector3D>( ( pNew1->P()[ 8 + iu ] + pNew2->P()[ 4 + iu ] ) / 2.0f );
		pNew2->P()[  0 + iu ] = static_cast<CqVector3D>( pNew1->P()[ 12 + iu ] );
	}

	// Now do the same for 'vertex' class primitive variables.
	std::vector<CqParameter*>::iterator iUP;
	for( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		if( (*iUP)->Class() == class_vertex )
		{
			switch( (*iUP)->Type() )
			{
				case type_float:
				{
					CqParameterTyped<TqFloat, TqFloat>* pTParam =  static_cast<CqParameterTyped<TqFloat, TqFloat>*>(*iUP);
					CqParameterTyped<TqFloat, TqFloat>* pTParam1 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>((*iUP)->Clone());
					CqParameterTyped<TqFloat, TqFloat>* pTParam2 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>((*iUP)->Clone());
					pTParam1->SetSize( pNew1->cVertex() );
					pTParam2->SetSize( pNew2->cVertex() );
					for( iu = 0; iu < 4; iu++ )
					{
						pTParam1->pValue()[  0 + iu ] = pTParam->pValue()[ 0 + iu ];
						pTParam1->pValue()[  4 + iu ] = ( pTParam->pValue()[ 0 + iu ] + pTParam->pValue()[ 4 + iu ] ) / 2.0f;
						pTParam1->pValue()[  8 + iu ] = pTParam1->pValue()[ 4 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f;

						pTParam2->pValue()[ 12 + iu ] = pTParam->pValue()[ 12 + iu ];
						pTParam2->pValue()[  8 + iu ] = ( pTParam->pValue()[ 8 + iu ] + pTParam->pValue()[ 12 + iu ] ) / 2.0f;
						pTParam2->pValue()[  4 + iu ] = pTParam2->pValue()[ 8 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f;

						pTParam1->pValue()[ 12 + iu ] = ( pTParam1->pValue()[ 8 + iu ] + pTParam2->pValue()[ 4 + iu ] ) / 2.0f;
						pTParam2->pValue()[  0 + iu ] = pTParam1->pValue()[ 12 + iu ];
					}
					pNew1->AddPrimitiveVariable( pTParam1 );
					pNew2->AddPrimitiveVariable( pTParam2 );
					break;
				}

				case type_integer:
				{
					CqParameterTyped<TqInt, TqFloat>* pTParam =  static_cast<CqParameterTyped<TqInt, TqFloat>*>(*iUP);
					CqParameterTyped<TqInt, TqFloat>* pTParam1 = static_cast<CqParameterTyped<TqInt, TqFloat>*>((*iUP)->Clone());
					CqParameterTyped<TqInt, TqFloat>* pTParam2 = static_cast<CqParameterTyped<TqInt, TqFloat>*>((*iUP)->Clone());
					pTParam1->SetSize( pNew1->cVertex() );
					pTParam2->SetSize( pNew2->cVertex() );
					for( iu = 0; iu < 4; iu++ )
					{
						pTParam1->pValue()[  0 + iu ] = pTParam->pValue()[ 0 + iu ];
						pTParam1->pValue()[  4 + iu ] = ( pTParam->pValue()[ 0 + iu ] + pTParam->pValue()[ 4 + iu ] ) / 2.0f;
						pTParam1->pValue()[  8 + iu ] = pTParam1->pValue()[ 4 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f;

						pTParam2->pValue()[ 12 + iu ] = pTParam->pValue()[ 12 + iu ];
						pTParam2->pValue()[  8 + iu ] = ( pTParam->pValue()[ 8 + iu ] + pTParam->pValue()[ 12 + iu ] ) / 2.0f;
						pTParam2->pValue()[  4 + iu ] = pTParam2->pValue()[ 8 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f;

						pTParam1->pValue()[ 12 + iu ] = ( pTParam1->pValue()[ 8 + iu ] + pTParam2->pValue()[ 4 + iu ] ) / 2.0f;
						pTParam2->pValue()[  0 + iu ] = pTParam1->pValue()[ 12 + iu ];
					}
					pNew1->AddPrimitiveVariable( pTParam1 );
					pNew2->AddPrimitiveVariable( pTParam2 );
					break;
				}

				case type_point:
				case type_normal:
				case type_vector:
				{
					CqParameterTyped<CqVector3D, CqVector3D>* pTParam =  static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>(*iUP);
					CqParameterTyped<CqVector3D, CqVector3D>* pTParam1 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>((*iUP)->Clone());
					CqParameterTyped<CqVector3D, CqVector3D>* pTParam2 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>((*iUP)->Clone());
					pTParam1->SetSize( pNew1->cVertex() );
					pTParam2->SetSize( pNew2->cVertex() );
					for( iu = 0; iu < 4; iu++ )
					{
						pTParam1->pValue()[  0 + iu ] = pTParam->pValue()[ 0 + iu ];
						pTParam1->pValue()[  4 + iu ] = ( pTParam->pValue()[ 0 + iu ] + pTParam->pValue()[ 4 + iu ] ) / 2.0f;
						pTParam1->pValue()[  8 + iu ] = pTParam1->pValue()[ 4 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f;

						pTParam2->pValue()[ 12 + iu ] = pTParam->pValue()[ 12 + iu ];
						pTParam2->pValue()[  8 + iu ] = ( pTParam->pValue()[ 8 + iu ] + pTParam->pValue()[ 12 + iu ] ) / 2.0f;
						pTParam2->pValue()[  4 + iu ] = pTParam2->pValue()[ 8 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f;

						pTParam1->pValue()[ 12 + iu ] = ( pTParam1->pValue()[ 8 + iu ] + pTParam2->pValue()[ 4 + iu ] ) / 2.0f;
						pTParam2->pValue()[  0 + iu ] = pTParam1->pValue()[ 12 + iu ];
					}
					pNew1->AddPrimitiveVariable( pTParam1 );
					pNew2->AddPrimitiveVariable( pTParam2 );
					break;
				}

				case type_hpoint:
				{
					CqParameterTyped<CqVector4D, CqVector3D>* pTParam =  static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>(*iUP);
					CqParameterTyped<CqVector4D, CqVector3D>* pTParam1 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>((*iUP)->Clone());
					CqParameterTyped<CqVector4D, CqVector3D>* pTParam2 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>((*iUP)->Clone());
					pTParam1->SetSize( pNew1->cVertex() );
					pTParam2->SetSize( pNew2->cVertex() );
					for( iu = 0; iu < 4; iu++ )
					{
						pTParam1->pValue()[  0 + iu ] = static_cast<CqVector3D>( pTParam->pValue()[ 0 + iu ] );
						pTParam1->pValue()[  4 + iu ] = static_cast<CqVector3D>( ( pTParam->pValue()[ 0 + iu ] + pTParam->pValue()[ 4 + iu ] ) / 2.0f );
						pTParam1->pValue()[  8 + iu ] = static_cast<CqVector3D>( pTParam1->pValue()[ 4 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f );

						pTParam2->pValue()[ 12 + iu ] = static_cast<CqVector3D>( pTParam->pValue()[ 12 + iu ] );
						pTParam2->pValue()[  8 + iu ] = static_cast<CqVector3D>( ( pTParam->pValue()[ 8 + iu ] + pTParam->pValue()[ 12 + iu ] ) / 2.0f );
						pTParam2->pValue()[  4 + iu ] = static_cast<CqVector3D>( pTParam2->pValue()[ 8 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f );

						pTParam1->pValue()[ 12 + iu ] = static_cast<CqVector3D>( ( pTParam1->pValue()[ 8 + iu ] + pTParam2->pValue()[ 4 + iu ] ) / 2.0f );
						pTParam2->pValue()[  0 + iu ] = static_cast<CqVector3D>( pTParam1->pValue()[ 12 + iu ] );
					}
					pNew1->AddPrimitiveVariable( pTParam1 );
					pNew2->AddPrimitiveVariable( pTParam2 );
					break;
				}

				case type_color:
				{
					CqParameterTyped<CqColor, CqColor>* pTParam =  static_cast<CqParameterTyped<CqColor, CqColor>*>(*iUP);
					CqParameterTyped<CqColor, CqColor>* pTParam1 = static_cast<CqParameterTyped<CqColor, CqColor>*>((*iUP)->Clone());
					CqParameterTyped<CqColor, CqColor>* pTParam2 = static_cast<CqParameterTyped<CqColor, CqColor>*>((*iUP)->Clone());
					pTParam1->SetSize( pNew1->cVertex() );
					pTParam2->SetSize( pNew2->cVertex() );
					for( iu = 0; iu < 4; iu++ )
					{
						pTParam1->pValue()[  0 + iu ] = pTParam->pValue()[ 0 + iu ];
						pTParam1->pValue()[  4 + iu ] = ( pTParam->pValue()[ 0 + iu ] + pTParam->pValue()[ 4 + iu ] ) / 2.0f;
						pTParam1->pValue()[  8 + iu ] = pTParam1->pValue()[ 4 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f;

						pTParam2->pValue()[ 12 + iu ] = pTParam->pValue()[ 12 + iu ];
						pTParam2->pValue()[  8 + iu ] = ( pTParam->pValue()[ 8 + iu ] + pTParam->pValue()[ 12 + iu ] ) / 2.0f;
						pTParam2->pValue()[  4 + iu ] = pTParam2->pValue()[ 8 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f;

						pTParam1->pValue()[ 12 + iu ] = ( pTParam1->pValue()[ 8 + iu ] + pTParam2->pValue()[ 4 + iu ] ) / 2.0f;
						pTParam2->pValue()[  0 + iu ] = pTParam1->pValue()[ 12 + iu ];
					}
					pNew1->AddPrimitiveVariable( pTParam1 );
					pNew2->AddPrimitiveVariable( pTParam2 );
					break;
				}

				case type_string:
				{
					CqParameterTyped<CqString, CqString>* pTParam =  static_cast<CqParameterTyped<CqString, CqString>*>(*iUP);
					CqParameterTyped<CqString, CqString>* pTParam1 = static_cast<CqParameterTyped<CqString, CqString>*>((*iUP)->Clone());
					CqParameterTyped<CqString, CqString>* pTParam2 = static_cast<CqParameterTyped<CqString, CqString>*>((*iUP)->Clone());
					pTParam1->SetSize( pNew1->cVertex() );
					pTParam2->SetSize( pNew2->cVertex() );
					for( iu = 0; iu < 4; iu++ )
					{
						pTParam1->pValue()[  0 + iu ] = pTParam->pValue()[ 0 + iu ];
						pTParam1->pValue()[  4 + iu ] = ( pTParam->pValue()[ 0 + iu ] + pTParam->pValue()[ 4 + iu ] ) / 2.0f;
						pTParam1->pValue()[  8 + iu ] = pTParam1->pValue()[ 4 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f;

						pTParam2->pValue()[ 12 + iu ] = pTParam->pValue()[ 12 + iu ];
						pTParam2->pValue()[  8 + iu ] = ( pTParam->pValue()[ 8 + iu ] + pTParam->pValue()[ 12 + iu ] ) / 2.0f;
						pTParam2->pValue()[  4 + iu ] = pTParam2->pValue()[ 8 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f;

						pTParam1->pValue()[ 12 + iu ] = ( pTParam1->pValue()[ 8 + iu ] + pTParam2->pValue()[ 4 + iu ] ) / 2.0f;
						pTParam2->pValue()[  0 + iu ] = pTParam1->pValue()[ 12 + iu ];
					}
					pNew1->AddPrimitiveVariable( pTParam1 );
					pNew2->AddPrimitiveVariable( pTParam2 );
					break;
				}

//				case type_matrix:
//				{
//					CqParameterTyped<CqMatrix, CqMatrix>* pTParam =  static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>(*iUP);
//					CqParameterTyped<CqMatrix, CqMatrix>* pTParam1 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>((*iUP)->Clone());
//					CqParameterTyped<CqMatrix, CqMatrix>* pTParam2 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>((*iUP)->Clone());
//					pTParam1->SetSize( pNew1->cVertex() );
//					pTParam2->SetSize( pNew2->cVertex() );
//					for( iu = 0; iu < 4; iu++ )
//					{
//						pTParam1->pValue()[  0 + iu ] = pTParam->pValue()[ 0 + iu ];
//						pTParam1->pValue()[  4 + iu ] = ( pTParam->pValue()[ 0 + iu ] + pTParam->pValue()[ 4 + iu ] ) / 2.0f;
//						pTParam1->pValue()[  8 + iu ] = pTParam1->pValue()[ 4 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f;
//
//						pTParam2->pValue()[ 12 + iu ] = pTParam->pValue()[ 12 + iu ];
//						pTParam2->pValue()[  8 + iu ] = ( pTParam->pValue()[ 8 + iu ] + pTParam->pValue()[ 12 + iu ] ) / 2.0f;
//						pTParam2->pValue()[  4 + iu ] = pTParam2->pValue()[ 8 + iu ] / 2.0f + ( pTParam->pValue()[ 4 + iu ] + pTParam->pValue()[ 8 + iu ] ) / 4.0f;
//
//						pTParam1->pValue()[ 12 + iu ] = ( pTParam1->pValue()[ 8 + iu ] + pTParam2->pValue()[ 4 + iu ] ) / 2.0f;
//						pTParam2->pValue()[  0 + iu ] = pTParam1->pValue()[ 12 + iu ];
//					}
//					pNew1->AddPrimitiveVariable( pTParam1 );
//					pNew2->AddPrimitiveVariable( pTParam2 );
//					break;
//				}
			}
		}
	}


	// Subdivide the normals
	if ( USES( Uses(), EnvVars_N ) && bHasN() ) 
	{
		pNew1->N().SetSize( cVertex() );
		pNew2->N().SetSize( cVertex() );
		// Subdivide the Normals.
		pNew1->N() = N();
		pNew1->N().vSubdivide( &pNew2->N() );
	}

	vSubdivideUserParameters( pNew1, pNew2 );
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
		CqVector3D	vecV = P() [ i ];
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


void CqSurfacePatchBicubic::NaturalInterpolate(CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData)
{
	switch( pParameter->Type() )
	{
		case type_float:
		{
			CqParameterTyped<TqFloat, TqFloat>* pTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>(pParameter);
			TypedNaturalInterpolate( uDiceSize, vDiceSize, pTParam, pData );
			break;
		}

		case type_integer:
		{
			CqParameterTyped<TqInt, TqFloat>* pTParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>(pParameter);
			TypedNaturalInterpolate( uDiceSize, vDiceSize, pTParam, pData );
			break;
		}

		case type_point:
		case type_vector:
		case type_normal:
		{
			CqParameterTyped<CqVector3D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>(pParameter);
			TypedNaturalInterpolate( uDiceSize, vDiceSize, pTParam, pData );
			break;
		}

		case type_hpoint:
		{
			CqParameterTyped<CqVector4D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>(pParameter);
			TypedNaturalInterpolate( uDiceSize, vDiceSize, pTParam, pData );
			break;
		}

		case type_color:
		{
			CqParameterTyped<CqColor, CqColor>* pTParam = static_cast<CqParameterTyped<CqColor, CqColor>*>(pParameter);
			TypedNaturalInterpolate( uDiceSize, vDiceSize, pTParam, pData );
			break;
		}

		case type_string:
		{
			CqParameterTyped<CqString, CqString>* pTParam = static_cast<CqParameterTyped<CqString, CqString>*>(pParameter);
			TypedNaturalInterpolate( uDiceSize, vDiceSize, pTParam, pData );
			break;
		}

		case type_matrix:
		{
			CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>(pParameter);
			TypedNaturalInterpolate( uDiceSize, vDiceSize, pTParam, pData );
			break;
		}
	}
}

//---------------------------------------------------------------------
/** Split the patch into smaller patches.
 */

TqInt CqSurfacePatchBicubic::Split( std::vector<CqBasicSurface*>& aSplits )
{
	TqInt cSplits = 0;

	// Split the surface in u or v
	CqSurfacePatchBicubic * pNew1 = new CqSurfacePatchBicubic;
	CqSurfacePatchBicubic * pNew2 = new CqSurfacePatchBicubic;

	// If this primitive is being split because it spans the e and hither planes, then
	// we should split in both directions to ensure we overcome the crossing.
	if ( m_SplitDir == SplitDir_U )
		uSubdivide(pNew1, pNew2);
	else
		vSubdivide(pNew1, pNew2);

	pNew1->SetSurfaceParameters( *this );
	pNew2->SetSurfaceParameters( *this );
	pNew1->m_fDiceable = TqTrue;
	pNew2->m_fDiceable = TqTrue;
	pNew1->m_SplitDir = (m_SplitDir == SplitDir_U)? SplitDir_V:SplitDir_U;
	pNew2->m_SplitDir = (m_SplitDir == SplitDir_U)? SplitDir_V:SplitDir_U;
	pNew1->AddRef();
	pNew2->AddRef();

	if ( !m_fDiceable)
	{
		cSplits += pNew1->Split( aSplits );
		cSplits += pNew2->Split( aSplits );
		pNew1->Release();
		pNew2->Release();
	}
	else
	{
		aSplits.push_back( pNew1 );
		aSplits.push_back( pNew2 );

		cSplits += 2;
	}
	return ( cSplits );
}

//---------------------------------------------------------------------
/** Determine whether or not the patch is diceable
 */

TqBool	CqSurfacePatchBicubic::Diceable()
{
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
	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute("System", "ShadingRate")[0];
	if ( poptGridSize )
		gridsize = poptGridSize[ 0 ];
	else
		gridsize = static_cast<TqInt>( m_XBucketSize * m_XBucketSize / ShadingRate );
	for ( i = 0; i < 16; i++ )
		avecHull[ i ] = matCtoR * P() [ i ];

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
	m_uDiceSize = CEIL_POW2(m_uDiceSize);
	m_vDiceSize = CEIL_POW2(m_vDiceSize);

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
/** Transform the patch by the specified matrix.
 */

void	CqSurfacePatchBicubic::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx )
{
	// Tansform the control hull by the specified matrix.
	TqInt i;
	for ( i = 0; i < 16; i++ )
		P() [ i ] = matTx * P() [ i ];
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqSurfacePatchBilinear::CqSurfacePatchBilinear() : CqSurface()
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
/** Evaluate a bilinear spline patch normal at the specified intervals.
 */

CqVector4D CqSurfacePatchBilinear::EvaluateNormal( TqFloat s, TqFloat t ) const
{
	CqVector3D vecNAB, vecNCD;
	// Work out where the u points are first, then linear interpolate the v value.
	if ( s <= 0.0 )
	{
		vecNAB = N() [ 0 ];
		vecNCD = N() [ 2 ];
	}
	else
	{
		if ( s >= 1.0 )
		{
			vecNAB = N() [ 1 ];
			vecNCD = N() [ 3 ];
		}
		else
		{
			vecNAB = ( N() [ 1 ] * s ) + ( N() [ 0 ] * ( 1.0 - s ) );
			vecNCD = ( N() [ 3 ] * s ) + ( N() [ 2 ] * ( 1.0 - s ) );
		}
	}

	CqVector3D vecN;
	if ( t <= 0.0 )
		vecN = vecNAB;
	else
	{
		if ( t >= 1.0 )
			vecN = vecNCD;
		else
			vecN = ( vecNCD * t ) + ( vecNAB * ( 1.0 - t ) );
	}

	return ( vecN );
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqSurfacePatchBilinear& CqSurfacePatchBilinear::operator=( const CqSurfacePatchBilinear& From )
{
	CqSurface::operator=( From );

	return ( *this );
}


//---------------------------------------------------------------------
/** Generate the vertex normals if not specified.
 */

void CqSurfacePatchBilinear::GenerateGeometricNormals( TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pNormals )
{
	assert( P().Size() == 4 );
	N().SetSize( 4 );

	// Get the handedness of the coordinate system (at the time of creation) and
	// the coordinate system specified, to check for normal flipping.
	TqInt O = pAttributes() ->GetIntegerAttribute("System", "Orientation")[0];

	// For each of the four points, calculate the normal as the cross product of its
	// two vectors.
	N() [ 0 ] = ( P() [ 1 ] - P() [ 0 ] ) % ( P() [ 2 ] - P() [ 0 ] );
	N() [ 1 ] = ( P() [ 3 ] - P() [ 1 ] ) % ( P() [ 0 ] - P() [ 1 ] );
	N() [ 2 ] = ( P() [ 0 ] - P() [ 2 ] ) % ( P() [ 3 ] - P() [ 2 ] );
	N() [ 3 ] = ( P() [ 2 ] - P() [ 3 ] ) % ( P() [ 1 ] - P() [ 3 ] );

	CqVector3D	N;
	TqInt v, u;
	for ( v = 0; v <= vDiceSize; v++ )
	{
		for ( u = 0; u <= uDiceSize; u++ )
		{
			TqInt igrid = ( v * ( uDiceSize + 1 ) ) + u;
			N = EvaluateNormal( u, v );
			N = ( O == OrientationLH )? N : -N;
			pNormals->SetNormal( N, igrid );
		}
	}
}


//---------------------------------------------------------------------
/** Subdivide a bicubic patch in the u direction, return the left side.
 */

void CqSurfacePatchBilinear::uSubdivide( CqSurfacePatchBilinear* pNew1, CqSurfacePatchBilinear* pNew2 )
{
	pNew1->P().SetSize( cVertex() );
	pNew2->P().SetSize( cVertex() );
	// Subdivide the vertices
	pNew1->P() = P();
	pNew1->P().uSubdivide( &pNew2->P() );

	std::vector<CqParameter*>::iterator iUP;
	for( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		if( (*iUP)->Class() == class_vertex )
		{
			CqParameter* pParam1 = (*iUP)->Clone();
			CqParameter* pParam2 = (*iUP)->Clone();
			pParam1->uSubdivide( pParam2 );
			pNew1->AddPrimitiveVariable( pParam1 );
			pNew2->AddPrimitiveVariable( pParam2 );
		}
	}

	// Subdivide the normals
	if ( USES( Uses(), EnvVars_N ) && bHasN() ) 
	{
		pNew1->N().SetSize( cVertex() );
		pNew2->N().SetSize( cVertex() );
		// Subdivide the Normals.
		pNew1->N() = N();
		pNew1->N().uSubdivide( &pNew2->N() );
	}

	uSubdivideUserParameters( pNew1, pNew2 );
}


//---------------------------------------------------------------------
/** Subdivide a bicubic patch in the v direction, return the top side.
 */

void CqSurfacePatchBilinear::vSubdivide( CqSurfacePatchBilinear* pNew1, CqSurfacePatchBilinear* pNew2 )
{
	pNew1->P().SetSize( cVertex() );
	pNew2->P().SetSize( cVertex() );
	// Subdivide the vertices.
	pNew1->P() = P();
	pNew1->P().vSubdivide( &pNew2->P() );

	std::vector<CqParameter*>::iterator iUP;
	for( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		if( (*iUP)->Class() == class_vertex )
		{
			CqParameter* pParam1 = (*iUP)->Clone();
			CqParameter* pParam2 = (*iUP)->Clone();
			pParam1->vSubdivide( pParam2 );
			pNew1->AddPrimitiveVariable( pParam1 );
			pNew2->AddPrimitiveVariable( pParam2 );
		}
	}

	// Subdivide the normals
	if ( USES( Uses(), EnvVars_N ) && bHasN() ) 
	{
		pNew1->N().SetSize( cVertex() );
		pNew2->N().SetSize( cVertex() );
		// Subdivide the Normals.
		pNew1->N() = N();
		pNew1->N().vSubdivide( &pNew2->N() );
	}

	vSubdivideUserParameters( pNew1, pNew2 );
}


//---------------------------------------------------------------------
/** Return the boundary extents in camera space of the surface patch
 */

CqBound CqSurfacePatchBilinear::Bound() const
{
	// Get the boundary in camera space.
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqInt i;
	for ( i = 0; i < 4; i++ )
	{
		CqVector3D	vecV = P() [ i ];
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

void CqSurfacePatchBilinear::NaturalInterpolate(CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData)
{
	pParameter->BilinearDice(uDiceSize, vDiceSize, pData);
}


//---------------------------------------------------------------------
/** Split the patch into smaller patches.
 */

TqInt CqSurfacePatchBilinear::Split( std::vector<CqBasicSurface*>& aSplits )
{
	TqInt cSplits = 0;

	// Split the surface in u or v
	CqSurfacePatchBilinear* pNew1 = new CqSurfacePatchBilinear;
	CqSurfacePatchBilinear* pNew2 = new CqSurfacePatchBilinear;

	if ( m_SplitDir == SplitDir_U )
		uSubdivide(pNew1, pNew2);
	else
		vSubdivide(pNew1, pNew2);

	pNew1->SetSurfaceParameters( *this );
	pNew2->SetSurfaceParameters( *this );
	pNew1->m_fDiceable = TqTrue;
	pNew2->m_fDiceable = TqTrue;
	pNew1->m_SplitDir = (m_SplitDir == SplitDir_U)? SplitDir_V:SplitDir_U;
	pNew2->m_SplitDir = (m_SplitDir == SplitDir_U)? SplitDir_V:SplitDir_U;
	pNew1->m_EyeSplitCount = m_EyeSplitCount;
	pNew2->m_EyeSplitCount = m_EyeSplitCount;
	pNew1->AddRef();
	pNew2->AddRef();

	if( !m_fDiceable )
	{
		cSplits += pNew1->Split( aSplits );
		cSplits += pNew2->Split( aSplits );
		pNew1->Release();
		pNew2->Release();
	}
	else
	{
		aSplits.push_back( pNew1 );
		aSplits.push_back( pNew2 );

		cSplits += 2;
	}

	return ( cSplits );
}


//---------------------------------------------------------------------
/** Determine whether or not the patch is diceable
 */

TqBool	CqSurfacePatchBilinear::Diceable()
{
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
	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute("System", "ShadingRate")[0];
	if ( poptGridSize )
		gridsize = poptGridSize[ 0 ];
	else
		gridsize = static_cast<TqInt>( m_XBucketSize * m_XBucketSize / ShadingRate );
	for ( i = 0; i < 4; i++ )
		avecHull[ i ] = matCtoR * P() [ i ];

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
	m_uDiceSize = CEIL_POW2(m_uDiceSize);
	m_vDiceSize = CEIL_POW2(m_vDiceSize);

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
/** Transform the patch by the specified matrix.
 */

void	CqSurfacePatchBilinear::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx )
{
	// Tansform the control hull by the specified matrix.
	TqInt i;
	for ( i = 0; i < 4; i++ )
	{
		P() [ i ] = matTx * P() [ i ];
		if ( N().Size() == 4 ) N() [ i ] = matITTx * N() [ i ];
	}
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
	// Get the boundary in camera space.
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqUint i;
	for ( i = 0; i < P().Size(); i++ )
	{
		CqVector3D	vecV = P() [ i ];
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
/** Transform the patch by the specified matrix.
 */

void CqSurfacePatchMeshBicubic::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx )
{
	// Tansform the control hull by the specified matrix.
	TqUint i;
	for ( i = 0; i < P().Size(); i++ )
		P() [ i ] = matTx * P() [ i ];
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
	TqInt uStep = pAttributes() ->GetIntegerAttribute("System", "BasisStep")[0];
	TqInt vStep = pAttributes() ->GetIntegerAttribute("System", "BasisStep")[1];

	TqInt nvaryingu = ( m_uPeriodic ) ? m_uPatches : m_uPatches + 1;
	TqInt nvaryingv = ( m_vPeriodic ) ? m_vPatches : m_vPatches + 1;

	TqInt MyUses = Uses();

	const TqFloat* pTC = pAttributes() ->GetFloatAttribute("System", "TextureCoordinates");
	CqVector2D st1( pTC[0], pTC[1]);
	CqVector2D st2( pTC[2], pTC[3]);
	CqVector2D st3( pTC[4], pTC[5]);
	CqVector2D st4( pTC[6], pTC[7]);

	// Fill in the points
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

			pSurface->P().SetSize( pSurface->cVertex() );
			RtInt v;
			for ( v = 0; v < 4; v++ )
			{
				iP = PatchCoord( vRow + v, uCol );
				pSurface->P() [ ( v * 4 ) ] = P() [ iP ];
				iP = PatchCoord( vRow + v, uCol + 1 );
				pSurface->P() [ ( v * 4 ) + 1 ] = P() [ iP ];
				iP = PatchCoord( vRow + v, uCol + 2 );
				pSurface->P() [ ( v * 4 ) + 2 ] = P() [ iP ];
				iP = PatchCoord( vRow + v, uCol + 3 );
				pSurface->P() [ ( v * 4 ) + 3 ] = P() [ iP ];
			}

			TqInt iTa = PatchCorner( i, j );
			TqInt iTb = PatchCorner( i, j + 1 );
			TqInt iTc = PatchCorner( i + 1, j );
			TqInt iTd = PatchCorner( i + 1, j + 1 );

			TqFloat u0 = ( 1.0f / m_uPatches ) * j;
			TqFloat u1 = ( 1.0f / m_uPatches ) * ( j + 1 );

			// Copy any user specified primitive variables.
			std::vector<CqParameter*>::iterator iUP;
			for( iUP = aUserParams().begin(); iUP != aUserParams().end(); iUP++ )
			{
				if( (*iUP)->Class() == class_varying )
				{
					CqParameter* pNewUP = (*iUP)->Clone();
					pNewUP->Clear();
					pNewUP->SetSize( pSurface->cVarying() );

					pNewUP->SetValue( (*iUP), 0, iTa );
					pNewUP->SetValue( (*iUP), 1, iTb );
					pNewUP->SetValue( (*iUP), 2, iTc );
					pNewUP->SetValue( (*iUP), 3, iTd );
					pSurface->AddPrimitiveVariable(pNewUP);
				}
				else if( (*iUP)->Class() == class_vertex )
				{
					CqParameter* pNewUP = (*iUP)->Clone();
					pNewUP->Clear();
					pNewUP->SetSize( pSurface->cVertex() );

					for ( v = 0; v < 4; v++ )
					{
						iP = PatchCoord( vRow + v, uCol );
						pNewUP->SetValue( (*iUP), ( v * 4 ), iP );
						iP = PatchCoord( vRow + v, uCol + 1 );
						pNewUP->SetValue( (*iUP), ( v * 4 ) + 1, iP );
						iP = PatchCoord( vRow + v, uCol + 2 );
						pNewUP->SetValue( (*iUP), ( v * 4 ) + 2, iP );
						iP = PatchCoord( vRow + v, uCol + 3 );
						pNewUP->SetValue( (*iUP), ( v * 4 ) + 3, iP );
					}
					pSurface->AddPrimitiveVariable(pNewUP);
				}
			}

			// If the shaders need u/v or s/t and they are not specified, then we need to put them in as defaults.
			if( USES( MyUses, EnvVars_u ) && !bHasu() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("u") );
				pSurface->u()->SetSize(4);
				pSurface->u()->pValue( 0 )[0] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u0, v0 );
				pSurface->u()->pValue( 1 )[0] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u1, v0 );
				pSurface->u()->pValue( 2 )[0] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u0, v1 );
				pSurface->u()->pValue( 3 )[0] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u1, v1 );
			}

			if( USES( MyUses, EnvVars_v ) && !bHasv() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("v") );
				pSurface->v()->SetSize(4);
				pSurface->v()->pValue( 0 )[0] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u0, v0 );
				pSurface->v()->pValue( 1 )[0] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u1, v0 );
				pSurface->v()->pValue( 2 )[0] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u0, v1 );
				pSurface->v()->pValue( 3 )[0] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u1, v1 );
			}

			if( USES( MyUses, EnvVars_s ) && !bHass() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("s") );
				pSurface->s()->SetSize(4);
				pSurface->s()->pValue( 0 )[0] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u0, v0 );
				pSurface->s()->pValue( 1 )[0] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u1, v0 );
				pSurface->s()->pValue( 2 )[0] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u0, v1 );
				pSurface->s()->pValue( 3 )[0] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u1, v1 );
			}

			if( USES( MyUses, EnvVars_t ) && !bHast() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("t") );
				pSurface->t()->SetSize(4);
				pSurface->t()->pValue( 0 )[0] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u0, v0 );
				pSurface->t()->pValue( 1 )[0] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u1, v0 );
				pSurface->t()->pValue( 2 )[0] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u0, v1 );
				pSurface->t()->pValue( 3 )[0] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u1, v1 );
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
	// Get the boundary in camera space.
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqUint i;
	for ( i = 0; i < P().Size(); i++ )
	{
		CqVector3D	vecV = P() [ i ];
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
/** Transform the patch by the specified matrix.
 */

void CqSurfacePatchMeshBilinear::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx )
{
	// Tansform the control hull by the specified matrix.
	TqUint i;
	for ( i = 0; i < P().Size(); i++ )
		P() [ i ] = matTx * P() [ i ];
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

	const TqFloat* pTC = pAttributes() ->GetFloatAttribute("System", "TextureCoordinates");
	CqVector2D st1( pTC[0], pTC[1]);
	CqVector2D st2( pTC[2], pTC[3]);
	CqVector2D st3( pTC[4], pTC[5]);
	CqVector2D st4( pTC[6], pTC[7]);

	TqInt i;
	for ( i = 0; i < m_vPatches; i++ )      	// Fill in the points
	{
		TqFloat v0 = ( 1.0f / m_vPatches ) * i;
		TqFloat v1 = ( 1.0f / m_vPatches ) * ( i + 1 );
		RtInt j;
		for ( j = 0; j < m_uPatches; j++ )
		{
			CqSurfacePatchBilinear*	pSurface = new CqSurfacePatchBilinear;
			pSurface->AddRef();
			pSurface->SetSurfaceParameters( *this );
			pSurface->P().SetSize( 4 );

			// Calculate the position in the point table for u taking into account
			// periodic patches.
			iP = PatchCoord( i, j );
			pSurface->P() [ 0 ] = P() [ iP ];
			iP = PatchCoord( i, j + 1 );
			pSurface->P() [ 1 ] = P() [ iP ];
			iP = PatchCoord( i + 1, j );
			pSurface->P() [ 2 ] = P() [ iP ];
			iP = PatchCoord( i + 1, j + 1 );
			pSurface->P() [ 3 ] = P() [ iP ];

			RtInt iTa = PatchCoord( i, j );
			RtInt iTb = PatchCoord( i, j + 1 );
			RtInt iTc = PatchCoord( i + 1, j );
			RtInt iTd = PatchCoord( i + 1, j + 1 );

			TqFloat u0 = ( 1.0f / m_uPatches ) * j;
			TqFloat u1 = ( 1.0f / m_uPatches ) * ( j + 1 );

			// Copy any user specified primitive variables.
			std::vector<CqParameter*>::iterator iUP;
			for( iUP = aUserParams().begin(); iUP != aUserParams().end(); iUP++ )
			{
				if( (*iUP)->Class() == class_varying )
				{
					CqParameter* pNewUP = (*iUP)->Clone();
					pNewUP->Clear();
					pNewUP->SetSize( pSurface->cVarying() );

					pNewUP->SetValue( (*iUP), 0, iTa );
					pNewUP->SetValue( (*iUP), 1, iTb );
					pNewUP->SetValue( (*iUP), 2, iTc );
					pNewUP->SetValue( (*iUP), 3, iTd );

					pSurface->AddPrimitiveVariable(pNewUP);
				}
				else if( (*iUP)->Class() == class_vertex )
				{
					CqParameter* pNewUP = (*iUP)->Clone();
					pNewUP->Clear();
					pNewUP->SetSize( pSurface->cVertex() );

					iP = PatchCoord( i, j );
					pNewUP->SetValue( (*iUP), 0, iP );
					iP = PatchCoord( i, j + 1 );
					pNewUP->SetValue( (*iUP), 1, iP );
					iP = PatchCoord( i + 1, j );
					pNewUP->SetValue( (*iUP), 2, iP );
					iP = PatchCoord( i + 1, j + 1 );
					pNewUP->SetValue( (*iUP), 3, iP );

					pSurface->AddPrimitiveVariable(pNewUP);
				}
			}

			// If the shaders need u/v or s/t and they are not specified, then we need to put them in as defaults.
			if( USES( MyUses, EnvVars_u ) && !bHasu() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("u") );
				pSurface->u()->SetSize(4);
				pSurface->u()->pValue( 0 )[0] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u0, v0 );
				pSurface->u()->pValue( 1 )[0] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u1, v0 );
				pSurface->u()->pValue( 2 )[0] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u0, v1 );
				pSurface->u()->pValue( 3 )[0] = BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u1, v1 );
			}

			if( USES( MyUses, EnvVars_v ) && !bHasv() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("v") );
				pSurface->v()->SetSize(4);
				pSurface->v()->pValue( 0 )[0] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u0, v0 );
				pSurface->v()->pValue( 1 )[0] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u1, v0 );
				pSurface->v()->pValue( 2 )[0] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u0, v1 );
				pSurface->v()->pValue( 3 )[0] = BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u1, v1 );
			}

			if( USES( MyUses, EnvVars_s ) && !bHass() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("s") );
				pSurface->s()->SetSize(4);
				pSurface->s()->pValue( 0 )[0] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u0, v0 );
				pSurface->s()->pValue( 1 )[0] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u1, v0 );
				pSurface->s()->pValue( 2 )[0] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u0, v1 );
				pSurface->s()->pValue( 3 )[0] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u1, v1 );
			}

			if( USES( MyUses, EnvVars_t ) && !bHast() )
			{
				pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("t") );
				pSurface->t()->SetSize(4);
				pSurface->t()->pValue( 0 )[0] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u0, v0 );
				pSurface->t()->pValue( 1 )[0] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u1, v0 );
				pSurface->t()->pValue( 2 )[0] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u0, v1 );
				pSurface->t()->pValue( 3 )[0] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u1, v1 );
			}

			aSplits.push_back( pSurface );
			cSplits++;
		}
	}
	return ( cSplits );
}


END_NAMESPACE( Aqsis )
