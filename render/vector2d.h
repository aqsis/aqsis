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
		\brief Declares the CqVector2D class which encapsulates a 2D vector.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef VECTOR2D_H_INCLUDED
#define VECTOR2D_H_INCLUDED 1

#include	<math.h>

#include	"ri.h"
#include	"specific.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

//-----------------------------------------------------------------------

class CqVector3D;
class CqVector4D;

//----------------------------------------------------------------------
/** \class CqVector2D
 * Define class structure for 2D vector.
 */

class _qShareC CqVector2D
{
	public:
		_qShareM				CqVector2D()	{}
		_qShareM				CqVector2D(TqFloat x, TqFloat y)		: m_x(x),		m_y(y)			{}
		_qShareM				CqVector2D(const CqVector3D &From);
		_qShareM				~CqVector2D()							{}
						
							/** Get the x component.
							 */
		_qShareM	TqFloat	x() const		{return(m_x);} 
							/** Set the x component.
							 * \param x Float new value.
							 */
		_qShareM	void		x(TqFloat x)	{m_x=x;}
							/** Get the y component.
							 */
		_qShareM	TqFloat	y() const		{return(m_y);} 
							/** Set the y component.
							 * \param y Float new value.
							 */
		_qShareM	void		y(TqFloat y)	{m_y=y;}

							/** Access the components as an array.
							 * \param i Integer component index, 0 or 1.
							 * \return Appropriate component or y if invalid index.
							 */
		_qShareM	TqFloat&	operator[](TqInt i)
												{ 
													switch(i)
													{
														case 0:		return(m_x);	break;
														case 1:		return(m_y);	break;
														default:	return(m_y);	break;
													}
												}

							/** Access the components as an array.
							 * \param i Integer component index, 0 or 1.
							 * \return Appropriate component or y if invalid index.
							 */
		_qShareM	const TqFloat&	operator[](TqInt i) const
												{ 
													switch(i)
													{
														case 0:		return(m_x);	break;
														case 1:		return(m_y);	break;
														default:	return(m_y);	break;
													}
												}

							/** Get the length squared.
							 */
		_qShareM	TqFloat	Magnitude2() const	{return((m_x*m_x)+(m_y*m_y));}
							/** Get the length.
							 */
		_qShareM	TqFloat	Magnitude()	 const	{return(sqrt((m_x*m_x)+(m_y*m_y)));}
		_qShareM	void	Unit();

		_qShareM	CqVector2D& operator= (const CqVector3D &From);
		_qShareM	CqVector2D& operator= (const CqVector4D &From);
								/** Addition assignment operator.
								 */
		_qShareM	CqVector2D& operator+=(const CqVector2D &From)			{m_x+=From.m_x; m_y+=From.m_y;	return(*this);	}
								/** Componentwise addition assignment operator.
								 */
		_qShareM	CqVector2D& operator+=(const TqFloat &f)					{m_x+=f; m_y+=f;	return(*this);	}
								/** Subtraction assignment operator.
								 */
		_qShareM	CqVector2D& operator-=(const CqVector2D &From)			{m_x-=From.m_x; m_y-=From.m_y;	return(*this);	}
								/** Componentwise subtraction assignment operator.
								 */
		_qShareM	CqVector2D& operator-=(const TqFloat &f)					{m_x-=f; m_y-=f;	return(*this);	}
								/** Coponent wise scale operator.
								 */
		_qShareM	CqVector2D& operator*=(const TqFloat Scale)				{m_x*=Scale;	m_y*=Scale;		return(*this);	}
								/** Scale operator.
								 */
		_qShareM	CqVector2D& operator*=(const CqVector2D &Scale)			{m_x*=Scale.m_x;m_y*=Scale.m_y;	return(*this);	}		
								/** Inverse scale operator.
								 */
		_qShareM	CqVector2D& operator/=(const CqVector2D &Scale)			{m_x/=Scale.m_x;m_y/=Scale.m_y;	return(*this);	}
								/** Component wise inverse scale operator.
								 */
		_qShareM	CqVector2D& operator/=(const TqFloat Scale)				{m_x/=Scale;	m_y/=Scale;		return(*this);	}
								/** Equality operator.
								 */
		_qShareM	TqBool		operator==(const CqVector2D &Cmp)  const	{return((m_x==Cmp.m_x) && (m_y==Cmp.m_y));		}
								/** Inequality operator.
								 */
		_qShareM	TqBool		operator!=(const CqVector2D &Cmp)  const	{return((m_x!=Cmp.m_x) || (m_y!=Cmp.m_y));		}

		_qShareM	friend CqVector2D	operator+(const TqFloat f, const CqVector2D& v)	{return CqVector2D(f+v.x(), f+v.y());}
		_qShareM	friend CqVector2D	operator+(const CqVector2D& v, const TqFloat f)	{CqVector2D r(v); return(r+=f);}
		_qShareM	friend CqVector2D	operator-(const TqFloat f, const CqVector2D& v)	{return CqVector2D(f-v.x(), f-v.y());}
		_qShareM	friend CqVector2D	operator-(const CqVector2D& v, const TqFloat f)	{CqVector2D r(v); return(r-=f);}
		_qShareM	friend CqVector2D	operator*(const TqFloat f, const CqVector2D& v)	{return CqVector2D(f*v.x(), f*v.y());}
		_qShareM	friend CqVector2D	operator*(const CqVector2D& v, const TqFloat f)	{CqVector2D r(v); return(r*=f);}
		_qShareM	friend CqVector2D	operator/(const TqFloat f, const CqVector2D& v)	{return CqVector2D(f/v.x(), f/v.y());}
		_qShareM	friend CqVector2D	operator/(const CqVector2D& v, const TqFloat f)	{CqVector2D r(v); return(r/=f);}

		_qShareM	friend CqVector2D	operator+(const CqVector2D& a, const CqVector2D& b)	{CqVector2D r(a); return(r+=b);}
		_qShareM	friend CqVector2D	operator-(const CqVector2D& a, const CqVector2D& b)	{CqVector2D r(a); return(r-=b);}
		_qShareM	friend CqVector2D	operator/(const CqVector2D& a, const CqVector2D& b)	{CqVector2D r(a); return(r/=b);}
		_qShareM	friend CqVector2D	operator-(const CqVector2D& v)						{return(CqVector2D(-v.m_x,-v.m_y));} // Negation

		_qShareM	friend TqFloat		operator*(const CqVector2D& a, const CqVector2D& b)	{return(a.m_x*b.m_x+a.m_y*b.m_y);} // Dot product
			
	protected:
			TqFloat	m_x;		///< X component.
			TqFloat	m_y;		///< Y component.
};

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !VECTOR2D_H_INCLUDED
