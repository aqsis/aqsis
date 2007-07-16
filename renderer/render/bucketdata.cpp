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

#include	"bucketdata.h"

START_NAMESPACE( Aqsis );


CqBucketData::CqBucketData()
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

	m_DofBounds.clear();
	m_aieImage.clear();
	m_SamplePoints.clear();
	m_NextSamplePoint = 0;

	for( std::vector<std::vector<CqVector2D> >::iterator i = m_aSamplePositions.begin();
	     i != m_aSamplePositions.end();
	     i++ )
	{
		i->clear();
	}
	m_aSamplePositions.clear();
	m_aFilterValues.clear();
	m_aDatas.clear();
	m_aCoverages.clear();
}

void CqBucketData::setupOcclusionHierarchy(const CqBucket* bucket)
{
	assert(bucket);
	m_OcclusionBox.SetupHierarchy(bucket);
}

bool CqBucketData::canCull(const CqBound* bound) const
{
	return m_OcclusionBox.KDTree()->CanCull(bound);
}



END_NAMESPACE( Aqsis );
