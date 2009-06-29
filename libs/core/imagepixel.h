// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Declares the CqImagePixel class responsible for storing the results.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef IMAGEPIXEL_H_INCLUDED //{
#define IMAGEPIXEL_H_INCLUDED 1

#include	<aqsis/aqsis.h>

#include	<vector>
#include	<cfloat> // for FLT_MAX

#include	<boost/intrusive_ptr.hpp>
#include	<boost/scoped_array.hpp>
#include	<boost/noncopyable.hpp>

#include	<aqsis/math/color.h>
#include	<aqsis/math/vector2d.h>
#include	"csgtree.h"
#include	"optioncache.h"
#include	"isampler.h"

namespace Aqsis {

//-----------------------------------------------------------------------
/** Structure representing the information at a sample point in the image.
 */

enum EqSampleIndices
{
    Sample_Red = 0,
    Sample_Green = 1,
    Sample_Blue = 2,
    Sample_ORed = 3,
    Sample_OGreen = 4,
    Sample_OBlue = 5,
    Sample_Depth = 6,
    Sample_Coverage = 7,
    Sample_Alpha = 8,
};


/** \brief Holder for data from a hit of a micropoly against a sample point.
 *
 * We allocate hit data in a single large block for all the samples inside a
 * pixel.  This explicit pooling of the data improves cache locality
 * dramatically which is particularly necessary when pulling pixels from the
 * bucket overlap cache and for scenes with lots of semitransparent depth
 * complexity.
 *
 * The float array values stored at index in the associated CqImagePixel follow
 * the EqSampleIndices enum for the standard values, anything above
 * Sample_Alpha is a custom entry AOV usage.  See SqImagePixel::sampleHitData()
 */
struct SqImageSample
{
	/// Index to the data sample hit array managed by the associated CqImagePixel
	TqInt index;
	/// Flags for this sample, using the anonymous enum below.
	TqUint flags;
	/// A shared pointer to the CSG node for this sample.
	/// If the sample originated from a surface that was part of a CSG tree
	/// this pointer will be valid, otherwise, it will be null.
	boost::shared_ptr<CqCSGTreeNode> csgNode;

	/** \brief Flags indicating the type of sample.
	 *
	 * MatteAlpha is a special aqsis-specific type of matte object which is
	 * *always* fully opaque from the point of view of the hider, but which
	 * actually has a user-specifiable alpha and colour.  This is helpful when
	 * trying to render shadows cast by CG objects onto parts of a live-action
	 * set.  Ditto for reflections.
	 */
	enum {
	    Flag_Matte = 0x0001,
	    Flag_MatteAlpha = 0x0002,
	    Flag_Valid = 0x0004
	};

	static TqInt sampleSize;

	/** \brief Default constructor.
 	 */
	SqImageSample();
	/** \brief Copy constructor.
 	 */
	SqImageSample(const SqImageSample& from);

	/** \brief Assignment operator overload.
 	 *  Does a deep copy of the data assigned to the sample.
 	 *
 	 *  \param from - The sample to copy from.
 	 */
	SqImageSample& operator=(const SqImageSample& from);
};


/** Structure to hold the info about a sample point.
 */

struct SqSampleData : private boost::noncopyable
{
	CqVector2D	position;			///< Sample position
	CqVector2D	dofOffset;			///< Dof lens offset.
	TqUint      occlusionIndex;     ///< Index for sample in occlusion tree.
	TqFloat		time;				///< Float sample time.
	TqFloat		detailLevel;		///< Float level-of-detail sample.
	std::vector<SqImageSample> data;	///< Array of surface "hits" for this sample.
	/** \brief Minimum depth hit which occludes any hits further away
	 *
	 * During micropolygon sampling, occludingHit is used to store the surface
	 * hit which is closest to the camera for the sample point.  Any
	 * micropolygon hits further away than this can be culled without being
	 * stored.  A micropolygon hit can occlude other surfaces when
	 * 1) The micropoly is opaque
	 * 2) The micropoly does not participate in CSG
	 * 3) The z depthfilter is "min" or "midpoint" (midpoint uses special case code).
	 */
	SqImageSample occludingHit;
	/** \brief Occluding depth.
	 *
	 * This should be the same as the depth in occludingHit, *except* when a
	 * depth filter mode not equal to "min" is enabled.  (ie, the "midpoint"
	 * or other more exotic depth filters)
	 */
	TqFloat occlZ;

