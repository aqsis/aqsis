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
		\brief Declares the classes and support structures for handling RenderMan patch primitives.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef PATCH_H_INCLUDED
#define PATCH_H_INCLUDED 1

#include	"aqsis.h"

#include	"matrix.h"
#include	"surface.h"
#include	"vector4d.h"
#include	"bilinear.h"
#include	"forwarddiff.h"

START_NAMESPACE( Aqsis )


//----------------------------------------------------------------------
/** \class CqSurfacePatchBicubic
 * Bicubic spline patch
 */

class CqSurfacePatchBicubic : public CqSurface
{
	public:
		CqSurfacePatchBicubic();
		virtual	~CqSurfacePatchBicubic();

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqSurfacePatchBicubic");
		}
#endif

		template <class T, class SLT>
		void	TypedNaturalDice( TqFloat uSize, TqFloat vSize, CqParameterTyped<T, SLT>* pParam, IqShaderData* pData )
		{
			CqForwardDiffBezier<T> vFD0( 1.0f / vSize );
			CqForwardDiffBezier<T> vFD1( 1.0f / vSize );
			CqForwardDiffBezier<T> vFD2( 1.0f / vSize );
			CqForwardDiffBezier<T> vFD3( 1.0f / vSize );
			CqForwardDiffBezier<T> uFD0( 1.0f / uSize );

			vFD0.CalcForwardDiff( pParam->pValue() [ 0 ], pParam->pValue() [ 4 ], pParam->pValue() [ 8 ], pParam->pValue() [ 12 ] );
			vFD1.CalcForwardDiff( pParam->pValue() [ 1 ], pParam->pValue() [ 5 ], pParam->pValue() [ 9 ], pParam->pValue() [ 13 ] );
			vFD2.CalcForwardDiff( pParam->pValue() [ 2 ], pParam->pValue() [ 6 ], pParam->pValue() [ 10 ], pParam->pValue() [ 14 ] );
			vFD3.CalcForwardDiff( pParam->pValue() [ 3 ], pParam->pValue() [ 7 ], pParam->pValue() [ 11 ], pParam->pValue() [ 15 ] );

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
					pData->SetValue( static_cast<SLT>( vec ), igrid );
				}
			}
		}

		template <class T, class SLT>
		void	TypedNaturalSubdivide( CqParameterTyped<T, SLT>* pParam, CqParameterTyped<T, SLT>* pResult1, CqParameterTyped<T, SLT>* pResult2, TqBool u )
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

		/** Get a reference to the indexed control point.
		 * \param iRow Integer row index.
		 * \param iCol Integer column index.
		 * \return CqVector4D reference.
		 */
		const	CqVector4D& CP( TqInt iRow, TqInt iCol ) const
		{
			return ( P()->pValue( ( iRow * 4 ) + iCol )[0] );
		}
		/** Get a reference to the indexed control point.
		 * \param iRow Integer row index.
		 * \param iCol Integer column index.
		 * \return CqVector4D reference.
		 */
		CqVector4D& CP( TqInt iRow, TqInt iCol )
		{
			return ( P()->pValue( ( iRow * 4 ) + iCol )[0] );
		}

		virtual	CqBound	Bound() const;
		virtual TqBool	Diceable();

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual TqBool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( TqFalse );
		}

		virtual	TqUint	cUniform() const
		{
			return ( 1 );
		}
		virtual	TqUint	cVarying() const
		{
			return ( 4 );
		}
		virtual	TqUint	cVertex() const
		{
			return ( 16 );
		}
		virtual	TqUint cFaceVarying() const
		{
			/// \todo Must work out what this value should be.
			return ( 1 );
		}

		virtual void NaturalDice( CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData );
		virtual	TqInt PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, TqBool u );
		virtual void NaturalSubdivide( CqParameter* pParam, CqParameter* pParam1, CqParameter* pParam2, TqBool u );

		void	ConvertToBezierBasis( CqMatrix& matuBasis, CqMatrix& matvBasis );

		virtual CqSurface* Clone() const;

	protected:
};


