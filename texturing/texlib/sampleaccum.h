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

/**
 * \file
 *
 * \brief Facilities for accumulating samples.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#ifndef SAMPLEACCUM_H_INCLUDED
#define SAMPLEACCUM_H_INCLUDED

#include "aqsis.h"

namespace Aqsis {

/** \name SampleAccumulatorConcept
 *
 * A sample accumulator serves as a place for accumulated sample data to be
 * stored.  During filtering, we iterate over some region in the source image.
 * For each position, (x,y) in the image we get a vector of sample data s(x,y).
 * Typically, this sample data is then weighted by some filter weight before
 * being added to an accumulated output sample vector:
 *
 * out = 0;
 * for (x,y) in filter support:
 *   out += weight(x,y) * s(x,y)
 * end
 *
 * However, other strategies for accumulation are possible, including things
 * like percentage-closer filtering.  This motivates us to write the operation
 * more abstractly as
 *
 * accumulator a;
 * for (x,y) in filter support:
 *   a.accumulate(x, y, s(x,y))
 * end
 *
 * This example illustrates the one of the methods which classes conforming to
 * SampleAccumulatorConcept must implement:
 *
 * template<typename SampleVectorT>
 * inline void accumulate(TqInt x, TqInt y, SampleVectorT samples);
 *
 * SampleVectorT is assumed to be some type which has an indexing operator[]
 * which returns floating point values.
 *
 * A further method should also be implemented for the SampleAccumulatorConcept:
 * the accumulator needs an efficient way to determine the length of the
 * samples, this is provided by the method:
 *
 * void setSampleVectorLength(TqInt sampleVectorLength);
 */

/** \brief A class to accumulate weighted sample data during filtering.
 *
 * This class implements the SampleAccumulatorConcept, representing a weighted
 * filtering operation over some filter support region.  The weights arise from
 * the FilterWeightT type, which must have have the following methods:
 *
 * operator()()
 * isNormalized()
 *
 * \todo Document some sort of rather specific FilterWeightConcept for
 * FilterWeightT.
 */
template<typename FilterWeightT>
class CqSampleAccum
{
	public:
		/** \brief Construct a filter sample accumulator
		 *
		 * \param filterWeights - functor returning filter weights
		 * \param startChan - channel index to begin extracting data from the
		 *                    accumulated sample vectors
		 * \param numChans - number of channels in the result
		 * \param resultBuf - float buffer to place the filtered result into.
		 * \param fill - value to fill nonexistant channels with.
		 */
		inline CqSampleAccum(const FilterWeightT& filterWeights,
				TqInt startChan, TqInt numChans, TqFloat* resultBuf,
				TqFloat fill = 0);

		/** \brief Set length for sample vectors passed to accumulate().
		 *
		 * In principle, this information could be a part of the inSamples
		 * parameter to accumulate(), but it's wasteful to have to check such
		 * information at each invocation of accumulate() when all the sample
		 * vectors should be the same length.
		 *
		 * \param length - the length of the sample vectors passed to accumulate.
		 */
		inline void setSampleVectorLength(TqInt sampleVectorLength);

		/** \brief Accumulate a sample into the output buffer at the given position.
		 *
		 * \param x
		 * \param y - position of the sample in the image plane.
		 * \param inSamples - input sample data to be accumulated
		 */
		template<typename SampleVectorT>
		inline void accumulate(TqInt x, TqInt y, const SampleVectorT& inSamples);

		/// Cleanup; renormalize the accumulated data if necessary.
		inline ~CqSampleAccum();
	private:
		const FilterWeightT& m_filterWeights;
		TqInt m_startChan;
		TqInt m_numChans;
		TqInt m_numChansFill;
		TqFloat* m_resultBuf;
		TqFloat m_fill;
		TqFloat m_totWeight;
};



//==============================================================================
// Implementation details
//==============================================================================

template<typename FilterWeightT>
inline CqSampleAccum<FilterWeightT>::CqSampleAccum(
		const FilterWeightT& filterWeights, TqInt startChan, TqInt numChans,
		TqFloat* resultBuf, TqFloat fill)
	: m_filterWeights(filterWeights),
	m_startChan(startChan),
	m_numChans(numChans),
	m_numChansFill(0),
	m_resultBuf(resultBuf),
	m_fill(fill),
	m_totWeight(0)
{
	// Zero the output channel on construction
	for(TqInt i = 0; i < m_numChans; ++i)
		m_resultBuf[i] = 0;
}

template<typename FilterWeightT>
inline void CqSampleAccum<FilterWeightT>::setSampleVectorLength(TqInt sampleVectorLength)
{
	assert(sampleVectorLength > 0);
	TqInt totNumChans = m_numChans + m_numChansFill;
	if(m_startChan + totNumChans <= sampleVectorLength)
	{
		// All channels should be filled with sample data
		m_numChans = totNumChans;
		m_numChansFill = 0;
	}
	else if(m_startChan >= sampleVectorLength)
	{
		// All channels should be filled with the "fill" value
		m_numChans = 0;
		m_numChansFill = totNumChans;
	}
	else
	{
		// Some channels get sample data; some get fill values.
		m_numChans = sampleVectorLength - m_startChan;
		m_numChansFill = totNumChans - m_numChans;
	}
}

template<typename FilterWeightT>
template<typename SampleVectorT>
inline void CqSampleAccum<FilterWeightT>::accumulate(TqInt x, TqInt y,
		const SampleVectorT& inSamples)
{
	TqFloat weight = m_filterWeights(x,y);

	// Some filters are likely to return a lot of zeros (eg, sinc, EWA), so we
	// check that the weight is nonzero before doing any filtering.
	if(weight != 0)
	{
		if(!m_filterWeights.isNormalized())
			m_totWeight += weight;
		for(TqInt i = 0; i < m_numChans; ++i)
			m_resultBuf[i] += weight*inSamples[i + m_startChan];
	}
}

template<typename FilterWeightT>
inline CqSampleAccum<FilterWeightT>::~CqSampleAccum()
{
	// Renormalize by the total weight if necessary.
	if(!m_filterWeights.isNormalized() && m_totWeight != 0)
	{
		TqFloat renorm = 1/m_totWeight;
		for(TqInt i = 0; i < m_numChans; ++i)
			m_resultBuf[i] *= renorm;
	}
	// Fill extra non-sampled channels with the "fill" value.
	for(TqInt i = 0; i < m_numChansFill; ++i)
		m_resultBuf[i+m_numChans] = m_fill;
}

} // namespace Aqsis

#endif // SAMPLEACCUM_H_INCLUDED
