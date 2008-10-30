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
 *
 * \brief RIB request handler interfaces
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */


#ifndef IRIBREQUEST_H_INCLUDED
#define IRIBREQUEST_H_INCLUDED

#include "aqsis.h"

#include <vector>
#include <string>

namespace Aqsis {

class CqRibParser;


//------------------------------------------------------------------------------
/** \brief RIB request handler interface
 *
 * Code expecting to handle RIB requests should inherit from this class.  This
 * allows it to be attached to and called by the RIB parser at runtime.
 */
class IqRibRequestHandler
{
	public:
		/** \brief Handle a RIB request by reading from the parser
		 *
		 * The request handler is expected to call the CqRibParser::get*
		 * functions of the parser object in order to read the required
		 * parameters for the request.
		 *
		 * \param requestName - name of the request to handle.
		 * \param parser - source from which request parameters should be read.
		 */
		virtual void handleRequest(const std::string& requestName,
				CqRibParser& parser) = 0;

		virtual ~IqRibRequestHandler() {}
};

//------------------------------------------------------------------------------
/// RIB parameter list handler.
class IqRibParamListHandler
{
	public:
		/** \brief Read a RIB parameter with the supplied name from the parser.
		 *
		 * The handler should be prepared to determine the type of parameter to
		 * be read by inspecting the parameter name.  It should then read the
		 * parameter using CqRibParser::getIntParam(), getFloatParam() or
		 * getStringParam().
		 *
		 * \param name - raw parameter name as read from the input stream
		 * \param parser - source from which the parameter should be read.
		 */
		virtual void readParameter(const std::string& name, CqRibParser& parser) = 0;

		virtual ~IqRibParamListHandler() {}
};

} // namespace Aqsis

#endif // IRIBREQUEST_H_INCLUDED
