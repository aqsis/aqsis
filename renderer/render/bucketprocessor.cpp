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
	m_bucket(0), m_bucketCol(-1), m_bucketRow(-1)

{
}

CqBucketProcessor::~CqBucketProcessor()
{
}

void CqBucketProcessor::setBucket(CqBucket* bucket, TqInt bucketCol, TqInt bucketRow)
{
	assert(m_bucket == 0);

	m_bucket = bucket;
	m_bucket->SetBucketData(&m_bucketData);

	m_bucketCol = bucketCol;
	m_bucketRow = bucketRow;
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

	m_bucketCol = -1;
	m_bucketRow = -1;
	m_initiallyEmpty = false;
}

bool CqBucketProcessor::canCull(const CqBound* bound) const
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
		TIME_SCOPE("Prepare bucket");
		m_bucket->PrepareBucket( pos, size,
					 pixelXSamples, pixelYSamples, filterXWidth, filterYWidth,
					 viewRangeXMin, viewRangeXMax, viewRangeYMin, viewRangeYMax,
					 clippingNear, clippingFar,
					 true, m_initiallyEmpty );
	}

	if ( !m_initiallyEmpty )
	{
		TIME_SCOPE("Occlusion culling");
		m_bucketData.setupOcclusionHierarchy(m_bucket);
	}
}

void CqBucketProcessor::process()
{
	if (!m_bucket)
		return;

	TIME_SCOPE("Render MPs");
	m_bucket->RenderWaitingMPs();
}

void CqBucketProcessor::postProcess( bool imager, EqFilterDepth depthfilter, const CqColor& zThreshold )
{
	if (!m_bucket)
		return;

	// Combine the colors at each pixel sample for any
	// micropolygons rendered to that pixel.
	if (!m_initiallyEmpty)
	{
		TIME_SCOPE("Combine");
		m_bucket->CombineElements(depthfilter, zThreshold);
	}

	TIMER_START("Filter");
	m_bucket->FilterBucket(m_initiallyEmpty, imager);
	if (!m_initiallyEmpty)
	{
		m_bucket->ExposeBucket();
		// \note: Used to quantize here too, but not any more, as it is handled by
		//	  ddmanager in a specific way for each display.
	}
	TIMER_STOP("Filter");

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

TqInt CqBucketProcessor::getBucketCol() const
{
	return m_bucketCol;
}

TqInt CqBucketProcessor::getBucketRow() const
{
	return m_bucketRow;
}


END_NAMESPACE( Aqsis );
