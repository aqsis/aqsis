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
		\brief Implements the CqGridSampler class responsible for providing stratified sample positions.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include "grid.h"

#include <algorithm>

#include <aqsis/math/math.h>

namespace Aqsis {


//----------------------------------------------------------------------

void CqGridSampler::setupGridPattern()
{
	TqInt nSamples = numSamples();
	// Initialize positions to a grid.
	TqFloat xScale = 1.0/m_pixelXSamples;
	TqFloat yScale = 1.0/m_pixelYSamples;
	for(TqInt j = 0; j < m_pixelYSamples; j++)
	{
		for(TqInt i = 0; i < m_pixelXSamples; i++)
		{
			m_2dSamples[j*m_pixelXSamples + i] = CqVector2D(xScale*(i+0.5), yScale*(j+0.5));
		}
	}

	// Fill in motion blur and LoD with the same regular grid
	TqFloat dt = 1/nSamples;
	TqFloat sample = dt*0.5;
	for(TqInt i = 0; i < nSamples; ++i)
	{
		m_1dSamples[i] = sample;
		sample += dt;
	}

	// Shuffled indices in grid sampler are just canonical distribution.
	for(TqInt i = 0; i < nSamples; ++i)
		m_shuffledIndices[i] = i;
}

const CqVector2D* CqGridSampler::get2DSamples()		
{
	return &m_2dSamples[0];
}

const TqFloat* CqGridSampler::get1DSamples()		
{
	return &m_1dSamples[0];
}

const TqInt* CqGridSampler::getShuffledIndices()		
{
	return &m_shuffledIndices[0];
}

//---------------------------------------------------------------------

} // namespace Aqsis
