/* Aqsis - bucketprocessor.h
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
#include	"bucket.h"
#include	"channelbuffer.h"
#include	"occlusion.h"


namespace Aqsis {


/**
 * \brief Class to process Buckets.
 */
class CqBucketProcessor
{
	public:
		/** Default constructor */
		CqBucketProcessor();
		/** Destructor */
		~CqBucketProcessor();

		/** Set the bucket to be processed */
		void setBucket(CqBucket* bucket);
		/** Get the bucket to be processed */
		const CqBucket* getBucket() const;

		/** Reset the status of the object */
		void reset();

		/** Get the next sample point from the bucket data */
		SqSampleDataPtr GetNextSamplePoint();

		/** Prepare the data for the bucket to be processed */
		void preProcess(TqInt xMin, TqInt yMin, TqInt xMax, TqInt yMax,
				TqInt pixelXSamples, TqInt pixelYSamples, TqFloat filterXWidth, TqFloat filterYWidth,
				TqInt viewRangeXMin, TqInt viewRangeXMax, TqInt viewRangeYMin, TqInt viewRangeYMax,
				TqFloat clippingNear, TqFloat clippingFar);

		/** Process the bucket, basically rendering the waiting MPs
		 */
		void process();

		/** Post-process the bucket, which involves the operations
		 * Combine and Filter
		 */
		void postProcess( bool imager, EqFilterDepth depthfilter, const CqColor& zThreshold );

		//-------------- Reorganise -------------------------
		
		CqChannelBuffer& getChannelBuffer();

		TqUint numSamples() const;
		const std::vector<SqSampleDataPtr>& samplePoints() const;

		const CqRegion& SampleRegion() const;
		const CqRegion& DisplayRegion() const;
		const CqRegion& DataRegion() const;

	private:
		void	InitialiseFilterValues();
		void	CalculateDofBounds();
		void	CombineElements(enum EqFilterDepth eDepthFilter, CqColor zThreshold);
		void	FilterBucket(bool fImager);
		void	ExposeBucket();

		TqFloat	FilterXWidth() const;
		TqFloat	FilterYWidth() const;
		TqInt	PixelXSamples() const;
		TqInt	PixelYSamples() const;
		void ImageElement( TqInt iXPos, TqInt iYPos, CqImagePixel*& pie );

		CqImagePixel& ImageElement(TqUint index) const;
		/** Render any waiting MPs.
		 */
		void RenderWaitingMPs();
		void RenderSurface( boost::shared_ptr<CqSurface>& surface);
		void ImageElement( TqInt iXPos, TqInt iYPos, CqImagePixel*& pie ) const;
		/** Render a particular micropolygon.
		 *
		 * \param pMPG Pointer to the micropolygon to process.
		 * \see CqBucket, CqImagePixel
		 */
		void	RenderMicroPoly( CqMicroPolygon* pMP );
		void	RenderMPG_MBOrDof( CqMicroPolygon* pMP, bool IsMoving, bool UsingDof );
		void	StoreSample( CqMicroPolygon* pMPG, CqImagePixel* pie2, TqInt index, TqFloat D );
		void	StoreExtraData( CqMicroPolygon* pMPG, SqImageSample& sample);
		/** This function assumes that neither dof or mb are
		 * being used. It is much simpler than the general
		 * case dealt with above. */
		void	RenderMPG_Static( CqMicroPolygon* pMPG);
		/** This function assumes that either dof or mb or
		 * both are being used. */
		bool 	occlusionCullSurface(const boost::shared_ptr<CqSurface>& surface);
		const CqBound& DofSubBound(TqInt index) const;


		/// Pointer to the current bucket
		CqBucket* m_bucket;

		/// Bucket data for the current bucket
		TqInt	m_NumDofBounds;

		std::vector<CqBound>		m_DofBounds;
		std::vector<CqImagePixel>	m_aieImage;
		std::vector<SqSampleDataPtr>	m_samplePoints;
		TqInt	m_NextSamplePoint;
		/// Vector of vectors of jittered sample positions precalculated.
		std::vector<std::vector<CqVector2D> >	m_aSamplePositions;
		/// Vector of filter weights precalculated.
		std::vector<TqFloat>	m_aFilterValues;

		SqMpgSampleInfo m_CurrentMpgSampleInfo;

		CqOcclusionTree m_OcclusionTree;

		// View range and clipping info (to know when to skip rendering)
		/// The total size of the array of sample available for this bucket.
		CqRegion	m_DataRegion;
		/// The area of the samples array that will be used to sample micropolygons.
		CqRegion	m_SampleRegion;
		/// The area of the samples array that will be sent to the display. 
		CqRegion	m_DisplayRegion;

		TqInt	m_DiscreteShiftX;
		TqInt	m_DiscreteShiftY;
		TqInt	m_PixelXSamples;
		TqInt	m_PixelYSamples;
		TqFloat	m_FilterXWidth;
		TqFloat	m_FilterYWidth;

		TqFloat	m_clippingNear;
		TqFloat	m_clippingFar;

		bool	m_hasValidSamples;

		CqChannelBuffer	m_channelBuffer;
};

inline const CqRegion& CqBucketProcessor::DataRegion() const
{
	return m_DataRegion;
}

inline const CqRegion& CqBucketProcessor::SampleRegion() const
{
	return m_SampleRegion;
}

inline const CqRegion& CqBucketProcessor::DisplayRegion() const
{
	return m_DisplayRegion;
}

inline TqFloat	CqBucketProcessor::FilterXWidth() const
{
	return m_FilterXWidth;
}

inline TqFloat	CqBucketProcessor::FilterYWidth() const
{
	return m_FilterYWidth;
}

inline TqInt	CqBucketProcessor::PixelXSamples() const
{
	return m_PixelXSamples;
}

inline TqInt	CqBucketProcessor::PixelYSamples() const
{
	return m_PixelYSamples;
}

inline SqSampleDataPtr CqBucketProcessor::GetNextSamplePoint()
{
	TqInt index = m_NextSamplePoint;
	m_NextSamplePoint++;
	return(m_samplePoints[index]);
}

inline TqUint CqBucketProcessor::numSamples() const
{
	return DataRegion().area() * PixelXSamples() * PixelYSamples();
}

inline const std::vector<SqSampleDataPtr>& CqBucketProcessor::samplePoints() const
{
	return m_samplePoints;
}

inline CqChannelBuffer& CqBucketProcessor::getChannelBuffer()
{
	return m_channelBuffer;
}

inline const CqBound& CqBucketProcessor::DofSubBound(TqInt index) const
{
	assert(index < m_NumDofBounds);
	return m_DofBounds[index];
}

} // namespace Aqsis

#endif