//----------------------------------------------------------------------
/** \class CqSurfacePatchBilinear
 * Bilinear spline patch
 */

class CqSurfacePatchBilinear : public CqSurface
{
	public:
		CqSurfacePatchBilinear();
		virtual	~CqSurfacePatchBilinear();

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqSurfacePatchBilinear");
		}
#endif

		void	SetfHasPhantomFourthVertex(TqBool fHasPhantomFourthVertex)
		{
			m_fHasPhantomFourthVertex = fHasPhantomFourthVertex;
		}

		TqBool	fHasPhantomFourthVertex() const
		{
			return(m_fHasPhantomFourthVertex);
		}

		void	GenNormals();
		virtual	CqBound	Bound() const;
		virtual TqBool	Diceable();

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual TqBool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( TqFalse );
		}

		virtual	TqUint	cUniform() const
		{
			return ( 1 );
		}
		virtual	TqUint	cVarying() const
		{
			return ( 4 );
		}
		virtual	TqUint	cVertex() const
		{
			return ( 4 );
		}
		virtual	TqUint	cFaceVarying() const
		{
			/// \todo Must work out what this value should be.
			return ( 1 );
		}
		virtual CqSurface* Clone() const;

		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual	TqInt	PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, TqBool u );
		virtual void	PostDice(CqMicroPolyGrid * pGrid)
		{
			if(m_fHasPhantomFourthVertex)
				pGrid->SetfTriangular(TqTrue);
		}

	protected:
		TqBool	m_fHasPhantomFourthVertex;
		TqInt	m_iInternalu;
		TqInt	m_iInternalv;
};


//----------------------------------------------------------------------
/** \class CqSurfacePatchMeshBicubic
 * Bicubic spline patch mesh
 */

class CqSurfacePatchMeshBicubic : public CqSurface
{
	public:
		CqSurfacePatchMeshBicubic() :
				CqSurface(),
				m_nu( 0 ),
				m_nv( 0 ),
				m_uPeriodic( TqFalse ),
				m_vPeriodic( TqFalse )
		{}
		CqSurfacePatchMeshBicubic( TqInt nu, TqInt nv, TqBool uPeriodic = TqFalse, TqBool vPeriodic = TqFalse ) :
				CqSurface(),
				m_nu( nu ),
				m_nv( nv ),
				m_uPeriodic( uPeriodic ),
				m_vPeriodic( vPeriodic )
		{
			TqInt uStep = pAttributes() ->GetIntegerAttribute( "System", "BasisStep" ) [ 0 ];
			TqInt vStep = pAttributes() ->GetIntegerAttribute( "System", "BasisStep" ) [ 1 ];
			m_uPatches = ( uPeriodic ) ? nu / uStep : ( ( nu - 4 ) / uStep ) + 1;
			m_vPatches = ( vPeriodic ) ? nv / vStep : ( ( nv - 4 ) / vStep ) + 1;
		}
		virtual	~CqSurfacePatchMeshBicubic();

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqSurfacePatchMeshBicubic");
		}
