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
		\brief Declares the class to provide jittered stratified sample data.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef MULTIJITTER_H_INCLUDED //{
#define MULTIJITTER_H_INCLUDED 1

#include	<aqsis/aqsis.h>

#include	<vector>

#include	"isampler.h"
#include	<aqsis/math/vector2d.h>
#include	<aqsis/math/random.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A class that produces jittered stratified samples.
 *
 * This class provides sample distribution data via the IqSampler interface that 
 * uses a standard stratified pattern, with jittering that maintains the 
 * sample distribution.
 *
 */
class CqMultiJitteredSampler : public IqSampler
{
	public:
		CqMultiJitteredSampler(TqInt samplesPerPixelX, TqInt samplesPerPixelY);
		~CqMultiJitteredSampler();

		/* Interface functions from IqSampler */
		virtual const CqVector2D* get2DSamples();		
		virtual const TqFloat* get1DSamples();		
		virtual const TqInt* getShuffledIndices();

	private:
		/// Static define for the number of distribution patterns to cache.
		static const TqInt m_cacheSize = 250;
		TqInt numSamples() const;
		void multiJitterIndices(TqInt* indices, TqInt numX, TqInt numY);
		/** \brief Set up a jittered sample pattern for a pixel's worth of samples.
		 *
		 * Jitter the sample array using the multijitter function from GG IV.
		 *
		 * The sample positions are multi-jittered from the canonical form,
		 * the dof offset vectors from the canonical form are shuffled, and the motion
		 * blur time offsets are randomised.
		 *
		 * \param offset - The offset within the table of sample arrays to start
		 * 					storing the values.
		 */
		void setupJitterPattern(TqInt offset);

		TqInt					m_pixelXSamples;
		TqInt					m_pixelYSamples;
		TqFloat					m_openTime;
		TqFloat					m_closeTime;
		std::vector<CqVector2D>	m_2dSamples;
		std::vector<TqFloat>	m_1dSamples;
		std::vector<TqInt>		m_shuffledIndices;
		CqRandom				m_random;
};

//==============================================================================
// Implementation details
//==============================================================================

inline CqMultiJitteredSampler::CqMultiJitteredSampler(TqInt pixelXSamples, TqInt pixelYSamples) :
	m_pixelXSamples(pixelXSamples),
	m_pixelYSamples(pixelYSamples)
{
	m_1dSamples.resize(numSamples()*m_cacheSize);
	m_2dSamples.resize(numSamples()*m_cacheSize);
	m_shuffledIndices.resize(numSamples()*m_cacheSize);

	for(TqInt i = 0; i < m_cacheSize; ++i)
		setupJitterPattern(i*numSamples());
	m_random.Reseed(19);
}

inline CqMultiJitteredSampler::~CqMultiJitteredSampler()
{
}

inline TqInt CqMultiJitteredSampler::numSamples() const
{
	return m_pixelXSamples * m_pixelYSamples;
}




} // namespace Aqsis

#endif //} MULTIJITTER_H_INCLUDED

