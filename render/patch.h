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

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE( Aqsis )


//----------------------------------------------------------------------
/** \class CqSurfacePatchBicubic
 * Bicubic spline patch
 */

class CqSurfacePatchBicubic : public CqSurface
{
	public:
		CqSurfacePatchBicubic();
		CqSurfacePatchBicubic( const CqSurfacePatchBicubic& From );
		virtual	~CqSurfacePatchBicubic();

		template<class T, class SLT>
		void	TypedNaturalInterpolate( TqFloat uSize, TqFloat vSize, CqParameterTyped<T,SLT>* pParam, IqShaderData* pData )
		{
			CqForwardDiffBezier<T> vFD0( 1.0f / vSize );
			CqForwardDiffBezier<T> vFD1( 1.0f / vSize );
			CqForwardDiffBezier<T> vFD2( 1.0f / vSize );
			CqForwardDiffBezier<T> vFD3( 1.0f / vSize );
			CqForwardDiffBezier<T> uFD0( 1.0f / uSize );

			vFD0.CalcForwardDiff( pParam->pValue()[0], pParam->pValue()[4], pParam->pValue()[8], pParam->pValue()[12] );
			vFD1.CalcForwardDiff( pParam->pValue()[1], pParam->pValue()[5], pParam->pValue()[9], pParam->pValue()[13] );
			vFD2.CalcForwardDiff( pParam->pValue()[2], pParam->pValue()[6], pParam->pValue()[10], pParam->pValue()[14] );
			vFD3.CalcForwardDiff( pParam->pValue()[3], pParam->pValue()[7], pParam->pValue()[11], pParam->pValue()[15] );

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
					TqInt igrid = ( iv * ( uSize + 1 ) ) + iu;
					pData->SetValue( static_cast<SLT>(vec), igrid );
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
			return ( (*P()) [ ( iRow * 4 ) + iCol ] );
		}
		/** Get a reference to the indexed control point.
		 * \param iRow Integer row index.
		 * \param iCol Integer column index.
		 * \return CqVector4D reference.
		 */
		CqVector4D& CP( TqInt iRow, TqInt iCol )
		{
			return ( (*P()) [ ( iRow * 4 ) + iCol ] );
		}
		/** Get a reference to the basis matrix for the u direction.
		 */
		const	CqMatrix&	matuBasis()
		{
			return ( pAttributes() ->GetMatrixAttribute("System", "Basis")[0] );
		}
		/** Get a reference to the basis matrix for the v direction.
		 */
		const	CqMatrix&	matvBasis()
		{
			return ( pAttributes() ->GetMatrixAttribute("System", "Basis")[1] );
		}
		CqSurfacePatchBicubic& operator=( const CqSurfacePatchBicubic& From );

		void uSubdivide(CqSurfacePatchBicubic* pNew1, CqSurfacePatchBicubic* pNew2);
		void vSubdivide(CqSurfacePatchBicubic* pNew1, CqSurfacePatchBicubic* pNew2);

		virtual	CqBound	Bound() const;
		virtual	TqInt	Split( std::vector<CqBasicSurface*>& aSplits );
		virtual TqBool	Diceable();

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx );
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

		virtual void NaturalInterpolate(CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData);

	protected:
};


//----------------------------------------------------------------------
/** \class CqSurfacePatchBilinear
 * Bilinear spline patch
 */

class _qShareC CqSurfacePatchBilinear : public CqSurface
{
	public:
		CqSurfacePatchBilinear();
		CqSurfacePatchBilinear( const CqSurfacePatchBilinear& From );
		virtual	~CqSurfacePatchBilinear();

		CqSurfacePatchBilinear& operator=( const CqSurfacePatchBilinear& From );

		void	GenNormals();
		void	uSubdivide(CqSurfacePatchBilinear* pNew1, CqSurfacePatchBilinear* pNew2);
		void	vSubdivide(CqSurfacePatchBilinear* pNew1, CqSurfacePatchBilinear* pNew2);

