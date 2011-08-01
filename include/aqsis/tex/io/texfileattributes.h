// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/** \file
 *
 * \brief Declare standard image attributes
 *
 * \author Chris Foster
 */


#ifndef TEXFILEATTRIBUTES_H_INCLUDED
#define TEXFILEATTRIBUTES_H_INCLUDED

#include <aqsis/aqsis.h>

#include <string>

#include <aqsis/tex/buffers/channellist.h>
#include <aqsis/math/matrix.h>
#include <aqsis/tex/filtering/wrapmode.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/// A box type specifying an image region.
struct SqImageRegion
{
	TqInt width;     ///< width of the box
	TqInt height;    ///< height of the box
	TqInt topLeftX;  ///< x-position of the top left of the box.
	TqInt topLeftY;  ///< y-position of the top left of the box.

	/// Trivial constructor
	SqImageRegion(TqInt width = 0, TqInt height = 0,
			TqInt topLeftX = 0, TqInt topLeftY = 0);
};

//------------------------------------------------------------------------------
/// Hold information about texture tiles (width, height, ...)
struct SqTileInfo
{
	TqInt width;
	TqInt height;

	/// Trivial constructor
	SqTileInfo(TqInt width = 32, TqInt height = 32)
		: width(width), height(height) {}
};

//------------------------------------------------------------------------------
/// Enumeration specifying the texture format
enum EqTextureFormat
{
	TextureFormat_Plain,
	TextureFormat_CubeEnvironment,
	TextureFormat_LatLongEnvironment,
	TextureFormat_Shadow,
	TextureFormat_Occlusion,
	TextureFormat_Unknown
};

//------------------------------------------------------------------------------
/** \brief Standard image header attributes.
 *
 * This namespace contains all the "tag" structs which represent possible image
 * attributes to be stored in a CqTexFileHeader
 */
namespace Attr
{
	/** \brief Macro to aid in defining the standard image attributes
	 *
	 * It's not certain that the name() function is necessary at this stage,
	 * but it might prove useful for diagnostics
	 */
#	define AQSIS_IMAGE_ATTR_TAG(attrName, attrType)                       \
	struct attrName                                                       \
	{                                                                     \
		typedef attrType type;                                            \
		static const char* name() { return #attrName; }                   \
	}

	//--------------------------------------------------
	/** \name DisplayWindow
	 *
	 * In addition to the image dimensions, the header may specify a
	 * DisplayWindow attribute (mainly for use with cropped images).  The
	 * display window specifies the extent of the non-cropped image.  The
	 * coordinates used are such that the image data of size Width x Height
	 * has top left coordinates of (0,0).  (So negative values of
	 * topLeftX,topLeftY are valid).
	 *
	 * \verbatim
	 *
	 * (topLeftX, topLeftY)
	 *  x---Display window-------------------+
	 *  |                                    |
	 *  |                                    |
	 *  |      (0,0)                         |
	 *  |       x----Data window------+      |
	 *  |       |                     | |    |
	 *  |       |                     | |    |
	 *  |       |                     |      |
	 *  |       |                   Height   |
	 *  |       |                     |      |
	 *  |       |                     | |    |
	 *  |       |                     | |    |
	 *  |       |                     | |    |
	 *  |       |                     | v    |
	 *  |       +---------------------+      |
	 *  |        ------ Width ------->       |
	 *  |                                    |
	 *  +------------------------------------+
	 *
	 * \endverbatim
	 */
	AQSIS_IMAGE_ATTR_TAG(DisplayWindow, SqImageRegion);
	/// aspect ratio = pix_width/pix_height
	AQSIS_IMAGE_ATTR_TAG(PixelAspectRatio, TqFloat);

	//--------------------------------------------------
	/// Tile information
	AQSIS_IMAGE_ATTR_TAG(TileInfo, SqTileInfo);

	//--------------------------------------------------
	/// Information strings
	// image creation software
	AQSIS_IMAGE_ATTR_TAG(Software, std::string);
	// computer host name
	AQSIS_IMAGE_ATTR_TAG(HostName, std::string);
	// description of image
	AQSIS_IMAGE_ATTR_TAG(Description, std::string);
	// date and time of creation
	// (this could usefully be a struct rather than a string in future)
	AQSIS_IMAGE_ATTR_TAG(DateTime, std::string);

	//--------------------------------------------------
	/// Texture map metadata
	// texture wrap modes
	AQSIS_IMAGE_ATTR_TAG(WrapModes, SqWrapModes);
	// texture format (one of "texture" "shadow" or "environment" ? )
	/// \todo Decide on standard names for these.
	AQSIS_IMAGE_ATTR_TAG(TextureFormat, EqTextureFormat);
	// cotan(FOV/2) - this is needed for environment map lookup
	AQSIS_IMAGE_ATTR_TAG(FieldOfViewCot, TqFloat);

	//--------------------------------------------------
	/// Transformation matrices
	AQSIS_IMAGE_ATTR_TAG(WorldToScreenMatrix, CqMatrix);
	AQSIS_IMAGE_ATTR_TAG(WorldToCameraMatrix, CqMatrix);

	//--------------------------------------------------
	/// Compression
	/// Compression type
	AQSIS_IMAGE_ATTR_TAG(Compression, std::string);
	/// compression quality (for lossy compression)
	AQSIS_IMAGE_ATTR_TAG(CompressionQuality, TqInt);
}


//==============================================================================
// Implementation details
//==============================================================================

inline SqImageRegion::SqImageRegion(TqInt width, TqInt height,
		TqInt topLeftX, TqInt topLeftY)
	: width(width),
	height(height),
	topLeftX(topLeftX),
	topLeftY(topLeftY)
{ }

//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TEXFILEATTRIBUTES_H_INCLUDED
