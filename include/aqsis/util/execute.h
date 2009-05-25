// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
		\brief Declares the CqExecute class for executing external applications.
		\author Paul C. Gregory (pgregory@aqsis.org)

		Implementation is platform specific, existing in the platform folders.
*/

#ifndef EXECUTE_H_INCLUDED
#define EXECUTE 1

#include <aqsis/aqsis.h>

#include <boost/function.hpp>

#include <string>
#include <vector>

namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqExecute
 *  \brief Utility class for executing external programs in a system
 *			agnostic way.
 *			Provides the ability to redirect stdout/stdin/stderr and 
 *			call registered callbacks with the data.
 */
class AQSIS_UTIL_SHARE CqExecute
{
public:
	CqExecute(const std::string& command, const std::vector<std::string>& args, const std::string& curDir);
	
	typedef boost::function<void (const char* string)> TqCallback;

	void setStdOutCallback(TqCallback& callback);
	void operator()();
private:
	std::string	m_command;
	std::vector<std::string>	m_args;
	std::string	m_currDir;
	TqCallback	m_stdCallback;
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !EXECUTE_H_INCLUDED
