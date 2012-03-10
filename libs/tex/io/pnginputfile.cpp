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
 * \brief Scanline-oriented pixel access for TIFF input - implementation.
 *
 * \author Peter Dusel  hdusel _at_ tangerine-soft.de
 *
 */

#include "pnginputfile.h"
#include "boost/scoped_array.hpp"
#include <stdlib.h>

namespace Aqsis {

class CPNGReader
{
    FILE* m_fileHandle;
    png_structp m_PNGHandle;
    png_infop  m_infoPtr;
    png_bytep  m_ImageBuff;
    png_bytepp m_ImageBuffPtr;

public:
    CPNGReader(const boostfs::path& fileName)
    : m_fileHandle(::fopen(native(fileName).c_str(), "rb"))
    , m_PNGHandle(NULL)
    , m_infoPtr(NULL)

    , m_ImageBuff(NULL)
    , m_ImageBuffPtr(NULL)
    {
        initAll();
    }

    ~CPNGReader()
    {
        releaseImageBuffer();
        if ( NULL != m_fileHandle )
        {
            ::fclose(m_fileHandle);
        }
        ::png_destroy_read_struct(&m_PNGHandle, (png_infopp)&m_infoPtr,(png_infopp)NULL);
    }

    TqInt32 getWidth() const
    {
        return isValid() ? ::png_get_image_width(m_PNGHandle, m_infoPtr) : -1;
    }

    TqInt32 getHeight() const
    {
        return isValid() ? ::png_get_image_height(m_PNGHandle, m_infoPtr) : -1;
    }

    TqInt8 getNrOfChannels() const
    {
        return isValid() ? ::png_get_channels(m_PNGHandle, m_infoPtr) : 0;
    }

    TqInt32 getRowBytes() const
    {
        return isValid() ? ::png_get_rowbytes(m_PNGHandle, m_infoPtr) : 0;
    }

    const TqUint8* const getRowPtr(size_t inRowIndex) const
    {
        return (const TqUint8 * const)(isValid() ? m_ImageBuffPtr[inRowIndex] : NULL);
    }

    bool isValid() const {return NULL != m_fileHandle;}

private:
    bool initAll()
    {
        if (m_fileHandle)
        {
            bool success ( initialize_png_reader() );
            if (success)
            {
                ::png_init_io(m_PNGHandle, m_fileHandle);
                ::png_read_info(m_PNGHandle, m_infoPtr);

                success = initImageBuffer();
                if (success)
                {
                    return true; // success
                }
            }

        }

        ::fclose(m_fileHandle);
        m_fileHandle = NULL;
        return false; // failure
    }

    bool initImageBuffer()
    {
        assert( NULL == m_ImageBuff );
        assert( NULL == m_ImageBuffPtr );

        const size_t rowCnt(getHeight());

        if (rowCnt > 0)
        {
            const size_t bytesPerRow(getRowBytes());

            // Allocate the whole image buffer at once!
            const size_t imageBuffSize(rowCnt * bytesPerRow);
            png_bytep imagePtr = (png_bytep)::malloc(imageBuffSize);
            if ( NULL == imagePtr )
            {
                releaseImageBuffer();
                return false; // error
            }

            m_ImageBuff = imagePtr;
            m_ImageBuffPtr = (png_bytepp)::calloc(rowCnt, sizeof(png_bytep));

            for (size_t i=0; i != rowCnt; ++i, imagePtr += bytesPerRow)
            {
                m_ImageBuffPtr[i] = imagePtr;
            }
            ::png_read_image(m_PNGHandle, m_ImageBuffPtr);
        }
        return true;
    }

    void releaseImageBuffer()
    {
        if (m_ImageBuff)
        {
            ::free(m_ImageBuff);
            m_ImageBuff = NULL;
        }

        if (m_ImageBuffPtr)
        {
            ::free(m_ImageBuffPtr);
        }
        m_ImageBuffPtr = NULL;
    }

    /*  An example code fragment of how you would
     initialize the progressive reader in your
     application. */
    bool initialize_png_reader()
    {
        png_voidp user_error_ptr  = NULL;
        png_error_ptr user_error_fn   = NULL;
        png_error_ptr user_warning_fn = NULL;

        m_PNGHandle = ::png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                             (png_voidp)user_error_ptr,
                                             user_error_fn, user_warning_fn);
        if ( NULL != m_PNGHandle )
        {
            m_infoPtr = ::png_create_info_struct(m_PNGHandle);
            if ( NULL != m_infoPtr )
            {
                return true; // success
            }
        }

