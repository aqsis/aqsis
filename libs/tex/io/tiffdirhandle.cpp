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
 * \brief A C++ wrapper around tiff files for the functions of interest in aqsis.
 *
 * \author Chris Foster
 */

#include "tiffdirhandle.h"

#include <sstream>
#include <cstring>  // for memcpy()

#include <tiffio.hxx>

#include <aqsis/math/math.h>
#include <aqsis/math/matrix.h>
#include <aqsis/util/logging.h>
#include <aqsis/tex/texexception.h>

namespace Aqsis
{

/// An exception for internal usage by the tiff handling code.
AQSIS_DECLARE_XQEXCEPTION(XqUnknownTiffFormat, XqInternal);

//------------------------------------------------------------------------------
// Helper functions and data for dealing with tiff <--> header conversions.
//------------------------------------------------------------------------------
namespace {

/// String constants which describe the various texture types.
const char* plainTextureFormatStr = "Plain Texture";
const char* cubeEnvTextureFormatStr = "CubeFace Environment";
const char* latlongEnvTextureFormatStr = "LatLong Environment";
const char* shadowTextureFormatStr = "Shadow";
const char* occlusionTextureFormatStr = "Occlusion";

/// Convert from a string to an EqTextureFormat
EqTextureFormat texFormatFromString(const std::string& str)
{
	if(str == plainTextureFormatStr)
		return TextureFormat_Plain;
	else if(str == cubeEnvTextureFormatStr)
		return TextureFormat_CubeEnvironment;
	else if(str == latlongEnvTextureFormatStr)
		return TextureFormat_LatLongEnvironment;
	else if(str == shadowTextureFormatStr)
		return TextureFormat_Shadow;
	else if(str == occlusionTextureFormatStr)
		return TextureFormat_Occlusion;
	return TextureFormat_Unknown;
}

/// Convert from an EqTextureFormat to a string.
const char* texFormatToString(EqTextureFormat format)
{
	switch(format)
	{
		case TextureFormat_Plain:
			return plainTextureFormatStr;
		case TextureFormat_CubeEnvironment:
			return cubeEnvTextureFormatStr;
		case TextureFormat_LatLongEnvironment:
			return latlongEnvTextureFormatStr;
		case TextureFormat_Shadow:
			return shadowTextureFormatStr;
		case TextureFormat_Occlusion:
			return occlusionTextureFormatStr;
		case TextureFormat_Unknown:
			return "unknown";
	}
	assert("unhandled format type" && 0);
	return "unknown"; // shut up compiler warning.
}

typedef std::pair<uint16, const char*> TqComprPair;
TqComprPair comprTypesInit[] = {
	TqComprPair(COMPRESSION_NONE, "none"),
	TqComprPair(COMPRESSION_LZW, "lzw"),
	TqComprPair(COMPRESSION_JPEG, "jpeg"),
	TqComprPair(COMPRESSION_PACKBITS, "packbits"),
	TqComprPair(COMPRESSION_SGILOG, "log"),
	TqComprPair(COMPRESSION_DEFLATE, "deflate"),
};
/// vector holding the TIFF compression alternatives that we want to deal with.
const std::vector<TqComprPair> compressionTypes( comprTypesInit,
		comprTypesInit + sizeof(comprTypesInit)/sizeof(TqComprPair));

/// Get the tiff compression type from a string description.
uint16 tiffCompressionTagFromName(const std::string& compressionName)
{
	for(std::vector<TqComprPair>::const_iterator i = compressionTypes.begin();
			i != compressionTypes.end(); ++i)
	{
		if(i->second == compressionName)
			return i->first;
	}
	return COMPRESSION_NONE;
}

/// Get the tiff compression type from a string description.
const char* tiffCompressionNameFromTag(uint16 compressionType)
{
	for(std::vector<TqComprPair>::const_iterator i = compressionTypes.begin();
			i != compressionTypes.end(); ++i)
	{
		if(i->first == compressionType)
			return i->second;
	}
	return "unknown";
}

} // unnamed namespace

//------------------------------------------------------------------------------
// CqTiffDirHandle
//------------------------------------------------------------------------------

CqTiffDirHandle::CqTiffDirHandle(const boost::shared_ptr<CqTiffFileHandle>& fileHandle, const tdir_t dirIdx)
	: m_fileHandle(fileHandle)
{
	fileHandle->setDirectory(dirIdx);
}

tdir_t CqTiffDirHandle::dirIndex() const
{
	return m_fileHandle->m_currDir;
}

void CqTiffDirHandle::fillHeader(CqTexFileHeader& header) const
{
	fillHeaderRequiredAttrs(header);
	fillHeaderOptionalAttrs(header);
	fillHeaderPixelLayout(header);
}

void CqTiffDirHandle::writeHeader(const CqTexFileHeader& header)
{
	writeRequiredAttrs(header);
	writeOptionalAttrs(header);
}

void CqTiffDirHandle::writeRequiredAttrs(const CqTexFileHeader& header)
{
	// Width, height...
	setTiffTagValue<uint32>(TIFFTAG_IMAGEWIDTH, header.width());
	setTiffTagValue<uint32>(TIFFTAG_IMAGELENGTH, header.height());

	// Orientation & planar config should always be fixed.
	setTiffTagValue<uint16>(TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	setTiffTagValue<uint16>(TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	// Pixel aspect ratio
	// We have no meaningful resolution unit - we're only interested in pixel
	// aspect ratio, so set the resolution unit to none.
	setTiffTagValue<uint16>(TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);
	setTiffTagValue<float>(TIFFTAG_XRESOLUTION, 1.0f);
	setTiffTagValue<float>(TIFFTAG_YRESOLUTION, header.find<Attr::PixelAspectRatio>(1));

	// Compression-related stuff
	writeCompressionAttrs(header);
	// Channel-related stuff
	writeChannelAttrs(header);

	const SqTileInfo* tileInfo = header.findPtr<Attr::TileInfo>();
	if(tileInfo)
	{
		// Set tile dimensions if present.
		setTiffTagValue<uint32>(TIFFTAG_TILEWIDTH, tileInfo->width);
		setTiffTagValue<uint32>(TIFFTAG_TILELENGTH, tileInfo->height);
	}
	else
	{
		// Else write strip size - AFAICT libtiff uses the values of some other
		// fields (compression) to choose a default, so do this last.
		setTiffTagValue<uint32>(TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tiffPtr(), 0));
	}
}

void CqTiffDirHandle::writeCompressionAttrs(const CqTexFileHeader& header)
{
	// Set the compression type.
	uint16 compression = tiffCompressionTagFromName(header.find<Attr::Compression>("none"));
	if(!TIFFIsCODECConfigured(compression))
	{
		Aqsis::log() << warning << "No TIFF codec found for compression scheme \""
			<< header.find<Attr::Compression>("none") << "\"\n";
		return;
	}
	setTiffTagValue<uint16>(TIFFTAG_COMPRESSION, compression);

	if(compression == COMPRESSION_LZW || compression == COMPRESSION_DEFLATE)
	{
		// Add a compression predictor if possible; this drastically increases
		// the compression ratios.  Even though the online docs seem to suggest
		// that predictors are independent of the compression codec, this is
		// not the case for libtiff, which appears to give errors if predictors
		// used with anything other than the lzw or deflate codecs.
		//
		// (the innards of libtiff suggest that TIFFPredictorInit() is only
		// called by certian codecs)
		//
		// \todo Test whether PREDICTOR_FLOATINGPOINT is actually beneficial.
		// (Some places on the web suggest not.)
		if(header.channelList().sharedChannelType() == Channel_Float32)
			setTiffTagValue<uint16>(TIFFTAG_PREDICTOR, PREDICTOR_FLOATINGPOINT);
		else
			setTiffTagValue<uint16>(TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
	}
	if(compression == COMPRESSION_JPEG)
	{
		// Set the jpeg compression quality level if necessary.
		setTiffTagValue<int>(TIFFTAG_JPEGQUALITY,
				header.find<Attr::CompressionQuality>(85));
	}
}

void CqTiffDirHandle::writeChannelAttrs(const CqTexFileHeader& header)
{
	const CqChannelList& channelList = header.channelList();
	EqChannelType channelType = channelList.sharedChannelType();
	// Assume that the channel type is uniform across the various channels.
	assert(channelType != Channel_TypeUnknown && channelType != Channel_Float16);
	TqInt numChannels = channelList.numChannels();
	assert(numChannels > 0);

	setTiffTagValue<uint16>(TIFFTAG_SAMPLESPERPIXEL, numChannels); 
	setTiffTagValue<uint16>(TIFFTAG_BITSPERSAMPLE, 8*bytesPerPixel(channelType));
	// It's hard to know which algorithm for deciding the photometric type is
	// the best here.  Perhaps it would be better to simply depend on the
	// number of channels, since TIFF doesn't have a standard facility to store
	// channel names...
	if( (channelList.hasIntensityChannel() || numChannels <= 2)
			&& !channelList.hasRgbChannel() )
	{
		// greyscale image
		setTiffTagValue<uint16>(TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		if(numChannels == 2)
		{
			// Set extra sample types
			std::vector<uint16> extraSamples(numChannels - 1, EXTRASAMPLE_UNSPECIFIED);
			if(channelList[1].name == "a")
				extraSamples[0] = EXTRASAMPLE_ASSOCALPHA;
			setTiffTagValue(TIFFTAG_EXTRASAMPLES, extraSamples);
		}
		// \todo PHOTOMETRIC_LOGL alternative for floats
	}
	else
	{
		// Assume a colour image by default (use PHOTOMETRIC_RGB)
		setTiffTagValue<uint16>(TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		/// \todo PHOTOMETRIC_LOGLUV alternative for floats
		if(numChannels > 3)
		{
			std::vector<uint16> extraSamples(numChannels - 3, EXTRASAMPLE_UNSPECIFIED);
			// Set type of extra samples.
			if(channelList[3].name == "a")
				extraSamples[0] = EXTRASAMPLE_ASSOCALPHA;
			if(numChannels >= 6)
			{
				// Initial support for setting extra samples for three channel
				// alpha... This isn't likely to be terribly robust...
				if(channelList[0].name == "r" && channelList[3].name == "ra")
					extraSamples[0] = EXTRASAMPLE_ASSOCALPHA;
				if(channelList[1].name == "g" && channelList[4].name == "ga")
					extraSamples[1] = EXTRASAMPLE_ASSOCALPHA;
				if(channelList[2].name == "b" && channelList[5].name == "ba")
					extraSamples[2] = EXTRASAMPLE_ASSOCALPHA;
			}
			setTiffTagValue(TIFFTAG_EXTRASAMPLES, extraSamples);
		}
	}
	/// \todo: deal with TIFFTAG_SGILOGDATAFMT
	uint16 sampleFormat = 0;
	switch(channelType)
	{
        case Channel_Float32:
			sampleFormat = SAMPLEFORMAT_IEEEFP;
			break;
        case Channel_Signed32:
        case Channel_Signed16:
        case Channel_Signed8:
			sampleFormat = SAMPLEFORMAT_INT;
			break;
        case Channel_Unsigned32:
        case Channel_Unsigned16:
        case Channel_Unsigned8:
			sampleFormat = SAMPLEFORMAT_UINT;
			break;
		default:
			AQSIS_THROW_XQERROR(XqInternal, EqE_Limit,
				"Cannot handle provided pixel sample format");
			break;
    }
	setTiffTagValue<uint16>(TIFFTAG_SAMPLEFORMAT, sampleFormat);
}


namespace {

/** Convert a type held in the header to the appropriate type understood by
 * libtiff.
 */
template<typename Tattr, typename Ttiff>
Ttiff attrTypeToTiff(const Tattr& attr)
{
	return Ttiff(attr);
}
// Specialize for std::string -> const char*
template<>
const char* attrTypeToTiff(const std::string& attr)
{
	return attr.c_str();
}
// Specialize for CqMatrix -> TqFloat*
template<>
const float* attrTypeToTiff(const CqMatrix& attr)
{
	return attr.pElements();
}
// Specialize for EqTextureFormat -> const char*
template<>
const char* attrTypeToTiff(const EqTextureFormat& format)
{
	return texFormatToString(format);
}

/**
 * Add an attribute with the given tag and name from the header to the given
 * tiff directory.  If the attribute isn't present in the header, silently do
 * nothing.
 */
template<typename Tattr, typename Ttiff>
void addAttributeToTiff(ttag_t tag,
		const CqTexFileHeader& header, CqTiffDirHandle& dirHandle)
{
	const typename Tattr::type* headerVal = header.findPtr<Tattr>();
	if(headerVal)
	{
		try
		{
			dirHandle.setTiffTagValue<Ttiff>(tag, 
					attrTypeToTiff<typename Tattr::type,Ttiff>(*headerVal));
		}
		catch(XqInternal& e)
		{
			Aqsis::log() << e << "\n";
		}
	}
}

} // unnamed namespace


void CqTiffDirHandle::writeOptionalAttrs(const CqTexFileHeader& header)
{
	// Add various descriptive strings to the header if they exist
	addAttributeToTiff<Attr::Software,const char*>(TIFFTAG_SOFTWARE, header, *this);
	addAttributeToTiff<Attr::HostName,const char*>(TIFFTAG_HOSTCOMPUTER, header, *this);
	addAttributeToTiff<Attr::Description,const char*>(TIFFTAG_IMAGEDESCRIPTION, header, *this);
	addAttributeToTiff<Attr::DateTime,const char*>(TIFFTAG_DATETIME, header, *this);
	addAttributeToTiff<Attr::TextureFormat,const char*>(TIFFTAG_PIXAR_TEXTUREFORMAT, header, *this);

	/// \todo Consider the need for TIFFTAG_SMINSAMPLEVALUE and TIFFTAG_SMAXSAMPLEVALUE

	// Add some matrix attributes
	addAttributeToTiff<Attr::WorldToScreenMatrix,const float*>(
			TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, header, *this);
	addAttributeToTiff<Attr::WorldToCameraMatrix,const float*>(
			TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, header, *this);

	// Add cotan of the field of view.
	addAttributeToTiff<Attr::FieldOfViewCot,float>(
			TIFFTAG_PIXAR_FOVCOT, header, *this);

	// Set texture wrap mode string
	const SqWrapModes* wrapModes = header.findPtr<Attr::WrapModes>();
	if(wrapModes)
	{
		std::ostringstream oss;
		oss << wrapModes->sWrap << " " << wrapModes->tWrap;
		setTiffTagValue<const char*>(TIFFTAG_PIXAR_WRAPMODES, oss.str().c_str());
	}

	// Set size of display window if present
	const SqImageRegion* displayWindow = header.findPtr<Attr::DisplayWindow>();
	if(displayWindow)
	{
		setTiffTagValue<uint32>(TIFFTAG_PIXAR_IMAGEFULLWIDTH, displayWindow->width);
		setTiffTagValue<uint32>(TIFFTAG_PIXAR_IMAGEFULLLENGTH, displayWindow->height);
		setTiffTagValue<float>(TIFFTAG_XPOSITION, displayWindow->topLeftX);
		setTiffTagValue<float>(TIFFTAG_YPOSITION, displayWindow->topLeftY);
	}
}

void CqTiffDirHandle::fillHeaderRequiredAttrs(CqTexFileHeader& header) const
{
	// Fill header with general metadata which won't affect the details of the
	// pixel memory layout.
	header.setWidth(tiffTagValue<uint32>(TIFFTAG_IMAGEWIDTH));
	header.setHeight(tiffTagValue<uint32>(TIFFTAG_IMAGELENGTH));
	if(TIFFIsTiled(tiffPtr()))
	{
		header.set<Attr::TileInfo>( SqTileInfo(
					tiffTagValue<uint32>(TIFFTAG_TILEWIDTH),
					tiffTagValue<uint32>(TIFFTAG_TILELENGTH)) );
	}
	// Get the compression type.
	header.set<Attr::Compression>(
			tiffCompressionNameFromTag(tiffTagValue<uint16>(TIFFTAG_COMPRESSION)) );
	// Compute pixel aspect ratio
	TqFloat xRes = 0;
	TqFloat yRes = 0;
	if(TIFFGetField(tiffPtr(), TIFFTAG_XRESOLUTION, &xRes)
			&& TIFFGetField(tiffPtr(), TIFFTAG_YRESOLUTION, &yRes))
	{
		// yRes/xRes should be the correct quantity corresponding to the
		// pixelAspectRatio used in OpenEXR.
		header.set<Attr::PixelAspectRatio>(yRes/xRes);
	}
	else
	{
		header.set<Attr::PixelAspectRatio>(1.0f);
	}
}


namespace {

template<typename Tattr, typename Ttiff>
typename Tattr::type attrTypeFromTiff(const Ttiff& tiffAttr)
{
	return typename Tattr::type(tiffAttr);
}
// specialize for const char* -> EqTextureFormat
template<>
EqTextureFormat attrTypeFromTiff<Attr::TextureFormat, const char*>(
		const char* const& texFormatStr)
{
	return texFormatFromString(texFormatStr);
}

/// Extract an attribute from dirHandle and add it to header, if present.
template<typename Tattr, typename Ttiff>
void addAttributeToHeader(ttag_t tag, CqTexFileHeader& header,
		const CqTiffDirHandle& dirHandle)
{
	Ttiff temp;
	if(TIFFGetField(dirHandle.tiffPtr(), tag, &temp))
		header.set<Tattr>(attrTypeFromTiff<Tattr, Ttiff>(temp));
}

/// Add texture wrap modes to the header if they can be found in the TIFF.
void addWrapModesToHeader(CqTexFileHeader& header, const CqTiffDirHandle& dirHandle)
{
	char* wrapModesStr = 0;
	if(TIFFGetField(dirHandle.tiffPtr(), TIFFTAG_PIXAR_WRAPMODES, &wrapModesStr))
	{
		std::istringstream iss(wrapModesStr);
		SqWrapModes modes;
		iss >> modes.sWrap >> modes.tWrap;
		header.set<Attr::WrapModes>(modes);
	}
}

} // unnamed namespace


void CqTiffDirHandle::fillHeaderOptionalAttrs(CqTexFileHeader& header) const
{
	// Add various descriptive strings to the header if they exist
	addAttributeToHeader<Attr::Software,char*>(TIFFTAG_SOFTWARE, header, *this);
	addAttributeToHeader<Attr::HostName,char*>(TIFFTAG_HOSTCOMPUTER, header, *this);
	addAttributeToHeader<Attr::Description,char*>(TIFFTAG_IMAGEDESCRIPTION, header, *this);
	addAttributeToHeader<Attr::DateTime,char*>(TIFFTAG_DATETIME, header, *this);
	addAttributeToHeader<Attr::TextureFormat,const char*>(TIFFTAG_PIXAR_TEXTUREFORMAT, header, *this);

	// Add texturemap-specific stuff to the header if it exists.
	addWrapModesToHeader(header, *this);

	// Add some matrix attributes
	addAttributeToHeader<Attr::WorldToScreenMatrix,float*>(
			TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, header, *this);
	addAttributeToHeader<Attr::WorldToCameraMatrix,float*>(
			TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, header, *this);
	// Add cotan of the field of view.
	addAttributeToHeader<Attr::FieldOfViewCot,float>(
			TIFFTAG_PIXAR_FOVCOT, header, *this);

	// Retrieve tags relevant to the display window
	// The origin of the image is apparently given in resolution units, but
	// here we want to interpret it as the number of pixels from the top left
	// of the image, hence the lfloor.
	uint32 fullWidth = header.width();
	uint32 fullHeight = header.height();
	float xPos = 0;
	float yPos = 0;
	if( TIFFGetField(tiffPtr(), TIFFTAG_PIXAR_IMAGEFULLWIDTH, &fullWidth)
		| TIFFGetField(tiffPtr(), TIFFTAG_PIXAR_IMAGEFULLLENGTH, &fullHeight)
		| TIFFGetField(tiffPtr(), TIFFTAG_XPOSITION, &xPos)
		| TIFFGetField(tiffPtr(), TIFFTAG_YPOSITION, &yPos)
		// bitwise OR used since we don't want shortcut evaluation
		)
	{
		header.set<Attr::DisplayWindow>( SqImageRegion(fullWidth, fullHeight,
					lfloor(xPos), lfloor(yPos)) );
	}
}

void CqTiffDirHandle::fillHeaderPixelLayout(CqTexFileHeader& header) const
{
	header.set<Attr::TiffUseGenericRGBA>(false);
	// Deal with fields which determine the pixel layout.
	try
	{
		// Deduce image channel information.
		guessChannels(header.channelList());
		// Check that channels are interlaced, otherwise we'll be confused.
		TqInt planarConfig = tiffTagValue<uint16>(TIFFTAG_PLANARCONFIG,
				PLANARCONFIG_CONTIG);
		if(planarConfig != PLANARCONFIG_CONTIG)
			AQSIS_THROW_XQERROR(XqUnknownTiffFormat, EqE_BadFile,
				"non-interlaced channels detected");
		// Check that the origin is at the topleft of the image.
		TqInt orientation = tiffTagValue<uint16>(TIFFTAG_ORIENTATION,
				ORIENTATION_TOPLEFT);
		if(orientation != ORIENTATION_TOPLEFT)
		{
			Aqsis::log() << warning
				<< "TIFF orientation for file \"" << m_fileHandle->fileName()
				<< "\" is not top-left.  This may result in unexpected results\n";
			/// \todo Decide whether to use generic TIFF loading facilities for this.
			//AQSIS_THROW_XQERROR(XqUnknownTiffFormat, EqE_Limit,
			//	"orientation isn't top-left");
		}
	}
	catch(XqUnknownTiffFormat& e)
	{
		// The format is something strange that we don't know how to handle
		// directly... Use the generic RGBA handling built into libtiff...
		Aqsis::log() << warning
			<< "Cannot handle desired tiff format efficiently: \"" << e.what() << "\".\n"
			"Switching to generic RGBA handling - this may result in some loss of precision\n";
		char errBuf[1024];
		if(!TIFFRGBAImageOK(tiffPtr(), errBuf))
		{
			AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
				"Cannot use generic RGBA tiff interface for file \"" << m_fileHandle->fileName()
				<< "\".  " << "Libtiff says: " << errBuf);
		}
		EqChannelType chanType = Channel_Unsigned8;
		CqChannelList& channelList = header.channelList();
		channelList.clear();
		channelList.addChannel( SqChannelInfo("r", chanType) );
		channelList.addChannel( SqChannelInfo("g", chanType) );
		channelList.addChannel( SqChannelInfo("b", chanType) );
		channelList.addChannel( SqChannelInfo("a", chanType) );
		header.set<Attr::TiffUseGenericRGBA>(true);
	}
}

EqChannelType CqTiffDirHandle::guessChannelType() const
{
	TqInt bitsPerSample = tiffTagValue<uint16>(TIFFTAG_BITSPERSAMPLE);
	TqInt sampleFormat = tiffTagValue<uint16>(TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	switch(bitsPerSample)
	{
		case 32:
			switch(sampleFormat)
			{
				case SAMPLEFORMAT_IEEEFP:
					return Channel_Float32;
				case SAMPLEFORMAT_INT:
					return Channel_Signed32;
				case SAMPLEFORMAT_UINT:
					return Channel_Unsigned32;
				default:
					Aqsis::log() << warning
						<< "Unknown tiff format for 32 bits per sample: "
						"TIFFTAG_SAMPLEFORMAT = " << sampleFormat
						<< ".  Assuming unsigned int.\n";
					return Channel_Unsigned32;
			}
			break;
		case 16:
			switch(sampleFormat)
			{
				case SAMPLEFORMAT_INT:
					return Channel_Signed16;
				case SAMPLEFORMAT_UINT:
					return Channel_Unsigned16;
				default:
					Aqsis::log() << warning
						<< "Unknown tiff format for 16 bits per sample: "
						"TIFFTAG_SAMPLEFORMAT = " << sampleFormat
						<< ".  Assuming unsigned int.\n";
					return Channel_Unsigned16;
			}
			break;
		case 8:
			switch(sampleFormat)
			{
				case SAMPLEFORMAT_INT:
					return Channel_Signed8;
				case SAMPLEFORMAT_UINT:
					return Channel_Unsigned8;
				default:
					Aqsis::log() << warning
						<< "Unknown tiff format for 8 bits per sample: "
						"TIFFTAG_SAMPLEFORMAT = " << sampleFormat
						<< ".  Assuming unsigned int.\n";
					return Channel_Unsigned8;
			}
			break;
		default:
			// We give up and use the generic 8bpp tiff loading.
			return Channel_TypeUnknown;
	}
}

void CqTiffDirHandle::guessChannels(CqChannelList& channelList) const
{
	channelList.clear();
	EqChannelType chanType = guessChannelType();
	if(chanType == Channel_TypeUnknown)
		AQSIS_THROW_XQERROR(XqUnknownTiffFormat, EqE_Limit,
			"Cannot determine channel type");
	else
	{
		// Determine the channel type held in the tiff
		switch(tiffTagValue<uint16>(TIFFTAG_PHOTOMETRIC))
		{
			case PHOTOMETRIC_MINISBLACK:
				{
					TqInt samplesPerPixel = tiffTagValue<uint16>(TIFFTAG_SAMPLESPERPIXEL, 1);
					// We have an intensity (y) channel only.
					channelList.addChannel(SqChannelInfo("y", chanType));
					if(samplesPerPixel == 2)
					{
						// For two channels, assume the second is alpha
						channelList.addChannel(SqChannelInfo("a", chanType));
					}
					else
					{
						// Otherwise we're a bit confused; just add the
						// additional channels as unnamed.
						channelList.addUnnamedChannels(chanType, samplesPerPixel-1);
					}
				}
				break;
			case PHOTOMETRIC_RGB:
				{
					TqInt samplesPerPixel = tiffTagValue<uint16>(TIFFTAG_SAMPLESPERPIXEL);
					if(samplesPerPixel < 3)
						channelList.addUnnamedChannels(chanType, samplesPerPixel);
					else
					{
						// add RGB channels
						channelList.addChannel(SqChannelInfo("r", chanType));
						channelList.addChannel(SqChannelInfo("g", chanType));
						channelList.addChannel(SqChannelInfo("b", chanType));
						/// \todo Investigate what to do about TIFFTAG_EXTRASAMPLES
						if(samplesPerPixel == 4)
						{
							// add alpha channel
							channelList.addChannel(SqChannelInfo("a", chanType));
						}
						else if(samplesPerPixel == 6)
						{
							// add RGB alpha channels
							channelList.addChannel(SqChannelInfo("ra", chanType));
							channelList.addChannel(SqChannelInfo("ga", chanType));
							channelList.addChannel(SqChannelInfo("ba", chanType));
						}
						else
						{
							// Or not sure what to do here... add some unnamed
							// channels?
							channelList.addUnnamedChannels(chanType, samplesPerPixel-3);
						}
					}
				}
				break;
			/// \todo Should also handle the following?
			//case PHOTOMETRIC_LOGL:
			//case PHOTOMETRIC_LOGLUV:
			default:
				AQSIS_THROW_XQERROR(XqUnknownTiffFormat, EqE_Limit,
					"Unknown photometric type");
		}
	}
}


//------------------------------------------------------------------------------
// CqTiffFileHandle
//------------------------------------------------------------------------------

namespace {

void safeTiffClose(TIFF* tif)
{
	if(tif)
		TIFFClose(tif);
}

} // unnamed namespace

CqTiffFileHandle::CqTiffFileHandle(const boostfs::path& fileName, const char* openMode)
	: m_fileName(fileName),
	m_tiffPtr(TIFFOpen(native(fileName).c_str(), openMode), safeTiffClose),
	m_isInputFile(openMode[0] == 'r'),
	m_currDir(0)
{
	if(!m_tiffPtr)
		AQSIS_THROW_XQERROR(XqInvalidFile, EqE_NoFile,
			"Could not open tiff file \"" << fileName << "\"");
}

CqTiffFileHandle::CqTiffFileHandle(std::istream& inputStream)
	: m_tiffPtr(TIFFStreamOpen("stream", &inputStream), safeTiffClose),
	m_isInputFile(true),
	m_currDir(0)
{
	if(!m_tiffPtr)
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_NoFile,
			"Could not use input stream for tiff");
	}
}

CqTiffFileHandle::CqTiffFileHandle(std::ostream& outputStream)
	: m_tiffPtr(TIFFStreamOpen("stream", &outputStream), safeTiffClose),
	m_isInputFile(false),
	m_currDir(0)
{
	if(!m_tiffPtr)
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_NoFile,
			"Could not use output stream for tiff");
	}
}


void CqTiffFileHandle::writeDirectory()
{
	assert(!m_isInputFile);
	if(!TIFFWriteDirectory(m_tiffPtr.get()))
		AQSIS_THROW_XQERROR(XqInternal, EqE_BadFile,
			"Could not write tiff subimage to file");
	++m_currDir;
}


tdir_t CqTiffFileHandle::numDirectories()
{
	return TIFFNumberOfDirectories(m_tiffPtr.get());
}


void CqTiffFileHandle::setDirectory(tdir_t dirIdx)
{
	if(m_isInputFile && dirIdx != m_currDir)
	{
		if(!TIFFSetDirectory(m_tiffPtr.get(), dirIdx))
		{
			AQSIS_THROW_XQERROR(XqInternal, EqE_Bug,
					"Requested tiff directory " << dirIdx << " out of range for file \""
					<< m_fileName << "\"");
		}
		m_currDir = dirIdx;
	}
}

//------------------------------------------------------------------------------
// Free functions

void stridedCopy(TqUint8* dest, TqInt destStride, const TqUint8* src, TqInt srcStride,
		TqInt numElems, TqInt elemSize)
{
	for(TqInt i = 0; i < numElems; ++i)
	{
		std::memcpy(dest, src, elemSize);
		dest += destStride;
		src += srcStride;
	}
}

//------------------------------------------------------------------------------
} // namespace Aqsis
