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

RifTokenType mapTokType(Ri::TypeSpec::Type type)
{
	// This mapping is written as switch() rather than an array since it's more
	// robust to changes of the order of associated enums.
	switch(type)
	{
		case Ri::TypeSpec::Float:   return k_RifFloat;
		case Ri::TypeSpec::Integer: return k_RifInteger;
		case Ri::TypeSpec::Point:   return k_RifPoint;
		case Ri::TypeSpec::String:  return k_RifString;
		case Ri::TypeSpec::Color:   return k_RifColor;
		case Ri::TypeSpec::HPoint:  return k_RifHPoint;
		case Ri::TypeSpec::Normal:  return k_RifNormal;
		case Ri::TypeSpec::Vector:  return k_RifVector;
		case Ri::TypeSpec::Matrix:  return k_RifMatrix;
		case Ri::TypeSpec::MPoint:  return k_RifMPoint;
		default:
			assert(0);
			return k_RifFloat;
	}
}

RifTokenDetail mapTokClass(Ri::TypeSpec::IClass iclass)
{
	// This mapping is written as switch() rather than an array since it's more
	// robust to changes of the order of associated enums.
	switch(iclass)
	{
		case Ri::TypeSpec::Constant:    return k_RifConstant;
		case Ri::TypeSpec::Uniform:     return k_RifUniform;
		case Ri::TypeSpec::Varying:     return k_RifVarying;
		case Ri::TypeSpec::Vertex:      return k_RifVertex;
		case Ri::TypeSpec::FaceVarying: return k_RifFaceVarying;
		case Ri::TypeSpec::FaceVertex:  return k_RifFaceVertex;
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
		Ri::TypeSpec spec;
		// Use the current context dictionary if it exists, otherwise try to
		// parse as an inline declaration.
		if(QGetRenderContext())
			spec = QGetRenderContext()->tokenDict().lookup(name);
		else
			spec = parseDeclaration(name);
		// Unknown indicates a parse problem.  Pointer is a nonstandard aqsis
		// extension which has no equivalent k_Rif type (should we add one?).
		if(spec.type == Ri::TypeSpec::Unknown ||
		   spec.type == Ri::TypeSpec::Pointer)
			return 1;
		if(tokType)
			*tokType = mapTokType(spec.type);
		if(tokDetail)
			*tokDetail = mapTokClass(spec.iclass);
		if(arrayLen)
			*arrayLen = spec.arraySize;
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

