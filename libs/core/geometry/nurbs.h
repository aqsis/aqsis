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
		\brief Declares the CqSurfaceNurbs classes for handling Renderman NURBS primitives.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef NURBS_H_INCLUDED
#define NURBS_H_INCLUDED 1

#include	<vector>

#include	<aqsis/aqsis.h>

#include	<aqsis/ri/ri.h>
#include	<aqsis/math/vector4d.h>
#include	<aqsis/math/vector2d.h>
#include	"surface.h"
#include	"trimcurve.h"

namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqSurfaceNURBS
 * RenderMan NURBS surface.
 */

class CqSurfaceNURBS : public CqSurface
{
	private:
	public:
		CqSurfaceNURBS();
		virtual	~CqSurfaceNURBS()
		{}

		void	Torus();

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqSurfaceNURBS");
		}
#endif

		/** Get the order of the NURBS surface in the u direction.
		 */
		TqUint	uOrder() const
		{
			return ( m_uOrder );
		}
		/** Get the order of the NURBS surface in the v direction.
		 */
		TqUint	vOrder() const
		{
			return ( m_vOrder );
		}
		/** Get the degree of the NURBS surface in the u direction.
		 */
		TqUint	uDegree() const
		{
			return ( m_uOrder -1 );
		}
		/** Get the degree of the NURBS surface in the v direction.
		 */
		TqUint	vDegree() const
		{
			return ( m_vOrder -1 );
		}
		/** Get the number of control points in the u direction.
		 */
		TqUint	cuVerts() const
		{
			return ( m_cuVerts );
		}
		/** Get the number of control points in the v direction.
		 */
		TqUint	cvVerts() const
		{
			return ( m_cvVerts );
		}
		/** Get the length of the knot vector for the u direction.
		 */
		TqUint	cuKnots() const
		{
			return ( m_cuVerts + m_uOrder );
		}
		/** Get the length of the knot vector for the v direction.
		 */
		TqUint	cvKnots() const
		{
			return ( m_cvVerts + m_vOrder );
		}
		/** Get the minimum u value of the surface.
		 */
		TqFloat	umin() const
		{
			return ( m_umin );
		}
		/** Set the minimum u value of the surface.
		 */
		void	Setumin( TqFloat umin )
		{
			m_umin = umin;
		}
		/** Get the minimum v value of the surface.
		 */
		TqFloat	vmin() const
		{
			return ( m_vmin );
		}
		/** Set the minimum v value of the surface.
		 */
		void	Setvmin( TqFloat vmin )
		{
			m_vmin = vmin;
		}
		/** Get the maximum u value of the surface.
		 */
		TqFloat	umax() const
		{
			return ( m_umax );
		}
		/** Set the maximum u value of the surface.
		 */
		void	Setumax( TqFloat umax )
		{
			m_umax = umax;
		}
		/** Get the maximum v value of the surface.
		 */
		TqFloat	vmax() const
		{
			return ( m_vmax );
		}
		/** Set the maximum v value of the surface.
		 */
		void	Setvmax( TqFloat vmax )
		{
			m_vmax = vmax;
		}
		/** Get a reference to the knot vector for the u direction.
		 */
		std::vector<TqFloat>& auKnots()
		{
			return ( m_auKnots );
		}
		/** Get a reference to the knot vector for the v direction.
		 */
		std::vector<TqFloat>& avKnots()
		{
			return ( m_avKnots );
		}
		/** Determine how many segments in u for this surface patch.
		 */
		TqInt cuSegments() const
		{
			return ( 1 + m_cuVerts - m_uOrder );
		}
		/** Determine how many segments in v for this surface patch.
		 */
		TqInt cvSegments() const
		{
			return ( 1 + m_cvVerts - m_vOrder );
		}
		/** Determine whether this surface is a mesh, and needs to be split into segments before continuing.
		 */
		bool fPatchMesh() const
		{
			return ( m_fPatchMesh );
		}
		/** Mark this mesh as being part of a mesh or not.
		 */
		void SetfPatchMesh( bool fPatchMesh = true )
		{
			m_fPatchMesh = fPatchMesh;
		}


		TqInt	operator==( const CqSurfaceNURBS& from );
		/** Get the control point at the specified u,v index.
		 * \param u Index in the u direction.
		 * \param v Index in the v direction.
		 * \return Reference to the 4D homogenous control point.
		 */
		CqVector4D&	CP( const TqUint u, TqUint v )
		{
			return ( P()->pValue( ( v * m_cuVerts ) + u )[0] );
		}
		/** Get the control point at the specified u,v index.
		 * \param u Index in the u direction.
		 * \param v Index in the v direction.
		 * \return Reference to the 4D homogenous control point.
		 */
		const	CqVector4D&	CP( const TqUint u, TqUint v ) const
		{
			return ( P()->pValue( ( v * m_cuVerts ) + u )[0] );
		}

		/** Initialise the NURBS structures to take a NURBS surfafe of the specified dimensions.
		 * \param uOrder The required order in the u direction.
		 * \param vOrder The required order in the v direction.
		 * \param cuVerts The required control point count in the u direction.
		 * \param cvVerts The required control point count in the v direction.
		 */
		void	Init( TqUint uOrder, TqUint vOrder, TqUint cuVerts, TqUint cvVerts )
		{
			TqUint uKnots = cuVerts + uOrder;
			TqUint vKnots = cvVerts + vOrder;
			m_auKnots.resize( uKnots );
			m_avKnots.resize( vKnots );
			m_uOrder = uOrder;
			m_vOrder = vOrder;
			m_cuVerts = cuVerts;
			m_cvVerts = cvVerts;
		}
		TqUint	FindSpanU( TqFloat u ) const;
		TqUint	FindSpanV( TqFloat v ) const;
		void	BasisFunctions( TqFloat u, TqUint span, std::vector<TqFloat>& aKnots, TqInt k, std::vector<TqFloat>& BasisVals );
		void	DersBasisFunctions( TqFloat u, TqUint i, std::vector<TqFloat>& U, TqInt k, TqInt n, std::vector<std::vector<TqFloat> >& ders );

		template <class T, class SLT>
		T	Evaluate( TqFloat u, TqFloat v, CqParameterTyped<T, SLT>* pParam, TqInt arrayIndex = 0 )
		{
			std::vector<TqFloat> Nu( m_uOrder );
			std::vector<TqFloat> Nv( m_vOrder );

			/* Evaluate non-uniform basis functions (and derivatives) */

			TqUint uspan = FindSpanU( u );
			BasisFunctions( u, uspan, m_auKnots, m_uOrder, Nu );
			TqUint vspan = FindSpanV( v );
			BasisFunctions( v, vspan, m_avKnots, m_vOrder, Nv );
			TqUint uind = uspan - uDegree();

			T S = T();
			TqUint l, k;
			for ( l = 0; l <= vDegree(); l++ )
			{
				T temp = T();
				TqUint vind = vspan - vDegree() + l;
				for ( k = 0; k <= uDegree(); k++ )
					temp = static_cast<T>( temp + Nu[ k ] * ( pParam->pValue( ( vind * m_cuVerts ) + uind + k )[arrayIndex] ) );
				S = static_cast<T>( S + Nv[ l ] * temp );
			}
			return ( S );
		}

		CqVector4D	EvaluateWithNormal( TqFloat u, TqFloat v, CqVector4D& P );
		void	SplitNURBS( CqSurfaceNURBS& nrbA, CqSurfaceNURBS& nrbB, bool dirflag );
		void	SubdivideSegments( std::vector<boost::shared_ptr<CqSurfaceNURBS> >& Array );
		void	RefineKnotU( const std::vector<TqFloat>& X );
		void	RefineKnotV( const std::vector<TqFloat>& X );
		TqUint	InsertKnotU( TqFloat u, TqInt r );
		TqUint	InsertKnotV( TqFloat v, TqInt r );
		void	ClampU();
		void	ClampV();
		/** Clamp the surface to ensure the knot vectors are 0-1 in each direction.
		 */
		void	Clamp()
		{
			ClampU();
			ClampV();
		}

		void	OutputMesh();
		void	AppendMesh( const char* name, TqInt index );
		void	Output( const char* name );

		const CqTrimLoopArray&	TrimLoops() const
		{
			return ( m_TrimLoops );
		}
		CqTrimLoopArray& TrimLoops()
		{
			return ( m_TrimLoops );
		}

		// Function from CqSurface
		virtual void uSubdivide( CqSurfaceNURBS*& pnrbA, CqSurfaceNURBS*& pnrbB );
		virtual void vSubdivide( CqSurfaceNURBS*& pnrbA, CqSurfaceNURBS*& pnrbB );
		virtual void NaturalDice( CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData );

		virtual	void	Bound(CqBound* bound) const;
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual bool	Diceable(const CqMatrix& matCtoR);

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}

		virtual	void	SetDefaultPrimitiveVariables( bool bUseDef_st = true );

		/** Get the number of uniform variables for a NURBS surface.
		 */
		virtual	TqUint cUniform() const
		{
			TqInt nuSegments = ( 1 + m_cuVerts - m_uOrder );
			TqInt nvSegments = ( 1 + m_cvVerts - m_vOrder );
			return ( nuSegments * nvSegments );
		}
		/** Get the number of varying variables for a NURBS surface.
		 */
		virtual	TqUint cVarying() const
		{
			TqInt nuSegments = ( 1 + m_cuVerts - m_uOrder );
			TqInt nvSegments = ( 1 + m_cvVerts - m_vOrder );
			return ( ( nuSegments + 1 ) * ( nvSegments + 1 ) );
		}
		/** Get the number of vertex variables for a NURBS surface.
		 */
		virtual	TqUint cVertex() const
		{
			return ( m_cuVerts * m_cvVerts );
		}
		/** Get the number of varying variables for a NURBS surface.
		 */
		virtual	TqUint cFaceVarying() const
		{
			return ( cVarying() );
		}

		virtual const bool bCanBeTrimmed() const
		{
			return ( true );
		}
		virtual const bool bIsPointTrimmed( const CqVector2D& p ) const
		{
			return ( m_TrimLoops.TrimPoint( p ) );
		}
		virtual const bool bIsLineIntersecting( const CqVector2D& v1, const CqVector2D& v2 ) const
		{
			return ( m_TrimLoops.LineIntersects( v1, v2 ) );
		}
		virtual	TqInt	TrimDecimation( const CqTrimCurve& Curve );
		virtual	void	PrepareTrimCurve()
		{
			m_TrimLoops.Prepare( this );
		}
		virtual CqSurface* Clone() const;


	protected:
		std::vector<TqFloat>	m_auKnots;	///< Knot vector for the u direction.
		std::vector<TqFloat>	m_avKnots;	///< Knot vector for the v direction.
		TqUint	m_uOrder;	///< Surface order in the u direction.
		TqUint	m_vOrder;	///< Surface order in the v direction.
		TqUint	m_cuVerts;	///< Control point count in the u direction.
		TqUint	m_cvVerts;	///< Control point count in the v direction.
		TqFloat	m_umin;		///< Minimum value of u over surface.
		TqFloat m_umax;		///< Maximum value of u over surface.
		TqFloat	m_vmin;		///< Minimum value of v over surface.
		TqFloat m_vmax;		///< Maximum value of v over surface.
		CqTrimLoopArray	m_TrimLoops;	///< Local trim curves, prepared for this surface.
		bool	m_fPatchMesh;	///< Flag indicating this is an unsubdivided mesh.
}
;


//---------------------------------------------------------------------
/** Evaluate the nurbs surface at parameter values u,v.
 */



//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !NURBS_H_INCLUDED
