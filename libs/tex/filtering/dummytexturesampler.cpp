// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
 *
 * \brief Dummy texture sampler for missing files
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "dummytexturesampler.h"

namespace Aqsis {

void CqDummyTextureSampler::sample(const SqSamplePllgram& samplePllgram,
	const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	SqSamplePllgram pllgram = samplePllgram;
	pllgram.remapPeriodic(true, true);

	// Make an ugly unfiltered black-on-white cross shape to indicate that the
	// texture isn't present.
	TqFloat x = pllgram.c.x();
	TqFloat y = pllgram.c.y();
	const TqFloat crossWidth = 0.10;
	const TqFloat borderWidth = 0.05;
	TqFloat result = 1;
	if((x < borderWidth || y < borderWidth) && y < 1-x)
	{
		result = 0.3;
	}
	else if((x > 1 - borderWidth || y > 1 - borderWidth) && y >= 1-x)
	{
		result = 0.7;
	}
	else if( (y < x + crossWidth && y > x - crossWidth)
		|| (y < 1 + crossWidth - x && y > 1 - crossWidth - x) )
	{
		result = 0;
	}

	// Scatter result into output channels.
	for(int i = 0; i < sampleOpts.numChannels(); ++i)
	{
		outSamps[i] = result;
	}
}


} // namespace Aqsis