	/// Default construct members & set numeric members to 0 except occlZ=FLT_MAX.
	SqSampleData();
};


//-----------------------------------------------------------------------
/** Storage class for all data relating to a single pixel in the image.
 */

class CqImagePixel : private boost::noncopyable
{
	public:
		/** \brief Construct a pixel with a given supersampling resolution.
		 *
		 * \param xSamples - number of sub-pixel samples in the x-direction
		 * \param ySamples - number of sub-pixel samples in the y-direction
		 */
		CqImagePixel(TqInt xSamples, TqInt ySamples);

		/** \brief Swap the internal sample data with another pixel.
		 *
		 * This function swaps both the sample data and associated DoF indices
		 * with another pixel by efficiently swapping the internal data
		 * structures.
		 */
		void swap(CqImagePixel& other);

		/** \brief Get the number of horizontal samples in this pixel
		 * \return The number of samples as an integer.
		 */
		TqInt	XSamples() const;
		/** \brief Get the number of vertical samples in this pixel
		 * \return The number of samples as an integer.
		 */
		TqInt	YSamples() const;

		/** \brief Set up a jittered sample pattern for the pixel
		 *
		 * Jitter the sample array using the multijitter function from GG IV.
		 *
		 * The sample positions are multi-jittered from the canonical form,
		 * the dof offset indices from the canonical form are shuffled, and the motion
		 * blur time offsets are randomised.
		 *
		 * \param offset - The raster space offset of the bucket.
		 * \param opentime - The motion blur shutter open time.
		 * \param closetime - The motion blur shutter close time.
		 */
		void setupJitterPattern(CqVector2D& offset, TqFloat opentime,
				TqFloat closetime);
		/** \brief Set up a regular sample pattern for the pixel
		 *
		 * The samples are evenly distributed over the pixel on a regular grid.
		 * Sample times and detail levels are also regularly placed.  DoF
		 * offsets are left untouched.
		 *
		 * \param offset - The raster space offset of the bucket.
		 * \param opentime - The motion blur shutter open time.
		 * \param closetime - The motion blur shutter close time.
		 */
		void setupGridPattern(CqVector2D& offset, TqFloat opentime,
				TqFloat closetime);

		/** \brief Clear all sample information from this pixel.
		 *
		 * Removes all the semitransparent sample hits and resets the occluding
		 * sample hits to invalid.
		 */
		void clear();

		/** \brief Get a reference to the array of values for the specified sample.
		 * \param index the index of the sample point within the pixel
		 */
		std::vector<SqImageSample>& Values( TqInt index );

		/** \brief Get a reference to the image hit that represents the top
		 * if the closest sample is occluding.
		 *
		 *  \param index - The index of the sample within the pixel to query.
		 */
		SqImageSample& occludingHit( TqInt index );

		//@{
		/** \brief Return the sample data associated with a micropolygon sample hit.
		 *
		 * \param hit - a sample hit which has had data allocated with allocateHitData()
		 */
		const TqFloat* sampleHitData(const SqImageSample& hit) const;
		TqFloat* sampleHitData(const SqImageSample& hit);
		//@}

		/// Allocate space for a single block of sample hit data.
		void allocateHitData(SqImageSample& hit);

		/** \brief Combine the sample values accumulated at each sample.
		 *  
		 *  The successful sample hits recorded at each sample point are
		 *  combined using alpha blending to produce a final visible color
		 *  at the top of the sample.
		 *
		 *  \param eDepthFilter - The filter to use to combine depth values.
		 *  \param zThreshold - The color value at which to consider a sample opaque
		 *  					when sampling depth.
		 */
		void	Combine( EqDepthFilter eDepthFilter, CqColor zThreshold );

		/** \brief Get the sample data for the specified sample index.
		 *
		 * \param The index of the required sample point.
		 * \return A reference to the sample data.
		 */
		SqSampleData const& SampleData( TqInt index ) const;

		/** \brief Get the sample data for the specified sample index.
		 *
		 * \param The index of the required sample point.
		 * \return A reference to the sample data.
		 */
		SqSampleData& SampleData( TqInt index );

