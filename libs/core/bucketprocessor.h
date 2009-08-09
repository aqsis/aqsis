// Aqsis - bucketprocessor.h
//
// Copyright (C) 2007 Manuel A. Fernadez Montecelo <mafm@users.sourceforge.net>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

/** \file
 *
 * \brief Bucket processing code
 *
 * \author Manuel A. Fernadez Montecelo <mafm@users.sourceforge.net>
 */

#ifndef BUCKETPROCESSOR_H_INCLUDED
#define BUCKETPROCESSOR_H_INCLUDED

#include	<aqsis/aqsis.h>

#include	<boost/array.hpp>

#include	"bucket.h"
#include	"channelbuffer.h"
#include	"imagepixel.h"
#include	"isampler.h"
#include	"occlusion.h"
#include	"optioncache.h"


namespace Aqsis {

class CqSampleIterator;
class CqRenderer;
class CqImageBuffer;

/** \brief Reyes processor for geometry covering a bucket.
 *
 * This class is responsible for rendering the geometry attached to a bucket
 * using the usual reyes procedure:
 *   1) split surfaces
 *   2) dice surfaces into micropolygon grids
 *   3) shade the grids
 *   4) sample the micropolygons against the sample points
 *   5) postprocess the samples (combine, filter, run imager shaders etc)
 *   6) Send the completed bucket to the display manager
 */
class CqBucketProcessor
{
	public:
		/** Default constructor */
		CqBucketProcessor(CqImageBuffer& imageBuf, const SqOptionCache& optCache);

		/** Set the bucket to be processed */
		void setBucket(CqBucket* bucket);
		/** Get the bucket to be processed */
		const CqBucket* getBucket() const;

		/** Reset the status of the object */
		void reset();

		/** Prepare the data for the bucket to be processed */
		void preProcess(IqSampler* sampler);

		/** Process the bucket, basically rendering the waiting MPs
		 */
		void process();

		/** Post-process the bucket, which involves the operations
		 * Combine and Filter
		 */
		void postProcess();

		//-------------- Reorganise -------------------------
		
		CqChannelBuffer& getChannelBuffer();

		const SqOptionCache& optCache() const;

		const CqRegion& SampleRegion() const;
		const CqRegion& DisplayRegion() const;
		const CqRegion& DataRegion() const;

		std::vector<CqImagePixelPtr>&	pixels();
		const std::vector<CqImagePixelPtr>&	pixels() const;

		/// Get an iterator over the samples in the region r.
		CqSampleIterator pixels(CqRegion& r);


	private:
		//--------------------------------------------------
		friend class CqSampleIterator;

		void	InitialiseFilterValues();
		void	CalculateDofBounds();
		void	CombineElements();
		void	FilterBucket();
		void	ExposeBucket();

		void	buildCacheSegment(SqBucketCacheSegment::EqBucketCacheSide side, boost::shared_ptr<SqBucketCacheSegment>& seg);
		void	applyCacheSegment(SqBucketCacheSegment::EqBucketCacheSide side, const boost::shared_ptr<SqBucketCacheSegment>& seg);
		void	dropSegment(TqInt side);

		void ImageElement( TqInt iXPos, TqInt iYPos, CqImagePixelPtr*& pie );

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
		/** This function assumes that either dof or mb or
		 * both are being used. */
		void	RenderMPG_MBOrDof( CqMicroPolygon* pMP, bool IsMoving, bool UsingDof );
		/** This function assumes that neither dof or mb are
		 * being used. It is much simpler than the general
		 * case dealt with above. */
		void	RenderMPG_Static( CqMicroPolygon* pMPG);
		void	StoreSample(CqMicroPolygon* pMPG, CqImagePixel* pie2, TqInt index,
							TqFloat D, const CqVector2D& uv);
		void	StoreExtraData( CqMicroPolygon* pMPG, TqFloat* hitData);
		const CqBound& DofSubBound(TqInt index) const;

		void setupCacheInformation();


		//--------------------------------------------------
		/// Pointer to the current bucket
		CqBucket* m_bucket;
		/// Pointer to the image buffer managing the render.
		CqImageBuffer& m_imageBuf;

		/// Cache of RiOptions for fast access during rendering.
		const SqOptionCache m_optCache;
		TqInt	m_DiscreteShiftX;
		TqInt	m_DiscreteShiftY;

		/// Bucket data for the current bucket
		TqInt	m_NumDofBounds;

		std::vector<CqBound>		m_DofBounds;
		std::vector<CqImagePixelPtr>	m_aieImage;
		CqPixelPool m_pixelPool;

		/// Vector of precalculated filter weights
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

		bool	m_hasValidSamples;

		CqChannelBuffer	m_channelBuffer;

		boost::array<CqRegion, SqBucketCacheSegment::last> m_cacheRegions;
};


/** \brief Iterator for a 2D region of samples
 *
 * This iterator provides access to samples held within a bucket processor.  It
 * encapsules all the machinary necessary to iterate over a 2D region of the
 * tiled sample storage.
 */
class CqSampleIterator
{
	public:
		/** Construct an iterator over samples held in the given processor,
		 * restricted to the given region
		 */
		CqSampleIterator(CqBucketProcessor& processor, CqRegion& r);

