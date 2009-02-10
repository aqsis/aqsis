// Aqsis
// Copyright Î÷Î÷ 1997 - 2001, Paul C. Gregory
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

#include	"csgtree.h"
#include	"color.h"
#include	"vector2d.h"

// Forward declare SqSampleData for the purposes of the
// boost::intrusive_ptr functionality.
namespace Aqsis
{
	struct SqSampleData;
};

// Declare required functions for boost::intrusive_ptr
// reference counting of SqSampleData.
namespace boost
{
	void intrusive_ptr_add_ref(Aqsis::SqSampleData* p);
	void intrusive_ptr_release(Aqsis::SqSampleData* p);
};

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


/** \brief A structure holding an array of sample values as floats.
 *
 * The data managed is a float array. The values stored follow the EqSampleIndices 
 * enum for the standard values, anything above Sample_Alpha is a custom 
 * entry AOV usage.
 */
struct SqImageSample
{
	/// Flags for this sample, using the anonymous enum below.
	TqInt		flags;
	/// Pointer to the managed data.
	TqFloat*	data;
	/// A shared pointer to the CSG node for this sample.
	/// If the sample originated from a surface that was part of a CSG tree
	/// this pointer will be valid, otherwise, it will be null.
	boost::shared_ptr<CqCSGTreeNode>	csgNode;	///< Pointer to the CSG node this sample is part of, NULL if not part of a solid.

	enum {
	    Flag_Occludes = 0x0001,
	    Flag_Matte = 0x0002,
	    Flag_Valid = 0x0004
	};

	static TqUint	sampleSize;

	/** \brief Default constructor.
 	 */
	SqImageSample();
	/** \brief Copy constructor.
 	 */
	SqImageSample(const SqImageSample& from);
	/** \brief Destructor.
 	 */
	~SqImageSample();

	/** \brief Assignment operator overload.
 	 *  Does a deep copy of the data assigned to the sample.
 	 *
 	 *  \param from - The sample to copy from.
 	 */
	SqImageSample& operator=(const SqImageSample& from);
};


/** Structure to hold the info about a sample point.
 */

struct SqSampleData
{
	SqSampleData()	:	references(0) {}

	CqVector2D	position;			///< Sample position
	CqVector2D	dofOffset;			///< Dof lens offset.
	TqInt		dofOffsetIndex;
	TqInt		subCellIndex;		///< Subcell index.
	TqFloat		time;				///< Float sample time.
	TqFloat		detailLevel;		///< Float level-of-detail sample.
	std::deque<SqImageSample>	data;	///< Array of sampled surface data for this sample.
	SqImageSample opaqueSample;	///< Single opaque sample for optimised processing if all encountered surfaces are opaque

	int			references;		///< Reference count for boost::intrusive_ptr
	/// boost::intrusive_ptr required function, to increment the reference count.
	friend		void ::boost::intrusive_ptr_add_ref(SqSampleData* p);
	/// boost::intrusive_ptr required function, to decrement the reference count.
	/// and delete if necessary.
	friend		void ::boost::intrusive_ptr_release(SqSampleData* p);
};

/// Intrusive reference counted pointer to a sample data structure.
typedef	boost::intrusive_ptr<SqSampleData>			SqSampleDataPtr;



//-----------------------------------------------------------------------
/** Storage class for all data relating to a single pixel in the image.
 */

class CqImagePixel
{
	public:
		/** \brief The default constructor.
		 */
		CqImagePixel();
		/** \brief The copy constructor.
		 * 
		 *  \param from	-	The pixel object to copy from.
		 */
		CqImagePixel( const CqImagePixel& ieFrom );
		/** \brief The destructor.
		 */
		~CqImagePixel();

		/** \brief Get the number of horizontal samples in this pixel
		 * \return The number of samples as an integer.
		 */
		TqInt	XSamples() const;
		/** \brief Get the number of vertical samples in this pixel
		 * \return The number of samples as an integer.
		 */
		TqInt	YSamples() const;
		/** \brief Allocate the sample pointer array.
		 *  
		 *  Allocates an array of shared pointers to the sample data stored on the 
		 *  bucket processor, thus registering an interest in those samples for the
		 *  lifetime of this bucket.
		 *
		 *  \param bp - A pointer to the bucket processor that this pixel belongs to.
		 *  \param XSamples - The number of samples in the x direction for the pixel.
		 *  \param YSamples - The number of samples in the y direction for the pixel.
		 */ 
		void	AllocateSamples( CqBucketProcessor* bp, TqInt XSamples, TqInt YSamples );
		/** \brief Initialise the sample positions for this pixel.
		 *
		 *  This function fills in the canonical information for the samples. It needs to 
		 *  be called only once when instantiated. It fills in default position, dof offset
		 *  and motion blur time information for later processing.
		 *
		 *  The sample positions are stored on the passed array, and 
		 *  are copied and adjusted for the bucket position when OffsetSamples is called.
		 *
		 *  \param vecSamples - An array of sample positions to fill in.
		 */
		void	InitialiseSamples( std::vector<CqVector2D>& vecSamples );
		/** \brief Jitter the sample positions.
		 *
		 *  Jitter the sample array using the multijitter function from GG IV.
		 *
		 *  The sample positions are multi-jittered from the canonical form in vecSamples,
		 *  the dof offset indices from the canonical form are shuffled, and the motion
		 *  blur time offsets are randomised.
		 *
		 *  \param vecSamples - The sample positions in canonical form, not adjusted for bucket position.
		 *  \param opentime - The motion blur shutter open time.
		 *  \param closetime - The motion blur shutter close time.
		 */
		void	JitterSamples( std::vector<CqVector2D>& vecSamples, TqFloat opentime, TqFloat closetime );
		/** \brief Offset the sample information according to the bucket position.
		 *  
		 *  Apply the offset in vecPixel to the sample positions stored in vecSamples, and then
		 *  store them on the sample data directly.
		 *
		 *  It is presumed that JitterSamples has already been called.
		 *
		 *  \param vecPixel - The 2D coordinate in screen space of this pixel.
		 *  \param vecSamples - The jittered positions to shift and apply.
		 *
		 */
		void	OffsetSamples( CqVector2D& vecPixel, std::vector<CqVector2D>& vecSamples );

