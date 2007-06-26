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

/** \file
 *
 * \brief File holding code to process buckets.
 *
 * \author Manuel A. Fernadez Montecelo <mafm@users.sourceforge.net>
 */

#ifndef BUCKETPROCESSOR_H_INCLUDED
#define BUCKETPROCESSOR_H_INCLUDED 1

#include	"aqsis.h"
#include	"bucketdata.h"

class CqBucket;


START_NAMESPACE( Aqsis );


/**
 * \brief Class to process Buckets.
 */
class CqBucketProcessor
{
public:
	/** Default constructor */
	CqBucketProcessor();
	/** Default constructor
	 *
	 * \param bucket Bucket to be used
	 */
	CqBucketProcessor(CqBucket* bucket);
	/** Destructor */
	~CqBucketProcessor();

	/** Set the bucket to be processed */
	void setBucket(CqBucket* bucket);

	/** Reset the status of the object */
	void reset();

private:
	/// Pointer to the current bucket
	CqBucket* m_bucket;

	/// Bucket data for the current bucket
	CqBucketData m_bucketData;
};


/**
 * \brief Class to schedule the processing of Buckets.
 */
class CqBucketProcessorScheduler
{
public:
	
private:
	
};


END_NAMESPACE( Aqsis );

#endif
