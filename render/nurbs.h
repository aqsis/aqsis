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
		\brief Declares the CqSurfaceNurbs classes for handling Renderman NURBS primitives.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef NURBS_H_INCLUDED
#define NURBS_H_INCLUDED 1

#include	<vector>

#include	"aqsis.h"

#include	"ri.h"
#include	"vector4d.h"
#include	"vector2d.h"
#include	"surface.h"
#include	"trimcurve.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqSurfaceNURBS
 * RenderMan NURBS surface.
 */

class CqSurfaceNURBS : public CqSurface
{
	private:
	public:
		CqSurfaceNURBS();
		CqSurfaceNURBS( const CqSurfaceNURBS& From );
		virtual	~CqSurfaceNURBS()
		{}

		void	Torus();

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
		void	Setumin(TqFloat umin)
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
		void	Setvmin(TqFloat vmin)
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
		void	Setumax(TqFloat umax)
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
		void	Setvmax(TqFloat vmax)
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
			return(1 + m_cuVerts - m_uOrder );
		}
		/** Determine how many segments in v for this surface patch.
		 */
		TqInt cvSegments() const
		{
			return(1 + m_cvVerts - m_vOrder );
		}

		void	operator=( const CqSurfaceNURBS& From );
		TqInt	operator==( const CqSurfaceNURBS& from );
		/** Get the control point at the specified u,v index.
		 * \param u Index in the u direction.
		 * \param v Index in the v direction.
		 * \return Reference to the 4D homogenous control point.
		 */
		CqVector4D&	CP( const TqUint u, TqUint v )
		{
			return ( P() [ ( v * m_cuVerts ) + u ] );
		}
		/** Get the control point at the specified u,v index.
		 * \param u Index in the u direction.
		 * \param v Index in the v direction.
		 * \return Reference to the 4D homogenous control point.
		 */
		const	CqVector4D&	CP( const TqUint u, TqUint v ) const
		{
			return ( P() [ ( v * m_cuVerts ) + u ] );
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
			P().SetSize( cuVerts * cvVerts );
			m_uOrder = uOrder;
			m_vOrder = vOrder;
			m_cuVerts = cuVerts;
			m_cvVerts = cvVerts;
		}
		TqUint	FindSpanU( TqFloat u ) const;
		TqUint	FindSpanV( TqFloat v ) const;
		void	BasisFunctions( TqFloat u, TqUint span, std::vector<TqFloat>& aKnots, TqInt k, std::vector<TqFloat>& BasisVals );
		void	DersBasisFunctions( TqFloat u, TqUint i, std::vector<TqFloat>& U, TqInt k, TqInt n, std::vector<std::vector<TqFloat> >& ders );
		CqVector4D	Evaluate( TqFloat u, TqFloat v );
		CqVector4D	EvaluateNormal( TqFloat u, TqFloat v );
		void	SplitNURBS( CqSurfaceNURBS& nrbA, CqSurfaceNURBS& nrbB, TqBool dirflag );
		void	Decompose( std::vector<CqSurfaceNURBS*>& Array );
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
			ClampU(); ClampV();
		}


		// Surface construction functions.
		void	SurfaceOfRevolution( const CqSurfaceNURBS& profile, const CqVector3D& S, const CqVector3D& Tvec, TqFloat theta );

		// Curve functions.
		void	Circle( const CqVector3D& O, const CqVector3D& X, const CqVector3D& Y, TqFloat r, TqFloat as, TqFloat ae );
		void	LineSegment( const CqVector3D& P1, CqVector3D& P2 );

		// Utility functions.
		TqBool	IntersectLine( CqVector3D& P1, CqVector3D& T1, CqVector3D& P2, CqVector3D& T2, CqVector3D& P );
		void	ProjectToLine( const CqVector3D& S, const CqVector3D& Trj, const CqVector3D& pnt, CqVector3D& p );

		void	OutputMesh();
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
		virtual void NaturalInterpolate(CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData);
		virtual TqBool CanGenerateNormals() const	{ return( TqTrue ); }
		virtual	void GenerateGeometricNormals( TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pNormals );

		virtual	CqBound	Bound() const;
		virtual	TqInt	Split( std::vector<CqBasicSurface*>& aSplits );
		virtual TqBool	Diceable();

		virtual	void	SetDefaultPrimitiveVariables( TqBool bUseDef_st = TqTrue );

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx );
		/** Get the number of uniform variables for a NURBS surface.
		 */
		virtual	TqUint cUniform() const
		{
			TqInt nuSegments = (1 + m_cuVerts - m_uOrder );
			TqInt nvSegments = (1 + m_cvVerts - m_vOrder );
			return ( nuSegments * nvSegments );
		}
		/** Get the number of varying variables for a NURBS surface.
		 */
		virtual	TqUint cVarying() const
		{
			TqInt nuSegments = (1 + m_cuVerts - m_uOrder );
			TqInt nvSegments = (1 + m_cvVerts - m_vOrder );
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
			/// \todo Must work out what this value should be.
			return ( 1 );
		}

		virtual const TqBool bCanBeTrimmed() const
		{
			return ( TqTrue );
		}
		virtual const TqBool bIsPointTrimmed( const CqVector2D& p ) const
		{
			return ( m_TrimLoops.TrimPoint( p ) );
		}
		virtual	TqInt	TrimDecimation( const CqTrimCurve& Curve );
		virtual	void	PrepareTrimCurve()
		{
			m_TrimLoops.Prepare( this );
		}


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
}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !NURBS_H_INCLUDED
