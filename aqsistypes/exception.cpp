// Aqsis
// Copyright 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This program is free software; you can redistribute it and/or
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
		\brief Exception implementation
*/

#include "exception.h"
#include <ostream>

START_NAMESPACE(Aqsis)

//////////////////////////////////////////////////////////////////////////////
XqException::XqException (const std::string& reason)
: std::runtime_error (reason), m_file ("Unspecified"), m_line (0)
{
}

XqException::XqException (const std::string& reason, const std::string& detail,
const std::string& file, const unsigned int line)
: std::runtime_error (reason), m_detail (detail), m_file (file), m_line (line)
{
}

XqException::XqException (const std::string& reason,	const std::string& file,
const unsigned int line)
: std::runtime_error (reason), m_file (file), m_line (line)
{
}

const std::string& XqException::detail () const
{
	return m_detail;
}

std::pair<std::string, unsigned int> XqException::where () const
{
	return std::make_pair (m_file, m_line);
}

const char* XqException::description () const
{
	return "General error";
}

XqException::~XqException () throw ()
{
}

//////////////////////////////////////////////////////////////////////////////
XqInternal::XqInternal (const std::string& reason, const std::string& detail,
const std::string& file, const unsigned int line)
: XqException (reason, detail, file, line)
{
}

XqInternal::XqInternal (const std::string& reason,	const std::string& file,
	const unsigned int line)
: XqException (reason, file, line)
{
}

const char* XqInternal::description () const
{
	return "Internal error";
}

XqInternal::~XqInternal () throw ()
{
}

//////////////////////////////////////////////////////////////////////////////
XqEnvironment::XqEnvironment (const std::string& reason, const std::string& detail,
const std::string& file, const unsigned int line)
: XqException (reason, detail, file, line)
{
}

XqEnvironment::XqEnvironment (const std::string& reason,	const std::string& file,
	const unsigned int line)
: XqException (reason, file, line)
{
}

const char* XqEnvironment::description () const
{
	return "Environment error";
}

XqEnvironment::~XqEnvironment () throw ()
{
}

//////////////////////////////////////////////////////////////////////////////
XqValidationFailure::XqValidationFailure (const std::string& reason, const std::string& detail,
const std::string& file, const unsigned int line)
: XqException (reason, detail, file, line)
{
}

XqValidationFailure::XqValidationFailure (const std::string& reason,	const std::string& file,
const unsigned int line)
: XqException (reason, file, line)
{
}

const char* XqValidationFailure::description () const
{
	return "Validation error";
}

XqValidationFailure::~XqValidationFailure () throw ()
{
}
	
//////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& o, const XqException& e)
{
	o << e.description () << " (" << e.where ().first << "," << e.where ().second << ")";	
	o <<": " << e.what ();
	
	if (!e.detail ().empty ())
		o << " - " << e.detail ();
		
	return o;
}

END_NAMESPACE(Aqsis)
