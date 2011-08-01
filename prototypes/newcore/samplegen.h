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

#ifndef AQSIS_SAMPLEGEN_H_INCLUDED
#define AQSIS_SAMPLEGEN_H_INCLUDED

#include <vector>

#include "util.h"

namespace Aqsis {

/// Make a set of well-distributed time and lens samples.
///
/// The sample data is put into the tuv array as follows:
/// [t1 u1 v1  t2 u2 v2  ...  tn uv vn]
///
/// time samples are uniformly distributed on the interval [0,1], while lens
/// samples are uniformly distributed on the unit disk.
///
void canonicalTimeLensSamps(std::vector<float>& tuv, int nsamps);


/// Create a complete three color corner tile set of time & lens indices.
///
/// Each tile in the set has a "color" assigned to each corner.  The idea is
/// that we can put corners with the same colouring together, and the sample
/// positions will be will distributed across the tile boundaries.  For
/// example, the tiles RGBR and GBRB fit together like this:
///
///    R -- G -- B
///    |    |    |
///    |    |    |
///    B -- R -- B
///
/// (Contrast this with the more traditional Wang tilings which colour the
/// tile edges rather than the corners.)
///
/// For creating the tiles, we use the scheme given in the paper "An
/// Alternative for Wang Tiles: Colored Edges versus Colored Corners" by A.
/// Lagae and P. Dutre, with modifications.  In Lagae & Dutre they focus on
/// creating tiles which fit together at the edges into a good spatial
/// poission disk distribution.  Instead, here we assume we have the /spatial/
/// sample locations fixed on a regular grid, and try to distribute the time
/// and lens offsets so that any small rectangular spatial region has a evenly
/// distributed set of times & lens positions.
///
/// It's convenient to store all tiles together in a single array.
/// Conceptually this is a multidimensional array indexed by the corner colors
/// c1,c2,c3,c4 and spatial position x,y as follows:
///
///    tiles[(((((ncol*c4 + c3)*ncol + c2)*ncol) + c1)*width + y)*width + x]
///
/// This assumes a corner ordering of
///
///    c1-c2
///    |   |
///    c3-c4
///
/// \param tiles - destination array for tiles, with tile ordering as discussed
///                above.
/// \param width - desired tile width in number of samples.  Must have
///                width >= 4 for the sample tile scheme to work correctly.
/// \param tuv - time and lens offsets in the format [t1 u1 v1  t2 u2 v2 ...]
/// \param timeStratQuality - How important stratification in the time
///              dimension is relative to stratification in the lens dimension.
///              The value should be between 0 and 1, with 1 indicating that
///              only time is included in the stratification quality estimate.
///              The default of 0.5 indicates roughly equal importance between
///              time and lens.
///
void makeTileSet(std::vector<int>& tiles, int width, std::vector<float>& tuv,
                 float timeStratQuality = 0.5f);


//------------------------------------------------------------------------------
struct TimeLens
{
    float time;  ///< sample time
    V2f lens;   ///< position of sample on lens
};

/// Storage for a tile set as described in the makeTileSet() docs.
class DofMbTileSet
{
    public:
        DofMbTileSet(int tileWidth, float timeStratQuality,
                     float shutterMin, float shutterMax);

        /// Construct tile set, first looking in a cache for efficiency.
        ///
        /// Use this instead of the constructor to avoid the tile set creation
        /// costs where possible.  Creating tile sets with large tileWidth
        /// using makeTileSet can be expensive!
        ///
        /// This function is threadsafe.
        static const DofMbTileSet& create(int tileWidth,
                                          float timeStratQuality,
                                          float shutterMin, float shutterMax);

        /// Get tile with given corner colour indices.
        ///
        ///  c1-c2
        ///  |   |
        ///  c3-c4
        ///
        const int* getTile(int c1, int c2, int c3, int c4) const
        {
            const int ncol = 3;
            return &m_tileIndices[(((c4*ncol + c3)*ncol + c2)*ncol + c1) *
                                  m_tileWidth*m_tileWidth];
        }

        const std::vector<TimeLens>& timeLensPositions() const
        {
            return m_tuv;
        }

        V2i tileSize() const
        {
            return V2i(m_tileWidth);
        }

