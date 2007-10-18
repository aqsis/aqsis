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
 * \brief Scanline-oriented pixel access for OpenEXR input.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#ifndef EXRINPUTFILE_H_INCLUDED
#define EXRINPUTFILE_H_INCLUDED

#include "aqsis.h"

#include <string>

#include <boost/shared_ptr.hpp>

#include "itexinputfile.h"

//------------------------------------------------------------------------------
namespace Imf {
	class InputFile;
}

namespace Aqsis {

//------------------------------------------------------------------------------
class CqExrInputFile : public IqTexInputFile
{
	public:
		CqExrInputFile(const std::string& fileName);
		/// Constructor which takes an input stream rather than a file name
		//CqExrInputFile(std::istream& inStream);
		virtual const char* fileName() const;
		inline virtual const char* fileType() const;
		inline virtual const CqTexFileHeader& header() const;
	private:
		virtual void readPixelsImpl(TqUchar* buffer, TqInt startLine,
				TqInt numScanlines) const;

		/// Perform shared initializations needed in construction.
		void initialize();

		CqTexFileHeader m_header;
		boost::shared_ptr<Imf::InputFile> m_exrFile;
};


//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

inline const char* CqExrInputFile::fileType() const
{
	return "OpenEXR";
}

inline const CqTexFileHeader& CqExrInputFile::header() const
{
	return m_header;
}

} // namespace Aqsis

#endif // EXRINPUTFILE_H_INCLUDED
