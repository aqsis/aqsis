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

#ifndef TIFFFILE_H_INCLUDED
#define TIFFFILE_H_INCLUDED

#include <string>

#include <boost/shared_array.hpp>
#include <tiffio.h>

#include "aqsis.h"
#include "exception.h"

namespace Aqsis
{

/** \brief Find a file in the current Ri Path.
 *
 * \param fileName - file name to search for
 * \param riPath - name of the Ri path to look in.
 *
 * \return the fully qualified path to the file.
 */
std::string findFileInRiPath(const std::string& fileName, const std::string& riPath);

//------------------------------------------------------------------------------
/** \brief Class wrapper around the libtiff library functions for reading and
 * writing multi-directory tiled tiff files.
 *
 * In principle this class would extend from CqFile.  However, CqFile isn't a
 * very good base class because it's targeted toward file IO based on the
 * standard iostreams.  Here we wrap the C-like file IO functions from libtiff
 * instead.
 *
 * CqTiffInputFile provides a uniform interface for dealing with both tiled and
 * strip-oriented TIFF files - here we just call both types tiles.
 */
class CqTiffInputFile
{
	public:
		/** \brief Construct a tiff input file.  The resulting file object is
		 * guaranteed to be an valid and accessable TIFF file.
		 *
		 * \throw XqIoError
		 * \throw XqTiffError
		 *
		 * \param fileName
		 * \param searchPath
		 * \param directory
		 */
		CqTiffInputFile(const std::string& fileName, const std::string& riPath, const TqUint directory = 0);
		/** \brief Copy constructor
		 */
		CqTiffInputFile(CqTiffInputFile& toBeCopied);
		/** \brief Virtual destructor
		 */
		virtual ~CqTiffInputFile();

		/** \brief Get the width of tiles held in this tiff
		 * \return tile width
		 */
		inline TqUint tileWidth() const;
		/** \brief Get the height of tiles held in this tiff
		 * \return tile height
		 */
		inline TqUint tileHeight() const;

		/** \brief Read a tile or strip from the tiff file.
		 *
		 * \throw XqTiffError if the requested tile is outside the bounds of the image
		 *
		 * \param x - tile column index (counting from top left, starting with 0)
		 * \param y - tile row index (counting from top left, starting with 0)
		 * \return a tile containing the desired data.
		 */
		template<typename T>
		boost::intrusive_ptr<CqTextureTile<T> > readTile(const TqUint x, const TqUint y)

		/** \brief Check that this tiff file represents a mipmap.
		 *
		 * A mipmap consists of successive downscalings of the original file by
		 * a factor of two.  The smallest mipmap level consists of a single
		 * pixel (this is the convention taken in the OpenExr spec.)
		 *
		 * This function checks only that the tiff has the correct number of
		 * directories (each mipmap level corresponds to a tiff directory), and
		 * that the directories have the correct sizes.
		 *
		 * \return true if the file represents a mipmap.
		 */
		bool isMipMap();

		/** \brief Create a copy, of this Tiff file, but with a different directory.
		 *
		 * \todo: Some performance testing - is it better to clone, or keep the
		 * same underlying file handle etc, and simply use TIFFSetDirectory a
		 * lot of times.  Also, have a think about the number of file handles
		 * it would require if each mipmap level gets its own...
		 *
		 * \return a copy of this tiff file, set to the given directory.
		 */
		CqTiffInputFile& cloneWithNewDirectory(const TqUint newDir);

		/** \brief assignment operator
		 *
		 * \param rhs - tiff file to assign to this one
		 * \return reference to this.
		 */
		CqTiffInputFile& operator=(const CqTiffInputFile& rhs);

