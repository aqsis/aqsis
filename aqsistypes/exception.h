// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Declares the XqException base class thrown during exceptions.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is exception.h included already?
#ifndef EXCEPTION_H_INCLUDED
//{
#define EXCEPTION_H_INCLUDED 1

#include	"aqsis.h"
#include <iosfwd>
#include <stdexcept>
#include <utility>
#include <string>

START_NAMESPACE( Aqsis )

//-----------------------------------------------------------------------
/** General message based exception.  specific exceptions are derived from this.
 */

class COMMON_SHARE XqException : public std::runtime_error
{
	public:
		/**
		* @deprecated Legacy constructor for backwards compatibility
		*/
		XqException (const std::string& reason);
		
		XqException (const std::string& reason, const std::string& detail,
		const std::string& file, const unsigned int line);
		
		XqException (const std::string& reason,	const std::string& file,
			const unsigned int line);
		
		const std::string& detail () const;
		
		std::pair<std::string, unsigned int> where () const;
		
		virtual const char* description () const;
		
		/**
		* C++ Standard,  [except.spec] 15.4.14, Example
		* a function that overrides a virtual function from a base class shall have an exception specification
		* at least as restrictive as that in the base class.
		*/
		virtual ~XqException () throw ();
		
	private:
		const std::string 	m_detail; 	//< Optional, a detailed message
		const std::string 	m_file;		//< The file name where the exception was thrown
		const unsigned int	m_line;		//< The line numer where the exception was thrown
}
;

COMMON_SHARE std::ostream& operator<<(std::ostream& o, const XqException& e);

class COMMON_SHARE XqInternal : public XqException
{
	public:
		XqInternal (const std::string& reason, const std::string& detail,
		const std::string& file, const unsigned int line);
		
		XqInternal (const std::string& reason,	const std::string& file,
			const unsigned int line);
		
		virtual const char* description () const;
		
		virtual ~XqInternal () throw ();
}
;

class COMMON_SHARE XqEnvironment : public XqException
{
	public:
		XqEnvironment (const std::string& reason, const std::string& detail,
		const std::string& file, const unsigned int line);
		
		XqEnvironment (const std::string& reason,	const std::string& file,
			const unsigned int line);
		
		virtual const char* description () const;
		
		virtual ~XqEnvironment () throw ();
}
;

class COMMON_SHARE XqValidationFailure : public XqException
{
	public:
		XqValidationFailure (const std::string& reason, const std::string& detail,
		const std::string& file, const unsigned int line);
		
		XqValidationFailure (const std::string& reason,	const std::string& file,
		const unsigned int line);
		
		virtual const char* description () const;
		
		virtual ~XqValidationFailure () throw ();
}
;

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )


#endif // EXCEPTION_H_INCLUDED
