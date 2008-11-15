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
 * \brief RIB request handler implementation for aqsis.
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#ifndef RIBREQUESTHANDLER_H_INCLUDED
#define RIBREQUESTHANDLER_H_INCLUDED

#include "aqsis.h"

#include "iribrequest.h"
#include "tokendictionary.h"

namespace Aqsis
{

class CqRibParser;

//------------------------------------------------------------------------------
class CqRibRequestHandler : public IqRibRequestHandler
{
	public:
		/// Construct a request handler.
		CqRibRequestHandler();

		virtual void handleRequest(const std::string& requestName,
				CqRibParser& parser);

	private:
		//--------------------------------------------------
		/// \name Helpers for handler methods
		//@{
		static RtBasis getBasis(CqRibParser& parser);
		//@}

		//--------------------------------------------------
		/// \name handler methods for RI calls
		//@{
		void handleBasis(CqRibParser& parser);
		void handleColor(CqRibParser& parser);
		void handleOpacity(CqRibParser& parser);

		void handleDeclare(CqRibParser& parser);
		void handleDepthOfField();

#		include "requesthandler_declarations.inl"
		//@}

		// State variables for the parser.
		/// State
		TqInt m_numColorComps;
		CqTokenDictionary m_tokenDict;
};

//==============================================================================
// Implementation details
//==============================================================================

} // namespace Aqsis

#endif // RIBREQUESTHANDLER_H_INCLUDED
