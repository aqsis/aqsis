/* Aqsis
 *
 * Copyright (C) 2007 Manuel A. Fernadez Montecelo <mafm@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include	"bucket.h"
#include	"MultiTimer.h"

#include	"bucketprocessor.h"


START_NAMESPACE( Aqsis );


CqBucketProcessor::CqBucketProcessor() :
	m_bucket(0)
{
}

CqBucketProcessor::~CqBucketProcessor()
{
}

void CqBucketProcessor::setBucket(CqBucket* bucket)
{
	assert(m_bucket == 0);

	m_bucket = bucket;
	m_bucket->SetBucketData(&m_bucketData);
}

void CqBucketProcessor::reset()
{
	assert(m_bucket && m_bucket->IsProcessed());

	m_bucket->SetBucketData(static_cast<CqBucketData*>(0));
	m_bucket = 0;
}

bool CqBucketProcessor::canCull(const CqBound* bound) const
{
	return m_bucketData.canCull(bound);
}

void CqBucketProcessor::preProcess(TqInt xorigin, TqInt yorigin, TqInt xsize, TqInt ysize,
				   TqInt pixelXSamples, TqInt pixelYSamples, TqFloat filterXWidth, TqFloat filterYWidth,
				   bool empty)
{
	assert(m_bucket);

	{
		TIME_SCOPE("Prepare bucket");
		m_bucket->PrepareBucket( xorigin, yorigin, xsize, ysize,
					 pixelXSamples, pixelYSamples, filterXWidth, filterYWidth,
					 true, empty );
	}

	if ( !empty )
	{
		TIME_SCOPE("Occlusion culling");
		m_bucketData.setupOcclusionHierarchy(m_bucket);
	}
}

void CqBucketProcessor::process( long xmin, long xmax, long ymin, long ymax, TqFloat clippingFar, TqFloat clippingNear )
{
	assert(m_bucket);

	TIME_SCOPE("Render MPs");
	m_bucket->RenderWaitingMPs( xmin, xmax, ymin, ymax, clippingFar, clippingNear );
}

void CqBucketProcessor::postProcess( bool empty, bool imager, EqFilterDepth depthfilter, const CqColor& zThreshold )
{
	assert(m_bucket);

	// Combine the colors at each pixel sample for any
	// micropolygons rendered to that pixel.
	if (!empty)
	{
		TIME_SCOPE("Combine");
		m_bucket->CombineElements(depthfilter, zThreshold);
	}

	TIMER_START("Filter");
	m_bucket->FilterBucket(empty, imager);
	if (!empty)
	{
		m_bucket->ExposeBucket();
		// \note: Used to quantize here too, but not any more, as it is handled by
		//	  ddmanager in a specific way for each display.
	}
	TIMER_STOP("Filter");

	assert(!m_bucket->IsProcessed());
	m_bucket->SetProcessed();
}

END_NAMESPACE( Aqsis );
