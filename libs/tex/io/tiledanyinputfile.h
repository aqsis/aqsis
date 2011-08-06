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
 * \brief Tiled input wrapper for files which don't really hold tiled data.
 *
 * \author Chris Foster
 */

#ifndef ITILEDANYINPUTFILE_H_INCLUDED
#define ITILEDANYINPUTFILE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>

#include <aqsis/tex/io/itiledtexinputfile.h>

namespace Aqsis {

class IqTexInputFile;

/** \brief Tiled input wrapper interface for non-tiled image formats
 *
 * This interface is a simple wrapper around any image format which can be
 * loaded using the IqTexInputFile interface.  As such, the "tiles" which are
 * provided are the size of the entire image which is likely to cause excessive
 * memory usage.  However, we will warn when using the interface and if the
 * user persists in not correctly mipmapping their files then they probably
 * deserve what they get ;-)
 */
class AQSIS_TEX_SHARE CqTiledAnyInputFile : public IqTiledTexInputFile
{
	public:
		/** \brief Open any texture file and interpret as a tiled file.
		 *
		 * This simply calls through to IqTexInputFile::open(), so fails in the
		 * same ways.
		 */
		CqTiledAnyInputFile(const boostfs::path& fileName);

		virtual boostfs::path fileName() const;
		virtual EqImageFileType fileType() const;
		virtual const CqTexFileHeader& header(TqInt index = 0) const;
		virtual SqTileInfo tileInfo() const;

		virtual TqInt numSubImages() const;
		virtual TqInt width(TqInt index) const;
		virtual TqInt height(TqInt index) const;
	private:
		virtual void readTileImpl(TqUint8* buffer, TqInt tileX, TqInt tileY,
				TqInt subImageIdx, const SqTileInfo tileSize) const;

		/// Underlying input file which does the real work.
		boost::shared_ptr<IqTexInputFile> m_texFile;
		/// Tile information
		SqTileInfo m_tileInfo;
};

} // namespace Aqsis

#endif // ITILEDANYINPUTFILE_H_INCLUDED
