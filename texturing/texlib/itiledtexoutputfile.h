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
 * \brief A tile-based output interface for texture files.
 *
 * \author Chris Foster
 */

#ifndef ITILEDTEXOUTPUTFILE_H_INCLUDED
#define ITILEDTEXOUTPUTFILE_H_INCLUDED

#include "aqsis.h"

#include <string>

#include "exception.h"
#include "imagefiletype.h"
#include "mixedimagebuffer.h"
#include "smartptr.h"
#include "texfileheader.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Tile-oriented image file output.
 */
class AQSISTEX_SHARE IqTiledTexOutputFile
{
	public:
		virtual ~IqTiledTexOutputFile() {};

		//--------------------------------------------------
		/// \name Metadata access
		//@{
		/// get the file name
		virtual const char* fileName() const = 0;
		/// get the file type
		virtual EqImageFileType fileType() = 0;
		/// Get the file header data
		virtual const CqTexFileHeader& header() const = 0;
		//@}

		/** \brief Write the entire image at once.
		 *
		 * The buffer should be written into the file as tiles; subsequent
		 * calls to this function should write a new subimage into the
		 * multimage file with each call.
		 *
		 * Array2DType is a type modelling a 2D array interface.  It should
		 * provide the following methods:
		 *
		 *   - TqUint8* Array2dType::rawData() returns a pointer to the raw
		 *     data.  The raw data is assumed at this stage to be contiguous -
		 *     (ie, not a nontrivial slice).
		 *
		 *   - Array2DType::channelList() returns a channel list for the array
		 *
		 *   - Array2DType::width() and
		 *   - Array2DType::height() return the dimensions of the array.
		 *
		 * \param buffer - buffer to read scanline data from.
		 */
		template<typename Array2DType>
		void writePixels(const Array2DType& buffer);

		/** \brief Open an input image file in a given format
		 *
		 * \param fileName - file to open.  Can be in any of the formats
		 * understood by aqsistex.
		 * \param fileType - the file type.
		 * \return The newly opened input file
		 */
		static boost::shared_ptr<IqTiledTexOutputFile> open(const std::string& fileName,
				EqImageFileType fileType, const CqTexFileHeader& header);

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
// Implementation details
//==============================================================================

template<typename Array2DType>
void IqTiledTexOutputFile::writePixels(const Array2DType& buffer)
{
	CqMixedImageBuffer newBuf(buffer.channelList(),
			boost::shared_array<TqUint8>(const_cast<TqUint8*>(buffer.rawData()), nullDeleter),
			buffer.width(), buffer.height());
	writePixelsImpl(newBuf);
}

} // namespace Aqsis

#endif // ITILEDTEXOUTPUTFILE_H_INCLUDED
