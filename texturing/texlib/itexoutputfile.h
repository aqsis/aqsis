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
 * \brief A scanline-based output interface for texture files.
 *
 * \author Chris Foster
 */

#ifndef ITEXOUTPUTFILE_H_INCLUDED
#define ITEXOUTPUTFILE_H_INCLUDED

#include "aqsis.h"

#include <string>

#include "texfileheader.h"
#include "exception.h"
#include "smartptr.h"
#include "mixedimagebuffer.h"

namespace Aqsis {

//------------------------------------------------------------------------------
class AQSISTEX_SHARE IqTexOutputFile
{
	public:
		virtual ~IqTexOutputFile() {};

		/// get the file name
		virtual const char* fileName() const = 0;

		/// get a string representing the file type
		virtual const char* fileType() = 0;

		/// Get the file header data
		virtual const CqTexFileHeader& header() const = 0;

		/** Get the index for the current line
		 *
		 * The "current line" is equal to number of scanlines written so far + 1
		 * That is, the current line is the next line which will be written to
		 * in a subsequent call of writePixels()
		 *
		 * \return the current scan line
		 */
		virtual TqInt currentLine() const = 0;

		/** \brief Write a region of scanlines
		 *
		 * Array2DType is a type modelling a 2D array interface.  It should
		 * provide the following methods:
		 *
		 *   - TqUchar* Array2dType::rawData() returns a pointer to the raw
		 *     data.  The raw data is assumed at this stage to be contiguous -
		 *     (ie, not a nontrivial slice).
		 *
		 *   - Array2DType::channelList() returns a channel list for the array
		 *
		 *   - Array2DType::width() and
		 *   - Array2DType::height() return the dimensions of the array.
		 *
		 * All the scanlines in buffer are read and written to the output file,
		 * starting from the current write line as reported by currentLine().
		 * If the buffer is higher than the specified image height, use only
		 * the first few rows.
		 *
		 * \param buffer - buffer to read scanline data from.
		 */
		template<typename Array2DType>
		void writePixels(const Array2DType& buffer);

		/** \brief Open an input image file in any format
		 *
		 * Uses magic numbers to determine the file format of the file given by
		 * fileName.  If the format is unknown or the file cannot be opened for
		 * some other reason, throw an exception.
		 *
		 * \param fileName - file to open.  Can be in any of the formats
		 * understood by aqsistex.
		 * \param fileType - string describing the file type.
		 * \return The newly opened input file
		 */
		static boost::shared_ptr<IqTexOutputFile> open(const std::string& fileName,
				const std::string& fileType, const CqTexFileHeader& header);

	protected:
		/** \brief Low-level virtual implementation for writePixels().
		 *
		 * The parameter is a CqMixedImageBuffer - this allows image formats
		 * the maximum flexibility in deciding what to do, since they have full
		 * access to the channel structure.
		 *
		 * \param buffer - pixel data will be read from here.
		 */
		virtual void writePixelsImpl(const CqMixedImageBuffer& buffer) = 0;
};


//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

template<typename Array2DType>
void IqTexOutputFile::writePixels(const Array2DType& buffer)
{
	TqInt numScanlines = min(buffer.height(), header().height() - currentLine());
	if(buffer.width() != header().width())
		throw XqInternal("Provided buffer has wrong width for output file",
				__FILE__, __LINE__);
	CqMixedImageBuffer newBuf(buffer.channelList(),
			boost::shared_array<TqUchar>(const_cast<TqUchar*>(buffer.rawData()),
				nullDeleter), buffer.width(), numScanlines);
	writePixelsImpl(newBuf);
}

} // namespace Aqsis

#endif // ITEXOUTPUTFILE_H_INCLUDED
