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

#include "primvartoken.h"

namespace Aqsis {

class CqRibParser;


//------------------------------------------------------------------------------
// Array types which are passed to RI request handlers from the RIB parser.

/// integer array type for RI request array paramters
typedef std::vector<TqInt> TqRiIntArray;
/// float array type for RI request array paramters
typedef std::vector<TqFloat> TqRiFloatArray;
/// string array type for RI request array paramters
typedef std::vector<std::string> TqRiStringArray;


//------------------------------------------------------------------------------
/** \brief RIB request handler interface
 *
 * Code expecting to handle RIB requests should inherit from this class.  This
 * allows it to be attached to and called by the RIB parser at runtime.
 */
class IqRibRequest
{
	public:
		/** \brief Handle a RIB request by reading from the parser
		 *
		 * The request handler is expected to call the CqRibParser::get*
		 * functions of the parser object in order to read the required
		 * parameters for the request.
		 */
		virtual void handleRequest(CqRibParser& parser) = 0;

		virtual ~IqRibRequest() {}
};

//------------------------------------------------------------------------------
/** \brief RIB parameter list interface.
 *
 * An implementation of this object needs to be prepared to accept a list of
 * (token, value) pairs as they are parsed.
 */
class IqRibParamList
{
	public:
		/// Add an (token, integer array) pair to the end of the parameter list
		virtual void append(const CqPrimvarToken& token,
				const TqRiIntArray& value) = 0;
		/// Add an (token, float array) pair to the end of the parameter list
		virtual void append(const CqPrimvarToken& token,
				const TqRiFloatArray& value) = 0;
		/// Add an (token, string array) pair to the end of the parameter list
		virtual void append(const CqPrimvarToken& token,
				const TqRiStringArray& value) = 0;

		virtual ~IqRibParamList() {}
};

} // namespace Aqsis

#endif // IRIBREQUEST_H_INCLUDED
