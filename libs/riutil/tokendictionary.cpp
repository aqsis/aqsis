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
 * \brief List of predeclared standard primitive variables
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#include <aqsis/riutil/tokendictionary.h>

namespace Aqsis {

//------------------------------------------------------------------------------
TokenDict::TokenDict()
{
	const std::vector<CqPrimvarToken>& vars = standardPrimvars();
	for(int i = 0, end = vars.size(); i < end; ++i)
		m_dict.insert(Dict::value_type(vars[i].name(), toTypeSpec(vars[i])));
}

void TokenDict::declare(const char* name, const char* type)
{
	const char* nameInType = 0;
	Ri::TypeSpec spec = parseDeclaration(type, &nameInType);
	if(nameInType)
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
			"bad token type \"" << type << "\"");
	}
	if(spec.type == Ri::TypeSpec::Unknown)
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
			"type not specified in string \"" << type << "\"");
	}
	m_dict[name] = spec;
}

void TokenDict::declare(const char* name, const Ri::TypeSpec& spec)
{
	m_dict[name] = spec;
}

Ri::TypeSpec TokenDict::lookup(const char* token, const char** nameBegin,
							   const char** nameEnd) const
{
	const char* nb = 0;
	const char* ne = 0;
	if(!nameBegin) nameBegin = &nb;
	if(!nameEnd) nameEnd = &ne;
	Ri::TypeSpec spec = parseDeclaration(token, nameBegin, nameEnd);
	if(spec.type != Ri::TypeSpec::Unknown)
		return spec;
	Dict::const_iterator i
		= m_dict.find(std::string(*nameBegin, *nameEnd));
	if(i == m_dict.end())
	{
		AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
			"undeclared token \"" << token << "\"");
	}
	return i->second;
}

Ri::TypeSpec TokenDict::lookup(const char* token, std::string* name) const
{
	const char* nameBegin = 0;
	const char* nameEnd = 0;
	Ri::TypeSpec spec = lookup(token, &nameBegin, &nameEnd);
	name->assign(nameBegin, nameEnd);
	return spec;
}


