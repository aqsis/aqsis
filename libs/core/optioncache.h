// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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

/// \file \brief Option cache

#ifndef OPTIONCACHE_H_INCLUDED
#define OPTIONCACHE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <aqsis/math/color.h>
#include <aqsis/core/ioptions.h>

namespace Aqsis {


/// Filter types for shadow map depth data.
enum EqDepthFilter
{
    Filter_Min,
    Filter_MidPoint,
    Filter_Max,
    Filter_Average
};


/** \brief Cache for RiOptions for fast access during rendering.
 *
 * The generic mechanism for storing RiOptions is too slow for accesses which
 * happen very very frequently, so we store them here.
 */
struct SqOptionCache
{
	TqFloat xFiltSize;  ///< x filter size of pixel filter
	TqFloat yFiltSize;  ///< y filter size of pixel filter
	TqInt xSamps;       ///< number of PixelSamples in the x-direction
	TqInt ySamps;       ///< number of PixelSamples in the y-direction

	TqFloat clipNear;   ///< Depth of the near clipping plane
	TqFloat clipFar;    ///< Depth of the far clipping plane
	TqInt projectionType; ///< Type for the camera->raster transform

	TqFloat shutterOpen;  ///< Camera shutter open time
	TqFloat shutterClose; ///< Camera shutter close time

	TqInt xBucketSize;  ///< Bucket size in the x-direction
	TqInt yBucketSize;  ///< Bucket size in the y-direction
	TqInt maxEyeSplits; ///< Maximum allowed number of eye splits

	EqDisplayMode displayMode; ///< Type of the connected displays

	EqDepthFilter depthFilter; ///< Type of depth filter to use
	CqColor zThreshold; ///< Opacity threshold for inclusion in depth maps

	/// Initialise all options to non-catastrophic defaults.
	SqOptionCache();
	/// Populate the cache with options extracted from opts.
	void cacheOptions(const IqOptions& opts);
};

} // namespace Aqsis

#endif // OPTIONCACHE_H_INCLUDED
