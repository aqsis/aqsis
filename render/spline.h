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
		\brief Declares the CqSplineCubic class for generic spline functionality.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef SPLINE_H_INCLUDED
#define SPLINE_H_INCLUDED 1

#include	<vector>

#include	"matrix.h"
#include	"vector4d.h"
#include	"sstring.h"

#include	"specific.h"	// Needed for namespace macros.

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)


//----------------------------------------------------------------------
/** \class CqSplineCubic
 * Cubic spline curve
 */

class _qShareC CqSplineCubic
{
	public:
	_qShareM			CqSplineCubic(TqInt cu=4);
	_qShareM	virtual	~CqSplineCubic()	{}

	_qShareM	virtual	CqVector4D	Evaluate(TqFloat t)	const;
	_qShareM	virtual	void		InitFD(TqInt n);
	_qShareM	virtual	CqVector4D	EvaluateFD();
	_qShareM	virtual	TqInt		cSections()			const;

						/** Get a reference to the array of control points.
						 */
	_qShareM			std::vector<CqVector4D>& aControlPoints()	{return(m_aControlPoints);}
						/** Get a reference to the cubic spline basis matrix.
						 */
	_qShareM			CqMatrix&	matBasis() {return(m_matBasis);}
						/** Set the cubic spline basis matrix.
						 * \param mat Basis matrix.
						 */
	_qShareM			void		SetmatBasis(CqMatrix& mat)		
														{m_matBasis=mat;}
						/** Set the cubic spline basis matrix.
						 * \param name Basis name.
						 */
	_qShareM			void		SetmatBasis(const char* name)		
														{
															RtBasis b;
															if(BasisFromName(b,name))
																SetmatBasis(CqMatrix(b));
														}
						/** Set the cubic spline basis matrix.
						 * \param name Basis name.
						 */
	_qShareM			void		SetmatBasis(const CqString& name)		
														{
															RtBasis b;
															if(BasisFromName(b,name.c_str()))
																SetmatBasis(CqMatrix(b));
														}
						/** Get the control point step size for the evaluation window.
						 * \return Integer step size.
						 */
	_qShareM			TqInt		Step() const		{return(m_Step);}
						/** Set the control point step size for the evaluation window.
						 * \param Step Integer step size.
						 */
	_qShareM			void		SetStep(const TqInt Step) {m_Step=Step;}

						/** Indexed access to the control points.
						 * \param i Integer index.
						 */
	_qShareM	CqVector4D&	operator[](TqInt i)	{assert(i<m_cu); return(m_aControlPoints[i]);}
						/** Indexed access to the control points.
						 * \param i Integer index.
						 */
	_qShareM	const CqVector4D&	operator[](TqInt i) const {assert(i<m_cu); return(m_aControlPoints[i]);}

	protected:
		CqMatrix		m_matBasis;						///< Basis matrix.
		TqInt			m_Step;							///< Evaluation window step size.
		TqInt			m_cu;							///< Number of control points.
		std::vector<CqVector4D>	m_aControlPoints;		///< Array of 4D control points.

		CqVector4D		m_vecFDPoint;					///< Forward difference evaluation parameters.
		CqVector4D		m_vecFDDelta;					///< Forward difference evaluation parameters.
		CqVector4D		m_vecFDDelta2;					///< Forward difference evaluation parameters.
		CqVector4D		m_vecFDDelta3;					///< Forward difference evaluation parameters.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !SPLINE_H_INCLUDED
