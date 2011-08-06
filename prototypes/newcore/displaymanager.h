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

#ifndef AQSIS_DISPLAYMANAGER_H_INCLUDED
#define AQSIS_DISPLAYMANAGER_H_INCLUDED

#include <vector>

#include <boost/shared_ptr.hpp>

#include <aqsis/riutil/ricxx.h>

#include "arrayview.h"
#include "renderer.h" // for OutvarSpec
#include "thread.h"
#include "util.h"

namespace Aqsis {


//------------------------------------------------------------------------------
/// Display - a bare bones image output interface
///
/// TODO: We probably want to use OIIO's ImageOutput here instead, but for now
/// I'm trying to avoid that dependency.
class Display
{
    public:
        /// Open the image, recording some information
        ///
        /// Return true on success.
        virtual bool open(const std::string& fileName, const V2i& imageSize,
                          const V2i& tileSize, const VarSpec& varSpec) = 0;

        /// Write tile data at position pos to the file.  Must be threadsafe.
        ///
        /// pos - tile position
        /// data - pixel data
        ///
        /// Return true on success.
        virtual bool writeTile(const V2i& pos, void* data) = 0;

        /// Close the image
        ///
        /// Return true on success.
        virtual bool close() = 0;

        virtual ~Display() {}
};

typedef boost::shared_ptr<Display> DisplayPtr;


//------------------------------------------------------------------------------
/// A list of displays to use.
///
/// This class is mainly used to construct the list of displays in the API and
/// pass that to the renderer.
class DisplayList
{
    public:
        struct DisplayInfo
        {
            std::string name;
            VarSpec outputVar;
            DisplayPtr display;
        };

        /// Create one of the builtin displays from the given arguments.
        ///
        /// This corresponds more or less to the RI Display call.
        ///
        /// name - name of the image
        /// type - display type to create
        /// outVar - specification of output channels (the AOV for the display)
        /// pList - variable list of arguments to the display.
        ///
        /// Returns true if the display was created, false otherwise.
        bool addDisplay(const char* name, const char* type,
                        const VarSpec& outVar, const Ri::ParamList& pList);

        /// Remove currently held displays
        void clear() { m_displayInfo.clear(); }

        /// Access to individual displays
        int size() const { return m_displayInfo.size(); }
        const DisplayInfo& operator[](int i) const { return m_displayInfo[i]; }

        /// Get list of output variables (AOVs) which the current set of
        /// displays need.
        VarList requiredVars() const;

    private:
        typedef std::vector<DisplayInfo> Container;
        Container m_displayInfo;
};


//------------------------------------------------------------------------------
/// Class to prepare image data for output.
///
/// DisplayManager quantizes the incoming filtered data, rearranges the
/// channel structure, and generally formats the pixel data in a way that the
/// displays like to receive.
class DisplayManager
{
    public:
        /// Initialize displays with
        ///
        /// imageSize - dimensions of filtered image
        /// tileSize - size of filtered buckets
        /// outVars - set of variables specifying the channel structure of
        ///           incoming tiles coming from the renderer.
        /// displays - list of displays to send the data to
        DisplayManager(const V2i& imageSize, const V2i& tileSize,
                       const OutvarSet& outVars, const DisplayList& displays);

        /// Delete the display manager and close currently active displays.
        ~DisplayManager();

        /// Write a tile of filtered data at the given position 
        ///
        /// pos - the position of the pixel at the top left of the tile
        /// data - a tile of filtered data, with size and channels as described
        ///        to the DisplayManager constructor.
        ///
        /// This function is threadsafe.
        void writeTile(V2i pos, const float* data);


    private:
        struct DisplayData
        {
            OutvarSpec var;     ///< Location of variable in incoming data.
            bool quantize;      ///< Do quantize for this display?
            DisplayPtr display; ///< Display instance
        };

        void* tmpStorage(size_t size);
        static void quantize(ConstFvecView src, int nPix, unsigned char* out);

        V2i m_imageSize;
        V2i m_tileSize;
        int m_totChans;      ///< number of channels per pixel
        Mutex m_mutex;       ///< For locking during writeTile().

        std::vector<DisplayData> m_displayInfo;
        ThreadSpecificPtr<std::vector<char> >::type m_tileTmpStorage;
                                            ///< Storage for writing to files
};


} // namespace Aqsis

#endif // AQSIS_DISPLAYMANAGER_H_INCLUDED
