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
		\brief Declares the XqException base class thrown during exceptions.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is exception.h included already?
#ifndef EXCEPTION_H_INCLUDED
//{
#define EXCEPTION_H_INCLUDED 1

#include	"sstring.h"
#include	"specific.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

//-----------------------------------------------------------------------
/** General message based exception.  specific exceptions are derived from this.
 */

class _qShareC XqException
{
	public:
							/** Default constructor.
							 * \param pcharReason Pointer to a string associated with the error which caused the exception.
							 */
	_qShareM				XqException(const char* pcharReason=0)	:
											m_strReason(pcharReason)	{}
	_qShareM	virtual		~XqException()		{}

							/** Get a reference to the error message.
							 * \return a constant string reference.
							 */
	_qShareM	const CqString&	strReason()	{return(m_strReason);}

	private:
				CqString	m_strReason;	///< The message associated with this exception.
};

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)


#endif // EXCEPTION_H_INCLUDED
