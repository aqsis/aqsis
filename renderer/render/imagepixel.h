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

//? Is imagebuffer.h included already?
#ifndef IMAGEPIXEL_H_INCLUDED
//{
#define IMAGEPIXEL_H_INCLUDED 1

#include	"aqsis.h"

#include	<vector>
#include	<stack>
#include	<deque>
#include	<valarray>

#include	<boost/intrusive_ptr.hpp>
#include	<boost/scoped_array.hpp>
#include	<boost/noncopyable.hpp>

#include	"csgtree.h"
#include	"color.h"
#include	"vector2d.h"
#include	"pool.h"

namespace Aqsis {

class CqBucketProcessor;

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

enum EqFilterDepth
{
    Filter_Min = 0,
    Filter_MidPoint = 1,
    Filter_Max = 2,
    Filter_Average = 3,
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
	/// Flags for this sample, using the anonymous enum below.
	TqInt flags;
	/// Index to the data sample hit array managed by the associated CqImagePixel
	TqInt index;
	/// A shared pointer to the CSG node for this sample.
	/// If the sample originated from a surface that was part of a CSG tree
	/// this pointer will be valid, otherwise, it will be null.
	boost::shared_ptr<CqCSGTreeNode> csgNode;

	enum {
	    Flag_Occludes = 0x0001,
	    Flag_Matte = 0x0002,
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
	TqInt		subCellIndex;		///< Subcell index.
	TqFloat		time;				///< Float sample time.
	TqFloat		detailLevel;		///< Float level-of-detail sample.
	std::deque<SqImageSample>	data;	///< Array of sampled surface data for this sample.
	SqImageSample opaqueSample;	///< Single opaque sample for optimised processing if all encountered surfaces are opaque

	/// Default constructs all class members and sets numeric members to zero.
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
		 * Removes all the semitransparent sample hits and resets the opaque
		 * sample hits to invalid.
		 */
		void clear();

		/** \brief Get a reference to the array of values for the specified sample.
		 * \param index the index of the sample point within the pixel
		 */
		std::deque<SqImageSample>&	Values( TqInt index );

		/** \brief Get a reference to the image sample that represents
		 *  the top if the closest sample is opaque.
		 *
		 *  \param index - The index of the sample within the pixel to query.
		 */
		SqImageSample& OpaqueValues( TqInt index );

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
		void	Combine( EqFilterDepth eDepthFilter, CqColor zThreshold );

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

	private:
		void initialiseDofOffsets();
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
}; 

/// Intrusive reference counted pointer to a pixel class.
typedef	boost::intrusive_ptr<CqImagePixel>			CqImagePixelPtr;


//==============================================================================
// Implementation details
//==============================================================================

//------------------------------------------------------------------------------
// SqImageSample implementation
inline SqImageSample::SqImageSample()
	: flags(0),
	index(-1),
	csgNode()
{ }

inline SqImageSample::SqImageSample(const SqImageSample& from)
	: flags(from.flags),
	index(from.index),
	csgNode(from.csgNode)
{ }

inline SqImageSample& SqImageSample::operator=(const SqImageSample& from)
{
	flags = from.flags;
	index = from.index;
	csgNode = from.csgNode;

	return *this;
}


//------------------------------------------------------------------------------
// SqSampleData implementation
inline SqSampleData::SqSampleData()
	: position(),
	dofOffset(),
	subCellIndex(0),
	time(0),
	detailLevel(0),
	data(),
	opaqueSample()
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

inline std::deque<SqImageSample>&	CqImagePixel::Values( TqInt index )
{
    assert(index < numSamples());
	return m_samples[index].data;
}

inline SqImageSample& CqImagePixel::OpaqueValues( TqInt index )
{
	assert(index < numSamples());
	return m_samples[index].opaqueSample;
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

} // namespace Aqsis

//}  // End of #ifdef IMAGEPIXEL_H_INCLUDED
#endif
