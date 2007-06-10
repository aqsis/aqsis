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
 * \brief A C++ wrapper around tiff files for the functions of interest in aqsis.
 *
 * \author Chris Foster
 */

#include <boost/format.hpp>

#include "tifffile.h"
#include "rifile.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// stand-alone functions
//------------------------------------------------------------------------------

std::string findFileInRiPath(const std::string& fileName, const std::string& riPath)
{
	std::string fullFileName;
	CqRiFile riFile(fileName.c_str(), searchPath.c_str());
	if(riFile.IsValid())
	{
		fullFileName = riFile.strRealName();
		riFile.Close();
	}
	else
		throw XqIoError(boost::str(boost::format("Could not fild file '%s'") % fileName));
	return fullFileName;
}


//------------------------------------------------------------------------------
// CqTiffInputFile
//------------------------------------------------------------------------------
// constructor
CqTiffInputFile::CqTiffInputFile(const std::string& fileName, const std::string& riPath, TqUint directory)
	: m_fileName(fileName),
	m_fullFileName(findFileInRiPath(fileName, riPath)),
	m_tiffPtr(0),
	m_currDir()
{
	m_tiffPtr = boost::shared_ptr<TIFF>(TIFFOpen(m_fullFileName.c_str(), "r"), TIFFClose);
	if(!m_tiffPtr)
		throw XqIoError( boost::str(boost::format("Could not open tiff file '%s'") % fileName) );
	setDirectory(directory);
}

//------------------------------------------------------------------------------
// copy constructor
CqTiffInputFile::CqTiffInputFile(CqTiffInputFile& toBeCopied)
	: m_fileName(toBeCopied.m_fileName),
	m_fullFileName(toBeCopied.m_fullFileName),
	m_tiffPtr(0),
	m_currDir(toBeCopied.m_currDir)
{
	m_tiffPtr = boost::shared_ptr<TIFF>(TIFFOpen(m_fullFileName.c_str(), "r"), TIFFClose);
}

//------------------------------------------------------------------------------
// destructor
CqTiffInputFile::~CqTiffInputFile()
{ }

//------------------------------------------------------------------------------
bool CqTiffInputFile::isMipMap()
{
	// iterate through levels, checking that each is the correct size.
	/// \todo implementation.
	return false;
}

//------------------------------------------------------------------------------
CqTiffInputFile& CqTiffInputFile::operator=(const CqTiffInputFile& rhs)
{
	m_fileName = rhs.fileName;
	m_fullFileName = rhs.fullFileName;
	m_tiffPtr = boost::shared_ptr<TIFF>(TIFFOpen(m_fullFileName.c_str(), "r"), TIFFClose);
	m_currDir = rhs.m_currDir;
}

//------------------------------------------------------------------------------
void CqTiffInputFile::setDirectory(const TqUint directory)
{
	// Check that the tiff directory exists, and we can open it.
	if(!TIFFSetDirectory(m_tiffPtr.get(), directory))
		throw XqTiffError( boost::format("Directory number %d not found in file '%s'") % directory % m_fileName);

	SqDirectoryData newDirectory = SqDirectoryData(*this, directory);

	// Check that the tiff file is of a type which we know how to handle
	// easily.  If it's not, then we should use the generic RGBA handling of
	// libtiff to read the data (probably slower; doesn't preserve precision (?))
	if( !(
		// require either RGB, or grayscale photometric interpretation.
		( newDirectory.photometricInterp == PHOTOMETRIC_RGB
			|| newDirectory.photometricInterp == PHOTOMETRIC_MINISBLACK )
		// require directly addressable data samples
		&& (newDirectory.bitsPerSample % 8) == 0
		// require (0,0) coord to be in the top left
		&& newDirectory.orientation == ORIENTATION_TOPLEFT
		// require that colour values are stored packed together, not seperately.
		&& newDirectory.planarConfig == PLANARCONFIG_CONTIG
		// require that the sample type is either uint, int, or floating point.
		&& (newDirectory.sampleFormat == SAMPLEFORMAT_UINT
			|| newDirectory.sampleFormat == SAMPLEFORMAT_INT
			|| newDirectory.sampleFormat == SAMPLEFORMAT_IEEEFP)
		) )
	{
		// for the moment, just throw an error...
		throw XqTiffError("Unrecognised tiff format");
	}
	m_currDir = newDirectory;
}


/* Not needed?
TqUint CqTiffInputFile::countDirectories() const
{
	TIFFSetDirectory(m_tiffPtr.get(), 0);
	TqUint numDirs = 0;
	while(TIFFReadDirectory(m_tiffPtr.get()))
	{
		numDirs++;
	}
	return numDirs;
}
*/


//------------------------------------------------------------------------------
// CqTiffInputFile::SqDirectoryData
//------------------------------------------------------------------------------
// Constructors
CqTiffInputFile::SqDirectoryData::SqDirectoryData()
	: index(0),
	isTiled(0),
	imageWidth(0),
	imageHeight(0),
	bitsPerSample(0),
	samplesPerPixel(0),
	tileWidth(0),
	tileHeight(0),
	numTilesX(0),
	numTilesY(0),
	photometricInterp(0)
	sampleFormat(0)
	planarConfig(0)
	orientation(0)
{ }

CqTiffInputFile::SqDirectoryData::SqDirectoryData(CqTiffInputFile& file, TqUint index)
	: index(index),
	isTiled(0),
	imageWidth(0),
	imageHeight(0),
	bitsPerSample(0),
	samplesPerPixel(0),
	tileWidth(0),
	tileHeight(0),
	numTilesX(0),
	numTilesY(0),
	photometricInterp(0)
	sampleFormat(0)
	planarConfig(0)
	orientation(0)
{
	isTiled = static_cast<bool>(TIFFIsTiled(file.m_tiffPtr.get()));
	// get required tags
	imageWidth = file.tiffTagValue<uint32>(TIFFTAG_IMAGEWIDTH);
	imageHeight = file.tiffTagValue<uint32>(TIFFTAG_IMAGELENGTH);
	bitsPerSample = file.tiffTagValue<uint16>(TIFFTAG_BITSPERSAMPLE);
	samplesPerPixel = file.tiffTagValue<uint16>(TIFFTAG_SAMPLESPERPIXEL);
	if(isTiled)
	{
		tileWidth = file.tiffTagValue<uint32>(TIFFTAG_TILEWIDTH);
		tileHeight = file.tiffTagValue<uint32>(TIFFTAG_TILELENGTH);
	}
	else
	{
		// Treat strips as a variety of tile.
		tileWidth = imageWidth;
		tileHeight = file.tiffTagValue<uint32>(TIFFTAG_ROWSPERSTRIP);
	}
	photometricInterp = file.tiffTagValue<uint16>(TIFFTAG_PHOTOMETRIC);
	// compute number of tiles in x and y directions
	numTilesX = static_cast<TqUint>(ceil(static_cast<TqFloat>(imageWidth)/tileWidth));
	numTilesY = static_cast<TqUint>(ceil(static_cast<TqFloat>(imageHeight)/tileHeight));
	// get tags with sensible default values
	sampleFormat = file.tiffTagValue<uint16>(TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	planarConfig = file.tiffTagValue<uint16>(TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	orientation = file.tiffTagValue<uint16>(TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
}

//------------------------------------------------------------------------------
} // namespace Aqsis