		/** \brief Get the approximate coverage of this pixel.
		 * \return Float fraction of the pixel covered.
		 */
		TqFloat	Coverage() const;
		/** \brief Set the approximate coverage of this pixel.
		 * \param c - Fraction of the pixel covered.
		 */
		void	SetCoverage( TqFloat c );
		/** \brief Get the averaged color of this pixel
		 * \return A color representing the averaged color at this pixel.
		 * \attention Only call this after already calling FilterBucket().
		 */
		CqColor	Color() const;
		/** \brief Get the averaged opacity of this pixel
		 * \return A color representing the averaged opacity at this pixel.
		 * \attention Only call this after already calling FilterBucket().
		 */
		CqColor	Opacity() const;
		/** \brief Get the averaged depth of this pixel
		 * \return A float representing the averaged depth at this pixel.
		 * \attention Only call this after already calling FilterBucket().
		 */
		TqFloat	Depth() const;
		/** \brief Get the premultiplied alpha of this pixel
		 * \return A float representing the premultiplied alpha value of this pixel.
		 * \attention Only call this after already calling FilterBucket().
		 */
		TqFloat	Alpha() const;
		/** \brief Get a pointer to the sample data
		 * \return A constant pointer to the sample data.
		 */
		const TqFloat*	Data();

		/** \brief Clear all sample information from this pixel.
		 */
		void	Clear();
		/** \brief Get a reference to the array of values for the specified sample.
		 * \param m The horizontal index of the required sample point.
		 * \param n The vertical index of the required sample point.
		 * \return A Reference to a vector of SqImageSample data.
		 */
		std::deque<SqImageSample>&	Values( TqInt index );

		/** \brief Get a reference to the image sample that represents
		 *  the top if the closest sample is opaque.
		 *
		 *  \param index - The index of the sample within the pixel to query.
		 */
		SqImageSample& OpaqueValues( TqInt index );

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
		static void ProjectToCircle(CqVector2D& pos);

		/* These are public to allow direct shuffling */
		std::vector<SqSampleDataPtr> m_samples;
		std::vector<TqInt> m_DofOffsetIndices;	///< A mapping from dof bounding-box index to the sample that contains a dof offset in that bb.
	private:
		TqInt	m_XSamples;						///< The number of samples in the horizontal direction.
		TqInt	m_YSamples;						///< The number of samples in the vertical direction.
		SqImageSample	m_Data;
}
;

//-----------------------------------------------------------------------
// Implementation details
//


inline SqImageSample::SqImageSample() : flags(0)
{
	data = new TqFloat[sampleSize];
}

inline SqImageSample::SqImageSample(const SqImageSample& from)
{
	data = new TqFloat[sampleSize];
	*this = from;
}

inline SqImageSample::~SqImageSample()
{
	delete[](data);
}

inline SqImageSample& SqImageSample::operator=(const SqImageSample& from)
{
	flags = from.flags;
	csgNode = from.csgNode;

	const TqFloat* fromData = from.data;
	TqFloat* toData = data;
	for(TqUint i=0; i<sampleSize; ++i)
		toData[i] = fromData[i];

	return(*this);
}

inline TqInt CqImagePixel::XSamples() const
{
	return ( m_XSamples );
}

inline TqInt CqImagePixel::YSamples() const
{
	return ( m_YSamples );
}

inline TqFloat CqImagePixel::Coverage() const
{
	return ( m_Data.data[Sample_Coverage] );
}

inline void CqImagePixel::SetCoverage( TqFloat c )
{
	m_Data.data[Sample_Coverage] = c;
}

inline CqColor CqImagePixel::Color() const
{
	const TqFloat* data = m_Data.data;
	return ( CqColor(data[Sample_Red], data[Sample_Green], data[Sample_Blue]) );
}

inline CqColor CqImagePixel::Opacity() const
{
	const TqFloat* data = m_Data.data;
	return ( CqColor(data[Sample_ORed], data[Sample_OGreen], data[Sample_OBlue]) );
}

inline TqFloat CqImagePixel::Depth() const
{
	return ( m_Data.data[Sample_Depth] );
}

inline TqFloat CqImagePixel::Alpha() const
{
	return ( m_Data.data[Sample_Alpha] );
}

inline const TqFloat* CqImagePixel::Data()
{
	return ( &m_Data.data[0] );
}

inline TqInt CqImagePixel::GetDofOffsetIndex(TqInt i) const
{
	return m_DofOffsetIndices[i];
}

inline void CqImagePixel::ProjectToCircle(CqVector2D& pos)
{
	TqFloat r = pos.Magnitude();
	if( r == 0.0 )
		return;

	TqFloat adj = max(fabs(pos.x()), fabs(pos.y())) / r;
	pos.x(pos.x() * adj);
	pos.y(pos.y() * adj);
}

//-----------------------------------------------------------------------

} // namespace Aqsis

// Required implementation for boost::intrusive_ptr
namespace boost
{
	inline void intrusive_ptr_add_ref(Aqsis::SqSampleData* p)
	{
		++(p->references);
	}

	inline void intrusive_ptr_release(Aqsis::SqSampleData* p)
	{
		if(--(p->references) == 0)
			delete p;
	}
} // namespace boost

//}  // End of #ifdef IMAGEPIXEL_H_INCLUDED
#endif
