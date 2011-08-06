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

#ifndef AQSIS_OPTIONS_H_INCLUDED
#define AQSIS_OPTIONS_H_INCLUDED

#include <cfloat>

#include "filters.h"
#include "refcount.h"
#include "util.h"

namespace Aqsis {

/// Renderer options.
struct Options : public RefCounted
{
    // Reyes options
    int eyeSplits;   ///< maximum number of splits before discarding a surface
    int gridSize;    ///< Desired grid side length.
    float clipNear;  ///< Near clipping plane (cam coords)
    float clipFar;   ///< Far clipping plane (cam coords)

    // Sampling options
    V2i resolution;   ///< image resolution
    V2i bucketSize;   ///< linear edge length of bucket in pixels
    V2i superSamp;    ///< supersampling resolution
    int interleaveWidth; ///< Width of interleaved sampling tiles
    float shutterMin; ///< shutter start time for motion blur
    float shutterMax; ///< shutter end time for motion blur
    float fstop;      ///< focalLength/lensDiameter
    float focalLength;   ///< lens focal length
    float focalDistance; ///< distance at which lens is focussed

    // Filtering options
    FilterPtr pixelFilter; ///< pixel filter functor
    bool doFilter;    ///< If false, turn off filtering & return raw samples

    // Other options
    int statsVerbosity; ///< Verbosity for render statistics reporting
    int nthreads;       ///< Number of threads to use

    Options()
        : eyeSplits(20),
        gridSize(16),
        clipNear(FLT_EPSILON),
        clipFar(FLT_MAX),
        resolution(640,480),
        bucketSize(16),
        superSamp(2,2),
        interleaveWidth(6),
        shutterMin(0),
        shutterMax(0),
        fstop(FLT_MAX),
        focalLength(FLT_MAX),
        focalDistance(FLT_MAX),
        pixelFilter(makeGaussianFilter(V2f(2,2))),
        doFilter(true),
        statsVerbosity(0),
        nthreads(-1)
    { }
};

typedef boost::intrusive_ptr<Options> OptionsPtr;


} // namespace Aqsis

#endif // AQSIS_OPTIONS_H_INCLUDED
