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
 * \brief Implementation of functions for creating texture maps.
 *
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#include "maketexture.h"

#include <boost/shared_ptr.hpp>

#include "bake.h"
#include "itexinputfile.h"
#include "itexoutputfile.h"
#include "magicnumber.h"
#include "downsample.h"
#include "texturebuffer.h"
#include "texexception.h"
#include "version.h"

namespace Aqsis {


//------------------------------------------------------------------------------
namespace {

/** /brief Downsample the provided buffer into the given output file.
 *
 * \param buf - Pointer to source data.  This smart pointer is reset to save
 *              memory during the mipmapping process, so make sure a copy of it
 *              is kept elsewhere if desired.
 * \param outFile - output file for the mipmapped data
 * \param filterInfo - information about which filter type and size to use
 * \param wrapModes - specifies how the texture will be wrapped at the edges.
 */
template<typename ChannelT>
void downsampleToFile(boost::shared_ptr<CqTextureBuffer<ChannelT> >& buf,
		IqMultiTexOutputFile& outFile, const SqFilterInfo& filterInfo,
		const SqWrapModes wrapModes)
{
	outFile.writePixels(*buf);
	typedef CqDownsampleIterator<CqTextureBuffer<ChannelT> > TqDownsampleIter;
	for(TqDownsampleIter i = ++TqDownsampleIter(buf, filterInfo, wrapModes),
			end = TqDownsampleIter(); i != end; ++i)
	{
		buf = *i;
		outFile.newSubImage(buf->width(), buf->height());
		outFile.writePixels(*buf);
	}
}

/** \brief Create a mipmapped file from the image found in the given input file.
 *
 * \param inFile - input file from which the data should be read
 * \param outFileName - name of output file for the mipmapped data
 * \param filterInfo - information about which filter type and size to use
 * \param wrapModes - specifies how the texture will be wrapped at the edges.
 */
template<typename ChannelT>
void createMipmapFile(IqTexInputFile& inFile, const std::string& outFileName,
		CqTexFileHeader& header, const SqFilterInfo& filterInfo,
		const SqWrapModes wrapModes)
{
	// Read pixels into the input buffer.
	boost::shared_ptr<CqTextureBuffer<ChannelT> > buf(new CqTextureBuffer<ChannelT>());
	inFile.readPixels(*buf);
	// Create mipmap
	boost::shared_ptr<IqMultiTexOutputFile> outFile
		= IqMultiTexOutputFile::open(outFileName, ImageFile_Tiff, header);
	downsampleToFile(buf, *outFile, filterInfo, wrapModes);
}

/** \brief Create a mipmap from input data in the openEXR "half" data format.
 *
 * This is a special case, since the "half" data needs to be converted into
 * floating point data because TIFF files don't support half data.
 *
 * \see createMipmapFile, generic version.
 */
void createMipmapFileHalf(IqTexInputFile& inFile, const std::string& outFileName,
		CqTexFileHeader& header, const SqFilterInfo& filterInfo,
		const SqWrapModes wrapModes)
{
#	ifdef USE_OPENEXR
	// Read pixels into the input buffer.
	CqTextureBuffer<half> halfBuf;
	inFile.readPixels(halfBuf);
	// Convert to 32-bit floating point; we can't handle the half data
	// type in TIFF.
	boost::shared_ptr<CqTextureBuffer<TqFloat> > buf(
			new CqTextureBuffer<TqFloat>(halfBuf));
	// Correct the channel list in the output file header so that it indicates
	// float32 data.  We don't bother to preserve the channel names since they
	// can't be stored natively by TIFF anyway.
	header.channelList() = CqChannelList(Channel_Float32,
				header.channelList().numChannels());
	// Create mipmap
	boost::shared_ptr<IqMultiTexOutputFile> outFile
		= IqMultiTexOutputFile::open(outFileName, ImageFile_Tiff, header);
	downsampleToFile(buf, *outFile, filterInfo, wrapModes);
#	else
	assert(0);
#	endif
}

} // unnamed namespace


//------------------------------------------------------------------------------
void makeTexture(const std::string& inFileName, const std::string& outFileName,
		const SqFilterInfo& filterInfo, const SqWrapModes& wrapModes,
		const CqRiParamList& paramList)
{
	std::string inFileRealName = inFileName;
	if(guessFileType(inFileName) == ImageFile_AqsisBake)
	{
		inFileRealName = inFileName + ".tif";

		TqInt bakeRes = static_cast<TqInt>(paramList.find<TqFloat>("bake", 256));
		bakeToTiff(inFileName.c_str(), inFileRealName.c_str(), bakeRes);
	}

	boost::shared_ptr<IqTexInputFile> inFile = IqTexInputFile::open(inFileRealName);

	// Take a copy of the file header.  This means that the output file will
	// inherit all the recognized attributes of the input file.
	CqTexFileHeader header = inFile->header();

	// Set some attributes in the new file header.
	header.set<Attr::WrapModes>(wrapModes);
	header.set<Attr::TextureFormat>(TextureFormat_Plain);
	header.set<Attr::TileInfo>(SqTileInfo(32,32));
	header.set<Attr::Software>(STRNAME " " VERSION_STR_PRINT);

	if(const char* const* comp = paramList.find<char*>("compression"))
		header.set<Attr::Compression>(*comp);
	if(const TqFloat* quality = paramList.find<TqFloat>("quality"))
		header.set<Attr::CompressionQuality>(static_cast<TqInt>(*quality));

	// Open output file & create mipmap.
	switch(inFile->header().channelList().sharedChannelType())
	{
		case Channel_Float32:
			createMipmapFile<TqFloat>(*inFile, outFileName, header, filterInfo, wrapModes);
			break;
		case Channel_Unsigned32:
			createMipmapFile<TqUint32>(*inFile, outFileName, header, filterInfo, wrapModes);
			break;
		case Channel_Signed32:
			createMipmapFile<TqInt32>(*inFile, outFileName, header, filterInfo, wrapModes);
			break;
		case Channel_Unsigned16:
			createMipmapFile<TqUint16>(*inFile, outFileName, header, filterInfo, wrapModes);
			break;
		case Channel_Signed16:
			createMipmapFile<TqInt16>(*inFile, outFileName, header, filterInfo, wrapModes);
			break;
		case Channel_Unsigned8:
			createMipmapFile<TqUint8>(*inFile, outFileName, header, filterInfo, wrapModes);
			break;
		case Channel_Signed8:
			createMipmapFile<TqInt8>(*inFile, outFileName, header, filterInfo, wrapModes);
			break;
		case Channel_Float16:
			createMipmapFileHalf(*inFile, outFileName, header, filterInfo, wrapModes);
			break;
		default:
			assert(0);
	}
}

void makeShadow(const std::string& inFileName, 
		const std::string& outFileName,
		const CqRiParamList& paramList)
{
	boost::shared_ptr<IqTexInputFile> inFile = IqTexInputFile::open(inFileName);

	// Take a copy of the file header.  This means that the output file will
	// inherit all the recognized attributes of the input file.
	CqTexFileHeader header = inFile->header();

	// Ensure that the header contains 32-bit floating poing data.  It might be
	// possible to relax this requirement to also allow 16-bit OpenEXR "half"
	// data...
	if(header.channelList().sharedChannelType() != Channel_Float32)
		AQSIS_THROW(XqBadTexture, "input for shadow map creation must "
				"contain 32 bit floating point data");

	// Ensure that the screen and camera transformation matrices are present.
	// If not, the texture will be useless for shadow mapping.
	if( header.findPtr<Attr::WorldToCameraMatrix>() == 0
			|| header.findPtr<Attr::WorldToCameraMatrix>() == 0 )
	{
		AQSIS_THROW(XqBadTexture, "world->camera and world->screen"
				" matrices not specified in input file");
	}

	// Set some attributes in the new file header.
	header.set<Attr::TextureFormat>(TextureFormat_Shadow);
	header.set<Attr::TileInfo>(SqTileInfo(32,32));
	header.set<Attr::Software>(STRNAME " " VERSION_STR_PRINT);

	if(const char* const* comp = paramList.find<char*>("compression"))
		header.set<Attr::Compression>(*comp);

	// Read all pixels into a buffer (not particularly memory efficient...)
	CqTextureBuffer<TqFloat> pixelBuf;
	inFile->readPixels(pixelBuf);
	// Open output file and write pixel data.
	boost::shared_ptr<IqTexOutputFile> outFile
		= IqTexOutputFile::open(outFileName, ImageFile_Tiff, header);
	outFile->writePixels(pixelBuf);
}

} // namespace Aqsis
