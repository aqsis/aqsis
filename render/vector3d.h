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
		\brief Declares the CqVector3D class which encapsulates a 3D vector/point/normal.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef VECTOR3D_H_INCLUDED
#define VECTOR3D_H_INCLUDED 1

#include	"ri.h"

#include	"specific.h"
#include	"vector2d.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

//-----------------------------------------------------------------------

class CqVector4D;
class CqColor;
struct SqVMStackEntry;

//----------------------------------------------------------------------
/** \class CqVector3D
 * Define class structure for 3D vector.
 */

class _qShareC CqVector3D
{
	public:
		_qShareM				CqVector3D()	{}
		_qShareM				CqVector3D(const CqVector2D &From)			:m_x(From.x()), m_y(From.y()),	m_z(0)	{}
		_qShareM				CqVector3D(const CqColor &From);
		_qShareM				CqVector3D(TqFloat x, TqFloat y, TqFloat z)	:m_x(x),		m_y(y),			m_z(z)			{}
		_qShareM				CqVector3D(TqFloat f)						:m_x(f),		m_y(f),			m_z(f)			{}
		_qShareM				CqVector3D(const CqVector4D &From);
		_qShareM				CqVector3D(const TqFloat Array[3])			:m_x(Array[0]),	m_y(Array[1]),	m_z(Array[2])	{}
		_qShareM				~CqVector3D()									{}
						
							/** Get the x component.
							 */
		_qShareM	TqFloat	x() const		{return(m_x);} 
							/** Set the x component.
							 */
		_qShareM	void		x(TqFloat x)	{m_x=x;}
							/** Get the y component.
							 */
		_qShareM	TqFloat	y() const		{return(m_y);} 
							/** Set the y component.
							 */
		_qShareM	void		y(TqFloat y)	{m_y=y;}
							/** Get the z component.
							 */
		_qShareM	TqFloat	z() const		{return(m_z);}
							/** Set the z component.
							 */
		_qShareM	void		z(TqFloat z)	{m_z=z;}

							/** Array based component access.
							 * \param i Integer component index, 0-2.
							 * \return Appropriate component, or z if index is invalid.
							 */
		_qShareM	TqFloat&	operator[](TqInt i)
												{ 
													switch(i)
													{
														case 0:		return(m_x);	break;
														case 1:		return(m_y);	break;
														case 2:		return(m_z);	break;
														default:	return(m_z);	break;
													}
												}

							/** Array based component access.
							 * \param i Integer component index, 0-2.
							 * \return Appropriate component, or z if index is invalid.
							 */
		_qShareM	const TqFloat&	operator[](TqInt i) const
												{ 
													switch(i)
													{
														case 0:		return(m_x);	break;
														case 1:		return(m_y);	break;
														case 2:		return(m_z);	break;
														default:	return(m_z);	break;
													}
												}

							/** Get the length squared.
							 */ 
		_qShareM	TqFloat	Magnitude2() const	{return((m_x*m_x)+(m_y*m_y)+(m_z*m_z));}
							/** Get the length.
							 */ 
		_qShareM	TqFloat	Magnitude()	 const	{return(sqrt((m_x*m_x)+(m_y*m_y)+(m_z*m_z)));}
		_qShareM	void	Unit();

		_qShareM	CqVector3D& operator= (const CqVector4D &From);
		_qShareM	CqVector3D& operator= (const CqColor &From);
		_qShareM	CqVector3D&	operator= (const SqVMStackEntry* pVal);
								/** Addition assignment operator.
								 */
		_qShareM	CqVector3D& operator+=(const CqVector3D &From)		{m_x+=From.m_x;	m_y+=From.m_y;	m_z+=From.m_z;	return(*this);	}
								/** Component wise addition assignment operator.
								 */
		_qShareM	CqVector3D& operator+=(const TqFloat f)				{m_x+=f; m_y+=f; m_z+=f;	return(*this);	}
								/** Subtraction assignment operator.
								 */
		_qShareM	CqVector3D& operator-=(const CqVector3D &From)		{m_x-=From.m_x;	m_y-=From.m_y;	m_z-=From.m_z;	return(*this);	}
								/** Component wise subtraction assignment operator.
								 */
		_qShareM	CqVector3D& operator-=(const TqFloat f)				{m_x-=f; m_y-=f; m_z-=f;	return(*this);	}
		_qShareM	CqVector3D& operator%=(const CqVector3D &From);
								/** Component wise scale assignment operator.
								 */
		_qShareM	CqVector3D& operator*=(const TqFloat Scale)			{m_x*=Scale;	m_y*=Scale;		m_z*=Scale;		return(*this);	}		
								/** Scale assignment operator.
								 */
		_qShareM	CqVector3D& operator*=(const CqVector3D &Scale)		{m_x*=Scale.m_x;m_y*=Scale.m_y;	m_z*=Scale.m_z;	return(*this);	}				
								/** Inverse scale assignment operator.
								 */
		_qShareM	CqVector3D& operator/=(const CqVector3D &Scale)		{m_x/=Scale.m_x;m_y/=Scale.m_y;	m_z/=Scale.m_z;	return(*this);	}		
								/** Component wise inverse scale assignment operator.
								 */
		_qShareM	CqVector3D& operator/=(const TqFloat Scale)			{m_x/=Scale;	m_y/=Scale;		m_z/=Scale;		return(*this);	}		
							