	protected:
		//----------------------------------------
		// Protected member functions
		/** \brief Get the value of a tiff tag for the current image.
		 *
		 * Note that unfortunately this isn't type-safe: you *must* specify the
		 * correct type, T for the TIFF tag desired, otherwise you'll get
		 * strange results, or a crash!  This is unfortunately due to the
		 * nature of the underlying library and isn't easy to avoid.
		 *
		 * \throw XqTiffError if the tag is not defined.
		 *
		 * \param tag - the tiff tag to obtain.
		 *
		 * \return the tag value
		 */
		template<typename T>
		T tiffTagValue(const uint32 tag) const;
		/** \brief Get the value of a tiff tag for the current image.
		 *
		 * Note that unfortunately this isn't type-safe: you *must* specify the
		 * correct type, T for the TIFF tag desired, otherwise you'll get
		 * strange results, or a crash!
		 *
		 * Unlike the one-parameter version of tiffTagValue, this version does
		 * not throw an exception when the tag is not found.  Instead it
		 * quitely returns the supplied default value.
		 *
		 * \param tag - the tiff tag to obtain.
		 * \param defaultVal - value to return if the tag is not present in the file.
		 *
		 * \return the tag value
		 */
		template<typename T>
		T tiffTagValue(const uint32 tag, const T defaultVal) const;
		/** \brief Ensure that a given tiff tag has a particular value.
		 *
		 * \param tag - the tiff tag to query
		 * \param desiredValue - desired value for the tag.
		 * \param tagNotFoundVal - return this when the tag is not found
		 * \return true if the tag has the desired value, false if not.
		 */
		template<typename T>
		bool checkTagValue(const uint32 tag, const T desiredValue,
				const bool tagNotFoundVal = true) const;
	private:
		//----------------------------------------
		/** \brief Hold data describing the current directory
		 */
		struct SqDirectoryData
		{
			uint32 index;			///< directory index
			bool isTiled;			///< true if tile-based, false if strip-based
			// The following are required tiff fields
			uint32 imageWidth;		///< full image width
			uint32 imageHeight;		///< full image height
			uint32 bitsPerSample;	///< number of bits per channel sample
			uint32 samplesPerPixel;	///< number of channels per pixel
			uint32 tileWidth;		///< tile or strip width
			uint32 tileHeight;		///< tile or strip height
			uint16 photometricInterp;	///< photometric interpretation for samples
			// the next two are computed
			uint32 numTilesX;		///< number of columns of tiles
			uint32 numTilesY;		///< number of rows of tiles
			// The rest are values which may or may not be present.  We try to
			// choose sensible defaults for these.
			uint16 sampleFormat;	///< the format of the samples - uint/int/float etc
			uint16 planarConfig;	///< are different channels stored packed together?
			uint16 orientation;		///< position of the (0,0) index to the tiles

			/// Default constructor
			SqDirectoryData();
			/// Constructor taking a CqTiffInputFile to grab the directory data from
			SqDirectoryData(CqTiffInputFile& file, TqUint index);
			/// Use compiler generated assignment operator and copy constructor.
		};

		//----------------------------------------
		// Private member functions
		/** \brief Set the directory for this tiff file.
		 *
		 * \throw XqTiffError if the required directory could not be read.
		 *
		 * \param directory - directory to connect to.
		 */
		void setDirectory(const TqUint directory);
		/** \brief Set data to all ones, and a data read error to the log.
		 *
		 * \param data - array which wasn't written because of a tiff read error.
		 * \param dataSize - length of array in bytes.
		 */
		template<typename T>
		static void handleDataReadError(boost::shared_array<T> data, const tsize_t dataSize) const;
		/** \brief Allocate memory with _TIFFmalloc and encapsulate the memory
		 * in a boost::shared_array.
		 *
		 * \throw XqMemoryError if the allocation fails
		 *
		 * \param size - number of bytes to allocate.
		 * \return 'size' bytes of memory allocated with _TIFFmalloc
		 */
		template<typename T>
		static boost::shared_array<T> tiffMalloc(const tsize_t size) const;

		//----------------------------------------
		// Member data
		std::string m_fileName;			///< file name
		std::string m_fullFileName;		///< file name including full path
		boost::shared_ptr<TIFF> m_tiffPtr;	///< pointer to the underlying tiff
		//boost::shared_ptr<TIFFRGBAImage> m_rgbaImage; ///< libtiff RGBA image for decoding unusual formats

