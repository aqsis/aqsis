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

#include	"specific.h"	// Needed for namespace macros.

#include	<vector>

#include	"ri.h"
#include	"vector4d.h"
#include	"vector2d.h"
#include	"surface.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \class CqSurfaceNURBS
 * RenderMan NURBS surface.
 */

class CqSurfaceNURBS : public CqSurface
{
	private:
	public:
				CqSurfaceNURBS();
				CqSurfaceNURBS(const CqSurfaceNURBS& From);
		virtual	~CqSurfaceNURBS()				{}

				void		Torus();

							/** Get the order of the NURBS surface in the u direction.
							 */
				TqInt		uOrder() const		{return(m_uOrder);}
							/** Get the order of the NURBS surface in the v direction.
							 */
				TqInt		vOrder() const		{return(m_vOrder);}
							/** Get the degree of the NURBS surface in the u direction.
							 */
				TqInt		uDegree() const		{return(m_uOrder-1);}
							/** Get the degree of the NURBS surface in the v direction.
							 */
				TqInt		vDegree() const		{return(m_vOrder-1);}
							/** Get the number of control points in the u direction.
							 */
				TqInt		cuVerts() const		{return(m_cuVerts);}
							/** Get the number of control points in the v direction.
							 */
				TqInt		cvVerts() const		{return(m_cvVerts);}
							/** Get the length of the knot vector for the u direction.
							 */
				TqInt		cuKnots() const		{return(m_cuVerts+m_uOrder);}
							/** Get the length of the knot vector for the v direction.
							 */
				TqInt		cvKnots() const		{return(m_cvVerts+m_vOrder);}
							/** Get a reference to the knot vector for the u direction.
							 */
				std::vector<TqFloat>& auKnots()	{return(m_auKnots);}
							/** Get a reference to the knot vector for the v direction.
							 */
				std::vector<TqFloat>& avKnots()	{return(m_avKnots);}

				void		operator=(const CqSurfaceNURBS& From);
				TqInt		operator==(const CqSurfaceNURBS& from);
							/** Get the control point at the specified u,v index.
							 * \param u Index in the u direction.
							 * \param v Index in the v direction.
							 * \return Reference to the 4D homogenous control point.
							 */
				CqVector4D&	CP(const TqInt u, TqInt v)	{return(P()[(v*m_cuVerts)+u]);}
							/** Get the control point at the specified u,v index.
							 * \param u Index in the u direction.
							 * \param v Index in the v direction.
							 * \return Reference to the 4D homogenous control point.
							 */
		const	CqVector4D&	CP(const TqInt u, TqInt v) const	
														{return(P()[(v*m_cuVerts)+u]);}

							/** Initialise the NURBS structures to take a NURBS surfafe of the specified dimensions.
							 * \param uOrder The required order in the u direction.
							 * \param vOrder The required order in the v direction.
							 * \param cuVerts The required control point count in the u direction.
							 * \param cvVerts The required control point count in the v direction.
							 */
				void		Init(TqInt uOrder, TqInt vOrder, TqInt cuVerts, TqInt cvVerts)
												{
													TqInt uKnots=cuVerts+uOrder;
													TqInt vKnots=cvVerts+vOrder;
													m_auKnots.resize(uKnots);
													m_avKnots.resize(vKnots);
													P().SetSize(cuVerts*cvVerts);
													m_uOrder=uOrder;
													m_vOrder=vOrder;
													m_cuVerts=cuVerts;
													m_cvVerts=cvVerts;
												}
				TqInt		FindSpanU(TqFloat u) const;
				TqInt		FindSpanV(TqFloat v) const;
				void		BasisFunctions(TqFloat u, TqInt span, std::vector<TqFloat>& aKnots, TqInt k, std::vector<TqFloat>& BasisVals);
				CqVector4D	Evaluate(TqFloat u, TqFloat v);
				void		SplitNURBS(CqSurfaceNURBS& nrbA, CqSurfaceNURBS& nrbB,	TqBool dirflag);
				void		Decompose(std::vector<CqSurfaceNURBS>& Array);
				void		RefineKnotU(const std::vector<TqFloat>& X);
				void		RefineKnotV(const std::vector<TqFloat>& X);
				TqInt		InsertKnotU(TqFloat u, TqInt r);
				TqInt		InsertKnotV(TqFloat v, TqInt r);
				void		ClampU();
				void		ClampV();
							/** Clamp the surface to ensure the knot vectors are 0-1 in each direction.
							 */
				void		Clamp()				{ClampU(); ClampV();}
				

				// Surface construction functions.
				void		SurfaceOfRevolution(const CqSurfaceNURBS& profile, const CqVector3D& S, const CqVector3D& Tvec, TqFloat theta);

				// Curve functions.
				void		Circle(const CqVector3D& O, const CqVector3D& X, const CqVector3D& Y, TqFloat r, TqFloat as, TqFloat ae);
				void		LineSegment(const CqVector3D& P1, CqVector3D& P2);

				// Utility functions.
				TqBool	IntersectLine(CqVector3D& P1, CqVector3D& T1, CqVector3D& P2, CqVector3D& T2, CqVector3D& P);
				void		ProjectToLine(const CqVector3D& S, const CqVector3D& Trj, const CqVector3D& pnt, CqVector3D& p);
				
				void		OutputMesh();
				void		Output(char* name);


				// Function from CqSurface
						void uSubdivide(CqSurfaceNURBS*& pnrbA, CqSurfaceNURBS*& pnrbB);
						void vSubdivide(CqSurfaceNURBS*& pnrbA, CqSurfaceNURBS*& pnrbB);

				virtual	CqBound		Bound() const;
				virtual	CqMicroPolyGridBase* Dice();
				virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits);
				virtual TqBool	Diceable();

				virtual void		Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx);
							/** Get the number of uniform variables for a NURBS surface.
							 */	
				virtual	TqInt cUniform() const	{return(1);}	// NOTE: These are WRONG, see appnote #13
							/** Get the number of varying variables for a NURBS surface.
							 */	
				virtual	TqInt cVarying() const	{return(4);}
							/** Get the number of vertex variables for a NURBS surface.
							 */	
				virtual	TqInt cVertex() const		{return(m_cuVerts*m_cvVerts);}

				
	protected:
				std::vector<TqFloat>	m_auKnots;	///< Knot vector for the u direction.
				std::vector<TqFloat>	m_avKnots;	///< Knot vector for the v direction.
				TqInt				m_uOrder;	///< Surface order in the u direction.
				TqInt				m_vOrder;	///< Surface order in the v direction.
				TqInt				m_cuVerts;	///< Control point count in the u direction.
				TqInt				m_cvVerts;	///< Control point count in the v direction.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !NURBS_H_INCLUDED
