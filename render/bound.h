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
		\brief Declares the geometric boundary handling class.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef BOUND_H_INCLUDED
#define BOUND_H_INCLUDED 1

#include	"ri.h"

#include	"matrix.h"
#include	"specific.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \class CqBound
 * Class specifying a 3D geometric bound.
 */

class CqBound
{
	public:
						CqBound(TqFloat* pBounds=0)
									{
										if(pBounds)
										{
											m_vecMin.x(pBounds[0]);	m_vecMin.y(pBounds[1]);	m_vecMin.z(pBounds[2]);
											m_vecMax.x(pBounds[3]);	m_vecMax.y(pBounds[4]);	m_vecMax.z(pBounds[5]);
										}
									}
						CqBound(TqFloat XMin, TqFloat YMin, TqFloat ZMin, TqFloat XMax, TqFloat YMax, TqFloat ZMax)
									{
										m_vecMin.x(XMin);	m_vecMin.y(YMin);	m_vecMin.z(ZMin);
										m_vecMax.x(XMax);	m_vecMax.y(YMax);	m_vecMax.z(ZMax);
									}
						CqBound(const CqVector3D& vecMin, const CqVector3D& vecMax)
									{
										m_vecMin=vecMin;
										m_vecMax=vecMax;
									}
						CqBound(const CqBound& From);
						~CqBound()	{}

	const	CqVector3D&	vecMin() const			{return(m_vecMin);}
			CqVector3D&	vecMin()				{return(m_vecMin);}
	const	CqVector3D&	vecMax() const			{return(m_vecMax);}
			CqVector3D&	vecMax()				{return(m_vecMax);}
			CqBound&	operator=(const CqBound& From);

			void		Transform(const CqMatrix&	matTrans);
			CqBound		Combine(const CqBound& bound);
			void		Encapsulate(const CqVector3D& v);
			void		Encapsulate2D(const CqVector2D& v);
			TqBool	Contains3D(const CqVector3D& v)
									{	
										if((v.x()>=m_vecMin.x() && v.x()<=m_vecMax.x()) &&
										   (v.y()>=m_vecMin.y() && v.y()<=m_vecMax.y()) &&
										   (v.z()>=m_vecMin.z() && v.z()<=m_vecMax.z()))
											return(TqTrue);
										else
											return(TqFalse);
									}
			TqBool	Contains2D(const CqVector2D& v)
									{	
										if((v.x()>=m_vecMin.x() && v.x()<=m_vecMax.x()) &&
										   (v.y()>=m_vecMin.y() && v.y()<=m_vecMax.y()))
											return(TqTrue);
										else
											return(TqFalse);
									}

	private:
			CqVector3D	m_vecMin;
			CqVector3D	m_vecMax;
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif // BOUND_H_INCLUDED
