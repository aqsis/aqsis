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
		\brief Declares an extended string class, CqString, derived from std::string
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef SSTRING_H_INCLUDED
#define SSTRING_H_INCLUDED 1

#include <string>
#include <iostream>

#include "aqsis.h"
#include "specific.h"

#include "ri.h"

#define	 _qShareName	CORE
#include "share.h"


START_NAMESPACE(Aqsis)

typedef std::string CqStringBase;

//----------------------------------------------------------------------
/** \class CqString
 * An extended string class, derived from std::string
 */

class _qShareC CqString : public CqStringBase
{
	public:
	_qShareM	CqString()	:	CqStringBase()	{}
	_qShareM	CqString(const CqString& str) : CqStringBase(str)		{}
	_qShareM	CqString(const CqStringBase& str) : CqStringBase(str)	{}
	_qShareM	CqString(const TqChar* s) : CqStringBase(s)				{}
				/** Construct from an integer,
				 */
	_qShareM	CqString(TqInt i)	{
										*this+=i;
									}
				/** Construct from a float,
				 */
	_qShareM	CqString(TqFloat f)	{
										*this+=f;
									}

		// Format a string printf style.
	_qShareM	CqString&	Format(const TqChar* Format, ...);
	_qShareM	CqString	ExpandEscapes() const;
	_qShareM	CqString	TranslateEqcapes() const;

		// Concatenation functions not provided by std::string
	_qShareM	CqString&	operator+=(const CqString& str);
	_qShareM	CqString&	operator+=(const TqChar* str);
	_qShareM	CqString&	operator+=(TqChar c);
	_qShareM	CqString&	operator+=(TqInt i);
	_qShareM	CqString&	operator+=(TqFloat f);
};


// Some useful functions
_qShare	std::ostream& operator<<(std::ostream & stmOutput,const CqString& strString);
_qShare	CqString operator+(const CqString& strAdd1, const CqString& strAdd2);
_qShare	CqString operator+(const TqChar* strAdd1, const CqString& strAdd2);
_qShare	CqString operator+(const CqString& strAdd1, const TqChar* strAdd2);
_qShare	CqString operator+(const CqString& strAdd1, TqChar ch);
_qShare	CqString operator+(TqChar ch, const CqString& strAdd2);

// These must be defined so that std::string can be used as a type in the ShaderVM and
// in the parameter dicing code.
_qShare	CqString operator/(const CqString& strAdd1, const CqString& strAdd2);
_qShare	CqString operator*(const CqString& strAdd1, const CqString& strAdd2);
_qShare	CqString operator*(const CqString& strAdd1, TqFloat f);
_qShare	CqString operator-(const CqString& strAdd1, const CqString& strAdd2);


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !SSTRING_H_INCLUDED

