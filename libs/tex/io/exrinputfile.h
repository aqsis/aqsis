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
