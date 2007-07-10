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

#include	"bucketprocessor.h"


START_NAMESPACE( Aqsis );


CqBucketProcessor::CqBucketProcessor() :
	m_bucket(0)
{
}

CqBucketProcessor::CqBucketProcessor(CqBucket* bucket) :
	m_bucket(bucket)
{
	setBucket(m_bucket);
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

void CqBucketProcessor::process( long xmin, long xmax, long ymin, long ymax, TqFloat clippingFar, TqFloat clippingNear )
{
	m_bucket->RenderWaitingMPs( xmin, xmax, ymin, ymax, clippingFar, clippingNear );
}

void CqBucketProcessor::finishProcessing()
{
	assert(m_bucket && !m_bucket->IsProcessed());

	m_bucket->SetProcessed();
}


END_NAMESPACE( Aqsis );