		/// Move to the next sample
		CqSampleIterator& operator++();

		//@{
		/// Return the associated sample data
		SqSampleData& operator*() const;
		SqSampleData* operator->() const;
		//@}

		//@{
		/** \brief Get the subpixel coordinates of the current sample
		 *
		 * These are *global* subpixel coordinates, calculated such that the
		 * top-left of the image (not the bucket) has coordinates 0,0.
		 * Multiplying the raster coordinates by the number of samples per
		 * pixel in the x and y directions gives the global subpixel
		 * coordinates.
		 */
		TqInt subPixelX() const;
		TqInt subPixelY() const;
		//@}

		/** \brief Test whether the iterator still points inside the region
		 *
		 * Return true if the iterator still points to samples within the
		 * valid region.  This unfortunately has to replace the standard "check
		 * against an end iterator" test, since the usual idiom doesn't easily
		 * generalise from 1D to 2D ranges.
		 */
		bool inRegion() const;

	private:
		CqImagePixel* getPixel(TqInt pixelX, TqInt pixelY);

		/// Bucket processor holding the pixels
		CqBucketProcessor* m_processor;
		/// Region of pixels to iterate over
		CqRegion m_region;
		/// Current pixel
		CqImagePixel* m_pixel;

		/// Pixel position inside m_region
		TqInt m_pixelX;
		TqInt m_pixelY;

		/// Number of subpixels within a pixel
		TqInt m_numSubPixels;
		/// Index of current subpixel
		TqInt m_subPixelIndex;
};


//==============================================================================
// Implementation details
//==============================================================================
// CqBucketProcessor implementation.

inline const SqOptionCache& CqBucketProcessor::optCache() const
{
	return m_optCache;
}

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

inline CqChannelBuffer& CqBucketProcessor::getChannelBuffer()
{
	return m_channelBuffer;
}

inline const CqBound& CqBucketProcessor::DofSubBound(TqInt index) const
{
	assert(index < m_NumDofBounds);
	return m_DofBounds[index];
}

inline std::vector<CqImagePixelPtr>& CqBucketProcessor::pixels()
{
	return m_aieImage;
}

inline const std::vector<CqImagePixelPtr>& CqBucketProcessor::pixels() const
{
	return m_aieImage;
}

inline CqSampleIterator CqBucketProcessor::pixels(CqRegion& r)
{
	return CqSampleIterator(*this, r);
}


//------------------------------------------------------------------------------
// CqSampleIterator implementation.
inline CqSampleIterator::CqSampleIterator(CqBucketProcessor& processor, CqRegion& r)
	: m_processor(&processor),
	m_region(r),
	m_pixel(0),
	m_pixelX(r.xMin()),
	m_pixelY(r.yMin()),
	m_numSubPixels(processor.m_optCache.xSamps*processor.m_optCache.ySamps),
	m_subPixelIndex(0)
{
	// Only get the pixel if the region is non-empty.
	if(r.width() > 0 && r.height() > 0)
		m_pixel = getPixel(m_pixelX, m_pixelY);
}

inline CqSampleIterator& CqSampleIterator::operator++()
{
	// Advance to the next subpixel
	++m_subPixelIndex;
	if(m_subPixelIndex >= m_numSubPixels)
	{
		// We went off the end of the pixel; advance to the next
		m_subPixelIndex = 0;
		++m_pixelX;
		if(m_pixelX >= m_region.xMax())
		{
			m_pixelX = m_region.xMin();
			++m_pixelY;
			if(m_pixelY < m_region.yMax())
				m_pixel = getPixel(m_pixelX, m_pixelY);
			else
				m_pixel = 0;
		}
		else
			m_pixel = getPixel(m_pixelX, m_pixelY);
	}
	return *this;
}

inline SqSampleData& CqSampleIterator::operator*() const
{
	assert(m_pixel);
	return m_pixel->SampleData(m_subPixelIndex);
}

inline SqSampleData* CqSampleIterator::operator->() const
{
	return &(this->operator*());
}

inline TqInt CqSampleIterator::subPixelX() const
{
	TqInt xSamples = m_processor->m_optCache.xSamps;
	return xSamples*m_pixelX + m_subPixelIndex % xSamples;
}
inline TqInt CqSampleIterator::subPixelY() const
{
	return m_processor->m_optCache.ySamps*m_pixelY
		+ m_subPixelIndex / m_processor->m_optCache.xSamps;
}

inline bool CqSampleIterator::inRegion() const
{
	return m_pixel;
}

/// Get the pixel at raster coordinates (pixelX, pixelY) from the processor.
inline CqImagePixel* CqSampleIterator::getPixel(TqInt pixelX, TqInt pixelY)
{
	TqInt x = pixelX - m_processor->m_DataRegion.xMin();
	TqInt y = pixelY - m_processor->m_DataRegion.yMin();
	return m_processor->m_aieImage[
		y*m_processor->m_DataRegion.width() + x].get();
}


} // namespace Aqsis

#endif
