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

#include	"matrix.h"
#include	"surface.h"
#include	"vector4d.h"
#include	"bilinear.h"

#include	"specific.h"	// Needed for namespace macros.

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)


//----------------------------------------------------------------------
/** \class CqSurfacePatchBicubic
 * Bicubic spline patch
 */

class CqSurfacePatchBicubic : public CqSurface
{
	public:
				CqSurfacePatchBicubic();
				CqSurfacePatchBicubic(const CqSurfacePatchBicubic& From);
		virtual	~CqSurfacePatchBicubic();

		virtual	CqVector4D	Evaluate(TqFloat s, TqFloat t)	const;

									/** Get a reference to the indexed control point.
									 * \param iRow Integer row index.
									 * \param iCol Integer column index.
									 * \return CqVector4D reference.
									 */
		const	CqVector4D& CP(TqInt iRow, TqInt iCol) const	{return(P()[(iRow*4)+iCol]); }
									/** Get a reference to the indexed control point.
									 * \param iRow Integer row index.
									 * \param iCol Integer column index.
									 * \return CqVector4D reference.
									 */
				CqVector4D& CP(TqInt iRow, TqInt iCol)		{return(P()[(iRow*4)+iCol]); }
									/** Get a reference to the basis atrix for the u direction.
									 */
		const	CqMatrix&	matuBasis()			{return(pAttributes()->matuBasis());}
									/** Get a reference to the basis atrix for the v direction.
									 */
		const	CqMatrix&	matvBasis()			{return(pAttributes()->matvBasis());}
				CqSurfacePatchBicubic& operator=(const CqSurfacePatchBicubic& From);

				CqSurfacePatchBicubic*	uSubdivide();
				CqSurfacePatchBicubic*	vSubdivide();

		virtual	CqBound		Bound() const;
		virtual	CqMicroPolyGridBase* Dice();
		virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits);
		virtual TqBool		Diceable();

		virtual void		Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx);
		virtual	TqInt		cUniform() const	{return(1);}
		virtual	TqInt		cVarying() const	{return(4);}
		virtual	TqInt		cVertex() const		{return(16);}

	protected:

		virtual	void		GetGeometryMatrices(TqFloat& s, TqFloat &t,CqMatrix& Gx,CqMatrix& Gy,CqMatrix& Gz) const;
				CqVector4D	EvaluateMatrix(TqFloat s, TqFloat t, CqMatrix& Gx, CqMatrix& Gy, CqMatrix& Gz) const;
				void		InitFD(TqInt cu, TqInt cv,
										 CqMatrix&	matDDx,
										 CqMatrix&	matDDy,
										 CqMatrix&	matDDz,
										 CqVector4D&	DDxA,
										 CqVector4D&	DDyA,
										 CqVector4D&	DDzA);
				CqVector4D	EvaluateFD(CqMatrix&	matDDx,
										 CqMatrix&	matDDy,
										 CqMatrix&	matDDz,
										 CqVector4D&	DDxA,
										 CqVector4D&	DDyA,
										 CqVector4D&	DDzA);
				void		AdvanceFD(CqMatrix&	matDDx,
										 CqMatrix&	matDDy,
										 CqMatrix&	matDDz,
										 CqVector4D&	DDxA,
										 CqVector4D&	DDyA,
										 CqVector4D&	DDzA);

};


//----------------------------------------------------------------------
/** \class CqSurfacePatchBilinear
 * Bilinear spline patch
 */

class _qShareC CqSurfacePatchBilinear : public CqSurface
{
	public:
				CqSurfacePatchBilinear();
				CqSurfacePatchBilinear(const CqSurfacePatchBilinear& From);
		virtual	~CqSurfacePatchBilinear();

		virtual	CqVector4D	EvaluateNormal(TqFloat s, TqFloat t)	const;

				CqSurfacePatchBilinear& operator=(const CqSurfacePatchBilinear& From);

				void					GenNormals();
				CqSurfacePatchBilinear*	uSubdivide();
				CqSurfacePatchBilinear*	vSubdivide();

		virtual	CqBound		Bound() const;
		virtual	CqMicroPolyGridBase* Dice();
		virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits);
		virtual TqBool	Diceable();

		virtual void		Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx);
		virtual	TqInt		cUniform() const	{return(1);}
		virtual	TqInt		cVarying() const	{return(4);}
		virtual	TqInt		cVertex() const		{return(4);}

	protected:
};

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !PATCH_H_INCLUDED
