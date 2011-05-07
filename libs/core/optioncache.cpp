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

#include "optioncache.h"

#include <aqsis/util/logging.h>
#include <aqsis/util/sstring.h>

namespace Aqsis {

// SqOptionCache implementation
SqOptionCache::SqOptionCache()
	: xFiltSize(1),
	yFiltSize(1),
	xSamps(1),
	ySamps(1),
	clipNear(0),
	clipFar(0),
	projectionType(ProjectionPerspective),
	shutterOpen(0),
	shutterClose(0),
	xBucketSize(16),
	yBucketSize(16),
	maxEyeSplits(1),
	displayMode(DMode_None),
	depthFilter(Filter_Min),
	zThreshold()
{ }

void SqOptionCache::cacheOptions(const IqOptions& opts)
{
	// Filter size
	const TqFloat* filtSize = opts.GetFloatOption("System", "FilterWidth");
	assert(filtSize);
	xFiltSize = filtSize[0];
	yFiltSize = filtSize[1];

	// Number of pixel samples.
	const TqInt* pixelSamps = opts.GetIntegerOption("System", "PixelSamples");
	assert(pixelSamps);
	xSamps = pixelSamps[0];
	ySamps = pixelSamps[1];

	// Location of the clipping planes.
	const TqFloat* clipLoc = opts.GetFloatOption("System", "Clipping");
	assert(clipLoc);
	clipNear = clipLoc[0];
	clipFar = clipLoc[1];

	// Type of projection
	projectionType = opts.GetIntegerOption("System", "Projection")[0];

	// Shutter times.
	const TqFloat* shutterTimes = opts.GetFloatOption("System", "Shutter");
	assert(shutterTimes);
	shutterOpen = shutterTimes[0];
	shutterClose = shutterTimes[1];

	// Bucket size.
	xBucketSize = 16;
	yBucketSize = 16;
	if(const TqInt* bktSize = opts.GetIntegerOption("limits", "bucketsize"))
	{
		xBucketSize = bktSize[0];
		yBucketSize = bktSize[1];
	}
	// Eye split bound
	maxEyeSplits = 10;
	if(const TqInt* splits = opts.GetIntegerOption("limits", "eyesplits"))
		maxEyeSplits = splits[0];

	// Display mode.
	const TqInt* dMode = opts.GetIntegerOption("System", "DisplayMode");
	assert(dMode);
	displayMode = static_cast<EqDisplayMode>(dMode[0]);

	// Depth filter
	depthFilter = Filter_Min;
	if(const CqString* filtStr = opts.GetStringOption("Hider", "depthfilter"))
	{
		Aqsis::log() << debug << "Depth filter = " << *filtStr << "\n";

		if(*filtStr == "min")            depthFilter = Filter_Min;
		else if(*filtStr == "midpoint")  depthFilter = Filter_MidPoint;
		else if(*filtStr == "max")       depthFilter = Filter_Max;
		else if(*filtStr == "average")   depthFilter = Filter_Average;
		else
		{
			Aqsis::log() << warning << "Invalid depthfilter \"" << *filtStr
				<< "\", depthfilter set to \"min\"\n";
		}
	}

	// Cache zthreshold.  The default threshold of 1,1,1 means that any objects
	// which are partially transparent won't appear in shadow maps.
	zThreshold = CqColor(1.0f);
	if(const CqColor* zTh = opts.GetColorOption("limits", "zthreshold"))
		zThreshold = zTh[0];
}

} // namespace Aqsis