		/// Get the number of samples in the contained within the pixel.
		TqInt numSamples() const;

		/** \brief Get the index of the sample that contains a dof offset that lies
		 *  in bounding-box number i.
		 *
		 * \param The index of the bounding box in question.
		 * \return The index of the sample that contains a dof offset in said bb.
		 */
		TqInt GetDofOffsetIndex(TqInt i) const;

		/** \brief Convert a coord in the unit square to one inside the unit circle.
		 *  used in generating dof sample positions.
		 *
		 *  \param pos - The 2D coordinate to convert.
		 */
		static CqVector2D projectToCircle(const CqVector2D& pos);

		/// Return the number of references to the pixel
		TqInt refCount() const;

		/// Mark this pixel as having valid samples.
		void markHasValidSamples();
	
		/// Check if the pixel has any valid samples.
		bool hasValidSamples();

		/** \brief Fill in the sample data using the given distribution object.
		 *  Initialise the camera sample information for this pixel, including
		 *  position, depth of field data, motion time and level of detail values.
		 *
		 *  \param sampler - A pointer to an object that provides a sample distribution
		 *					 via the IqSampler interface.
		 */
		void setSamples(IqSampler* sampler, CqVector2D& offset);

	private:
		/// boost::intrusive_ptr required function, to increment the reference count.
		friend		void intrusive_ptr_add_ref(CqImagePixel* p);
		/// boost::intrusive_ptr required function, to decrement the reference count.
		/// and delete if necessary.
		friend		void intrusive_ptr_release(CqImagePixel* p);

		/// The number of samples in the horizontal direction.
		TqInt m_XSamples;
		/// The number of samples in the vertical direction.
		TqInt m_YSamples;
		/// Array of sample positions within this pixel
		boost::scoped_array<SqSampleData> m_samples;
		/// Vector storing sample data for the sample hits within the pixel.
		std::vector<TqFloat> m_hitSamples;
		/// A mapping from dof bounding-box index to the sample that contains a
		/// dof offset in that bb.
		boost::scoped_array<TqInt> m_DofOffsetIndices;
		/// Reference count for boost::intrusive_ptr
		int m_refCount;
		/// A flag to indicate successful sample hits in this pixel.
		bool m_hasValidSamples;
}; 

/// Intrusive reference counted pointer to a pixel class.
typedef	boost::intrusive_ptr<CqImagePixel>			CqImagePixelPtr;


/** \brief A pool for reusing pixel data structures.
 *
 * Reusing pixels is useful, since it allows the allocated per-pixel
 * sample hit storage to grow adaptively to the necessary size, after
 * which no further allocation is necessary.
 */
class CqPixelPool
{
	public:
		/** \brief Initialise the pool
		 *
		 * \param xSamples - number of subpixel samples in the x-direction
		 * \param ySamples - number of subpixel samples in the y-direction
		 */
		CqPixelPool(TqInt xSamples, TqInt ySamples);

		/// Allocate a new CqImagePixel, or return a pooled one.
		CqImagePixelPtr allocate();

