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
 * \brief Code generator backend to output a graphviz dot diagram of the parse tree.
 * \author Chris Foster - chris42f (at) gmail (dot) com
*/

#ifndef CODEGENGRAPHVIZ_H_INCLUDED
#define CODEGENGRAPHVIZ_H_INCLUDED

#include "aqsis.h"

#include <string>

#include "iparsenode.h"
#include "icodegen.h"

namespace Aqsis {

//-----------------------------------------------------------------------
/** \brief Code gen wrapper around CqParseTreeViz.
 *
 * This code generator produces graphs of the provided AST in the graphviz dot
 * graph-description language.
 *
 * \see CqParseTreeViz for more information.
 */
class CqCodeGenGraphviz : public IqCodeGen
{
	public:
		virtual void OutputTree( IqParseNode* pNode, std::string strOutName );
};


} // namespace Aqsis

#endif // CODEGENGRAPHVIZ_H_INCLUDED