//------------------------------------------------------------------------------
namespace {

// Array of standard variables
const CqPrimvarToken standardVarsInit[] = {
	//--------------------------------------------------
	// Standard shader instance variables
	CqPrimvarToken(class_uniform,  type_float,   1, "Ka"),
	CqPrimvarToken(class_uniform,  type_float,   1, "Kd"),
	CqPrimvarToken(class_uniform,  type_float,   1, "Ks"),
	CqPrimvarToken(class_uniform,  type_float,   1, "Kr"),
	CqPrimvarToken(class_uniform,  type_float,   1, "Km"),
	CqPrimvarToken(class_uniform,  type_float,   1, "roughness"),
	CqPrimvarToken(class_uniform,  type_string,  1, "texturename"),
	CqPrimvarToken(class_uniform,  type_color,   1, "specularcolor"),
	CqPrimvarToken(class_uniform,  type_float,   1, "intensity"),
	CqPrimvarToken(class_uniform,  type_color,   1, "lightcolor"),
	CqPrimvarToken(class_uniform,  type_point,   1, "from"),
	CqPrimvarToken(class_uniform,  type_point,   1, "to"),
	CqPrimvarToken(class_uniform,  type_float,   1, "coneangle"),
	CqPrimvarToken(class_uniform,  type_float,   1, "conedeltaangle"),
	CqPrimvarToken(class_uniform,  type_float,   1, "beamdistribution"),
	CqPrimvarToken(class_uniform,  type_float,   1, "mindistance"),
	CqPrimvarToken(class_uniform,  type_float,   1, "maxdistance"),
	CqPrimvarToken(class_uniform,  type_float,   1, "distance"),
	CqPrimvarToken(class_uniform,  type_color,   1, "background"),

	//--------------------------------------------------
	// Standard primitive variable names
	CqPrimvarToken(class_vertex,   type_point,   1, "P"),
	CqPrimvarToken(class_vertex,   type_point,   1, "Pz"),
	CqPrimvarToken(class_vertex,   type_hpoint,  1, "Pw"),
	CqPrimvarToken(class_varying,  type_normal,  1, "N"),
	CqPrimvarToken(class_varying,  type_normal,  1, "Ng"),
	CqPrimvarToken(class_uniform,  type_normal,  1, "Np"),
	CqPrimvarToken(class_varying,  type_color,   1, "Cs"),
	CqPrimvarToken(class_varying,  type_color,   1, "Os"),
	CqPrimvarToken(class_varying,  type_float,   1, "s"),
	CqPrimvarToken(class_varying,  type_float,   1, "t"),
	CqPrimvarToken(class_varying,  type_float,   2, "st"),
	CqPrimvarToken(class_varying,  type_float,   1, "width"),
	CqPrimvarToken(class_constant, type_float,   1, "constantwidth"),

	//--------------------------------------------------
	// Standard Options
	// Option "limits"
	CqPrimvarToken(class_uniform,  type_integer, 1, "gridsize"),
	CqPrimvarToken(class_uniform,  type_integer, 1, "texturememory"),
	CqPrimvarToken(class_uniform,  type_integer, 2, "bucketsize"),
	CqPrimvarToken(class_uniform,  type_integer, 1, "eyesplits"),
	CqPrimvarToken(class_uniform,  type_color,   1, "zthreshold"),
	// Option "searchpath"
	CqPrimvarToken(class_uniform,  type_string,  1, "shader"),
	CqPrimvarToken(class_uniform,  type_string,  1, "archive"),
	CqPrimvarToken(class_uniform,  type_string,  1, "texture"),
	CqPrimvarToken(class_uniform,  type_string,  1, "display"),
	CqPrimvarToken(class_uniform,  type_string,  1, "procedural"),
	CqPrimvarToken(class_uniform,  type_string,  1, "resource"),
	// Option "statistics"
	CqPrimvarToken(class_uniform,  type_integer, 1, "endofframe"),
	CqPrimvarToken(class_uniform,  type_integer, 1, "echoapi"),
	// Option "shutter"
	CqPrimvarToken(class_uniform,  type_float,   1, "offset"),
	// Projection
	CqPrimvarToken(class_uniform,  type_float,   1, "fov"),

	//--------------------------------------------------
	// Standard Attributes
	// Attribute "displacementbound"
	CqPrimvarToken(class_uniform,  type_float,   1, "sphere"),
	CqPrimvarToken(class_uniform,  type_string,  1, "coordinatesystem"),
	// Attribute "identifier"
	CqPrimvarToken(class_uniform,  type_string,  1, "name"),
	CqPrimvarToken(class_uniform,  type_string,  1, "shadinggroup"), // (not used in aqsis)
	// Attribute "trimcurve"
	CqPrimvarToken(class_uniform,  type_string,  1, "sense"),
	// Attribute "shadow"
	CqPrimvarToken(class_uniform,  type_float,   1, "bias0"),
	CqPrimvarToken(class_uniform,  type_float,   1, "bias1"),
	CqPrimvarToken(class_uniform,  type_float,   1, "bias"),
	// Display
	CqPrimvarToken(class_uniform,  type_string,  1, "compression"),
	CqPrimvarToken(class_uniform,  type_integer, 1, "quality"),
	CqPrimvarToken(class_uniform,  type_float,   4, "quantize"),
	CqPrimvarToken(class_uniform,  type_float,   1, "dither"),
	// Hider
	CqPrimvarToken(class_uniform,  type_integer, 1, "jitter"),
	CqPrimvarToken(class_uniform,  type_string,  1, "depthfilter"),
	// Attribute "dice"
	CqPrimvarToken(class_uniform,  type_integer, 1, "binary"),
	// Attribute "mpdump"
	CqPrimvarToken(class_uniform,  type_integer, 1, "enabled"),
	// Attribute "derivatives"
	CqPrimvarToken(class_uniform,  type_integer, 1, "centered"),

	//--------------------------------------------------
	// Aqsis-specific options / attributes
	// Attribute "autoshadows"
	CqPrimvarToken(class_uniform,  type_string,  1, "shadowmapname"),
	CqPrimvarToken(class_uniform,  type_integer, 1, "res"),
	// Attribute "Render"
	CqPrimvarToken(class_uniform,  type_integer, 1, "multipass"),
	// Attribute "aqsis"
	CqPrimvarToken(class_uniform,  type_float,   1, "expandgrids"),

	//--------------------------------------------------
	// Extra options not used by aqsis, but apparently commonly exported in RIB files.
	// Attribute "light"
	CqPrimvarToken(class_uniform,  type_string,  1, "shadows"),
};

const std::vector<CqPrimvarToken> standardVars(standardVarsInit,
		standardVarsInit + sizeof(standardVarsInit)/sizeof(CqPrimvarToken));

} // unnamed namespace

const std::vector<CqPrimvarToken>& standardPrimvars()
{
	return standardVars;
}

}
