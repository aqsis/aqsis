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
 * \brief Declare input interface specification which should be implemented by
 * all classes wrapping scanline-oriented texture files.
 *
 * \author Chris Foster
 */

#ifndef ITEXINPUTFILE_H_INCLUDED
#define ITEXINPUTFILE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <aqsis/util/file.h>
#include <aqsis/tex/io/imagefiletype.h>
#include <aqsis/tex/io/texfileheader.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief An input interface for texture files.
 *
 * This class provides simple scanline-based texture I/O into buffers modeling
 * a 2D image buffer concept.  Pixel data may be read as blocks of scanlines
 * into the 2D buffer class.
 *
 * Image metadata is accessed through an extensible image attributes mechanism.
 * All attributes are contained in a CqTexFileHeader object.
 *
 * A file-type-aware open() function is provided for type-agnostic loading of
 * image files.
 */
class AQSIS_TEX_SHARE IqTexInputFile
{
	public:
		virtual ~IqTexInputFile() {};

		//------------------------------------------------------------
		/// \name Metadata access
		//@{
		/// get the file name
		virtual boostfs::path fileName() const = 0;

		/// get a string representing the file type
		virtual EqImageFileType fileType() const = 0;

		/// Get the file header data
		virtual const CqTexFileHeader& header() const = 0;
		//@}

		/** \brief Read in a region of scanlines
		 *
		 * Array2DType is a type modelling a simple resizeable 2D array
		 * interface.  It should provide the following methods:
		 *   - void resize(TqInt width, TqInt height, const CqChannelList& channels)
		 *     Resizes the buffer.  (width, height) is the new dimensions for
		 *     the buffer.  channels describes the new desired channel
		 *     structure for the buffer.  If the buffer cannot handle the
		 *     given channel structure it should throw.
		 *   - TqUint8* rawData()
		 *     Gets a raw pointer to the data.
		 *
		 * \param buffer - buffer to read scanlines into
		 * \param startLine - scanline to start reading the data from (top == 0)
		 * \param numScanlines - number of scanlines to read.  If <= 0, read
		 *                       to the end of the image.
		 */
		template<typename Array2DType>
		void readPixels(Array2DType& buffer, TqInt startLine = 0,
				TqInt numScanlines = -1) const;

		/** \brief Low-level readPixels() function to be overridden by child classes
		 *
		 * The implementation of readPixels simply validates the input
		 * parameters against the image dimensions as reported by header(),
		 * sets up the buffer, and calls readPixelsImpl().
		 *
		 * Implementations of readPixelsImpl() can assume that startLine and
		 * numScanlines specify a valid range.
		 *
		 * \note Don't use this low-level interface unless you have a good
		 * reason.  The higher level readPixels() function is a lot safer and
		 * more user friendly.
		 *
		 * \param buffer - Raw buffer to read the data into.  Must be large
		 *                 enough to read width*numScanlines pixels with the
		 *                 same channel structure specified in the image
		 *                 header.
		 * \param startLine - scanline to start reading the data from (top == 0)
		 * \param numScanlines - number of scanlines to read, must be positive!
		 */
		virtual void readPixelsImpl(TqUint8* buffer, TqInt startLine,
				TqInt numScanlines) const = 0;

		/** \brief Open an input image file in any format
		 *
		 * Uses magic numbers to determine the file format of the file given by
		 * fileName.  If the format is unknown or the file cannot be opened for
		 * some other reason, throw an exception.
		 *
		 * \param fileName - file to open.  Can be in any of the formats
		 * understood by aqsistex.
		 * \return The newly opened input file
		 */
		static boost::shared_ptr<IqTexInputFile> open(const boostfs::path& fileName);
};


//------------------------------------------------------------------------------
/** \brief Interface for image files supporting multiple sub-images.
 *
 * Some formats (eg, TIFF) allow unrelated content in the various images of a
 * multi-image file.  This class provides support for such files, in addition
 * to the operations supported by the IqTexInputFile interface.
 */
class AQSIS_TEX_SHARE IqMultiTexInputFile : public IqTexInputFile
{
	public:
		//--------------------------------------------------
		/// \name Functions for accessing multiple sub-images.
		//@{
		/** Set the image index in a multi-image file.
		 *
		 * In general, this function may be expected to modify the image header
		 * to reflect the metadata for the new image level.
		 *
		 * \param newIndex - new index in the multi-image file.
		 */
		virtual void setImageIndex(TqInt newIndex) = 0;
		/** Get the image index for a multi-image file.
		 *
		 * \return the current image index
		 */
		virtual TqInt imageIndex() const = 0;
		/** Get the number of images in the multi-image file.
		 *
		 * \return The number of images
		 */
		virtual TqInt numSubImages() const = 0;
		//@}

		/** \brief Open an input image file in any format supporting multiple sub-images.
		 *
		 * Uses magic numbers to determine the file format of the file given by
		 * fileName.  If the format is unknown or the file cannot be opened for
		 * some other reason, throw an exception.
		 *
		 * \param fileName - file to open.  Must be a format capable of holding
		 * multiple sub-images.
		 * \return The newly opened input file
		 */
		static boost::shared_ptr<IqMultiTexInputFile> open(const boostfs::path& fileName);
};


//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

template<typename Array2DType>
void IqTexInputFile::readPixels(Array2DType& buffer, TqInt startLine,
		TqInt numScanlines) const
{
	TqInt imageHeight = header().height();
	// if numScanlines is negative, read until the last line
	if(numScanlines <= 0)
		numScanlines = imageHeight - startLine;
	// check that startLine is in the image range & that the ending line is
	// reasonable.
	if(startLine < 0 || startLine >= imageHeight
			|| startLine + numScanlines > imageHeight)
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_Bug, "Attempt to read scanlines "
				<< startLine << " to " << startLine + numScanlines - 1
				<< " outside image boundaries for file \"" << fileName() << "\".");
	}
	// Resize the buffer to deal with the new data
	buffer.resize(header().width(), numScanlines, header().channelList());
	readPixelsImpl(buffer.rawData(), startLine, numScanlines);
}


} // namespace Aqsis

#endif // ITEXINPUTFILE_H_INCLUDED