		SqDirectoryData m_currDir;		///< data describing current directory.
};


//------------------------------------------------------------------------------
/** \brief Class for reporting errors encountered on reading TIFF files.
 */
class XqTiffError : public XqException
{
	public:
		inline XqTiffError(const char* reason = 0);
};




//==============================================================================
// Implementation of inline functions and templates
//==============================================================================
//------------------------------------------------------------------------------
// CqTiffInputFile
//------------------------------------------------------------------------------
// inlines
inline TqInt CqTiffInputFile::tileWidth() const
{
	return m_currDir.tileWidth;
}

inline TqInt CqTiffInputFile::tileHeight() const
{
	return m_currDir.tileHeight;
}

//------------------------------------------------------------------------------
// templates
//------------------------------------------------------------------------------
template<typename T>
boost::intrusive_ptr<CqTextureTile<T> > CqTiffInputFile::readTile(const TqUint x, const TqUint y)
{
	if(x >= m_currDir.numTilesX)
		throw XqTiffError("Column index out of bounds");
	if(y >= m_currDir.numTilesY)
		throw XqTiffError("Row index out of bounds");

	// Check if the data size, T, matches up with the size of the internal
	// representation.  Ideally we'd check if the types are equal, but I don't
	// know how to do that.
	//
	// We could do data format conversion here instead of throwing an error I suppose...
	if(sizeof(T)*8 != m_currDir.bitsPerSample)
		throw XqTiffError("miss-match between requested vs actual bits per sample");

	// The tile size is smaller if it falls off the edge of the image - take
	// account of this.
	TqUint tileWidth = m_currDir.tileWidth;
	TqUint tileHeight = m_currDir.tileHeight;
	if((x+1)*tileWidth > m_currDir.imageWidth)
		tileWidth = m_currDir.imageWidth - x*tileWidth;
	if((y+1)*tileHeight > m_currDir.imageHeight)
		tileHeight = m_currDir.imageHeight - y*tileHeight;

	// Read in the tile data.
	boost::shared_array<T> tileData(0);
	tsize_t dataSize = 0;
	if(m_currDir.isTiled)
	{
		// Tiff is stored as tiles
		dataSize = TIFFTileSize(m_tiffPtr.get());
		tileData = tiffMalloc(dataSize);
		if(TIFFReadTile(m_tiffPtr.get(), tileData, x*m_currDir.tileWidth,
					y*m_currDir.tileHeight, 0, 0) == -1)
			handleDataReadError(tileData, dataSize);
	}
	else
	{
		// Tiff is stored in strips - treat each strip as a wide tile.
		dataSize = TIFFStripSize(m_tiffPtr.get());
		tileData = tiffMalloc(dataSize);
		if(TIFFReadEncodedStrip(m_tiffPtr.get(), TIFFComputeStrip(m_tiffPtr.get(),
						y*m_currDir.tileHeight, 0), tileData, -1) == -1)
			handleDataReadError(tileData, dataSize);
	}

	return boost::intrusive_ptr<CqTextureTile<T> >  tile(
			new CqTextureTile<T>(
				reinterpret_cast<T*>(tileData), tileWidth,
				x*tileWidth, y*tileHeight, m_samplesPerPixel) );
}


//------------------------------------------------------------------------------
template<typename T>
void CqTiffInputFile::handleDataReadError(boost::shared_array<T> data, const tsize_t dataSize) const
{
	// gracefully degrade when there's an error reading data from a tiff file.
	Aqsis::log() << error << "Error reading data from tiff file \""
		<< m_fileName << "\".  Using blank (white) tile\n";
	_TIFFmemset(data.get(), 0xFF, dataSize);
}


//------------------------------------------------------------------------------
template<typename T>
boost::shared_array<T> CqTiffInputFile::tiffMalloc(const tsize_t size) const
{
	boost::shared_array<T> buf(reinterpret_cast<T>(_TIFFmalloc(size)));
	if(!buf)
		throw XqMemoryError("Could not allocate memory for tiff tile");
	return buf;
}

//------------------------------------------------------------------------------
template<typename T>
T CqTiffInputFile::tiffTagValue(const uint32 tag) const
{
	T temp = 0;
	if(TIFFGetField(m_tiffPtr.get(), tag, &temp))
		return temp;
	else
		throw XqTiffError(boost::str(boost::format("Could not get tag with value %d") % tag));
}

//------------------------------------------------------------------------------
template<typename T>
T CqTiffInputFile::tiffTagValue(const uint32 tag, const T defaultVal) const
{
	T temp = 0;
	if(TIFFGetField(m_tiffPtr.get(), tag, &temp))
		return temp;
	else
		return defaultVal;
}


//------------------------------------------------------------------------------
// XqTiffError
//------------------------------------------------------------------------------
// inlines
inline XqTiffError::XqTiffError(const char* reason)
	: XqException(reason)
{ }



//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TIFFFILE_H_INCLUDED
