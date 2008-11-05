/* Aqsis - bucketprocessor.cpp
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
#include	"multitimer.h"

#include	"bucketprocessor.h"


namespace Aqsis {


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
	m_initiallyEmpty = m_bucket->IsEmpty();
}

const CqBucket* CqBucketProcessor::getBucket() const
{
	return m_bucket;
}

void CqBucketProcessor::reset()
{
	if (!m_bucket)
		return;

	assert(m_bucket && m_bucket->IsProcessed());

	m_bucket->SetBucketData(static_cast<CqBucketData*>(0));
	m_bucket = 0;

	m_initiallyEmpty = false;
}

bool CqBucketProcessor::canCull(const CqBound& bound) const
{
	return m_bucketData.canCull(bound);
}

void CqBucketProcessor::preProcess(const CqVector2D& pos, const CqVector2D& size,
				   TqInt pixelXSamples, TqInt pixelYSamples, TqFloat filterXWidth, TqFloat filterYWidth,
				   TqInt viewRangeXMin, TqInt viewRangeXMax, TqInt viewRangeYMin, TqInt viewRangeYMax,
				   TqFloat clippingNear, TqFloat clippingFar)
{
	assert(m_bucket);

	{
		AQSIS_TIME_SCOPE(Prepare_bucket);
		m_bucket->PrepareBucket( pos, size,
					 pixelXSamples, pixelYSamples, filterXWidth, filterYWidth,
					 viewRangeXMin, viewRangeXMax, viewRangeYMin, viewRangeYMax,
					 clippingNear, clippingFar,
					 true, m_initiallyEmpty );
	}

	if ( !m_initiallyEmpty )
	{
		AQSIS_TIME_SCOPE(Occlusion_culling_initialisation);
		m_bucketData.setupOcclusionTree(*m_bucket, viewRangeXMin, viewRangeYMin, viewRangeXMax, viewRangeYMax);
	}
}

void CqBucketProcessor::process()
{
	if (!m_bucket)
		return;

	// Render any waiting subsurfaces.
	// \todo Need to refine the exit condition, to ensure that all previous buckets have been
	// duly processed.
	while ( hasPendingSurfaces() )
	{
		boost::shared_ptr<CqSurface> surface = getTopSurface();
		if (surface)
		{
			// Advance to next surface
			popSurface();
			m_bucket->RenderSurface( surface );
			{
				AQSIS_TIME_SCOPE(Render_MPGs);
				m_bucket->RenderWaitingMPs();
			}
		}
	}
	{
		AQSIS_TIME_SCOPE(Render_MPGs);
		m_bucket->RenderWaitingMPs();
	}
}

void CqBucketProcessor::postProcess( bool imager, EqFilterDepth depthfilter, const CqColor& zThreshold )
{
	if (!m_bucket)
		return;

	// Combine the colors at each pixel sample for any
	// micropolygons rendered to that pixel.
	if (!m_initiallyEmpty)
	{
		AQSIS_TIME_SCOPE(Combine_samples);
		m_bucket->CombineElements(depthfilter, zThreshold);
	}

	AQSIS_TIMER_START(Filter_samples);
	m_bucket->FilterBucket(m_initiallyEmpty, imager);
	if (!m_initiallyEmpty)
	{
		m_bucket->ExposeBucket();
		// \note: Used to quantize here too, but not any more, as it is handled by
		//	  ddmanager in a specific way for each display.
	}
	AQSIS_TIMER_STOP(Filter_samples);

	assert(!m_bucket->IsProcessed());
	m_bucket->SetProcessed();
}

bool CqBucketProcessor::isInitiallyEmpty() const
{
	return m_initiallyEmpty;
}

void CqBucketProcessor::setInitiallyEmpty(bool value)
{
	m_initiallyEmpty = value;
}

bool CqBucketProcessor::hasPendingSurfaces() const
{
	assert(m_bucket);

	return m_bucket->hasPendingSurfaces();
}

bool CqBucketProcessor::hasPendingMPs() const
{
	assert(m_bucket);

	return m_bucket->hasPendingMPs();
}

boost::shared_ptr<CqSurface> CqBucketProcessor::getTopSurface()
{
	assert(m_bucket);

	return m_bucket->pTopSurface();
}

void CqBucketProcessor::popSurface()
{
	assert(m_bucket);

	return m_bucket->popSurface();
}


} // namespace Aqsis
