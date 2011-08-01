// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/**
 * \file
 *
 * \brief Facilities for accumulating samples.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#ifndef SAMPLEACCUM_H_INCLUDED
#define SAMPLEACCUM_H_INCLUDED

#include <aqsis/aqsis.h>

namespace Aqsis {

/** \class SampleAccumulatorConcept
 * \brief Accumulator concept for texture samples during filtering.
 *
 * A sample accumulator serves as a place for accumulated sample data to be
 * stored.  During filtering, we iterate over some region in the source image.
 * For each position, (x,y) in the image we get a vector of sample data s(x,y).
 * Typically, this sample data is then weighted by some filter weight before
 * being added to an accumulated output sample vector:
 *
 * \verbatim
 *
 *   out = 0;
 *   for (x,y) in filter support:
 *     out += weight(x,y) * s(x,y)
 *   end
 *
 * \endverbatim
 *
 *
 * However, other strategies for accumulation are possible, including things
 * like percentage-closer filtering.  This motivates us to write the operation
 * more abstractly as
 *
 * \verbatim
 *
 *   accumulator a;
 *   for (x,y) in filter support:
 *     a.accumulate(x, y, s(x,y))
 *   end
 *
 * \endverbatim
 *
 *
 * This example illustrates one of the methods which classes conforming to
 * SampleAccumulatorConcept must implement.  The full interface is as follows:
 *
 * \code
 *
 * // Accumulate samples from the vector "samples".
 * // 
 * // samples is of a type SampleVectorT is assumed to have an indexing operator[]
 * // which returns floating point values.
 * template<typename SampleVectorT>
 * inline void accumulate(TqInt x, TqInt y, SampleVectorT samples);
 *
 * // The length of the "samples" vectors is provided to
 * // SampleAccumulatorConcept via the the following method which is called
 * // before the first call to accumulate() with the given vector lengths.
 * // 
 * // The return value from setSampleVectorLength is false if the accumulator
 * // can't deal with the given number of samples, true otherwise.  This can be
 * // used to shortcut sample accumulation when the sample vector is too short.
 * bool setSampleVectorLength(TqInt sampleVectorLength);
 *
 * \endcode
 */

//------------------------------------------------------------------------------
/** \brief Accumulator for a weighted average of filtered sample data.
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
		 *
		 * \return true if the length of the vector is long enough for samples
		 * to be accumulated, false otherwise.
		 */
		inline bool setSampleVectorLength(TqInt sampleVectorLength);

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
		/// 2D Filter weight function.
		const FilterWeightT& m_filterWeights;
		/// Start channel in the source data
		TqInt m_startChan;
		/// Number of channels in dest buffer
		TqInt m_numChans;
		/** Number of channels at the end of dest buffer which get the fill
		 * value rather than data from the source buffer.
		 */
		TqInt m_numChansFill;
		/// Array to fill with accumulated data
		TqFloat* m_resultBuf;
		/// Fill value for filling extra source data channels.
		TqFloat m_fill;
		/// Total accumulated weight used to renormalize the samples.
		TqFloat m_totWeight;
};


//------------------------------------------------------------------------------
/** \brief Sample accumulator for percentage closer filtering.
 *
 * At its simplest, percentage closer filtering determines the percentage of
 * points inside some filter radius which are closer to the light than the
 * value recorded in the shadow map.  It can also be combined with an arbitrary
 * weight function to perform weighted averaging of the function
 *
 * \verbatim
 *
 *   closer(x,y) = / 1  whenever filter_region_depth(x,y) < map_depth(x,y)
 *                 \ 0  elsewhere.
 * \endverbatim
 *
 * This combined approach is the one we take here:
 *
 * FilterWeightT is a functor which specifies the weight to be applied to given
 * (x,y) coordinates (an elliptical filter for example)
 *
 * DepthFuncT is a functor which determines the filter_region_depth at (x,y) to
 * be compared against the value of the raster map sample.
 */
template<typename FilterWeightT, typename DepthFuncT>
class CqPcfAccum
{
	public:
		/** \brief Construct a filter sample accumulator
		 *
		 * \param filterWeights - functor returning filter weights
		 * \param 
		 * \param startChan - channel index to begin extracting data from the
		 *                    accumulated sample vectors
		 * \param resultBuf - float buffer to place the filtered result into.
		 */
		CqPcfAccum(const FilterWeightT& filterWeights,
				const DepthFuncT& depthFunc,
				TqInt startChan, TqFloat biasLow, TqFloat biasHigh,
				TqFloat* resultBuf);

		/** \brief Set length for sample vectors passed to accumulate().
		 *
		 * For simple PCF on a single channel, it doesn't make sense for this
		 * function 
		 *
		 * \param length - the length of the sample vectors passed to accumulate.
		 *
		 * \return false if the sample vector is too short to be usefully accumulated.
		 */
		bool setSampleVectorLength(TqInt sampleVectorLength);

