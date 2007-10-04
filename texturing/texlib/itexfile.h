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
 * \brief Declare input and output interface specifications which should be
 * implemented by all classes wrapping texture files.
 *
 * \author Chris Foster
 */

#ifndef ITEXFILE_H_INCLUDED
#define ITEXFILE_H_INCLUDED

#include "aqsis.h"

#include <string>

#include "texfileheader.h"
#include "texturebuffer.h"

namespace Aqsis {

//------------------------------------------------------------------------------
class IqTexInputFile
{
	public:
		virtual ~IqTexInputFile() {};

		virtual const std::string& fileName() const = 0;

		virtual const CqTexFileHeader& header() const = 0;
		/** \brief Read in a region of scanlines
		 *
		 * \param buffer - buffer to read scanlines into
		 * \param startLine - scanline to start reading the data from (top == 0)
		 * \param numScanlines - number of scanlines to read.  If <= 0, read
		 *                       to the end of the image.
		 */
		virtual void readPixels(CqTextureBufferBase& buffer,
				TqInt startLine = 0, TqInt numScanlines = 0) const;

		/** \brief Open an image file in any understood format
		 */
		static boost::shared_ptr<IqTexInputFile> open(const std::string& fileName);
	protected:
		virtual void readPixelsImpl(CqTextureBufferBase& buffer, TqInt startLine,
				TqInt numScanlines) const = 0;
};


} // namespace Aqsis

#endif // ITEXFILE_H_INCLUDED