    private:
        int m_tileWidth;
        std::vector<int> m_tileIndices;
        std::vector<TimeLens> m_tuv;
};


//------------------------------------------------------------------------------
/// A hash mapping from a 2D integer lattice to a (small) integer.
///
/// This is useful when tiling a plane where we want a pseudo-random (but
/// reproducible) integer at each lattice point.  For example, consider a tile
/// set of square "corner tiles" where each tile has a "colour" assigned to
/// each corner.  To tile the plane so that the colours of the corners match
/// up, we just hash each corner position to get a colour index, and look up
/// the associated tiles.  Using a deterministic hash in this case (rather than
/// pseudo-random numbers) is important so that the tiling can be generated in
/// any order.
///
/// TODO: Is this class sufficiently generally useful to exist, or should it
/// all just be in a function with a static permutation table?
class SpatialHash
{
    public:
        SpatialHash(int nColors, int tableSize = 0)
            : m_permTable(tableSize),
            m_nColors(nColors)
        {
            if(tableSize == 0)
            {
                // A particular permutation table for definiteness.  Having a
                // fixed table is really useful when debugging.
                int tableInit[] = {
                 41,  52, 248, 166, 162,  92,  95,  11,   3,  94, 219, 198, 170,
                165,  80, 231,  75,  19,  20, 240, 169, 105,  15,  51,  45,  84,
                 85, 157, 232,  23,  97,  14, 174,   0,  82,  42, 145,  69, 111,
                178, 158, 138,  17,  70, 238, 137, 179,  64, 172,  61,  53, 177,
                 91,  73, 234,  18, 148, 223,  68, 187, 123,   7,   8,  12,   2,
                 27, 133,  66, 194, 160, 189, 142, 253,  81,  87, 212, 193, 102,
                210, 154,  28,  21,  24, 101, 192, 249, 168,  71, 233, 237,  98,
                 79, 119, 152, 183, 116, 211, 217,  29, 184, 220,  54,  31, 163,
                134, 120,  78, 245, 195, 204,  56, 140, 180,  99, 112,  57, 146,
                228, 201, 106, 197, 115, 250,  47, 122,  40, 203, 132,  33,  89,
                225, 196,  83,  58, 182,  34, 218, 205,  50,  10, 206,  88,  49,
                  1, 109, 254,  67, 144, 150, 230, 235, 213, 181, 242, 208, 246,
                 86, 236, 190,  13, 188,  59, 209,  48,  74, 214, 175, 215,  77,
                114,  35, 252, 227, 118,  72, 167, 153, 129, 121, 164, 244, 243,
                226, 125,  44,  62, 239, 136, 151, 173, 171,   5, 110,  96, 216,
                222, 113, 229, 161, 155, 221, 124, 103,   6, 255, 108, 126, 147,
                 32,  93, 135, 200, 100,  43, 251, 141, 130, 117,  39,  60, 185,
                149,  38,  26, 143,  55,  36, 127, 128,   4,  16, 207, 139, 159,
                 22, 191,  25, 156, 131,  46,  30, 224, 176,  65,  90, 202, 186,
                104, 107,   9, 247,  63, 199,  37, 241,  76
                };
                m_permTable.assign(tableInit, tableInit+256);
            }
            else
            {
                for(int i = 0; i < tableSize; ++i)
                    m_permTable[i] = i;
                std::random_shuffle(m_permTable.begin(), m_permTable.end());
            }
        }

        /// Get hash value at position (x,y).
        int operator()(int x, int y) const
        {
            const int N = m_permTable.size();
            int i = x % N;
            if(i < 0) // allow for negative x
                i += N;
            i = (m_permTable[i%N] + y) % N;
            if(i < 0) // allow for negative y
                i += N;
            return m_permTable[i] % m_nColors;
        }

    private:
        std::vector<int> m_permTable;
        int m_nColors;
};


//------------------------------------------------------------------------------
// The following functions are exposed only for unit testing.

void findRemainingInds(const int* tile, int fullwidth, int width,
                       int* toPlace, int& nToPlace, int* remainingInds);

int getMinCostIdx(const int* remainingInds, int nRemain,
                  const int* validNeighbours, const float* validWeights,
                  int nvalid, const float* tuv, float timeStratQuality);

} // namespace Aqsis

#endif // AQSIS_SAMPLEGEN_H_INCLUDED
