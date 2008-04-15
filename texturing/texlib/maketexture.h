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
 * \brief Functions for creating texture maps.
 *
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#ifndef MAKETEXTURE_H_INCLUDED
#define MAKETEXTURE_H_INCLUDED

#include "aqsis.h"

#include <string>

#include "wrapmode.h"
#include "riparamlist.h"

namespace Aqsis {


//------------------------------------------------------------------------------
// Filter function type which coincides with the RtFilterFunc type from ri.h
typedef TqFloat (*TqFilterFunc)(TqFloat, TqFloat, TqFloat, TqFloat);

/// Filtering information for downsampling.
struct SqFilterInfo
{
	/// renderman filter function used in downsampling
	TqFilterFunc filterFunc;
	/// filter width in x-direction
	TqFloat xWidth;
	/// filter width in y-direction
	TqFloat yWidth;
	//bool isSeparable;

	/// trivial constructor
	SqFilterInfo(TqFilterFunc filterFunc, TqFloat xWidth, TqFloat yWidth)
		: filterFunc(filterFunc), xWidth(xWidth), yWidth(yWidth)
	{ }
};


//------------------------------------------------------------------------------
/// String constants which describe the various texture types.
extern const char* g_plainTextureFormatStr;
extern const char* g_cubeEnvTextureFormatStr;
extern const char* g_latlongEnvTextureFormatStr;
extern const char* g_shadowTextureFormatStr;


//------------------------------------------------------------------------------
/** \brief Convert a plain texture file to the mipmapped format used internally.
 *
 * See mipmap.h for details on the assumptions behind mipmapping in aqsis.
 *
 * \param inFileName - full path to the input texture file.
 * \param outFileName - full path to the output texture map file.
 * \param filterInfo - information about which filter type and size to use
 * \param wrapModes - how the texture will be wrapped at the edges.
 * \param paramList - A renderman param list of extra optional control
 *                    parameters for the mipmapping procedure.
 */
AQSISTEX_SHARE void makeTexture(const std::string& inFileName, 
		const std::string& outFileName,
		const SqFilterInfo& filterInfo, 
		const SqWrapModes& wrapModes,
		const CqRiParamList& paramList);


} // namespace Aqsis

#endif // MAKETEXTURE_H_INCLUDED