		/** \brief Add a pixel to the pool to be reused, and reset the pointer.
		 *
		 * The pixel is only freed if the associated reference count is one.
		 * If not, perform no action.
		 */
		void free(CqImagePixelPtr& pixel);
	private:
		TqInt m_xSamples;
		TqInt m_ySamples;
		std::vector<CqImagePixelPtr> m_pool;
};


//==============================================================================
// Implementation details
//==============================================================================

//------------------------------------------------------------------------------
// SqImageSample implementation
inline SqImageSample::SqImageSample()
	: index(-1),
	flags(0),
	csgNode()
{ }

inline SqImageSample::SqImageSample(const SqImageSample& from)
	: index(from.index),
	flags(from.flags),
	csgNode(from.csgNode)
{ }

inline SqImageSample& SqImageSample::operator=(const SqImageSample& from)
{
	index = from.index;
	flags = from.flags;
	csgNode = from.csgNode;

	return *this;
}


//------------------------------------------------------------------------------
// SqSampleData implementation
inline SqSampleData::SqSampleData()
	: position(),
	dofOffset(),
	occlusionIndex(0),
	time(0),
	detailLevel(0),
	data(),
	occludingHit(),
	occlZ(FLT_MAX)
{ }


//------------------------------------------------------------------------------
// CqImagePixel implementation
inline TqInt CqImagePixel::XSamples() const
{
	return ( m_XSamples );
}

inline TqInt CqImagePixel::YSamples() const
{
	return ( m_YSamples );
}

inline TqInt CqImagePixel::numSamples() const
{
	return m_XSamples*m_YSamples;
}

inline TqInt CqImagePixel::GetDofOffsetIndex(TqInt i) const
{
	return m_DofOffsetIndices[i];
}

inline CqVector2D CqImagePixel::projectToCircle(const CqVector2D& pos)
{
	TqFloat r = pos.Magnitude();
	if( r == 0.0 )
		return CqVector2D(0,0);
	TqFloat adj = max(fabs(pos.x()), fabs(pos.y())) / r;
	return adj*pos;
}

inline TqInt CqImagePixel::refCount() const
{
	return m_refCount;
}

inline std::vector<SqImageSample>& CqImagePixel::Values( TqInt index )
{
    assert(index < numSamples());
	return m_samples[index].data;
}

inline SqImageSample& CqImagePixel::occludingHit( TqInt index )
{
	assert(index < numSamples());
	return m_samples[index].occludingHit;
}

inline const TqFloat* CqImagePixel::sampleHitData(const SqImageSample& hit) const
{
	assert(hit.index >= 0);
	assert(hit.index + SqImageSample::sampleSize <= static_cast<TqInt>(m_hitSamples.size()));
	return &m_hitSamples[hit.index];
}

inline TqFloat* CqImagePixel::sampleHitData(const SqImageSample& hit)
{
	assert(hit.index >= 0);
	assert(hit.index + SqImageSample::sampleSize <= static_cast<TqInt>(m_hitSamples.size()));
	return &m_hitSamples[hit.index];
}

inline void CqImagePixel::allocateHitData(SqImageSample& hit)
{
	assert(hit.index == -1);
	// Using a std::vector for m_hitSamples allows the sample size to grow as
	// necessary with O(log(N)) reallocations for N hits.  The reallocation
	// time is irrelevant when CqImagePixel's are recycled through the pipeline
	// since the vector grows to the necessary size in the first few buckets
	// and remains there for the rest of the frame.
	hit.index = m_hitSamples.size();
	m_hitSamples.resize(m_hitSamples.size() + SqImageSample::sampleSize);
}

inline SqSampleData const& CqImagePixel::SampleData( TqInt index ) const
{
	assert(index < numSamples());
	return m_samples[index];
}

inline SqSampleData& CqImagePixel::SampleData( TqInt index )
{
	assert(index < numSamples());
	return m_samples[index];
}

inline void intrusive_ptr_add_ref(Aqsis::CqImagePixel* p)
{
	++(p->m_refCount);
}

inline void intrusive_ptr_release(Aqsis::CqImagePixel* p)
{
	if(--(p->m_refCount) == 0)
		delete p;
}

inline void CqImagePixel::markHasValidSamples()
{
	m_hasValidSamples = true;
}

inline bool CqImagePixel::hasValidSamples()
{
	return m_hasValidSamples;
}

//------------------------------------------------------------------------------
// CqPixelPool implementation
inline CqPixelPool::CqPixelPool(TqInt xSamples, TqInt ySamples)
	: m_xSamples(xSamples),
	m_ySamples(ySamples),
	m_pool()
{ }

inline CqImagePixelPtr CqPixelPool::allocate()
{
	if(!m_pool.empty())
	{
		CqImagePixelPtr pixel = m_pool.back();
		m_pool.pop_back();
		assert(pixel->XSamples() == m_xSamples);
		assert(pixel->YSamples() == m_ySamples);
		pixel->clear();
		return pixel;
	}
	return CqImagePixelPtr(new CqImagePixel(m_xSamples, m_ySamples));
}

inline void CqPixelPool::free(CqImagePixelPtr& pixel)
{
	assert(pixel->XSamples() == m_xSamples);
	assert(pixel->YSamples() == m_ySamples);
	if(pixel->refCount() == 1)
	{
		m_pool.push_back(pixel);
		pixel = 0;
	}
}


} // namespace Aqsis

#endif //} IMAGEPIXEL_H_INCLUDED
