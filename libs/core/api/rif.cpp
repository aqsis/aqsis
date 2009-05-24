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
 * \brief Implementation of the Rif interface.
 * \author Chris Foster
 */

#include <aqsis/ri/rif.h>

#include "renderer.h"

using namespace Aqsis;

namespace {

RifTokenType mapTokType(EqVariableType type)
{
	// This mapping is written as switch() rather than an array since it's more
	// robust to changes of the order of associated enums.
	switch(type)
	{
		case type_float:   return k_RifFloat;
		case type_integer: return k_RifInteger;
		case type_point:   return k_RifPoint;
		case type_string:  return k_RifString;
		case type_color:   return k_RifColor;
		case type_hpoint:  return k_RifHPoint;
		case type_normal:  return k_RifNormal;
		case type_vector:  return k_RifVector;
		case type_matrix:  return k_RifMatrix;
		default:
			assert(0);
			return k_RifFloat;
	}
}

RifTokenDetail mapTokClass(EqVariableClass Class)
{
	// This mapping is written as switch() rather than an array since it's more
	// robust to changes of the order of associated enums.
	switch(Class)
	{
		case class_constant: return k_RifConstant;
		case class_uniform: return k_RifUniform;
		case class_varying: return k_RifVarying;
		case class_vertex: return k_RifVertex;
		case class_facevarying: return k_RifFaceVarying;
		case class_facevertex: return k_RifFaceVertex;
		default:
			assert(0);
			return k_RifConstant;
	}
}

} // anon namespace

extern "C" RtInt RifGetDeclaration(RtToken name, RifTokenType *tokType,
		RifTokenDetail *tokDetail, RtInt *arrayLen)
{
	try
	{
		CqPrimvarToken tok;
		if(QGetRenderContext())
		{
			// Use the current context dictionary if it exists.
			tok = QGetRenderContext()->tokenDict().parseAndLookup(name);
		}
		else
		{
			// Else just try to parse as an inline declaration.
			tok = CqPrimvarToken(name);
			if(tok.type() == type_invalid)
				return 1; // Couldn't parse as an inline decl.
		}
		*tokType = mapTokType(tok.type());
		*tokDetail = mapTokClass(tok.Class());
		*arrayLen = tok.count();
		return 0;
	}
	catch(XqValidation&)
	{
		return 1;
	}
	catch(...)
	{
		return 1;
	}
}

