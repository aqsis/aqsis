// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
	\brief Declares generic exception types to be thrown by aqsis.
	\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef EXCEPTION_H_INCLUDED
#define EXCEPTION_H_INCLUDED 1

#include <string>

#include "aqsis.h"

namespace Aqsis
{

//-----------------------------------------------------------------------
/** \brief General message-based exception class.
 *
 * Specific exceptions are derived from this.
 */
class XqException
{
	public:
		/** Default constructor.
		 *
		 * \param reason - a message explaining the resons for the error.
		 */
		inline XqException(const char* reason = 0);
		/** \brief virtual destructor
		 */
		inline virtual ~XqException();

		/** \brief Get the reason for the error as a string
		 *
		 * \return error message
		 */
		inline const std::string& strReason() const;

	private:
		std::string m_reason;	///< The message associated with this exception.
};


//------------------------------------------------------------------------------
/** \brief Class for reporting input or output errors (eg, errors encountered
 * during file IO
 */
class XqMemoryError : public XqException
{
	public:
		inline XqMemoryError(const char* reason = 0);
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Implementation of inline functions.
//------------------------------------------------------------------------------
// Inline functions for XqException
inline XqException::XqException(const char* reason)
	: m_reason(reason)
{}

inline XqException::~XqException()
{}

inline const std::string& XqException::strReason() const
{
	return m_reason;
}


//------------------------------------------------------------------------------
// Inline functions for XqMemoryError
inline XqMemoryError::XqMemoryError(const char* reason)
	: XqException(reason)
{ }


//-----------------------------------------------------------------------

} // namespace Aqsis


#endif // EXCEPTION_H_INCLUDED