#endif

		virtual	void	SetDefaultPrimitiveVariables( TqBool bUseDef_st = TqTrue )
		{}

		virtual	CqBound	Bound() const;
		virtual	CqMicroPolyGridBase* Dice()
		{
			return ( 0 );
		}
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual TqBool	Diceable()
		{
			return ( TqFalse );
		}

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual TqBool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( TqFalse );
		}

		virtual	TqUint	cUniform() const
		{
			return ( m_uPatches * m_vPatches );
		}
		virtual	TqUint	cVarying() const
		{
			return ( ( ( m_uPeriodic ) ? m_uPatches : m_uPatches + 1 ) * ( ( m_vPeriodic ) ? m_vPatches : m_vPatches + 1 ) );
		}
		virtual	TqUint	cVertex() const
		{
			return ( m_nu * m_nv );
		}
		virtual	TqUint	cFaceVarying() const
		{
			/// \todo Must work out what this value should be.
			return ( 1 );
		}
		virtual CqSurface* Clone() const;

		virtual CqVector3D	SurfaceParametersAtVertex( TqInt index )
		{
			CqVector3D	vec( 0, 0, 0 );
			TqFloat u = static_cast<TqFloat>( index % m_nu );
			u /= ( m_nu - 1 );
			TqFloat v = static_cast<TqFloat>( index / m_nu );
			v /= ( m_nv - 1 );
			vec.x( u );
			vec.y( v );
			return ( vec );
		}

	protected:

		TqInt	m_uPatches,   			///< Number of patches in u.
		m_vPatches;			///< Number of patches in v.
		TqInt	m_nu,   				///< Number of control points in u.
		m_nv;				///< Number of control points in v.
		TqBool	m_uPeriodic,   		///< Is patches mesh periodic in u?
		m_vPeriodic;		///< Is patches mesh periodic in v?
}
;


//----------------------------------------------------------------------
/** \class CqSurfacePatchMeshlinear
 * Bilinear spline patch mesh
 */

class CqSurfacePatchMeshBilinear : public CqSurface
{
	public:
		CqSurfacePatchMeshBilinear() :
				CqSurface(),
				m_nu( 0 ),
				m_nv( 0 ),
				m_uPeriodic( TqFalse ),
				m_vPeriodic( TqFalse )
		{}
		CqSurfacePatchMeshBilinear( TqInt nu, TqInt nv, TqBool uPeriodic = TqFalse, TqBool vPeriodic = TqFalse ) :
				CqSurface(),
				m_nu( nu ),
				m_nv( nv ),
				m_uPeriodic( uPeriodic ),
				m_vPeriodic( vPeriodic )
		{
			m_uPatches = ( uPeriodic ) ? nu : nu - 1;
			m_vPatches = ( vPeriodic ) ? nv : nv - 1;
		}
		virtual	~CqSurfacePatchMeshBilinear();

		virtual	void	SetDefaultPrimitiveVariables( TqBool bUseDef_st = TqTrue )
		{}

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqSurfacePatchMeshBilinear");
		}
#endif

		virtual	CqBound	Bound() const;
		virtual	CqMicroPolyGridBase* Dice()
		{
			return ( 0 );
		}
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual TqBool	Diceable()
		{
			return ( TqFalse );
		}

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual TqBool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( TqFalse );
		}

		virtual	TqUint	cUniform() const
		{
			return ( m_uPatches * m_vPatches );
		}
		virtual	TqUint	cVarying() const
		{
			return ( ( ( m_uPeriodic ) ? m_uPatches : m_uPatches + 1 ) * ( ( m_vPeriodic ) ? m_vPatches : m_vPatches + 1 ) );
		}
		virtual	TqUint	cVertex() const
		{
			return ( m_nu * m_nv );
		}
		virtual	TqUint	cFaceVarying() const
		{
			/// \todo Must work out what this value should be.
			return ( 1 );
		}
		virtual CqSurface* Clone() const;

		virtual CqVector3D	SurfaceParametersAtVertex( TqInt index )
		{
			CqVector3D	vec( 0, 0, 0 );
			TqFloat u = static_cast<TqFloat>( index % m_nu );
			u /= ( m_nu - 1 );
			TqFloat v = static_cast<TqFloat>( index / m_nu );
			v /= ( m_nv - 1 );
			vec.x( u );
			vec.y( v );
			return ( vec );
		}

	protected:

		TqInt	m_uPatches,   			///< Number of patches in u.
		m_vPatches;			///< Number of patches in v.
		TqInt	m_nu,   				///< Number of control points in u.
		m_nv;				///< Number of control points in v.
		TqBool	m_uPeriodic,   		///< Is patches mesh periodic in u?
		m_vPeriodic;		///< Is patches mesh periodic in v?
}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !PATCH_H_INCLUDED