								/** Component wise equality operator.
								 */
		_qShareM	TqBool		operator==(const CqVector3D &Cmp) const	{return((m_x==Cmp.m_x) && (m_y==Cmp.m_y) && (m_z==Cmp.m_z));}
								/** Component wise inequality operator.
								 */
		_qShareM	TqBool		operator!=(const CqVector3D &Cmp) const	{return((m_x!=Cmp.m_x) || (m_y!=Cmp.m_y) || (m_z!=Cmp.m_z));}
								/** Component wise greater than or equal to operator.
								 */
		_qShareM	TqBool		operator>=(const CqVector3D &Cmp) const	{return((m_x>=Cmp.m_x) && (m_y>=Cmp.m_y) && (m_z>=Cmp.m_z));}
								/** Component wise less than or equal to operator.
								 */
		_qShareM	TqBool		operator<=(const CqVector3D &Cmp) const	{return((m_x<=Cmp.m_x) || (m_y<=Cmp.m_y) || (m_z<=Cmp.m_z));}
								/** Component wise greater than operator.
								 */
		_qShareM	TqBool		operator>(const CqVector3D &Cmp)  const	{return((m_x>Cmp.m_x) && (m_y>Cmp.m_y) && (m_z>Cmp.m_z));}
								/** Component wise less than operator.
								 */
		_qShareM	TqBool		operator<(const CqVector3D &Cmp)  const	{return((m_x<Cmp.m_x) || (m_y<Cmp.m_y) || (m_z<Cmp.m_z));}

		_qShareM	friend CqVector3D	operator+(const TqFloat f, const CqVector3D& v)	{return CqVector3D(f+v.x(), f+v.y(), f+v.z());}
		_qShareM	friend CqVector3D	operator+(const CqVector3D& v, const TqFloat f)	{CqVector3D r(v); return(r+=f);}
		_qShareM	friend CqVector3D	operator-(const TqFloat f, const CqVector3D& v)	{return CqVector3D(f-v.x(), f-v.y(), f-v.z());}
		_qShareM	friend CqVector3D	operator-(const CqVector3D& v, const TqFloat f)	{CqVector3D r(v); return(r-=f);}
		_qShareM	friend CqVector3D	operator*(const TqFloat f, const CqVector3D& v)	{return CqVector3D(f*v.x(), f*v.y(), f*v.z());}
		_qShareM	friend CqVector3D	operator*(const CqVector3D& v, const TqFloat f)	{CqVector3D r(v); return(r*=f);}
		_qShareM	friend CqVector3D	operator/(const TqFloat f, const CqVector3D& v)	{return CqVector3D(f/v.x(), f/v.y(), f/v.z());}
		_qShareM	friend CqVector3D	operator/(const CqVector3D& v, const TqFloat f)	{CqVector3D r(v); return(r/=f);}

		_qShareM	friend CqVector3D	operator+(const CqVector3D& a, const CqVector3D& b)	{CqVector3D r(a); return(r+=b);}
		_qShareM	friend CqVector3D	operator-(const CqVector3D& a, const CqVector3D& b)	{CqVector3D r(a); return(r-=b);}
		_qShareM	friend CqVector3D	operator/(const CqVector3D& a, const CqVector3D& b)	{CqVector3D r(a); return(r/=b);}
		_qShareM	friend CqVector3D	operator-(const CqVector3D& v)						{return(CqVector3D(-v.m_x,-v.m_y,-v.m_z));} // Negation

		_qShareM	friend TqFloat		operator*(const CqVector3D& a, const CqVector3D& b)
																							{return(a.m_x*b.m_x+a.m_y*b.m_y+a.m_z*b.m_z);} // Dot product
		_qShareM	friend CqVector3D	operator%(const CqVector3D& a, const CqVector3D& b);	// X product
			
	protected:
			TqFloat	m_x;	///< X component.
			TqFloat	m_y;	///< Y component.
			TqFloat	m_z;	///< Z component.
};

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// VECTOR3D_H_INCLUDED
