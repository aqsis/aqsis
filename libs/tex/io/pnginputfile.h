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
 * \brief Scanline-oriented pixel access for PNG input.
 *
 * \author Peter Dusel  hdusel _at_ tangerine-soft.de
 *
 */

#ifndef PNGINPUTFILE_H_INCLUDED
#define PNGINPUTFILE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <string>
#include <png.h>

#include <boost/shared_ptr.hpp>
#include <aqsis/tex/io/itexinputfile.h>

namespace Aqsis {

class CPNGReader;
//------------------------------------------------------------------------------
/** \brief Scanline-oriented input class for PNG files.
 *
 * This class puts a scanline interface onto strip-based or tiled PNG files,
 * and attempts to hide a lot of the complexity of the PNG format behind a
 * uniform interface.
 *
 * For cases of unusual internal formats, the class falls back on the generic
 * RGBA image handling built into libPNG.
 */
class AQSIS_TEX_SHARE CqPngInputFile : public IqTexInputFile
{
    CPNGReader *m_PNGReader;
    boostfs::path m_fileName;
    CqTexFileHeader m_header;

public:
    CqPngInputFile(const boostfs::path& fileName);
    /** \param Read the tiff from an input stream rather than a file.
     *
     * \param inStream - Stream to read from.  This is passed to the
     * underlying tiff (tiffxx) library.
     */
    //    CqPngInputFile(std::istream& inStream);

    virtual ~CqPngInputFile();

    // inherited
    /// \name Metadata access
    //@{
    /// get the file name
    virtual boostfs::path fileName() const
    {
        return m_fileName;
    }

    /// get a string representing the file type
    virtual EqImageFileType fileType() const
    {
        return ImageFile_Png;
    }

    /// Get the file header data
    virtual const CqTexFileHeader& header() const
    {
        return m_header;
    }
    //@}

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
            TqInt numScanlines) const;

private:
    TqUint8 getNrOfChannels() const;
    TqUint32 getRowBytes() const;

    TqUint32 getWidth() const;
    TqUint32 getHeight() const;

    const TqUint8* const getRowPtr(size_t inRowIndex) const;
};
//==============================================================================
// Implementation details
//==============================================================================

} // namespace Aqsis
#endif // PNGINPUTFILE_H_INCLUDED
