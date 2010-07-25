// Aqsis
// Copyright (C) 1997 - 2010, Paul C. Gregory
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

#ifndef OPTIONS_H_INCLUDED
#define OPTIONS_H_INCLUDED

#include <cfloat>
#include <OpenEXR/ImathVec.h>

#include "filters.h"

/// Renderer options.
struct Options
{
    // Reyes options
    int maxSplits;   ///< maximum number of splits before discarding a surface
    int gridSize;    ///< Desired grid side length.
    float clipNear;  ///< Near clipping plane (cam coords)
    float clipFar;   ///< Far clipping plane (cam coords)

    // Sampling options
    int xRes;         ///< image x-resolution
    int yRes;         ///< image y-resolution
    Imath::V2i superSamp;  ///< supersampling resolution
    float shutterMin; ///< shutter start time for motion blur
    float shutterMax; ///< shutter end time for motion blur
    float fstop;      ///< focalLength/lensDiameter
    float focalLength;   ///< lens focal length
    float focalDistance; ///< distance at which lens is focussed

    // Filtering options
    FilterPtr pixelFilter; ///< pixel filter functor
    bool doFilter;    ///< If false, turn off filtering & return raw samples

    Options()
        : maxSplits(20),
        gridSize(16),
        clipNear(FLT_EPSILON),
        clipFar(FLT_MAX),
        xRes(640),
        yRes(480),
        superSamp(2,2),
        shutterMin(0),
        shutterMax(0),
        fstop(FLT_MAX),
        focalLength(FLT_MAX),
        focalDistance(FLT_MAX),
        pixelFilter(makeGaussianFilter(Vec2(2,2))),
        doFilter(true)
    { }
};

#endif // OPTIONS_H_INCLUDED
