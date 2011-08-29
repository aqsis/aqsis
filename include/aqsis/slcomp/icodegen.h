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
		\brief Compiler backend to output VM code.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef ICODEGEN_H_INCLUDED
#define ICODEGEN_H_INCLUDED

#include	<aqsis/aqsis.h>

#include	<string>


namespace Aqsis {

class IqParseNode;

//----------------------------------------------------------------------
class IqCodeGen
{
	public:
		virtual void OutputTree( IqParseNode* pNode, std::string strOutName = "" ) = 0;

		virtual ~IqCodeGen()
		{
		};
};


//-----------------------------------------------------------------------
/// Version number for the virtual machine stack code produced by the
/// CqCodeGenVM code generator.
#define AQSIS_SLX_VERSION 2

/** \brief Compiler backend to output VM code.
 */
class AQSIS_SLCOMP_SHARE CqCodeGenVM : public IqCodeGen
{
	public:
		virtual void OutputTree( IqParseNode* pNode, std::string strOutName );
};


//-----------------------------------------------------------------------
/** \brief Compiler backend for AST visualisation.
 *
 * This code generator produces graphs of the provided AST in the graphviz dot
 * graph-description language.
 *
 * \see CqParseTreeViz for more information.
 */
class AQSIS_SLCOMP_SHARE CqCodeGenGraphviz : public IqCodeGen
{
	public:
		virtual void OutputTree( IqParseNode* pNode, std::string strOutName );
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// ICODEGEN_H_INCLUDED
