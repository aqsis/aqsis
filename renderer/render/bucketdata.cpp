/* Aqsis - bucketdata.cpp
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

#include	"bucketdata.h"

namespace Aqsis {


	CqBucketData::CqBucketData() : m_hasValidSamples(false)
{
}

CqBucketData::~CqBucketData()
{
	reset();
}

void CqBucketData::reset()
{
	m_XOrigin = 0;
	m_YOrigin = 0;
	m_XSize = 0;
	m_YSize = 0;
	m_RealWidth = 0;
	m_RealHeight = 0;

	m_DiscreteShiftX = 0;
	m_DiscreteShiftY = 0;
	m_PixelXSamples = 0;
	m_PixelYSamples = 0;
	m_FilterXWidth = 0.0f;
	m_FilterYWidth = 0.0f;
	m_NumDofBounds = 0;

	m_viewRangeXMin = 0;
	m_viewRangeXMax = 0;
	m_viewRangeYMin = 0;
	m_viewRangeYMax = 0;
	m_clippingNear = 0.0f;
	m_clippingFar = 0.0f;

	m_DofBounds.clear();
	m_aieImage.clear();
	m_SamplePoints.clear();
	m_NextSamplePoint = 0;

	for( std::vector<std::vector<CqVector2D> >::iterator i = m_aSamplePositions.begin();
	     i != m_aSamplePositions.end();
	     ++i )
	{
		i->clear();
	}
	m_aSamplePositions.clear();
	m_aFilterValues.clear();
	m_aDatas.clear();
	m_aCoverages.clear();

	m_hasValidSamples = false;
}

void CqBucketData::setupOcclusionTree(const CqBucket& bucket, TqInt xmin, TqInt ymin, TqInt xmax, TqInt ymax)
{
	m_OcclusionTree.setupTree(bucket, xmin, ymin, xmax, ymax);
}

bool CqBucketData::canCull(const CqBound& bound) const
{
	return m_OcclusionTree.canCull(bound);
}


} // namespace Aqsis
