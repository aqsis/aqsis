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
/// Filtering information for downsampling.
struct SqFilterInfo
{
	/// renderman filter function used in downsampling
	RtFilterFunc filterFunc;
	/// filter width in x-direction
	TqFloat xWidth;
	/// filter width in y-direction
	TqFloat yWidth;
	//bool isSeparable;

	/// trivial constructor
	SqFilterInfo(RtFilterFunc filterFunc = 0, TqFloat xWidth = 1, TqFloat yWidth = 1)
		: filterFunc(filterFunc), xWidth(xWidth), yWidth(yWidth)
	{ }
};


//------------------------------------------------------------------------------
/** \brief Convert a plain texture file to the mipmapped format used internally.
 *
 * The output file is a multi-image tiled TIFF file which is efficient for
 * texture lookup.  See downsample.h for details on the assumptions behind
 * mipmapping in aqsis.
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

/** \brief Convert a texture file into a latlong environment map
 *
 * The input texture coordinates are assumed to correspond to the latitude and
 * longitude for the vertical and horizontal directions respectively.  The
 * output texture is mipmapped tiled TIFF file for efficient texture lookup.
 *
 * \param inFileName - full path to the input texture file.
 * \param outFileName - full path to the output texture map file.
 * \param filterInfo - information about which filter type and size to use
 * \param paramList - A renderman param list of extra optional control
 *                    parameters for the mipmapping procedure.
 */
AQSISTEX_SHARE void makeLatLongEnvironment(
		const std::string& inFileName, 
		const std::string& outFileName,
		const SqFilterInfo& filterInfo, 
		const CqRiParamList& paramList);

/** \brief Convert a texture file to a shadow map.
 *
 * This function requires that the input data format be 32bit floating point.
 * World to camera and world to screen matrices must also be present.  The
 * output file is a tiled TIFF file which is efficient for shadow mapping.
 *
 * \param inFileName - full path to the input texture file.
 * \param outFileName - full path to the output texture map file.
 * \param paramList - A renderman param list of extra optional control
 *                    parameters for the shadow optimization.
 */
AQSISTEX_SHARE void makeShadow(const std::string& inFileName, 
		const std::string& outFileName,
		const CqRiParamList& paramList);

} // namespace Aqsis

#endif // MAKETEXTURE_H_INCLUDED
