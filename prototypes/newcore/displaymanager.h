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

#ifndef AQSIS_DISPLAYMANAGER_H_INCLUDED
#define AQSIS_DISPLAYMANAGER_H_INCLUDED

#include <vector>

#include <boost/shared_ptr.hpp>

#include "renderer.h" // for OutvarSpec
#include "thread.h"
#include "util.h"

namespace Aqsis {

/// Manager for image output to displays
class DisplayManager
{
    public:
        /// Initialize displays with
        ///
        /// imageSize - dimensions of filtered image
        /// tileSize - size of filtered buckets
        /// outVars - set of variables specifying the channel structure of
        ///           incoming filtered tiles.
        DisplayManager(const V2i& imageSize, const V2i& tileSize,
                       const OutvarSet& outVars);

        /// Write a tile of filtered data at the given position 
        ///
        /// pos - the position of the pixel at the top left of the tile
        /// data - a tile of filtered data, with size and channels as described
        ///        to the DisplayManager constructor.
        ///
        /// This function is threadsafe.
        void writeTile(V2i pos, const float* data);

        /// Write images to files
        void closeFiles();

    private:
        void* tmpStorage(size_t size);

        class ImageFile;

        V2i m_imageSize;
        V2i m_tileSize;
        OutvarSet m_outVars;
        int m_totChans;      ///< number of channels per pixel

        std::vector<boost::shared_ptr<ImageFile> > m_files; ///< Image files
        ThreadSpecificPtr<std::vector<char> >::type m_tileTmpStorage;
                                            ///< Storage for writing to files
};


} // namespace Aqsis

#endif // AQSIS_DISPLAYMANAGER_H_INCLUDED
