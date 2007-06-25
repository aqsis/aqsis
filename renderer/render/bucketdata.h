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
 * \brief File holding data to process buckets.
 *
 * \author Manuel A. Fernadez Montecelo <mafm@users.sourceforge.net>
 */

#ifndef BUCKETDATA_H_INCLUDED
#define BUCKETDATA_H_INCLUDED 1

#include	"aqsis.h"
#include	"bound.h"
#include	"color.h"
#include	"imagepixel.h"

#include	<vector>

START_NAMESPACE( Aqsis );


/** \brief Used to hold info about a Micropolygon that is used when
 * rendering the Micropolygon.
 *
 *  It caches the info for use by multiple samples.
 */
struct SqMpgSampleInfo
{
	CqColor		m_Colour;
	CqColor		m_Opacity;
	/// Whether the opacity is full.
	bool		m_Occludes;
	/// Whether the mpg can use the faster StoreOpaqueSample
	/// routine that assumes a few things.
	bool		m_IsOpaque;
};



/**
 * \brief Class to hold dynamic data of Buckets being processed.
 */
class CqBucketData
{
	/// The bucket class can manipulate directly our private data
	friend class CqBucket;

public:
	/** Default constructor */
	CqBucketData();

	/** Default destructor */
	~CqBucketData();

	/** Reset the values to the initial state */
	void reset();

private:
	/// Origin in discrete coordinates of this bucket.
	TqInt	m_XOrigin;
	/// Origin in discrete coordinates of this bucket.
	TqInt	m_YOrigin;
	/// Size of the rendered area of this bucket in discrete coordinates.
	TqInt	m_XSize;
	/// Size of the rendered area of this bucket in discrete coordinates.
	TqInt	m_YSize;
	/// Actual size of the data for this bucket including filter overlap.
	TqInt	m_RealWidth;
	/// Actual size of the data for this bucket including filter overlap.
	TqInt	m_RealHeight;

	TqInt	m_DiscreteShiftX;
	TqInt	m_DiscreteShiftY;
	TqInt	m_PixelXSamples;
	TqInt	m_PixelYSamples;
	TqFloat	m_FilterXWidth;
	TqFloat	m_FilterYWidth;
	TqInt	m_NumTimeRanges;
	TqInt	m_NumDofBounds;

	std::vector<CqBound>		m_DofBounds;
	std::vector<CqImagePixel>	m_aieImage;
	std::vector<SqSampleData>	m_SamplePoints;
	TqInt	m_NextSamplePoint;
	/// Vector of vectors of jittered sample positions precalculated.
	std::vector<std::vector<CqVector2D> >	m_aSamplePositions;
	/// Vector of filter weights precalculated.
	std::vector<TqFloat>	m_aFilterValues;
	std::vector<TqFloat>	m_aDatas;
	std::vector<TqFloat>	m_aCoverages;

	SqMpgSampleInfo m_CurrentMpgSampleInfo;
};


END_NAMESPACE( Aqsis );

#endif
