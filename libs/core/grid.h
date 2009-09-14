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
		\brief Declares the class to provide simple stratified sample data.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef GRID_H_INCLUDED //{
#define GRID_H_INCLUDED 1

#include	<aqsis/aqsis.h>

#include	<vector>

#include	"isampler.h"
#include	<aqsis/math/vector2d.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A class that produces jittered stratified samples.
 *
 * This class provides sample distribution data via the IqSampler interface that 
 * uses a standard stratified pattern, with jittering that maintains the 
 * sample distribution.
 *
 */
class CqGridSampler : public IqSampler
{
	public:
		CqGridSampler(TqInt samplesPerPixelX, TqInt samplesPerPixelY);
		~CqGridSampler();

		/* Interface functions from IqSampler */
		virtual const CqVector2D* get2DSamples();		
		virtual const TqFloat* get1DSamples();	
		virtual const TqInt* getShuffledIndices();

	private:
		TqInt numSamples() const;
		/** \brief Set up a single cached copy of the grid distribution.
		 *
		 */
		void setupGridPattern();

		TqInt					m_pixelXSamples;
		TqInt					m_pixelYSamples;
		TqFloat					m_openTime;
		TqFloat					m_closeTime;
		std::vector<CqVector2D>	m_2dSamples;
		std::vector<TqFloat>	m_1dSamples;
		std::vector<TqInt>		m_shuffledIndices;
};

//==============================================================================
// Implementation details
//==============================================================================

inline CqGridSampler::CqGridSampler(TqInt pixelXSamples, TqInt pixelYSamples) :
	m_pixelXSamples(pixelXSamples),
	m_pixelYSamples(pixelYSamples)
{
	m_1dSamples.resize(numSamples());
	m_2dSamples.resize(numSamples());
	m_shuffledIndices.resize(numSamples());
	setupGridPattern();
}

inline CqGridSampler::~CqGridSampler()
{
}

inline TqInt CqGridSampler::numSamples() const
{
	return m_pixelXSamples * m_pixelYSamples;
}


} // namespace Aqsis

#endif //} MULTIJITTER_H_INCLUDED

