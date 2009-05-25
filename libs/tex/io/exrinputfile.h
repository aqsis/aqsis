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

#include <aqsis/aqsis.h>

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include <aqsis/tex/io/itexinputfile.h>

//------------------------------------------------------------------------------
namespace Imf {
	class InputFile;
}

namespace Aqsis {

typedef std::map<std::string,std::string> TqChannelNameMap;

namespace Attr {
	/**
	 * Extra image attribute to record the mapping from CqChannelList to EXR
	 * channel names.
	 */
	AQSIS_IMAGE_ATTR_TAG(ExrChannelNameMap, TqChannelNameMap);
}

//------------------------------------------------------------------------------
/** \brief Scanline-oriented input of data from OpenEXR files.
 *
 * This class should be able to read most OpenEXR data which can be read by the
 * Imf::InputFile class from the OpenEXR library.  There are a few exceptions,
 * including data saved with subsampled colour channels, such as
 * luminance-chroma data.
 */
class AQSIS_TEX_SHARE CqExrInputFile : public IqTexInputFile
{
	public:
		CqExrInputFile(const boostfs::path& fileName);
		/// Constructor which takes an input stream rather than a file name
		//CqExrInputFile(std::istream& inStream);
		virtual boostfs::path fileName() const;
		virtual EqImageFileType fileType() const;
		virtual const CqTexFileHeader& header() const;
	private:
		virtual void readPixelsImpl(TqUint8* buffer, TqInt startLine,
				TqInt numScanlines) const;

		/// Perform shared initializations needed in construction.
		void initialize();

		/// Header data from EXR file.
		CqTexFileHeader m_header;
		/// pointer to underlying OpenEXR file.
		boost::shared_ptr<Imf::InputFile> m_exrFile;
};


//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

inline EqImageFileType CqExrInputFile::fileType() const
{
	return ImageFile_Exr;
}

inline const CqTexFileHeader& CqExrInputFile::header() const
{
	return m_header;
}

} // namespace Aqsis

#endif // EXRINPUTFILE_H_INCLUDED
