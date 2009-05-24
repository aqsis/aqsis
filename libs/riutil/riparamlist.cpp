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
 * \brief Class wrapping renderman parameter lists in a C++ interface.
 *
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#include <aqsis/riutil/riparamlist.h>

#include <boost/tokenizer.hpp>

namespace Aqsis {

// CqRiParamList implementation

void CqRiParamList::extractTokenNames(std::vector<std::string>& tokNames,
		RtToken* tokens, TqInt tokCount)
{
	tokNames.clear();
	for(TqInt i = 0; i < tokCount; ++i)
	{
		std::string tok = tokens[i];
		typedef boost::tokenizer<boost::char_separator<char> > TqTokenizer;
		// Break the RtToken into sub-tokens
		TqTokenizer tokens(tok, boost::char_separator<char>(" \t\n", "[]"));
		// Extract the last sub-token.  It should be the variable name.
		std::string name;
		for(TqTokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i)
			name = *i;
		tokNames.push_back(name);
	}
}

} // namespace Aqsis
