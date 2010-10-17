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
// The following functions are exposed only for unit testing.

void findRemainingInds(const int* tile, int fullwidth, int width,
                       int* toPlace, int& nToPlace, int* remainingInds);

int getMinCostIdx(const int* remainingInds, int nRemain,
                  const int* validNeighbours, const float* validWeights,
                  int nvalid, const float* tuv, float timeStratQuality);

} // namespace Aqsis

#endif // AQSIS_SAMPLEGEN_H_INCLUDED