        ::png_destroy_read_struct(&m_PNGHandle, (png_infopp)&m_infoPtr,(png_infopp)NULL);

        return false; // fail
    }
}; // class CPNGReader

//------------------------------------------------------------------------------
// CqPngInputFile - implementation

CqPngInputFile::CqPngInputFile(const boostfs::path& fileName)
: IqTexInputFile()
, m_PNGReader(new CPNGReader(fileName))
, m_fileName(fileName)
, m_header()
{
    if (m_PNGReader->isValid())
    {
        m_header.setWidth(m_PNGReader->getWidth());
        m_header.setHeight(m_PNGReader->getHeight());

        CqChannelList& channelList = m_header.channelList();

        // set the channels
        channelList.clear();

        const TqUint8 nrOfChannels ( m_PNGReader->getNrOfChannels() );

        static const char* channelStructure[] = {"r","g","b","a"};

        const EqChannelType chanType( Channel_Unsigned8 );
        for (int i=0; i!= nrOfChannels; ++i)
        {
            channelList.addChannel( SqChannelInfo(channelStructure[i], chanType) );
        }
    }
}

CqPngInputFile::~CqPngInputFile()
{
    delete m_PNGReader;
}

static void copyRGBPixel(TqUint8* inDestBuff, const TqUint8* inSrcBuff, size_t buffSize)
{
    ::memcpy((void*)inDestBuff, inSrcBuff, buffSize);
}

inline static TqUint8 premultiplyAlpha(TqUint8 inColorValue, TqUint8 inAlpha)
{
    return (((TqUint16)inColorValue) * inAlpha) / 255;
}

/* Copy the pixels from a source buffer to a destination buffer by assuming that each pixel
 * consists of 4 components (RGBA).
 *
 * It is noteworthy that PNG assumes "non premultiplied" data while Aqsis relies on
 * premultiplied color components.
 *
 * This implies that loading PNG files which consists of a alpha component requires an additional
 * precessing.
 *
 * In order to fulfill this needs this function performs a premultiply of each color component
 * with the value of the alpha component.
 */
static void copyRGBAPremultiPixel(TqUint8* inDestBuff, const TqUint8* inSrcBuff, size_t buffSize)
{
    for(; buffSize >= 4 ; buffSize -= 4)
    {
        const TqUint8 alpha( inSrcBuff[3] );

        *(inDestBuff++) = premultiplyAlpha(*(inSrcBuff++), alpha);
        *(inDestBuff++) = premultiplyAlpha(*(inSrcBuff++), alpha);
        *(inDestBuff++) = premultiplyAlpha(*(inSrcBuff++), alpha);
        *(inDestBuff++) = *(inSrcBuff++);
    }
}

typedef void (*LineCopyFuncPtr)(TqUint8* inDestBuff, const TqUint8* inSrcBuff, size_t buffSize);

void CqPngInputFile::readPixelsImpl(TqUint8* buffer, TqInt startLine,
                    TqInt numScanlines) const
{
    assert(buffer);

    const size_t destBytesPerPixel(getNrOfChannels() * sizeof(TqUint8));
    const size_t destBytesPerLine(destBytesPerPixel * getWidth());

    TqUint8* destBuff(buffer);

    // Dependent from the color model use a special copy routine.
    LineCopyFuncPtr lineCopyFunc = (getNrOfChannels() == 3) ?
        copyRGBPixel :         // If the number of channels is 3 then use copyRGBPixel(...)
        copyRGBAPremultiPixel; // If the number of channels is 4 then use copyRGBAPremultiPixel(...)

    for (int lineIdx = 0; lineIdx != numScanlines; ++lineIdx, destBuff += destBytesPerLine)
    {
        const TqUint8* srcBuff(getRowPtr(lineIdx));
        assert(srcBuff);
        if ( NULL != srcBuff )
        {
            lineCopyFunc(destBuff, srcBuff, destBytesPerLine);
        }
    }
}

 // Private

const TqUint8* const CqPngInputFile::getRowPtr(size_t inRowIndex) const
{
    return m_PNGReader->getRowPtr(inRowIndex);
}

TqUint8 CqPngInputFile::getNrOfChannels() const
{
    return m_PNGReader->getNrOfChannels();
}

TqUint32 CqPngInputFile::getRowBytes() const
{
    return m_PNGReader->getRowBytes();
}

TqUint32 CqPngInputFile::getWidth() const
{
    return m_PNGReader->getWidth();
}

TqUint32 CqPngInputFile::getHeight() const
{
    return m_PNGReader->getHeight();
}

} // namespace Aqsis
