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

#include <aqsis/aqsis.h>

#include <vector>

#include <aqsis/util/file.h>
#include <aqsis/riutil/riparamlist.h>
#include <aqsis/tex/filtering/wrapmode.h>

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
 * The output file contains a duplicate of the recognized metadata from the
 * input file, so stuff like transformation matrices will be preserved where
 * possible.
 *
 * \param inFileName - full path to the input texture file.
 * \param outFileName - full path to the output texture map file.
 * \param filterInfo - information about which filter type and size to use
 * \param wrapModes - how the texture will be wrapped at the edges.
 * \param paramList - A renderman param list of extra optional control
 *                    parameters for the mipmapping procedure.
 */
AQSIS_TEX_SHARE void makeTexture(const boostfs::path& inFileName, 
		const boostfs::path& outFileName, SqFilterInfo filterInfo, 
		const SqWrapModes& wrapModes, const CqRiParamList& paramList);

/** \brief Convert six texture files into a cube face environment map
 *
 * The input texture files represent the view through the faces of a cube, and
 * must all be of the same resolution.  Faces should be rendered with a larger
 * field of view slightly than 90 degrees to avoid filtering artifacts where
 * the faces meet.
 *
 * The output texture is a mipmapped tiled TIFF file for efficient texture
 * lookup.  Each mipmap level stores the full set of six cube faces in the
 * order
 *
 * \verbatim
 *
 *   +----+----+----+
 *   | +x | +y | +z |
 *   |    |    |    |
 *   +----+----+----+
 *   | -x | -y | -z |
 *   |    |    |    |
 *   +----+----+----+
 *
 * \endverbatim
 *
 * The appropriate orientation for the faces is specified by the RISpec in the
 * section detailing RiMakeCubeFaceEnvironment.
 *
 * The output file contains a duplicate of the recognized metadata from the
 * input file in the +x direction (inNamePx), so stuff like transformation
 * matrices will be preserved where possible.
 *
 * \param inNamePx - cube face in +x direction (full path)
 * \param inNameNx - cube face in -x direction (full path)
 * \param inNamePy - cube face in +y direction (full path)
 * \param inNameNy - cube face in -y direction (full path)
 * \param inNamePz - cube face in +z direction (full path)
 * \param inNameNz - cube face in -z direction (full path)
 * \param outFileName - full path to the output texture map file.
 * \param fieldOfView - full field of view in degrees.
 * \param filterInfo - information about which filter type and size to use
 * \param paramList - A renderman param list of extra optional control
 *                    parameters for the mipmapping procedure.
 */
AQSIS_TEX_SHARE void makeCubeFaceEnvironment(
		const boostfs::path& inNamePx, const boostfs::path& inNameNx, 
		const boostfs::path& inNamePy, const boostfs::path& inNameNy, 
		const boostfs::path& inNamePz, const boostfs::path& inNameNz, 
		const boostfs::path& outFileName, TqFloat fieldOfView,
		SqFilterInfo filterInfo, const CqRiParamList& paramList);

/** \brief Convert a texture file into a latlong environment map
 *
 * The input texture coordinates are assumed to correspond to the latitude and
 * longitude for the vertical and horizontal directions respectively.  The
 * output texture is mipmapped tiled TIFF file for efficient texture lookup.
 *
 * The output file contains a duplicate of the recognized metadata from the
 * input file, so stuff like transformation matrices will be preserved where
 * possible.
 *
 * \param inFileName - full path to the input texture file.
 * \param outFileName - full path to the output texture map file.
 * \param filterInfo - information about which filter type and size to use
 * \param paramList - A renderman param list of extra optional control
 *                    parameters for the mipmapping procedure.
 */
AQSIS_TEX_SHARE void makeLatLongEnvironment(
		const boostfs::path& inFileName, const boostfs::path& outFileName,
		SqFilterInfo filterInfo, const CqRiParamList& paramList);

/** \brief Convert a texture file to a shadow map.
 *
 * This function requires that the input data format be 32bit floating point.
 * World to camera and world to screen matrices must also be present.  The
 * output file is a tiled TIFF file which is efficient for shadow mapping.
 *
 * The output file contains a duplicate of the recognized metadata from the
 * input file where possible so the transformation matrices (among other
 * things) are preserved where possible.
 *
 * \param inFileName - full path to the input texture file.
 * \param outFileName - full path to the output texture map file.
 * \param paramList - A renderman param list of extra optional control
 *                    parameters for the shadow map generation.
 */
AQSIS_TEX_SHARE void makeShadow(const boostfs::path& inFileName, 
		const boostfs::path& outFileName, const CqRiParamList& paramList);

/** \brief Convert a collection of depth maps into an occlusion map.
 *
 * This function requires that each input file be in 32bit floating point
 * format.  World to camera and world to screen matrices must also be present
 * in each file.  The output file is a multi-image tiled TIFF file which is
 * efficient for ambient occlusion lookups.
 *
 * \param inFiles - List of full paths to file names containing depth maps of a
 *                  scene rendered from various different viewpoints.
 * \param outFileName - full path to output occlusion map file
 * \param paramList - A renderman param list of extra optional control
 *                    parameters for the occlusion map generation.
 */
AQSIS_TEX_SHARE void makeOcclusion( const std::vector<boostfs::path>& inFiles,
		const boostfs::path& outFileName, const CqRiParamList& paramList);

} // namespace Aqsis

#endif // MAKETEXTURE_H_INCLUDED
