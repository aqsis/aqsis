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
 * \brief Scanline-oriented pixel access for TIFF input.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#ifndef TIFFINPUTFILE_H_INCLUDED
#define TIFFINPUTFILE_H_INCLUDED

#include "aqsis.h"

#include <string>

#include <boost/shared_ptr.hpp>

#include "itexfile.h"
#include "tiffdirhandle.h"

namespace Aqsis {

//------------------------------------------------------------------------------
class CqTiffInputFile : public IqTexInputFile
{
	public:
		CqTiffInputFile(const std::string& fileName);
		/// Constructor which takes an input stream rather than a file name
		CqTiffInputFile(std::istream& inStream);
		virtual const std::string& fileName() const;
		inline virtual const char* fileType() const;
		inline virtual const CqTexFileHeader& header() const;
	private:
		virtual void readPixelsImpl(TqUchar* buffer, TqInt startLine,
				TqInt numScanlines) const;

		/// Perform shared initializations needed in construction.
		void initialize();

		CqTexFileHeader m_header;
		boost::shared_ptr<CqTiffFileHandle> m_fileHandle;
};


//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

inline const char* CqTiffInputFile::fileType() const
{
	return "tiff";
}

inline const CqTexFileHeader& CqTiffInputFile::header() const
{
	return m_header;
}

} // namespace Aqsis

#endif // TIFFINPUTFILE_H_INCLUDED