		/** \brief Accumulate a sample into the output buffer at the given position.
		 *
		 * \param x
		 * \param y - position of the sample in the image plane.
		 * \param inSamples - input sample data to be accumulated
		 */
		template<typename SampleVectorT>
		void accumulate(TqInt x, TqInt y, const SampleVectorT& inSamples);

		/// Cleanup; renormalize the accumulated data if necessary.
		~CqPcfAccum();
	private:
		/// 2D Filter weight function.
		const FilterWeightT& m_filterWeights;
		/// Functor determining the depth at a given (x,y) position.
		const DepthFuncT& m_depthFunc;
		/// Start channel in the source data
		TqInt m_startChan;
		/// Low value for shadow bias
		TqFloat m_biasLow;
		/// High value for shadow bias
		TqFloat m_biasHigh;
		/// Array to fill with accumulated data
		TqFloat* m_resultBuf;
		/// Total accumulated weight used to renormalize the samples.
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
inline bool CqSampleAccum<FilterWeightT>::setSampleVectorLength(TqInt sampleVectorLength)
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
		return false;
	}
	else
	{
		// Some channels get sample data; some get fill values.
		m_numChans = sampleVectorLength - m_startChan;
		m_numChansFill = totNumChans - m_numChans;
	}
	return true;
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

//------------------------------------------------------------------------------
// CqPcfAccum implementation
template<typename FilterWeightT, typename DepthFuncT>
inline CqPcfAccum<FilterWeightT, DepthFuncT>::CqPcfAccum(
		const FilterWeightT& filterWeights, const DepthFuncT& depthFunc,
		TqInt startChan, TqFloat biasLow, TqFloat biasHigh, TqFloat* resultBuf)
	: m_filterWeights(filterWeights),
	m_depthFunc(depthFunc),
	m_startChan(startChan),
	m_biasLow(biasLow),
	m_biasHigh(biasHigh),
	m_resultBuf(resultBuf),
	m_totWeight(0)
{
	// Zero the accumulated output channel
	m_resultBuf[0] = 0;
}

template<typename FilterWeightT, typename DepthFuncT>
inline bool CqPcfAccum<FilterWeightT, DepthFuncT>::setSampleVectorLength(TqInt sampleVectorLength)
{
	assert(sampleVectorLength > 0);
	if(sampleVectorLength <= m_startChan)
	{
		// Provided sample vectors will be too short; return false to indicate
		// that we don't want any calls to accumulate().
		return false;
	}
	return true;
}

template<typename FilterWeightT, typename DepthFuncT>
template<typename SampleVectorT>
inline void CqPcfAccum<FilterWeightT, DepthFuncT>::accumulate(TqInt x, TqInt y, const SampleVectorT& inSamples)
{
	TqFloat weight = m_filterWeights(x,y);
	if(weight != 0)
	{
		if(!m_filterWeights.isNormalized())
			m_totWeight += weight;
		TqFloat surfaceDepth = m_depthFunc(x,y);
		TqFloat shadDepth = inSamples[m_startChan];
		/// \todo Optimization (?): - we may make these decisions about biases *before* running the integration loop.  This will require several versions of CqPcfAccum.
		if(m_biasHigh == 0 && m_biasLow == 0)
		{
			// No shadow bias.
			m_resultBuf[0] += weight*(surfaceDepth > shadDepth);
		}
		else
		{
			if(m_biasHigh == m_biasLow)
			{
				m_resultBuf[0] += weight*(surfaceDepth > shadDepth + m_biasLow);
			}
			else
			{
				// handle biases; we interpolate from result == 0 when
				// surfaceDepth <= shadDepth+m_biasLow, to result == 1 when
				// surfaceDepth >= shadDepth+m_biasHigh.
				TqFloat shadAmount = 0;
				if(surfaceDepth >= shadDepth + m_biasHigh)
					shadAmount = 1;
				else if(surfaceDepth > shadDepth + m_biasLow)
				{
					shadAmount = (surfaceDepth - shadDepth - m_biasLow)/(m_biasHigh-m_biasLow);
				}
				m_resultBuf[0] += weight*shadAmount;
			}
		}
	}
}

template<typename FilterWeightT, typename DepthFuncT>
inline CqPcfAccum<FilterWeightT, DepthFuncT>::~CqPcfAccum()
{
	// Renormalize by the total weight if necessary.
	if(!m_filterWeights.isNormalized() && m_totWeight != 0)
	{
		m_resultBuf[0] /= m_totWeight;
	}
}

} // namespace Aqsis

#endif // SAMPLEACCUM_H_INCLUDED