		virtual	CqBound	Bound() const;
		virtual	TqInt	Split( std::vector<CqBasicSurface*>& aSplits );
		virtual TqBool	Diceable();

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx );
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

		virtual void NaturalInterpolate(CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData);
		virtual TqBool		CanGenerateNormals() const	{ return( TqTrue ); }
		virtual	void		GenerateGeometricNormals( TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pNormals );

	protected:
};


//----------------------------------------------------------------------
/** \class CqSurfacePatchMeshBicubic
 * Bicubic spline patch mesh
 */

class CqSurfacePatchMeshBicubic : public CqSurface
{
	public:
		CqSurfacePatchMeshBicubic( TqInt nu, TqInt nv, TqBool uPeriodic = TqFalse, TqBool vPeriodic = TqFalse ) :
				CqSurface(),
				m_nu( nu ),
				m_nv( nv ),
				m_uPeriodic( uPeriodic ),
				m_vPeriodic( vPeriodic )
		{
			TqInt uStep = pAttributes() ->GetIntegerAttribute("System", "BasisStep")[0];
			TqInt vStep = pAttributes() ->GetIntegerAttribute("System", "BasisStep")[1];
			m_uPatches = ( uPeriodic ) ? nu / uStep : ( ( nu - 4 ) / uStep ) + 1;
			m_vPatches = ( vPeriodic ) ? nv / vStep : ( ( nv - 4 ) / vStep ) + 1;
		}
		CqSurfacePatchMeshBicubic( const CqSurfacePatchMeshBicubic& From );
		virtual	~CqSurfacePatchMeshBicubic();

		CqSurfacePatchMeshBicubic& operator=( const CqSurfacePatchMeshBicubic& From );

		virtual	void	SetDefaultPrimitiveVariables( TqBool bUseDef_st = TqTrue ) {}

		/** Get a reference to the basis matrix for the u direction.
		 */
		const	CqMatrix&	matuBasis()
		{
			return ( pAttributes() ->GetMatrixAttribute("System", "Basis")[0] );
		}
		/** Get a reference to the basis matrix for the v direction.
		 */
		const	CqMatrix&	matvBasis()
		{
			return ( pAttributes() ->GetMatrixAttribute("System", "Basis")[1] );
		}

		virtual	CqBound	Bound() const;
		virtual	CqMicroPolyGridBase* Dice()
		{
			return ( 0 );
		}
		virtual	TqInt	Split( std::vector<CqBasicSurface*>& aSplits );
		virtual TqBool	Diceable()
		{
			return ( TqFalse );
		}

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx );
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

	protected:

		TqInt	m_uPatches,  			///< Number of patches in u.
		m_vPatches;			///< Number of patches in v.
		TqInt	m_nu,  				///< Number of control points in u.
		m_nv;				///< Number of control points in v.
		TqBool	m_uPeriodic,  		///< Is patches mesh periodic in u?
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
		CqSurfacePatchMeshBilinear( const CqSurfacePatchMeshBilinear& From );
		virtual	~CqSurfacePatchMeshBilinear();

		virtual	void	SetDefaultPrimitiveVariables( TqBool bUseDef_st = TqTrue ) {}

		CqSurfacePatchMeshBilinear& operator=( const CqSurfacePatchMeshBilinear& From );

		virtual	CqBound	Bound() const;
		virtual	CqMicroPolyGridBase* Dice()
		{
			return ( 0 );
		}
		virtual	TqInt	Split( std::vector<CqBasicSurface*>& aSplits );
		virtual TqBool	Diceable()
		{
			return ( TqFalse );
		}

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx );
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

	protected:

		TqInt	m_uPatches,  			///< Number of patches in u.
		m_vPatches;			///< Number of patches in v.
		TqInt	m_nu,  				///< Number of control points in u.
		m_nv;				///< Number of control points in v.
		TqBool	m_uPeriodic,  		///< Is patches mesh periodic in u?
		m_vPeriodic;		///< Is patches mesh periodic in v?
}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !PATCH_H_INCLUDED
